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
#include "mhp.h"
#include "vcd.h"

void
main()
{
	int	who;
	char 	*bp, buffer[1024], text[2048], command[78], result[80];
	FILE	*fptr;

	mhp_command("calc");

	while (!feof(stdin)) {
		fgets(buffer, 1024, stdin);
		if (feof(stdin))
		    exit(9);

		buffer[ strlen(buffer)-1 ] = '\0';
		bp=buffer;

		/* strip the command flag */	
		if (*bp == 'c')
			bp++;

		/* get the line number */
		who = atoi(bp);

		/* strip number */
		while (isdigit(*bp)) 
			bp++;

		/* strip space */
		bp++;

		/* strip $I */
		if (*bp == '$')
			bp = bp+2;

		/* strip command (.calc) */
		bp = bp+5;

		if (*bp == '\0' || *bp == '\t' || *bp == '\n') {
		    mhp_write(who, ">> Syntax is .calc <equation>");
		    goto skip;
		} else if (*bp == '?') {
		    mhp_write(who, ">> VCD Calculator 1.0");
		    mhp_write(who, ">> If you are on a Unix machine type ``man bc'' for help");
		    goto skip;
		}			    

		/*
		 * Look for illegal words that may cause bc to never terminate.
		 */
		if (strstr(buffer, "for") || strstr(buffer, "while") ||
		    strstr(buffer, "define") || strstr(buffer, "read")) {
		    mhp_write(who, ">> Illegal operation in equation!");
		    goto skip;
		}

		sprintf(command, "echo 'scale=5; %s' | bc -l", bp);
		fptr = popen(command, "r");
		if (!fptr) {
		    mhp_write(who, ">> Unable to run the calculator.");
		} else {
		    fgets(result, 70, fptr);
		    pclose(fptr);

		    if (result[0] == '\0')
			mhp_write(who, ">> calc: Invalid equation!");
		    else {
			sprintf(text, ">> calc: %s", result);
			mhp_write(who, text);
		    }

		    result[0] = '\0';
		}
	skip:
	}
}
