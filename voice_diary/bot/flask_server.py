from flask import Flask, redirect, url_for, request
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

        return response_string


api.add_resource(Login, '/login')

if __name__ == '__main__':
    app.run(debug=True) #avoid debug later