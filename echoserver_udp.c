/***************************************************************************
*                        UDP/IP Echo Server Example
*
*   File    : echoserver_udp.c
*   Purpose : This file provides Berkeley sockets implementation of a UDP/IP
*             echo server.  It evolved from some sockets base code that I
*             wrote back in 1990.  I find myself referencing it every few
*             years, so I formalized it and put it in a location that's
*             easy to find.
*   Author  : Michael Dipperstein
*   Date    : December 30, 2017
*
****************************************************************************
*
* Echo Server: A Berkeley socket implementation of a UDP/IP Echo Server
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

/***************************************************************************
*                                CONSTANTS
***************************************************************************/
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
*   Description: This is the main function for this program, it opens a
*                datagram (UDP) socket on the port specified in argv[1].
*                It binds to the specified port and accepts data all
*                received input.  The received input is echoed back to
*                the client.
*   Parameters : argc - number of parameters
*                argv - parameter list (argv[1] is port number)
*   Effects    : A socket is open and accepts all data on the specified
*                port.
*   Returned   : This function should never return
***************************************************************************/
int main(int argc, char *argv[])
{
    int result;
    int serverFD;           /* server's socket descriptor */

    /* structures for server and client internet addresses */
    struct sockaddr_in serverAddr;

    unsigned int addrLen;       /* size of struct sockaddr_in */

    /* argv[1] is port number, make sure it's passed to us */
    if (argc != 2)
    {
        fprintf(stderr, "Usage:  %s <port number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* create server socket descriptor */
    serverFD = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (serverFD < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    addrLen = sizeof(serverAddr);
    memset(&serverAddr, 0, addrLen);                /* clear data structure */

    /* allow internet data from any address on port argv[1] */
    serverAddr.sin_family = AF_INET;                /* internet address family */
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* any incoming address */
    serverAddr.sin_port = htons(atoi(argv[1]));     /* port number */

    /* bind to the local address */
    result = bind(serverFD, (struct sockaddr *)&serverAddr, addrLen);

    if (result < 0)
    {
        /* bind failed */
        perror("Error binding socket");
        close(serverFD);
        exit(EXIT_FAILURE);
    }

    /* we have a good socket, echo all received packets */
    while (1)
    {
        DoEcho(serverFD);
    }

    close(serverFD);
    return EXIT_SUCCESS;    /* we should not get here */
}

/***************************************************************************
*   Function   : DoEcho
*   Description: This routine receives a packet from a UDP socket and then
*                writes the received packet back to the address that it
*                received from on the same socket.
*   Parameters : serverFD - The socket descriptor for the socket to be read
*                from and echoed to.
*   Effects    : serverFD is read from and the values read are echoed back.
*   Returned   : Typically the size of the echoed message.  Values <= 0
*                mean something went wrong.
***************************************************************************/
int DoEcho(const int serverFD)
{
    struct sockaddr_in clientAddr;      /* address that sent the packet */
    unsigned int addrLen;               /* size of struct sockaddr_in */
    char buffer[BUF_SIZE + 1];          /* stores received message */
    int result;

    addrLen = sizeof(struct sockaddr_in);
    memset(&clientAddr, 0, addrLen);    /* clear data structure */

    printf("Waiting to receive a message.\n");
    memset(&buffer, 0, BUF_SIZE);
    result = recvfrom(serverFD, buffer, BUF_SIZE, 0,
        (struct sockaddr *)&clientAddr, &addrLen);

    if (result < 0)
    {
        perror("Error receiving message");
    }
    else if (0 == result)
    {
        printf("Orderly shutdown by %s.\n",
            inet_ntoa(clientAddr.sin_addr));
    }
    else
    {
        /* we received a valid message */
        printf("Received %d bytes from %s.\n", result,
            inet_ntoa(clientAddr.sin_addr));

        result = sendto(serverFD, buffer, strlen(buffer), 0,
            (struct sockaddr *)&clientAddr, addrLen);

        if (result < 0)
        {
            perror("Error sending message");
        }
    }

    return result;
}
