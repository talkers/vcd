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
 * This is db.c of vcd.  These functions deal with the manipulation of
 * the database.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "vcd.h"

/* local buffer for writing stuff */
static char buffer[512];

/*
 * Get the first character of a user's name.
 */ 
char
get_dir(name)
        char    *name;
{
        if (isalpha(*name))
            return tolower(*name);  /* letters */
        else if (isdigit(*name))
            return '#';             /* digits */
        else
            return '$';             /* symbols */
}


/*
 * Extract the password from a user's data file.
 */
char *
get_password(name)
	char	*name;
{
	FILE		*fptr;
	char 		lc[NAMEMAX], filename[PATHMAX];
	static char 	password[80];

	strcpy(lc, name);
	removecaps(lc);
	sprintf(filename, "%s/%c/%s", USERPATH, get_dir(lc), lc);

	fptr = fopen(filename, "r");
	fgets(password, 80, fptr);
	fclose(fptr);
	stripcr(password);

	/* "password " = 9 */
	return (password + 9);
}

/* 
 * Crypt a password with a random salt.
 */
char *
crypt_password(p)
        char    *p;
{
   char *c = "./abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	static char salt[2];      /* MUST be static */

	/* Seed the random number generator */
	srand(systemtime);

	/* Generate a random salt */
	salt[0] = c[rand() % 63];
	salt[1] = c[rand() % 63];
	salt[2] = '\0';

	return crypt(p, salt);
}


/*
 * Check to see if a user's datafile exists. If we find a file with a 
 * user's name, that means they are registered.
 */
int
user_exists(name)
	char	*name;
{
	struct stat 	fp;	
	char		filename[PATHMAX], lc[NAMEMAX];

	strcpy(lc, name);
	removecaps(lc);
	sprintf(filename, "%s/%c/%s", USERPATH, get_dir(lc), lc);

	if (!stat(filename, &fp))
		return 1;
	else
		return 0;
}


int
verify_user(me, name)
	struct user	*me;
	char		*name;
{
	char	*password, *saved;

	/* First check for a valid name */
	if (strchr(name, '/')) {
	    writetext(me, ">> Slashes are not allowed in user names.\r\n");
	    return 1;
	}

	if (*name == '.') {
	    writetext(me, ">> A username may not begin with a dot.\r\n");
	    return 1;
	}

        /* Check if they entered a password */
        if ((password = index(name, '=')) != (char *)0) {
            *password = '\0';
            password++;
            password = stripwhite(password);
            name     = stripwhite(name);

	    if (!user_exists(name))
		return 0;

	    saved = get_password(name);
	    if (!strcmp(saved, crypt(password, saved))) {
		load_user(me, name, L_LOGIN);
		return 1;
	    } else {
		writetext(me, ">> Invalid password attempt!\r\n");
		return 1;
	    }
        } else {
	    if (user_exists(name)) {
		writetext(me, ">> That is a reserved name.\r\n");
		return 1;
	    } else if (user_exists(me->name)) {
		/* Registered user changing their name */
		init_user(me);
		return 0;
	    } else {
		return 0;
	    }
	}
}


/*
 * Load a user's data file information into their user structure.
 */
