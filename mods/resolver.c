#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "mhp.h"
#include "vcd.h"


void
main()
{
	int	who, event, size = sizeof(struct in_addr);
	char 	buffer[1024], ip[80], message[80];
	struct  hostent *hp;
	struct  sockaddr_in sa;

	mhp_event(E_RESOLVE);
	mhp_channel("RESOLVER");

	while (!feof(stdin)) {
		fgets(buffer, 1024, stdin);
		if (feof(stdin))
		    exit(9);

		buffer[ strlen(buffer)-1 ] = '\0';
		sscanf(buffer, "e%d %d %s", &event, &who, (char *)&ip);

		if ((sa.sin_addr.s_addr = inet_addr((char *)ip)) == -1)
		    mhp_resolved(who, ip);
		else {
                    hp = gethostbyaddr((char *)&sa.sin_addr, size, AF_INET);
                    if (hp) {
		        mhp_resolved(who, hp->h_name);
			sprintf(message, "-- Resolved %s to %s on line %d.", ip, hp->h_name, who);
		    } else {
			mhp_resolved(who, ip);
		        sprintf(message, "-- Set host-%d to %s.", who, ip);
		    }
		}

		mhp_mod_write_chan(message);
	}
}
