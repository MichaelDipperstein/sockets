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
#include <errno.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <poll.h>

/***************************************************************************
*                                CONSTANTS
***************************************************************************/
#define MAX_BACKLOG 10          /* maximum outstanding connection requests */
#define BUF_SIZE    1024        /* size of receive buffer */

/***************************************************************************
*                                 TYPES
***************************************************************************/
typedef struct fd_list_t
{
    int fd;
    struct fd_list_t* next;
} fd_list_t;

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/
int DoEcho(const int clientFd, const fd_list_t *list);

int InsertFd(int fd, fd_list_t **list);
int RemoveFd(int fd, fd_list_t **list);
void PrintFdList(const fd_list_t *list);

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/***************************************************************************
*   Function   : main
*   Description: This is the main function for this program, it opens a TCP
*                socket on the port specified in argv[1].  It accepts all
*                connections and maintains connections to the specified
*                port.  If the connected socket may be read, DoEcho is
*                called to handle receiving and echoing data.
*   Parameters : argc - number of parameters
*                argv - parameter list (argv[1] is port number)
*   Effects    : A socket is open and accepts connections on the specified
*                port.  Readable connections are passed to DoEcho
*   Returned   : This function should never return
*
*   TODO: Add signalfd to handle ctrl-c and exit cleanly.
***************************************************************************/
int main(int argc, char *argv[])
{
    int result;
    int listenFd;   /* socket fd used to listen for connection requests */

    struct fd_list_t* fdList, *thisFd;
    struct pollfd *pfds;
    int numFds, changed;

    /* structures for server and client internet addresses */
    struct sockaddr_in serverAddr;

    int i;

    /* argv[1] is port number, make sure it's passed to us */
    if (argc != 2)
    {
        fprintf(stderr, "Usage:  %s <port number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fdList = NULL;

    /* create server socket descriptor */
    listenFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (listenFd < 0)
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
    result = bind(listenFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    if (result < 0)
    {
        /* bind failed */
        perror("Error binding socket");
        close(listenFd);
        exit(EXIT_FAILURE);
    }

    /* listen for incoming connections */
    result = listen(listenFd, MAX_BACKLOG);

    if (result < 0)
    {
        /* listen failed */
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }

    /* listenFd is our only fd when we start */
    numFds = 1;
    changed = 1;
    pfds = NULL;

    /* service all sockets as needed */
    while (1)
    {
        int startingFds = numFds;

        if (changed)
        {
            /* one or more fds changed.  we need to change the polling struct */

            if (NULL != pfds)
            {
                free(pfds);
            }

            pfds = (struct pollfd *)malloc(numFds * sizeof(struct pollfd));

            if (NULL == pfds)
            {
                perror("Error allocating fds for poll");
                break;
            }

            /* listen fd goes in slot 0, just because */
            pfds[0].fd = listenFd;
            pfds[0].events = POLLIN;

            /* now poll for input from the rest of the sockets */
            thisFd = fdList;
            for(i = 1; i < numFds; i++)
            {
                pfds[i].fd = thisFd->fd;
                pfds[i].events = POLLIN;
                thisFd = thisFd->next;
            }

            changed = 0;
        }

        /* block on poll until something needs servicing */
        if (-1 ==  poll(pfds, numFds, -1))
        {
            perror("Error poll failed");
            exit(EXIT_FAILURE);
        }

        /* one or more fds need servicing */
        if (pfds[0].revents & POLLIN)
        {
            /* listening socket we got a new connection request */
            int acceptedFd;     /* fd for accepted connection */

            /* accept the connection; we don't care about the address */
            acceptedFd = accept(listenFd, NULL, NULL);

            if (acceptedFd < 0)
            {
                /* accept failed.  keep processing */
                perror("Error accepting connections");
            }
            else
            {
                printf("New connection on socket %d.\n", acceptedFd);
                InsertFd(acceptedFd, &fdList);
                numFds++;
                changed = 1;
            }
        }

        for(i = 1; i < startingFds; i++)
        {
            /* one or more clients needs servicing */
            if (pfds[i].revents & POLLIN)
            {
                /* service this client */
                result = DoEcho(pfds[i].fd, fdList);

                if (result <= 0)
                {
                    /* socket closed normally or failed */
                    close(pfds[i].fd);
                    RemoveFd(pfds[i].fd, &fdList);
                    numFds--;
                    changed = 1;
                }
            }
        }
    }

    close(listenFd);
    return EXIT_SUCCESS;    /* we should not get here */
}


/***************************************************************************
*   Function   : DoEcho
*   Description: This routine receives from a client's socket and then
*                writes the received message back to each connected client
*                socket.  The write is non-blocking, so clients with busy
*                sockets will not receive the message.
*   Parameters : clientFd - The socket descriptor for the socket to be read
*                from.
*                list - a pointer to a list of fds for all connected sockets.
*   Effects    : clientFd is read from.  If the read succeeds, the value
*                that was read is sent to all client sockets.  The send will
*                only succeed if the socket may be written to without
*                blocking.
*   Returned   : 0 for normal disconnect of clientFd, < 0 for failure,
*                a positive value will be returned.
***************************************************************************/
int DoEcho(const int clientFd, const fd_list_t *list)
{
    int result;
    char buffer[BUF_SIZE + 1];  /* stores received message */

    (void)list;
    result = recv(clientFd, buffer, BUF_SIZE, 0);

    if (result < 0)
    {
        /* receive failed */
        perror("Error receiving message from client");
    }
    else if (0 == result)
    {
        printf("Socket %d disconnected.\n", clientFd);
    }
    else
    {
        const fd_list_t *here;

        buffer[result] = '\0';
        printf("Socket %d received %s", clientFd, buffer);

        /*******************************************************************
        * echo the buffer to all connected sockets, skip if waiting
        * is required.  Use threads or a complex polling loop if it's
        * important that every socket receive the echo.
        *******************************************************************/
        here = list;

        while (here != NULL)
        {
            result = send(here->fd, buffer, result, MSG_DONTWAIT);

            if (result == -1)
            {
                if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
                {
                    fprintf(stderr, "Socket %d is busy\n", here->fd);
                }
                else
                {
                    /* send failed */
                    fprintf(stderr, "Error echoing message to socket %d ",
                        here->fd);
                    perror("");
                }
            }

            here = here->next;
        }

        result = 1;     /* any echoing is success for this function */
    }

    return result;
}


/***************************************************************************
*   Function   : InsertFd
*   Description: This routine will traverse a linked list of file
*                descriptors.  If it reaches the end of the list without
*                finding the file descriptor passed a as parameter, it
*                will insert a node for the new file descriptor at the end
*                of the list.
*   Parameters : fd - The socket descriptor to be inserted to the list.
*                list - a pointer to a list of fds for all connected
*                sockets.
*   Effects    : A node for the fd is added to the end of the list of fds.
*   Returned   : 0 for success, otherwise errno for the failure.
*
*   NOTE: If duplicates aren't an issue, it's faster to insert the new
*         file descriptor to the head of the linked list.
***************************************************************************/
int InsertFd(int fd, fd_list_t **list)
{
    fd_list_t *here;

    /* handle empty list */
    if (NULL == *list)
    {
        *list = (fd_list_t *)malloc(sizeof(fd_list_t));

        if (NULL == list)
        {
            perror("Error allocating fd_list_t");
            return 1;
        }

        (*list)->fd = fd;
        (*list)->next = NULL;
        return 0;
    }

    /* find the end of the list */
    here = *list;
    while(here->next != NULL)
    {
        if (here->fd == fd)
        {
            fprintf(stderr, "Tried to insert fd that already exists: %d\n", fd);
            return EEXIST;  /* is there a better errno? */
        }

        here = here->next;
    }

    /* we're at the end insert here */
    here->next = (fd_list_t *)malloc(sizeof(fd_list_t));

    if (NULL == here->next)
    {
        perror("Error allocating fd_list_t");
        return errno;
    }

    here->next->fd = fd;
    here->next->next = NULL;
    return 0;
}


/***************************************************************************
*   Function   : RemoveFd
*   Description: This routine will traverse a linked list of file
*                descriptors until it find a node for the file descriptor
*                passed a as parameter.  Then it will remove that node from
*                the list.  If the file descriptor is not found, ENOENT
*                is returned.
*   Parameters : fd - The socket descriptor to be deleted from the list.
*                list - a pointer to a list of fds for all connected
*                sockets.
*   Effects    : The node for the fd is removed from the list of fds.
*   Returned   : 0 for success, otherwise ENOENT for the failure.
***************************************************************************/
int RemoveFd(int fd, fd_list_t **list)
{
    fd_list_t *here, *prev;

    if (NULL == *list)
    {
        return 0;
    }

    here = *list;
    prev = NULL;

    while (here != NULL)
    {
        if (here->fd == fd)
        {
            /* found it */
            if (NULL == prev)
            {
                /* deleted the head */
                *list = here->next;
            }
            else
            {
                prev->next = here->next;
            }

            free(here);
            return 0;
        }

        prev = here;
        here = here->next;
    }

    return ENOENT;
}


/***************************************************************************
*   Function   : PrintFdList
*   Description: This is a debugging routine for printing all of file
*                descriptors in a file descriptor list.
*   Parameters : list - a pointer to the head of an file descriptor list.
*   Effects    : The values of all of the file descriptors in the list are
*                written to stdout.
*   Returned   : None.
***************************************************************************/
void PrintFdList(const fd_list_t *list)
{
    const fd_list_t *here;

    if (NULL == list)
    {
        printf("No fds\n");
        return;
    }

    here = list;

    printf("fds: ");

    while (here != NULL)
    {
        if (here->next != NULL)
        {
            printf("%d, ", here->fd);
        }
        else
        {
            printf("%d\n", here->fd);
        }

        here = here->next;
    }
}
