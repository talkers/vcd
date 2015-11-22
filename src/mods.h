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


/* IF YOU RENAME THIS, YOU MUST RENAME THE RESOLVER MOD ACCORDINGLY!!! */
#define RESOLVER        "resolver"


/* Event IDs */
#define E_NONE          0
#define E_RESOLVE       1
#define E_NEW_USER      2
#define E_QUIT          3
#define E_IDENT         4
#define E_MAX_EVENTS    5


/* Module structure */
struct module {
        int             id,             /* Id of module */
                        fd,             /* Module's file descriptor */
                        pid;            /* Module's PID */
        fd_set          events;         /* eventS this module grabs */
        char            *name;          /* Filename of module */
        struct channel  *channel;       /* Channel the module resides in */
        struct module   *next;          /* Next module on the list */
};


/* Parse string structure */
struct pstring {
        char            *str,           /* Command to grab */
                        *fmt;           /* What args does the module need? */
        struct module   *mod;           /* Module associated with the pstr */
        struct pstring  *next;          /* Next pstring on the list */
};


struct module	*addmodule();
void		delmodule();
void		initmodules();
int		dup_module();
void		loadmodule();
void		listmodules();
void		servicemodule();
struct module	*findmodule();
void		killmodule();
struct pstring  *addpstring();
void		delpstring();
void		create_pstring();
void		listpstrings();
char		*expand_args();
void		send_pstr();
int		check_mod_command();
void		execute_event();


extern struct module    *modhead, *modtail;
extern struct pstring   *pstrhead, *pstrtail;
extern int		nextmodid;
