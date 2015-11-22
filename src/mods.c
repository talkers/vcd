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
 * This is mod.c of vcd. It holds the functions responsible for modules,
 * parse strings, and events.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <varargs.h>
#include "vcd.h"

/* static buffer for writing stuff */
static char buffer[1024];

struct module 	*modhead  = (struct module *)0,
	      	*modtail  = (struct module *)0;
struct pstring	*pstrhead = (struct pstring *)0,
		*pstrtail = (struct pstring *)0;	


int nextmodid = 1;


/* === Modules === */

struct module *
addmodule()
{
        struct module    *modptr;

        if (modhead == (struct module *) 0) {
                modhead = (struct module *) malloc(sizeof(struct module));
                if (modhead == (struct module *) 0) {
                        logerr("malloc failed", SYSLOG);
                        return (struct module *) 0;
                }
                modhead->next = (struct module *) 0;
                modtail = modhead;
        } else {
                modptr = (struct module *) malloc(sizeof(struct module));
                if (modptr == (struct module *) 0) {
                        logerr("malloc failed", SYSLOG);
                        return (struct module *) 0;
                }
                modptr->next = (struct module *) 0;
                modtail->next = modptr;
                modtail = modptr;
        }

        return modtail;
}


void
delmodule(m)
        struct module    *m;
{
        struct module    *mptr;
	struct pstring	 *p;

        if (m == modhead) {
            modhead = modhead->next;
            if (modtail == m)
                modtail = modhead;
        } else {
            for (mptr = modhead; mptr && mptr->next != m; mptr = mptr->next);
            mptr->next = m->next;
            if (m == modtail)
                modtail = mptr;
        }  

        fd_master_clear(m->fd);
	close(m->fd);
	kill(m->pid, 9);

	/* remove this module's pstrings */
	for (p = pstrhead; p; p = p->next)
	     if (p->mod == m)
		 delpstring(p);

	/* take care of module channel stuff if needed */
	if (m->channel) {
	    m->channel->type = C_USER;
	    if ((m->channel->module == m) && (m->channel->count == 0))
	        delete_channel(m->channel);
	}

	free(m->name);
        free(m);
}


void
initmodules()
{
        FILE    *fptr;
        char    name[80];

        fptr = fopen(MODULES, "r");
        if (!fptr) {
            logerr("error opening module file", SYSLOG);
            return;
        }

        fgets(name, 80, fptr);
        while(!feof(fptr)) {
                stripcr(name);
                loadmodule(name, "the haven");
                fgets(name, 80, fptr);
        }
        fclose(fptr);
}


int
dup_module(curr_fd)
	int curr_fd;
{
	int fd;

	/*
	 * Find the highest free fd. Remember fds start at 0. So
         * if MAXLOGINS is 256, our available fds are 0..255.
	 * We test for a full haven if a user is logging on to 
	 * the LAST fd (255 here) so start dup()'ing at 254.
	 */
	for (fd = MAXLOGINS-2; fd > 0; fd--)
		if (!fd_master_isset(fd))
			break;
	
	/* If there is an error, we'll have to live with it */
	if (dup2(curr_fd, fd) == -1) {
	    logerr("error with dup2()", MODLOG);
	    return curr_fd;
	}
	close(curr_fd);
	return fd;
}


void
loadmodule(modname, name)
	char		*modname, *name;
{
	struct module 	*mod;
	int	pid, fd[2], n;
	char	path[PATHMAX];

	sprintf(buffer, "-=> [Module '%s' loaded by %s]\r\n", modname, name);
	writelevel(SYSOP, buffer); 

	sprintf(path, "%s/%s", MODPATH, modname);	
	socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
	pid = fork();
	if (!pid) {
	    /* child */

	    dup2(fd[1], 0);
            dup2(fd[1], 1);

            /*
             * Since the parent and child share open fd's after the fork
             * we must close all open fds inside the child, or when a user
             * quits, their process will hang.
	     *
	     * Don't close stdin or stdout.
	     */
            
	    for (n=sysconf(_SC_OPEN_MAX)-1; n >= 2; n--)
		   close(n);

	    execl(path, modname, "(vcd module)", NULL);
	    
	    logerr("child dying", MODLOG);
	    exit(9);
	}

	/* parent */
	close(fd[1]);

	mod = addmodule();
	if (!mod) {
	    logerr("malloc failed", SYSLOG);
	    return;
	}

	mod->fd = dup_module(fd[0]);
	mod->id = nextmodid++;
	mod->pid = pid;
	FD_ZERO(&mod->events);
	mod->name = (char *)strdup(modname);
	mod->channel = (struct channel *)0;

	fd_master_set(mod->fd);
}


