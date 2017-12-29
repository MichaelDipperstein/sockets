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
echoclient.c    - TCP/IP echo client example using `getaddrinfo()` to find the
                  server address
echoserver.c    - TCP/IP echo server example with `select()` loop
Makefile        - makefile for this project (assumes gcc compiler and GNU make)
README.MD       - this file

## Building
To build these files with GNU make and gcc:
1. Open a terminal
2. Change directory to the directory containing this archive
3. Enter the command "make" from the command line.

## Usage

### echoserver

### echoclient
## History
12/27/17  - Initial release

## TODO
- Send and receive messages of any size
-

## BUGS
- No known bugs

## AUTHOR
Michael Dipperstein (mdipper@alumni.engr.ucsb.edu)
