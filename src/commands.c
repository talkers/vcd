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
#include <unistd.h>
#include <time.h>               /* For gettime(); */ 
#include <ctype.h>		/* For isdigit(). */
#include <memory.h>		/* For memcmp(). */
#include <stdio.h>		/* For sprintf(). */
#include <stdlib.h>		/* For atoi(). */
#include <string.h>		/* For strncpy(). */
#include <sys/time.h>
#include <sys/types.h>		/* For type fd_set, FD_SET(), FD_CLR(),
				 * FD_ISSET(), FD_ZERO(). */
#include "color.h"
#include "vcd.h"		

/* global variables */
/* Brackets for each power level. */
char     bra[] = "({<[", ket[] = ")}>]"; 

/* local buffer for writing stuff. */
static char buffer[512];


/* Commands */
/* Login and logout messages. */
/*
 * This writes a login message for a new user to everybody that doesn't have
 * their messages suppressed.
 */
void
greeting(new)
	struct user    *new;
{
	struct user    *them;

        sprintf(buffer, ">> New arrival on line %d from %s\r\n", 
		new->line, new->host);

	/* Only write to users that have their messages on. */
	for (them = userhead; them; them = them->next)
	     if (nottoggled(them, ARRMSGS) || them->channel == channelhead)
	         writetext(them, buffer);
}

/* 
 * This was used to test for .q power up attempts. Now it
 * just returns 0 to getcommand(); signalling the user quit.
 */
int
quit(me)
	struct user    *me;
{
	getout(me, "just left.\r\n");
	return 0;
}


/*
 * This writes a logout message for a user that has logged off to everybody
 * that doesn't have their messages suppressed.
 */
void
getout(me, mess)
	struct user    *me;
	char           *mess;
{
	struct user    *them;

	sprintf(buffer, ">> %c%d%c %s %s", bra[me->level], me->line,
		ket[me->level], me->name, mess);

	/* Only write to users that have their messages on. */
	for (them = userhead; them; them = them->next)
	     if (them != me && nottoggled(them, CHANMSGS) && 
                 (nottoggled(them, ARRMSGS) || them->channel == me->channel)) 
		 writetext(them, buffer);
}


/* Message types. */
/* Normal talking text to the present channel. */
void
talk(me)
	struct user    *me;
{
	struct user    *them;
	char           *ptr = (me->input);

	/* Check if they even typed anything. */
	if (*ptr == '\0')
	    return;
        
	/* Check for fuckwits */
	if (is_system(AUTOCAPS))
		auto_nocaps(me);

        /* Check if they've had caps removed. */ 
        if (istoggled(me, NOCAPS))
		removecaps(ptr); 

	/* Check if they're emoting. */
        if (*ptr == ':' || *ptr == ';') {
	    ptr++;	
	    if (*ptr == '\'' || *ptr == ',')
                sprintf(buffer, "%c%d%c %s", bra[me->level], me->line,
                        ket[me->level], me->name);
	    else  
	        sprintf(buffer, "%c%d%c %s ", bra[me->level], me->line,
		        ket[me->level], me->name);
	} else
		sprintf(buffer, "%c%d, %s%c ", bra[me->level], me->line,
			me->name, ket[me->level]);

	/* Write to users only on their channel and who are not gagging them. */
	for (them = userhead; them; them = them->next)
	     if (!FD_ISSET(me->line, &them->gags) && me->channel==them->channel)
		 if (them->talkcolor)
	             writelist(them, colors[them->talkcolor], buffer, ptr,
				colors[0], "\r\n", (char *)0 );
		 else
		     writelist(them, buffer, ptr, "\r\n", (char *)0 );
}


/* Private messages. */
void
private(me)
	struct user    *me;
{
	int             to, emote = 0;
	char            brackets[100], *ptr, p[6] = "";
	static char     b[2];
	struct user    *them;

	ptr = (char *) stripwhite(me->input + 2);
	if (!isdigit(*ptr))
	    to = me->lastp;
	else
	    to = atoi(ptr);

	them = findline(to);
	if (them == (struct user *) 0) {
	    writetext(me, BAD_LINE);
	    return;
	}
	me->lastp = to;

