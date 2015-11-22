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
 * This is commands.c of vcd.  This contains the functions used
 * to perform all commands that are not 'db' or 'op' related.
 */
/* system headers */
#include <ctype.h>		/* For isdigit(). */
#include <stdio.h>		/* For sprintf(). */
#include <stdlib.h>		/* For atoi(). */
#include <string.h>		/* For strncpy(). */
#include "vcd.h"		

/* local buffer for writing stuff. */
static char buffer[512];


/*
 * Appends a msg to the bulletin board file.
 */
void
appendlogfile(filename, msg)
        char *filename;
        char *msg;
{
        FILE *fptr;
        char buf[INPUTMAX], tempbuf[NUMPOSTS][INPUTMAX];
        int num = 0, i;

        fptr = fopen(filename, "rb");
        if (!fptr) {
            fptr = fopen(filename, "ab");
            fprintf(fptr, msg);
            fclose(fptr);
            return;
        }

        while (!feof(fptr) && (num < NUMPOSTS+1)) {
               fgets(buf, INPUTMAX-1, fptr);
               strcpy(tempbuf[num], buf);
               num++;
        }
        fclose(fptr);

        if (num == NUMPOSTS+1) {
            for (i=0; i<NUMPOSTS-1; i++)
                 strcpy(tempbuf[i], tempbuf[i+1]);
            strcpy(tempbuf[NUMPOSTS-1], msg);

            fptr = fopen(filename, "wb");
            for (i=0; i<=NUMPOSTS-1; i++)
                 fprintf(fptr, tempbuf[i]);
            fclose(fptr);
        } else {
            fptr = fopen(filename, "ab");
            fprintf(fptr, msg);
            fclose(fptr);
        }
}


/* Posts a message to the bulletin board */
void
post_message(me)
	struct user *me;
{    
     char *msg;
     char *currenttime, tempbuffer[INPUTMAX], outbuffer[INPUTMAX+80];
     time_t now = time(0);  

     msg = stripwhite(me->input + 2);
     
     if (!strcmp(me->name, DEFAULTNAME)) {
        writetext(me, ">> Get a REAL name before posting to the bulletin board!\r\n");
        return;
     }   

     if (!*msg) {  
         writetext(me, ">> Syntax is .B<msg> were <msg> is your message.\r\n");
         return;
     } 
       
     currenttime = ctime(&now) + 4;
     currenttime[12] = '\0';
                                             
     sprintf(outbuffer, "[%s on %s said]: %s\n", me->name, currenttime, msg);
     strncpy(tempbuffer, outbuffer, INPUTMAX-1);
     appendlogfile(BBOARD, tempbuffer);
     writetext(me, ">> Message was successfully posted!\r\n");
}


void
read_messages(me, message)
    	struct user 	*me;
	char		*message;
{   
	FILE *fptr;
	int i = 1, postnum = 1;
	char *arg;

	fptr = fopen(BBOARD, "r");
	if (!fptr) {
	    writetext(me, message);
	    return;
	}	

	arg = (char *) stripwhite(me->input + 2);
	postnum = atoi(arg);


	/* Read all messages */
	if (!*arg) {
	    writetext(me, ">> The current vcd messages are:\r\n");
	    while (!feof(fptr)) {
               if (fgets(buffer, INPUTMAX, fptr) == (char *)0) break;  
	       stripcr(buffer);
               writelist(me, itoa(i), ": ", buffer, "\r\n", NULL);
               i++;
	    }
	} else {
            /* Read single message */
            while (!feof(fptr)) {
                   if (fgets(buffer, INPUTMAX, fptr) == (char *)0) break;
                   if (i == postnum) {
		       stripcr(buffer);
                       writelist(me, itoa(i), ": ", buffer, "\r\n", NULL);
		       fclose(fptr);
                       return;
                   }
                   i++;
           } 
           writetext(me, ">> No such post number.\r\n");
	}

	fclose(fptr); 
}


void
erase_message(me)
	struct user *me;
{
	char *ptr, bboard[NUMPOSTS+1][INPUTMAX];
	int i=1, j=1, postnum;
	FILE *fptr;

	ptr = (char *) stripwhite(me->input +3);
	if (!isdigit(*ptr)) {
	    writetext(me, ">> No such post to erase.\r\n");
	    return;
	} 

	postnum = atoi(ptr);

	fptr = fopen(BBOARD, "r");
        /* Read posts into an array */
	for (i=1; !feof(fptr); i++) {
	     if (fgets(buffer, INPUTMAX, fptr) == (char *)0) break;
	     strncpy(bboard[i], buffer, INPUTMAX);
	}
	fclose(fptr);

        if (postnum >= i || postnum <= 0) {
		writetext(me, ">> No such post to erase.\r\n");
		return;
	}

	fptr = fopen(BBOARD, "w"); 

        /* Write posts back to file ignoring postnum */
	for (j=1; j < i; j++) {
	     if (j != postnum)
	         fprintf(fptr, bboard[j]);
	}
	fclose(fptr);
        writelist(me, ">> Post (", ptr, ") deleted.\r\n", NULL);
}   