void
servicemodule(m)
        struct module   *m;
{
  int	n, size, done;
  char  *string, buf[1024], *fp, field[20];
  char	*next = buf;
  struct user *u;

  size = read(m->fd, buf, 1024);
  if (size <= 0) {

      if (size < 0) {
          sprintf(errbuff, "module [%s] error: %s", m->name,
		  (size == 0) ? "read error" : "disconnected");
          logerr(errbuff, MODLOG);
      }

      if (!strcmp(m->name, RESOLVER)) 
          clr_system(ASYNCHRODNS);

      sprintf(buffer, "-=> [Module '%s' just died.]\r\n", m->name);
      writelevel(SYSOP, buffer);

      delmodule(m);

   } else {
      buf[size] = '\0';

      next = buf;
      for (string = buf; *string && *string != '\n'; string++);
      if (*string == '\n') {
          *string = '\0';
          next = string+1;
      } else {
          sprintf(errbuff, "[%s]: Original input string not \\n terminated", m->name);
	  log(errbuff, MODLOG);
          return;
      }

      string = buf;
      done = 0;
      while (!done) {
	
         /* EVENT -> let a module claim an event */
	if (!strncmp("event ", string, 6)) {
		n = atoi(string+6);
		FD_SET(n, &m->events);

                /* If mod wants event E_RESOLVE turn on nameservice */
                if (n == E_RESOLVE)
                    set_system(ASYNCHRODNS);

	/* CHANNEL -> Put a mod in a channel */
	} else if (!strncmp("channel ", string, 8)) {
		struct channel *c;

		string = string + 8;
		c = find_channel(string);
		if (c) {
		    m->channel = c;
		    m->channel->module = m;
		    /* The channel already exists, but it must get mod status */
		    m->channel->type = C_MODULE;
		} else {
		    c = (struct channel *) malloc(sizeof(struct channel));
		    if (c == (struct channel *)0) {
			logerr("malloc failed", SYSLOG);
			return;
		    }
		    add_channel(c, string, C_MODULE);
		    m->channel = c;
		    m->channel->module = m;
		}
	
	/* FLAG -> set/clear a user's flag */
	} else if (!strncmp("flag ", string, 5)) {
                string = string + 5;
                n = atoi(string);
                string = stripdigit(string);
                string++;

                u = findline(n);
                if (u) {

                    fp = field;
                    while (*string && *string != ' ')
                           *fp++ = *string++;

                    *fp = '\0';
                    string++;

		    if (!strcmp("clear", field))
			clearflag(u, atoi(string));
		    else if (!strcmp("set", field))
			setflag(u, atoi(string));
		    else if (!strcmp("toggle", field))
			toggleflag(u, atoi(string));
		    else {
			sprintf(errbuff, "[%s:flag] unknown operation: [%s]", 
				m->name, field);
			log(errbuff, MODLOG);
		    }
		} else {
		    sprintf(errbuff, "[%s:flag] No such user: %d", m->name, n); 		
		    log(errbuff, MODLOG);
		}

	 /* SET -> set a user's field */
	 } else if (!strncmp("set ", string, 4)) {
                string = string + 4;
                n = atoi(string);
                string = stripdigit(string);
                string++;

                u = findline(n);
                if (u) {

		    fp = field;
		    while (*string && *string != ' ')
			   *fp++ = *string++;

		    *fp = '\0';
		    string++;

		    if (!strcmp("channel", field)) {
			/* nothing yet */

		    } else if (!strcmp("email", field)) {
			strncpy(u->email, string, EMAILMAX);
			u->email[EMAILMAX] = '\0';

                    } else if (!strcmp("host", field)) {
			int len;

			if ((len = strlen(string)) > HOSTMAX)
			    string = string + (len - HOSTMAX);
			
                        strncpy(u->host, string, HOSTMAX);
			u->host[HOSTMAX] = '\0';

		    } else if (!strcmp("flags", field)) {
			u->flags = (unsigned)atol(string);

		    } else if (!strcmp("level", field)) {
			u->level = atoi(string);

		    } else if (!strcmp("name", field)) {
			strncpy(u->name, string, NAMEMAX);
			u->name[NAMEMAX] = '\0';

		    /* make sure string is encrypted*/
		    } else if (!strcmp("password", field)) {
			strncpy(u->pword, string, PASSMAX);
			u->pword[PASSMAX] = '\0';

                    } else if (!strcmp("resolved", field)) {
                        strncpy(u->host, string, HOSTMAX);
                        u->host[HOSTMAX] = '\0';

                        /* check to see if their site is banned */
                        if (checkban(string)) {
                            writefile(n, BANMESG, ">> Your site is BANNED!" \
                                                  "   Go Away!\r\n");
                            disconnect(u, QUIT_NORMAL);
                            return;
                        }

                        fd_master_set(n);
                        writefile(u->line, INTRO, ">> No intro screen.\r\n");
                        greeting(u);
		  
		    } else if (!strcmp("url", field)) {
			strncpy(u->url, string, URLMAX);
			u->url[URLMAX] = '\0';

                    } else {
			sprintf(errbuff, "[%s:set] bad field: %s", m->name, field);
			log(errbuff, MODLOG);
		    }

                } else {
                    sprintf(errbuff, "[%s:set] No such user %d", m->name, n);
		    log(errbuff, MODLOG);
		}

         /* PSTRING -> set a module's parse string(s) */
         } else if (!strncmp("pstring ", string, 8)) {
                create_pstring(m, string+8);

         /* WRITE -> write some text to a user */
         } else if (!strncmp("write ", string, 6)) {
                string = string + 6;
                n = atoi(string);
                u = findline(n);

		if (u) {
                    string = stripdigit(string);
                    string++;

                    writelist(u, string, "\r\n", NULL);
		} else {
		    sprintf(errbuff, "[%s:write] No such user %d", m->name, n);
		    log(errbuff, MODLOG);
		}

	 /* WRITECHAN -> Have the module write to all user's in its channel */
	 } else if (!strncmp("writechan ", string, 10)) {
		struct user	*temp;

		string = string + 10;
		for (temp=userhead; temp; temp=temp->next)
		     if (m->channel == temp->channel)
			 writelist(temp, string, "\r\n", NULL);


	/* SYSMSG -> Have the module write a global message */
	} else if (!strncmp("sysmsg ", string, 7)) {
		struct user	*them;

		string = string + 7;
		sprintf(buffer, ">> System message from [%s]: %s\r\n", 
			m->name, string);
		for (them = userhead; them; them = them->next)
			writetext(them, buffer);
	

         /* NOCOMM-> just continue for now */
         } else {
               sprintf(errbuff, "No such command [%s]", string);
	       log(errbuff, MODLOG);
         }

	string = next;
	if (!*string)
	    done = 1;
	else {
	    for (; *next && *next != '\n'; next++);
	    if (*next == '\n') {
	        *next = '\0';
	        next++;
	    } else {
	        sprintf(errbuff, "[%s]: Packet input string not \\n terminated",
			m->name);
		log(errbuff, MODLOG);
	        return;
	    }

	}
     }	        /* end while */
   }		/* end else  */ 
}


