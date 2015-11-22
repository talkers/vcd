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
 * This is io.c of vcd.  Various useful utilities for string manipulations
 * and I/O handling.
 */

/* system headers */
#include <ctype.h>		/* For isdigit(), isspace(), isprint(),
				 * isupper(). */
#include <stdio.h>		/* For sprintf(). */
#include <string.h>		/* For strcat(), strlen(). */
#include <time.h>		/* For time(). */
#include <varargs.h>
#include <unistd.h>		/* For write(). */
#include "vcd.h"


/* Various string formating and handling routines. */
/* 
 * This strips a leading space in a text string. 
 */
char *
stripspace(text)
	char           *text;
{
	char           *ptr = text;

	if (isspace(*ptr))
		ptr++;

	return ptr;
}


/*
 * This strips off the carriage returns and linefeed characters from the
 * trailing end of a text string.
 */
void
stripcr(text)
	char           *text;
{
	int             i = strlen(text);

	i--;

	while (i >= 0 && text[i] && !isprint(text[i])) {
		if (text[i] == '\r' || text[i] == '\n')
			text[i] = '\0';
		i--;
	}
}


/* 
 * This strips off any digits at the beginning end of a string. 
 */
char *
stripdigit(text)
	char           *text;
{
	char           *ptr = text;

	while (isdigit(*ptr))
		ptr++;

	return ptr;
}


/* 
 * This strips off all the leading and trailing whitespaces in a text string.
 */
char *
stripwhite(text)
	char           *text;
{
	char           *ptr = text;
	int             i = strlen(text);

	i--;

	while (i >= 0 && text[i] && isspace(text[i])) {
		text[i] = '\0';
		i--;
	}

	while (*ptr && isspace(*ptr)) 
		ptr++;

	return ptr;
}


/*
 * This returns back a 1 if the string text contains the substring pattern
 * and 0 if it doesn't.
 */
int
matchstring(text, pattern, casedep)
	char           *text,		/* String. */
	               *pattern;	/* Substring. */
	int             casedep;	/* 1 for case dependence and 0 for no 
				 	 * case dependence. */
{
	char           *tptr = text;	/* pointer to text */
	int             textlen = strlen(text), 
			patternlen = strlen(pattern);

	while (textlen >= patternlen) {
		if (casedep) {
			if (!strncmp(tptr, pattern, patternlen))
				return 1;
		} else {
			if (!strncasecmp(tptr, pattern, patternlen))
				return 1;
		}
		tptr++;
		textlen--;
	}
	return 0;
}


/* Various routines that read and process text from a user. */
/*
 * This reads the user's input and stores it in the user's output buffer. It
 * returns back a pointer to the text read and 0 on error.
 */
int
readline(me, buffer)
	struct user    *me;
	char           *buffer;
{
	int             length;
	char            tempbuff[INPUTMAX + 1];


        length = read(me->line, tempbuff, INPUTMAX);
        if (length == -1) {
            sprintf(errbuff, "%s on line %d read error", me->host, me->line);
            logerr(errbuff, SYSLOG);
            getout(me, "has disconnected.\r\n");
            return 0;
        } else if (length == 0) {
	    /* Damage control */
	    fd_master_clear(me->line);
            getout(me, "has disconnected.\r\n");
            return 0;
        } else if (length >= INPUTMAX) {
            writetext(me, ">> Too Much Input.\r\n");
            getout(me, "has been kicked off for spamming.\r\n");
            return 0;
        }

        tempbuff[length] = '\0';
        strncpy(buffer, tempbuff, length);
        buffer[length] = '\0';
        return length;
}


/*
 * This processes the raw input text read by the server.  It returns a 0 if
 * the user spams themself off, and a 1 otherwise.
 */
