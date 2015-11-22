/*
	VCD Haven Code Version 4.0.0
	Copyright (C) 1995 Angelo Brigante Jr. & Gordon Chan
	Copyright (C) 1996, 1997, 1998, 1999 Angelo Brigante Jr.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	Send comments and bug reports to:  ang@up-above-it.org
*/  


/*
 * This is socket.c of vcd.  This handles the socket interface to make the
 * program into a server.
 */

/* system headers */
#include <fcntl.h>		/* For fcntl(), F_SETFL, O_NONBLOCK. */
#include <memory.h>		/* For memset(). */
#include <netdb.h>		/* For struct hostent. */
/* netinet/in.h has to be before arpa/inet.h */
#include <netinet/in.h>		/* For htonl(), htons(), struct in_addr,
				 * struct sockaddr_in. */
#include <arpa/inet.h>		/* For inet_ntoa(). */
#include <stdio.h>		/* For sprintf(). */
#include <stdlib.h>		/* For exit(). */
#include <string.h>		/* For strncpy(). */
#include <sys/socket.h>		/* For setsockopt(), getsockopt(),
				 * SOCK_STREAM, struct linger, SO_* constants */
#include <sys/time.h>
#include <sys/types.h>
#include <sys/utsname.h>	/* For uname() and struct utsname. */
#include <unistd.h>		/* For close(). */
#include "vcd.h"


/* local variables */
fd_set		master, master_copy;
static int	fdsize   = sizeof(fd_set);

/*
 * Initializes a socket that will listen for incoming connection on servport.
 */
int
initsocket(servport)
	unsigned short int           servport;
{
	int             servsock, onoff = 1;
	struct utsname  thishost;
	struct hostent *presenthost;
	struct sockaddr_in myhost;
#ifdef USE_LINGER
	struct linger   lt;
#endif

	/* find out what the name of the localhost is */
	if (uname((struct utsname *) & thishost) == -1) {
		logerr("error with uname()", SYSLOG);
		exit(0);
	}

	/* do a DNS lookup of the localhost */
	presenthost = gethostbyname(thishost.nodename);
	if (presenthost == (struct hostent *) 0) {
		logerr("error with gethostbyname", SYSLOG);
		exit(0);
	}

	/* set the the myhost structure (going to be used by bind) */
	memset(&myhost, 0, sizeof(myhost));
	myhost.sin_family = presenthost->h_addrtype;
	myhost.sin_port = htons(servport);
	/*
	 * INADDR_ANY makes it that such that any ip's the localhost
	 * possesses can be use for remote incoming client connections.
	 */
	myhost.sin_addr.s_addr = htonl(INADDR_ANY);

	/* open up a tcp/ip type socket */
	servsock = socket(PF_INET, SOCK_STREAM, 0);
	if (servsock == -1) {
		logerr("error with socket", SYSLOG);
		exit(0);
	}

	/* set the socket such that the address can be used again */
	if (setsockopt(servsock, SOL_SOCKET, SO_REUSEADDR, (char *) &onoff,
		       sizeof(onoff)) == -1) {
		logerr("error with setsockopt SO_REUSEADDR", SYSLOG);
		exit(0);
	}

#ifdef USE_KEEPALIVE
	/* enable the socket for periodic transmission */
	if (setsockopt(servsock, SOL_SOCKET, SO_KEEPALIVE, (char *) &onoff,
		       sizeof(onoff)) == -1) {
		logerr("error with setsockopt SO_KEEPALIVE", SYSLOG);
		exit(0);
	}
#endif

#ifdef USE_LINGER
	/* set the socket to linger on closing */
	lt.l_onoff = 1;
	lt.l_linger = 30;
	if (setsockopt(servsock, SOL_SOCKET, SO_LINGER, (char *) &lt,
		       sizeof(lt)) == -1) {
		logerr("error with setsockopt SO_LINGER", SYSLOG);
		exit(0);
	}
#endif

	/* make the server's socket non-blocking */
	if (fcntl(servsock, F_SETFL, O_NONBLOCK) == -1) {
		logerr("error with fcntl nonblock", SYSLOG);
		exit(0);
	}

	/* bind the socket to the host on a port */
	if (bind(servsock, (struct sockaddr *)&myhost, sizeof(myhost)) == -1) {
		 logerr("error with bind", SYSLOG);
		 exit(0);
	}

	/* make the socket passive such that it will listen for connections */
	if (listen(servsock, 5) == -1) {
		logerr("error with listen", SYSLOG);
		exit(0);
	}

	return servsock;
}

