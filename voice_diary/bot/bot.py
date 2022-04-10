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

import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns

from subprocess import check_call #used for ffmpeg ogg to wav

import threading
import time


with open('key.json', 'r') as file:
    key = json.load(file)

bot = telebot.TeleBot(key["key"])


def sendDelayed(message):
	print("Delayed timer!")
	bot.reply_to(message, 'Delayed message!')


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



def saveImages(input_filename, output_filepath):

	os.remove(output_filepath +"/pcm.wav")
	command = f"ffmpeg -i {input_filename} -ar 16000 -ac 2 -ab 192K -f wav {output_filepath}/pcm.wav" #Optional converting to wav
	_ = check_call(command.split())

	temp_wav = output_filepath + "/pcm.wav"

	y, sr = librosa.load(temp_wav) #input_filename
	saveBlend(y, sr, output_filepath + '/rosaInfo.png') #TODO загружать Wav

	#TODO выделить в подфункцию

	snd = parselmouth.Sound(output_filepath + "/pcm.wav")
	intensity = snd.to_intensity()

	fig = plt.figure()
	pitch = snd.to_pitch()
	draw_pitch_praat(pitch)

	plt.twinx()
	draw_intensity_praat(intensity)
	plt.xlim([snd.xmin, snd.xmax])

	fig.set_size_inches(12, 9)
	
	plt.savefig(output_filepath + '/praatInfo.png', bbox_inches='tight')

	#Дополнительный анализ
	f0min = 60
	f0max = 300
	unit = "Hertz"

	duration = call(snd, "Get total duration") # duration
	pitch = call(snd, "To Pitch", 0.0, f0min, f0max) #create a praat pitch object
	meanF0 = call(pitch, "Get mean", 0, 0, unit) # get mean pitch
	stdevF0 = call(pitch, "Get standard deviation", 0 ,0, unit) # get standard deviation
	pointProcess = call(snd, "To PointProcess (periodic, cc)", f0min, f0max)
	localJitter = call(pointProcess, "Get jitter (local)", 0, 0, 0.0001, 0.02, 1.3)
	localShimmer =  call([snd, pointProcess], "Get shimmer (local)", 0, 0, 0.0001, 0.02, 1.3, 1.6)

	print(duration, meanF0, stdevF0)

	full_dict = {"duration":duration, "meanF0": meanF0, "stdevF0": stdevF0, "local jitter": localJitter,
	"local shimmer": localShimmer}

	pulses = call([snd, pitch], "To PointProcess (cc)")
	voice_report_str = call([snd, pitch, pulses], "Voice report", 0.0, 0.0, 75, 600, 1.3, 1.6, 0.03, 0.45)


	#https://stackoverflow.com/questions/45237091/how-to-automate-voice-reports-for-praat
	#https://www.fon.hum.uva.nl/praat/manual/Voice_6__Automating_voice_analysis_with_a_script.html
	#Refactoring + some more: https://github.com/drfeinberg/PraatScripts/blob/master/Measure%20Pitch%2C%20HNR%2C%20Jitter%2C%20Shimmer%2C%20and%20Formants.ipynb
	return full_dict, voice_report_str


def deplayed_recognition(path_user_logs, message, downloaded_file):

	record_file_path = path_user_logs + '/record_' + str(message.id) + '.ogg'
	spectrum_file_path = path_user_logs

	print(record_file_path, " <- filepath")

	with open(os.path.join(record_file_path), 'wb') as new_file:
		new_file.write(downloaded_file)

	full_dict, voice_report = saveImages(record_file_path, spectrum_file_path)

	rosaInfo = open(spectrum_file_path + '/rosaInfo.png', 'rb')
	bot.send_photo(message.chat.id, rosaInfo)

	praatInfo = open(spectrum_file_path + '/praatInfo.png', 'rb')
	bot.send_photo(message.chat.id, praatInfo)

	bot.reply_to(message, voice_report)


def saveBlend(y,  sr, output_filename):

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



@bot.message_handler(commands=['start', 'help'])
def send_welcome(message):
	bot.reply_to(message, "Menu yet not implemented")


@bot.message_handler(func=lambda message: True)
def echo_all(message):

	#TODO speech generation
	bot.reply_to(message, message.text)


@bot.message_handler(content_types=['voice'])
def process_voice_message(message):

	path_user_logs = key["dir"] + '/' + str(message.chat.id)
	if not os.path.exists(path_user_logs):
		os.makedirs(path_user_logs)

	file_info = bot.get_file(message.voice.file_id)
	downloaded_file = bot.download_file(file_info.file_path)

	bot.reply_to(message, f"Запись обрабатывается. Момент: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

	t = threading.Timer(1.0, deplayed_recognition, [path_user_logs, message, downloaded_file])
	t.start()

	print("Audio saved")



print("Starting bot")

bot.infinity_polling()

print("Bot is done")