        if (istoggled(me, PSUPPRESS)) {
            writetext(me, ">> You can't send .p's while suppressing them yourself.\r\n");
            return;
        } 

        if (istoggled(them, PSUPPRESS)) {
           sprintf(buffer, ">> %c%d%c %s is suppressing .p messages.\r\n",
                   bra[them->level], them->line, ket[them->level], them->name);
           writetext(me, buffer);
           return;
        }

	buffer[0] = '\0';

	/* Check if they have private beeps on. */
	if (istoggled(them, BEEPS)) {
            b[0] = '\007';
	    b[1] = '\0';
	} else
	    b[0] = '\0'; 

	ptr = stripdigit(ptr);
        ptr = stripwhite(ptr);

	/* Check if they're emoting. */
	if (*ptr == ':' || *ptr == ';') {
            emote = 1;
            ptr++; 
            if (*ptr == '\'' || *ptr == ',') 
                sprintf(brackets, "%c%d,p%c %s", bra[me->level], me->line,
                        ket[me->level], me->name);
	    else
		sprintf(brackets, "%c%d,p%c %s ", bra[me->level], me->line,
			ket[me->level], me->name);
	} else
		sprintf(brackets, "%c%d,p %s%c ", bra[me->level], me->line,
			me->name, ket[me->level]);

	strcat(buffer, brackets);

	ptr = stripwhite(ptr);
	sprintf(brackets, "sent to %c%d%c %s.\r\n", bra[them->level],
                them->line, ket[them->level], them->name);

	/* Check for .p echoes */
	if (istoggled(me, ECHO)) {
            if (emote) {
                if (*ptr == '\'' || *ptr == ',') 
                    writelist(me, ">> '", me->name, ptr, "' ", brackets, (char *)0);
                else { 
		    ptr = stripwhite(ptr);
                    writelist(me, ">> '", me->name, " ", ptr, "' ", brackets, (char *)0);
		}
            } else
		   writelist(me, ">> '", ptr, "' ", brackets, (char *) 0);
	} else
		   writelist(me, ">> /p ", brackets, (char *) 0);

	/* Write the /p to them if they're not gagged by the other user. */
	if (FD_ISSET(me->line, &them->gags))
	    return;

	if (istoggled(them, HIPOINTER))
	    sprintf(p, "****> ");

	if (istoggled(them, HILITE))
	    writelist(them, b, colors[them->hicolor], p, buffer, ptr, 
                      colors[0], "\r\n", (char *)0);
	else
	    writelist(them, b, p, buffer, ptr, "\r\n", (char *)0 );

}


/* Yells. */
void
yells(me)
	struct user    *me;
{
	struct user    *them;
	char           *ptr = stripspace(me->input+2);

	/*
	 * Check if they're hushed since they cannot yell if they're hushed.
	 */
	if (istoggled(me, HUSHED)) {
		writetext(me, ">> You can't yell when hushed!\r\n");
		return;
	}

	/* Check for fuckwits */
	if ( is_system(AUTOCAPS) ) auto_nocaps(me);

        /* Check if they've had caps removed. */
        if (istoggled(me, NOCAPS)) removecaps(ptr);

	/* Check if they're emoting. */
	if (*ptr == ':' || *ptr == ';') {
	    ptr++;
	    if (*ptr == '\'' || *ptr == ',')
		sprintf(buffer, "%c**%d**%c %s", bra[me->level], me->line,
			ket[me->level], me->name);
	    else 
		sprintf(buffer, "%c**%d**%c %s ", bra[me->level], me->line,
			ket[me->level], me->name);
	} else
		sprintf(buffer, "%c**%d, %s**%c ", bra[me->level], me->line,
			me->name, ket[me->level]);

	/* Send only to users which are not hushed and not gagging them. */
	for (them = userhead; them; them = them->next)
	    if (nottoggled(them, HUSHED) && !FD_ISSET(me->line, &them->gags))
		if (them->yellcolor)
	            writelist(them, colors[them->yellcolor], buffer,
                             ptr, colors[0], "\r\n", (char *)0);
		else
		    writelist(them, buffer, ptr, "\r\n", NULL);
}
/* End Message Types */


/* These commands toggle flags. */
/* This toggles a user's private echo flag. */
void
togglepecho(me)
	struct user    *me;
{

