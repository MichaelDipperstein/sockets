/***************************************************************************
*                        TCP/IP Echo Client Example
*
*   File    : echoclient.c
*   Purpose : This file provides Berkeley sockets implementation of a TCP/IP
*             echo client.  It evolved from some sockets base code that I
*             wrote back in 1990.  I find myself referencing it every few
*             years, so I formalized it and put it in a location that's
*             easy to find.
*   Author  : Michael Dipperstein
*   Date    : December 27, 2017
*
****************************************************************************
*
* Echo Server: A Berkeley socket implementation of a TCP/IP Echo Client
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

#include <netdb.h>

#include <poll.h>

/***************************************************************************
*                                CONSTANTS
***************************************************************************/
#define BUF_SIZE    1024        /* size of receive buffer */

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/
int DoEchoClient(const int socketFd);

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/***************************************************************************
*   Function   : main
*   Description: This is the main function for this program, it opens a TCP
*                connection to the host specified in argv[1] on the port
*                specified in argv[2].  Then calls DoEchoClient to handle
*                sending and receiving messages.
*   Parameters : argc - number of parameters
*                argv - parameter list (argv[1] is host name, argv[2] is
*                port)
*   Effects    : A connection to is established with the echo server and
*                DoEchoClient is called to handle transmitting and
*                receiving messages.
*   Returned   : EXIT_SUCCESS for success, otherwise exits with
*                EXIT_FAILURE.
***************************************************************************/
int main(int argc, char **argv)
{
    int result;
    int socketFd;               /* TCP/IP socket descriptor */

    /* structures for use with getaddrinfo() */
    struct addrinfo hints;      /* hints for getaddrinfo() */
    struct addrinfo *servInfo;  /* list of info returned by getaddrinfo() */
    struct addrinfo *p;         /* pointer for iterating list in servInfo */

    /* argv[1] is host name, argv[2] is port number, make sure we have them */
    if (argc != 3)
    {
        fprintf(stderr,
            "Usage:  %s <server hostname or address> <port number>\n",
            argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));

    /* type of server we're looking for */
    hints.ai_family = AF_INET;          /* internet address family */
    hints.ai_socktype = SOCK_STREAM;    /* stream sock */
    hints.ai_protocol = IPPROTO_TCP;    /* tcp/ip protocol */
    hints.ai_flags = AI_CANONNAME;      /* include canonical name */

    /* get a linked list of likely servers pointed to by servInfo */
    result = getaddrinfo(argv[1], argv[2], &hints, &servInfo);

    if (result != 0)
    {
        fprintf(stderr, "Error getting addrinfo: %s\n", gai_strerror(result));
        exit(EXIT_FAILURE);
    }


    printf("Trying %s...\n", argv[1]);
    p = servInfo;

    while (p != NULL)
    {
        /* use current info to create a socket */
        socketFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        if (socketFd >= 0)
        {
            /***************************************************************
            * We got the socket we asked for try to connect.
            *
            * NOTE: connect() has an unspecified time out.  For a good
            * a sample of connecting with a timeout see
            * http://developerweb.net/viewtopic.php?id=3196 for
            ***************************************************************/
            result = connect(socketFd, p->ai_addr, p->ai_addrlen);

            if (result != 0)
            {
                /* this socket wouldn't except our connection */
                close(socketFd);
            }
            else
            {
                /* we're connected, get out of this loop */
                break;
            }
        }

        p = p->ai_next;     /* try next address */
    }

    if (NULL == p)
    {
        /* we never found a server to connect to */
        fprintf(stderr, "Unable to connect to server.\n");
        freeaddrinfo(servInfo);
        exit(EXIT_FAILURE);
    }

    printf("Connected to %s\n", p->ai_canonname);
    freeaddrinfo(servInfo);     /* we're done with this */

    /***********************************************************************
    * send messages to echo server and receive echos until user sends empty
    * message or the server disconnects.
    ***********************************************************************/
    DoEchoClient(socketFd);

    close(socketFd);
    return EXIT_SUCCESS;
}


/***************************************************************************
*   Function   : DoEchoClient
*   Description: This routine contains the polling loop to handle sending
*                and receiving of echo messages.  Message from stdin are
*                written to the socket passed as a parameter.  Messages
*                received from the socket passed as a parameter are written
*                to stdout.  If an empty message is received from stdin,
*                this routine will exit without transmitting it.  This
*                routine will also exit if the server side of the socket
*                is closed.
*   Parameters : socketFd - The socket descriptor for the socket to be read
*                from and written to.
*   Effects    : stdin is read for messages, which are sent to socketFd.
*                socketFd is read for messages, which are sent to stdout.
*   Returned   : 0 for empty message from stdin or closed socket.
***************************************************************************/
int DoEchoClient(const int socketFd)
{
    int result;
    char buffer[BUF_SIZE + 1];  /* stores received message */
    struct pollfd pfds[2];      /* file descriptors for polling */

    printf("Enter messages to send [empty message exits]:\n");

    /* enter a poll loop until empty message or server disconnects */
    while (1)
    {
        /* stdin for user input */
        pfds[0].fd = STDIN_FILENO;
        pfds[0].events = POLLIN;

        /* socket for input from echos */
        pfds[1].fd = socketFd;
        pfds[1].events = POLLIN;

        result = poll(pfds, 2, -1);

        if (pfds[0].revents &  POLLIN)
        {
            /* we can read the user's input to send */
            fgets(buffer, BUF_SIZE, stdin);

            if (strlen(buffer) <= 1)
            {
                /* exit on empty message */
                break;
            }

            /* send the message line to the server (write is blocking) */
            result = write(socketFd, buffer, strlen(buffer));

            if (result != (int)strlen(buffer))
            {
                perror("Error sending message to server");
            }
        }

        if (pfds[1].revents &  POLLIN)
        {
            /* get server's echo */
            result = read(socketFd, buffer, BUF_SIZE);

            if (result < 0)
            {
                perror("Error receiving echo");
            }
            else if (0 == result)
            {
                /* the server side of the */
                printf("Server closed connection.  Exiting ...\n");
                break;
            }

            buffer[result] = '\0';
            printf("Received: %s", buffer);
        }
    }

    return 0;
}
