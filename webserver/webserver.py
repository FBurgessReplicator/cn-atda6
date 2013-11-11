#!/usr/bin/env python3

# File: webserver.py
# Description: cn-atda6 socket programming assignment 1, python version
#              a simple webserver which accepts a file path and return that file
#              in a http response
# Author: Burgess Wong
# Created Date: 2013-9-26

import io
import mimetypes
import socket

def resolve_request(request):
    request_line, sep, request = request.partition(b'\r\n')
    method, url, version = request_line.split(b' ')

    return (url, version)

def generate_response(version, status_code, phrase, content_type, file_contents):
    status_line = version + b' ' + ' '.join([status_code, phrase]).encode('ASCII')
    connection_line = b'Connection: close'
    content_length_line = ('Content-Length: ' + str(len(file_contents))).encode('ASCII')
    content_type_line = ('Content-Type: ' + content_type).encode('ASCII')
    response_body = file_contents
    
    return b'\r\n'.join([status_line, connection_line, content_length_line, content_type_line + b'\r\n', response_body])

    
    
if __name__ == '__main__':

    # initialize the sock
    server_ip = ''
    server_port = 8080
    server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_sock.bind((server_ip, server_port))
    server_sock.listen(1)

    # infinite loop waiting for and processing requests
    while 1:
        print('---Listening---\n')

        connect_sock, address = server_sock.accept()
        request = connect_sock.recv(4096)
        
        print('---Request comes from IP: ' + str(address[0]))

        # ignore empty request
        if len(request) == 0:
            print('---Empty request, ignore')
            print('')
            continue

        # resolve the request
        url, version = resolve_request(request)
        path = url.decode('ASCII')

        print('---Requested file path: ' + path)

        # try to open the file in the filesystem
        try:
            file_contents = open(path, 'rb').read()
            status_code = '200'
            phrase = 'OK'
            content_type = mimetypes.guess_type(path)[0]
            if not content_type:
                content_type = 'text/plain'            
            print('---File ' + path + ' found, generate 200 OK response')
        except IOError:
            file_contents = b'404: Page Not Found - by Burgess Wong'
            status_code = '404'
            phrase = 'Not Found'
            content_type = 'text/html'
            print('---File ' + path + ' not found, generate 404 Not Found response')
            
        # make the response message
        response = generate_response(version, status_code, phrase, content_type, file_contents)

        # send the response and close the socket
        connect_sock.send(response)

        print('---Response sent')

        connect_sock.close()

        print('---Connection with IP ' + str(address[0]) + ' closed\n')

