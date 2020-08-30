/***************************************************************************
*                        UDP/IP Echo Client Example
*
*   File    : echoclient_udp.c
*   Purpose : This file provides Berkeley sockets implementation of a UDP/IP
*             echo client.  It evolved from some sockets base code that I
*             wrote back in 1990.  I find myself referencing it every few
*             years, so I formalized it and put it in a location that's
*             easy to find.
*   Author  : Michael Dipperstein
*   Date    : December 30, 2017
*
****************************************************************************
*
* Echo Server: A Berkeley socket implementation of a UDP/IP Echo Client
* Copyright (C) 2017 by Michael Dipperstein (mdipperstein@gmail.com)
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
***************************************************************************/

/***************************************************************************
*                             INCLUDED FILES
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <poll.h>

#include <netdb.h>

/***************************************************************************
*                                CONSTANTS
***************************************************************************/
#define BUF_SIZE    1024        /* size of send/receive buffer */

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/
int DoEchoClient(const int socketFd, const struct sockaddr_in *serverAddr);

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/***************************************************************************
*   Function   : main
*   Description: This is the main function for this program, it opens a
*                datagram (UDP) socket to communicate withe the host
*                specified in argv[1] on the port specified in argv[2].  Then
*                it transmits the user entered messages and receives the echos
*                until the user tries to send an empty message.
*   Parameters : argc - number of parameters
*                argv - parameter list (argv[1] is host name, argv[2] is port)
*   Effects    : A connection to is established with the echo server and
*                messages are transmitted and received.
*   Returned   : 0 for success, otherwise exits with EXIT_FAILURE.
***************************************************************************/
int main(int argc, char **argv)
{
    int result;
    int socketFd;               /* UDP socket descriptor */

    /* structures for use with getaddrinfo() */
    struct addrinfo hints;      /* hints for getaddrinfo() */
    struct addrinfo *info;      /* list of info returned by getaddrinfo() */
    struct addrinfo *p;         /* pointer for iterating list in info */

    struct sockaddr_in serverAddr;  /* the address of the server for sendto */

    /* argv[1] is host name or address, argv[2] is port number,
     * make sure we have them */
    if (argc != 3)
    {
        fprintf(stderr,
            "Usage:  %s <server hostname or address> <port number>\n",
            argv[0]);
        exit(EXIT_FAILURE);
    }

    /* indicate that we want ipv4 udp datagrams */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;          /* internet address family */
    hints.ai_socktype = SOCK_DGRAM;     /* datagram sock */
    hints.ai_protocol = IPPROTO_UDP;    /* udp/ip protocol */

    /* get a linked list of likely servers pointed to by info */
    result = getaddrinfo(argv[1], argv[2], &hints, &info);

    if (result != 0)
    {
        fprintf(stderr, "Error getting addrinfo: %s\n", gai_strerror(result));
        exit(EXIT_FAILURE);
    }

    printf("Trying %s...\n", argv[1]);
    p = info;

    while (p != NULL)
    {
        /* use current info to create a socket */
        socketFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        if (socketFd >= 0)
        {
            /* we succeeded in getting a socket get out of this loop */
            break;
        }

        p = p->ai_next;     /* try next address */
    }

    if (NULL == p)
    {
        /* we never found a server to connect to */
        fprintf(stderr, "Unable to connect to server\n.");
        freeaddrinfo(info);
        exit(EXIT_FAILURE);
    }

    /* make a copy of the server address, so we can send messages to it */
    memset(&serverAddr, 0, sizeof(serverAddr));
    memcpy(&serverAddr, p->ai_addr, p->ai_addrlen);
    freeaddrinfo(info);

    /* send and receive echo messages until user sends empty message */
    DoEchoClient(socketFd, &serverAddr);

    close(socketFd);
    return 0;
}

/***************************************************************************
*   Function   : DoEchoClient
*   Description: This routine gets a messages from stdin then writes them
*                server's socket.  It also receives any messages from  the
*                server's socket.  It will exit when an empty message is
*                received from stdin, or an error occurs.
*                stdin, this routine will exit without transmitting it.
*   Parameters : socketFd - The socket descriptor for the socket to be read
*                from and echoed to.  It must be bound to the server
*                address.
*                serverAddr - pointer to the Internet address struct for
*                the echo server.
*   Effects    : stdin is read for messages, which are sent to socketFd.
*                Echoed messages from socketFd are read and written to
*                stdout.
*   Returned   : 0 for successful operation, otherwise the error from recv,
*               sendto, or fgets will be returned.
***************************************************************************/
int DoEchoClient(const int socketFd, const struct sockaddr_in *serverAddr)
{
    int result;
    char buffer[BUF_SIZE + 1];  /* stores received message */
    struct pollfd pfds[2];      /* poll for socket recv and stdin */

    pfds[0].fd = socketFd;
    pfds[0].events = POLLIN;

    pfds[1].fd = STDIN_FILENO;
    pfds[1].events = POLLIN;

    /* get message line from the user */
    printf("Enter message to send [empty message exits]:\n");

    while (1)
    {
        /* block with poll until user input or socket receive data */
        poll(pfds, 2, -1);

        /* check for recv on socket */
        if (pfds[0].revents & POLLIN)
        {
            /* get the server's reply (recv actually accepts all replies) */
            result = recv(socketFd, buffer, BUF_SIZE, 0);

            if (result < 0)
            {
                /* receiver error, print error message and exit */
                perror("Error receiving echo");
                break;
            }
            else
            {
                printf("Received: %s", buffer);
            }
        }

        /* check for message to transmit */
        if (pfds[1].revents & POLLIN)
        {
            if (NULL == fgets(buffer, BUF_SIZE, stdin))
            {
                /* error, print error message, get error code, and exit */
                perror("Error reading user input");
                result = ferror(stdin);
                break;
            }
            else if (strlen(buffer) == 1)
            {
                /* this is a carriage return.  make it truely empty */
                buffer[0] = '\0';
            }

            /* send the message line to the server */
            result = sendto(socketFd, buffer, strlen(buffer), 0,
                (const struct sockaddr *)serverAddr,
                sizeof(struct sockaddr_in));

            if (result < 0)
            {
                /* error, print error message and exit */
                perror("Error sending message to server");
                break;
            }

            if (strlen(buffer) == 0)
            {
                /* exit on empty message */
                result = 0;
                break;
            }
            else
            {
                /* get next message line from the user */
                printf("Enter message to send [empty message exits]:\n");
            }
        }
    }

    return result;
}
