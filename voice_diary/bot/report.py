# -*- coding: utf-8 -*-
#acutally not:)

from cmath import pi
import os
import datetime

import telebot
import json

import librosa
import librosa.display
import matplotlib.pyplot as plt
import numpy as np

import parselmouth
from parselmouth.praat import call

import seaborn as sns

from subprocess import check_call #used for ffmpeg ogg to wav

import threading
import time
import requests

from cloud_storage import upload_file


#TODO 2: search for keywords from diary cards



class ReportGenerator:

	def __init__(self, config_name):

		with open(config_name, 'r') as file:
			self._config = json.load(file)

		self.bot = telebot.TeleBot(self._config["key"])

		self.use_cross_matrix = False
		self.de_personalization = False
		self.skip_plots = False
		self.include_sequences = True


	def request_recognition(self, record_file_path, alias_name):
		
		upload_file(record_file_path,  alias_name)

		filelink = 'https://storage.yandexcloud.net/' + self._config["bucket"]  + '/' + alias_name 

		POST = "https://transcribe.api.cloud.yandex.net/speech/stt/v2/longRunningRecognize"

		body ={
			"config": {
				"specification": {
					"languageCode": "ru-RU"
				}
			},
			"audio": {
				"uri": filelink
			}
		}

		key = self._config["api-key"]
		header = {'Authorization': 'Api-Key {}'.format(key)}

		req = requests.post(POST, headers=header, json=body)
		data = req.json()
		#print(data) #Log of received data

		id = data['id']
		return id



	def check_server_recognition(self, id):

		key = self._config["api-key"]
		header = {'Authorization': 'Api-Key {}'.format(key)}

		while True:

			GET = "https://operation.api.cloud.yandex.net/operations/{id}"
			req = requests.get(GET.format(id=id), headers=header)
			req = req.json()

			if req['done']: break
			print("Not ready")

			time.sleep(10) #TODO calc

		return req



	def make_sequence_cut(self, step_size, start, end, sequence):
		
		idx_start = int(start / step_size)
		idx_end = int(end / step_size)

		return sequence[idx_start: idx_end]



	def statistic_value(self, sequence, type):

		import statistics
		import math

		sub_sequence = [x for x in sequence if (math.isnan(x) == False and x != 0)]

		if len(sub_sequence) == 0:
			return 0.0

		if type == "mean":
			return statistics.mean(sub_sequence)

		if type == "mode":
			try:
				return statistics.mode(sub_sequence)
			except:
				return 0 #TODO, review


		if type == "median":
			return statistics.median(sub_sequence)

		if type == "min":
			return min(sub_sequence)
		
		if type == "max":
			return max(sub_sequence)

		if type == "stddev":
			try:
				return statistics.stdev(sub_sequence)
			except:
				return 0 #TODO, review

		print("WARNING: wrong statistic value: ", type)
		return 0.0



	def get_full_statistics(self, sequence):

		if len(sequence) <= 2:
			return {"error": "too small sequence"}

		full_stats = {"mean": self.statistic_value(sequence, type="mean"),
				"mode": self.statistic_value(sequence, type="mode"),
				"median": self.statistic_value(sequence, type="median"),
				"min": self.statistic_value(sequence, type="min"),
				"max": self.statistic_value(sequence, type="max"),
				"SD" : self.statistic_value(sequence, type="stddev")}

		return full_stats
				

	def make_morph_analysis(self, word):
		import pymorphy2
		morph = pymorphy2.MorphAnalyzer()
		p = morph.parse(word)[0]
		morph_tab = p.tag

		part_of_speech = morph_tab.POS
		aspect = morph_tab.aspect 
		case = p.tag.case    
		gender = p.tag.gender
		involement = p.tag.involvement
		mood = p.tag.mood
		number = p.tag.number
		person = p.tag.person
		tense = p.tag.tense
		transitivity = p.tag.transitivity
		voice = p.tag.voice       

		morph_analysis = {"part_of_speech" : part_of_speech, "aspect" : aspect, "case" : case,
		"gender": gender, "involement": involement, "mood":mood, "number": number, 
		"person": person, "tense": tense, "transitivity": transitivity, "voice": voice}

		return morph_analysis


	def parse_praat_info(self, info_text):

		if info_text.find("--undefined--") != -1:
			return {"error": "praat info undefined"}

		fields = [["duration:","seconds"], ["Median pitch:","Hz"], ["Mean pitch:","Hz"],
		["Standard deviation:","Hz"], ["Minimum pitch:","Hz"], ["Maximum pitch:","Hz"],
		["Number of pulses:",""], ["Number of periods:", ""], ["Mean period:","seconds"],
		["Standard deviation of period:", "seconds"], ["Fraction of locally unvoiced frames:", "%"],
		["Number of voice breaks:",""],["Degree of voice breaks:", "%"],
		["Jitter (local):", "%"], ["Jitter (local, absolute):", "seconds"],
		["Jitter (rap):", "%"], ["Jitter (ppq5):", "%"], ["Jitter (ddp):", "%"],
		["Shimmer (local):", "%"], ["Shimmer (local, dB):", "dB"], 
		["Shimmer (apq3):", "%"], ["Shimmer (apq5):", "%"],
		["Shimmer (apq11):", "%"], ["Shimmer (apq11):", "%"],
		["Shimmer (dda):", "%"], ["Mean autocorrelation:", ""],
		["Mean noise-to-harmonics ratio:",""],
		["Mean harmonics-to-noise ratio:", "dB"]]

		praat_dict = {}

		for field_info in fields:

			field_name = field_info[0]
			field_sepparator = field_info[1]

			name_pos = info_text.find(field_name)

			if name_pos != -1:

				field_value = ""

				if field_sepparator != "":

					sep_pos = info_text.find(field_sepparator, name_pos + len(field_name))
					next_line_pos = info_text.find("\n", name_pos)

					if sep_pos > next_line_pos:
						field_value = "0"
					else:
						field_value = info_text[name_pos + len(field_name): sep_pos - 1]

				else:
					sep_pos = info_text.find("\n", name_pos)
					field_value = info_text[name_pos + len(field_name): sep_pos]

				field_value = field_value.strip()
				field_name = field_name[:-1]
				
				dict_key = field_name

				#use_full_names = True #TODO to config
				#if use_full_names:
					#dict_key += field_sepparator

				praat_dict[field_name] = float(field_value)

				#TODO exceptions everywhere..!

		return praat_dict


	def make_json_report(self, req, seq_dict):

		import datetime

		start_moment = datetime.datetime.now()

		#TODO receive all f0/intense as dictionary
		#Check each field exists, if not - avoid calculation of stats, and cutting sequence

		#CHECK POINT
		#==========================================Prepare basic information sequences==========================================
		duration = seq_dict["duration"]
		intensity = seq_dict["praat_intensity"].values.T #Check copy avoided
		pitch = seq_dict["praat_pitch"]

		f0 = seq_dict["librosa_pitch"] #rename to librosa pitch TODO
		rms = seq_dict["librosa_rms"]

		#TODO maybe cover under seq_dict and use cut in another way
		f0_step = duration / len(f0)
		rms_step = duration / len(rms[0])
		pitch_step = duration / len(pitch)
		intensity_step = duration / len(intensity)

		pitch = pitch.selected_array['frequency']
		intensity = intensity.reshape(intensity.shape[0] * intensity.shape[1])
		rms = rms.reshape(rms.shape[0] * rms.shape[1])

		pitch = np.array(pitch) #TODO CHECK USAGE ONLY BY REFERENCE of list? Just slices
		intensity = np.array(intensity)
		rms = np.array(rms)
		f0 = np.array(f0)

		swipe_contour = np.array(seq_dict["swipe_contour"][0])
		swipe_step = duration / len(swipe_contour)
		intense_contour = np.array(seq_dict["intense_contour"][0]) #TODO full fill

		reshape_sequences_moment =  datetime.datetime.now()
		surf_moment =  datetime.datetime.now()

		snd = seq_dict["praat_sound"]

		pitch_for_praat = seq_dict["praat_pitch"]
		pulses = seq_dict["praat_pulses"]
		f0min = 60 #TODO move under seq_dict or better config
		f0max = 600


		#TODO parse report into dictionary
		full_report = call([snd, pitch_for_praat, pulses], "Voice report", 0, duration, f0min, f0max,
							1.3, 1.6, 0.03, 0.45) #TODO make configurable

		praat_moment =  datetime.datetime.now()
		#==========================================Prepare basic information sequences==========================================

		events = []
		prev_word_end = 0.0
		chunks = []
		words_freq = {}

		de_personalization = False #TODO to config

		tokens = {}
		tokens_count = 0

		full_stats = {"pyin_pitch":self.get_full_statistics(f0), "praat_pitch": self.get_full_statistics(pitch),
					"swipe_pitch" : self.get_full_statistics(swipe_contour),
					"rms":self.get_full_statistics(rms), "intensity":self.get_full_statistics(intensity)}

		full_text = ""

		all_starts = []
		all_ends = []

		total_words = 0

		chunkId = 0

		for chunk in req['response']['chunks']:

			altId = 0
			for alt in chunk['alternatives']: #We don't handle silence right yet in case for alts

				first_start = -1.0

				for word in alt['words']:

					start = float(word['startTime'][:-1])
					end = float(word['endTime'][:-1])

					word_duration = end - start
					letters_speed = word_duration / len(word['word'])

					all_starts.append(start)
					all_ends.append(end)

					if first_start == -1.0:
						first_start = start

					silence_start = prev_word_end
					silence_end = start

					pause_RMS = self.make_sequence_cut(rms_step, silence_start, silence_end, rms) #TODO dont copy, use numpy slices
					pause_intens = self.make_sequence_cut(intensity_step, silence_start, silence_end, intensity)#TODO dont copy, use numpy slices

					pause_RMS = self.get_full_statistics(pause_RMS)
					pause_intens = self.get_full_statistics(pause_intens)

					silence_report = ""

					single_pause = {"type":"pause", "startTime": silence_start, "endTime": silence_end, 
									"RMS": pause_RMS, "Intensity": pause_intens, "info": silence_report}; #, "dB": list(pause_intens)

					
					events.append(single_pause)

					prev_word_end = end

					f0_cut = np.array([])
					pitch_cut = np.array([])
					intens_cut = np.array([])
					rms_cut = np.array([])
					swipe_cut = np.array([])
					#TODO + volume v2

					if self.include_sequences:
						f0_cut = self.make_sequence_cut(f0_step, start, end, f0) #TODO dont copy, use numpy slices
						pitch_cut = self.make_sequence_cut(pitch_step, start, end, pitch)  #TODO dont copy, use numpy slices
						intens_cut = self.make_sequence_cut(intensity_step, start, end, intensity) #TODO dont copy, use numpy slices
						rms_cut = self.make_sequence_cut(rms_step, start, end, rms) #TODO dont copy, use numpy slices
						swipe_cut = self.make_sequence_cut(swipe_step, start, end, swipe_contour) #TODO dont copy, use numpy slices

					statistics_records = {"pyin_pitch":self.get_full_statistics(f0_cut), "praat_pitch": self.get_full_statistics(pitch_cut),
						"rms":self.get_full_statistics(rms_cut), "intensity":self.get_full_statistics(intens_cut),
						"swipe_pitch": self.get_full_statistics(swipe_cut)}

					report_string = call([snd, pitch_for_praat, pulses], "Voice report", start, end, f0min, f0max,
							1.3, 1.6, 0.03, 0.45)

					morph_analysis = self.make_morph_analysis(word['word'])

					token_id = 0

					current_word = word["word"]

					if current_word not in tokens:
						tokens[current_word] = tokens_count + 1
						tokens_count += 1
						token_id = tokens_count
					else:
						token_id = tokens[current_word]

					if de_personalization: #TODO CONFIG
						current_word = '-'


					singleWord =  {"type":"word",  "chunkId" : chunkId, "altId": altId, "word": current_word, 
					"startTime": start, "endTime": end, 
					"confidence": word['confidence'], 
					"pyin_pitch": list(f0_cut),
					"RMS": list(rms_cut), #TODO or use list? test speed
					"praat_pitch": list(pitch_cut),
					"swipe_pitch" : list(swipe_cut)
					#,"dB": np.array2string(intens_cut)
					,"stats" : statistics_records,  "info": self.parse_praat_info(report_string)
					,"morph" : morph_analysis
					,"token_id" : token_id
					,"word_idx" : total_words
					,"letters_speed" : letters_speed
					} #channel tag left away

					total_words += 1

					events.append(singleWord)

					if token_id in words_freq:
						words_freq[token_id] += 1
					else:
						words_freq[token_id] = 1

				#FILL chunk

				f0_cut = self.make_sequence_cut(f0_step, first_start, prev_word_end, f0) #TODO dont copy, use numpy slices
				pitch_cut = self.make_sequence_cut(pitch_step, first_start, prev_word_end, pitch)#TODO dont copy, use numpy slices
				intens_cut = self.make_sequence_cut(intensity_step, first_start, prev_word_end, intensity)#TODO dont copy, use numpy slices
				rms_cut = self.make_sequence_cut(rms_step, first_start, prev_word_end, rms)#TODO dont copy, use numpy slices
				swipe_cut = self.make_sequence_cut(swipe_step, start, end, swipe_contour)#TODO dont copy, use numpy slices

				statistics_records = {"pyin_pitch":self.get_full_statistics(f0_cut), "praat_pitch": self.get_full_statistics(pitch_cut),
					"swipe_pitch": self.get_full_statistics(swipe_cut), "rms":self.get_full_statistics(rms_cut), "intensity":self.get_full_statistics(intens_cut)}

				chunk_text = alt["text"]
				if self.de_personalization:
						chunk_text = '-'

				chunk_report = call([snd, pitch_for_praat, pulses], "Voice report", first_start, prev_word_end, f0min, f0max,
							1.3, 1.6, 0.03, 0.45) #TODO make configurable

				single_chunk = {"chunkId": chunkId, "altId": altId, "stats": statistics_records,  
								"start" : first_start, "end": prev_word_end, "words_speed": (prev_word_end - first_start) / len(alt["words"]),
								"text": chunk_text, "praat_report": self.parse_praat_info(chunk_report)}
				
				full_text += chunk_text + ". "

				chunks.append(single_chunk)

				altId += 1

			chunkId += 1

		if self.de_personalization == True:
			tokens = {}


		cross_stats = []

		all_chnunks_and_events_moment = datetime.datetime.now()

		if self.use_cross_matrix:
			for i in range(0, len(all_starts) - 1):
				for j in range(i + 1, len(all_ends)):
					cross_start = all_starts[i]
					cross_end = all_ends[j]

					#We can add here anything else yet its enough

					cross_report = call([snd, pitch_for_praat, pulses], "Voice report",
										cross_start, cross_end, f0min, f0max,
										1.3, 1.6, 0.03, 0.45) 

					cross_element = {"praat_report": self.parse_praat_info(cross_report), "start" : cross_start,
									"end": cross_end, "first_word_idx": i, "last_word_idx": j}

					cross_stats.append(cross_element)

		cross_matrix__moment = datetime.datetime.now()

		praat_dict = self.parse_praat_info(full_report)

		root_element = {"events": events, "full_stats": full_stats, "chunks": chunks,
						"words_freq": words_freq, "full_text": full_text, "tokens": tokens,
						"jitters": seq_dict["global_jitters"], "shimmers": seq_dict["global_shimmers"],
						"formants": seq_dict["global_formants"],
						"HNR": seq_dict["global_hnr"], "praat_report": praat_dict,
						"cross_stats": cross_stats, "duration": duration}

		json_report = json.dumps(root_element, indent = 4, ensure_ascii=False) 

		full_report_generated = datetime.datetime.now()

		total_spent = full_report_generated - start_moment
		print("Total on report: ", total_spent.seconds, "s ", total_spent.microseconds, " micro")

		rehape_spent = reshape_sequences_moment - start_moment
		surf_spent = surf_moment - reshape_sequences_moment
		praat_spemt = praat_moment - surf_moment
		all_chunks_spent = all_chnunks_and_events_moment - praat_moment
		cross_spent = cross_matrix__moment - all_chnunks_and_events_moment
		dump_spent = full_report_generated - cross_matrix__moment

		print("Reshape sequences ", rehape_spent.seconds, "s ", rehape_spent.microseconds, " micro")
		print("Surf ", surf_spent.seconds, "s ", surf_spent.microseconds, " micro")
		print("Praat ", praat_spemt.seconds, "s ", praat_spemt.microseconds, " micro")
		print("All chunks", all_chunks_spent.seconds, "s ", all_chunks_spent.microseconds, " micro")
		print("Cross matrix ", cross_spent.seconds, "s ", cross_spent.microseconds, " micro")
		print("Dump spent, ", dump_spent.seconds, "s ", dump_spent.microseconds, " micro")
			
		return json_report



	def save_downloaded_and_name(self, path_user_logs, message, downloaded_file):

		record_file_path = path_user_logs + '/record_' + str(message.id) + '.ogg'
		spectrum_dir_path = path_user_logs

		print(record_file_path, " <- dir path")

		with open(os.path.join(record_file_path), 'wb') as new_file:
			new_file.write(downloaded_file)

		alias_name = "a" + str(message.chat.id)  + "b" + str(message.id) + ".ogg"

		return record_file_path, alias_name, spectrum_dir_path


	def save_images_info(self, spectrum_dir_path, message, voice_report): #TODO lately sepprate bot and generator

		rosaInfo = open(spectrum_dir_path + '/rosaInfo.png', 'rb')
		self.bot.send_photo(message.chat.id, rosaInfo)

		praatInfo = open(spectrum_dir_path + '/praatInfo.png', 'rb')
		self.bot.send_photo(message.chat.id, praatInfo)

		self.bot.reply_to(message, voice_report)


	def save_json_products(self, path_user_logs, json_report, full_string):

		with open(path_user_logs + '/full_report.json', 'w') as outfile:
			outfile.write(json_report)

		with open(path_user_logs + '/stt.json', 'w') as outfile:
			outfile.write(full_string)


	def merge_text_from_request(self, req): #TODO использовать эту функцию при генерации полного текста

		text_lines = []
		message_text = ""

		print("Text chunks:")
		for chunk in req['response']['chunks']:
			print(chunk['alternatives'][0]['text'])
			text_lines.append(chunk['alternatives'][0]['text']) #TODO check alternatives
			message_text += chunk['alternatives'][0]['text'] + "\n"

		return message_text


	def send_message_and_reports(self, path_user_logs, message, message_text):

		self.bot.reply_to(message, message_text)

		doc = open(path_user_logs + '/stt.json', 'rb')
		self.bot.send_document(message.chat.id, doc)

		doc = open(path_user_logs + '/full_report.json', 'rb')
		self.bot.send_document(message.chat.id, doc)


	def deplayed_recognition(self, path_user_logs, message, downloaded_file):

		record_file_path, alias_name, spectrum_dir_path = self.save_downloaded_and_name(path_user_logs, message, downloaded_file)

		id = self.request_recognition(record_file_path, alias_name)

		voice_report, f0, rms, pitch, intensity, duration = self.save_images(record_file_path, spectrum_dir_path) 

		self.save_images_info(spectrum_dir_path, message, voice_report)

		req = self.check_server_recognition(id)

		wav_file = spectrum_dir_path + "/pcm.wav"

		full_string = json.dumps(req, ensure_ascii=False, indent=2)
		json_report = self.make_json_report(req, f0, rms, pitch, intensity, duration, wav_file)

		self.save_json_products(spectrum_dir_path, json_report, full_string)

		message_text = self.merge_text_from_request(req)
			
		self.send_message_and_reports(spectrum_dir_path, message, message_text)

		commands_response = self.detect_commands(message_text)
		if commands_response != '':
			self.bot.reply_to(message, commands_response)
			


	def detect_commands(self, text):

		text = text.lower()

		request_string = "создать задачу"
		create_aim_pos = text.find(request_string)
		if create_aim_pos != -1:
			return "Создается задача с именем: " + text[create_aim_pos + len(request_string):]

		request_string = "начать задачу"
		start_aim_pos = text.find(request_string)
		if start_aim_pos != -1:
			return "Начата задача с именем: " + text[start_aim_pos + len(request_string):]
			#TODO datetime - в словарь

		request_string = "завершить задачу"
		finish_aim_pos = text.find(request_string)
		if finish_aim_pos != -1:
			return "Завершена задача с именем: " + text[finish_aim_pos + len(request_string):]
			#TODO datetime - из словаря + разница

		request_string = "я съел"
		eat_pos = text.find(request_string)
		if eat_pos != -1:
			return "Записан продукт[ы] питания: " + text[eat_pos + len(request_string):]

		request_string = "я выпил"
		drink_pos = text.find(request_string)
		if drink_pos != -1:
			return "Записан напиток: " + text[drink_pos + len(request_string):]

		request_string = "я принял"
		meds_pos = text.find(request_string)
		if meds_pos != -1:
			return "Записан препарат: "  + text[meds_pos + len(request_string):]

		request_string = "применен навык"
		skills_pos = text.find(request_string)
		if skills_pos != -1:
			return "Зафиксированно применения навыка\[ов]" + text[skills_pos + len(request_string):]

		request_string = "заполнить поле"
		field_pos = text.find(request_string)

		request_string_2 = "значением"
		value_pos = text.find(request_string_2)

		if field_pos != -1 and value_pos != -1: #Если значение не задано - то это просто бинарное поле
			field_name = text[field_pos + len(request_string): value_pos - 1]
			value_text = text[value_pos + len(request_string_2): ]
			return "Поле: " + field_name +" заполненно значением " + value_text

		if field_pos != -1 and value_pos == -1:
			field_name = text[field_pos + len(request_string): value_pos - 1]
			return "Поле: " + field_name + " отмечено"

		#Рецепт - игредиенты и приготовление

		return ""


	def send_delayed_text(self, message):
		print("Озвучивание текста")

		from synth_speech import text_to_audio
		text_to_audio("123.ogg", message.text)

		self._bot.reply_to(message, 'Озвучивание:')
		voice = open("123.ogg", 'rb')
		self._bot.send_voice(message.chat.id, voice)

		print("Текст озвучен")


	def draw_intensity_praat(self, intensity): #Отделить всю отрисовку в отдельный класс

		plt.plot(intensity.xs(), intensity.values.T, linewidth=3, color='g')
		plt.plot(intensity.xs(), intensity.values.T, linewidth=1)
		plt.grid(False)
		plt.ylim(0, 100)
		plt.ylabel("intensity [dB]")



	def draw_pitch_praat(self, pitch):

		pitch_values = pitch.selected_array['frequency']
		pitch_values[pitch_values==0] = np.nan
		plt.plot(pitch.xs(), pitch_values, 'o', markersize=5, color='r')
		plt.plot(pitch.xs(), pitch_values, 'o', markersize=2, color='r')
		plt.grid(False)
		plt.ylim(0, pitch.ceiling)
		plt.ylabel("fundamental frequency [Hz]")



	def plot_pitches(self, seq_dict, output_filepath):

		fig = plt.figure()

		f0 = seq_dict["librosa_pitch"]
		pitch = seq_dict["praat_pitch"]

		times = librosa.times_like(f0)
		plt.plot(times, f0, color='green', linewidth=5)
		plt.ylim(0, pitch.ceiling)

		pitch_values = pitch.selected_array['frequency']
		pitch_values[pitch_values==0] = np.nan
		plt.plot(pitch.xs(), pitch_values, 'o', markersize=3, color='r')
		plt.grid(False)
		plt.ylim(0, pitch.ceiling)
		plt.ylabel("fundamental frequency [Hz]")

		fig.set_size_inches(12, 9)

		plt.savefig(output_filepath + '/pitches.png', bbox_inches='tight')



	#TODO REVIEW OPTIMIZE проверять расширение входного файла, вынести от сюда всю загрзуку и оставить только отрисовку
	def save_images(self, seq_dict):
		#TODO rename all
		self.plot_librosa(seq_dict, self._config["dir"]) 
		self.plot_praat(seq_dict, self._config["dir"])
		self.plot_pitches(seq_dict, self._config["dir"]) #TODO заменить self._config["dir"]



	def plot_praat(self, seq_dict, output_filepath): 

		snd = seq_dict["praat_sound"]
		intensity = seq_dict["praat_intensity"]

		pitch = seq_dict["praat_pitch"] #OR! snd.to_pitch()

		fig = plt.figure()
		
		self.draw_pitch_praat(pitch)

		plt.twinx()
		self.draw_intensity_praat(intensity)
		plt.xlim([snd.xmin, snd.xmax])

		fig.set_size_inches(12, 9)
		plt.savefig(output_filepath + '/praatInfo.png', bbox_inches='tight')



	def plot_librosa(self, seq_dict, output_filepath):

		rms = seq_dict["librosa_rms"]
		times = seq_dict["librosa_times"]
		f0 = seq_dict["librosa_pitch"]
		S = seq_dict["librosa_S"]

		fig, ax = plt.subplots(nrows=2, sharex=True)

		ax[0].semilogy(times, rms[0], label='RMS Energy')
		ax[0].set(xticks=[])
		ax[0].label_outer()

		#Can be avoided?
		librosa.display.specshow(librosa.amplitude_to_db(S, ref=np.max),
								y_axis='log', x_axis='time', ax=ax[1])

		ax[1].plot(times, f0, color='green', linewidth=3)			
		ax[1].set(title='log Power spectrogram')

		''' #Avoided on-sets-we don't really need them for speech maybe for singing
		o_env = librosa.onset.onset_strength(y=y, sr=sr)
		times = librosa.times_like(o_env, sr=sr)
		onset_frames = librosa.onset.onset_detect(onset_envelope=o_env, sr=sr)

		ax[2].plot(times, o_env)
		ax[2].vlines(times[onset_frames], 0, o_env.max(), color='r', alpha=0.9,
				linestyle='--')

		'''
		fig.set_size_inches(12, 9)
		plt.savefig(output_filepath + '/rosaInfo.png', bbox_inches='tight')


	def set_handlers(self):

		@self.bot.message_handler(commands=['start', 'help'])
		def send_welcome(message):
			self.bot.reply_to(message, "Menu yet not implemented")


		@self.bot.message_handler(func=lambda message: True)
		def echo_all(message):
			t = threading.Timer(1.0, self.send_delayed_text, [message])
			t.start()


		@self.bot.message_handler(content_types=['voice'])
		def process_voice_message(message):

			path_user_logs = self._config["dir"] + '/' + str(message.chat.id)
			if not os.path.exists(path_user_logs):
				os.makedirs(path_user_logs)

			file_info = self.bot.get_file(message.voice.file_id)
			downloaded_file = self.bot.download_file(file_info.file_path)

			self.bot.reply_to(message, f"Запись обрабатывается. Момент: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

			t = threading.Timer(1.0, self.deplayed_recognition, [path_user_logs, message, downloaded_file])
			t.start()

			print("Audio saved")


	def extract_features(self, wav_file):
		seq_dict = {}

		#LIBROSA
		y, sr = librosa.load(wav_file)

		S, phase = librosa.magphase(librosa.stft(y)) #TODO can be avoided?
		rms = librosa.feature.rms(S=S)

		times = librosa.times_like(rms)

		f0, voiced_flag, voiced_probs = librosa.pyin(y,
			fmin=librosa.note_to_hz('C2'),
			fmax=librosa.note_to_hz('C7'))

		seq_dict["librosa_pitch"] = f0
		seq_dict["librosa_rms"] = rms
		seq_dict["librosa_times"] = times
		seq_dict["librosa_S"] = S #TODO can be avoided if not plots?

		#PRAAT

		snd = parselmouth.Sound(wav_file) 
		intensity = snd.to_intensity()

		f0min = 60
		f0max = 600

		pitch = call(snd, "To Pitch", 0.0, f0min, f0max)  #TODO make global in class
		pulses = call([snd, pitch], "To PointProcess (cc)") #TODO make global in class
		duration = call(snd, "Get total duration") #TODO make global in class 

		#TODO maybe formants in time? snd.to_formant_burg

		seq_dict["praat_pitch"] = pitch
		seq_dict["praat_intensity"] = intensity
		seq_dict["duration"] = duration

		seq_dict["praat_sound"] = snd
		seq_dict["praat_pulses"] = pulses


		#SURFBOARD

		from surfboard.sound import Waveform #SAME THING UPPER, IF WE TURN OFF ANY, WE DON"T HAVE TO INSTALL THEM
		sound = Waveform(path=wav_file, sample_rate=44100) #TODO опциоальный, отключать иногда

		swipe_contour = sound.f0_contour()
		intense_contour = sound.intensity()  #TODO SPREAD everywhere

		global_shimmers = sound.shimmers()
		global_jitters = sound.jitters()
		global_formants = sound.formants()
		global_hnr = sound.hnr()

		seq_dict["swipe_contour"] = swipe_contour
		seq_dict["intense_contour"] = intense_contour
		seq_dict["global_jitters"] = global_jitters
		seq_dict["global_shimmers"] = global_shimmers
		seq_dict["global_formants"] = global_formants
		seq_dict["global_hnr"] = global_hnr

		return seq_dict



	def local_recognition(self, spectrum_dir_path, record_file_path, alias_name):

		import datetime

		start_moment = datetime.datetime.now()

		id = self.request_recognition(record_file_path, alias_name)

		request_sent_moment = datetime.datetime.now()

		#1: export from ogg to PCM here

		if os.path.exists(spectrum_dir_path + '/pcm.wav'): #TODO rename spectrum_dir_path + осторожней тут
			os.remove(spectrum_dir_path +"/pcm.wav")

			command = f"ffmpeg -hide_banner -loglevel error -i {record_file_path} -ar 48000 -ac 2 -ab 192K -f wav {spectrum_dir_path}/pcm.wav" #Optional converting to wav #update SR
		_ = check_call(command.split())

		wav_file = f"{spectrum_dir_path}/pcm.wav"

		seq_dict = self.extract_features(wav_file)

		if self.skip_plots == False:
			self.save_images(seq_dict) #TODO возможно надо сохранять ещё директорию, но вначале возьмём из конфига

		images_saved_moment = datetime.datetime.now()

		req = self.check_server_recognition(id)
		full_string = json.dumps(req, ensure_ascii=False, indent=2)

		recognition_received_moment = datetime.datetime.now()

		json_report = self.make_json_report(req, seq_dict)

		full_report_generated = datetime.datetime.now()

		self.save_json_products(spectrum_dir_path, json_report, full_string)

		measure_time = True

		if measure_time:

			spent_on_send = request_sent_moment - start_moment
			spent_on_imaged = images_saved_moment - request_sent_moment
			spent_on_received = recognition_received_moment - images_saved_moment
			spent_on_report = full_report_generated - recognition_received_moment
			total_spent = full_report_generated - start_moment

			print("Spent on send: ", spent_on_send.seconds, "s ", spent_on_send.microseconds, " micro")
			print("Spent on imaged: ", spent_on_imaged.seconds, "s ", spent_on_imaged.microseconds, " micro")
			print("Spent on received: ", spent_on_received.seconds, "s ", spent_on_received.microseconds, " micro")
			print("Spent on report: ", spent_on_report.seconds, "s ", spent_on_report.microseconds, " micro")
			print("Total spent ", total_spent.seconds, "s ", total_spent.microseconds, " micro")

		print("Done!")


	def start_bot(self):

		self.set_handlers()

		print("Starting bot")
		self.bot.infinity_polling()
		print("Bot is finished")



r = ReportGenerator('key.json')
#r.start_bot()

r.local_recognition(r._config['dir'] , r._config['dir'] + '/local.ogg', "changen")