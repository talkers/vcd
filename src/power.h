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
 * This is power.h of vcd.  These are the passwds to change your power 
 * level. 
 */

/* power levels */
#define		JOEUSER	0	/* John Q. Havener */
#define		SYSOP	1	/* We like these people a lot */
#define		ADMIN	2	/* Our good friends :) */
#define		ROOT	3	/* In GOD we trust */


#define P_OPCHANNEL	SYSOP	/* Use op channel */
#define P_AUTONOCAP	ADMIN	/* Toggle Auto no caps */
#define P_BANSITE	ADMIN	/* Ban a site */
#define P_ERASEPOST	SYSOP	/* Erase bulletin board post */
#define P_OPHUSH	SYSOP	/* Hush op channel */
#define P_INVISMESS	ADMIN	/* Send an invisible message */
#define P_KICK		SYSOP	/* Kick a user off the haven */
#define P_NUKE		ADMIN	/* Nuke a user */
#define P_FAKEHOST	SYSOP	/* Change our host name */
#define P_REGISTER	SYSOP	/* Register a user */
#define P_TIMEOUTS	ADMIN	/* Turn on idle timeouts */
#define P_TAKECAPS	SYSOP	/* De-cap a fuckwit */
#define P_SOFTBOOT	ADMIN	/* Softboot the haven */
#define P_GLOBALYELL	ADMIN	/* Send a system message */
#define P_PURGE		ROOT	/* Purge expired users */
#define P_SHUTDOWN	ROOT	/* Shutdown the haven */	
#define P_PROMOTE	ROOT	/* Raise user's power level */
#define P_DEMOTE	ROOT	/* Lower user's power level */	
#define P_LOADMOD	ADMIN	/* Load a module */
#define P_KILLMOD	ADMIN	/* Kill a module */


void	globalyell();
void	invismess();
void	insuf_power();
void	kick();
void	promote();
void	demote();
void	fakehost();
void	opchannel();
void	takecaps();
void	havenshutdown();
void	toggletimeouts();
void	auto_nocaps();
void	toggleauto_nocaps();
void	toggleophush();


extern char *power_names[]; 
