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


#TODO to class
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



def make_json_report(req, f0, rms, pitch, intensity, duration):

	intensity = intensity.values.T
	
	f0_step = duration / len(f0)
	rms_step = duration / len(rms[0])
	pitch_step = duration / len(pitch)
	intensity_step = duration / len(intensity)

	pitch = pitch.selected_array['frequency']

	intensity = intensity.reshape(intensity.shape[0] * intensity.shape[1])
	rms = rms.reshape(rms.shape[0] * rms.shape[1])

	words = []
	prev_word_end = 0.0

	chunkId = 0
	for chunk in req['response']['chunks']:

		altId = 0
		for alt in chunk['alternatives']: #We don't handle silence right yet in case for alts

			for word in alt['words']:

				start = float(word['startTime'][:-1])
				end = float(word['endTime'][:-1])

				silence_start = prev_word_end
				silence_end = start

				#TODO create silence object

				f0_cut = make_cut(f0_step, start, end, f0)
				pitch_cut = make_cut(pitch_step, start, end, pitch)
				intens_cut = make_cut(intensity_step, start, end, intensity)
				rms_cut = make_cut(rms_step, start, end, rms)

				import statistics #TODO mean, median, mode #function to calculate for all

				singleWord =  {"chunkId" : chunkId, "altId": altId, "word": word['word'], 
				"startTime": start, "endTime": end, 
				"confidence": word['confidence'], 
				"pYin": list(f0_cut), "RMS": list(rms_cut), "pPitch": list(pitch_cut), "dB": list(intens_cut)} #channel tag left away

				words.append(singleWord)

			altId += 1

		chunkId += 1

	root_element = {"words": words}
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

	command = f"ffmpeg -i {input_filename} -ar 16000 -ac 2 -ab 192K -f wav {output_filepath}/pcm.wav" #Optional converting to wav #update SR
	_ = check_call(command.split())

	wave_file = output_filepath + "/pcm.wav"

	y, sr = librosa.load(wave_file) #input_filename
	f0, rms = extract_save_librosa(y, sr, output_filepath + '/rosaInfo.png') #TODO загружать Wav

	voice_report_str, pitch, intensity, duration = extract_save_praat(wave_file, output_filepath)

	save_pitches(f0, pitch, output_filepath)

	return voice_report_str, f0, rms, pitch, intensity, duration


def extract_save_praat(wave_file, output_filepath):

	snd = parselmouth.Sound(wave_file)
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
	f0max = 300

	pitch = call(snd, "To Pitch", 0.0, f0min, f0max) 
	pulses = call([snd, pitch], "To PointProcess (cc)")
	voice_report_str = call([snd, pitch, pulses], "Voice report", 0.0, 0.0, 75, 600, 1.3, 1.6, 0.03, 0.45)
	duration = call(snd, "Get total duration") 

	return voice_report_str, pitch, intensity, duration



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

	voice_report, f0, rms, pitch, intensity, duration = extract_save_images(record_file_path, spectrum_dir_path) 

	req = check_server_recognition(id)

	full_string = json.dumps(req, ensure_ascii=False, indent=2)
	json_report = make_json_report(req, f0, rms, pitch, intensity, duration)

	save_json_products(spectrum_dir_path, json_report, full_string)



#print("Starting bot")
#bot.infinity_polling()
#print("Bot is done")

local_recognition('C:/Users/constcut/Desktop/local', 'C:/Users/constcut/Desktop/local/local.ogg', "localtest")
