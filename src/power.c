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
 * This is power.c of vcd.  This is where the security and power commands
 * are.
 */

/* system headers */
#include <ctype.h>		/* For isdigit(). */
#include <stdio.h>		/* For sprintf(). */
#include <stdlib.h>		/* Fro atoi(). */
#include <string.h>		/* For strcmp(). */
#include "vcd.h"

/* local variables */
static char buffer[512];

/* global variable */
char *power_names[4] = { "User", "SysOp", "Admin", "Root" };        

/* Power commands. */
/* This does a global yell regardless of hush. */
void
globalyell(me)
	struct user *me;
{
	struct user 	*them;
	char 		*ptr;

	ptr = stripwhite(me->input+3);  /* .@Y */
	sprintf(buffer, ">> System message from %c%d%c %s: ", bra[me->level],
		me->line, ket[me->level], me->name);

	for (them = userhead; them; them = them->next)
	     writelist(them, buffer, ptr, "\r\n", (char *)0);
	
}


/* This write an invisible message to either a user or globally. */
void
invismess(me)
	struct user *me;
{
	int who;
	char *ptr;
	struct user *them;

	ptr = (me->input+3);  /* .@i */

	/* See if the message is suppose to go to a user. */
	who = atoi(ptr);
	them = findline(who);

	if (isdigit(*ptr) && them == (struct user *)0) {
		/* Case where the user doesn't exist. */
		writetext(me, BAD_LINE);
		return;
	} else if (them != (struct user *)0) {
		/* Case where the user exists. */
		ptr = stripdigit(ptr);
		ptr = stripspace(ptr);
		writelist(them, ptr, "\r\n", (char *)0);
		return;
	}

	/* Global invisible message case. */
	ptr = stripspace(ptr);
	for (them = userhead; them; them = them->next)
		writelist(them, ptr, "\r\n", (char *)0);

}
	

void
insuf_power(me, them, message)
	struct user	*me, *them;
	char		*message;
{
        sprintf(buffer, ">> You don't have power over %c%d%c %s.\r\n", 
		bra[them->level], them->line, ket[them->level], them->name);
        writetext(me, buffer);
        sprintf(buffer, ">> %c%d%c %s %s\r\n", bra[me->level],
                me->line, ket[me->level], me->name, message);
        writetext(them, buffer);
}




/* This kicks a user off. */
void
kick(me)
	struct user *me;
{
	int it;
	char *ptr;
	struct user *them;

	ptr = (char *)stripwhite(me->input+3);   /* .@k */
	/* Check if the user line is even logged on. */
	if (!isdigit(*ptr)) {
		writetext(me, BAD_LINE);
		return;
	}

	it = atoi(ptr);
	them = findline(it);
	if (them == (struct user *)0) {
		writetext(me, BAD_LINE);
		return;
	}

	/* Check to see if the victim is of a power level less than the
         * kicker. Only a user of a lower power level can be kicked off */
	if (me->level > them->level) {
		getout(them, "has been kicked off.  Ha ha ha!\r\n");
		writelist(me, ">> You kicked ", them->name, 
			      " off the haven.\r\n", NULL);
		writetext(them, ">> You have been KICKED off.\r\n");
		disconnect(them, QUIT_NORMAL);
	} else
		insuf_power(me, them, "has tried to KICK you off!");
}


/* This raises a user's power level */
void
promote(me)
       struct user *me;
{
       struct user *them;
       char *ptr;
       int to;
       
       
       ptr = (char *) stripwhite(me->input + 3);     /* .@+ */
       if(!isdigit(*ptr)) {
          writetext(me, BAD_LINE);
          return;
       }

       to = atoi(ptr);
       them = findline(to);
       if (them == (struct user *)0) {
          writetext(me, BAD_LINE);
          return;
       }

       if (them->level >= ADMIN) {
           writelist(me, ">> ", them->name, " already has ", 
		     power_names[ADMIN], " status.\r\n", NULL);
           return;
       }
 
       them->level++;
       sprintf(buffer, ">> Your power level has been raised to %s, by %c%d%c %s.\r\n", 
              power_names[them->level], bra[me->level], me->line, ket[me->level], 
	      me->name);
       writetext(them, buffer);
       sprintf(buffer, ">> You raised %c%d%c %s's power level to %s.\r\n",
              bra[them->level], them->line, ket[them->level],
	      them->name, power_names[them->level]);
       writetext(me, buffer); 

}     
    

/* This lowers a dork's power level. Only ROOT has this command. */
void
demote(me)
	struct user *me;
{
	struct user *them;
	char *ptr;
	int dork;

	ptr = (char *) stripwhite(me->input + 3);    /* .@- */
	if (!isdigit(*ptr)) {
	    writetext(me, BAD_LINE);
            return;
        }

	dork = atoi(ptr);
	them = findline(dork);
	if (them == (struct user *)0) {
	    writetext(me, BAD_LINE);
	    return;
	}

