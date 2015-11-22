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
 * This is channels.c of vcd. It contains all the functions necessary for
 * channels.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vcd.h"

/* local buffer for writing stuff */
static char buffer[512];

struct channel	*channelhead = (struct channel *)0,
		*channeltail = (struct channel *)0;

/* Routines used for handling channels */
void 
change_channel(me)
	struct user *me;
{
	char *name;
	struct user *them;
	struct channel *new_channel, *old_channel;

	name = (char *) stripwhite(me->input+2);	/* .c */

	new_channel = (struct channel *) find_channel(name);
	old_channel = me->channel;

	/* If this channel doesn't exist, then create it */
	if (!new_channel) {
	    new_channel = (struct channel *) malloc(sizeof(struct channel));
	    if (new_channel == (struct channel *)0) {
	        writetext(me, ">> Channel could not be allocated."\
			      " Channel has not been changed.\r\n");
	        return;
	    } else {
	        add_channel(new_channel, name, C_USER);
		new_channel->owner = me;
	    }
	}

	if (new_channel == old_channel) {
	    writetext(me, ">> Same channel buddy.\r\n");
	    return;
	}

	if (new_channel->lock) {
	    sprintf(buffer, ">> Channel %s is locked.  Sorry.\r\n",
			new_channel->name);
	    writetext(me, buffer);
	    return;
	}

	me->channel = new_channel;
	new_channel->count++;

	if (--(old_channel->count)) {
	    sprintf(buffer, ">> %s on line %d has wandered off to %s.\r\n",
		    me->name, me->line, me->channel->name);

	    /* If I was the owner and changed channels, clear ownership */
	    if (old_channel->owner == me) {
		old_channel->owner = (struct user *) 0;
		old_channel->lock = C_UNLOCKED;
		strcat(buffer, ">> Channel has been unlocked.\r\n");
	    }

	    for (them = userhead; them; them = them->next)
	         if ((them->channel == old_channel) && nottoggled(them, CHANMSGS))
	             writetext(them, buffer);

	} else
	    delete_channel(old_channel);

	sprintf(buffer, ">> %s on line %d has joined.\r\n", me->name, me->line);
	for (them = userhead; them; them = them->next)
	     if ((them->channel == new_channel) && nottoggled(them, CHANMSGS))
		 writetext(them, buffer);
}


struct channel *
find_channel(channel)
	char *channel;
{
	struct channel *c;

	for (c = channelhead; c && strncmp(channel, c->name, CHANNELMAX); c = c->next);
	return c;
}


void 
add_channel(chan, name, type)
	struct channel *chan;
	char *name;
	int  type;
{
	strncpy(chan->name, name, CHANNELMAX);
        chan->name[CHANNELMAX] = '\0';
        chan->next = (struct channel *) 0;
        chan->owner = (struct user *) 0;
	chan->module = (struct module *) 0;
	chan->lock = C_UNLOCKED;
        chan->count = 0;
	chan->type = type;

	channeltail->next = chan;
	channeltail = chan;
}


void
initmain(mainname)
	char *mainname;
{
	channelhead = (struct channel *) malloc(sizeof(struct channel));
	if (channelhead == (struct channel *)0) {
	    logerr("malloc failed. Main channel couldn't be allocated!", SYSLOG);
	    exit(0);
	}

	channelhead->next = (struct channel *) 0;
	strncpy(channelhead->name, mainname, CHANNELMAX);
	channelhead->name[CHANNELMAX] = '\0';
	channelhead->owner = NULL;
	channelhead->lock = C_UNLOCKED;
	channelhead->count = 0;
	channelhead->type = C_USER;
	channeltail = channelhead;
}


void 
delete_channel(channel)
	struct channel *channel;
{
	struct channel *c;

	if (channel == channelhead)
	    return;
	else if (channel->type == C_MODULE) 
	    return;
	else { 
	    for (c = channelhead; c->next && channel != c->next; c = c->next);
	    c->next = channel->next;

	    if (channel == channeltail)
		channeltail = c;
	}