void
load_user(u, name, login)
	struct user 	*u;
	char		*name;	
	int		login;
{
	FILE	*fptr;
	char    filename[PATHMAX], lc[NAMEMAX];

	strcpy(lc, name);
	removecaps(lc);
	sprintf(filename, "%s/%c/%s", USERPATH, get_dir(lc), lc);

        fptr = fopen(filename, "r");
	if (!fptr) {
		writetext(u, ">> Your user file doesnt exist!!!!\r\n");
		return;
	}

        fgets(buffer, 255, fptr);
        while (!feof(fptr)) {
                stripcr(buffer);
                if (!strncmp("name ", buffer, 5)) {
                    if (login)
                        strncpy(u->name, name, NAMEMAX);
                    else
                        strncpy(u->name, buffer+5, NAMEMAX);
                } else if (!strncmp("password ", buffer, 9)) {
                        strncpy(u->pword, buffer+9, PASSMAX);
                } else if (!strncmp("flags ", buffer, 6)) {
                        u->flags = (unsigned)atol(buffer+6);
                } else if (!strncmp("level ", buffer, 6)) {
                        u->level = atoi(buffer+6);
                } else if (!strncmp("wrap ", buffer, 5)) {
                        u->wrap = atoi(buffer+5);
                } else if (!strncmp("hicolor ", buffer, 8)) {
                        u->hicolor = atoi(buffer+8);
                } else if (!strncmp("talkcolor ", buffer, 10)) {
                        u->talkcolor = atoi(buffer+10);
                } else if (!strncmp("yellcolor ", buffer, 10)) {
                        u->yellcolor = atoi(buffer+10);
                } else if (!strncmp("email ", buffer, 6)) {
                        strncpy(u->email, buffer+6, EMAILMAX);
                } else if (!strncmp("url ", buffer, 4)) {
                        strncpy(u->url, buffer+4, URLMAX);
                } else if (!strncmp("laston ", buffer, 7)) {
			u->laston = (time_t)atol(buffer+7);
		}

                fgets(buffer, 255, fptr);
        }
        fclose(fptr);	

        if (login) {
            writelist(u, ">> Name set to: ", u->name, "\r\n", NULL);
            writetext(u, ">> Flags toggled.\r\n");
            if (u->level >= SYSOP)
                writelist(u, ">> Your power level: ", power_names[u->level], "\r\n", NULL);
            if (istoggled(u, NEWMAIL))
                writetext(u, "\033[1mYOU HAVE NEW MAIL.\033[0m\r\n");
        }
}


/*
 * Dump a user's information to their datafile.
 * Their password must be written to the file first! 
 */ 
void
save_user(me, update, msg)
        struct user *me;
        int update;
        char *msg;
{
        FILE *fptr;
        char filename[PATHMAX], lc[NAMEMAX];
        time_t now = time(0);

	strcpy(lc, me->name);
	removecaps(lc);
        sprintf(filename, "%s/%c/%s", USERPATH, get_dir(lc), lc);

        fptr = fopen(filename, "w");
	fprintf(fptr, "password %s\n", me->pword);
        fprintf(fptr, "name %s\n", me->name);
        fprintf(fptr, "flags %lu\n", me->flags);
        fprintf(fptr, "level %d\n", me->level);
        fprintf(fptr, "wrap %d\n", me->wrap);
        fprintf(fptr, "hicolor %d\n", me->hicolor);
        fprintf(fptr, "talkcolor %d\n", me->talkcolor);
        fprintf(fptr, "yellcolor %d\n", me->yellcolor);

        if (*me->email)
                fprintf(fptr, "email %s\n", me->email);

        if (update)
                fprintf(fptr, "laston %lu\n", now);
        else
                fprintf(fptr, "laston %lu\n", me->laston);

        if (*me->url)
                fprintf(fptr, "url %s\n", me->url);

        fclose(fptr);

        if (*msg)
            writetext(me, msg);
}


/* 
 * Set a password if we don't have one.
 */
void
set_password(me)
        struct user *me;
{
        char    *verify, *ptr = (char *)stripwhite(me->input + 2); /* .P */

	if ((verify = index(ptr, '=')) != (char *)0) {
	    *verify = '\0';
	    verify++;
	    verify = stripwhite(verify);
	    ptr = stripwhite(ptr);

	    if (!*ptr || !*verify) {
		writetext(me, ">> Syntax is .P <password> = <password>\r\n");
		return;
	    }

	    if (strcmp(ptr, verify)) {
		writetext(me, ">> Passwords don't match. Password not changed.\r\n");
		return;
	    } 

	    strncpy(me->pword, crypt_password(ptr), PASSMAX);
	    me->pword[PASSMAX] = '\0';
	    writetext(me, ">> Password set.\r\n");

	} else {
	    writetext(me, ">> Syntax is .P <password> = <password>\r\n");
	}
}


/*
 * Attempt to register a new user.
 */
void
register_user(me)
	struct user *me;
{
	char		*ptr = stripwhite(me->input + 5);  /* .@reg */
	int		who;
	struct user	*them;

