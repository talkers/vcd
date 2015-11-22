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
 * This is lists.h of vcd.  This contains the structures which are used for
 * the users, channels, and sitebans.
 */

/* system headers */
#include <sys/types.h>		/* For type fd_set. */

/* quit types */
#define QUIT_NORMAL	0	/* Quit, spammed out, killed */
#define QUIT_DISCONNECT 1	/* Read error, don't print exit&save message */

/* local constants */
#define INPUTMAX	1024	/* Maximum input buffer size.    */
#define HOSTMAX		50	/* Maximum hostname length.      */
#define SHOWHOSTMAX  	31	/* Maximum hostname we can fit on the screen */
#define NAMEMAX		20	/* Maximum login name length.    */
#define PASSMAX         20      /* Maximum password length       */ 
#define EMAILMAX        42      /* Maximum email adddress length */
#define URLMAX		44	/* Maximum url length		 */

/* user flags */
#define GOTCR		0x0001	/* Did we find a carriage return? */
#define TYPING		0x0002	/* Is the user still typing? */
#define HUSHED		0x0004	/* Are we blocking yells? */
#define ARRMSGS		0x0008	/* Are we blocking new arrival messages? */
#define CHANMSGS        0x0010	/* Are we blocking channel messages? */
#define ECHO		0x0020	/* Do we want .p echoes? */
#define HILITE		0x0040	/* Do we want .p hilites? */
#define BEEPS		0x0080	/* Do we want .p beeps? */
#define PSUPPRESS       0x0100	/* Are we blocking privates? */
#define NOCAPS          0x0200	/* Did we lose our caps? Fuckwit! */
#define WRAP            0x0400	/* Are we on a LAME terminal? */
#define HIPOINTER       0x0800	/* Are we on an even LAMER terminal? */
#define OPHUSH		0x1000	/* Are we blocking the op channel? */
#define NEWMAIL		0x2000	/* Does anyone out there like us??? */


/* global structures and variables */
/* users */
struct user {
	char            input[INPUTMAX+1],	 /* Input buffer */
	               *queue,	                 /* Pointer to input position */
	                host[HOSTMAX+1], 	 /* Hostname */
	                name[NAMEMAX+1], 	 /* User's name */
                        pword[PASSMAX+1],    	 /* User's password */
                        email[EMAILMAX+1],     	 /* User's email address */
			url[URLMAX+1];		 /* User's url */

        struct channel *channel;    /* User's channel structure */

	int             line,	    /* File descriptor */
	                level,	    /* Power level */
	                length,	    /* Length of text in the input buffer */
	                numlines,   /* Number of lines received */
			commands,   /* Spam protection for .y, .c, .p, etc */
			wrap,       /* Word wrap width */
                        hicolor,    /* Hilights color */ 
                        yellcolor,  /* Yell text color */
                        talkcolor,  /* Normal channel talking color */
			lastp;	    /* User last .p was sent to */

        unsigned long	flags;      /* User flags */

	time_t		laston,	    /* Last time they logged out */
			idle,	    /* Time user last typed something */
	                logintime;  /* Time user logged in. */

	fd_set		gags;	    /* Line gags */

	long            port;	    /* Port on their host */
	struct user    *next;	    /* Next user on the linked user list */
};


/* sitebans */
struct badsite {
	char            site[HOSTMAX+1];   /* Banned host pattern */
	int		id;	   	   /* Site id */
	time_t		banover;           /* Time ban is over */ 	
	struct badsite *next;	           /* Next badsite on the list */
};


struct user		*adduser();
struct user		*disconnect();
struct user		*findline();
void			loadban();
int			checkban();
void			showban();
struct badsite		*findsite();
void			banadd();
void			bandel();
int			getsecs();	


extern struct user	*userhead, *usertail;
extern struct badsite	*sitehead, *sitetail;
extern int		numlogins;
