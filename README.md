# Overview

This repository contains several applications that demonstrate step-by-step how to exchange UDP messages between an ESP-IDF application and a PC application.

These applications are:
* *wifi*, stored in `01-wifi` directory: an ESP-IDF application that demonstrates how to connect to a Wi-Fi Access Point
* *pc_udp_rx*, stored in `02-pc_app_rx` directory: a PC Python application that waits for UDP messages and displays them
* *udp_tx*, stored in `03-udp_tx` directory: an ESP-IDF application that sends UDP messages to *pc_udp_rx*
* *udp_txrx*, stored in `04-udp_txrx` directory: an ESP-IDF application that sends and receives messages
* *pc_udp_rxtx*, stored in `05-pc_udp_rxtx` directory: a PC Python application that sends and receives UDP messages

There are two takeaways in the sample ESP-IDF applications:
* how to handle a connection to the Internet via a Wi-Fi access point, in a way that can easily be adapted to "rainy days", as Espressif says in [their documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/wifi.html#event-handling)
* the use of a design pattern relying on non-blocking inter-task communication (see [this article](https://www.monblocnotes.org/node/1906) for a little bit more information)

<a name="target"></a>

# Target

The target for the ESP-IDF applications is an [ESP32-DevKitC](https://www.espressif.com/en/products/devkits/esp32-devkitc/overview) with an ESP32-WROVER-B module.

ESP-IDF release is `4.2.1`.

The target for the PC applications is a PC with Python 3.

<a name="developmentEnvironment"></a>

# Development environment for ESP-IDF applications

This [short tutorial](https://github.com/PascalBod/lm20.1-esp32-eclipse) explains how to set up a virtual machine configured for ESP32 software development with Eclipse.

<a name="referenceDocuments"></a>

# Reference documents

* [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/v4.2.1/esp32/index.html)
* [Wi-Fi Driver](https://docs.espressif.com/projects/esp-idf/en/v4.2.1/esp32/api-guides/wifi.html)
* [General Notes About ESP-IDF Programming / Application startup](https://docs.espressif.com/projects/esp-idf/en/v4.2.1/esp32/api-guides/general-notes.html#application-startup)

<a name="configuration"></a>

# Configuration

To configure an ESP-IDF application, run `idf.py menuconfig`, and select **Espidf-udp Configuration**. Depending on the application, some or all of the following parameters have to be set:
* **WiFi SSID**: the SSID of the access point to be used
* **WiFi Password**: associated password
* **Retry Period, in ms**: period between two successive connection attempts, after the connection has been lost
* **IPV4 Address**: address of the host where to send datagrams
* **Port**: host port 

<a name="buildAndFlash"></a>

# Building and flashing

## From command prompt
 
To build, flash and monitor the output from an ESP-IDF application, run:

```
idf.py -p <port> flash monitor
```

Replace `<port>` by the name of the serial-over-USB port.

To exit the serial monitor, type ``Ctrl-]``.

## From Eclipse

Check the end of this [short tutorial](https://github.com/PascalBod/lm20.1-esp32-eclipse).

<a name="license"></a>

# License

Espidf-udp is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version. Check the `COPYING` file for 
more information.

