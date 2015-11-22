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
 * This is parse.c of vcd.  This contains the command parser.
 */

/* system headers */
#include <ctype.h>		/* For isdigit(). */
#include <stdio.h>		/* For sprintf(). */
#include <stdlib.h>		/* For atoi(). */
#include <string.h>		/* For strncpy(). */
#include "vcd.h"


/* Command parser.  If the user quits, return a 0.  Otherwise return a 1. */
int
getcommand(me)
	struct user    	*me;
{
	char           *ptr = me->input;

	ptr++;
	switch (*ptr) {

        case 'a':
                writefile(me->line, NEWS, ">> There are no ANNOUNCEMENTS.\r\n");
                break;
        case 'b':
                if (!strcmp(ptr, "ban"))
                        showban(me);
                else
                        togglebeeps(me);
                break;
        case 'c':
                ptr++;
                ptr = (char *)stripwhite(ptr);
                if (*ptr == '\0')
			channel_list(me);
                else {
			me->commands++;
			change_channel(me);
		}
                break;
        case 'e':
                togglepecho(me);
                break;
        case 'f':
                ptr++;
		ptr = (char *)stripwhite(ptr);
                if (*ptr == '\0')
			fingerpage(me, 0);
                else
			fingeruser(me);
                break;
	case 'g':
		togglegag(me);
		break;
        case 'h':
		switch (*(ptr + 1)) {
                case 'u':
                        togglehush(me);
                        break;
                case 'i':
                        togglehilite(me);
                        break;
                case 'p':
                        togglehipointer(me);
                        break;
                case '\0':
                        writetext(me, ">> Be more specific: .hu, .hi, or .hp.\r\n");
                        break;
                default:
                        goto clueless;
	       }
               break;
        case 'i':
                if (!strcmp(ptr, "intro"))
			writefile(me->line, INTRO, ">> There is no intro.\r\n");
                else
                        fingerpage(me, 1);
                break;
	case 'k':
		channel_kick(me);
		break;
        case 'l':
                lock_channel(me);
                break;
        case 'm':
		if (!strncmp(ptr, "mods", 4))
			listpstrings(me);
		else
                	togglearrivals(me);
                break;
        case 'n':
                changename(me);
                break;
        case 'p':
		me->commands++;
                private(me);
                break;
        case 'q':
                return quit(me);
	case 'r': 
		read_messages(me, ">> There are no posts on the board.\r\n");
                break;
	case 's':
               	togglepsuppress(me);
                break;
        case 't':
                gettime(me);
                break;
        case 'u':
                uptime(me);
                break;
        case 'v':
                showversion(me);
                break;
        case 'w':
                wholist(me, 1);
                break;
        case 'y':
		me->commands++;
                yells(me);
                break;
        case 'B':
                post_message(me);
                break;
	case 'C':
		set_colors(me);
		break;
	case 'D':
                if (user_exists(me->name))
                    deletemail(me);
                else {
                   writetext(me, ">> You are not registered so you have no mail to delete.\r\n");
		   writetext(me, ">> Type .?reg for help on registering.\r\n");
                }
                break;
        case 'E':
                set_email(me);
                break;
	case 'F':
		finger_user_name(me);
		break;
        case 'M':
                toggleallmsgs(me);
                break;
        case 'O':
                listops(me);
                break;
        case 'P':
		set_password(me);
                break;
	case 'R':
		if (user_exists(me->name))
		   readmail(me);
		else {
		   writetext(me, ">> You are not registered so you have no mail to read.\r\n");
		   writetext(me, ">> Type .?reg for help on registering.\r\n");
		}
		break;
	case 'S':
		if (user_exists(me->name))
		    sendmail(me);
		else {
		   writetext(me, ">> You must be registered to send mail.\r\n");
		   writetext(me, ">> Type .?reg for help on registering.\r\n");
		}
		break;
        case 'T':
                viewtoggles(me);
                break;
        case 'U':
                set_url(me);
                break;
	case 'W':
		togglewrap(me);
		break;
        case '~':
                pagedwholist(me);
                break;
	case '?':
                getuserhelp(me, ">> NO help for YOU!\r\n");
		break;
        case '*':
                wholist(me, 0);
                break;

	/* Power commands */
	case 'o': 
		if (checkpower(me, P_OPCHANNEL))
			opchannel(me);
		else
			goto clueless;
		break;

        case '@':
	switch (*(ptr + 1)) {

        case 'a':
                if (checkpower(me, P_AUTONOCAP))
                        toggleauto_nocaps(me);
                else
                        goto clueless;
                break;
        case 'b':
                if (checkpower(me, P_BANSITE))
                        banadd(me);
                else
                        goto clueless;
                break;
        case 'e':
                if (checkpower(me, P_ERASEPOST))
                        erase_message(me);
                else
                        goto clueless;
                break;
        case 'h':
                if (checkpower(me, P_OPHUSH))
                        toggleophush(me);
                else
                        goto clueless;
                break;
        case 'i':
                if (checkpower(me, P_INVISMESS))
                        invismess(me);
                else
                        goto clueless;
                break;
        case 'k':
                if (checkpower(me, P_KICK))
                        kick(me);
                else
                        goto clueless;
                break;
        case 'l':
                if (checkpower(me, P_TAKECAPS))
                        takecaps(me);
                else
                        goto clueless;
                break;
	case 'n':
		if (checkpower(me, P_NUKE) && !strncmp(ptr, "@nuke", 5))
			nuke_user(me);
		else
			goto clueless;
		break;
	case 'p':
		if (checkpower(me, ROOT) && !strncmp(ptr, "@purge", 6))
			purge_users(me);
		else
			goto clueless;
		break;
        case 'r':
		if (checkpower(me, P_REGISTER) && !strncmp(ptr, "@reg", 4))
                	register_user(me);
		else
			goto clueless;
                break;
        case 's':
                if (checkpower(me, P_SHUTDOWN) && !strncmp(ptr, "@shutdown", 9))
                        havenshutdown(me);
                else
                        goto clueless;
                break;
        case 't':
                if (checkpower(me, P_TIMEOUTS))
                        toggletimeouts(me);
                else
                        goto clueless;
                break;
	case 'u':
		if (checkpower(me, P_BANSITE))
			bandel(me, 0);
		else
			goto clueless;
		break;
        case 'H':
                if (checkpower(me, P_FAKEHOST))
                        fakehost(me);
                else
                        goto clueless;
                break;
        case 'K':
                if (checkpower(me, P_KILLMOD))
                        killmodule(me);
                else
                        goto clueless;
                break;
        case 'L':
                if (checkpower(me, P_LOADMOD)) {
			char	*modname;
			
			modname = (char *)stripwhite(me->input + 3);
                        loadmodule(modname, me->name);
                } else
                        goto clueless;
                break;
	case 'S':
		if (checkpower(me, P_SOFTBOOT)) {
			writetext(me, ">> Softbooting!\r\n");
			softboot();
		}
		else
			goto clueless;
		break;
        case 'Y':
                if (checkpower(me, P_GLOBALYELL))
                        globalyell(me);
                else
                        goto clueless;
                break;
	case '?':
		if (checkpower(me, SYSOP))
			getpowerhelp(me, ">> NO help for YOU!!!\r\n");
		else
			goto clueless;
		break;
        case '+':
                if (checkpower(me, P_PROMOTE))
			promote(me);
                else
			goto clueless;
                break;
        case '-':
                if (checkpower(me, P_DEMOTE))
			demote(me);
                else 
			goto clueless;
                break;
        default:
		goto clueless;
        }	/* end power switch() */
	break;


clueless:
	default:
		writetext(me, ">> Unrecognized command.  Type .? for help.\r\n");
	
	}       /* End switch() */

	return 1;
}
