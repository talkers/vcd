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
 * This is lists.c of vcd.  This file contains the functions which handle the
 * structures found in lists.h
 */

#include <stdio.h>		/* For fgets(), FILE, fopen(), sprintf(). */
#include <stdlib.h>		/* For exit(). */
#include <string.h>		/* For strlen(), strstr(). */
#include <sys/time.h>
#include <sys/types.h>		/* For type fd_set, FD_CLR(). */
#include <unistd.h>		/* For write(). */
#include "vcd.h"

/* local buffer for writeing stuff */
static char	buffer[512];	

/* global variables */
int             numlogins = 0,
		nextid    = 1;

struct user    *userhead = (struct user *) 0,
               *usertail = (struct user *) 0;

struct badsite *sitehead = (struct badsite *) 0,
               *sitetail = (struct badsite *) 0;

/* struct user related stuff. */
/*
 * This initializes a user's structure and puts them onto the linked list of
 * users. Returns back the user's structure on success and 0 on failure.
 */
struct user *
adduser()
{
	struct user    *uptr;

	/* Add a new user to the users linked list. */
	if (userhead == (struct user *) 0) {
		/* case if there was previously no users logged on */
		userhead = (struct user *) malloc(sizeof(struct user));
		if (userhead == (struct user *) 0) {
			logerr("malloc failed", SYSLOG);
			return (struct user *) 0;
		}
		userhead->next = (struct user *) 0;
		usertail = userhead;
	} else {
		/* case where there's already users logged on */
		uptr = (struct user *) malloc(sizeof(struct user));
		if (uptr == (struct user *) 0) {
			logerr("malloc failed", SYSLOG);
			return (struct user *) 0;
		}
		uptr->next = (struct user *) 0;
		usertail->next = uptr;
		usertail = uptr;
	}
	return usertail;
}


/*
 * This deallocates a deaduser's structure and takes them off the linked list
 * of users. It returns back a pointer to the deaduser->next's structure.
 */
struct user *
disconnect(deaduser, quit_type)
	struct user    	*deaduser;
	int		quit_type;
{
	struct user    *uptr;

	/* Don't save the NOCAPS flag if the database is in use. */
	clearflag(deaduser, NOCAPS);

        /* Check to see if data should be saved. */
        if (user_exists(deaduser->name)) {
		if (quit_type == QUIT_NORMAL) {
		    sprintf(buffer, ">> Saving user %s.\r\n", deaduser->name);
		    save_user(deaduser, S_UPDATE, buffer);
		} else
		    save_user(deaduser, S_UPDATE, NULL);
	}

	/* Send the exit message! */
	if (quit_type == QUIT_NORMAL)
	    writetext(deaduser, EXITMSG);

	/* Ungag this line. */
	for (uptr = userhead; uptr; uptr = uptr->next)
		FD_CLR(deaduser->line, &uptr->gags);

	/* Remove deaduser from the users linked list. */
	if (deaduser == userhead) {
		/* Case where deaduser is first on the users linked list. */
		userhead = userhead->next;

		/* Check if deaduser is the only user logged on. */
		if (usertail == deaduser)
			usertail = userhead;

	} else {
		/*
		 * Case where deaduser is not first on the users linked list.
		 */

		/*
		 * Find where the user structure before deaduser is in the
		 * users linked list.
		 */
		for (uptr = userhead; uptr && uptr->next != deaduser;
		     uptr = uptr->next);

		uptr->next = deaduser->next;

		/* Check if deaduser is also the usertail. */
		if (deaduser == usertail)
			usertail = uptr;

	}	/* End of removing deaduser from the users linked list */

	uptr = deaduser->next;
	close(deaduser->line);
	fd_master_clear(deaduser->line);

        /* remove user from channel */
        if (--(deaduser->channel->count)) {
	    struct user *them;

            if (deaduser->channel->owner == deaduser) {
		deaduser->channel->owner = (struct user *)0;
                deaduser->channel->lock = C_UNLOCKED;

                for (them = userhead; them; them = them->next)
                     if (them->channel == deaduser->channel) 
                         writetext(them, ">> Channel has been unlocked.\r\n");

	    }
 
        } else
	    delete_channel(deaduser->channel);
      

	FREE(deaduser);
	numlogins--;

	return uptr;
}


/*
 * This searches for a user structure in the linked list with a particular
 * line number.  It returns a pointer to the particular user's structure on
 * success and a null pointer on failure.
 */
struct user *
findline(who)
	int             who;
{
	struct user    *ptr;

	for (ptr = userhead; ptr; ptr = ptr->next) {
		if (ptr->line == who)
			return ptr;
	}
	return (struct user *) 0;
}


/* struct badsite releated stuff. */
/* This loads in banned sites in the BANFILE file. */
void
loadban()
{
	FILE           *fptr;
	char           *buffptr;
	struct badsite *bptr = sitehead;

	fptr = fopen(BANFILE, "r");
	if (fptr == (FILE *) 0) {
		logerr("BANFILE doesn't exist", SYSLOG);
		return;
	}

	while (!feof(fptr)) {
		if (fgets(buffer, 255, fptr) == (char *)0) break;
		stripcr(buffer);
		buffptr = stripwhite(buffer);

		/* Ignore blank lines */ 
		if (*buffptr == '\0')
			continue;

		bptr = (struct badsite *) malloc(sizeof(struct badsite));
		if (bptr == (struct badsite *) 0) {
			log("siteban could not be loaded", SYSLOG);
			logerr("malloc failed", SYSLOG);
			continue;
		}

		if (!sitehead) {
			sitehead = bptr;
			sitetail = sitehead;
		} else {
			sitetail->next = bptr;
			sitetail = bptr;
		}
		sitetail->next = (struct badsite *)0;

		strncpy(sitetail->site, buffptr, HOSTMAX);
		if (fgets(buffer, 255, fptr) == (char *)0) break;
		stripcr(buffer);
		sitetail->banover = (time_t)atol(buffer);
		sitetail->id = nextid++;

	}
	fclose(fptr);
}


