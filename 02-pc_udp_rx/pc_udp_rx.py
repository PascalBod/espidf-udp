# This python script listens on UDP port 3333 
# for messages from the ESP32 board and prints them.
# 
# Derived from https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiUDPClient/udp_server.py

import socket
import sys

try :
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
except OSError as msg :
    print(f'Failed to create socket. Error Code : {str(msg[0])} - Message {msg[1]}')
    sys.exit()

try:
    s.bind(('', 3333))
except OSError as msg:
    print(f'Bind failed. Error: {str(msg[0])} : {msg[1]}')
    sys.exit()
     
print('Server listening')

while 1:
    d, a = s.recvfrom(1024)
    print(f'Message from {a}: {d.decode()}')
    
s.close()
