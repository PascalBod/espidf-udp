# Table of contents

* [Overview](#overview)
* [Architecture](#architecture)
  * [Application tasks](#applicationTasks)
    * [connect_wifi](#connectWifi)
    * [send_datagram](#sendDatagram)
    * [rec_datagram](#recDatagram)
    * [supervisor](#supervisor)
  * [Overview of messages and tasks](#overviewOfMessagesAndTasks)

<a name="overview"></a>

# Overview

This application initiates a connection to a Wi-Fi access point. Once the connection 
is established, it sends UDP datagrams to a remote IP address and port, on a periodic basis.
Additionally, it waits for incoming UDP datagrams. When one is received, a log message
displays its contents and the IP address of the node that sent it.

When the connection with the Wi-Fi access point is lost, the application tries to reconnect.

<a name="architecture"></a>

# Architecture

<a name="applicationTasks"></a>

## Application tasks

The application is made of four tasks:
* *connect_wifi*
* *send_datagram*
* *rec_datagram*
* *supervisor*

<a name="connectWifi"></a>

### connect_wifi

The connect_wifi task is in charge of maintaining a connection to the Internet via a given Wi-Fi access point, and sending UDP datagrams to a remote host.

It accepts the following messages:
* *connect* - payload: the Wi-Fi access point to use
* *disconnect* - payload: none
* *send_datagram* - payload: a pointer to a byte array (datagram payload)

It generates the following messages:
* *connection_status* - payload: connection status - generated on a connection state change - sent to the send_datagram task
* *send_error* - payload: send error - generated on a send error - sent to the send_datagram task
* *internal_error* - payload: internal error - generated on an internal error - sent to the supervisor task

After having received the connect message, the task tries to connect to the designated Wi-Fi access point, and to get an IP address. Once this is done, it sends the connection_status message to the send_datagram task.

If the access to the Internet is lost, the task sends a connection_status message to the send_datagram task, tries to reconnect on a periodic basis and, if it succeeds, sends another connection_status message to the send_datagram task.

The connect_wifi task contains following states and transitions:
![](connect_wifi.svg)

Several transitions are not present in the diagram, in order to keep it simple:
* transitions leading to the error state
* transitions going from a task to itself, corresponding to ignored unexpected messages

The send_datagram message is accepted only when the task is in wait_msg_disconnect state.

<a name="sendDatagram"></a>

### send_datagram

The send_datagram task requests the transmission of a datagram to the remote host, on a periodic basis.

It accepts the following messages:
* *connection_status* - see connect_wifi task
* *send_error* - see connect_wifi task

It generates the following messages:
* *send_datagram* - see connect_wifi task
* *internal_error* - payload: internal error - generated on an internal error - sent to the supervisor task

After initialization, the task waits for a connection_status message informing it that access to the Internet is available. Upon reception of this message, it sends the send_datagram message to the connect_wifi task. Then, on a periodic basis, it sends another send_datagram message, until it receives a connection_status message saying that the access to the Internet is lost.

<a name="recDatagram"></a>

### rec_datagram

The rec_datagram task waits for incoming UDP datagrams. When one is received, a log message displays
its contents and the IP address of the host that sent it.

It accepts no message.

It generates the following messages:
* *internal_error* - payload: internal error - generated on an internal error - sent to the supervisor task

<a name="supervisor"></a>

### supervisor

The supervisor task starts the connect_wifi task and then sends the connect messages to it.

When it receives an internal_error message, it reacts depending on the origin of the error.

<a name="overviewOfMessagesAndTasks"></a>

## Overview of messages and tasks

The following diagram presents the messages that can be exchanged between the tasks:

![](tasks.svg)