/*
 * Checks if a host is banned. Returns a 1 if the host is banned and 0 if
 * it's not.
 */
int
checkban(sitename)
	char           *sitename;
{
	struct badsite *ptr;

	for (ptr = sitehead; ptr; ptr = ptr->next) {
	     /* check if ptr->site is a substring of sitename */
	     if (matchstring(sitename, ptr->site, 0)) {
		 /* Check if expire time is up */
		 if (ptr->banover == 0) {
		     return 1;				/* Perm ban */
		 } else if (time(0) > ptr->banover) {	
		     bandel(NULL, ptr->id);
		     return 0;				/* Ban over */
		 } else
		     return 1;				/* Ban not over */
	     }
	}
	return 0;					/* Not on ban list */
}


/* This shows all the sites banned. */
void
showban(me)
	struct user    *me;
{
	struct badsite *bptr;

	/* Check if there's any banned sites. */
	if (sitehead == (struct badsite *) 0) {
		writetext(me, ">> No banned sites.\r\n");
		return;
	}
        
	writetext(me, ">>  ID  Banned site\t\t\t\t\t   Expires\r\n");
	for (bptr = sitehead; bptr; bptr = bptr->next) {
	     sprintf(buffer, ">> %3d  %-*s %s", bptr->id, HOSTMAX, 
                     bptr->site, (bptr->banover == 0) ? "Never\r\n" :
                     ctime(&bptr->banover)+4);
	     writetext(me, buffer);
	}
}


struct badsite *
findsite(id)
        int	id;
{
        struct badsite *b;

        for (b = sitehead; b; b = b->next) {
            if (b->id == id)
               return b;
        }
        return (struct badsite *)0;
}


void
banadd(me)
	struct user *me;
{
	struct badsite *new;
        char *ptr = (char *)stripwhite(me->input + 3);
        FILE *fptr;
	int when, len;
	time_t now = time(0); 

	if (*ptr == '\0') {
		showban(me);
		return;
	}

	when = getsecs(ptr);
	if (!when)
	    now = 0;
	else
	    when += now;

        new = (struct badsite *) malloc(sizeof(struct badsite));
	if (new == (struct badsite *)0) {
	    logerr("malloc failed", SYSLOG);
	    return;
	}

        if (!sitehead) {
            sitehead = new;
	    sitetail = sitehead;
	} else {
 	    sitetail->next = new;
	    sitetail = new;
        }
        sitetail->next = (struct badsite *)0;

	/* Chop host name to the LAST HOSTMAX characters. */ 
	if ((len=strlen(ptr)) > HOSTMAX)
		ptr = ptr + (len - HOSTMAX);

        strncpy(new->site, ptr, HOSTMAX);
	new->banover = when;
	new->id = nextid++;
      
	writelist(me, ">> ", ptr, " added to the ban list.\r\n", NULL);
	sprintf(buffer, "%s banned %s", me->name, ptr);
	log(buffer, BANLOG);

        fptr = fopen(BANFILE, "a");
             fprintf(fptr, "%s\n", new->site);
	     fprintf(fptr, "%ld\n", new->banover);
        fclose(fptr); 
}


void
bandel(me, siteid)
	struct user 	*me;
	int		siteid;
{
	struct badsite *b = (struct badsite *)0, *del = (struct badsite *)0;
	FILE *fptr;
	char *ptr;	

	/* If a user is unbanning a site */
	if (me) {
	    ptr = stripwhite(me->input + 3);
	    siteid = atoi(ptr);
	    del = findsite(atoi(ptr));
	    if (!del) {
		writetext(me, ">> No such site id to delete!\r\n");
		return;
	    }
	} else 
            /* Else the haven is unbanning a site thruogh checkban() */
	    del = findsite(siteid);  

	if (del == sitehead) {
	    sitehead = sitehead->next;
	} else {
	    for (b = sitehead; b && b->next != del; b = b->next);
	         b->next = del->next;
	}
        if (del == sitetail) 
            sitetail = b;

	if (me) {
            writelist(me, ">> ", del->site, " removed from ban list.\r\n", NULL);
            sprintf(buffer, "%s unbanned %s", me->name, del->site);
            log(buffer, BANLOG);
	}

	FREE((struct badsite *) del);

	/* Write banned sites back to the file. */
	fptr = fopen(BANFILE, "w");	
	for (b = sitehead; b; b = b->next) {
		fprintf(fptr, "%s\n", b->site);
		fprintf(fptr, "%ld\n", b->banover);
	}
       	fclose(fptr);
}


int
getsecs(ptr)
	char	*ptr;
{
	char	*s;

	for (s=ptr; *s; s++) 
	     if (*s == ':') 
		break;

	if (*s) {
	    *s = '\0';
	    return atoi(s+1);
	} else
	    return 0;	
}