/*
 * Log in a new user and resolve their ip to a hostname.  Return back their
 * file descriptor and hostname.
 */
int
newuser(servdesc, hostname, port)
	int             servdesc;
	char           *hostname;
	long           *port;
{
	int                newsock, length, val;
	struct sockaddr_in theirhost;
	struct hostent 	   *theirname = (struct hostent *)0;

	length = sizeof(struct sockaddr_in);
	/*
	 * getsockname returns the real size of length for the socket the
	 * server is listening on and as well as the sockaddr_in info of the
	 * localhost.
	 */
	if (getsockname(servdesc, (struct sockaddr *) & theirhost,
			&length) == -1) {
		logerr("error with getsockname", SYSLOG);
		return -1;
	}
	/* accept() brings in an incoming connection */
	newsock = accept(servdesc, (struct sockaddr *) & theirhost, &length);
	if (newsock == -1) {
	     logerr("error with accept", SYSLOG);
	     return -1;
	}
	/*
	 * Check how many users's we have logged on.  If numlogins is at the
	 * MAXLOGINS-1, that means somebody is logging in on the last file
	 * descriptor.  Send them a message saying we're full and log them
	 * off.
	 */
	if (numlogins >= MAXLOGINS - 1) {
		writedesc(newsock, ">> We Full.  Try Again Later.\r\n");
		close(newsock);
		return -1;
	}

	/* make the new connect's socket non-blocking */
	if (fcntl(newsock, F_SETFL, O_NONBLOCK) == -1) {
		logerr("error with fcntl nonblock", SYSLOG);
		loginerror(newsock);
		return -1;
	}
	*port = htons(theirhost.sin_port);

	val = MAXSOCKBUFFER;
	if (setsockopt(newsock, SOL_SOCKET, SO_RCVBUF, (char *) &val,
		       sizeof(int)) == -1) {
		logerr("error with setsockopt() SO_RCVBUF", SYSLOG);
	}

	if (setsockopt(newsock, SOL_SOCKET, SO_SNDBUF, (char *) &val,
		       sizeof(int)) == -1) {
		logerr("error with setsockopt() SO_SNDBUF", SYSLOG);
	}

	if (not_system(ASYNCHRODNS))
	    /* resolve the ip into a hostname */
	    theirname = gethostbyaddr((char *) &theirhost.sin_addr, 
				sizeof(struct in_addr), AF_INET);

	if (theirname == (struct hostent *)0)
		strncpy(hostname, inet_ntoa(theirhost.sin_addr), HOSTMAX);
	else
		strncpy(hostname, theirname->h_name, HOSTMAX);

	hostname[HOSTMAX] = '\0';
	return newsock;
}


/*
 * Prints a message to the new user which could not be logged in due to
 * internal errors and logs them out.
 */
void
loginerror(line)
	int             line;
{
	writedesc(line, "-- Internal Error.  You could not be allocated.\r\n");
	writedesc(line, "-- Try again later.\r\n");
	close(line);
}


int
haven_select(numdescs)
	int	numdescs;
{
	memcpy(&master_copy, &master, fdsize);
	return select(numdescs, &master_copy, (fd_set *)0, (fd_set *)0, 0);
}


void
fd_master_set(fd)
	int fd;
{
	FD_SET(fd, &master);
}


int
fd_ready(fd)
	int fd;
{
	return FD_ISSET(fd, &master_copy);
}


int
fd_master_isset(fd)
	int fd;
{
	return FD_ISSET(fd, &master);
}


void
fd_master_zero()
{
	FD_ZERO(&master);
}


void
fd_master_clear(fd)
	int fd;
{
	FD_CLR(fd, &master);
}
