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

/* Maximum number of users. */
#define MAXLOGINS 256

/* Default port. */
#define DEFAULTPORT 9999	

/* Default idle timeout in seconds. */
#define DEFAULTIDLE 3600

/* Any more lines than this, they're kicked. */
#define SPAMMAX	7

/* Time interval (secs) between lines that would be constituted as spamming. */
#define SPAMINTERVAL 2

/* Default main channel name. */
#define DEFAULTMAIN     "0"

/* Default user name. */
#define DEFAULTNAME	"?"

/* Your haven's name. */
#define HAVENNAME       " Another Haven! "

/* Header line for certain functions -- Dont make it more than 78 characters. */
#define DASHLINE        "------------------------------------------------------------------------------\r\n"

/* Line not found message. */
#define BAD_LINE        ">> User line doesn't exist.\r\n" 

/* Exit message. */
#define EXITMSG         ">> And they all lived happily ever after...\r\n" 

/* Number of days a user may remain inactive before being deleted. */
#define PURGE_DAYS	(90 * 86400)

/*
 * Code version. If you modify this code, wherever you display the version
 * number, you MUST say that it is based on VCD of the version below.
 */
#define VERSION         "4.0.2"

/* Code Date */
#define VERDATE         "February 5th, 1999"

/* You really shouldn't change this! :) */
#define ROOTNAME	"root"
