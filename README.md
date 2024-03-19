# s-talk-

stalk (simple-talk)
Overview

The "s-talk" project is a simple chat-like facility enabling real-time communication between users on different terminals. It leverages a client-server architecture over UDP to facilitate this communication, with the help of pthreads for concurrent processing within each client instance.

Features

Real-time text communication between two terminals.
Support for specifying local and remote port numbers and the remote machine name.
Real-time display of typed characters on both the sender's and the receiver's terminals.
Graceful termination of the chat session upon entering a single '!' character.

Requirements

Linux operating system with support for pthreads and UDP sockets.
GCC compiler for building the project.
Installation

Clone this repository to your local machine:

bash
git clone <repository-url>

Navigate to the cloned directory and use the provided Makefile to compile the project:

bash
cd stalk
make s-talk

Usage
To initiate an s-talk session, both users must agree on the machine each will be running on and the port numbers each will use. For example, if User1 is on machine1 using port 6060 and User2 is on machine2 using port 6001, User1 would initiate the session with:

bash
./s-talk 6060 machine2 6001
And User2 would initiate the session with:

bash
./s-talk 6001 machine1 6060
Once both users have initiated the session, they can start typing messages. Every line typed at each terminal will appear on both terminals once the sender presses enter. To terminate the s-talk session, either user needs to enter '!' on a new line and press enter.

Architecture
This application consists of four main threads within each process:

A thread that listens for keyboard input.
A thread that awaits UDP datagrams.
A thread that prints characters to the screen.
A thread that sends data over the network using UDP.
These threads interact with a shared list ADT for message passing and synchronization is achieved through mutexes and conditional variables to manage concurrent access.
