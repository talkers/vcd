/*
        VCD Haven Code Version 4.0.0
        Copyright (C) 1995 Angelo Brigante Jr. & Gordon Chan
        Copyright (C) 1996, 1997, 1998 Angelo Brigante Jr.

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

        Send comments and bug reports to:  ang@hemi.com
*/


/*
 * MHP - Mod-Haven Protocol.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "mhp.h"
#include "vcd.h"

#define PACKETMAX	INPUTMAX*2


/* Send the event number to the haven */
void
mhp_event(id)
	int	id;
{
	printf("event %d\n", id);
	fflush(stdout);
}

/* Write 'text' to a user on the haven. */
void
mhp_write(line, text)
	int	line;
	char	*text;
{
	printf("write %d %s\n", line, text);
	fflush(stdout);
}


/* Let the module claim command 's' */
void
mhp_command(s)
	char	*s;
{
	printf("pstring %s\n", s);
	fflush(stdout);
}

void
mhp_mod_write_chan(text)
	char	*text;
{
	printf("writechan %s\n", text);
	fflush(stdout);
}

/* Put a module into a channel */
void
mhp_channel(c)
	char	*c;
{
	printf("channel %s\n", c);
	fflush(stdout);
}

/* Clear a user's flag */
void
mhp_clrflag(line, flag)
	int		line;
	unsigned long	flag;
{
	printf("flag %d clear %lu\n", line, flag);
	fflush(stdout);
}

void
mhp_setflag(line, flag)
	int		line;
	unsigned long	flag;
{
	printf("flag %d set %lu\n", line, flag);
	fflush(stdout);
}

void
mhp_togflag(line, flag)
	int		line;
	unsigned long	flag;
{
	printf("flag %d toggle %lu\n", line, flag);
	fflush(stdout);
}

void
mhp_setchannel(line, channel)
	int	line;
	char	*channel;
{
	printf("set %d channel %s\n", line, channel);
	fflush(stdout);
}

void
mhp_setemail(line, email)
	int	line;
	char	*email;	
{
	printf("set %d email %s\n", line, email);
	fflush(stdout);
}

void
mhp_sethost(line, host)
	int	line;
	char	*host;
{
	printf("set %d host %s\n", line, host);
	fflush(stdout);
}

void
mhp_setflags(line, flags)
	int		line;
	unsigned long	flags;
{
	printf("set %d flags %lu\n", line, flags);
	fflush(stdout);
}

void
mhp_setlevel(line, level)
	int	line, level;
{
	printf("set %d level %d\n", line, level);
	fflush(stdout);
}

void
mhp_name(line, name)
	int	line;
	char	*name;
{
	printf("set %d name %s\n", line, name);
	fflush(stdout);
}

void
mhp_resolved(line, host)
	int	line;
	char	*host;
{
	printf("set %d resolved %s\n", line, host);
	fflush(stdout);
}

void
mhp_url(line, url)
	int	line;
	char	*url;
{
	printf("set %d url %s\n", line, url);
	fflush(stdout);
}


void
packet_parse(u, s)
	struct user 	*u;
	char		*s;
{
	int	j = 0;

	/* get packet type */
	if (*s == 'c') s++;

	/* Get the line number */
	u->line = atoi(s);

	/* Strip off the line number */
	while (isdigit(*s)) 
		s++;

	/* Strip the space */
	s++;
	while (*s != '\0') {
	       if (*s == '$') {
		    s++;
		    j = 0;	
		  switch(*s++) {
		  case 'c':
			while (*s != '\t') u->channel->name[j++] = *s++;
			u->channel->name[j] = '\0';
			break;
		  case 'e':
			while (*s != '\t') u->email[j++] = *s++;
			u->email[j] = '\0';
			break;
		  case 'f':
			u->flags = (unsigned long)atol(s);
			while (isdigit(*s)) s++; 
			break;
		  case 'h':
			while (*s != '\t') u->host[j++] = *s++;
			u->host[j] = '\0';
			break;
		  case 'i':
			u->idle = (time_t)atol(s);
			while (isdigit(*s)) s++;
			break;
		  case 'l':
			u->level = (int)atoi(s);
			while (isdigit(*s)) s++;
			break;
		  case 'n':
			while (*s != '\t') u->name[j++] = *s++;
			u->name[j] = '\0';
			break;
		  case 'o':
                        u->logintime = (time_t)atol(s);
                        while (isdigit(*s)) s++;
                        break;
		  case 'p':
			while (*s != '\t') u->pword[j++] = *s++;
			u->pword[j] = '\0';
			break;
		  case 'u':
			while (*s != '\t') u->url[j++] = *s++;
			u->url[j] = '\0';
			break;
		  case 'H':
			u->hicolor = (int)atoi(s);
			while (isdigit(*s)) s++;
                        break;
		  case 'L':
			u->lastp = (int)atoi(s);
                        while (isdigit(*s)) s++;
                        break;
		  case 'P':
			u->port = (unsigned long)atol(s);
		        while (isdigit(*s)) s++;
                        break;
		  case 'T':
			u->talkcolor = (int)atoi(s);
                        while (isdigit(*s)) s++;
                        break;
		  case 'Y':
			u->yellcolor = (int)atoi(s);
                        while (isdigit(*s)) s++;
                        break;
		  default:
			s++;
		  }
	       } else {
		  s++;
	       }
	}
}
