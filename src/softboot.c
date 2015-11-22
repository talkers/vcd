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
 * This is softboot.c of vcd. It holds the functions responsible for softboots.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory.h>
#include "vcd.h"


void
loadchannels()
{
	FILE	*fptr;
	struct channel *c = (struct channel *)0;
	char str[80];
	int  type;

	fptr = fopen(SBCHAN, "r");

	initmain(DEFAULTMAIN);
	fgets(str, 80, fptr);
	while (!feof(fptr)) {
		stripcr(str);
		c = (struct channel *) malloc(sizeof(struct channel));
		if (c == (struct channel *)0) {
		    logerr("malloc failed", SYSLOG);
		    return;
		}

		if (!strncmp("type ", str, 5)) {
			type = atoi(str+5);
		} else if (!strncmp("name ", str, 5)) {
			add_channel(c, str+5, type);
		} else if (!strncmp("owner ", str, 6)) {
			c->owner = findline(atoi(str+6));
		}
		fgets(str, 80, fptr);
	}
	fclose(fptr);
}



void
loadusers()
{
	FILE	*fptr;
	struct user	*u = (struct user *)0;
	char	str[80];
	int	desc;

	fptr = fopen(SBUSER, "r");

	fgets(str, 80, fptr);
	while (!feof(fptr)) {
		stripcr(str);
		if (!strncmp("line ", str, 5)) {
		    desc = atoi(str+5);
		    u = adduser();
		    if (u != NULL) {
		        fd_master_set(desc);	
		        FD_ZERO(&u->gags);	
		        u->line = desc;
		        numlogins++;		
		    } else {
			logerr("Malloc failed during softboot. User couldn't be allocated", SYSLOG);
			writedesc(desc, ">> Allocation error during softboot. Sorry, closing connection.\r\n");
			close(desc);
			return;
		    }
		} else if (!strncmp("name ", str, 5)) {
			   strncpy(u->name, str+5, NAMEMAX);
		} else if (!strncmp("chan ", str, 5)) {
			   u->channel = find_channel(str+5);
			   u->channel->count++;
		} else if (!strncmp("lock", str, 4)) {
			   u->channel->lock = C_LOCKED;
		} else if (!strncmp("owner", str, 5)) {
			   u->channel->owner = u;
		} else if (!strncmp("pass ", str, 5)) {
			   strncpy(u->pword, str+5, PASSMAX);
		} else if (!strncmp("emai ", str, 5)) {
			   strncpy(u->email, str+5, EMAILMAX);
		} else if (!strncmp("urlx ", str, 5)) {
			   strncpy(u->url, str+5, URLMAX);
		} else if (!strncmp("flag ", str, 5)) {
			   u->flags = atoi(str+5);
		} else if (!strncmp("gags ", str, 5)) {
			   memcpy((char *)&u->gags, (char *)(str+5), sizeof(fd_set *));
		} else if (!strncmp("plev ", str, 5)) {
			   u->level = atoi(str+5);
		} else if (!strncmp("host ", str, 5)) {
			   strncpy(u->host, str+5, HOSTMAX);
		} else if (!strncmp("idle ", str, 5)) {
			   u->idle = (time_t)atol(str+5);
		} else if (!strncmp("logi ", str, 5)) {
			   u->logintime = (time_t)atol(str+5);
		} else if (!strncmp("wrap ", str, 5)) {
			   u->wrap = atoi(str+5);
		} else if (!strncmp("hcol ", str, 5)) {
			   u->hicolor = atoi(str+5);
		} else if (!strncmp("tcol ", str, 5)) {
                           u->talkcolor = atoi(str+5);
                } else if (!strncmp("ycol ", str, 5)) {
                           u->yellcolor = atoi(str+5);
                } else if (!strncmp("port ", str, 5)) {
			   u->port = atol(str+5);
		}

		fgets(str, 80, fptr);
	}
	fclose(fptr);
}

	
void
loadmain()
{
        FILE    *fptr;
        char    str[80];

        fptr = fopen(SBMAIN, "r");

        fgets(str, 80, fptr);
        while (!feof(fptr)) {
                stripcr(str);
                if (!strncmp("idle ", str, 5)) {
                        idletimeout = atol(str+5);
                } else if (!strncmp("boot ", str, 5)) {
                        boottime = (time_t)atol(str+5);
		} else if (!strncmp("next ", str, 5)) {
			nextcheck = (time_t)atol(str+5);
                } else if (!strncmp("port ", str, 5)) {
                        port = atoi(str+5);
                } else if (!strncmp("serv ", str, 5)) {
                        server = atoi(str+5);
                } else if (!strncmp("flag ", str, 5)) {
			system_flags = atoi(str+5);
		}
                fgets(str, 80, fptr);
        }
        fclose(fptr);
}


void
loadmods()
{
	FILE		*fptr;
	char		str[80];
	int		desc;
	struct module   *m = (struct module *)0;

	fptr = fopen(SBMODS, "r");

	fgets(str, 80, fptr);
	while (!feof(fptr)) {
		stripcr(str);
		if (!strncmp("desc ", str, 5)) {
		    desc = atoi(str+5);
		    m = addmodule();
		    if (m) {
			m->fd = desc;
			fd_master_set(desc);
		    } else {
			close(desc);
		    }
		} else if (!strncmp("m_id ", str, 5)) {
			   m->id = atoi(str+5);
		} else if (!strncmp("mpid ", str, 5)) {
			   m->pid = atoi(str+5);
		} else if (!strncmp("name ", str, 5)) {
			   m->name = strdup(str+5); 
		} else if (!strncmp("chan ", str, 5)) {
			   m->channel = find_channel(str+5);
/****
			   m->channel->count++;
****/
			   m->channel->module = m;
                } else if (!strncmp("m_ev ", str, 5)) {
                           memcpy((char *)&m->events, (char *)(str+5), sizeof(fd_set *)); 
		} else if (!strncmp("next ", str, 5)) {
			   nextmodid = atoi(str+5);
		}
		fgets(str, 80, fptr);
	}
	fclose(fptr);
}