int
processinput(me, rawbuffer)
	struct user    *me;
	char           *rawbuffer;
{
	char           	*ptr = rawbuffer;
	time_t          mastertime;

	/*
	 * If the user wasn't typing before, set the queue pointer to point
	 * to the beginning of the input buffer and turn on the TYPING flag.
	 */
	if (nottoggled(me, TYPING)) {
		me->queue = me->input;
		me->length = 0;
		toggleflag(me, TYPING);
	}

	/* Each line terminated with a \r\n is processed line by line. */
	while (*ptr != '\0') {
		/*
		 * See if we can extract a line of text from the raw text
		 * buffer. The GOTCR flag will be turned on and the TYPING
		 * flag turned off if we extract a string terminated by \r\n.
		 */
		ptr = extractline(ptr, me);

		/* Check for spamming. */
		if (istoggled(me, GOTCR)) {
		    /* Get the present time. */
		    mastertime = time((time_t *) 0);
		    /*
		     * Check if the time interval between the last
		     * message and the present message is shorter than
		     * SPAMINTERVAL.
		     */
		    if ((mastertime - me->idle) >= SPAMINTERVAL)
			me->numlines = me->commands = 0;

		    me->idle = time((time_t *) 0);

		    if (((me->numlines >= SPAMMAX && me->channel == channelhead) 
			|| me->commands >= SPAMMAX) && me->level <= JOEUSER) {
			getout(me, "has been kicked off for spamming.\r\n");
			writetext(me, ">> Spam Alert!\r\n");
		   	return 0;
		    }
		}

		/* Parse the text only if we get a carriage return. */
		if (istoggled(me, GOTCR)) {
			
		    /* Is this needed? This is original from Schrodinger. */
		    /* This SHOULD be checked in readline(); Where it is. */
		    if (me->length >= INPUTMAX) {
			getout(me, "has been kicked off for spamming.\r\n");
			writetext(me, ">> Too Much Input.\r\n");
			return 0;
		    }

		    toggleflag(me, GOTCR);
		    me->queue = me->input;
		    me->length = 0;
		    /*
		     * If there's still stuff in the raw text buffer,
		     * turn the TYPING flag back on. (The TYPING flag was
		     * turned off when the GOTCR flag was turned on).
		     */
		     if (*ptr != '\0')
		         toggleflag(me, TYPING);

		     /* Check whether or not this is a command. */
		     if (me->input[0] == '.' ||
		         me->input[0] == '/' ||
		         me->input[0] == ',') {

			 /*
			  * If the command is a module command execute it 
			  * and ONLY it. If it isn't a module command check
			  * if it is a haven command.
			  */
			 if (!check_mod_command(me))
			     if (!getcommand(me))
				 return 0;

		     } else {
		         me->numlines++;
		         talk(me);
		     }
		}
	}

	return 1;
}


/*
 * This extracts a line of text from the raw text and returns a pointer
 * pointing to the next line of text.
 */
char *
extractline(rawbuffer, my)
	char           *rawbuffer;
	struct user    *my;
{
	char           *rawptr = rawbuffer, *myptr = my->queue;


	/* Only keep the characters that are printable. */
	while (*rawptr && !(*rawptr == '\n' || *rawptr == '\r')) {

		/*
		 * Copy the printable characters to where my->queue is
		 * pointing to while limiting the number of characters in the
		 * my->input string buffer (pointed to by my->queue) to less
		 * than INPUTMAX.
		 */
		if (my->length <= INPUTMAX) {
		    if (*rawptr != '\b' && *rawptr != '\177') {
			if (isprint(*rawptr)) {
			    *myptr++ = *rawptr++;
			    my->length++;
			} else {
			    rawptr++;
			}
		    } else {
			myptr--;
			rawptr++;
		    }
		}
	}

	*myptr = '\0';
	my->queue = myptr;

	/*
	 * If the carriage return and/or linefeed characters exist, get rid
	 * of them and turn on the GOTCR flag and turn off the TYPING flag.
	 */
	while (*rawptr && (*rawptr == '\n' || *rawptr == '\r')) {
		if (nottoggled(my, GOTCR))
			toggleflag(my, GOTCR);
		if (istoggled(my, TYPING))
			toggleflag(my, TYPING);
		rawptr++;
	}

	return rawptr;
}


/* 
 * Write the user's buffer to them while wrapping it. 
 */
void
writewrap(desc, width, text)
	int	desc, width;
	char	*text;
{
	int n, w;
	char *s = text,
	     *s2,
	      new[3];

	strcpy(new, "\r\n");

	n = strlen(s);

	while (n > width ) {
           s2 = s+width; 

              while (s2 > s && *s2 != ' ')   
                    s2--; 

	   if (s==s2)
	      w = width;
	   else
	      w = s2 - s;
	   write(desc, s, w);

           write(desc, new, 2);

	   s += w;

	   if (*s == ' ')
	      s++;
	   n = strlen(s);
	}
	write(desc, s, n);
	return;
}
	
		
/*
 * Write some text to a file descriptor.  -1 is returned if write could not
 * write the text. 0 is returned if all the text is written successfully. If
 * only a part of text was sent, the remaining unsent text will be copied
 * into 'text' and the length of the remaining text will be returned.
 */
int
writedesc(desc, text)
	int             desc;
	char           *text;
{
	unsigned int    i, j;
	char           *ptr = text;

	j = strlen(ptr);

	i = write(desc, ptr, j);
	if (i == -1) {
	    logerr("error with write", SYSLOG);
	    return -1;
	} else if (i != j) {
	    sprintf(errbuff, "only %d bytes out of %d bytes were sent", i, j);
	    logerr(errbuff, SYSLOG);
	    ptr += i;
	    j -= i;
	    strncpy(text, ptr, j);
	    text[j] = '\0';
	    return j;
	}
	return 0;
}