struct module *
findmodule(id)
	int	id;
{
	struct module *m;

	for (m = modhead; m; m = m->next) {
               if (m->id == id)
                       return m;
	}
	return (struct module *) 0;
}


void
killmodule(me)
	struct user *me;
{
	char *ptr = stripwhite(me->input+3);	    /* .#k */
	struct module *m;
	int	id;

	id = atoi(ptr);
	m = findmodule(id);
	if (m == (struct module *)0) {
	    writetext(me, ">> No such module to kill.\r\n");
	    return;
	}
	
	if (!strcmp(m->name, RESOLVER))
	    clr_system(ASYNCHRODNS);

	sprintf(buffer, "-=> [Module '%s' killed by %s]\r\n", m->name, me->name);
	writelevel(SYSOP, buffer);

	delmodule(m);
}

	

/* === Parse Strings === */

struct pstring *
addpstring()
{
        struct pstring    *p;

        if (pstrhead == (struct pstring *) 0) {
                pstrhead = (struct pstring *) malloc(sizeof(struct pstring));
                if (pstrhead == (struct pstring *) 0) {
                        logerr("malloc failed", SYSLOG);
                        return (struct pstring *) 0;
                }
                pstrhead->next = (struct pstring *) 0;
                pstrtail = pstrhead;
        } else {
                p = (struct pstring *) malloc(sizeof(struct pstring));
                if (p == (struct pstring *) 0) {
                        logerr("malloc failed", SYSLOG);
                        return (struct pstring *) 0;
                }
                p->next = (struct pstring *) 0;
                pstrtail->next = p;
                pstrtail = p;
        }
	
        return pstrtail;
}


