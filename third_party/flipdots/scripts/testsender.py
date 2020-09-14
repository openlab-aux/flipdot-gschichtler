#!/usr/bin/env python2

import socket
import time
import sys

UDPHOST="flipdot.lab"
UDPPORT=2323

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def send(value):
  msg = chr(value)*((16*80)/8)
  sock.sendto(msg, (UDPHOST, UDPPORT))

v = int(sys.argv[1])
send(v)
