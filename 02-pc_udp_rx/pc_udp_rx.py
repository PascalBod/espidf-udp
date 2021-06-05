# This python program listens on UDP port 3333 
# for messages from the ESP32 and prints them.

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
