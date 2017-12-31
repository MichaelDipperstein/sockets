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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/***************************************************************************
*                                CONSTANTS
***************************************************************************/
#define BUF_SIZE    1024        /* size of send/receive buffer */

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/
int DoEchoClient(const int socketFD, struct sockaddr_in *serverAddr);

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
    int socketFD;               /* UDP socket descriptor */

    struct sockaddr_in serverAddr;

    unsigned int addrLen;       /* size of struct sockaddr_in */

    /* argv[1] is host name, argv[2] is port number, make sure we have them */
    if (argc != 3)
    {
        fprintf(stderr, "Usage:  %s <host name> <port number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* create server socket descriptor */
    socketFD = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (socketFD < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    addrLen = sizeof(serverAddr);
    memset(&serverAddr, 0, addrLen);                /* clear data structure */

    /* allow internet data from any address on port argv[1] */
    serverAddr.sin_family = AF_INET;                /* internet address family */
    serverAddr.sin_port = htons(atoi(argv[2]));     /* port number */

    result = inet_aton(argv[1], &serverAddr.sin_addr);

    if (0 == result)
    {
        fprintf(stderr, "Error obtaining Internet address of %s\n", argv[1]);
        close(socketFD);
        exit(EXIT_FAILURE);
    }

    /* send and receive echo messages until user sends empty message */
    while (DoEchoClient(socketFD, &serverAddr));

    close(socketFD);
    return 0;
}

/***************************************************************************
*   Function   : DoEchoClient
*   Description: This routine gets a message from stdin then writes to the
*                server's socket, waits for a reply from the server, and
*                writes it to stdout.  If an empty message is received from
*                stdin, this routine will exit without transmitting it.
*   Parameters : socketFD - The socket descriptor for the socket to be read
*                from and echoed to.
*   Effects    : stdin is read for a messages, which is sent to socketFD
*                then the reply from socketFD is read and written to stdout.
*   Returned   : 0 for empty message from stdin, otherwise 1.
***************************************************************************/
int DoEchoClient(const int socketFD, struct sockaddr_in *serverAddr)
{
    int result;
    char buffer[BUF_SIZE + 1];  /* stores received message */
    unsigned int addrLen;       /* size of struct sockaddr_in */

    addrLen = sizeof(struct sockaddr_in);

    /* get message line from the user */
    printf("Enter message to send [empty message exits]:\n");
    memset(&buffer, 0, sizeof(char) * BUF_SIZE);
    fgets(buffer, BUF_SIZE, stdin);

    if (strlen(buffer) <= 1)
    {
        /* exit on empty message */
        return 0;
    }

    /* send the message line to the server */
    result = sendto(socketFD, buffer, strlen(buffer), 0,
        (struct sockaddr *)serverAddr, addrLen);

    if (result < 0)
    {
        perror("Error sending message to server");
        return 1;
    }

    /* print the server's reply */
    result = recvfrom(socketFD, buffer, BUF_SIZE, 0, NULL, NULL);

    if (result < 0)
    {
        perror("Error receiving echo");
        return 1;
    }

    printf("Received:\n%s", buffer);
    return 1;
}