void
delpstring(p)
        struct pstring    *p;
{
        struct pstring    *ptmp;

        if (p == pstrhead) {
            pstrhead = pstrhead->next;
            if (pstrtail == p)
                pstrtail = pstrhead;
        } else {
            for (ptmp = pstrhead; ptmp && ptmp->next != p; ptmp = ptmp->next);
            ptmp->next = p->next;
            if (p == pstrtail)
                pstrtail = ptmp;
        }  

	FREE(p->fmt);
	FREE(p->str);
        FREE(p);
}


void
create_pstring(m, s)
	struct module *m;
	char	*s;
{
	struct pstring 	*p;
	char  *str, *fmt;

	p = addpstring();
	if (p == (struct pstring *)0) {
		logerr("malloc failed", SYSLOG);
		return;
	}

	str = s;
	fmt = index(str, ' ');
	
	if (fmt) {
	    *fmt = '\0';
	    fmt++;
	    p->fmt = (char *) strdup(fmt);
	} else {
	    p->fmt = (char *)0;
	}

	p->str = (char *) strdup(str);
	p->mod = m;
}


void
listpstrings(me)
	struct user *me;
{
	char	*ptr = stripwhite(me->input + 5);   /* .mods */
	struct pstring	*p;
	struct module	*m;
	char		buffer[80];	
	

	if (!strncmp("-a", ptr, 2)) {
            writetext(me, ">> id  fd   pid   Mod name    Channel\r\n");
            for (m = modhead; m; m = m->next) {
                 sprintf(buffer, ">> %-2d  %-3d  %-5d %-10s  %s\r\n",
                         m->id, m->fd, m->pid, m->name,
                         (m->channel) ? m->channel->name : "");
                 writetext(me, buffer);
            }
	} else {
	    writetext(me, ">> Mod name   Command\r\n");
	    for (p = pstrhead; p; p = p->next) {
	         sprintf(buffer, ">> %-10s %s\r\n", p->mod->name, p->str);
	         writetext(me, buffer);
	    }
	}			  
}


