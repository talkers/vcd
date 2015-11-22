#include <stdio.h>
#include <stdlib.h>
#include "mhp.h"
#include "vcd.h"


void
main()
{
	int	who;
	char 	buffer[1024];

	mhp_command("echo $n");

	while (!feof(stdin)) {
		fgets(buffer, 1024, stdin);
		if (feof(stdin))
		    exit(9);

		buffer[ strlen(buffer)-1 ] = '\0';

		who = atoi(buffer);

		printf("write %d >> %s\n", who, buffer);
		fflush(stdout);
	}
}
