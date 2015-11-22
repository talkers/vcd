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
 * This is macros.h of vcd.  This contains various useful macros and
 * declarations used for error handling and logging.
 */

#include <errno.h>

extern void logmess();
extern void logerr();
extern char errbuff[];

#define log(x, y)    logmess(x, y, __FILE__, __LINE__);
#define logerr(x, y) logerror(x, y, __FILE__, __LINE__, errno);

#define toggleflag(x, y)     x->flags ^= y
#define istoggled(x, y)      x->flags & y
#define nottoggled(x, y)     !(istoggled(x,y))
#define clearflag(x, y)	    x->flags &= ~y
#define setflag(x, y)	    x->flags |= y

#define set_system(x)   (system_flags |=  x)
#define clr_system(x)   (system_flags &= ~x)
#define  is_system(x)   (system_flags & x)
#define not_system(x)   !is_system(x)

#define checkpower(x, y)    (x->level >= y)

#define FREE(x)		free(x)
#define MALLOC(x)	malloc(x)