	toggleflag(me, ECHO);
	if (istoggled(me, ECHO))
		writetext(me, ">> .p echoing on.\r\n");
	else
		writetext(me, ">> .p echoing off.\r\n");
}


/* This toggles a user's private messages hilite flag. */
void
togglehilite(me)
	struct user    *me;
{

        toggleflag(me, HILITE);
	if (istoggled(me, HILITE))
		writetext(me, ">> .p hilites on.\r\n");
	else
		writetext(me, ">> .p hilites off.\r\n");
}


/* This toggles a user's private echo beeps. */
void
togglebeeps(me)
	struct user    *me;
{

	toggleflag(me, BEEPS);
	if (istoggled(me, BEEPS))
		writetext(me, ">> .p beeps on.\r\n");
	else
		writetext(me, ">> .p beeps off.\r\n");
}


/* This toggles a users's messages flag. */
void
togglearrivals(me)
	struct user	*me;
{
	switch(me->flags & (CHANMSGS | ARRMSGS)) {
	
	case (CHANMSGS | ARRMSGS):
		toggleflag(me, CHANMSGS | ARRMSGS);
	case (0):
		toggleflag(me, ARRMSGS);
		writetext(me, ">> Arrival and departure messages are suppressed.\r\n");
		break;
	case (CHANMSGS):
		toggleflag(me, ARRMSGS | CHANMSGS);
	case (ARRMSGS):
		toggleflag(me, ARRMSGS);
		writetext(me, ">> All messages enabled.\r\n");
		break;
	}
}


void
toggleallmsgs(me)
	struct	user	*me;
{
	switch(me->flags & (CHANMSGS | ARRMSGS)) {

	case (ARRMSGS):
		toggleflag(me, ARRMSGS);
	case (0):
		toggleflag(me, ARRMSGS | CHANMSGS);
		writetext(me, ">> All messages suppresed.\r\n");
		break;
	case (CHANMSGS):
		toggleflag(me, CHANMSGS);
	case (ARRMSGS | CHANMSGS):
		toggleflag(me, ARRMSGS | CHANMSGS);
		writetext(me, ">> All messages enabled.\r\n");
		break;
	}
}  


/* This toggles a users's hush flag. */
void
togglehush(me)
	struct user    *me;
{

	toggleflag(me, HUSHED);
	if (istoggled(me, HUSHED))
		writetext(me, ">> Yells are now suppressed.\r\n");
	else
		writetext(me, ">> You can now hear yells.\r\n");
}


/* Toggle a line gag. */
void
togglegag(me)
	struct user    *me;
{
	struct user    *them;
	int             dork;
	char           *ptr;

	ptr = stripwhite(me->input + 2);
	if (*ptr == '\0') {
		showgags(me);
		return;
	}
	/* Check to see if we have a valid line number. */
	if (!isdigit(*ptr)) {
		writetext(me, BAD_LINE);
		return;
	}

	dork = atoi(ptr);
	them = findline(dork);
	if (them == (struct user *) 0) {
		writetext(me, BAD_LINE);
		return;
	}

	/* Check to see if they're trying to gag themselves. */
	if (them->line == me->line) {
		writetext(me, ">> Gagging yourself with a spoon?\r\n");
		return;
	}

	if (!FD_ISSET(them->line, &me->gags)) {
		/* Gagging them. */
		FD_SET(them->line, &me->gags);
		sprintf(buffer, ">> %c%d%c %s has been gagged.  Ha ha ha!\r\n",
		       bra[them->level], them->line, ket[them->level], them->name);
	} else {
		/* Ungagging them. */
		FD_CLR(them->line, &me->gags);
		sprintf(buffer, ">> %c%d%c %s has been ungagged.\r\n",
			bra[them->level], them->line, ket[them->level], them->name);
	}

	writetext(me, buffer);
	return;
}


/* This shows the lines a user has gagged. */
void
showgags(me)
	struct user    *me;
{
	struct user    *them;
	fd_set          dummy;

	FD_ZERO(&dummy);

	/* Check if they are gagging anyone. */
	if (!memcmp((char *) &dummy, (char *) &me->gags, sizeof(fd_set *))) {
		writetext(me, ">> You have no gags.\r\n");
		return;
	}

