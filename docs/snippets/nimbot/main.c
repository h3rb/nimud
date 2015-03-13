/*
 * NimBOT v0.1c for use with NimMUD 5.1a
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main( int argc, char **argv )
{
    int out_socket;
    struct sockaddr_in remote_server;
    struct hostent *hp;
    char buf[1024];

    /*
     * Reserve one channel for our use.
    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
        perror( NULL_FILE );
        exit( 1 );
    }
     */

/*
 * Attempts to resolve said host.
 */
    if ( argv[1][0] == '\0' ) {
        puts( "No address specified.\n\r" );
        exit( 1 );
    }

    out_socket = socket(AF_INET, SOCK_STREAM, 0 );
    remote_server.sin_family = AF_INET;

    hp = gethostbyname(argv[1]);
    if ( hp == NULL ) {
        fputs( "Invalid host.\n\r", stdout );
        exit( 1 );
    }

    remote_server.sin_port = htons(atoi(argv[2]));
    bcopy( hp->h_addr, &remote_server, hp->h_length);

    if ( connect( out_socket, (struct sockaddr*)&remote_server, 
	  	  sizeof(remote_server)) >= 0 ) {

    write( out_socket, argv[3], strlen(argv[3]) );
    close( out_socket );
    }

    exit( 0 );
}

