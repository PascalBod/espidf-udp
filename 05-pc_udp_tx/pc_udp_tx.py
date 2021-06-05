# This python program sends a UDP datagram to the ESP32, on a periodic basis.

import socket
import sys
import time
 
counter = 1

# Adapt to the IP address of the ESP32.
serverAddressPort   = ("192.168.1.27", 3333)

try :
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
except OSError as msg :
    print(f'Failed to create socket. Error Code : {str(msg[0])} - Message {msg[1]}')
    sys.exit()

while 1:
    msg = f'This is message {counter:03} from PC.'
    bytesToSend  = str.encode(msg)
    print(f'Sending a datagram to ESP32: {msg}')
    s.sendto(bytesToSend, serverAddressPort)
    time.sleep(25)
    if counter == 255:
        counter = 1
    else:
        counter += 1

