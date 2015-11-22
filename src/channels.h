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

#define C_MODULE	0	/* Channel that can't be locked. */
#define C_USER		1	/* Channel that can be locked.   */
#define C_UNLOCKED	0	/* Is the channel unlocked?	 */
#define C_LOCKED	1	/* Is the channel locked?	 */	
#define CHANNELMAX	12	/* Maximum channel name length   */


/* channels */
struct channel {
        char           name[CHANNELMAX+1];     /* Channel name */
        int            count,                  /* Number of users on channel */
		       lock,		       /* Is the channel locked?     */
                       type;                   /* Public or private          */
        struct user    *owner;                 /* User who created channel   */
	struct module  *module;		       /* This channel's module	     */

        struct channel *next;                  /* Next channel on the list   */
};


void    		change_channel();
struct channel 		*find_channel();
void    		add_channel();
void			initmain();
void    		delete_channel();
void			lock_channel();
void			channel_kick();
void    		channel_list();


extern struct channel   *channelhead, *channeltail;
