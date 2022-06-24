# -*- coding: utf-8 -*-
#acutally not:)

from cmath import pi
import os
import datetime

import telebot
import json

import numpy as np

import parselmouth
from parselmouth.praat import call

from subprocess import check_call 

import threading
import time
import requests

from cloud_storage import upload_file


class ReportGenerator:

	def __init__(self, config_name):

		with open(config_name, 'r') as file:
			self._config = json.load(file)

		self.bot = telebot.TeleBot(self._config["key"])

		self.use_cross_matrix = False

		self.de_personalization = False
		self.skip_plots = True
		self.include_sequences = True

		self.use_surf = False
		self.use_rosa = False

		self.every_word_praat_report = True
		self.calc_every_stat = True

		self.use_morph_analysis = False

		self.measure_time = False 
		self.verbose = False
		self.required_cleaning = True


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

		id = data['id']
		if self.verbose:
			print("Y id", id)

		return id



	def check_server_recognition(self, id):

		key = self._config["api-key"]
		header = {'Authorization': 'Api-Key {}'.format(key)}

		while True:

			time.sleep(7)

			GET = "https://operation.api.cloud.yandex.net/operations/{id}"
			req = requests.get(GET.format(id=id), headers=header)
			req = req.json()

			if req['done']: 
				break

			if self.verbose:
				print("Not ready")

			time.sleep(5) #Это число можно рассчитывать, но я ленив :)

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
				return 0 #Мода может выпадать если их несколько


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
				return 0 #Мода может выпадать если их несколько

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
				praat_dict[field_name] = float(field_value)

		return praat_dict



	def make_json_report(self, req, seq_dict): #REFACTORING - GOD функция TODO

		import datetime

		start_moment = datetime.datetime.now()

		#==========================================
		duration = seq_dict["duration"]
		intensity = seq_dict["praat_intensity"].values.T 
		pitch = seq_dict["praat_pitch"]

		if self.use_rosa:
			f0 = seq_dict["librosa_pitch"] 
			rms = seq_dict["librosa_rms"]
			f0_step = duration / len(f0)
			rms_step = duration / len(rms[0])
			rms = rms.reshape(rms.shape[0] * rms.shape[1])

		pitch_step = duration / len(pitch)
		intensity_step = duration / len(intensity)

		pitch = pitch.selected_array['frequency']
		intensity = intensity.reshape(intensity.shape[0] * intensity.shape[1])
		
		pitch = np.array(pitch) 
		intensity = np.array(intensity)

		if self.use_rosa:
			rms = np.array(rms)
			f0 = np.array(f0)

		if self.use_surf:

			swipe_pitch = np.array(seq_dict["swipe_pitch"][0])
			swipe_step = duration / len(swipe_pitch)

			surf_intensity = np.array(seq_dict["surf_intensity"][0])
			surf_intensity = surf_intensity.astype(float)

			surf_intens_step = duration / len(surf_intensity)


		reshape_sequences_moment =  datetime.datetime.now()
		surf_moment =  datetime.datetime.now()

		snd = seq_dict["praat_sound"]

		pitch_for_praat = seq_dict["praat_pitch"]
		pulses = seq_dict["praat_pulses"]
		f0min = 60 #TODO Перенести в члены класса
		f0max = 600

		full_report = call([snd, pitch_for_praat, pulses], "Voice report", 0, duration, f0min, f0max,
							1.3, 1.6, 0.03, 0.45)  #TODO Перенести в члены класса

		praat_moment =  datetime.datetime.now()
		#==========================================Prepare basic information sequences==========================================

		events = []
		prev_word_end = 0.0
		chunks = []
		words_freq = {}

		tokens = {}
		tokens_count = 0

		full_stats = {"praat_pitch": self.get_full_statistics(pitch),
					  "intensity":self.get_full_statistics(intensity)}

		if self.use_rosa:
			full_stats["rms"] = self.get_full_statistics(rms)
			full_stats["pyin_pitch"] = self.get_full_statistics(f0)

		if self.use_surf:
			full_stats["surf intensity"] = self.get_full_statistics(surf_intensity)
			full_stats["swipe_pitch"] = self.get_full_statistics(swipe_pitch)
		

		full_text = ""
		all_starts = []
		all_ends = []
		total_words = 0
		chunkId = 0


		for chunk in req['response']['chunks']:

			altId = 0
			for alt in chunk['alternatives']: #We don't handle silence right yet in case for alts ATTENTION

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

					

					if self.use_surf:
						pause_RMS = self.make_sequence_cut(rms_step, silence_start, silence_end, rms) 
						
					pause_intens = self.make_sequence_cut(intensity_step, silence_start, silence_end, intensity)

					if self.use_surf:
						pause_surf_intens = self.make_sequence_cut(surf_intens_step, silence_start, silence_end, surf_intensity)

					if self.use_rosa:
						pause_RMS = self.get_full_statistics(pause_RMS)
					
					pause_intens = self.get_full_statistics(pause_intens)

					if self.use_surf:
						pause_surf_intens = self.get_full_statistics(pause_surf_intens)

					silence_report = ""#От него отказались пока что
				
					single_pause = {"type":"pause", "startTime": silence_start, "endTime": silence_end}

					single_pause["Intensity"] = pause_intens
					single_pause["info"] = silence_report

					if self.use_rosa:		
						single_pause["RMS"] = pause_RMS 		

					if self.use_surf:
						single_pause["Surf Intencity"] = pause_surf_intens
						
					events.append(single_pause)

					

					prev_word_end = end

					if self.include_sequences:

						if self.use_rosa:	
							f0_cut = self.make_sequence_cut(f0_step, start, end, f0) 
							rms_cut = self.make_sequence_cut(rms_step, start, end, rms) 

						pitch_cut = self.make_sequence_cut(pitch_step, start, end, pitch)  
						intens_cut = self.make_sequence_cut(intensity_step, start, end, intensity) 
						
						if self.use_surf:
							swipe_cut = self.make_sequence_cut(swipe_step, start, end, swipe_pitch) 
							surf_intens_cut = self.make_sequence_cut(surf_intens_step, start, end, surf_intensity)



					if self.calc_every_stat:

						statistics_records = {"praat_pitch": self.get_full_statistics(pitch_cut),
											"intensity":self.get_full_statistics(intens_cut)}

						if self.use_surf:
							statistics_records["swipe_pitch"] = self.get_full_statistics(swipe_cut)
							statistics_records["surf intensity"] = self.get_full_statistics(surf_intens_cut)

						if self.use_rosa:	
							statistics_records["rms"] = self.get_full_statistics(rms_cut)
							statistics_records["pyin_pitch"] = self.get_full_statistics(f0_cut)


					if self.every_word_praat_report:
						report_string = call([snd, pitch_for_praat, pulses], "Voice report", start, end, f0min, f0max,
								1.3, 1.6, 0.03, 0.45)

					
					morph_analysis = []

					if self.use_morph_analysis or duration < 20.0:
						morph_analysis = self.make_morph_analysis(word['word'])

					token_id = 0

					current_word = word["word"]

					if current_word not in tokens:
						tokens[current_word] = tokens_count + 1
						tokens_count += 1
						token_id = tokens_count
					else:
						token_id = tokens[current_word]

					if self.de_personalization: 
						current_word = '-'

					singleWord =  {"type":"word",  "chunkId" : chunkId, "altId": altId, "word": current_word, 
					"startTime": start, "endTime": end, 
					"confidence": word['confidence'], 
					"praat_pitch": list(pitch_cut),
					"praat_intensity": list(intens_cut)
					,"morph" : morph_analysis
					,"token_id" : token_id
					,"word_idx" : total_words
					,"letters_speed" : letters_speed
					,"letters_freq" : 1.0 / letters_speed
					} #channel tag left away TODO attention
					

					if self.calc_every_stat:
						singleWord["stats"] = statistics_records 

					if self.every_word_praat_report:
						singleWord["info"] = self.parse_praat_info(report_string)

					if self.use_rosa:	 
						singleWord["pyin_pitch"] = list(f0_cut)
						singleWord["RMS"] = list(rms_cut)


					if self.use_surf:
						singleWord["swipe_pitch"] = list(swipe_cut)
						singleWord["surt_inten"] = list(surf_intens_cut)


					total_words += 1

					events.append(singleWord)

					if token_id in words_freq:
						words_freq[token_id] += 1
					else:
						words_freq[token_id] = 1

				
				pitch_cut = self.make_sequence_cut(pitch_step, first_start, prev_word_end, pitch)
				intens_cut = self.make_sequence_cut(intensity_step, first_start, prev_word_end, intensity)
				
				
				if self.calc_every_stat:

					statistics_records = {"praat_pitch": self.get_full_statistics(pitch_cut),
										"intensity":self.get_full_statistics(intens_cut),}

					if self.use_rosa:
						f0_cut = self.make_sequence_cut(f0_step, first_start, prev_word_end, f0)
						rms_cut = self.make_sequence_cut(rms_step, first_start, prev_word_end, rms)
						statistics_records["pyin_pitch"]  =  self.get_full_statistics(f0_cut)
						statistics_records["rms"] = self.get_full_statistics(rms_cut)


					if self.use_surf:
						swipe_cut = self.make_sequence_cut(swipe_step, first_start, prev_word_end, swipe_pitch)
						surf_intens_cut = self.make_sequence_cut(surf_intens_step, first_start, prev_word_end, surf_intensity)
						statistics_records["surf intensity"]  =  self.get_full_statistics(surf_intens_cut)
						statistics_records["swipe_pitch"] = self.get_full_statistics(swipe_cut)


				chunk_text = alt["text"]
				if self.de_personalization:
						chunk_text = '-'

				chunk_report = call([snd, pitch_for_praat, pulses], "Voice report", first_start, prev_word_end, f0min, f0max,
							1.3, 1.6, 0.03, 0.45) #TODO Перенести в члены класса

				single_chunk = {"chunkId": chunkId, "altId": altId, 
								"start" : first_start, "end": prev_word_end, "words_speed": (prev_word_end - first_start) / len(alt["words"]),
								"text": chunk_text, "info": self.parse_praat_info(chunk_report)}

				if self.calc_every_stat:
					single_chunk["stats"] = statistics_records 
				
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

					cross_element = {"info": self.parse_praat_info(cross_report), "start" : cross_start,
									"end": cross_end, "first_word_idx": i, "last_word_idx": j}

					cross_stats.append(cross_element)

		cross_matrix__moment = datetime.datetime.now()

		praat_dict = self.parse_praat_info(full_report)

		steps = {"praat_putch_step": pitch_step,
				 "intensity_step": intensity_step }

		if self.use_rosa:
			steps[ "librosa_pitch_step"] = f0_step
			steps[ "rms_step"] = rms_step

		if self.use_surf:
			steps[ "swipe_step"] = swipe_step
			steps[ "surf_intens_step"] = surf_intens_step

		root_element = {"events": events, "full_stats": full_stats, "chunks": chunks,
						"words_freq": words_freq, "full_text": full_text, "tokens": tokens,
						"info": praat_dict,
						"cross_stats": cross_stats, "duration": duration, "steps_sizes": steps}

		if self.use_surf:
			root_element["jitters"] = seq_dict["global_jitters"]
			root_element["shimmers"] = seq_dict["global_shimmers"]
			root_element["formants"] = seq_dict["global_formants"]
			root_element["HNR"] = seq_dict["global_hnr"]

		json_report = json.dumps(root_element, ensure_ascii=False) 
		json_report = json.dumps(json.loads(json_report, parse_float=lambda x: round(float(x), 9)), indent = 4)

		#TODO если установить , ensure_ascii=False тогда репорт human readable - иначе проблемы на стороне QT

		if self.measure_time == True:

			full_report_generated = datetime.datetime.now()

			total_spent = full_report_generated - start_moment
			print("Total on report: ", total_spent.seconds, "s ", total_spent.microseconds / 1000.0, " ms")

			rehape_spent = reshape_sequences_moment - start_moment
			surf_spent = surf_moment - reshape_sequences_moment
			praat_spemt = praat_moment - surf_moment
			all_chunks_spent = all_chnunks_and_events_moment - praat_moment
			cross_spent = cross_matrix__moment - all_chnunks_and_events_moment
			dump_spent = full_report_generated - cross_matrix__moment

			print("Reshape sequences ", rehape_spent.seconds, "s ", rehape_spent.microseconds/ 1000.0, " ms")
			print("Surf ", surf_spent.seconds, "s ", surf_spent.microseconds/ 1000.0, " ms")
			print("Praat ", praat_spemt.seconds, "s ", praat_spemt.microseconds/ 1000.0, " ms")
			print("All chunks", all_chunks_spent.seconds, "s ", all_chunks_spent.microseconds/ 1000.0, " ms")
			print("Cross matrix ", cross_spent.seconds, "s ", cross_spent.microseconds/ 1000.0, " ms")
			print("Dump spent, ", dump_spent.seconds, "s ", dump_spent.microseconds/ 1000.0, " ms")
			
		return json_report



	def save_downloaded_and_name(self, path_user_logs, message, downloaded_file):

		record_file_path = path_user_logs + '/record_' + str(message.id) + '.ogg'

		if self.verbose:
			print(record_file_path, " <- dir path")

		with open(os.path.join(record_file_path), 'wb') as new_file:
			new_file.write(downloaded_file)

		alias_name = "a" + str(message.chat.id)  + "b" + str(message.id) + ".ogg"

		return record_file_path, alias_name


	def save_images_info(self, path_user_logs, message, voice_report): #TODO разделить бота и генератор репортов

		rosaInfo = open(path_user_logs + '/rosaInfo.png', 'rb')
		self.bot.send_photo(message.chat.id, rosaInfo)

		praatInfo = open(path_user_logs + '/praatInfo.png', 'rb')
		self.bot.send_photo(message.chat.id, praatInfo)

		self.bot.reply_to(message, voice_report)


	def save_json_products(self, path_user_logs, json_report, full_string, message_id):

		with open(path_user_logs + '/full_report_' + message_id + '.json', 'w') as outfile:
			outfile.write(json_report)

		if self.verbose == True:
			with open(path_user_logs + '/stt_' + message_id + '.json', 'w') as outfile:
				outfile.write(full_string)


	def merge_text_from_request(self, req): 

		text_lines = []
		message_text = ""

		if self.verbose == True:
			print("Text chunks:")

		for chunk in req['response']['chunks']:

			if self.verbose == True:
				print(chunk['alternatives'][0]['text'])

			text_lines.append(chunk['alternatives'][0]['text']) #Внимание не собираются alternatives ATTENTION
			message_text += chunk['alternatives'][0]['text'] + "\n"

		return message_text


	def send_message_and_reports(self, path_user_logs, message, message_text):

		self.bot.reply_to(message, message_text)

		if self.verbose:
			doc = open(path_user_logs + '/stt_' + str(message.id)  + '.json', 'rb')
			self.bot.send_document(message.chat.id, doc)

		doc = open(path_user_logs + '/full_report_'  + str(message.id)  + '.json', 'rb')
		self.bot.send_document(message.chat.id, doc)



	def deplayed_audio_document(self, path_user_logs, message, downloaded_file):

			print("audio documents blocked for a while") #TODO unlock later
			return

			if message.document != None:
				record_file_path = path_user_logs + '/audio_' + str(message.id) +  "_" + message.document.file_name
			else:
				record_file_path = path_user_logs + '/audio_' + str(message.id) +  "_" + message.audio.file_name

			print(record_file_path, " <- document\audio file path")

			with open(os.path.join(record_file_path), 'wb') as new_file:
				new_file.write(downloaded_file)

			new_file = path_user_logs + "/converted.ogg"

			self.convert_wav_to_ogg(record_file_path, new_file)

			alias = "doc" + str(message.id) + "x" + str(message.chat.id)

			id = r.request_recognition(new_file, alias)

			wav_file = self.convert_ogg_to_wav(path_user_logs, new_file, str(message.id)) 
			seq_dict = self.extract_features(wav_file)

			req = self.check_server_recognition(id)

			full_string = json.dumps(req, ensure_ascii=False, indent=2)
			json_report = self.make_json_report(req, seq_dict)

			self.save_json_products(path_user_logs, json_report, full_string, str(message.id))

			message_text = self.merge_text_from_request(req)
			self.send_message_and_reports(path_user_logs, message, message_text)
			



	def deplayed_recognition(self, path_user_logs, message, downloaded_file):

		record_file_path, alias_name = self.save_downloaded_and_name(path_user_logs, message, downloaded_file)

		id = self.request_recognition(record_file_path, alias_name)

		wav_file = self.convert_ogg_to_wav(path_user_logs, record_file_path, str(message.id))

		seq_dict = self.extract_features(wav_file)

		if self.skip_plots == False:
			self.save_images(seq_dict)

		req = self.check_server_recognition(id)

		full_string = json.dumps(req, ensure_ascii=False, indent=2)
		json_report = self.make_json_report(req, seq_dict)

		self.save_json_products(path_user_logs, json_report, full_string, str(message.id))

		message_text = self.merge_text_from_request(req)
			
		self.send_message_and_reports(path_user_logs, message, message_text)

		if self.required_cleaning: #TODO later move into another function to share with docments send
			
			if os.path.exists(wav_file):
				os.remove(wav_file)

			if os.path.exists(record_file_path):
				os.remove(record_file_path) 

			full_report_name = path_user_logs + '/full_report_' + str(message.id) + '.json'

			if os.path.exists(full_report_name):
				os.remove(full_report_name) 

		#TODO remove alias

		#commands_response = self.detect_commands(message_text) #blocked:)
		#if commands_response != '':
		#	self.bot.reply_to(message, commands_response)
			


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

		request_string = "завершить задачу"
		finish_aim_pos = text.find(request_string)
		if finish_aim_pos != -1:
			return "Завершена задача с именем: " + text[finish_aim_pos + len(request_string):]

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

		self.bot.reply_to(message, 'Озвучивание:')
		voice = open("123.ogg", 'rb')
		self.bot.send_voice(message.chat.id, voice)

		print("Текст озвучен")


	def draw_intensity_praat(self, intensity): #Отделить всю отрисовку в отдельный класс

		import matplotlib.pyplot as plt

		plt.plot(intensity.xs(), intensity.values.T, linewidth=3, color='g')
		plt.plot(intensity.xs(), intensity.values.T, linewidth=1)
		plt.grid(False)
		plt.ylim(0, 100)
		plt.ylabel("intensity [dB]")



	def draw_pitch_praat(self, pitch):

		import matplotlib.pyplot as plt

		pitch_values = pitch.selected_array['frequency']
		pitch_values[pitch_values==0] = np.nan
		plt.plot(pitch.xs(), pitch_values, 'o', markersize=5, color='r')
		plt.plot(pitch.xs(), pitch_values, 'o', markersize=2, color='r')
		plt.grid(False)
		plt.ylim(0, pitch.ceiling)
		plt.ylabel("fundamental frequency [Hz]")



	def plot_pitches(self, seq_dict, output_filepath):

		import matplotlib.pyplot as plt
		import librosa

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



	def save_images(self, seq_dict):	

		if self.use_rosa:
			self.plot_librosa(seq_dict, self._config["dir"])
			
		self.plot_praat(seq_dict, self._config["dir"])

		if self.use_rosa:
			self.plot_pitches(seq_dict, self._config["dir"])



	def plot_praat(self, seq_dict, output_filepath): 

		import matplotlib.pyplot as plt
		import seaborn as sns 

		snd = seq_dict["praat_sound"]
		intensity = seq_dict["praat_intensity"]
		pitch = seq_dict["praat_pitch"] 

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

		import matplotlib.pyplot as plt

		fig, ax = plt.subplots(nrows=2, sharex=True)

		ax[0].semilogy(times, rms[0], label='RMS Energy')
		ax[0].set(xticks=[])
		ax[0].label_outer()

		#Can be avoided?
		import librosa
		import librosa.display

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
			#t = threading.Timer(1.0, self.send_delayed_text, [message])
			#t.start()
			self.bot.reply_to(message, 'На текст больше не отвечаю :P')
			print("Input message blocked: ", message.text)


		@self.bot.message_handler(content_types=['document'])
		def process_docs_audio(message):

			path_user_logs = self._config["dir"] + '/' + str(message.chat.id)
			if not os.path.exists(path_user_logs):
				os.makedirs(path_user_logs)

			file_info = self.bot.get_file(message.document.file_id)
			downloaded_file = self.bot.download_file(file_info.file_path)

			file_extenstion = message.document.file_name[-3:]

			print("Doc file-name", message.document.file_name, "ext", file_extenstion)

			if file_extenstion != "mp3" and file_extenstion != "wav":
				self.bot.reply_to(message, "Допускаются только .wav или .mp3")
			else:
				self.bot.reply_to(message, f"Аудио обрабатывается. Момент: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

				t = threading.Timer(1.0, self.deplayed_audio_document, [path_user_logs, message, downloaded_file])
				t.start()

				print("Document saved")


		@self.bot.message_handler(content_types=['audio'])
		def process_docs_audio(message):

			path_user_logs = self._config["dir"] + '/' + str(message.chat.id)
			if not os.path.exists(path_user_logs):
				os.makedirs(path_user_logs)

			file_info = self.bot.get_file(message.audio.file_id)
			downloaded_file = self.bot.download_file(file_info.file_path)

			file_extenstion = message.audio.file_name[-3:]

			print("Doc file-name", message.audio.file_name, "ext", file_extenstion)

			if file_extenstion != "mp3" and file_extenstion != "wav":
				self.bot.reply_to(message, "Допускаются только .wav или .mp3")
			else:
				self.bot.reply_to(message, f"Аудио обрабатывается. Момент: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

				t = threading.Timer(1.0, self.deplayed_audio_document, [path_user_logs, message, downloaded_file])
				t.start()

				print("Document saved")



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

			if self.verbose:
				print("Audio saved")


	def extract_features(self, wav_file):

		seq_dict = {}

		start_moment =  datetime.datetime.now() #Позже убрать или закамуфлировать
		librosa_loaded_moment =  datetime.datetime.now()
		librosa_stft_moment =  datetime.datetime.now()

		if self.use_rosa:

			import librosa

			y, sr = librosa.load(wav_file)
			S, phase = librosa.magphase(librosa.stft(y))  #can be avoided, if no plots!
			
			rms = librosa.feature.rms(S=S)
			times = librosa.times_like(rms)

			f0 = librosa.yin(y, fmin=librosa.note_to_hz('C2'), fmax=librosa.note_to_hz('C7'))

			#f0, voiced_flag, voiced_probs = librosa.pyin(y, #Работает дольше
			#	fmin=librosa.note_to_hz('C2'),
			#	fmax=librosa.note_to_hz('C7'))

			seq_dict["librosa_pitch"] = f0
			seq_dict["librosa_rms"] = rms
			seq_dict["librosa_times"] = times
			seq_dict["librosa_S"] = S #TODO can be avoided if not plots!

		librosa_done_moment = datetime.datetime.now()

		snd = parselmouth.Sound(wav_file) 
		
		f0min = 60 #КАК и выше по стеку вызова!
		f0max = 600

		pitch = call(snd, "To Pitch", 0.0, f0min, f0max)  
		intensity = snd.to_intensity()

		pulses = call([snd, pitch], "To PointProcess (cc)") 
		duration = call(snd, "Get total duration")

		#formants = snd.to_formant_burg()
		#print("Formants ", len(formants), " ", formants) # nFormants
		#TOOO BIG :(

		seq_dict["praat_pitch"] = pitch
		seq_dict["praat_intensity"] = intensity

		seq_dict["duration"] = duration

		seq_dict["praat_sound"] = snd
		seq_dict["praat_pulses"] = pulses

		praat_done_moment = datetime.datetime.now()

		if self.use_surf == True:

			from surfboard.sound import Waveform 
			sound = Waveform(path=wav_file, sample_rate=44100) 

			swipe_pitch = sound.f0_contour()
			surf_intensity = sound.intensity() 

			#formants_sequence = sound.formants_slidingwindow()
			#print("Formants sequence: ", len(formants_sequence), " and signle ",
			#	len(formants_sequence[0]))

			global_shimmers = sound.shimmers()
			global_jitters = sound.jitters()
			global_formants = sound.formants()
			global_hnr = sound.hnr()

			seq_dict["swipe_pitch"] = swipe_pitch
			seq_dict["surf_intensity"] = surf_intensity

			seq_dict["global_jitters"] = global_jitters
			seq_dict["global_shimmers"] = global_shimmers
			seq_dict["global_formants"] = global_formants
			seq_dict["global_hnr"] = global_hnr

		surf_done_moment = datetime.datetime.now()

		if self.measure_time:

			librosa_load_spent = librosa_loaded_moment - start_moment
			librosa_stft_spent = librosa_stft_moment - librosa_loaded_moment
			librosa_rest_spent = librosa_done_moment - librosa_loaded_moment
			praat_spent = praat_done_moment - librosa_done_moment
			surf_spent = surf_done_moment - praat_done_moment
			total_spent = surf_done_moment - start_moment

			print("Total ", total_spent.seconds, "s ", total_spent.microseconds / 1000.0, " ms")

			print("Rosa load ", librosa_load_spent.seconds, "s ", librosa_load_spent.microseconds / 1000.0, " ms")
			print("Rosa stft ", librosa_stft_spent.seconds, "s ", librosa_stft_spent.microseconds / 1000.0, " ms")
			print("Rosa rest ", librosa_rest_spent.seconds, "s ", librosa_rest_spent.microseconds / 1000.0, " ms")

			print("Praat ", praat_spent.seconds, "s ", praat_spent.microseconds / 1000.0, " ms")
			print("Surf ", surf_spent.seconds, "s ", surf_spent.microseconds / 1000.0, " ms")

		return seq_dict


	def convert_ogg_to_wav(self, path_user_logs, record_file_path, message_id):

		wav_file = f"{path_user_logs}/pcm_{message_id}.wav"

		if os.path.exists(wav_file):
			os.remove(wav_file)

		command = f"ffmpeg -hide_banner -loglevel error -i {record_file_path} -ar 48000 -ac 2 -ab 192K -f wav {wav_file}"
		_ = check_call(command.split())

		return wav_file


	def convert_wav_to_ogg(self, input_file, output_file):

		if os.path.exists(output_file): 
			os.remove(output_file)

		command = f"ffmpeg -hide_banner -loglevel error -i {input_file} -c:a libopus {output_file}" 
		_ = check_call(command.split())



	def local_recognition(self, path_user_logs, record_file_path, alias_name):

		import datetime

		start_moment = datetime.datetime.now()

		id = self.request_recognition(record_file_path, alias_name)

		request_sent_moment = datetime.datetime.now()

		wav_file = self.convert_ogg_to_wav(path_user_logs, record_file_path, "local_id")

		seq_dict = self.extract_features(wav_file)

		if self.skip_plots == False:
			self.save_images(seq_dict) #возможно надо сохранять ещё директорию, но вначале возьмём из конфига - abadoned

		images_saved_moment = datetime.datetime.now()

		req = self.check_server_recognition(id)
		full_string = json.dumps(req, ensure_ascii=False, indent=2)

		recognition_received_moment = datetime.datetime.now()

		json_report = self.make_json_report(req, seq_dict)

		full_report_generated = datetime.datetime.now()

		self.save_json_products(path_user_logs, json_report, full_string, "local_id")


		if self.measure_time:

			spent_on_send = request_sent_moment - start_moment
			spent_on_imaged = images_saved_moment - request_sent_moment
			spent_on_received = recognition_received_moment - images_saved_moment
			spent_on_report = full_report_generated - recognition_received_moment
			total_spent = full_report_generated - start_moment

			print("Spent on send: ", spent_on_send.seconds, "s ", spent_on_send.microseconds / 1000.0, " ms")
			print("Spent on imaged: ", spent_on_imaged.seconds, "s ", spent_on_imaged.microseconds / 1000.0, " ms")
			print("Spent on received: ", spent_on_received.seconds, "s ", spent_on_received.microseconds / 1000.0, " ms")
			print("Spent on report: ", spent_on_report.seconds, "s ", spent_on_report.microseconds / 1000.0, " ms")
			print("Total spent ", total_spent.seconds, "s ", total_spent.microseconds / 1000.0, " ms")

		print("Done!")


	def start_bot(self):

		self.set_handlers()

		print("Starting bot")
		self.bot.infinity_polling()
		print("Bot is finished")




def async_load_dir(r):

	files_dict = {}

	i = 0

	for filename in os.listdir(r._config['temp']):

		f = os.path.join(r._config['temp'], filename)

		if os.path.isfile(f):

			print('FILE', f)

			i += 1
			filename_no_ext =  "idfile" + str(i) 

			new_file = r._config['out'] + '/' + filename + '_out.ogg'

			r.convert_wav_to_ogg(f, new_file) 

			id = r.request_recognition(new_file, filename_no_ext) 

			files_dict[filename] = id
			continue

			
	id_texts = json.dumps(files_dict, indent = 4, ensure_ascii=False) 
	with open(r._config['out'] + '/ids.json', 'w') as outfile:
			outfile.write(id_texts)




def async_extract(r):

	files_dict = {}

	with open(r._config['out'] + '/ids.json', 'r') as file:
		files_dict = json.load(file)

	for filename in os.listdir(r._config['temp']):

		f = os.path.join(r._config['temp'], filename) 

		if os.path.isfile(f):

			print('FILE: ', f)
			new_file = r._config['out'] + '/' + filename + '_out.ogg'

			id = files_dict[filename] 
			
			seq_dict = r.extract_features(f) 

			req = r.check_server_recognition(id)

			if 'chunks' not in req['response']:
				continue

			full_text = json.dumps(req, ensure_ascii=False, indent=2)

			reports_pre_name = r._config['out'] + '/' + filename

			with open(reports_pre_name +  '_full_text.json', 'w') as outfile:
				outfile.write(full_text)

			full_report = r.make_json_report(req, seq_dict)
			
			with open(reports_pre_name +  '_full_report.json', 'w') as outfile:
				outfile.write(full_report)

			print("File done")



def rename_files(r):

	for filename in os.listdir(r._config['temp']):

		f = os.path.join(r._config['temp'], filename) 

		if os.path.isfile(f):

			if f.find(" ") != -1:
				f_replaces = f.replace(" ", "_")
				os.rename(f, f_replaces)

				print("Renaming ", f, f_replaces)



def reports_to_csv(r):


	import csv
	with open(r._config['to-csv'] + '/full.csv', 'w', newline='') as csvfile:
		table = csv.writer(csvfile, delimiter=',')

		table.writerow(["name", "text", "duration", "Median pitch", "Mean pitch", "Number of pulses", "Fraction of locally unvoiced frames", "Jitter (local)", "Shimmer (local)", "Mean harmonics-to-noise ratio", "Pitch SD", "Intensity SD"])

		
		for filename in os.listdir(r._config['to-csv']):

			f = os.path.join(r._config['to-csv'], filename) 
			
			if os.path.isfile(f) and f.find("full_report") != -1:

				print("FILE ", f, " opening")

				with open(f, 'r') as file:
					report_dict = json.load(file)

				report_dict["full_stats"]["praat_pitch"]["SD"]
				report_dict["full_stats"]["intensity"]["SD"]
				
				if "duration" not in report_dict["info"]:
					continue

				f_name = filename.replace(",","_")

				table.writerow([f_name, report_dict["full_text"], report_dict["info"]["duration"], report_dict["info"]["Median pitch"], report_dict["info"]["Mean pitch"], report_dict["info"]["Number of pulses"],
				report_dict["info"]["Fraction of locally unvoiced frames"], report_dict["info"]["Jitter (local)"], report_dict["info"]["Shimmer (local)"], report_dict["info"]["Mean harmonics-to-noise ratio"],
				report_dict["full_stats"]["praat_pitch"]["SD"], report_dict["full_stats"]["intensity"]["SD"]])

				print("FILE ", f_name, " parsed")


#rename_files(r)
#async_load_dir(r)
#async_extract(r) #WHY long runs out of memory?
#reports_to_csv(r)\
#print("CSV DONE!")

print("Waiting for wifi")

#time.sleep(10) # Для Raspbery Pi установить связь с Wifi

r = ReportGenerator("key.json")
r.start_bot()

#r.local_recognition(r._config['dir'] , r._config['dir'] + '/local_2.ogg', "changen7")
#r.extract_features(r._config["dir"] + "/pcm.wav")



