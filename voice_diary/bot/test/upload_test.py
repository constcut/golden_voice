

import requests


local_file_path = 'local_z.ogg'

upload_url = "http://127.0.0.1:8000/new_test.ogg"

data = open(local_file_path, 'rb').read()
headers = {
    "Content-Type":"application/binary",
}
upload = requests.put(upload_url, data=data, headers=headers)


print("Upload done: ", upload)

print("DONE!")