/*
 * Put together a string made up from a variable list of string arguments and
 * write it to a user. The last text in the string has to be a null character.  
 */
void
writelist(me, va_alist)
	struct user *me;
        va_dcl
{
	va_list         ap;
	char           *bptr, buffer[2000];

	buffer[0] = '\0';


	/* Find the beginning of our list. */
	va_start(ap);

	/*
	 * The arguments in the list are strings that make up a
	 * message. Concatenate each argument in the rest of the list into a
	 * buffer sequentially.
	 */

	bptr = va_arg(ap, char *);
	while (bptr != (char *) 0) {
		strcat(buffer, bptr);
		bptr = va_arg(ap, char *);
	}
	/* Indicate that our list is finished. */
	va_end(ap);

	writetext(me, buffer);  
}


/*
 * Chooses to send text to writewrap() or writedesc() depending on
 * whether or not the user has their WRAP flag toggled.
 */ 
void
writetext(me, text)
	struct user *me;
	char	*text;
{
	   if (istoggled(me, WRAP))
	       writewrap(me->line, me->wrap, text);
	   else
	       writedesc(me->line, text);
}


/*
 * This writes the contents of a file to the user. If the file doesn't exist,
 * send them the defaultmsg.
 */
void
writefile(desc, thefile, defaultmsg)
	int desc;
	char *thefile, *defaultmsg;
{
	FILE *fptr;
	char buffer[256];

	fptr = fopen(thefile, "r");
	if (fptr == (FILE *)0) {
	    writedesc(desc, defaultmsg);
	    return;
	}

	while (!feof(fptr)) {
	       if (fgets(buffer, 255, fptr) == (char *)0) break;
	       stripcr(buffer);
	       strcat(buffer, "\r\n");
	       writedesc(desc, buffer);
	}
	fclose(fptr);
}


/*
 * Write text to all users of a certain level.
 */
void
writelevel(level, text)
	int	level;
	char	*text;
{
	struct user	*them;
	
	for (them = userhead; them; them = them->next)
		if (them->level >= level)
			writetext(them, text);
}


/* 
 * This converts integers to strings. 
 */
char *
itoa(i)
	unsigned long i; 
{
	static char   s[15] = "";

	sprintf(s, "%lu", i);
	return s;
}


/*
 * This converts time in seconds to month/day/hour/min/sec format.      
 *  Parameter i is 1 for idle times (i.e. 1h4m or 12m17s) and 4 for uptime
 *  (i.e. it is possible to get 1M3d7h45m23s)                          
 */
char *
timeform(t, i)
        time_t  t;
        int     i;
{
        static char buffer[80] = "";
	char x[10];
        int months, days, hours, mins, secs;
        int length = 0;

        buffer[0] = '\0';
        secs = t % 60;
        t /= 60;
        mins = t % 60;
        t /= 60;
        hours = t % 24;
        t /= 24;
        days = t % 30;
        t /= 30;
        months = t;
        if (months) {
	    sprintf(x, "%s", i==1 ? "M" : (months==1 ? " Month " : " Months "));
            strcat(buffer, itoa(months));
            strcat(buffer, x);
            if (++length > i)
                return buffer;
        }
        if (days) {
	    sprintf(x, "%s", i==1 ? "d" : (days==1 ? " Day " : " Days "));
            strcat(buffer, itoa(days));
            strcat(buffer, x);
            if (++length > i)
                return buffer;
        }
        if (hours) {
	    sprintf(x, "%s", i==1 ? "h" : (hours==1 ? " Hour " : " Hours "));	
            strcat(buffer, itoa(hours));
            strcat(buffer, x);
            if (++length > i )
                return buffer;
        }
        if (mins) {
	    sprintf(x, "%s", i==1 ? "m" : (mins==1 ? " Minute " : " Minutes "));
            strcat(buffer, itoa(mins));
            strcat(buffer, x);
            if (++length > i )
                return buffer;
        }
        if (secs) {
	    sprintf(x, "%s", i==1 ? "s" : (secs==1 ? " Second " : " Seconds "));
            strcat(buffer, itoa(secs));
            strcat(buffer, x);
            if (++length > i)
                return buffer;
        }
        return buffer;
}


/* 
 * Converts a string to all lower case.
 */ 
void
removecaps(s)
	char *s;
{
	register int 	i;
	int 		len = strlen(s);

	for (i=0; i<=len; i++)
	     s[i] = tolower(s[i]);
}


/* 
 * Create a header or footer with dashes around a string of text. 
 */
void
titlebar(me, text)
	struct user *me;	
	char   *text;
{
	int	i = (78 - strlen(text)) / 2;
	char	title[100];

	strcpy(title, DASHLINE);
	
	while (*text)
	       title[i++] = *text++;

	writetext(me, title);
}
