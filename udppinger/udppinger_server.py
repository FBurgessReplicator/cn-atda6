#!/usr/bin/env python3

# File: udppinger_server.py
# Description: cn-atda6 socket programming assignment 2, python version
#              a server which receives ping messages and responds with pong messages
# Author: Burgess Wong
# Created Date: 2013-9-29

import socket

if __name__ == '__main__':
    server_ip = ''
    server_port = 8080
    server_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_sock.bind((server_ip, server_port))

    print('---Ready for receiving and responding ping messages\n')

    while 1:
        message, address = server_sock.recvfrom(4096)

        print('---Receive a ping from %s:%d' % address)
        
        server_sock.sendto(b'pong' + message[4:6], address)
    