	writetext(me, ">> Line gags.\r\n");
	for (them = userhead; them; them = them->next) {
	    if (FD_ISSET(them->line, &me->gags)) {
		sprintf(buffer, ">> %c%d%c %s\r\n", bra[them->level],
                        them->line, ket[them->level], them->name);
		writetext(me, buffer);
	    }
	}
}


/* This toggles a user's /p suppression */
void
togglepsuppress(me)
       struct user *me;
{
       toggleflag(me, PSUPPRESS);
       if (istoggled(me, PSUPPRESS))
          writetext(me, ">> Private messages are now suppressed.\r\n");
       else
          writetext(me, ">> Private messages are now active.\r\n");
}


/* This toggles a user's word wrap */
void
togglewrap(me)
        struct user *me;
{
        int     size;
        char    *ptr;

        ptr = (char *)stripwhite(me->input+2);
        size = atoi(ptr);

        if (*ptr) {
            if (size <= 10) {
                writetext(me, ">> Invalid screen size.\r\n");
                return;
            }
            me->wrap = size;
        }

        if (!size) {
           toggleflag(me, WRAP);
           if (nottoggled(me, WRAP)) {
              writetext(me, ">> Word wrap disabled.\r\n");
              return;
           }
        }

        me->flags |= WRAP;
        writelist(me, ">> Word wrap enabled for a size of: ",
                          itoa(me->wrap), "\r\n", NULL);
}


/* This toggles the hilite pointer */
void
togglehipointer(me)
        struct user *me;
{
        toggleflag(me, HIPOINTER);
        if (istoggled(me, HIPOINTER))
           writetext(me, ">> Hilite pointer on.\r\n");
        else
           writetext(me, ">> Hilite pointer off.\r\n");
}
/* End flag toggles */


/* This changes a users's name. */
void
changename(me)
	struct user    *me;
{
	char           *name;

	name = stripwhite(me->input + 2);
	if (!*name) {
	    writetext(me, ">> Please choose a REAL name!\r\n");
	    return;
	}

	/* We load or handled the name message in the db */
	if (verify_user(me, name))
		return;

	strncpy(me->name, name, NAMEMAX);
	me->name[NAMEMAX] = '\0';
	writelist(me, ">> Name changed to ", me->name, ".\r\n", NULL);
}


/* Current haven time */
void
gettime(me)
	struct user *me;
{  
	time_t now = time(0);
   
	writelist(me, ">> Current time is: ", ctime(&now), (char *)0 );
}


/* How long the haven's been up since last reboot */
void
uptime(me)
	struct user *me;
{    
    time_t now = time(0);

    writelist(me,   ">> Boot time: ", ctime(&boottime), NULL);
    sprintf(buffer, ">> Up time:   %s\r\n", timeform(now - boottime, T_LONG));
    writetext(me, buffer);
} 


/* List of connected users. Used for commands .f and .i# */
void
fingerpage(me, idle)
        struct user *me;
        int     idle;
{
        struct user *them;
        char entry[80], flag;
        int     n = 0, i = 0, offset;
        time_t  now = time(0);

        if (idle) {
            char        *ptr;

            ptr = (char *) stripwhite(me->input+2);
            n = atoi(ptr);
            if (!n)
                n = 300;
            else
                n *= 60;
        }

        titlebar(me, HAVENNAME);
        buffer[0] = '\0';

        for (them = userhead; them; them = them->next) {
             if (!idle || (idle && now - them->idle < n)) {

		 flag = '\0';
                 if (istoggled(them, HUSHED))
                     flag = 'H';
                 if (istoggled(them, PSUPPRESS))
                     flag = 'P';
                 if (istoggled(them, HUSHED) && istoggled(them, PSUPPRESS))
                     flag = 'Q';

                 sprintf(entry, " %3d%c%-*s ", them->line,
                         ket[them->level], NAMEMAX, them->name);

                 if (them->line <= 9)
                     offset = 2;
                 else if ((them->line >= 9) && (them->line <= 99))
                     offset = 1;
		 else
		     offset = 0;

                 if (flag == '\0') {
                     entry[offset] = bra[them->level];
                 } else {
		     if (offset)
                         entry[offset-1] = bra[them->level];
                     entry[offset] = flag;
                 } 

                 /* See if this line is done, then clear buffer */
                 if (strlen(buffer) + strlen(entry) > 80) {
                     writelist(me, buffer, "\r\n", NULL);
                     buffer[0] = '\0';
                 }

                 strcat(buffer, entry);
                 if (now - them->idle < n)
                        i++;
             }
         }

         if (*buffer)
             writelist(me, buffer, "\r\n", NULL);

         if (!idle)
             sprintf(buffer, " %d user%s connected ", numlogins,
                     numlogins == 1 ? "" : "s");
         else
             sprintf(buffer, " %d user%s idle less than %s ", i,
                     i == 1 ? "" : "s", timeform(n, T_LONG));

         titlebar(me, buffer);
}


