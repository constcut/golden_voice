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


#TODO 1: to class
#TODO 2: поиск листа булевых элементов карточки
# поиск цифровых, поиск перечислительных
#
#заполнение цифровых, цифрой, которая идёт после ключевого слова, до следующего слова
#
#поиск 

with open('key.json', 'r') as file:
    config = json.load(file)

bot = telebot.TeleBot(config["key"])


def request_recognition(record_file_path, alias_name):
	
	upload_file(record_file_path,  alias_name)

	filelink = 'https://storage.yandexcloud.net/' + config["bucket"]  + '/' + alias_name #TODO или даже хранить алиас без расширения

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

	key = config["api-key"]
	header = {'Authorization': 'Api-Key {}'.format(key)}

	req = requests.post(POST, headers=header, json=body)
	data = req.json()
	print(data)

	id = data['id']
	return id



def check_server_recognition(id):

	key = config["api-key"]
	header = {'Authorization': 'Api-Key {}'.format(key)}

	while True:

		GET = "https://operation.api.cloud.yandex.net/operations/{id}"
		req = requests.get(GET.format(id=id), headers=header)
		req = req.json()

		if req['done']: break
		print("Not ready")

		time.sleep(10) #TODO рассчитывать от длины изначального аудио

	return req


def make_cut(step_size, start, end, sequence):
	
	cut = []
	idx_start = int(start / step_size)
	idx_end = int(end / step_size)

	for i in range(idx_start, idx_end + 1):
		cut.append(sequence[i])

	return cut



def stats(sequence, type):

	import statistics
	import math

	sub_sequence = [x for x in sequence if (math.isnan(x) == False and x != 0)]

	if len(sub_sequence) == 0:
		return 0.0

	if type == "mean":
		return statistics.mean(sub_sequence)

	if type == "mode":
		return statistics.mode(sub_sequence)

	if type == "median":
		return statistics.median(sub_sequence)

	if type == "min":
		return min(sub_sequence)
	
	if type == "max":
		return max(sub_sequence)

	if type == "stddev":
		return statistics.stdev(sub_sequence)

	print("WARNING: wrong statistic value: ", type)
	return 0.0



def get_full_stats(sequence):

	if len(sequence) <= 2:
		return {"error": "too small sequence"}

	full_stats = {"mean": stats(sequence, type="mean"),
			 "mode": stats(sequence, type="mode"),
			 "median": stats(sequence, type="median"),
			 "min": stats(sequence, type="min"),
			 "max": stats(sequence, type="max"),
			 "SD" : stats(sequence, type="stddev")}

	return full_stats
			 