char *
expand_args(u, args)
	struct user	*u;
	char		*args;
{
	char    *bp, buffer[1024];  /* should be a safe size */

	buffer[0] = '\0';
	bp = buffer;

	while (*args) {
		if (*args == ' ' || *args == '\n')
		    args++;
		if (*args == '$') {
		    args++;
		    switch (*args) {
		     case 'c':
			     strcat(buffer, "$c");
			     strcat(buffer, u->channel->name);
			     strcat(buffer, "\t");
			     break;
		     case 'e':
			     strcat(buffer, "$e");
			     strcat(buffer, u->email);
			     strcat(buffer, "\t");
			     break;
		     case 'f':
			     strcat(buffer, "$f");
			     strcat(buffer, itoa(u->flags));
			     strcat(buffer, "\t");
			     break;
		     case 'g':
			     strcat(buffer, "$g");
			     strcat(buffer, (char *)&u->gags);
			     strcat(buffer, "\t");
			     break;
		     case 'h':
			     strcat(buffer, "$h");
			     strcat(buffer, u->host);
			     strcat(buffer, "\t");
			     break;
		     case 'i':
			     strcat(buffer, "$i");
			     strcat(buffer, itoa(u->idle));
			     strcat(buffer, "\t");
			     break;
		     case 'l':
			     strcat(buffer, "$l");
			     strcat(buffer, itoa(u->level));
			     strcat(buffer, "\t");
			     break;
		     case 'n':
			     strcat(buffer, "$n");
			     strcat(buffer, u->name);
			     strcat(buffer, "\t");
			     break;
		     case 'o':
			     strcat(buffer, "$o");
			     strcat(buffer, itoa(u->logintime));
			     strcat(buffer, "\t");
			     break;
		     case 'p':
			     strcat(buffer, "$p");
			     strcat(buffer, u->pword);
			     strcat(buffer, "\t");
			     break;
		     case 'u':
			     strcat(buffer, "$u");
			     strcat(buffer, u->url);
			     strcat(buffer, "\t");
			     break;
		     case 'w':
			     strcat(buffer, "$w");
			     strcat(buffer, itoa(u->wrap));
			     strcat(buffer, "\t");
			     break;
		     case 'H':
			     strcat(buffer, "$H");
			     strcat(buffer, itoa(u->hicolor));
			     strcat(buffer, "\t");
			     break;
		     case 'L':
			     strcat(buffer, "$L");
			     strcat(buffer, itoa(u->lastp));
			     strcat(buffer, "\t");
			     break;
                     case 'P':
                             strcat(buffer, "$P");
                             strcat(buffer, itoa(u->port));
                             strcat(buffer, "\t");
                             break;
                     case 'T':
                             strcat(buffer, "$T");
                             strcat(buffer, itoa(u->talkcolor));
                             strcat(buffer, "\t");
                             break;
		     case 'Y':
			     strcat(buffer, "$Y");
			     strcat(buffer, itoa(u->yellcolor));
			     strcat(buffer, "\t");
			     break;
	             default:
			     sprintf(errbuff, "expand_args(): [%c]", *args);
			     log(errbuff, MODLOG);
		  }
	    }
		  args++;	/* remove the arg */   
	}

	return bp;
}



void
send_pstr(me, p)
	struct user 	*me;
	struct pstring 	*p;
{
	char buffer[2*INPUTMAX];

	buffer[0] = '\0';

	/*
	 * The leading c signals that this packet is a command.
	 */
	if (p->fmt)
	    sprintf(buffer, "c%d\t%s$I%s\t\n", me->line, 
			expand_args(me, p->fmt), me->input);
	else
	    sprintf(buffer, "c%d\t$I%s\t\n", me->line, me->input);

	writedesc(p->mod->fd, buffer);
}


int
check_mod_command(me)
	struct user *me;
{
	struct pstring	*p;

	for (p = pstrhead; p; p = p->next) {
	     if (!strncmp(p->str, me->input+1, strlen(p->str))) {
		 send_pstr(me, p);
		 return 1;
	     }
	}

	return 0;
}


/* === Events === */
/* The argument list MUST be NULL terminated!!!!! */
void
execute_event(id, va_alist)
        int     id;
        va_dcl
{
        va_list         ap;
        char            *bptr, buffer[INPUTMAX*2];
	struct module	*m;

        buffer[0] = '\0';

	/*
	 * Signal that this packet is an event and send the 
	 * event number with it to the module.
	 */ 
	sprintf(buffer, "e%d ", id);

        va_start(ap);

        bptr = va_arg(ap, char *);
        while (bptr != (char *) 0) {
                strcat(buffer, bptr);
                bptr = va_arg(ap, char *);
        }
        va_end(ap);

	/* write to ALL modules that want this event */
	for (m = modhead; m; m = m->next)
	     if (FD_ISSET(id, &m->events))	
         	 writedesc(m->fd, buffer);
}