	who = atoi(ptr);
	them = findline(who);
	if (!them) {
	    writetext(me, BAD_LINE);
	    return;
	}

	if (!strcmp(them->name, DEFAULTNAME)) {
	    writetext(me, ">> That user must have a REAL NAME before being registered.\r\n");
	    return;
	}

	if (!*them->pword) {
	    writetext(me, ">> That user has not set a valid password.\r\n");
	    writetext(them, ">> You must first set a password before being registered.\r\n" \
	                    ">> Use .P <password> = <password> to set one for yourself.\r\n");
	    return;
	}

	if (user_exists(them->name)) {
	    writetext(me, ">> That user is already registered.\r\n");
	    return;
	} 

	sprintf(buffer, "%s registered %s", me->name, them->name);
	log(buffer, REGLOG);

	save_user(them, S_NOUPDATE, ">> You've just been registered!\r\n");
	writetext(them, ">> Next time you log in type \033[1m.n<name>=<password>\033[0m\r\n");
	writelist(me, ">> You just registered ", them->name, ".\r\n", NULL);
} 

/*
 * Delete a user's data file and mail file; they've just been nuked!
 */
void
nuke_user(me)
	struct user *me;
{
	struct user *nuked, *them;
	char	*name = stripwhite(me->input + 6);  /* .@nuke */
	char	lc[NAMEMAX], userfile[PATHMAX], mailfile[PATHMAX]; 

	if (!*name) {
		writetext(me, ">> Syntax is .@nuke <name>.\r\n");
		return;
	}

	if (!user_exists(name)) {
		writetext(me, ">> No such user to nuke.\r\n");
		return;
	}

	strcpy(lc, name);
	removecaps(lc);
	sprintf(userfile, "%s/%c/%s", USERPATH, get_dir(lc), lc);
	sprintf(mailfile, "%s/%c/%s", MAILPATH, get_dir(lc), lc);

        nuked = (struct user *) malloc(sizeof(struct user));
        if (nuked == (struct user *)0) {
            logerr("malloc failed", SYSLOG);
            return;
        }

        load_user(nuked, name, L_LOAD);
        if (me->level > nuked->level) {
            sprintf(buffer, "%s nuked %s", me->name, name);
            log(buffer, NUKELOG);
            writelist(me, ">> You just nuked ", name, ".\r\n", NULL);
        } else {
            writelist(me, ">> You don't have sufficient power to nuke ",
		      name, ".\r\n", NULL);
	    FREE(nuked);
	    return;
        }
        FREE(nuked);

        /* Delete the user file */
        unlink(userfile);
        /* Delete the mail file */
        unlink(mailfile);

	/* See if the user is logged on. */
	for (them  = userhead; them; them = them->next)
	     if (!strcasecmp(them->name, name)) {
		 getout(them, "has just been NUKED!\r\n");
		 writelist(them, "\r\n\r\n>> You've just been nuked by ", 
			   me->name, "!!!\r\n\r\n", NULL);
		 disconnect(them, QUIT_NORMAL);
	     }
} 
	    
	
/*	
 * Finger a user who is not logged on.
 */
void
finger_user_name(me)
	struct user	*me;
{
	char *name = stripwhite(me->input + 2);  /* .F */
	struct user *saved, *them;
	
	if (!*name) {
	    writetext(me, ">> Syntax is .F <name>\r\n");
	    return;
	}

	/* See if the user is online; if they are force me to do .f# */
	for (them = userhead; them; them = them->next)
	     if (!strcasecmp(them->name, name)) {
		 sprintf(me->input, ".f%d", them->line);
		 getcommand(me);
		 return;
	     } 

	if (!user_exists(name)) {
	    writelist(me, ">> User '", name, "' was not found in the database.\r\n", NULL);
	    return;
	}
	    
	saved = (struct user *) malloc(sizeof(struct user));
	if (saved == (struct user *)0) {
	    logerr("malloc failed", SYSLOG);
	    return;
	}
	init_user(saved);
	load_user(saved, name, L_LOAD);

	writetext(me, DASHLINE);

	sprintf(buffer, "Name: %-*s     Email: %s\r\n", NAMEMAX, saved->name,
		(*saved->email) ? saved->email : "???");
	writetext(me, buffer);

