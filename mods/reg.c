/*
 * VCD Loadable calculator. Requires that your system have ``bc''.
 * Angelo Brigante, Jr.
 *
 * This is a simplified version of bc. 
 * The for, while, read, and define commands have been removed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "vcd.h"

struct user *dummy;

void
main()
{
	int	who;
	char 	*bp, buffer[1024], command[78], result[80];
	FILE	*fptr;

	dummy = (struct user *) malloc(sizeof(struct user));

	printf("pstring reg $n$p\n");
	fflush(stdout);

	while (!feof(stdin)) {
		fgets(buffer, 1024, stdin);
		if (feof(stdin))
		    exit(9);

		buffer[ strlen(buffer)-1 ] = '\0';
		bp=buffer;

		/* We got a command; not an event */	
		if (*bp == 'c')
			packet_parse(dummy, buffer);
		else
			goto skip;

		if (dummy->pword[0] == '\0') {
		    printf("write %d >> You have not set a password.\n", dummy->line);
		    printf("write %d >> Type .?P to see how\n", dummy->line);
		    fflush(stdout);
		} else {
		    FILE	*fptr;
		    char	new[80];

		    sprintf(new, "%s/%c/%s", USERPATH, dummy->name[0], dummy->name);
		    fptr = fopen(new, "w");
		    fprintf(fptr, "password %s\n", dummy->pword);
		    fprintf(fptr, "name %s\n", dummy->name);
		    fclose(fptr);

		    printf("write %d >> You're registered!!!\n", dummy->line);
		    fflush(stdout);
		}
	skip:
	}
}
