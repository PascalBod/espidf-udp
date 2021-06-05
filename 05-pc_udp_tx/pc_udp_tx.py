# This python program sends a UDP datagram to the ESP32, on a periodic basis.

import socket
import sys
import time
 
msgFromPC = "Hello from PC"
bytesToSend  = str.encode(msgFromPC)

# Use the IP address of the ESP32.
serverAddressPort   = ("192.168.1.25", 3333)

bufferSize          = 1024

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

while 1:
    s.sendto(bytesToSend, serverAddressPort)
    time.sleep(25)