/* Personal stats of user # */
void
fingeruser(me) 
	struct user *me;
{    
	struct user *them;
	int who;
	char *ptr, *logintime, flagbuf[80] = "";   
	time_t now = time(0);
     
	ptr = stripwhite(me->input + 2);
	who = atoi(ptr);
	them = findline(who);

	if (them == (struct user *)0) {
	   writetext(me, BAD_LINE);
           return;
        }

	/* WHAT A MESS */
        writetext(me, DASHLINE);
        sprintf(buffer, "Name: %-*s Email: %s\r\n", NAMEMAX, them->name,
		(*them->email) ? them->email : "???");
        writetext(me, buffer);

	sprintf(buffer, "Channel: %-*s      Url:   %s\r\n", CHANNELMAX, 
		them->channel->name, (*them->url) ? them->url : "???");
	writetext(me, buffer);

	logintime = ctime(&them->logintime);
	logintime[19] = '\0';
        sprintf(buffer, "On since %s on line %d", logintime, them->line); 
	writetext(me, buffer);

	if (me != them) {
	    sprintf(buffer, "\t %s idle\r\n",timeform(now-them->idle, T_SHORT));
            writetext(me, buffer);
	} else {
	    writetext(me, "\r\n");
	}

        if (istoggled(them, NEWMAIL))
            writetext(me, "New Mail.\r\n");
        else
            writetext(me, "No Mail.\r\n");

	sprintf(buffer, "From: %s\r\n", them->host);
	writetext(me, buffer);

        sprintf(buffer, "For: %s\r\n", timeform(now - them->logintime, T_LONG));
	writetext(me, buffer);

	if (istoggled(them, HUSHED))
            strcat(flagbuf, "Hushed, ");

	if (istoggled(them, PSUPPRESS))
            strcat(flagbuf,".p suppression, ");
        
	if (istoggled(them, HILITE) || istoggled(them, HIPOINTER))
            strcat(flagbuf, ".p Hilites, ");

	if (istoggled(them, ECHO))
            strcat(flagbuf, ".p echoes, ");

	if (istoggled(them, CHANMSGS))
	    strcat(flagbuf, "No Msgs, ");
	else if (istoggled(them, ARRMSGS))
	    strcat(flagbuf, "No Amsgs, ");

	if (istoggled(them, BEEPS))
	    strcat(flagbuf, "Beeps, ");

	if (istoggled(them, WRAP))
	    strcat(flagbuf, "Wrap, ");
   
	if (flagbuf[0]) {
	    flagbuf[(strlen(flagbuf)-2)] = ' '; 
	    writelist(me, flagbuf, "\r\n", NULL);
	}   
    
	writetext(me, DASHLINE);	
}


void
showversion(me)
	struct user *me;
{
	writelist(me, ">> VCD ", VERSION, " (", VERDATE, ")  ", 
		  "Developed by Angelo & Schrodinger.\r\n", (char *)0 );
}


char *
whostats(them)
	struct user *them;
{
	static 	char line[80] = "";
	char	*host;
	int	len;

	if ((len = strlen(them->host)) > SHOWHOSTMAX)
	    host = them->host + (len - SHOWHOSTMAX);
	else
	    host = them->host;

	sprintf(line, "   %2d %-*s %-*s %-6s %-*s\r\n", them->line,
                NAMEMAX, them->name, CHANNELMAX, them->channel->name,
                timeform(time(0) - them->idle, T_SHORT), SHOWHOSTMAX, 
		host);

