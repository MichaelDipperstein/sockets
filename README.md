# sockets - Berkeley Socket Examples

## Description
This archive contains C source code sample implementations of clients and
servers using Berkeley Socket.  Most of the source code evolved from code that
I original wrote back in 1990.  Every few years I was using it as a reference
and updating it to replace obsolete functions with modern functions.  I've
placed the resulting code on github so that I could find it and access it from
just about anywhere.

These files are released under the BSD License.

The latest revision of this program may be found at:
https://github.com/MichaelDipperstein/sockets

## Files
File Name | Contents
--- | ---
echoclient.c | TCP/IP echo client example using `getaddrinfo()` to find the server address
echoserver.c | TCP/IP echo server example with `select()` loop
Makefile     | makefile for this project (assumes gcc compiler and GNU make)
README.MD    | this file

## Building
To build these files with GNU make and gcc:
1. Open a terminal
2. Change directory to the directory containing this archive
3. Enter the command "make" from the command line.

## Usage

### echoserver
echoserver &lt;port number&gt;

The echoserver will not exit until `CTRL-c` is pressed.

### echoclient
echoclient &lt;server address&gt; &lt;port number&gt;

Hit `Enter` on a blank line to exit from an echoclient.

Multiple `echoclient`s may connect to a single `echoserver` instant.

## History
12/27/17  - Initial release

## TODO
- Send and receive messages of any size
- Apply timeout to client connect attempts
- Add UDP versions of client and server

## BUGS
- No known bugs

## AUTHOR
Michael Dipperstein (mdipper@alumni.engr.ucsb.edu)
