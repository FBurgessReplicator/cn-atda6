#!/usr/bin/env python3

# File: udppinger_client.py
# Description: cn-atda6 socket programming assignment 2, python version
#              a client which sends ping messages and gets RTT between the client and server
# Author: Burgess Wong
# Created Date: 2013-10-11

import select
import socket
import sys
import time

def checkresp(client_sock, i):
    message, server_address = client_sock.recvfrom(6)
    number = int(message[4:6])

    return True if number == i else False

if __name__ == '__main__':

    # initialize the socket
    server_ip = sys.argv[1]
    server_port = 8080
    client_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # send 10 ping messages
    for i in range(0, 10):
        client_sock.sendto(('ping%.2d' % i).encode('utf-8'), (server_ip, server_port))

        resttime = 1
        firsttime = time.time()

        while resttime > 0:
            starttime = time.time()
            readable_socks = select.select([client_sock], [], [], resttime)[0]
            endtime = time.time()

            if len(readable_socks) == 0: # client_sock not available, timeout
                print('---ping %.2d timeout' % i)
                break
            else: # receive a pong message, check it
                resttime = resttime - (endtime - starttime)

                if checkresp(client_sock, i): # a corresponding message, print OK
                    print('---pong %.2d received, RTT=%.3fms' % (i, (endtime - firsttime) * 1000))
                    time.sleep(resttime)
                    break
                    
                    