	FREE(channel);
}


void 
lock_channel(me)
     struct user *me;
{
	struct user *them;

	if (me->channel == channelhead) {
	    writetext(me, ">> You cannot lock the main channel.\r\n");
	    return;
	}

	if (me->channel->type == C_MODULE) {
	    writetext(me, ">> You can't lock a module's channel!\r\n");
	    return;
	}

	/* I own the channel, but it _ISN'T_ locked */
	if (me->channel->owner == me && !me->channel->lock) {
	    me->channel->lock = C_LOCKED; 
	    sprintf(buffer, ">> Channel '%s' has been locked by %s on line %d.\r\n",
		    me->channel->name, me->name, me->line);

	    for (them = userhead; them; them = them->next)
		 if (them->channel == me->channel)
		     writetext(them, buffer);

	/* I own the channel but it _IS_ locked */
	} else if (me->channel->owner == me && me->channel->lock) {
	    me->channel->lock = C_UNLOCKED;
	    sprintf(buffer, ">> Channel '%s' has been unlocked by %s on line %d.\r\n",
                    me->channel->name, me->name, me->line);

            for (them = userhead; them; them = them->next)
	         if (them->channel == me->channel)
	             writetext(them, buffer);

        /* Someone just locked a channel that has no owner. */
        } else if (!me->channel->owner) {
            me->channel->owner = me;
            me->channel->lock = C_LOCKED;
            sprintf(buffer, ">> Channel '%s' has been locked by %s on line %d.\r\n",
		    me->channel->name, me->name, me->line);

            for (them = userhead; them; them = them->next)
                 if (them->channel == me->channel)
                     writetext(them, buffer);

	/* Someone is trying to lock a channel they don't own. */
        } else if (me != me->channel->owner) {
            sprintf(buffer, ">> You can't lock channel '%s'. You don't own it.\r\n",
		    me->channel->name);
            writetext(me, buffer);
	}
}


void
channel_kick(me)
	struct user 	*me;
{
	char		*ptr = stripwhite(me->input + 2);  /* .k */
	int		who;
	struct user	*dork, *them;

	if (me->channel->owner != me) {
	    writetext(me, ">> You can't kick someone out of a channel that you don't own!\r\n");
	    return;
	}

	who = atoi(ptr);
	dork = findline(who);

	if (dork == (struct user *)0) {
	    writetext(me, BAD_LINE);
	    return;
	}

	if (dork->channel != me->channel) {
	    sprintf(buffer, ">> %c%d%c %s isn't in your channel!\r\n", 
		    bra[dork->level], dork->line, ket[dork->level], 
		    dork->name);
	    writetext(me, buffer);
	    return;
	}

	/* Else they are in our channel and I own the channel */
	dork->channel->count--;

	dork->channel = channelhead;
	dork->channel->count++;

	/* Tell main this dork has joined. */
        sprintf(buffer, ">> %s on line %d has joined.\r\n",
		dork->name, dork->line);
	for (them = userhead; them; them = them->next)
	     if ((them->channel == dork->channel) && nottoggled(them, CHANMSGS))
		 writetext(them, buffer);

	/* Tell the present channel this dork was kicked out. */
	sprintf(buffer, ">> %s on line %d has been kicked to the CURB!\r\n",
		dork->name, dork->line);
        for (them = userhead; them; them = them->next)
             if ((them->channel == me->channel) && nottoggled(them, CHANMSGS))
                 writetext(them, buffer);
}

void 
channel_list(me)
	struct user *me;
{
	struct channel *c;

	writetext(me, "  Channel      Users    Owner\r\n");

	for (c = channelhead; c; c = c->next) {
	     sprintf(buffer, "  %-*s  %-d       %s\r\n", CHANNELMAX, c->name,
		     c->count, (c->owner) ? c->owner->name : "");

	     if (c->lock)
		 buffer[0] = '*';

	     writetext(me, buffer);
	}
}