void
loadpstr()
{
	FILE	*fptr;
	char	str[80];
	int	n;
	struct pstring	*p = (struct pstring *)0;
	struct module	*m = (struct module *)0;

	fptr = fopen(SBPSTR, "r");

	fgets(str, 80, fptr);
	while (!feof(fptr)) {
		stripcr(str);
		if (!strncmp("pstr ", str, 5)) {
		    p = addpstring();
		    if (p) {
			p->str = strdup(str+5);
		    } else {
			logerr("Malloc failed during softboot. Pstring not allocated", SYSLOG);
		    }
		} else if (!strncmp("pfmt ", str, 5)) {
			p->fmt = strdup(str+5);
		} else if (!strncmp("pmod ", str, 5)) {
			n = atoi(str+5);
			m = findmodule(n);
			if (m) {
			    p->mod = m;
		        }
		}
		fgets(str, 80, fptr);
	}
	fclose(fptr);
}



void
savechannels()
{
	FILE *fptr;
	struct channel *c;
		

	fptr = fopen(SBCHAN, "w");

	for (c = channelhead->next; c; c = c->next) {
	     fprintf(fptr, "type %d\n", c->type);
	     fprintf(fptr, "name %s\n", c->name);
	     if (c->owner)
		 fprintf(fptr, "owner %d\n", c->owner->line);	
	}

	fclose(fptr);
}


void
saveusers()
{
	FILE *fptr;
	struct user *u;
	fd_set	dummy;

	FD_ZERO(&dummy);

	fptr = fopen(SBUSER, "w");

	for (u=userhead; u; u=u->next) {
		fprintf(fptr, "----------------\n");
		fprintf(fptr, "line %d\n", u->line);
		fprintf(fptr, "name %s\n", u->name);
		fprintf(fptr, "chan %s\n", u->channel->name);
		if (u->channel->owner == u)
		    fprintf(fptr, "owner\n");
		if (u->channel->lock == C_LOCKED)
		    fprintf(fptr, "lock\n");
		if (user_exists(u->name)) {
		    save_user(u, S_NOUPDATE, "");
		    fprintf(fptr, "pass %s\n", u->pword);
		}
		if (*u->email)
		    fprintf(fptr, "emai %s\n", u->email);
		if (*u->url)
		    fprintf(fptr, "urlx %s\n", u->url);
		fprintf(fptr, "host %s\n", u->host); 	
		fprintf(fptr, "flag %lu\n", u->flags);
		if (memcmp((char *)&dummy, (char *)&u->gags, sizeof(fd_set *)))
		    fprintf(fptr, "gags %s\n", (char *)&u->gags);
		fprintf(fptr, "plev %d\n", u->level);
		fprintf(fptr, "idle %ld\n", u->idle);
		fprintf(fptr, "logi %ld\n", u->logintime);
		fprintf(fptr, "wrap %d\n", u->wrap);
		fprintf(fptr, "hcol %d\n", u->hicolor);
		fprintf(fptr, "tcol %d\n", u->talkcolor);
		fprintf(fptr, "ycol %d\n", u->yellcolor);
		fprintf(fptr, "port %ld\n", u->port);
	}
	fclose(fptr);
}


void
savemain()
{
	FILE	*fptr;

	fptr = fopen(SBMAIN, "w");

	fprintf(fptr, "serv %d\n",  server);
        fprintf(fptr, "port %d\n",  port);
        fprintf(fptr, "boot %lu\n", boottime);
        fprintf(fptr, "idle %lu\n", idletimeout);
	fprintf(fptr, "next %lu\n", nextcheck);
	fprintf(fptr, "flag %d\n",  system_flags);

	fclose(fptr);
}


void
savemods()
{
	FILE		*fptr;
	struct module 	*m;
	fd_set		dummy;

	FD_ZERO(&dummy);
	fptr= fopen(SBMODS, "w");

	fprintf(fptr, "next %d\n", nextmodid);
	for (m = modhead; m; m = m->next) {
	     fprintf(fptr, "----------------\n");
	     fprintf(fptr, "desc %d\n", m->fd);
	     fprintf(fptr, "m_id %d\n", m->id);
	     fprintf(fptr, "mpid %d\n", m->pid);
	     fprintf(fptr, "name %s\n", m->name);
	     if (m->channel)
		 fprintf(fptr, "chan %s\n", m->channel->name);
	     if (memcmp((char *)&dummy, (char *)&m->events, sizeof(fd_set *)))
                 fprintf(fptr, "m_ev %s\n", (char *)&m->events);
	}
	fclose(fptr);
}


void
savepstr()
{
	FILE	*fptr;
	struct pstring *p;

	fptr = fopen(SBPSTR, "w");

	for (p = pstrhead; p; p = p->next) {
                fprintf(fptr, "----------------\n");
                fprintf(fptr, "pstr %s\n", p->str);
                fprintf(fptr, "pfmt %s\n", p->fmt);
                fprintf(fptr, "pmod %d\n", p->mod->id);
        }
        fclose(fptr);
}


void
softboot()
{	
	savemain();
	savechannels();
	saveusers();
	savemods();
	savepstr();
	execl("vcd", "vcd", "softboot", itoa(port), 0);

	return;
}
