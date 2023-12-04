
import os
import http.server

import threading


class SimpleHTTPRequestHandler(http.server.BaseHTTPRequestHandler):

    def delayed_recognition(self, filename, report_path):

        # os.makedirs(os.path.dirname(report_path)) check exists

        print("Starting delayed_recognition")

        from report import ReportGenerator
        r = ReportGenerator('key.json')

        r.local_recognition(report_path, filename, "testserver")
        print("Delayed done!")

    def get_request_parts(self):

        params_steps = []
        params_pos = self.path.find("?")

        if params_pos != -1:

            full_path = self.path[0: params_pos]
            full_params = self.path[params_pos + 1:]

            print("full params ", full_params)

            params_groups = full_params.split("&")
            for i in range(0, len(params_groups)):
                params_steps.append(
                    params_groups[i].split("="))  # TODO as dict

        else:

            full_path = self.path

        path_steps = full_path.split("/")

        return path_steps, params_steps

    def do_PUT(self):

        path_steps, params_steps = self.get_request_parts()

        username = ""
        for param in params_steps:
            if param[0] == "login":
                username = param[1]
                break

        file_path = ""

        if path_steps[1] == "audio":
            # TODO ID from DB #TODO audio type = wav, mp3, opus
            file_path = username + "/audio/id0.ogg"

        if path_steps[1] == "image":
            file_path = username + "/image/id0.png"

        if path_steps[1] == "text":
            file_path = username + "text/id0.txt"

        if (path_steps[1] != "audio" and path_steps[1] != "image" and path_steps[1] != "text") or username == "":
            self.send_response(405, "Method Not Allowed")
            self.wfile.write("Missing type of username\n".encode())
            return

        try:
            os.makedirs(os.path.dirname(file_path))

        except FileExistsError:
            pass

        length = int(self.headers['Content-Length'])
        with open(file_path, 'wb') as f:
            f.write(self.rfile.read(length))
            f.close()

        if path_steps[1] == "audio":
            t = threading.Timer(1.0, self.delayed_recognition, [
                                file_path, username + "/reports/"])
            t.start()

        print('Written ', length, " bytes")

        self.send_response(201, "Created")
        self.end_headers()

        response = '{"done":"+","id":"id0", "key":"TEST"}'
        self.wfile.write(response.encode("ascii"))

    def do_GET(self):

        print(" do_GET ", self.path)

        path_steps, params_steps = self.get_request_parts()

        response_string = '{"done":"true", "some":"param"}'

        print("PATH STEPS ", path_steps)

        # TODO subfunction to form reply

        check_login = "none"
        check_password = "none"

        for param in params_steps:  # TODO rewrite on dict

            if param[0] == "login":
                check_login = param[1]

            if param[0] == "password":
                check_password = param[1]

        if path_steps[1] == "login":

            if check_login == "testlogin" and check_password == "testpassword":
                response_string = "Logged in!"
            else:
                response_string = "Login or password incorrect"

        if path_steps[1] == "process":
            # TODO read id + key - check them from DB
            # + FORM REAL RESPONSE - yet we use only
            # response_string = '{"done":true, "report":"full text"}'
            f = open(check_login + "/reports/full_report.json")
            response_string = f.read()
            f.close()

        self.send_response(200)
        ctype = 'text/plain'
        self.send_header("Content-type", ctype)
        self.send_header("Content-Length", len(response_string))
        self.end_headers()

        self.wfile.write(response_string.encode('utf-8'))  # Russian letters

    def do_HEAD(self):
        print(" HEAD request not implemented ")

    def do_POST(self):
        print(" POST request not implemented (yet) ")


def test(HandlerClass=SimpleHTTPRequestHandler,
         ServerClass=http.server.HTTPServer):

    http.server.test(HandlerClass, ServerClass)


if __name__ == '__main__':
    test()
