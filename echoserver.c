/***************************************************************************
*                        TCP/IP Echo Server Example
*
*   File    : echoserver.c
*   Purpose : This file provides Berkeley sockets implementation of a TCP/IP
*             echo server.  It evolved from some sockets base code that I
*             wrote back in 1990.  I find myself referencing it every few
*             years, so I formalized it and put it in a location that's
*             easy to find.
*   Author  : Michael Dipperstein
*   Date    : December 27, 2017
*
****************************************************************************
*
* Echo Server: A Berkeley socket implementation of a TCP/IP Echo Server
* Copyright (C) 2017 by Michael Dipperstein (mdipper@alumni.engr.ucsb.edu)
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

#include <sys/socket.h>
#include <arpa/inet.h>

#include <sys/select.h>

/***************************************************************************
*                                CONSTANTS
***************************************************************************/
#define MAX_BACKLOG 10          /* maximum outstanding connection requests */
#define BUF_SIZE    1024        /* size of receive buffer */

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/
int DoEcho(const int clientFD);

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/***************************************************************************
*   Function   : main
*   Description: This is the main function for this program, it opens a TCP
*                socket on the port specified in argv[1].  It accepts all
*                connections to the specified port and echos back all
*                received input.
*   Parameters : argc - number of parameters
*                argv - parameter list (argv[1] is port number)
*   Effects    : A socket is open and accepts connections on the specified
*                port.
*   Returned   : This function should never return
***************************************************************************/
int main(int argc, char *argv[])
{
    int result;
    int serverFD;           /* server's socket descriptor */
    int clientFD;           /* client's socket descriptor */

    /* structures for server and client internet addresses */
    struct sockaddr_in serverAddr;
    struct sockaddr_in clinetAddr;

    unsigned int addrLen;   /* size of clinetAddr */


    /* sets of file descriptors for select functions */
    fd_set fdActiveSet;     /* all active fds */
    fd_set fdReadSet;       /* working set of fds */
    int fdMax;              /* maximum file descriptor number */

    int i;

    /* argv[1] is port number, make sure it's passed to us */
    if (argc != 2)
    {
        fprintf(stderr, "Usage:  %s <port number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* create server socket descriptor */
    serverFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverFD < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));     /* clear data structure */

    /* allow internet connection from any address on port argv[1] */
    serverAddr.sin_family = AF_INET;                /* internet address family */
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* any incoming address */
    serverAddr.sin_port = htons(atoi(argv[1]));     /* port number */

    /* bind to the local address */
    result = bind(serverFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    if (result < 0)
    {
        /* bind failed */
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    /* listen for incoming connections */
    result = listen(serverFD, MAX_BACKLOG);

    if (result < 0)
    {
        /* listen failed */
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }


    FD_ZERO(&fdActiveSet);              /* clear set of fds*/
    FD_SET(serverFD, &fdActiveSet);     /* add server fd to the set */

    fdMax = serverFD;   /* serverFD is the only fd, so it's the largest */

    /* service all sockets as needed */
    while (1)
    {
        fdReadSet = fdActiveSet;

        /* block on select until something needs servicing */
        if (select(fdMax+1, &fdReadSet, NULL, NULL, NULL) == -1)
        {
            perror("Select failed");
            exit(EXIT_FAILURE);
        }

        /* one or more fds need servicing.  start with max, it may change. */
        for(i = 0; i <= fdMax; i++)
        {
            if (FD_ISSET(i, &fdReadSet))
            {
                /* this fd needs servicing.  figure out which one it is. */
                if (i == serverFD)
                {
                    /* we got a new connection */
                    addrLen = sizeof(clinetAddr);

                    clientFD = accept(serverFD,
                        (struct sockaddr *)&clinetAddr,  &addrLen);

                    if (clientFD < 0)
                    {
                        /* accept failed.  keep processing */
                        perror("Error accepting connections");
                    }
                    else
                    {
                        /* add clientFD to set of fds*/
                        FD_SET(clientFD, &fdActiveSet);

                        if (clientFD > fdMax)
                        {
                            fdMax = clientFD;
                        }

                        printf("Connected to %s on socket %d.\n",
                            inet_ntoa(clinetAddr.sin_addr), clientFD);
                    }
                }
                else
                {
                    /* one of the clients needs servicing */
                    result = DoEcho(i);

                    if (result <= 0)
                    {
                        /* socket closed normally or failed */
                        close(i);
                        FD_CLR(i, &fdActiveSet);
                    }
                }
            }   /* end loop through fds */
        }
    }

    return EXIT_SUCCESS;    /* we should not get here */
}

/***************************************************************************
*   Function   : DoEcho
*   Description: This routine receives from a client's socket and then
*                writes the received message back to the same socket.
*   Parameters : clientFD - The socket descriptor for the socket to be read
*                from and echoed to.
*   Effects    : clientFD is read from and the values read are echoed back.
*   Returned   : 0 for normal disconnect, < 0 for failure, otherwise the
*                number of bytes read from the socket.
***************************************************************************/
int DoEcho(const int clientFD)
{
    int result;
    char buffer[BUF_SIZE + 1];  /* stores received message */

    result = recv(clientFD, buffer, BUF_SIZE, 0);

    if (result < 0)
    {
        /* receive failed */
        perror("Error receiving message from client");
    }
    else if (0 == result)
    {
        printf("Disconnected from socket %d.\n", clientFD);
    }
    else
    {
        /* echo: send back buffer */
        if (result != send(clientFD, buffer, result, 0))
        {
            /* send failed */
            perror("Error echoing message");
        }
    }

    return result;
}
