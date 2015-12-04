#! /usr/bin/python
import socket
import struct
import lz4

def sendPkt(s, jstr):
	#packed_data = struct.pack('!I%ss' % len(jstr), len(jstr), lz4.dumps(jstr))
	packed_data = struct.pack('!I%ss' % len(jstr), len(jstr), jstr.encode())
	print(len(packed_data))
	s.sendall(packed_data)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("localhost", 30050))


sendPkt(s, '{"o":3,"data":{"auth_token":"12345","server_name":"testserver"}}')
sendPkt(s, '{"o":4,"data":{"message":"test msg","author":"pupol","channel":"main"}}')

s.close()