def make_json_report(req, f0, rms, pitch, intensity, duration, wav_file):

	intensity = intensity.values.T
	
	f0_step = duration / len(f0)
	rms_step = duration / len(rms[0])
	pitch_step = duration / len(pitch)
	intensity_step = duration / len(intensity)

	pitch = pitch.selected_array['frequency']

	intensity = intensity.reshape(intensity.shape[0] * intensity.shape[1])
	rms = rms.reshape(rms.shape[0] * rms.shape[1])

	events = []
	prev_word_end = 0.0
	chunks = []
	words_freq = {}

	de_personalization = False #TODO to config

	tokens = {}
	tokens_count = 0

	#https://github.com/novoic/surfboard
	#from surfboard.sound import Waveform
	#import numpy as np
	#sound = Waveform(path=wav_file, sample_rate=44100)

	#f0_contour = sound.f0_contour()
	#shimmers = sound.shimmers()
	#jitters = sound.jitters()
	#formants = sound.formants()

	#print(len(f0_contour), len(shimmers), len(jitters), len(formants), ' ! All types of length')

	#TODO full stats for a file
	#!!!!!!!!!!!!!!!!!! TODO TODO TODO 

	full_stats = {"f0":get_full_stats(f0), "pitch": get_full_stats(pitch),
					"rms":get_full_stats(rms), "intensity":get_full_stats(intensity)}

	full_text = ""

	chunkId = 0
	for chunk in req['response']['chunks']:

		altId = 0
		for alt in chunk['alternatives']: #We don't handle silence right yet in case for alts

			first_start = -1.0

			for word in alt['words']:

				start = float(word['startTime'][:-1])
				end = float(word['endTime'][:-1])

				if first_start == -1.0:
					first_start = start

				silence_start = prev_word_end
				silence_end = start

				pause_RMS = make_cut(rms_step, silence_start, silence_end, rms)
				#сжимать данные
				pause_intens = make_cut(intensity_step, silence_start, silence_end, intensity)
				#TODO only stats
				
				pause_RMS = get_full_stats(pause_RMS)
				pause_intens = get_full_stats(pause_intens)

				silence_report = ""

				single_pause = {"type":"pause", "startTime": silence_start, "endTime": silence_end, 
								"RMS": pause_RMS, "Intensity": pause_intens, "info": silence_report}; #, "dB": list(pause_intens)

				
				events.append(single_pause)

				prev_word_end = end

				f0_cut = make_cut(f0_step, start, end, f0)
				pitch_cut = make_cut(pitch_step, start, end, pitch)
				intens_cut = make_cut(intensity_step, start, end, intensity)
				rms_cut = make_cut(rms_step, start, end, rms)

				statistics_records = {"f0":get_full_stats(f0_cut), "pitch": get_full_stats(pitch_cut),
					"rms":get_full_stats(rms_cut), "intensity":get_full_stats(intens_cut)}

				report_string = "" #call([snd, pitch_full, pulses_full], "Voice report", start, end, 60, 600, 1.3, 1.6, 0.03, 0.45)

				#TODO to sub function
				import pymorphy2
				morph = pymorphy2.MorphAnalyzer()
				p = morph.parse(word['word'])[0]
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

				token_id = 0

				if current_word not in tokens:
					tokens[current_word] = tokens_count + 1
					tokens_count += 1
					token_id = tokens_count
				else:
					token_id = tokens[current_word]

				current_word = word["word"]
				if de_personalization:
					current_word = '-'

				singleWord =  {"type":"word",  "chunkId" : chunkId, "altId": altId, "word": current_word, 
				"startTime": start, "endTime": end, 
				"confidence": word['confidence'], 
				"pitch_yin": list(f0_cut), "RMS": list(rms_cut), "pitch_praat": list(pitch_cut) 
				#,"dB": list(intens_cut)
				,"stats" : statistics_records,  "info": report_string
				,"morph" : morph_analysis
				,"token_id" : token_id
				} #channel tag left away

				events.append(singleWord)

				if token_id in words_freq: #TODO rewrite using token_id, so we got freq event there are no words
					words_freq[token_id] += 1
				else:
					words_freq[token_id] = 1

			#FILL chunk

			f0_cut = make_cut(f0_step, first_start, prev_word_end, f0)
			pitch_cut = make_cut(pitch_step, first_start, prev_word_end, pitch)
			intens_cut = make_cut(intensity_step, first_start, prev_word_end, intensity)
			rms_cut = make_cut(rms_step, first_start, prev_word_end, rms)

			statistics_records = {"f0":get_full_stats(f0_cut), "pitch": get_full_stats(pitch_cut),
				"rms":get_full_stats(rms_cut), "intensity":get_full_stats(intens_cut)}

			chunk_text = alt["text"]
			if de_personalization:
					chunk_text = '-'

			single_chunk = {"chunkId": chunkId, "altId": altId, "stats": statistics_records,
							"text": chunk_text}
			
			full_text += chunk_text + ". "

			chunks.append(single_chunk)

			altId += 1

		chunkId += 1

	if de_personalization == True:
		tokens = {}

	root_element = {"events": events, "full_stats": full_stats, "chunks": chunks,
				    "words_freq": words_freq, "full_text": full_text, "tokens": tokens}

	json_report = json.dumps(root_element, indent = 4, ensure_ascii=False) 

	return json_report


def save_downloaded_and_name(path_user_logs, message, downloaded_file):

	record_file_path = path_user_logs + '/record_' + str(message.id) + '.ogg'
	spectrum_dir_path = path_user_logs

	print(record_file_path, " <- dir path")

	with open(os.path.join(record_file_path), 'wb') as new_file:
		new_file.write(downloaded_file)

	alias_name = "a" + str(message.chat.id)  + "b" + str(message.id) + ".ogg"

	return record_file_path, alias_name, spectrum_dir_path


