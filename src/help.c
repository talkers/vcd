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
 * This is help.c of vcd. This contains the help functions.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vcd.h"


/* Get a user some help */
void
getuserhelp(me, msg)
	struct user 	*me;
	char		*msg;
{
	FILE *fptr;
	char hb[100], helpstr[60], command[50], topic[50];

	sprintf(command, "%s", stripwhite(me->input+2));
	if (command[0] != '\0') {
	    command[50] = '\0';
            sprintf(helpstr, " Help on %s ", command);
	} else {
	    sprintf(helpstr, " Help Index ");
	}
        sprintf(topic, "(%s)", command);


	fptr = fopen(HELP, "r");
	if (fptr == (FILE *)0) {
	   writetext(me, msg);
	   return;
	}
	
	hb[0] = '\0';

        titlebar(me, helpstr);
	while (!feof(fptr)) {
	      if (fgets(hb, 255, fptr) == (char *)0)
		  break;
		 if (hb[0] == '{') {
                       if (matchstring(hb, topic, 1)) {
			   while (hb[0] != '}') {
	                      if (fgets(hb, 255, fptr) == (char *)0)
				  break;
			      if (hb[0] == '}') continue;
			      stripcr(hb);
		              writelist(me, hb, "\r\n", NULL);
                           } 
			   writetext(me, DASHLINE);
			   fclose(fptr);
			   return;
                       }
                 }
  	}
	writetext(me, "No help found on your topic.\r\n");
	writetext(me, DASHLINE);
	fclose(fptr);
}



void
getpowerhelp(me, msg)
	struct user 	*me;
	char		*msg;
{
	FILE *fptr;
	char hb[100], helpstr[80], command[50], topic[100];
	int lev;

        sprintf(command, "%s", stripwhite(me->input+3));
        if (command[0] != '\0') {
            command[50] = '\0';
            sprintf(helpstr, " Help on %s ", command);
        } else {
            sprintf(helpstr, " Power Help Index ");
        }
        sprintf(topic, "(%s)", command);

	fptr = fopen(POWERHELP, "r");
	if (fptr == (FILE *)0) {
	   writelist(me, msg, (char *)0 );
	   return;
	}
	
	hb[0] = '\0';

        titlebar(me, helpstr);
	while (!feof(fptr)) {
	      if (fgets(hb, 255, fptr) == (char *)0) break;
		 if (hb[0] == '{') {
                    lev = atoi(hb+1);
                    if (me->level >= lev) {
                       if (matchstring(hb+2, topic, 1)) {
			  while (hb[0] != '}') {
	                      if (fgets(hb, 255, fptr) == (char *)0) break;
			      if (hb[0] == '}') continue;
		              stripcr(hb);
		              writelist(me, hb, "\r\n", NULL);
                           } 
                          writetext(me, DASHLINE);
			  fclose(fptr);
		          return;
                       }
                    }  
                 }
  	}
	writetext(me, "No help found on your power topic.\r\n");
	writetext(me, DASHLINE);
	fclose(fptr);
}

