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
 * This is talker.c of vcd.  The functions here initialize the program for a
 * server daemon listening on a port for incoming connections.
 */

/* system headers */
#include <ctype.h>		/* For isdigit(). */
#include <signal.h>		/* For signal handling stuff. */
/*
 * The sys/time.h header file has to be defined BEFORE the sys/resource.h
 * header file.  Required for getrlimit() and setrlimit().
 */
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>	/* For setrlimit() and getrlimit(). */
#include <sys/wait.h>		/* For wait(). */
#include <stdio.h>		/* For sprintf(). */
#include <stdlib.h>		/* For atoi(), exit(). */
#include <unistd.h>		/* For close(), fork(), sysconf(). */
#include "vcd.h"

/* global variables */
int server,
    port = DEFAULTPORT,
    system_flags = 0;


/* This closes all the available file descriptors. */
void
closefiles()
{
	int	j;

	for (j = sysconf(_SC_OPEN_MAX) - 1; j >= 0; j--)
		close(j);

}


#ifdef RLIMIT_NOFILE
/* This sets the process such that all of its file descriptors can be used. */
void
setmaxfiledescs()
{       
	struct rlimit   limit;

	if (getrlimit(RLIMIT_NOFILE, &limit) < 0) {
		logerr("error with getrlimit", SYSLOG);
		return;
	}

	/*
	 * limit.rlim_cur is the CURRENT number of file descriptors and
	 * limit.rlim_max is the MAXIMUM number of file descriptors allowed.
	 */
	sprintf(errbuff, "BEFORE -> current: %d, max: %d, getd = %d",
	  (int)limit.rlim_cur, (int)limit.rlim_max, (int)sysconf(_SC_OPEN_MAX));
	log(errbuff, SYSLOG);
	limit.rlim_cur = MAXLOGINS;

	if (setrlimit(RLIMIT_NOFILE, &limit) < 0) {
		logerr("error with setrlimit", SYSLOG);
		return;
	}

	if (getrlimit(RLIMIT_NOFILE, &limit) < 0) {
		logerr("error with getrlimit", SYSLOG);
		return;
	}

	sprintf(errbuff, "AFTER -> current: %d, max: %d, getd = %d",
	  (int)limit.rlim_cur, (int)limit.rlim_max, (int)sysconf(_SC_OPEN_MAX));
	log(errbuff, SYSLOG);
}
#endif


/*
 * Wait for our zombie processes to exit.
 * For some reason FreeBSD doesn't like signal(SIGCHLD, SIG_IGN);
 */
void
zombie_handler()
{
	int	pid = wait(0);

	sprintf(errbuff, "pid of exiting child was: %d", pid);
	log(errbuff, SYSLOG); 
	log(errbuff, MODLOG);

	signal(SIGCHLD, zombie_handler);
}


/*
 * No. It's not misspelled.
 */
void
krash(sig)
	int	sig;
{
	struct	user	*u;

	signal(sig, SIG_DFL);
	
	/* Let the user's know they are about to be havenless */	
	for (u = userhead; u; u = u->next) {
	     writetext(u, "-=> Something REALLY bad just happened!" \
			  " Haven shutting down!\r\n");

	     if (user_exists(u->name)) {
		 sprintf(errbuff, "-=> Saving user %s.\r\n", u->name);
		 save_user(u, S_UPDATE, errbuff);
	     }	
	}

	kill(getpid(), sig);
}

/*
 * Signals are set such that the process will ignore certain signals that may
 * cause it to terminate.
 */
void
setsignals()
{
	signal(SIGHUP, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGURG, SIG_IGN);
	signal(SIGCHLD, zombie_handler);
	signal(SIGBUS, krash);
	signal(SIGILL, krash);
}


/* Give a usage message and exit if invalid arguments are given at run time. */
void
usage()
{
	printf("usage: vcd [port]\n");
	exit(0);
}


/*
 * main() processes the arguments given to it at run time and puts the
 * process into the background as a daemon and does various housecleaning
 * jobs before the process is set up as a server listening on a port.
 */
void
main(argc, argv)
	int             argc;
	char          **argv;
{
	int             	child;
	FILE                   *fptr; 

	log("NEW PROCESS EXECUTED", SYSLOG);

	if (argc > 3) {
	    usage();
	} else if (argc == 3 && !strcmp(argv[1], "softboot")) { 
		log("PROCESS WAS A SOFTBOOT", SYSLOG);
		loadmain();
		set_system(SOFTBOOT);
	} else if (argc == 2) {
		if (isdigit(*argv[1])) {
			port =  atoi(argv[1]);
		} else
			usage();
	} 

	/* Check for root user */
	if (!user_exists(ROOTNAME)) {
	    printf("\n\n\033[1mNO ROOT USER!!!\033[0m\n");
	    create_root();
	}

	/*
	 * Dynamically set the program so that the maxiumum number of file
	 * descriptors can be used by the process.
	 */
#ifdef RLIMIT_NOFILE
	setmaxfiledescs();
#endif 

	/* Fork process into the background as a daemon. */
	child = fork();
	switch (child) {
	   case -1:
	   	printf("process could not fork!  aborting!\n");
		exit(0);
		break;
	   case 0:
		/* Child process will get this. */
		setsid();
		log("Daemon successfully put into background", SYSLOG);
		break;
	   default:
		if ( not_system(SOFTBOOT) ) {
		    /* Parent process will get this */
		    printf("Parent process exiting.\n");
		    printf("Daemon has PID %d.\n", child);
		    printf("Process sucessfully put into the background.\n");
		    printf("Server listening on port %d.\n", port);
		}
		sprintf(errbuff, "Process has pid %d", child);
		log(errbuff, SYSLOG);

                fptr = fopen(VCDPID, "w");
		fprintf(fptr, "%d", child);
		fclose(fptr);

		exit(1);
		break;
	}

	/* Wasn't a softboot */
	if ( not_system(SOFTBOOT) ) {

	  /*
	   * Close all file descriptors since a daemon has no need for stderr,
	   * stdin and stdout.
	   */
	   closefiles();

	  /*
	   * Set the signals such that the daemon ignores signals that may
	   * cause it terminate.
	   */
	   setsignals();

	  /* Set up a socket listening on port. */
	   sprintf(errbuff, "Port set to %d", port);
	   log(errbuff, SYSLOG);
	   server = initsocket(port);
           sprintf(errbuff, "Server is listening on desc %d", server);
           log(errbuff, SYSLOG);
		
        }  /* end of Regular Boot */

	/* Load in list of banned sites. */
	loadban();
	
	/* This brings us to the main loop of the program. */
	doit(server);
}