	if (me->level > them->level) {
            if (them->level == JOEUSER) {
                writelist(me, ">> ", them->name, "'s power level is as low as it can get.\r\n", NULL);
                return;
            }
            them->level--;
            sprintf(buffer, ">> Your power level has been lowered to %s, by %c%d%c %s\r\n",
                    power_names[them->level], bra[me->level], me->line, 
		    ket[me->level], me->name);
            writetext(them, buffer);
            sprintf(buffer, ">> You lowered %c%d%c %s's power level to %s.\r\n", 
                    bra[them->level], them->level, ket[them->level], them->name,
                    power_names[them->level]);
            writetext(me, buffer);
	} else
            insuf_power(me, them, "tried to DEMOTE you!");
}


void
fakehost(me)
	struct user *me;
{
        char *ptr;
	int  len;	

        ptr = (char *) stripwhite(me->input + 3);

	if ((len = strlen(ptr)) > HOSTMAX)
	    ptr = ptr + (len - HOSTMAX);

        strncpy(me->host, ptr, HOSTMAX);
        me->host[HOSTMAX] = '\0';
        writelist(me, ">> Host name changed to: ", me->host, "\r\n", (char *)0);
}



void
opchannel(me)
	struct user *me;
{
	char *ptr;
	struct user *them;

	if (istoggled(me, OPHUSH)) {
            writetext(me, ">> You can't use the op channel, you're blocking it!\r\n");
	    return;
	}

	ptr = (char *) stripwhite(me->input + 2);  /* .o */

	if ((*ptr == ':' || *ptr == ';') && ptr[1] != '\'') {
		ptr++;
		sprintf(buffer, "(**Op) %s ", me->name);
	} else if ((*ptr == ':' || *ptr == ';') && ptr[1] == '\'') {
		ptr++;
		sprintf(buffer, "(**Op) %s", me->name);
	} else
		sprintf(buffer, "(**Op: %s) ", me->name);

	for (them = userhead; them; them = them->next)
	     if (them->level >= SYSOP && nottoggled(them, OPHUSH))
		 writelist(them, buffer, ptr, "\r\n", NULL);
}


void
takecaps(me)
	struct user *me;
{
	struct user *them;
	int dork;
	char *ptr;

	ptr = (char *) stripwhite(me->input + 3);  /* .@L */
	if (!isdigit(*ptr)) {
	    writetext(me, BAD_LINE);
	    return;
	}

	dork = atoi(ptr);
	them = findline(dork);
	if (them == (struct user *)0) {
	    writetext(me, BAD_LINE);
	    return;
	}

        if (me->level > them->level) {
	    toggleflag(them, NOCAPS);
            if (istoggled(them, NOCAPS))
	        writelist(me, ">> ", them->name, " has lost caps.\r\n", NULL);
            else
	        writelist(me, ">> ", them->name, " has regained caps.\r\n", NULL);
	} else
            insuf_power(me, them, "tried to steal your caps!");
 
}


void
havenshutdown(me)
	struct user *me;
{
	struct user *them;

	for (them = userhead; them; them=them->next) {
	     writelist(them, ">> Haven shutdown by ", me->name, "\r\n", NULL); 
	     disconnect(them, QUIT_NORMAL);
	}
	exit(0);
}


void
toggletimeouts(me)
	struct user *me;
{	
	char	*ptr;
	long	 newidle;

	ptr = (char *) stripwhite(me->input+3);
	if (*ptr == '+') {
	    ptr++;
	    newidle = atoi(ptr);
	   
	    if (newidle < 300) {
	        writetext(me, ">> Timeout must be at least 300 secs.\r\n");
 	        return;
	    }
		
	    idletimeout = newidle;
	    sprintf(buffer, ">> Idle timeout set to %ld seconds.\r\n", newidle);
	    writetext(me, buffer);

	    /* Check for idlers since we just changed the idletimeout length */
	    nextcheck = checkidle();

	    return;  /* Dont toggle idle status */		
	}


	if (is_system(TIMEOUTS)) {
	    clr_system(TIMEOUTS);
	    writetext(me, ">> Idle timeouts are now off.\r\n");
	} else {
	    set_system(TIMEOUTS);
	    writetext(me, ">> Idle timeouts are now on.\r\n");
	}
}


void
auto_nocaps(me)
	struct user *me;
{
	register int	i;
		 int    upchars = 0, sl = strlen(me->input);

	if (istoggled(me, NOCAPS) || sl <= 30 || me->level >= SYSOP ||
		me->channel != channelhead)
		return;

	for (i=0; i<= sl; i++)
		if (isupper(me->input[i]))
			upchars++;

	if (sl - upchars <= upchars) 
		toggleflag(me, NOCAPS);

}


void
toggleauto_nocaps(me)
	struct user *me;
{
	if (is_system(AUTOCAPS)) {
	    clr_system(AUTOCAPS);
	    writetext(me, ">> Auto cap removal is now off.\r\n");
	} else {
	    set_system(AUTOCAPS);
	    writetext(me, ">> Auto cap removal is now on.\r\n");
	}
}


void
toggleophush(me)
	struct user *me;
{
	toggleflag(me, OPHUSH);
	if (istoggled(me, OPHUSH))
		writetext(me, ">> Op channel is now hushed.\r\n");
	else
		writetext(me, ">> Op channel now active.\r\n");
}
