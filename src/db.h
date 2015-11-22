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

/* Options for load_user() */
#define L_LOAD  0
#define L_LOGIN 1

/* Options for save_user() */
#define S_NOUPDATE 0
#define S_UPDATE   1

char		get_dir();
char		*get_password();
char		*crypt_password();
int		user_exists();	
int		verify_user();
void		load_user();
void		save_user();
void		set_password();
void		register_user();
void		nuke_user();
void		finger_user_name();
void		create_root();
void		set_email();
void		set_url();
void		init_user();
void		purge_users();
