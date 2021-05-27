# Table of contents

* [Overview](#overview)
* [Architecture](#architecture)
  * [Tasks](#tasks)
  * [Message protocol](#messageProtocol)
  * [Application tasks](#applicationTasks)
    * [connect_wifi](#connectWifi)
    * [supervisor](#supervisor)
  * [Overview of messages and tasks](#overviewOfMessagesAndTasks)

<a name="overview"></a>

# Overview

This application initiates a connection to a Wi-Fi access point. 

When the connection with the Wi-Fi access point is lost, the application tries to reconnect.

<a name="architecture"></a>

# Architecture

<a name="tasks"></a>

## Tasks

The application is built by composing *tasks*. A task is implemented as a FreeRTOS task, and has the following characteristics:
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

<a name="messageProtocol"></a>

## Message protocol

It's up to every task to define its message protocol, namely the list of messages it accepts, and how it reacts to each of them. The possible reactions are defined as transitions from one state to another one, depending on the messages, and as actions that are performed when entering a new state.

<a name="applicationTasks"></a>

## Application tasks

The application is made of two tasks:
* *connect_wifi*
* *supervisor*

<a name="connectWifi"></a>

### connect_wifi

The connect_wifi task is in charge of maintaining a connection to the Internet via a given Wi-Fi access point, and sending UDP datagrams to a remote host.

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

<a name="supervisor"></a>

### supervisor

The supervisor task starts the connect_wifi task and then sends the connect messages to it.

When it receives an internal_error message, it reacts depending on the origin of the error.

<a name="overviewOfMessagesAndTasks"></a>

## Overview of messages and tasks

The following diagram presents the messages that can be exchanged between the tasks:

![](tasks.svg)
