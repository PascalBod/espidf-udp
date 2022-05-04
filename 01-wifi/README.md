### Table of contents

Click on the ![](../images/tocIcon.png) icon above.

# Overview

This application initiates a connection to a Wi-Fi access point. 

When the connection with the Wi-Fi access point is lost, the application tries to reconnect.

# Architecture

## Application tasks

The application is made of two tasks:
* *connect_wifi*
* *supervisor*

### connect_wifi

The connect_wifi task is in charge of maintaining a connection to the Internet via a given Wi-Fi access point.

It accepts the following messages:
* *connect* - payload: the Wi-Fi access point to use
* *disconnect* - payload: none

It generates the following messages:
* *internal_error* - payload: internal error - generated on an internal error - sent to the supervisor task

After having received the connect message, the task tries to connect to the designated Wi-Fi access point, and to get an IP address.

If the access to the Internet is lost, the task tries to reconnect on a periodic basis.

The connect_wifi task contains following states and transitions:
![](connect_wifi.svg)

Several transitions are not present in the diagram, in order to keep it simple:
* transitions leading to the error state
* transitions going from a task to itself, corresponding to ignored unexpected messages

### supervisor

The supervisor task starts the connect_wifi task and then sends the connect messages to it.

When it receives an internal_error message, it reacts depending on the origin of the error. Currently, it simply generates a log message.

## Overview of messages and tasks

The following diagram presents the messages that can be exchanged between the tasks:

![](tasks.svg)
