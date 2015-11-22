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


void            greeting();
int             quit();
void		getout();
void		talk();
void		private();
void		yells();
void		togglepecho();
void		togglehilite();
void		togglebeeps();
void		togglearrivals();
void		toggleallmsgs();
void		togglehush();
void		togglegag();
void		showgags();
void		togglepsuppress();
void		togglewrap();
void		togglehipointer();
void            changename();
void            gettime();
void            uptime();
void            fingerpage();
void            fingeruser();
void		showversion();
char           *whostats();
void            wholist();
void            pagedwholist();
void            listops();
void		set_colors();
void            viewtoggles();

extern char	bra[], ket[];
