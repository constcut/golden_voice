import boto3
import json

#class yandex_object_storage class

def upload_file(filename, alias):

    with open('key.json', 'r') as file:
        config = json.load(file)


    session = boto3.session.Session()
    s3 = session.client(
        service_name='s3',
        endpoint_url='https://storage.yandexcloud.net'
    )


    s3.upload_file(filename, config["bucket"], alias)

    print("File uploaded")



def get_all_files():

    #TODO реализовать как класс, для единичного создания сессии и чтения JSON

    with open('key.json', 'r') as file:
        config = json.load(file)

    session = boto3.session.Session()
    s3 = session.client(
        service_name='s3',
        endpoint_url='https://storage.yandexcloud.net'
    )

    files_list = []

    for key in s3.list_objects(Bucket=config["bucket"])['Contents']:
        print(key['Key'])
        files_list.append(key['Key'])

    return files_list


def delete_file(alias): #TODO + another function to clean all using time (after 1 week)

    with open('key.json', 'r') as file:
        config = json.load(file)

    session = boto3.session.Session()
    s3 = session.client(
        service_name='s3',
        endpoint_url='https://storage.yandexcloud.net'
    )

    forDeletion = [{'Key':alias}] 
    response = s3.delete_objects(Bucket=config["bucket"], Delete={'Objects': forDeletion})



#Допольнительная справочная информация:
# Создать новый бакет
#s3.create_bucket(Bucket=key["bucket"])
## Сохранить строковый объект, это может быть JSON\CSV или просто строка
#s3.put_object(Bucket='audio', Key='object_name', Body='TEST', StorageClass='COLD')