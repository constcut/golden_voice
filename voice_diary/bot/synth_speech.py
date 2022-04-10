import argparse
from distutils.command.config import config
import requests
import json


with open('key.json', 'r') as file:
    config = json.load(file)
key = config["api-key"]


def synthesize(folder_id, iam_token, text):
    url = 'https://tts.api.cloud.yandex.net/speech/v1/tts:synthesize'
    headers = {
        'Authorization': 'Bearer ' + iam_token,
    }


    header = {'Authorization': 'Api-Key {}'.format(key)}

    data = {
        'text': text,
        'lang': 'ru-RU',
        'voice': 'filipp',
        #'folderId': folder_id
    }

    with requests.post(url, headers=header, data=data, stream=True) as resp:
        if resp.status_code != 200:
            raise RuntimeError("Invalid response received: code: %d, message: %s" % (resp.status_code, resp.text))

        for chunk in resp.iter_content(chunk_size=None):
            yield chunk


if __name__ == "__main__":

    with open(config["dir"] + "/chech.ogg", "wb") as f:
        for audio_content in synthesize('', '', "Продолжение экспериментов"):
            f.write(audio_content)