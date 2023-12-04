# -*- coding: utf-8 -*-

import requests
import time
import json


# Unite with synth_speech into yandex_speechkit

def recoginze_speech(alias):

    with open('key.json', 'r') as file:
        config = json.load(file)

    key = config["api-key"]
    filelink = 'https://storage.yandexcloud.net/' + \
        config["bucket"] + '/' + \
        alias  # TODO или даже хранить алиас без расширения

    POST = "https://transcribe.api.cloud.yandex.net/speech/stt/v2/longRunningRecognize"

    body = {
        "config": {
            "specification": {
                "languageCode": "ru-RU"
            }
        },
        "audio": {
            "uri": filelink
        }
    }

    header = {'Authorization': 'Api-Key {}'.format(key)}

    req = requests.post(POST, headers=header, json=body)
    data = req.json()
    print(data)

    id = data['id']

    while True:

        time.sleep(10)  # TODO рассчитывать от длины изначального аудио

        GET = "https://operation.api.cloud.yandex.net/operations/{id}"
        req = requests.get(GET.format(id=id), headers=header)
        req = req.json()

        if req['done']:
            break
        print("Not ready")

    print("Response:")

    full_string = json.dumps(req, ensure_ascii=False, indent=2)

    print(full_string)

    with open(config['dir'] + '/stt.json', 'w') as outfile:
        outfile.write(full_string)

    text_lines = []

    print("Text chunks:")
    for chunk in req['response']['chunks']:
        print(chunk['alternatives'][0]['text'])
        # TODO check alternatives
        text_lines.append(chunk['alternatives'][0]['text'])

    return full_string, text_lines


recoginze_speech('present_test2.ogg')