	if (them->channel->owner == them && them->channel->lock) 
	    line[0] = '*';
	else if (them->channel->lock) 
	    line[0] = '+';

	if (istoggled(them, HUSHED)) 
	    line[1] = 'H';
	if (istoggled(them, PSUPPRESS)) 
	    line[1] = 'P';
	if (istoggled(them, HUSHED) && istoggled(them, PSUPPRESS)) 
	    line[1] = 'Q';

	if (istoggled(them, CHANMSGS)) 
	    line[2] = 'M';
	else if (istoggled(them, ARRMSGS)) 
	    line[2] = 'm';	
	
	return line;
}


void
wholist(me, substr)
	struct user 	*me;
	int		substr;	
{
    struct user *them;
    static char whoheader[80] = "";
    int who, found=0, names=0, onchannel=0, onhost=0;
    char *arg;

    arg = (char *) stripwhite(me->input+2);

    sprintf(whoheader, " Line %-*s %-*s %-6s %s\r\n", NAMEMAX, "Name", 
	    CHANNELMAX, "Channel", "Idle", "Site");
    
    writetext(me, whoheader);  

  /* Search for arg */
  if (*arg) {
      /*number search*/
      if ((who=atoi(arg))) 
          for (them = userhead; them; them=them->next) 
               if (who == them->line) {
                   found = 1;
                   writetext(me, whostats(them));
                   writetext(me, ">> 1 user found.\r\n");
                   break; 
               }

      /*name search*/
      for (them = userhead; them; them = them->next) {
           if (substr) {
	       if (matchstring(them->name, arg, 0)) {
                   names++;
                   writetext(me, whostats(them));
	       }
            } else if (!strcasecmp(arg, them->name)) {
                   names++;
                   writetext(me, whostats(them));
	    }
      }
	
      if (names) {
          sprintf(buffer, ">> %d name%s found.\r\n", names, names == 1 ? "" : "s");                     
          writetext(me, buffer);
      }

      /*channel search*/ 
      for (them = userhead; them; them = them->next) {
	   if (substr) {
               if (matchstring(them->channel->name, arg, 0)) {
                   onchannel++; 
		   writetext(me, whostats(them));
	       }
           } else if (!strcasecmp(arg, them->channel->name)) {
               onchannel++;
               writetext(me, whostats(them));
           }
      }

      if (onchannel) {
          sprintf(buffer, ">> %d user%s on channel.\r\n", onchannel,
                  onchannel == 1 ? "" : "s");
          writetext(me, buffer);
      }

      /*host search*/
      for (them = userhead; them; them = them->next) {
	   if (substr) {
	       if (matchstring(them->host, arg, 0)) {
		   onhost++;
		   writetext(me, whostats(them));
	       }	
           } else if (!strcasecmp(arg, them->host)) {
               onhost++;
               writetext(me, whostats(them));
           }
      }

      if (onhost) {
         sprintf(buffer, ">> %d user%s on host.\r\n", onhost, 
                 onhost == 1 ? "" : "s"); 
         writetext(me, buffer);
      }

      if (!found && !names && !onchannel && !onhost)
          writetext(me, ">> Nothing found.\r\n");
  
  /* Print the whole linked list */
  } else { 
         for (them = userhead; them; them = them->next)
              writetext(me, whostats(them));
        
         sprintf(buffer, ">> %d user%s found.\r\n", numlogins,
                 numlogins == 1 ? "" : "s");
         writetext(me, buffer);
  }

}   /* end wholist */


/* This function pages wholist output. command is .~1, .~2, etc. */
void
pagedwholist(me)
       struct user *me;
{
       struct user *them;
       char *arg;
       static char whoheader[80] = ""; 
       int pagelength = 18, i = 0, pageno = 0,
           numpages = 0, start = 0, stop = 0, tmp = numlogins;

       while (tmp > 0) {
            tmp = tmp - pagelength;
            numpages++;
       }
       if (tmp > 0) numpages++;  
     

       arg = (char *) stripwhite(me->input + 2);
       pageno = atoi(arg);

       if (!isdigit(*arg)) 
	   pageno = 1; 
       if (pageno > numpages) 
	   pageno = numpages;