def save_images_info(spectrum_dir_path, message, voice_report):

	rosaInfo = open(spectrum_dir_path + '/rosaInfo.png', 'rb')
	bot.send_photo(message.chat.id, rosaInfo)

	praatInfo = open(spectrum_dir_path + '/praatInfo.png', 'rb')
	bot.send_photo(message.chat.id, praatInfo)

	bot.reply_to(message, voice_report)


def save_json_products(path_user_logs, json_report, full_string):

	with open(path_user_logs + '/full_report.json', 'w') as outfile:
		outfile.write(json_report)

	with open(path_user_logs + '/stt.json', 'w') as outfile:
		outfile.write(full_string)


def merge_text_from_request(req):

	text_lines = []
	message_text = ""

	print("Text chunks:")
	for chunk in req['response']['chunks']:
		print(chunk['alternatives'][0]['text'])
		text_lines.append(chunk['alternatives'][0]['text']) #TODO check alternatives
		message_text += chunk['alternatives'][0]['text'] + "\n"

	return message_text


def send_message_and_reports(path_user_logs, message, message_text):

	bot.reply_to(message, message_text)

	doc = open(path_user_logs + '/stt.json', 'rb')
	bot.send_document(message.chat.id, doc)

	doc = open(path_user_logs + '/full_report.json', 'rb')
	bot.send_document(message.chat.id, doc)


def deplayed_recognition(path_user_logs, message, downloaded_file):

	record_file_path, alias_name, spectrum_dir_path = save_downloaded_and_name(path_user_logs, message, downloaded_file)

	id = request_recognition(record_file_path, alias_name)

	voice_report, f0, rms, pitch, intensity, duration = extract_save_images(record_file_path, spectrum_dir_path) 

	save_images_info(spectrum_dir_path, message, voice_report)

	req = check_server_recognition(id)

	full_string = json.dumps(req, ensure_ascii=False, indent=2)
	json_report = make_json_report(req, f0, rms, pitch, intensity, duration)

	save_json_products(config['dir'], json_report, full_string)

	message_text = merge_text_from_request(req)
		
	send_message_and_reports(config['dir'], message, message_text)



def send_delayed_text(message):
	print("Озвучивание текста")

	from synth_speech import text_to_audio
	text_to_audio("123.ogg", message.text)

	bot.reply_to(message, 'Озвучивание:')
	voice = open("123.ogg", 'rb')
	bot.send_voice(message.chat.id, voice)

	print("Текст озвучен")


def draw_intensity_praat(intensity):

    plt.plot(intensity.xs(), intensity.values.T, linewidth=3, color='g')
    plt.plot(intensity.xs(), intensity.values.T, linewidth=1)
    plt.grid(False)
    plt.ylim(0, 100)
    plt.ylabel("intensity [dB]")



def draw_pitch_praat(pitch):

    pitch_values = pitch.selected_array['frequency']
    pitch_values[pitch_values==0] = np.nan
    plt.plot(pitch.xs(), pitch_values, 'o', markersize=5, color='r')
    plt.plot(pitch.xs(), pitch_values, 'o', markersize=2, color='r')
    plt.grid(False)
    plt.ylim(0, pitch.ceiling)
    plt.ylabel("fundamental frequency [Hz]")



def save_pitches(f0, pitch, output_filepath):

	fig = plt.figure()

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




def extract_save_images(input_filename, output_filepath):
	
	if os.path.exists(output_filepath + '/pcm.wav'):
		os.remove(output_filepath +"/pcm.wav")

	command = f"ffmpeg -i {input_filename} -ar 48000 -ac 2 -ab 192K -f wav {output_filepath}/pcm.wav" #Optional converting to wav #update SR
	_ = check_call(command.split())

	wave_file = output_filepath + "/pcm.wav"

	y, sr = librosa.load(wave_file) #input_filename
	f0, rms = extract_save_librosa(y, sr, output_filepath + '/rosaInfo.png') #TODO загружать Wav

	report, pitch, intensity, duration = extract_save_praat(wave_file, output_filepath)

	save_pitches(f0, pitch, output_filepath)

	return report, f0, rms, pitch, intensity, duration


