from flask import Flask, redirect, url_for, request, make_response
from flask_restful import Resource, Api, reqparse

app = Flask(__name__)
api = Api(app)


class Login(Resource):

    def get(self):

        args = request.args
        print(args)

        print(args["login"], " and ", args["login"] == "testlogin")
        print(args["password"], " and ", args["password"] == "testpassword")

        response_string = "empty"

        if args["login"] == "testlogin" and args["password"] == "testpassword":
            print("FINE!")
            response_string = "Logged in!"
        else:
            print("incorrect ", args["login"] == "testlogin", args["password"] == "testpassword")
            response_string = "Login or password incorrect"

        response = make_response(response_string, 200)
        response.mimetype = "text/plain"

        return response


api.add_resource(Login, '/login')

if __name__ == '__main__':
    app.run(debug=True) #avoid debug later