       /* get page number to list */
       start = ((pageno * pagelength + 1) - pagelength);
       stop  = (pageno * pagelength);

       sprintf(whoheader, " Line %-*s %-*s %-6s %s\r\n", NAMEMAX, "Name", 
               CHANNELMAX, "Channel", "Idle", "Site");

       writetext(me, whoheader);

       /* list users from start to stop */
       for (them=userhead; them; them=them->next) {
            i++;
            if (i >= start && i <= stop) 
                writetext(me, whostats(them, me->level));
       }
       sprintf(buffer,  ">> Page %d of %d\r\n", pageno, numpages);
       writetext(me, buffer);
}  


void
listops(me)
	struct user *me;
{
	struct user *them;
	int	i=0;	

	titlebar(me, " Haven Ops ");
	sprintf(buffer, " Line %-*s %-6s %s\r\n", NAMEMAX, "Name", "Idle", "Level");
	writetext(me, buffer);
	for (them=userhead; them; them=them->next) {
	    if (them->level >= SYSOP) {	
	        sprintf(buffer, "   %2d %-*s %-6s %s\r\n", them->line,
		        NAMEMAX, them->name, 
			timeform(time(0) - them->idle, T_SHORT), 
			power_names[them->level]);
		if (me->level >= SYSOP && istoggled(them, OPHUSH))
		    buffer[0] = 'h';	
	        writetext(me, buffer);
		i++;
	    }
	}

	sprintf(buffer, " %d Op%s Connected ", i, i == 1 ? "" : "s");
	titlebar(me, buffer);
} 


void
set_colors(me)
        struct user *me;
{
        char    *ptr, s[80];
        int     num;

        ptr = (char *) stripwhite(me->input + 2);
        num = atoi(stripwhite(ptr+1));

        if (num < 0 || num > 13) {
                writetext(me, ">> Valid colors are 0 through 13.\r\n");
                return;
        }

        switch (*ptr) {
        case 'c':
                me->talkcolor = num;
                sprintf(s, ">> Normal channel text is now");
                break;
        case 'h':
		if (num == 0) {
		    clearflag(me, HILITE);
		    me->hicolor = 7;
		    writetext(me, ">> .p hilites off.\r\n");
		    return;
		}
                me->hicolor = num;
                if (nottoggled(me, HILITE)) toggleflag(me, HILITE);
                sprintf(s, ">> Hilights are now");
                break;
        case 'y':
                me->yellcolor = num;
                sprintf(s, ">> Yells are now");
                break;
	case '?':
		writelist(me, ">> Normal channel text is", colors[me->talkcolor], colorname[me->talkcolor], colors[0], "\r\n", NULL);
		writelist(me, ">> Your hilites are", colors[me->hicolor], colorname[me->hicolor], colors[0], "\r\n", NULL);
		writelist(me, ">> Your yells are", colors[me->yellcolor], colorname[me->yellcolor], colors[0], "\r\n", NULL);
		return;
		break; 
        case '\0':
                writetext(me, ">> Be more specific. .Cc, .Ch, Cy, or .C?.\r\n");
                return;
                break;
        default:
                writetext(me, ">> Unrecognized command. Type .? for help\r\n");
                return;
        }

        writelist(me, colors[num], s, colorname[num], colors[0], "\r\n", NULL);
}


void
viewtoggles(me)
	struct user *me;
{
	sprintf(buffer, ">> Auto-cap removal is %s\r\n", 
		is_system(AUTOCAPS) ? "on" : "off");
	writetext(me, buffer);

	sprintf(buffer, ">> Idle timeouts are %s\r\n",
		is_system(TIMEOUTS) ? "on":"off");
	writetext(me, buffer);

	sprintf(buffer, ">> Idle timeout length is: %s\r\n", 
			timeform(idletimeout, T_LONG)); 
	writetext(me, buffer);

	sprintf(buffer, ">> Next idle check is at: %s", ctime(&nextcheck));
	writetext(me, buffer);

	sprintf(buffer, ">> Nameservice: %s\r\n", 
		is_system(ASYNCHRODNS) ? "Asynchronous" : "Haven Based");
	writetext(me, buffer); 
}
