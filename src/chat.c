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
 * This is chat.c of vcd.  This contains the main loop of the server and
 * various functions that login and logout users.
 */

/* system headers */
#include <memory.h>		/* For memcpy(). */
#include <stdio.h>		/* For sprintf(). */
#include <string.h>		/* For strncpy(). */
#include <sys/time.h>		/* For FD_* types. */
#include <sys/types.h>		/* For type fd_set, FD_CLR(), FD_SET(),
				 * FD_ZERO(). */
#include <time.h>		/* For type time_t. */
#include <unistd.h>		/* For close(), _SC_OPEN_MAX, sysconf(). */
#include "vcd.h"

/* local variables */
int           	SERVERSHUTDOWN;

/* global variables */
long            idletimeout;
time_t          boottime, nextcheck, systemtime;


void
doit(servdesc)
	int             servdesc;
{
	int             num, max_haven_fds;
	struct user    *uptr;
	char            inputbuffer[INPUTMAX];
	struct module	*m;

	
	/* Get maximum number of fds the haven can use and log it */
	max_haven_fds = sysconf(_SC_OPEN_MAX);
        sprintf(errbuff, "max haven fds = %d", max_haven_fds);
        log(errbuff, SYSLOG);

	/* Put the server's file descriptor onto the master list. */
	fd_master_zero();
	fd_master_set(servdesc);

	SERVERSHUTDOWN = 0;
	idletimeout = DEFAULTIDLE;

	/* Keep track of the boot time. */
	boottime = time((time_t *) 0);
	nextcheck = boottime + idletimeout;


	/* Softboot */
	if ( is_system(SOFTBOOT) ) {
	    numlogins = 0;
	    loadmain();  /* Needed again for idletimeout and boottime */
	    loadchannels();
	    loadusers();	
	    loadmods();
	    loadpstr();
	    clr_system(SOFTBOOT);
	    log("SOFTBOOT SUCCESSFUL", SYSLOG);
	} else {
	    initmain(DEFAULTMAIN);
	    initmodules();
	}

	/* This is the main loop. */
	while (!SERVERSHUTDOWN) {

		/* Update the system time. */
		systemtime = time((time_t *) 0);

		/* asychronous I/O */
		num = haven_select(max_haven_fds);

		if (num == -1) {
		    if (errno == EINTR) {
		        logerr("select interrupted by signal (EINTR)", SYSLOG);
			continue;
		    }
		    logerr("error with select (read)", SYSLOG);
		    continue;
		    /* keep on doin' it until select gets it right */
		} else if (!num && is_system(TIMEOUTS)) {
		    nextcheck = checkidle();
		    continue;
		}


                /* Service modules */
                for (m = modhead; m; m = m->next)
                     if (fd_ready(m->fd))
                         servicemodule(m);


		/* Cycle thru linked list of users for I/O. */
		uptr = userhead;
		while (uptr) {
			/* Check if they are ready for reading. */
			if (fd_ready(uptr->line)) {
			   /*
			    * Read in some text and process it.  If
			    * readline() gave back 0, then the user will
			    * be disconnected.
			    */
                            if (readline(uptr, inputbuffer)) {
                                if (!processinput(uptr, inputbuffer)) {
                                    /* They spammed themself off. */
                                    uptr = disconnect(uptr, QUIT_NORMAL);
                                } else
                                    uptr = uptr->next;
                            } else {
                                /* They disconnected. */
                                uptr = disconnect(uptr, QUIT_DISCONNECT);
                            }
			} else
			    uptr = uptr->next;

		}	/* End of cycle thru user linked list. */

		/* Check for idle users. */
		if (is_system(TIMEOUTS) && (systemtime >= nextcheck))
		    nextcheck = checkidle(); 

		/* Look for new users logging in. */
		if (fd_ready(servdesc))
		    logonnew(servdesc);

	}	/* End of main loop. */
}


/*
 * Check for users idling and log them off if their idletime exceeds the
 * idletimeout.  Returns the time the NEXT check should be done.
 */
time_t
checkidle()
{
	struct user	*u;
	time_t		highestidle = 0;

	u = userhead;
	while (u) {
		if ((systemtime - u->idle) >= highestidle)
		    highestidle = systemtime - u->idle;
		/*
		 * If their idle time is greater than the idletimeout and
		 * they aren't an op, log them off.
		 */
		if (u->level==JOEUSER && (systemtime - u->idle) > idletimeout) {
                    writetext(u, ">> You've idled your life away.\r\n");
		    getout(u, "just idled out.\r\n");
		    u = disconnect(u, QUIT_NORMAL);
		} else
		    u = u->next;
	}

	/* Return the time the NEXT check should be done */
	return (systemtime + (idletimeout - highestidle)); 
}


/* Log in new users and set up their user structure. */
void
logonnew(servdesc)
	int             servdesc;
{
	char            host[HOSTMAX+1];
	int             newdesc;
	long            port;
	struct user    *new;

	newdesc = newuser(servdesc, host, &port);
	if (newdesc == -1)
		return;

	/* check to see if their site is banned */
	if (not_system(ASYNCHRODNS) && checkban(host)) {
	    writefile(newdesc, BANMESG, ">> Your site is BANNED! Go Away!\r\n");
	    close(newdesc);
	    return;
	}

	/* Send IP to name service module */
	if (is_system(ASYNCHRODNS)) 
	    execute_event(E_RESOLVE, itoa(newdesc), " ", host, "\n", NULL);

	/*
	 * Put them onto the linked list and initialize a structure for them.
	 */
	new = adduser();
	if (new != (struct user *) 0) {
                new->logintime = new->idle = time((time_t *) 0);
                new->line = newdesc;
                new->level = JOEUSER;
                new->port = port;
                strncpy(new->host, host, HOSTMAX);
                new->host[HOSTMAX] = '\0';
                strncpy(new->name, DEFAULTNAME, NAMEMAX);
                new->name[NAMEMAX] = '\0';
		new->pword[0] = '\0';
                new->email[0] = '\0';
		new->url[0] = '\0';
                new->flags = 0;
                new->lastp = 0;
                new->channel = channelhead;
                new->channel->count++;
                new->numlines = 0;
                new->commands = 0;
                new->yellcolor = 0;
                new->talkcolor = 0;
                new->hicolor = 7;
                new->wrap = 80;
		FD_ZERO(&new->gags);
		if (not_system(ASYNCHRODNS)) {
	            fd_master_set(newdesc);
	            writefile(new->line, INTRO, ">> No intro screen.\r\n"); 
	            greeting(new);
		}
		numlogins++;
	} else
		loginerror(newdesc);

}
