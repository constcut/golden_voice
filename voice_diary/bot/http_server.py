
"""Simple HTTP Server With Upload.
This module builds on BaseHTTPServer by implementing the standard GET
and HEAD requests in a fairly straightforward manner.
see: https://gist.github.com/UniIsland/3346170
"""
 
 
__version__ = "0.1"
__all__ = ["SimpleHTTPRequestHandler"]
__author__ = "bones7456"
__home_page__ = "http://li2z.cn/"
 
import os
import posixpath
import http.server
import urllib.request, urllib.parse, urllib.error
import html
import shutil
import mimetypes
import re
from io import BytesIO


#TODO clean exceeds
 
 
class SimpleHTTPRequestHandler(http.server.BaseHTTPRequestHandler):
 
    """Simple HTTP request handler with GET/HEAD/POST commands.
    This serves files from the current directory and any of its
    subdirectories.  The MIME type for files is determined by
    calling the .guess_type() method. And can reveive file uploaded
    by client.
    The GET/HEAD/POST requests are identical except that the HEAD
    request omits the actual contents of the file.
    """
 
    server_version = "SimpleHTTPWithUpload/" + __version__

    def do_PUT(self):

        path = self.translate_path(self.path)

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

            self.send_response(201, "Created")
            self.send_header("Response", '{"json":"field"}')
            self.end_headers()

            f = open("C:/Users/constcut/Desktop/local/zx.json", 'wb') #TODO при повторной отправке того же имени
            f.write(b'{"done":"+","id":"DONE", "key":"TEST"}')
            f.close()

            f = open("C:/Users/constcut/Desktop/local/zx.json", 'rb')
            if f:
                self.copyfile(f, self.wfile)
                f.close()

            



    def do_GET(self):

        print(" do_GET ", self.path)

        f = open("C:/Users/constcut/Desktop/local/get.txt", 'wb') #TODO при повторной отправке того же имени
        f.write(b'{"result":"report"}')
        f.close()    

        f = open("C:/Users/constcut/Desktop/local/get.txt", 'rb')

        #/report/fullusername_1.2.3 - in to parts TODO
        #/check/id_key - same structure

        #++ parse all the params

        print("GET FULL PATH ", self.path)

        self.send_response(200)
        ctype = self.guess_type(self.path)
        self.send_header("Content-type", ctype)
        fs = os.fstat(f.fileno())
        self.send_header("Content-Length", str(fs[6]))
        self.send_header("Last-Modified", self.date_time_string(fs.st_mtime))
        self.end_headers()

        if f:
            self.copyfile(f, self.wfile)
            f.close()

        #"""Serve a GET request."""
        #f = self.send_head()
        #if f:
        #    self.copyfile(f, self.wfile)
        #    f.close()
 
    def do_HEAD(self):

        print(" do_HEAD ")

        """Serve a HEAD request."""
        f = self.send_head()
        if f:
            f.close()
 
    def do_POST(self):

        print(" do_POST ") #TODO try another python test

        """Serve a POST request."""

        r, info = self.deal_post_data()

        print((r, info, "by: ", self.client_address))

        f = BytesIO() 
        f.write(b'JSON ANSWER') 
        length = f.tell()
        f.seek(0)

        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.send_header("Content-Length", str(length))
        self.end_headers()

        if f:
            self.copyfile(f, self.wfile)
            f.close()
        

    def deal_post_data(self):
        content_type = self.headers['content-type']

        if not content_type:
            return (False, "Content-Type header doesn't contain boundary")

        print("DEBUG: ", content_type.split("="))

        boundary = content_type.split("=")[1].encode()
        remainbytes = int(self.headers['content-length'])
        line = self.rfile.readline()
        remainbytes -= len(line)

        if not boundary in line:
            return (False, "Content NOT begin with boundary")

        line = self.rfile.readline()
        remainbytes -= len(line)
        fn = re.findall(r'Content-Disposition.*name="file"; filename="(.*)"', line.decode())

        if not fn:
            return (False, "Can't find out file name...")

        path = self.translate_path(self.path)
        fn = os.path.join(path, fn[0])
        line = self.rfile.readline()
        remainbytes -= len(line)
        line = self.rfile.readline()
        remainbytes -= len(line)
        try:
            out = open(fn, 'wb')
        except IOError:
            return (False, "Can't create file to write, do you have permission to write?")
                
        preline = self.rfile.readline()
        remainbytes -= len(preline)
        while remainbytes > 0:
            line = self.rfile.readline()
            remainbytes -= len(line)
            if boundary in line:
                preline = preline[0:-1]
                if preline.endswith(b'\r'):
                    preline = preline[0:-1]
                out.write(preline)
                out.close()
                return (True, "File '%s' upload success!" % fn)
            else:
                out.write(preline)
                preline = line
        return (False, "Unexpect Ends of data.")
 
    def send_head(self):
        """Common code for GET and HEAD commands.
        This sends the response code and MIME headers.
        Return value is either a file object (which has to be copied
        to the outputfile by the caller unless the command was HEAD,
        and must be closed by the caller under all circumstances), or
        None, in which case the caller has nothing further to do.
        """
        path = self.translate_path(self.path)
        f = None

        if os.path.isdir(path):
            return None;
            
        ctype = self.guess_type(path)
        try:
            # Always read in binary mode. Opening files in text mode may cause
            # newline translations, making the actual size of the content
            # transmitted *less* than the content-length!
            f = open(path, 'rb')
        except IOError:
            self.send_error(404, "File not found")
            return None
        self.send_response(200)
        self.send_header("Content-type", ctype)
        fs = os.fstat(f.fileno())
        self.send_header("Content-Length", str(fs[6]))
        self.send_header("Last-Modified", self.date_time_string(fs.st_mtime))
        self.end_headers()
        return f
 
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