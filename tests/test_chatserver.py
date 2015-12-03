#! /usr/bin/python
import socket
import struct

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("localhost", 30050))

s.sendall('{"o":3,"data":{"auth_token":"12345","server_name":"testserver"}}\0'.encode())
s.sendall('{"o":4,"data":{"message":"test msg","author":"pupol","channel":"main"}}\0'.encode())

s.close()
