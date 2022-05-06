

import requests


local_file_path = 'local_z.ogg'


def send_using_put():

    upload_url = "http://127.0.0.1:8000/new_test.ogg"

    data = open(local_file_path, 'rb').read()
    headers = {
        "Content-Type":"application/binary",
    }
    upload = requests.put(upload_url, data=data, headers=headers)


    print("Upload done: ", upload)

    print("DONE!")



def send_using_post():

    test_file = open(local_file_path, "rb")

    upload_url = "http://127.0.0.1:8000/"

    test_response = requests.post(upload_url, files = {"file": test_file})
    
    print("Upload done: ", test_response)

    print("DONE!")



#send_using_put()
send_using_post()