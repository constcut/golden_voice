from distutils.command.config import config
import requests
import json


with open('key.json', 'r') as file:
    config = json.load(file)
key = config["api-key"]


def text_to_speech(text):
    url = 'https://tts.api.cloud.yandex.net/speech/v1/tts:synthesize'

    header = {'Authorization': 'Api-Key {}'.format(key)}

    data = {
        'text': text,
        'lang': 'ru-RU',
        'voice': 'filipp',
    }

    with requests.post(url, headers=header, data=data, stream=True) as resp:
        if resp.status_code != 200:
            raise RuntimeError("Invalid response received: code: %d, message: %s" % (resp.status_code, resp.text))

        for chunk in resp.iter_content(chunk_size=None):
            yield chunk


def text_to_audio(filename, text):
    with open(filename, "wb") as f:
        for audio_content in text_to_speech('', '', text):
            f.write(audio_content)