def extract_save_praat(wave_file, output_filepath):

	snd = parselmouth.Sound(wave_file) #TODO make global in class
	intensity = snd.to_intensity()

	fig = plt.figure()
	pitch = snd.to_pitch()
	draw_pitch_praat(pitch)

	plt.twinx()
	draw_intensity_praat(intensity)
	plt.xlim([snd.xmin, snd.xmax])

	fig.set_size_inches(12, 9)
	
	plt.savefig(output_filepath + '/praatInfo.png', bbox_inches='tight')

	f0min = 60
	f0max = 600

	from parselmouth.praat import call

	pitch = call(snd, "To Pitch", 0.0, f0min, f0max)  #TODO make global in class
	pulses = call([snd, pitch], "To PointProcess (cc)") #TODO make global in class
	duration = call(snd, "Get total duration") #TODO make global in class 

	full_report = call([snd, pitch, pulses], "Voice report", 0, duration, 60, 600, 1.3, 1.6, 0.03, 0.45)
	
	return full_report, pitch, intensity, duration



def extract_save_librosa(y,  sr, output_filename):

	S, phase = librosa.magphase(librosa.stft(y))
	rms = librosa.feature.rms(S=S)
	fig, ax = plt.subplots(nrows=3, sharex=True)

	times = librosa.times_like(rms)

	ax[0].semilogy(times, rms[0], label='RMS Energy')
	ax[0].set(xticks=[])
	ax[0].legend()
	ax[0].label_outer()

	librosa.display.specshow(librosa.amplitude_to_db(S, ref=np.max),
							y_axis='log', x_axis='time', ax=ax[1])

	f0, voiced_flag, voiced_probs = librosa.pyin(y,
		fmin=librosa.note_to_hz('C2'),
		fmax=librosa.note_to_hz('C7'))

	ax[1].plot(times, f0, color='green', linewidth=3)
	ax[1].legend(loc='upper right')
							
	ax[1].set(title='log Power spectrogram')

	o_env = librosa.onset.onset_strength(y=y, sr=sr)
	times = librosa.times_like(o_env, sr=sr)
	onset_frames = librosa.onset.onset_detect(onset_envelope=o_env, sr=sr)

	ax[2].plot(times, o_env)
	ax[2].vlines(times[onset_frames], 0, o_env.max(), color='r', alpha=0.9,
			linestyle='--')
	ax[2].legend()

	fig.set_size_inches(12, 9)

	plt.savefig(output_filename, bbox_inches='tight')

	return f0, rms #voiced_flag, voiced_probs



@bot.message_handler(commands=['start', 'help'])
def send_welcome(message):
	bot.reply_to(message, "Menu yet not implemented")


@bot.message_handler(func=lambda message: True)
def echo_all(message):
	t = threading.Timer(1.0, send_delayed_text, [message])
	t.start()


@bot.message_handler(content_types=['voice'])
def process_voice_message(message):

	path_user_logs = config["dir"] + '/' + str(message.chat.id)
	if not os.path.exists(path_user_logs):
		os.makedirs(path_user_logs)

	file_info = bot.get_file(message.voice.file_id)
	downloaded_file = bot.download_file(file_info.file_path)

	bot.reply_to(message, f"Запись обрабатывается. Момент: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

	t = threading.Timer(1.0, deplayed_recognition, [path_user_logs, message, downloaded_file])
	t.start()

	print("Audio saved")




def local_recognition(spectrum_dir_path, record_file_path, alias_name):

	id = request_recognition(record_file_path, alias_name)

	full_report, f0, rms, pitch, intensity, duration = extract_save_images(record_file_path, spectrum_dir_path) 
	wav_file = f"{spectrum_dir_path}/pcm.wav"

	req = check_server_recognition(id)

	full_string = json.dumps(req, ensure_ascii=False, indent=2)
	json_report = make_json_report(req, f0, rms, pitch, intensity, duration, wav_file)

	save_json_products(spectrum_dir_path, json_report, full_string)

	with open(spectrum_dir_path + '/info_.txt', 'w') as outfile:
		outfile.write(full_report)

	#with open(spectrum_dir_path + '/info2.txt', 'w') as outfile:
	#	outfile.write(voice_report_str2)



#print("Starting bot")
#bot.infinity_polling()
#print("Bot is done")

local_recognition('C:/Users/constcut/Desktop/local', 'C:/Users/constcut/Desktop/local/local.ogg', "localtest")
