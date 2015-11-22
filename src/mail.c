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
 * This is mail.c of vcd. The haven-mail system functions are located here. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "vcd.h"

int
countmail(me)
	struct user *me;
{
	FILE	*fptr;
	int	n = 0;
	char	lc[NAMEMAX], filename[PATHMAX], buffer[INPUTMAX];

	strcpy(lc, me->name);
	removecaps(lc);
        sprintf(filename, "%s/%c/%s", MAILPATH, get_dir(lc), lc);

	fptr = fopen(filename, "r");
	fgets(buffer, INPUTMAX, fptr);
	while (!feof(fptr)) {
		n++;
		fgets(buffer, INPUTMAX, fptr);
	}
	fclose(fptr);

	return n;
}


void
append_mail_spool(me, name, message)
	struct user 	*me;
	char		*name, *message;
{
	FILE	*fptr;
	char	lc[NAMEMAX], filename[PATHMAX], *timestr;
	time_t  now = time(0);


	strcpy(lc, name);
	removecaps(lc);
	sprintf(filename, "%s/%c/%s", MAILPATH, get_dir(lc), lc);

	timestr = ctime(&now) + 4;
	timestr[12] = '\0';

	fptr = fopen(filename, "a");
	       fprintf(fptr, "(%s) [%s]: %s\r\n", me->name, timestr, message);
	fclose(fptr);
}


void
notify_new_mail(me, name)
	struct user  *me;
	char 	     *name;
{
	struct user *them;

	/* Check if user is on */
	for (them = userhead; them; them = them->next) 
	     if (!strcasecmp(them->name, name)) {
		 writelist(them, ">> NEW MAIL from ", me->name, ".\r\n", NULL);
                 setflag(them, NEWMAIL);
	     	 return;
	     }

	/* User not on, set NEWMAIL flag */
        them = (struct user *) malloc(sizeof(struct user));
	if (them == (struct user *)0) {
	    logerr("malloc failed", SYSLOG);
	    return;
	}
        load_user(them, name, L_LOAD);
	setflag(them, NEWMAIL);
	save_user(them, S_NOUPDATE, "");
	FREE(them);
}	




void
sendmail(me)
        struct user *me;
{
        char    *name, *message;

        name = (char *) stripwhite(me->input + 2);  /* .S */

        if ((message = index(name, '=')) == (char *)0) {
            writetext(me, ">> Syntax is: .S <name> = <message>\r\n");
            return;
        } else {
            *message= '\0';
            message++;
            message = stripwhite(message);
            name    = stripwhite(name);

            if (!user_exists(name)) {
                writetext(me, ">> No such user to send mail to!\r\n");
                return;
	    }
	}	

	append_mail_spool(me, name, message);
	notify_new_mail(me, name);
	writelist(me, ">> Mail sent to ", name, ".\r\n", NULL);
}


void
readmail(me)
	struct user *me;
{
	FILE *fptr;
	int i = 0, mailnum = 1;
	char *arg, lc[NAMEMAX], title[80], buffer[INPUTMAX], path[PATHMAX];

	arg = (char *) stripwhite(me->input + 2);  /* .R */
	mailnum = atoi(arg);

	if (*arg) {
	    if ( !(mailnum = atoi(arg)) ) {
                writetext(me, ">> Invalid mail number!\r\n");
                return;
	    }
        }

	strcpy(lc, me->name);
	removecaps(lc);
	sprintf(path, "%s/%c/%s", MAILPATH, get_dir(lc), lc);
	fptr = fopen(path, "r");
	if (!fptr) {
	    writetext(me, ">> You have no mail.\r\n");
	    return;
	}
	
	/* Clear NEWMAIL bit */
	clearflag(me, NEWMAIL);

	/* Read all messages */
	if (!*arg) {
	    sprintf(title, " You have %d mail messages ", countmail(me));
	    titlebar(me, title);
            while (!feof(fptr)) {
                   if (fgets(buffer, INPUTMAX, fptr) == (char *)0) break;
		   i++;
		   writelist(me, itoa(i), ": ", (char *)0 );
		   writewrap(me->line, 78 - strlen(itoa(i)) - 2, buffer);
		   writetext(me, DASHLINE); 
            }
	    fclose(fptr);
	} else {
            /* Read single message */
            while (!feof(fptr)) {
                   if (fgets(buffer, INPUTMAX, fptr) == (char *)0) break;
		   i++;
                   if (i == mailnum) {
		       sprintf(title, " Mail Message #%d ", i);
		       titlebar(me, title);
                       writewrap(me->line, 78, buffer);
		       writetext(me, DASHLINE);
                       fclose(fptr);
                       return;
                   }
             }
             writelist(me, ">> You only have ", itoa(i), " messages.\r\n", NULL);
	     fclose(fptr);	
	}
}


void
deletemail(me)
	struct user *me;
{
	FILE	*fptr, *new;
	char	*ptr, lc[NAMEMAX], f1[PATHMAX], f2[PATHMAX], str[2048];
	int	num, all = 0, i = 0, total;
		
	ptr = (char *) stripwhite(me->input + 2);  /* .D */
	if (!strcmp(ptr, "all")) 
		all = 1;
	num = atoi(ptr);

	if (!num && !all) {
	    writetext(me, ">> Invalid mail number!\r\n");
	    return;
	}

        /* check for mail file existance */
	strcpy(lc, me->name);
	removecaps(lc);
	sprintf(f1, "%s/%c/%s", MAILPATH, get_dir(lc), lc);
	fptr = fopen(f1, "r");
	if (!fptr) {
	    writetext(me, ">> You have no mail.\r\n");	
	    return;
	} 
	fclose(fptr);

	total = countmail(me);
	if ( (num > total) || (num < 1)) {
	    writelist(me, ">> You only have ", itoa(total), " messages.\r\n", NULL);
	    return;
	}

	if (all) {	
	    unlink(f1);
	    writetext(me, ">> All mail erased!\r\n");
	/* special case...mail #1 is only message...delete spool */
	} else if (total == 1 && num == 1) {
	    all = 1;
	    unlink(f1);
	    writetext(me, ">> Mail (1) deleted.\r\n");
	} else {
	    sprintf(f2, "%s/%c/%s.tmp", MAILPATH, get_dir(lc), lc);
	    rename(f1, f2);
	    fptr = fopen(f2, "r");
	    new  = fopen(f1, "w");

	    fgets(str, INPUTMAX, fptr);
	    while (!feof(fptr)) {
	           i++;	
	           if (i != num)
	               fprintf(new, "%s", str);
		   fgets(str, INPUTMAX, fptr);
	    }
	    fclose(new);
	    fclose(fptr);
	    unlink(f2);

	    writelist(me, ">> Mail (", itoa(num), ") deleted.\r\n", NULL);
	}	 		 
	
	/* check if we deleted all mail and NEWMAIL bit is still set */
	if (all) 
	    clearflag(me, NEWMAIL);
}
