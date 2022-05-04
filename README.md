### Table of contents

Click on the ![](images/tocIcon.png) icon above.

# Overview

This repository contains several applications that demonstrate step-by-step how to exchange messages between an ESP-IDF application and a PC application using UDP datagrams.

These applications are:
* *wifi*, stored in `01-wifi` directory: an ESP-IDF application that demonstrates how to connect to a Wi-Fi Access Point
* *pc_udp_rx*, stored in `02-pc_udp_rx` directory: a PC Python application that waits for UDP datagrams and displays them
* *udp_tx*, stored in `03-udp_tx` directory: an ESP-IDF application that sends UDP datagrams to *pc_udp_rx*
* *udp_txrx*, stored in `04-udp_txrx` directory: an ESP-IDF application that sends and receives datagrams
* *pc_udp_tx*, stored in `05-pc_udp_tx` directory: a PC Python application that sends UDP datagrams

The ESP-IDF applications demonstrates:
* how to handle a connection to the Internet via a Wi-Fi access point, in a way that can easily be adapted to "rainy days", as Espressif says in [their documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/wifi.html#event-handling)
* the use of a design pattern relying on non-blocking inter-task communication (see [this article](https://www.monblocnotes.org/node/1906) for a little bit more information)

The aim of these sample applications is not to provide a reliable communication between an ESP32 and a PC.  

# Target

## Hardware and software

The target for the ESP-IDF applications is an [ESP32-DevKitC](https://www.espressif.com/en/products/devkits/esp32-devkitc/overview) with an ESP32-WROVER-B module. Any equivalent board can be used.

ESP-IDF release is `4.2.1`, or a more recent one.

The target for the PC applications is a PC with Python 3.

[This short tutorial](https://github.com/PascalBod/lm-esp32-eclipse) describes a way to make a virtual machine (VM) configured for ESP32 software development with Eclipse, and configured with Python 3.

## Network connectivity

If you are using the VM referenced above, be sure to configure it so that it is reachable through your network:
* Select VirtualBox **Devices > Network > Network Settings...** menu
* Configure **Adapter 1** in the following way:
  * Set **Attached to** to **Bridged Adapter**
  * For **Name**, select the Wi-Fi network interface
* Click on the network icon of the VM, in the bottom right side and click on **Disconnect** and then click on **Wired connection 1**. This ensures that your VM immediately gets an IP adress in the right subnetwork

Starting from step 3, you'll have to connect the ESP32 board to the VM. At a connectivity point of view, the easiest way to do this is to ensure that the ESP32 and the VM both connect to the same Wi-Fi access point. 

In the VM, check the IP address that has been assigned to it, with the `ifconfig` command. For the ESP32, the assigned IP adress is displayed in the trace messages. They both must be in the same subnetwork.

# Configuration

To configure the ESP-IDF applications provided in steps 1, 3 and 4, double-click on `sdkconfig` in Eclipse's Project Explorer view, or run `idf.py menuconfig` in a terminal (in this case, don't forget to [configure the terminal environment as explained by ESP-IDF documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html#step-4-set-up-the-environment-variables)), and then select **Espidf-udp Configuration**. Depending on the application, some or all of the following parameters have to be set:
* **WiFi SSID**: the SSID of the access point to be used
* **WiFi Password**: associated password
* **Retry Period, in ms**: period between two successive connection attempts, after the connection has been lost
* **IPV4 Address**: address of the host where to send datagrams
* **Destination Port**: host port where to send datagrams
* **Reception port**: local port waiting for datagrams

# Building and flashing

## From command prompt
 
To build, flash and monitor the output from an ESP-IDF application, run:

```
idf.py -p <port> flash monitor
```

Replace `<port>` by the name of the serial-over-USB port.

To exit the serial monitor, type ``Ctrl-]``.

## From Eclipse

Check the end of this [short tutorial](https://github.com/PascalBod/lm-esp32-eclipse).

# Applications architecture

## Overview

Every ESP-IDF application in steps 1, 3 and 4 is built by composing *tasks* that exchange *messages*.

## Task

Every ESP-IDF application is built by composing *tasks*. A task is implemented as a FreeRTOS task, and has the following characteristics:
* the task is in one of a finite number of states at any given time
* the task can receive *messages*
* a message is sent to the task's *input queue*, a queue owned by the task
* the send operation is non blocking
* all received messages are processed by the task in order of reception (First In, First Out)
* the task changes from its current state to another one depending on each message
* after having processed a message, the task may generate one or more messages to some other task(s)
* when the task encounters an unrecoverable error, it sends a message to the supervisor task (see below) and enters an error state that it will never leave

Each task implements a finite state machine.

In order to be able to send a message to another task, a task must know the queue of the other task. In the current implementation, in order to keep it very simple, all queues are globally accessible.

A specific task, the *supervisor* task, is in charge of ensuring application consistency. It receives error messages from other tasks when these ones encounter unrecoverable errors, so that it can act on other tasks accordingly.

In these sample applications, the supervisor task is kept very simple: it does not try to act on errors.

## Message protocol

It's up to every task to define its message protocol, namely the list of messages it accepts, and how it reacts to each of them. The possible reactions are defined as transitions from one state to another one, depending on the messages, and as actions that are performed when entering a new state.

# Reference documents

* [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/index.html)
* [Wi-Fi Driver](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/wifi.html)

# License

Espidf-udp is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version. Check the `COPYING` file for 
more information.
