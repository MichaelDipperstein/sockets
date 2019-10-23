############################################################################
# Makefile for Berkeley Socket Examples
############################################################################
# Copyright (C) 2017 by Michael Dipperstein (mdipperstein@gmail.com)
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
############################################################################
CC = gcc
LD = gcc
CFLAGS = -O3 -Wall -Wextra -pedantic -c
LDFLAGS = -O3 -o

PROGS = echoserver echoclient echoserver_udp echoclient_udp

all:		$(PROGS)

echoserver:	echoserver.o
		$(LD) $< $(LDFLAGS) $@

echoserver.o:	echoserver.c
		$(CC) $(CFLAGS) $<

echoclient:	echoclient.o
		$(LD) $< $(LDFLAGS) $@

echoclient.o:	echoclient.c
		$(CC) $(CFLAGS) $<

echoserver_udp:	echoserver_udp.o
		$(LD) $< $(LDFLAGS) $@

echoserver_udp.o:	echoserver_udp.c
		$(CC) $(CFLAGS) $<

echoclient_udp:	echoclient_udp.o
		$(LD) $< $(LDFLAGS) $@

echoclient_udp.o:	echoclient_udp.c
		$(CC) $(CFLAGS) $<


clean:
		rm -f *.o
		rm -f $(PROGS)
