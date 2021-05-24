# Overview

This repository contains several applications that demonstrate ste-by-step how to exchange UDP messages between an ESP-IDF application and a PC application.

These applications are:
* *wifi*, stored in `01-wifi` directory: an ESP-IDF application that demonstrates how to connect to a Wi-Fi Access Point
* *pc_udp_rx*, stored in `02-pc_app_rx` directory: a PC Python application that waits for UDP messages and displays them
* *udp_tx*, stored in `03-udp_tx` directory: an ESP-IDF application that sends UDP messages to *pc_udp_rx*
* *udp_txrx*, stored in `04-udp_txrx` directory: an ESP-IDF application that sends and receives messages
* *pc_udp_rxtx*, stored in `05-pc_udp_rxtx` directory: a PC Python application that sends and receives UDP messages