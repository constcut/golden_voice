
import os
import posixpath
import http.server
import urllib.request, urllib.parse, urllib.error
import shutil
import mimetypes
from io import BytesIO


#TODO clean exceeds
 
 
class SimpleHTTPRequestHandler(http.server.BaseHTTPRequestHandler):

    def do_PUT(self):

        path = self.translate_path(self.path) # TODO Избавиться, мы будем выбирать директории вручную

        #Нужно распарсить полный путь, и отделить папку пользователя от самого названия
        #Однако если эта запись уже существует - выдать на неё ответ с ключем и id не осуществляя никаких других действий
        #TODO
        #Сделать 3 подпапки audio\text\image - и отправлять в виде accumerite.ru/audio/USERID_FILENAME.ext
        #итого USERID\audio\ USERID\text\ USEROD\image\

        if path.endswith('/'):
            self.send_response(405, "Method Not Allowed")
            self.wfile.write("PUT not allowed on a directory\n".encode())
            return
        else:
            try:
                os.makedirs(os.path.dirname(path))

            except FileExistsError: pass

            length = int(self.headers['Content-Length'])
            with open(path, 'wb') as f:
                f.write(self.rfile.read(length))
                f.close()

            self.send_response(201, "Created")
            self.end_headers()

            response = '{"done":"+","id":"DONE", "key":"TEST"}'
            self.wfile.write(response.encode("ascii"))



    def do_GET(self):

        print(" do_GET ", self.path)

        params_steps = []
        params_pos = self.path.find("?")

        if params_pos != -1:
            
            full_path = self.path[0: params_pos]
            full_params = self.path[params_pos + 1: ]

            print("full params ", full_params)

            params_groups = full_params.split("&")
            for i in range(0, len(params_groups)):
                params_steps.append(params_groups[i].split("="))

        else:

            full_path = self.path

        path_steps = full_path.split("/")

        response_string = '{"done":"true", "some":"param"}'

        print("PATH STEPS ", path_steps)

        if path_steps[1] == "login":

            check_login = "none"
            check_password = "none"

            for param in params_steps:

                if param[0] == "login":
                    check_login = param[1]

                if param[0] == "password":
                    check_password = param[1]

            if check_login == "testlogin" and check_password == "testpassword":
                response_string = "Logged in!"
            else:
                response_string = "Login or password incorrect"

        self.send_response(200)
        ctype = self.guess_type(self.path) 
        self.send_header("Content-type", ctype) #TODO заменить
        self.send_header("Content-Length", len(response_string)) 
        self.end_headers()

        self.wfile.write(response_string.encode('ascii'))
 

    def do_HEAD(self):
        print(" HEAD request not implemented ")
 

    def do_POST(self):
        print(" POST request not implemented (yet) ") 


    def translate_path(self, path):
        """Translate a /-separated PATH to the local filename syntax.
        Components that mean special things to the local file system
        (e.g. drive or directory names) are ignored.  (XXX They should
        probably be diagnosed.)
        """
        # abandon query parameters
        path = path.split('?',1)[0]
        path = path.split('#',1)[0]
        path = posixpath.normpath(urllib.parse.unquote(path))
        words = path.split('/')
        words = [_f for _f in words if _f]
        path = os.getcwd()
        for word in words:
            drive, word = os.path.splitdrive(word)
            head, word = os.path.split(word)
            if word in (os.curdir, os.pardir): continue
            path = os.path.join(path, word)
        return path
 
    def copyfile(self, source, outputfile):
        """Copy all data between two file objects.
        The SOURCE argument is a file object open for reading
        (or anything with a read() method) and the DESTINATION
        argument is a file object open for writing (or
        anything with a write() method).
        The only reason for overriding this would be to change
        the block size or perhaps to replace newlines by CRLF
        -- note however that this the default server uses this
        to copy binary data as well.
        """
        shutil.copyfileobj(source, outputfile)
 
    def guess_type(self, path):
        """Guess the type of a file.
        Argument is a PATH (a filename).
        Return value is a string of the form type/subtype,
        usable for a MIME Content-type header.
        The default implementation looks the file's extension
        up in the table self.extensions_map, using application/octet-stream
        as a default; however it would be permissible (if
        slow) to look inside the data to make a better guess.
        """
 
        base, ext = posixpath.splitext(path)
        if ext in self.extensions_map:
            return self.extensions_map[ext]
        ext = ext.lower()
        if ext in self.extensions_map:
            return self.extensions_map[ext]
        else:
            return self.extensions_map['']
 
    if not mimetypes.inited:
        mimetypes.init() # try to read system mime.types
    extensions_map = mimetypes.types_map.copy()
    extensions_map.update({
        '': 'application/octet-stream', # Default
        '.py': 'text/plain',
        '.c': 'text/plain',
        '.h': 'text/plain',
        })
 
 
def test(HandlerClass = SimpleHTTPRequestHandler,
        ServerClass = http.server.HTTPServer):

    http.server.test(HandlerClass, ServerClass)
 
if __name__ == '__main__':
    test()