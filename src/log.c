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
 * This is log.c of vcd.  The functions here handle error logging used
 * primarily for debugging purposes.
 */

/* system headers */
#include <stdio.h>		/* For FILE, fopen(), sprintf(). */
#include <time.h>		/* For ctime(), type time_t */
#include "vcd.h"

/* global variables */
/* This is used as a buffer to write debugging and error messages. */
char            errbuff[512];	
				

/*
 * This logs messages to the `logfile' along with what file and line it was
 * called from with the present time and date.
 */
void
logmess(message, logfile, filename, linenum)
	char           *message, *logfile, *filename;
	int             linenum;
{
	FILE            *fptr;
	time_t	        now = time((time_t *)0);
	char		date[25];

	fptr = fopen(logfile, "a");
	if (fptr == (FILE *) 0) {
	   return;
	}

	sprintf(date, "%s", ctime(&now)+4);
	date[20] = '\0';

	fprintf(fptr, "[%s(%d)<%s>] %s\n", filename, linenum, date, message);
	fclose(fptr);
}

/*
 * This logs messages along with the present errno to the `logfile' along with
 * what file and line it was called from with the present time and date.
 */
void
logerror(message, logfile, filename, linenum, myerrno)
	char           *message, *logfile, *filename;
	int             linenum, myerrno;
{
	FILE		*fptr;
	time_t		now = time(0);
	char		date[25];

#ifndef NOSMARTLOG
	if (smartlog(myerrno))
	    return;
#endif

	fptr = fopen(logfile, "a");
	if (fptr == (FILE *) 0) {
	   return;
	}

	sprintf(date, "%s", ctime(&now)+4);
	date[20] = '\0';

	fprintf(fptr, "[%s(%d)<%s>] %s (errno = %d)\n", filename, linenum, 
		date, message, myerrno);
	fclose(fptr);
}


/* Try to avoid logging extremely common unimportant errors.
 * return 1 to ignore logging the error.
 */
int
smartlog(myerrno)
	int	myerrno;
{
	switch (myerrno) {
		case EPIPE:  		
			return 1;	/* Broken Pipe */
		default: 
			return 0;
	}
}
	
