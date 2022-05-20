from flask import Flask, redirect, url_for, request, make_response
from flask_restful import Resource, Api, reqparse

app = Flask(__name__)
api = Api(app)


class Login(Resource):

    def get(self):

        args = request.args

        if args["login"] == "testlogin" and args["password"] == "testpassword":
            response_string = "Logged in!"
        else:
            response_string = "Login or password incorrect"

        response = make_response(response_string, 200)
        response.mimetype = "text/plain"
        return response


class Processed(Resource):

    def get(self):

        args = request.args

        login = args["login"]
        id = args["id"]
        key = args["key"]

        f = open(login + "/reports/full_report.json")
        response_string = f.read()
        f.close()

        response = make_response(response_string, 200)
        response.mimetype = "text/plain"
        return response


class AudioUpload(Resource): #add get later - to load last file back

    def put(self):

        args = request.args
        login = args["login"]

        filepath = login + "/audio/id0.ogg"

        with open(filepath, 'wb') as f:
            f.write(request.stream.read())
            f.close()

        response = make_response("Created", 200)
        response.mimetype = "text/plain"
        return response


class ImageUpload(Resource):

    def put(self):

        args = request.args
        login = args["login"]

        filepath = login + "/image/id0.png"

        with open(filepath, 'wb') as f:
            f.write(request.stream.read())
            f.close()

        response = make_response("Created", 200)
        response.mimetype = "text/plain"
        return response


#+ toto simple

api.add_resource(Login, '/login')
api.add_resource(Processed, '/processed')
api.add_resource(AudioUpload, "/audio")
api.add_resource(ImageUpload, "/image")

if __name__ == '__main__':
    app.run(debug=True) #avoid debug later