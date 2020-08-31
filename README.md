# sockets - Berkeley Socket Examples

## Description
This archive contains C source code sample implementations of clients and
servers using Berkeley Socket.  Most of the source code evolved from code that
I originally wrote back in 1990.  Every few years I was using it as a reference
and updating it to replace obsolete functions with modern functions.  I've
placed the resulting code on github so that I could find it and access it from
just about anywhere.

These files are released under the BSD License.

More information on run length encoding may be found at:
https://michaeldipperstein.github.io/sockets.html

## Files
File Name | Contents
--- | ---
echoclient.c | TCP/IP echo client example using `getaddrinfo()` to find the server address
echoserver.c | TCP/IP echo server example with `poll()` loop
echoclient_udp.c | UDP/IP echo client example using `getaddrinfo()` to find the server address
echoserver_udp.c | UDP/IP echo server example
Makefile | makefile for this project (assumes gcc compiler and GNU make)
README.MD | This file

## Building
To build these files with GNU make and gcc:
1. Open a terminal
2. Change directory to the directory containing this archive
3. Enter the command "make" from the command line.

## Usage
**NOTE:** It is not possible to mix and match TCP and UDP client/server pairs.

### echoserver or echoserver_udp
echoserver &lt;port number&gt;

echoserver_udp &lt;port number&gt;

The `echoserver` will not exit until `CTRL-c` is pressed.

### echoclient or echoclient_udp
echoclient &lt;server hostname or address&gt; &lt;port number&gt;

echoclient_udp &lt;server hostname or address&gt; &lt;port number&gt;

Hit `Enter` on a blank line to exit from an `echoclient`.

Multiple `echoclient`s may connect to a single `echoserver` instance.

## History
12/27/17
* Initial release

12/30/17
* Added UDP client and server examples

08/08/20
* Added polling and signal handling to allow UDP examples to exit cleanly when
waiting to receive data

08/18/20
* Rewrote TCP code so that the `echoserver` will echo a message will be
echoed to every connected `echoclient`.
* TCP `echoserver` now actively tracks all connected `echoclient` sockets.
* Replaced `select()` loop with `poll()` loop in TCP code.

08/30/20
* Rewrote UDP code so that the `echoserver` will echo a message will be
echoed to every connected `echoclient`.
* UDP `echoserver` now actively tracks all `echoclient` socket addresses.
  * Address is obtained when client sends its first message.
  * Address is released when the client sends and empty message.
* Replaced `select()` loop with `poll()` loop in USP code.


## TODO
- Send and receive messages of any size
- Apply timeout to client connect attempts
- Add support for IPv6

## BUGS
- No known bugs

## AUTHOR
Michael Dipperstein (mdipperstein@gmail.com)
