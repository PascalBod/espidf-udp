# Table of contents

* [Overview](#overview)
* [Target](#target)
* [Development environment for ESP-IDF applications](#developmentEnvironmentESPIDF)
* [Reference documents](#referenceDocuments)
* [Configuration](#configuration)
* [Building and flashing](#buildingAndFlashing)
  * [From command prompt](#fromCommandPrompt)
  * [From Eclipse](#fromEclipse)
* [Applications architecture](#applicationsArchitecture)
  * [Overview](#aaOverview)
  * [Task](#aaTask)
  * [Message protocol](#aaMessageProtocol)
* [License](#license)

<a name="overview"></a>

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

<a name="target"></a>

# Target

The target for the ESP-IDF applications is an [ESP32-DevKitC](https://www.espressif.com/en/products/devkits/esp32-devkitc/overview) with an ESP32-WROVER-B module. Any equivalent module can be used.

ESP-IDF release is `4.2.1`.

The target for the PC applications is a PC with Python 3.

The PC and the ESP32 module must be connected to the same LAN. This is the case if the two of them connect to your home Wi-Fi Access Point, for instance.

<a name="developmentEnvironmentESPIDF"></a>

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
* **Destination Port**: host port where to send datagrams
* **Reception port**: local port waiting for datagrams

The configuration can be modified from Eclipse as well (check the [documentation](https://github.com/espressif/idf-eclipse-plugin)).

<a name="buildingAndFlashing"></a>

# Building and flashing

<a name="fromCommandPrompt"></a>

## From command prompt
 
To build, flash and monitor the output from an ESP-IDF application, run:

```
idf.py -p <port> flash monitor
```

Replace `<port>` by the name of the serial-over-USB port.

To exit the serial monitor, type ``Ctrl-]``.

<a name="fromEclipse"></a>

## From Eclipse

Check the end of this [short tutorial](https://github.com/PascalBod/lm20.1-esp32-eclipse).

<a name="applicationsArchitecture"></a>

# Applications architecture

<a name="aaOverview"></a>

## Overview

Every ESP-IDF application is built by composing *tasks* that exchange *messages*.

<a name="aaTask"></a>

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

<a name="aaMessageProtocol"></a>

## Message protocol

It's up to every task to define its message protocol, namely the list of messages it accepts, and how it reacts to each of them. The possible reactions are defined as transitions from one state to another one, depending on the messages, and as actions that are performed when entering a new state.

<a name="license"></a>

# License

Espidf-udp is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version. Check the `COPYING` file for 
more information.