	sprintf(buffer, "Level: %-*s    Url: %s\r\n", NAMEMAX, 
		power_names[saved->level], (*saved->url) ? saved->url : "???");
	writetext(me, buffer);

	sprintf(buffer, "Last log in %s", ctime(&saved->laston));
	writetext(me, buffer);
 
	if (istoggled(saved, NEWMAIL))
	    writetext(me, "New Mail.\r\n");
	else
	    writetext(me, "No Mail.\r\n"); 

	writetext(me, DASHLINE);

	FREE(saved);
}

/*
 * Create the root user as defined by ROOTNAME upon boot up if it doesn't
 * exist. This is important as root is the first user to get power.
 */
void
create_root()
{
	char		*ptr, password[PASSMAX];
	struct user	*root;

	ptr = password;
	repeat:
	printf("\033[1mENTER A PASSWORD FOR %s:\033[0m ", ROOTNAME);
	fgets(password, 80, stdin);
	stripcr(password);
	ptr = stripwhite(ptr);
	if (*ptr == '\0')
		goto repeat;

	root = (struct user *) malloc(sizeof(struct user));
	if (root == (struct user *)0) {
	    logerr("malloc failed", SYSLOG);
	    return;
	}

	strncpy(root->name, ROOTNAME, strlen(ROOTNAME));
	strncpy(root->pword, crypt_password(ptr), PASSMAX);
	root->level = ROOT;
	
	save_user(root, S_UPDATE, "");
	FREE(root);

	printf("\033[1mROOT CREATED.\033[0m\n\n");
}


void
set_email(me)
        struct user *me;
{
        char *email = stripwhite(me->input + 2); /* .E */

        strncpy(me->email, email, EMAILMAX);
        me->email[EMAILMAX] = '\0';
        writelist(me, ">> Email set to: ", me->email, "\r\n", NULL);
}

void
set_url(me)
        struct user *me;
{
        char    *url = stripwhite(me->input + 2);  /* .U */

        strncpy(me->url, url, URLMAX);
        me->url[URLMAX] = '\0';
        writelist(me, ">> Url set to: ", me->url, "\r\n", NULL);
}

/* Only info that is loaded in from load_user() should be initialized here */
void
init_user(u)
        struct user *u;
{
        u->level = JOEUSER;
        u->pword[0] = '\0';
        u->email[0] = '\0';
        u->url[0] = '\0';
        u->wrap = 80;
}


void
purge_users(me)
	struct user	*me;
{
	char		*DIRS = "#$abcdefghijklmnopqrstuvwxyz", path[PATHMAX];
	char		userfile[PATHMAX], mailfile[PATHMAX];
        DIR             *dirp;
        struct dirent   *de;
        int 		i, count = 0, purged = 0;
        struct user     *u;
        time_t          now = time(0);


	sprintf(buffer, "PURGE start by %s", me->name);
	log(buffer, NUKELOG);

        u = (struct user *) malloc(sizeof(struct user));

        for (i = 0; DIRS[i] != '\0'; i++) {
             sprintf(path, "%s/%c", USERPATH, DIRS[i]);
             dirp = opendir(path);

             for (de = readdir(dirp); de != NULL; de = readdir(dirp))
                  if (*de->d_name != '.') {
                      load_user(u, de->d_name, L_LOAD);
		      count++;
                      if (u->level <= JOEUSER && now-u->laston > PURGE_DAYS) {
		    	  sprintf(userfile, "%s/%c/%s", USERPATH, get_dir(u->name), u->name);
			  sprintf(mailfile, "%s/%c/%s", MAILPATH, get_dir(u->name), u->name);
			  unlink(userfile);
			  unlink(mailfile);
			  sprintf(buffer, "%s nuked during a purge", u->name);
                          log(buffer, NUKELOG);
			  purged++;
		      }
                  }
             closedir(dirp);
        }

	sprintf(buffer, "Purge Complete. %d of %d users purged. %d users left.", purged,
		count, count - purged);
	log(buffer, NUKELOG);
	writelist(me, ">> ", buffer, "\r\n", NULL);
}
