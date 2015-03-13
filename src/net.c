/******************************************************************************
 * Locke's   __                      __         NIM Server Software           *
 * ___ ___  (__)__    __ __   __ ___|  | v5     Version 5 (ALPHA)             *
 * |  /   \  __|  \__/  |  | |  |      |                                      *
 * |       ||  |        |  \_|  | ()   |                                      *
 * |    |  ||  |  |__|  |       |      |                                      *
 * |____|__|___|__|  |__|\___/__|______|        http://www.nimud.org/nimud    *
 *   n a m e l e s s i n c a r n a t e          dedicated to chris cool       *
 ******************************************************************************
 * NiMUD copyright (c) 1993-2009 by Herb Gilliland                            *
 * Includes improvements by Chris Woodward (c) 1993-1994                      *
 * Based on Merc 2.1c / 2.2                                                   *
 ******************************************************************************
 * To use this software you must comply with its license.                     *
 * licenses.  See the file 'docs/COPYING' for more information about this.    *
 ******************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,           *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.      *
 *                                                                            *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael             *
 *  Chastain, Michael Quan, and Mitchell Tse.                                 *
 *                                                                            *
 *  Much time and thought has gone into this software and you are             *
 *  benefitting.  We hope that you share your changes too.  What goes         *
 *  around, comes around.                                                     *
 ******************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#if defined(BSD)
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/unistd.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <zlib.h>

#include "nimud.h"
#include "net.h"
#include "defaults.h"
//#include "sigsegv.h"

#define NOINCLUDE 1
#include "mssp.h"
#include "telnet.h"

    bool fCopyOver = FALSE;   /* Copyover Recover, a featured designed to maintain connection despite server reboots or crashes */

/*
 * Malloc debugging.
 * Sun incomprehensibility factor.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malledit.h>
extern	int	malloc_debug	args( ( int  ) );
extern	int	malloc_verify	args( ( void ) );
#endif


/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 * So dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif

/*
 * Socket and TCP/IP stuff.
 */
#if     defined(macintosh) || defined(MSDOS)
const char    go_ahead_str    [] = { '\0' };
#endif

#if     defined(unix) && !defined(ECHO_STR)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/telnet.h>

const char    go_ahead_str    [] = { IAC, GA, '\0' };
#endif

#include "network.h"


#if defined(unix)
    fd_set in_set;
    fd_set out_set;
    fd_set exc_set;
    struct timeval null_time;
#endif


/*
 * Global variables.
 */
CONNECTION *   connection_list;    /* All open connections      */

FILE *          fpReserve;              /* Reserved file handle      */

bool            newlock;                /* Game is newbie locked         */
bool            wizlock;                /* Game is wizlocked             */
bool            shut_down;              /* Shutdown mud?                 */
char            str_boot_time[MAX_INPUT_LENGTH];
time_t          current_time;           /* Time of this pulse            */
int             packet[60];             /* Communication packet info     */
int             byte_count;             /* Bytes sent.                   */
int             packet_count;           /* Packets sent.                 */
bool            QUIET_STC = FALSE;      /* If true, to_actors abort  */

extern int num_hour;

int up_time;

/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
CONNECTION dcon;                           /* Local session */

void    game_loop_mac_msdos   args( ( void ) );
bool    read_from_connection  args( ( CONNECTION *d ) );
bool    write_to_connection   args( ( int desc, char *txt, int length ) );
#endif

#if defined(unix)
void    game_loop_unix        args( ( int c ) );
int     init_socket           args( ( int p ) );
void    poll_connections      args( ( int c ) );
void    clean_connections     args( ( void ) );
void    attach_connection        args( ( int c ) );
bool    read_from_connection  args( ( CONNECTION *d ) );
bool    write_to_connection   args( ( int desc, char *txt, int length ) );
bool    write_to_descr_nice   args( ( CONNECTION *d ) );
#endif


/*
 * Local defines.
 */
#define DC(desc)   ( desc->connected )
#define MT(str, d) ( str == NULL || str[0] == '\0' ? d : str )
#define SMT(str)   ( str == NULL || str[0] == '\0' )

/*
 * Other local functions (OS-independent).
 */
void    newbie              args( ( CONNECTION *d, char *argument ) );
void    newbie_check        args( ( CONNECTION *d ) );
bool    check_parse_name   args( ( char *name ) );
bool    check_playing      args( ( CONNECTION *d, char *name ) );
bool    check_reconnect    args( ( CONNECTION *d, char *name,
                                   bool fConn ) );
int     main               args( ( int argc, char **argv ) );
bool    process_output     args( ( CONNECTION *d, bool fPrompt ) );
void    read_from_buffer   args( ( CONNECTION *d ) );
void    stop_idling        args( ( PLAYER *ch ) );
void    display_prompt     args( ( PLAYER *ch ) );
bool    apply_ok           args( ( PLAYER *ch ) );
void    print_gen_menu     args( ( PLAYER *ch ) );
void    print_doc_menu     args( ( PLAYER *ch ) );
void    print_login_menu   args( ( PLAYER *ch ) );
void    process_input      args( ( void ) );



int port = 5333;
int control=0; 

void send_cleartext_mssp( CONNECTION *d );

void handler_copyover( void );

bool fCopyOverride=FALSE;

#define COPYOVER_FILE "recovery.nfo"
#define EXE_FILE "nimud"

void handler_copyover( void ) {

        /* Close reserve and other always-open files and release other resources */
        fclose (fpReserve);

        /* exec - connections are inherited */
        go_mud_go( port, control );

        /* Failed - successful exec will not return */

        perror ("handler_copyover: execl");
        wtf_logf ("Recovery: execl() FAILED!\n\r" );
}

/* This is the handy CH() macro. I think that it was Tom Adriansen (sp?) */

#define CH(descriptor)  ((descriptor)->original ? \
(descriptor)->original : (descriptor)->character)

/*
 * Called after a player quits, or after a player enters the game, before reboots,
 * each time a connected player types.
 */
void save_copyover( void ) {
          
     CONNECTION *d, *d_next;
     FILE *fp;

     fp = fopen (COPYOVER_FILE, "w");

        if (!fp)
        {
                wtf_logf ("Could not write to recovery file: %s", COPYOVER_FILE);
                perror ("do_copyover:fopen");
                return;
        }

        /* For each playing connection, save its state */
        for (d = connection_list; d ; d = d_next)
        {
                       
                PLAYER * och = CH (d);
                d_next = d->next; /* We delete from the list , so need to save this */

                if (!d->character || d->connected > NET_PLAYING) /* drop those logging on */
                {
                }
                else
                {
                        fprintf (fp, "%d %s %s\n", d->connection, och->name, d->host);
                        save_actor_prop (och);
                }
        }

        fprintf (fp, "-1\n");
        fclose (fp);
}



void send_cleartext_mssp( CONNECTION *d ) {
   char buf[MSL];
   write_to_connection( d->connection, "\n\nMSSP-REPLY-START\n", 0 );
   snprintf( buf, MSL, "NAME\t%s\n", MUD_NAME ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "PLAYERS\t%d\n", number_range(2,20) ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "UPTIME\t%d\n", up_time ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "HOSTNAME\t%s\n", MUD_ADDRESS ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "CODEBASE\tNiMUD\n" ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "CREATED\t%d\n", 1993 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "ICON\thttp://nimud.divineright.org/nimud.ico\n" ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "IP\t12.180.48.166\n" ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "LANGUAGE\tEnglish\n" ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "LOCATION\tUnited States\n" ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "MINIMUM AGE\t%d\n", 0 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "WEBSITE\thttp://nimud.divineright.org\n" ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "FAMILY\tDikuMUD\tMerc\tTinyMUD\n" ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "GENRE\tFantasy\n"); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "GAMEPLAY\tRoleplaying\n" ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "STATUS\tLive\n" ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "GAMESYSTEM\tTyselorrd\n" ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "INTERMUD\t0\n" ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "SUBGENRE\tMultiverse\n" ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "AREAS\t%d\n", top_zone ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "HELPFILES\t%d\n", top_help ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "MOBILES\t%d\n", top_actor_template ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "OBJECTS\t%d\n", top_prop ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "ROOMS\t%d\n", top_scene ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "CLASSES\t%d\n", 1 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "LEVELS\t%d\n", 666 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "RACES\t%d\n", MAX_RACE ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "SKILLS\t%d\n", top_dbkey_skill ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "SPELLS\t%d\n", top_dbkey_spell ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "ANSI\t%d\n", 1 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "MCCP\t%d\n", 0 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "MCP\t%d\n", 0 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "MSP\t%d\n", 1 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "MXP\t%d\n", 1 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "PUEBLO\t%d\n", 0 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "VT100\t%d\n", 1 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "XTERM 256 COLORS\t%d\n", 1 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "PAY TO PLAY\t%d\n", 0 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "PAY FOR PERKS\t%d\n", 0 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "HIRING BUILDERS\t%d\n", 1 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "HIRING CODERS\t%d\n", 1 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "DBSIZE\t%d\n", 10000000 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "EXITS\t%d\n", top_exit ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "EXTRA DESCRIPTIONS\t%d\n", top_ed ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "MUDPROGS\t%d\n", top_script ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "MUDTRIGS\t%d\n", 0 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "RESETS\t%d\n", top_spawn ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "ADULT MATERIAL\t%d\n", 0 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "MULTICLASSING\t%d\n", 1 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "NEWBIE FRIENDLY\t%d\n", 1 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "PLAYER CITIES\t%d\n", 1 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "PLAYER CLANS\t%d\n", 1 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "EQUIPMENT SYSTEM\t%s\n", "Skill" ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "MULTIPLAYING\t%s\n", "Full" ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "PLAYERKILLING\t%s\n", "Full" ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "QUEST SYSTEM\t%s\n", "Integrated" ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "ROLEPLAYING\t%s\n", "Encouraged" ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "TRAINING SYSTEM\t%s\n", "Skill" ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "WORLD ORIGINALITY\t%s\n", "All original" ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "SSL\t%d\n", 0 ); write_to_connection( d->connection, buf, 0 );
   snprintf( buf, MSL, "ZMP\t%d\n", 0 ); write_to_connection( d->connection, buf, 0 );
   write_to_connection( d->connection, "MSSP-REPLY-END\n", 0 );
}


/*
 * Signal handling for copyover.
 * Make the best of crashes.
 */

void error_handler( int s ) {
   wtf_logf( "Signal was handled internally: %d", s );

/*
 * Perform copyover and reboot without saving the db.
 * Prior to this, we have stored online players in a list,
 * which is updated each time a player saves (which happens
 * anytime the player types) or quits.  Along with which
 * player, we also store their connection and their IP
 * so we can re-authenticate them during the copyover boot.
 * 
 */
   handler_copyover();

/*
 * This was part of the tutorial for signal handling, probably important.
 */
   psignal( s, "Signal thrown: " );
}




int main( int argc, char **argv )
{
    struct timeval now_time;

    up_time = time(NULL);
    /*
     * Signals we handle.
     */

   signal( SIGINT, error_handler );  // Erodes the interrupt Ctrl-C functionality to "kill" this internally from the console, Ctrl-C=hard reboot
   signal( SIGSEGV, error_handler ); // Ooops, common problems.
   signal( SIGABRT, error_handler ); // Just in case.

    /*
     * Init graphics routines.
     */
/*
    gmode( 77, 25 ); 
    page = 3; clear_page();
    page = 2; clear_page();
    page = 1; clear_page();
    page = 0; clear_page();
 */

#if defined(macintosh) || defined(MSDOS)
    {
    /*
     * Attach_connection analogue.
     */
    dcon.connection	= 0;
    dcon.connected  = NET_SHOW_TITLE;
    dcon.host		= str_dup( "localhost" );
    dcon.outsize	= 2000;
    dcon.outbuf		= alloc_mem( dcon.outsize );
    dcon.next		= connection_list;
    dcon.pEdit      = NULL;
    dcon.pString    = NULL;
    dcon.fpromptok  = TRUE;
    connection_list	= &dcon;
    }
#endif

#if defined(unix)
//    int c;
#endif

    /*
     * Memory debugging if needed.
     */
#if defined(MALLOC_DEBUG)
    malloc_debug( 2 );
#endif

    /*
     * Get boot time time.
     */
    gettimeofday( &now_time, NULL );
    current_time = (time_t) now_time.tv_sec;
    strcpy( str_boot_time, ctime( &current_time ) );

    /*
     * Macintosh console inits.
     */
#if defined(macintosh)
    console_options.nrows = 31;
    cshow( stdout );
    csetmode( C_RAW, stdin );
    cecho2file( "log file", 1, stderr );
#endif

    /*
     * Reserve one channel for our use.
     */
    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
	perror( NULL_FILE );
	exit( 1 );
    }

    /*
     * Get the port number.
     */
    port = DEF_PORT;

     if ( argc > 4 || !is_number( argv[1] ) || argc < 2 )
     {
         fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
         fprintf( stderr, "Note: running this application with two numeric arguments (ex. %s 1234 1) is for\n", argv[0] );
         fprintf( stderr, "      a special internal crash protection feature and is not required nor recommended.\n" );
         exit( 1 );
     }
     else if ( ( port = atoi( argv[1] ) ) <= 1024 )
     {
     fprintf( stderr, "Invalid (%d) Port number must be above 1024.\n", port );
     exit( 1 );
     }
    

    /*
     * Is this a recovery?
     */
    fCopyOver = argc > 2 && is_number( argv[2] );
    if ( fCopyOver )
    wtf_logf( "Recovering from previous execution." );

#if defined(IMC)
    imc_startup( FALSE );
#endif

    /*
     * Run the game.
     */
#if defined(macintosh) || defined(MSDOS)
    boot_db( 0 );
    log_string( "Local server connection established" );
    game_loop_mac_msdos( );
#endif

#if defined(unix)
    if ( !fCopyOver )
    control = init_socket( port );
    else control = atoi(argv[2]);

    boot_db( control, fCopyOver );
    sprintf( log_buf, "Server now operational on port %d.", port );
    log_string( log_buf );
    game_loop_unix( control );
    close( control );

#if defined(IMC)
    imc_shutdown( FALSE );
#endif

#endif

    /*
     * Shutdown.
     */
    log_string( "Normal termination." );
    exit( 0 );
    return 0;
}



#if defined(unix)
int init_socket( int p )
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    int fd;

    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
	perror( "Init_socket: socket" );
	exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
    (char *) &x, sizeof(x) ) < 0 )
    {
	perror( "Init_socket: SO_REUSEADDR" );
	close( fd );
	exit( 1 );
    }

#if defined(SO_DONTLINGER) && !defined(SYSV)
    {
	struct	linger	ld;

	ld.l_onoff  = 1;
	ld.l_linger = 1000;

	if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
	(char *) &ld, sizeof(ld) ) < 0 )
	{
	    perror( "Init_socket: SO_DONTLINGER" );
	    close( fd );
	    exit( 1 );
	}
    }
#endif

    sa		    = sa_zero;
    sa.sin_family   = AF_INET;
    sa.sin_port	    = htons( p );

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
    {
	perror( "Init_socket: bind" );
	close( fd );
	exit( 1 );
    }

    if ( listen( fd, 3 ) < 0 )
    {
	perror( "Init_socket: listen" );
	close( fd );
	exit( 1 );
    }

    return fd;
}
#endif



/*
 * Output.
 */
void connection_output( void )
{
    CONNECTION *d;
    CONNECTION *d_next;
    
#if defined(unix)
        for ( d = connection_list; d != NULL; d = d_next )
        {
            d_next = d->next;

            if ( ( d->fcommand || d->outtop > 0 )
            &&   FD_ISSET(d->connection, &out_set) )
            {
                if ( !process_output( d, TRUE ) )
                {
                    if ( d->character != NULL )
                        save_actor_prop( d->character );
                    d->outtop   = 0;
                    close_socket( d );
                }
            }
        }
#endif

#if defined(macintosh) || defined(MSDOS)
        for ( d = connection_list; d != NULL; d = d_next )
        {
            d_next = d->next;

            if ( ( d->fcommand || d->outtop > 0 ) )
            {
                if ( !process_output( d, TRUE ) )
                {
                    if ( d->character != NULL )
                        save_actor_prop( d->character );
                    d->outtop   = 0;
                    close_socket( d );
                }
            }
        }
#endif
}


/*
 * Process input.
 */
void process_input( void )
{
    CONNECTION *d, *d_next;

    for ( d = connection_list; d != NULL; d = d_next )
    {
        d_next      = d->next;
        d->fcommand = FALSE;

#if defined(MSDOS)
        if ( kbhit( ) )
#endif
#if defined(unix)
        if ( FD_ISSET( d->connection, &in_set ) )
#endif
        {
            if ( d->character != NULL )
                  d->character->timer = 0;

            if ( !read_from_connection( d ) )
            {
#if defined(unix)
                 FD_CLR( d->connection, &out_set );
#endif
                 if ( d->character != NULL )
                      save_actor_prop( d->character );
                 d->outtop   = 0;
                 close_socket( d );
                 continue;
            }
        }

        if ( d->character != NULL && d->character->wait > 0 )
        {
            if (IS_IMMORTAL(d->character))
                      d->character->wait = 0;
                 else --d->character->wait;
            continue;
        }


        read_from_buffer( d );
        if ( d->incomm[0] != '\0' )
        {
             d->fcommand     = TRUE;
             stop_idling( d->character );

             if ( d->pString != NULL ) string_add( d->character,  d->incomm );
             else
             if ( d->pager_point )   page_string( d, d->incomm );
             else
             {
             switch ( d->connected )
             {
                 case NET_PLAYING: interpret( d->character, d->incomm ); break;
                 case NET_ZEDITOR: zedit( d->character, d->incomm );     break;
                 case NET_REDITOR: redit( d->character, d->incomm );     break;
                 case NET_OEDITOR: oedit( d->character, d->incomm );     break;
                 case NET_AEDITOR: aedit( d->character, d->incomm );     break;
                 case NET_SEDITOR: sedit( d->character, d->incomm );     break;
                 case NET_HEDITOR: hedit( d->character, d->incomm );     break;
                 case NET_SPEDITOR: spedit( d->character, d->incomm );     break;
                 case NET_SKEDITOR: skedit( d->character, d->incomm );     break;
                          default: newbie( d, d->incomm );                break;
             }
             }

             /*
              * Application process stuff.  Kludgy.
              */
             if ( d->character != NULL )
                 newbie_check( d );

        d->incomm[0]    = '\0';
        }
    }
    return;
}




/*
 * Mac/msdos game loop.
 */
#if defined(macintosh) || defined(MSDOS)

void game_loop_mac_msdos( void )
{
    struct timeval last_time;
    struct timeval now_time;

    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /* Main loop */
    while ( !shut_down )
    {
	CONNECTION *d;


    process_input( );

	/*
	 * Autonomous game motion.
	 */
 	update_handler( );

    connection_output( );

	/*
	 * Synchronize to a clock.
	 * Busy wait (blargh).
	 */
	now_time = last_time;
	for ( ; ; )
	{
	    int delta;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( dcon.character != NULL )
		    dcon.character->timer = 0;
		if ( !read_from_connection( &dcon ) )
		{
            d = &dcon;

		    if ( dcon.character != NULL )
			save_actor_prop( d->character );
		    dcon.outtop	= 0;
		    close_socket( &dcon );
		}
#if defined(MSDOS)
		break;
#endif
	    }

	    gettimeofday( &now_time, NULL );
	    delta = ( now_time.tv_sec  - last_time.tv_sec  ) * 1000 * 1000
              + ( now_time.tv_usec - last_time.tv_usec );
	    if ( delta >= 1000000 / PULSE_PER_SECOND )
		break;
	}
	last_time    = now_time;
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif



/*
 * Unix-loop routines.
 */
#if defined(unix)


/*
 * Get rid of drops.
 */
void clean_connections( void )
{
    CONNECTION *d, *d_next;

	/*
	 * Kick out the freaky folks.
	 */
	for ( d = connection_list; d != NULL; d = d_next )
	{
	    d_next = d->next;   
	    if ( FD_ISSET( d->connection, &exc_set ) )
	    {
		FD_CLR( d->connection, &in_set  );
		FD_CLR( d->connection, &out_set );
		if ( d->character )
		    save_actor_prop( d->character );
		d->outtop	= 0;
		close_socket( d );
	    }
	}
    return;
}


/*
 * Poll all active connections.
 */
void poll_connections( int c )
{
    CONNECTION *d;
    int maxdesc;

	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( c, &in_set );
	maxdesc	= c;
	for ( d = connection_list; d; d = d->next )
	{
	    maxdesc = UMAX( maxdesc, d->connection );
	    FD_SET( d->connection, &in_set  );
	    FD_SET( d->connection, &out_set );
	    FD_SET( d->connection, &exc_set );
	}

	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	    perror( "Game_loop: select: poll" );
	    exit( 1 );
	}

	/*
	 * New connection?
     */
    if ( FD_ISSET( c, &in_set ) )
	    attach_connection( c );
    return;
}


/*
 * Unix game loop.
 */
void game_loop_unix( int c )
{
    struct timeval last_time;

    signal( SIGPIPE, SIG_IGN );
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /* Main loop */
    while ( !shut_down )
    {

#if defined(MALLOC_DEBUG)
	if ( malloc_verify( ) != 1 )
	    abort( );
#endif

        poll_connections( c );
        clean_connections( );
        process_input( );

	/*
	 * Autonomous game motion.
	 */
#if defined(IMC)
        imc_loop();   /* IMC2 */
#endif

        update_handler( );

    connection_output( );

	/*
	 * Synchronize to a clock.
	 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	 * Careful here of signed versus unsigned arithmetic.
	 */
	{
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;

	    gettimeofday( &now_time, NULL );
	    usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SECOND;
	    secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
	    while ( usecDelta < 0 )
	    {
		usecDelta += 1000000;
		secDelta  -= 1;
	    }

	    while ( usecDelta >= 1000000 )
	    {
		usecDelta -= 1000000;
		secDelta  += 1;
	    }

	    if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
	    {
		struct timeval stall_time;

		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;
		if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
		{
		    perror( "Game_loop: select: stall" );
		    exit( 1 );
		}
	    }
	}

    gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
    }

    if ( !fCopyOverride ) { handler_copyover(); exit(0); }
    return; // end of main loop
}



/*
 * New connections must pass through here.
 */
void attach_connection( int c )
{
    char buf[MAX_STRING_LENGTH];
    CONNECTION *dnew;
    BAN *pban;
    struct sockaddr_in sock;
    struct hostent *from;
    int desc;
    unsigned int size;
    extern bool fBootDb;

    size = sizeof(sock);
    getsockname( c, (struct sockaddr *) &sock, &size );
    if ( ( desc = accept( c, (struct sockaddr *) &sock, &size) ) < 0 )
    {
	perror( "Attach_connection: accept" );
	return;
    }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
    {
	perror( "Attach_connection: fcntl: FNDELAY" );
	return;
    }

    /*
     * Cons a new connection.
     */
    dnew = new_connection( );
    dnew->connection = desc;

    size = sizeof(sock);
    if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 )
    {
	perror( "Attach_connection: getpeername" );
	dnew->host = str_dup( "(unknown)" );
    }
    else
    {
 	/*
	 * Would be nice to use inet_ntoa here but it takes a struct arg,
	 * which ain't very compatible between gcc and system libraries.
	 */
	int addr;

	addr = ntohl( sock.sin_addr.s_addr );
	snprintf( buf, MAX_STRING_LENGTH, "%d.%d.%d.%d",
	    ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
	    ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
	    );
	sprintf( log_buf, "Sock.sinaddr:  %s", buf );
	log_string( log_buf );

/*
 * Bot ignore list; hard-coded. 
 */
     if ( str_cmp( buf, "63.227.23.180" ) 
       && str_cmp( buf, "67.19.128.11"  )
       && str_cmp( buf, "66.101.59.248" ) ) {
    sprintf( log_buf, "Notify>  New connection: %s", buf );
    NOTIFY( log_buf, LEVEL_IMMORTAL, WIZ_NOTIFY_LOGIN );
        }   /* ignore moosh.net's annoying bot */

	from = gethostbyaddr( (char *) &sock.sin_addr,
	    sizeof(sock.sin_addr), AF_INET );
	dnew->host = str_dup( from ? from->h_name : buf );
    }
	
    /*
     * Swiftest: I added the following to ban sites.  I don't
     * endorse banning of sites, but Copper has few connections now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Furey: added suffix check by request of Nickel of HiddenWorlds.
     */
    for ( pban = ban_list; pban != NULL; pban = pban->next )
    {
    if ( /*!*/str_suffix( pban->name, dnew->host ) )
	{
        write_to_connection( desc, "Your site has been banned.\n\r", 0 );
	    close( desc );
        free_connection( dnew );
	    return;
	}
    }

    /*
     * Init connection data.
     */
    dnew->showing=0;
    dnew->next			= connection_list;
    connection_list		= dnew;

    /*
     * Send the greeting.
     */
    if ( !fBootDb )
    {
	extern char * help_greeting;

// IAC WONT LINEMODE IAC WILL (373) ECHO
//    write_to_buffer(dnew, "\377\375\042\377\373\001",6);  // throws telnet into character mode
    send_char_mode( dnew );
    //announce_support( dnew ); // mssp

    write_to_buffer(dnew, VERSION_STR, 0 );
    write_to_buffer(dnew, "\n\r", 0 );

	if ( help_greeting[0] == '.' )
	    write_to_buffer( dnew, help_greeting+1, 0 );
	else
	    write_to_buffer( dnew, help_greeting  , 0 );
    }
    else write_to_buffer( dnew, "Please wait.\n\r", 0 );

    return;
}
#endif



void close_socket( CONNECTION *dclose )
{
    PLAYER *ch;

    if ( dclose->outtop > 0 )
    process_output( dclose, FALSE );

    /*
     * Nail snoopers.
     */
    if ( dclose->snoop_by != NULL )
    {
        char buf[MAX_STRING_LENGTH];
        snprintf( buf, MAX_STRING_LENGTH, "*%s*  Lost link.\n\r",
                 dclose->character ? dclose->character->name : "(null)" );
        write_to_buffer( dclose->snoop_by, buf, 0 );
    }

    /*
     * Nail snooped.
     */
    {
	CONNECTION *d;

	for ( d = connection_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == dclose )
		d->snoop_by = NULL;
	}
    }


    if ( ( ch = dclose->character ) != NULL )
    {
    /*
     * Purge followers.
     */
    {
        PLAYER *pet;
        PLAYER *pet_next;

        for ( pet = actor_list; pet != NULL; pet = pet_next )
        {
            pet_next = pet->next;
            if ( ch != pet
              && ( pet->master == ch )
              && IS_SET(pet->flag, ACTOR_PET) )
            extractor_char( pet, TRUE );
        }
    }

	sprintf( log_buf, "Closing link to %s.", ch->name );
    log_string( log_buf );
    if ( CONNECTED(dclose) )
	{
	    act( "$n has lost $s link.", ch, NULL, NULL, TO_SCENE );
	    ch->desc = NULL;
	}
	else
    {
	    free_char( dclose->character );
        dclose->character = NULL;
    }
    }

    if ( dclose == connection_list ) connection_list = connection_list->next;
    else
    {
	CONNECTION *d;

    for ( d = connection_list; d != NULL && d->next != dclose; d = d->next )
    ;

    if ( d != NULL )   d->next = dclose->next;
    else               bug( "Close_socket: dclose not found.", 0 );
    }

    dclose->next = NULL;
    close( dclose->connection );
    free_connection( dclose );

#if defined(MSDOS) || defined(macintosh)
    exit(1);
#endif
    return;
}



/*
 * Read and place in buffer.
 */
bool read_from_connection( CONNECTION *d )
{
    int iStart;

    /*
     * Abort if pending command already.
     */
    if ( d->incomm[0] != '\0' )
	return TRUE;


    /*
     * Check for overflow.
     */
    d->showing = iStart = strlen(d->inbuf);
    if ( iStart >= sizeof(d->inbuf) - 10 )
    {
	sprintf( log_buf, "%s input overflow!", d->host );
	log_string( log_buf );
	write_to_connection( d->connection,
        "\n\r*** BUFFER OVERFLOW ***\n\r", 0 );
	return FALSE;
    }

    /*
     * Grab input.
     */
#if defined(macintosh)
    for ( ; ; )
    {
	int c;
	c = getc( stdin );
	if ( c == '\0' || c == EOF )
	    break;
	putc( c, stdout );
	if ( c == '\r' )
	    putc( '\n', stdout );
	d->inbuf[iStart++] = c;
	if ( iStart > sizeof(d->inbuf) - 10 )
	    break;
    }
#endif

#if defined(MSDOS) || defined(unix)
    for ( ; ; )
    {
        char bufin[MIL];
	int nRead;

#if defined(NEVER)
	nRead = read( d->connection, /* bufin, */ d->inbuf + iStart,
	  /* sizeof(bufin)-10-iStart ); */ sizeof(d->inbuf) - 10 - iStart );
	if ( nRead > 0 )
	{  
            d->showing=iStart;
            iStart += nRead;
//  	    iStart += translate_telopts(d, bufin, nRead, d->inbuf+iStart );
            if( d->inbuf[iStart - 1] == '\n'
             || d->inbuf[iStart - 1] == '\r' )
               break;
#endif 
        nRead = read( d->connection, bufin, sizeof(bufin)-10-iStart );
        if ( nRead > 0 )
        {
            d->showing=iStart;

            iStart += translate_telopts(d, bufin, nRead, d->inbuf+iStart );

            if( d->inbuf[iStart - 1] == '\n'
             || d->inbuf[iStart - 1] == '\r' )
               break;
	}
	else if ( nRead == 0 )
	{
         char buf[MAX_STRING_LENGTH];

         if ( d != NULL && d->character != NULL
          && str_cmp( d->character->name, "INFO" ) ) {
         wtf_logf( "%s disconnected.", NAME(d->character) );
         snprintf( buf, MAX_STRING_LENGTH, "Notify> %s has disconnected (EOF).",
                      (d != NULL && d->character != NULL)
                                  ? d->character->name : "Guest" );
         NOTIFY( buf, LEVEL_IMMORTAL, WIZ_NOTIFY_LOGIN );
        } 

        return FALSE;
	}
    else if ( errno == EWOULDBLOCK )  break;
	else
	{
        perror( "Read_from_connection" );
	    return FALSE;
	}
    }
#endif

    d->inbuf[iStart] = '\0';
    return TRUE;
}




/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( CONNECTION *d )
{
    int i, j, k;

    /*
     * Hold horses if pending command already.
     */
//    if ( d->incomm[0] != '\0' )
//	return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( d->inbuf[i] == '\0' ) {
            d->showing=i;
	    return;
        }

        if ( d->character && d->inbuf[i] == '\x1b' ) {
              if ( d->inbuf[i+1] == '[' ) {
                  char acode[5];
                  acode[0] = '[';  acode[1] = d->inbuf[i+2];  acode[2] = d->inbuf[i+3];
                  if ( d->inbuf[i+3] == '~' ) acode[3]='\0'; 
                  else if ( d->inbuf[i+4] == '~' ) { acode[3]=d->inbuf[i+4]; acode[4]='\0'; }
                  else acode[2] = '\0';
                  ansi_keyboard( d->character, acode );
                  d->inbuf[0]='\0'; d->showing=0; return;
              }
          } else
           if ( i >= d->showing && DC(d) != NET_GET_NEW_PASSWORD 
            &&  DC(d) != NET_CONFIRM_NEW_PASSWORD 
            &&  DC(d) != NET_GET_OLD_PASSWORD ) { 

                if ( d->inbuf[i] == '\b' || d->inbuf[i] == 127 ) { char b[3]; b[0]='\b'; b[1]=' '; b[2]='\b';
                 write_to_connection( d->connection, b, 3 ); d->showing=i;
                } else
                if ( char_isof(d->inbuf[i],
" `1234567890-=qwertyuiop[]\\asdfghjkl;'zxcvbnm,./~!@#$%^&*()_+QWERTYUIOP{}|ASDFGHJKL:\"ZXCVBNM<>?\n" )
 ) { char b[2]; b[0]=d->inbuf[i]; b[1]='\0';
                 write_to_connection( d->connection, b, 1 ); d->showing=i; } //simulated local echo

              }
    } // end for

        write_to_buffer( d, "\n", 1 );

    /*
     * Canonical input processing. 
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( k >= MAX_INPUT_LENGTH - 2 )
	{
        write_to_connection( d->connection, "Line too long.  Truncating.\n\r", 0 );

	    /* skip the rest of the line */
	    for ( ; d->inbuf[i] != '\0'; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( (d->inbuf[i] == '\b' || d->inbuf[i] == 127) && k > 0 )  // backspace and delete
	    --k;
	else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */
    if ( k > 1 || d->incomm[0] == '!' )
    {
    	if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
	{
	    d->repeat = 0;
	}
	else
	{
        if ( ++d->repeat >= 50 && PC(d->character,level) <= LEVEL_HERO )
	    {
		sprintf( log_buf, "%s input spamming!", d->host );
		log_string( log_buf );
		write_to_connection( d->connection,
            "\n\r*** REPEAT OVERFLOW ***\n\r", 0 );
		strcpy( d->incomm, "quit" );
	    }
	}
    }

    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
	strcpy( d->incomm, d->inlast );
    else
	strcpy( d->inlast, d->incomm );

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}

/*
 * Generates a bar-graph for combat and other stats.
 * Move to graphics.c
 */
void gen_bar_graph( PLAYER *ch, int percent ) {
   int filler = percent-100;

   if ( HAS_ANSI(ch) ) to_actor( INVERSE, ch );
   to_actor( "[ ", ch );
   
   while ( percent > 0 ) { 
      percent -= 10;   
      if ( HAS_ANSI(ch) ) {
      if ( percent > 80 ) { to_actor( RED, ch ); }
      else if ( percent > 60 ) 
                          { to_actor( RED, ch ); }
      else if ( percent > 40 )
                          { to_actor( YELLOW, ch );  }
      else if ( percent > 20 )
                          { to_actor( CYAN, ch ); }
      else                { to_actor( BLUE, ch ); }
      }
      to_actor( "  ", ch );
   }

   if ( HAS_ANSI(ch) ) to_actor( NTEXT, ch );
   if ( HAS_ANSI(ch) ) to_actor( INVERSE, ch );

   while ( filler < 0 ) {
      if ( HAS_ANSI(ch) ) to_actor( "  ", ch );
      else to_actor( "[]", ch );
      filler +=10;
   }

   to_actor( "]", ch );
   if ( HAS_ANSI(ch) ) to_actor( NTEXT, ch );
   return;   
}

/*
 * Prompt and string variables.
 */
void display_interp(PLAYER *ch, const char * str )
{
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   const char *i="";
   char *p;

   if ( NPC(ch) ) { /*to_actor( str, ch );*/ return; }

//   ansi_color( NTEXT, ch );
//   ansi_color( default_color_variable_di, ch );  /* NEEDS FIXED */

   p = buf;
   while( *str != '\0' )
   {
      if( *str != '^' )
      {
         *p++ = *str++;
         continue;
      }
      ++str;
      switch( *str )
      {
         default: i = " "; break;
        case '+': i = !IS_SET( ch->flag2, PLR_ANSI ) ? "" :  B_BLACK;   break;
        case ':': i = !IS_SET( ch->flag2, PLR_ANSI ) ? "" :  B_BLUE;    break;
        case '#': i = !IS_SET( ch->flag2, PLR_ANSI ) ? "" :  B_GREEN;   break;
        case '<': i = !IS_SET( ch->flag2, PLR_ANSI ) ? "" :  B_CYAN;    break;
        case '?': i = !IS_SET( ch->flag2, PLR_ANSI ) ? "" :  B_RED;     break;
        case '>': i = !IS_SET( ch->flag2, PLR_ANSI ) ? "" :  B_PURPLE;  break;
        case '-': i = !IS_SET( ch->flag2, PLR_ANSI ) ? "" :  B_YELLOW;  break;
        case '=': i = !IS_SET( ch->flag2, PLR_ANSI ) ? "" :  B_GREY;    break;

        case '0': i = !IS_SET( ch->flag2, PLR_ANSI ) ? "" :  BLACK;   break;
        case '1': i = !IS_SET( ch->flag2, PLR_ANSI ) ? "" :  BLUE;    break;
        case '2': i = !IS_SET( ch->flag2, PLR_ANSI ) ? "" :  GREEN;   break;
        case '3': i = !IS_SET( ch->flag2, PLR_ANSI ) ? "" :  CYAN;    break;
        case '4': i = !IS_SET( ch->flag2, PLR_ANSI ) ? "" :  RED;     break;
        case '5': i = !IS_SET( ch->flag2, PLR_ANSI ) ? "" :  PURPLE;  break;
        case '6': i = !IS_SET( ch->flag2, PLR_ANSI ) ? "" :  YELLOW;  break;
        case '7': i = !IS_SET( ch->flag2, PLR_ANSI ) ? "" :  GREY;    break;
        case 'B': i = !IS_SET( ch->flag2, PLR_ANSI ) ? "" :  BOLD;    break;
        case 'I': i = !IS_SET( ch->flag2, PLR_ANSI ) ? "" :  INVERSE; break;
        case 'F': i = !IS_SET( ch->flag2, PLR_ANSI ) ? "" :  FLASH;   break;
        case 'N': sprintf( buf2, "%s", NTEXT );//, default_color_variable ); 
                  i = !IS_SET( ch->flag2, PLR_ANSI ) ? "" :  buf2;    break;

        case 'f': if (ch->fighting) 
                  gen_bar_graph( ch, PERCENTAGE(ch->fighting->hit, 
                                     MAXHIT(ch->fighting)) ); 
                break;

        case 'e': sprintf( buf2, "%s", STRING_MANA(ch) );
                  i = buf2;
                break;
        case 'E':
                  sprintf( buf2, "%d", PERCENTAGE(ch->mana, MAXMANA(ch)) );
                  i = buf2;
                break;
        case 'h': sprintf( buf2, "%s", STRING_HITS(ch) );
                  i = buf2;
                break;
        case 'H':
                  sprintf( buf2, "%d", PERCENTAGE(ch->mana, MAXHIT(ch)) );
                  i = buf2;
                break;
        case 'p': if ( ch->monitor )
                  sprintf( buf2, "%d%%", PERCENTAGE(ch->monitor->hit, MAXHIT(ch->monitor)) );
                  else sprintf( buf2, "-" );
                  i = buf2;
                break;
        case 'm': sprintf( buf2, "%s", STRING_MOVES(ch)  );
                  i = buf2;
                break;
        case 'M':
                  sprintf( buf2, "%d", PERCENTAGE(ch->move, MAXMOVE(ch)) );
                  i = buf2;
                break;
        case 'g': sprintf( buf2, "%s", name_amount( tally_coins( ch ) ) );
                  i = buf2;
                break;
        case 'r': if( IS_IMMORTAL( ch ) && ch->in_scene != NULL )
                  sprintf( buf2, "%d", ch->in_scene->dbkey );
                  else
                  sprintf( buf2, " " );
                  i = buf2;
                break;
        case 'z': if( IS_IMMORTAL( ch ) && ch->in_scene != NULL )
                  sprintf( buf2, "%s", ch->in_scene->zone->name );
                  else
                  sprintf( buf2, " " );
                  i = buf2;
                break;
        case 'Z': if( IS_IMMORTAL( ch ) && ch->in_scene != NULL )
                  sprintf( buf2, "%d", ch->in_scene->zone->dbkey );
                  else
                  sprintf( buf2, " " );
                  i = buf2;
                break;
        case 'v': if ( PC(ch,wizinvis) > 0 )
                  sprintf(buf2, "%d", PC(ch,wizinvis) );
                  else
                  if (IS_AFFECTED(ch, BONUS_INVISIBLE))
                  sprintf(buf2, "i");
                  else
                  sprintf(buf2, "V");
                  i = buf2;
                break;
        case 'c': if ( IS_IMMORTAL( ch ) )
                  {
                      switch (ch->desc->connected)
                      {
                           case NET_ZEDITOR: sprintf( buf2, "Zone" );  break;
                           case NET_REDITOR: sprintf( buf2, "Scene" ); break;
                           case NET_OEDITOR: sprintf( buf2, "Prop" );  break;
                           case NET_AEDITOR: sprintf( buf2, "Actor" ); break;
                           case NET_SEDITOR: sprintf( buf2, "Script"); break;
                           case NET_HEDITOR: sprintf( buf2, "Help" );  break;
                           case NET_SKEDITOR:sprintf( buf2, "Skill");  break;
                           case NET_SPEDITOR:sprintf( buf2, "Spell");  break;
                                    default: buf2[0] = '\0';           break;
                      }
                  }
                  else sprintf( buf2, " " );
                  i = buf2;
                break;
        case 'C': if ( IS_IMMORTAL( ch ) )
                  {
                      ZONE *pZone;
                      SCENE *pScene;
                      PROP_TEMPLATE *pProp;
                      ACTOR_TEMPLATE *pActor;
                      HELP *pHelp;

                  switch (ch->desc->connected)
                  {
                    case NET_ZEDITOR:
                    {
                        pZone = (ZONE *)ch->desc->pEdit;
                        sprintf( buf2, "%d", pZone != NULL ? pZone->dbkey : 0 );
                        break;
                    }
                    case NET_REDITOR:
                    {
                        pScene = ch->in_scene;
                        sprintf( buf2, "%d", pScene != NULL ? pScene->dbkey : 0 );
                        break;
                    }
                    case NET_AEDITOR:
                    {
                        pActor = (ACTOR_TEMPLATE *)ch->desc->pEdit;
                        sprintf( buf2, "%d", pActor  != NULL ? pActor->dbkey  : 0 );
                        break;
                    }
                    case NET_OEDITOR:
                    {
                        pProp = (PROP_TEMPLATE *)ch->desc->pEdit;
                        sprintf( buf2, "%d", pProp  != NULL ? pProp->dbkey  : 0 );
                        break;
                    }
                    case NET_HEDITOR:
                    {
                        pHelp = (HELP *)ch->desc->pEdit;
                        sprintf( buf2, "%s", pHelp != NULL ? pHelp->name : "No Help Entry Selected - use hedit <dbkey|list> to select" );
                        break;
                    }
                    default:
                        buf2[0] = '\0';
                        break;
                  }
                  }
                  else sprintf( buf2, " " );
                  i = buf2;
                break;
         case 'n' : sprintf( buf2, "\n\r" );  i = buf2; break;
         case '^' : sprintf( buf2, "^" );     i = buf2; break;
      } 
      ++str;
      while( (*p = *i) != '\0' )
         ++p, ++i;      
   }
   *p='\0';
   page_to_actor( buf, ch );
/*   write_to_buffer( ch->desc, buf, p - buf );*/
    return;
}

/*
 * Display prompt (player settable prompt).
 */
void display_prompt( PLAYER *ch )
{
   if( ch->prompt == NULL || ch->prompt[0] == '\0' )
   {
      to_actor( "\n\r\n\r", ch );
      return;
   }

   display_interp( ch, ch->prompt );
   return;
}



/*
 * Low level output function.
 */
bool process_output( CONNECTION *d, bool fPrompt )
{
    fPrompt = (d->fpromptok ? fPrompt : 0);
    /*
     * Bust a prompt.
     */
    if ( d->pString != NULL )       write_to_buffer( d, "> ", 2 );
    else
    if ( fPrompt  && !shut_down )
    {
    if ( d->pager_point ) write_to_buffer( d,  "<more> ",    0 );
    else
    if ( CONNECTED(d) )
    {
    PLAYER *ch;

	ch = d->original ? d->original : d->character;
    if ( ch != NULL )
    {
    if ( IS_SET(ch->flag2, PLR_BLANK) && !d->client )
	    write_to_buffer( d, "\n\r", 2 );

    if ( IS_SET(ch->flag2, PLR_PROMPT) && !d->client )
            display_prompt( d->character );

//    if ( IS_SET(d->character->flag2, PLR_TELNET_GA) && !d->client )
//	    write_to_buffer( d, go_ahead_str, 0 );
    }

    }
    }

    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
	return TRUE;

    /*
     * OS-dependent output.
     */
#if !defined(unix)
    if ( !write_to_connection( d->connection, d->outbuf, d->outtop ) )
    {
	d->outtop = 0;
	return FALSE;
    }
    else
    {
	d->outtop = 0;
	return TRUE;
    } 
#else
    if ( !write_to_descr_nice( d ) )
      {
        d->outtop = 0;
        return FALSE;
      }
    else
      {
        return TRUE;
      }
#endif
}



/*
 * Identical to Kalgen's write_to_buffer()
 */
void sendbuf( CONNECTION *d, const char *txt, int length )
{
    extern bool fBootDb;
    /*
     * Find length in case owner didn't.
     */
    if ( length <= 0 )
        length = strlen(txt);
 
    /*
     * Initial \n\r if needed.
     */
    if ( !fBootDb && d->outtop == 0 && !d->fcommand )
    {
        d->outbuf[0]    = '\n';
        d->outbuf[1]    = '\r';
        d->outtop       = 2;
    }
 
    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )
    {
        char *outbuf;
 
        if ( d->outsize >= 64000 )
        {
            d->outtop = 0;
            write_to_buffer( d, "OVER 64000 BYTES.  TRUNCATED.\n\r", 0 );
            return;
        }
        outbuf      = alloc_mem( 2 * d->outsize );
        strncpy( outbuf, d->outbuf, d->outtop );
        free_mem( d->outbuf, d->outsize );
        d->outbuf   = outbuf;
        d->outsize *= 2;
    }
 
    /*
     * Copy.
     */
    strcpy( d->outbuf + d->outtop, txt );
    d->outtop += length;
    return;
}

/*
 * Write to the connection's output buffer.
 */
void write_to_buffer( CONNECTION *d, const char *txt, int length )
{
#if defined(TRANSLATE)
    char *translated;
    char *b;
 
    if ( length <= 0 ) length=strlen(txt);
    b = str_dup( txt );
    translated = translate( d, b );
    sendbuf( d, translated, 0 );
    free_string(b);
    free_string(translated);
#else
    sendbuf( d, txt, length );
#endif

    return;
}




/*
 * Lowest level output function.
 * Write a block of text to the file connection.
 */
bool write_to_connection( int desc, char *txt, int length )
{
    int iStart;
    int nWrite;
    int nBlock;

#if defined(macintosh) || defined(MSDOS)
    if ( desc == 0 )
    desc = 1;
#endif

    if ( length <= 0 )
	length = strlen(txt);

    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
        nBlock = UMIN( length - iStart, 4096 );
        if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
        {
            perror( "Write_to_connection" );
            return FALSE;
        }
    } 

    return TRUE;
}

 

/*
 * Spill the blood let it run onto me.
 * Clipped from Kalgen.
 */
void parse_snoop( CONNECTION *d, const char *txt )
{
    char str[MAX_STRING_LENGTH];
    PLAYER *ch;
    int i, j;
    
    if ( d->character == NULL )
    {
        bug( "parse_snoop: NULL passed", 0 );
        return;
    }
    else if ( d->original )
    {
        ch = d->original;
    }
    else
    {
        ch = d->character;
    }
 
    for ( i = 0,j=0; j < (MAX_STRING_LENGTH-3) ; i++, j++ )
    {
        if ( txt[i] == 0 )
        {
/*
        if ( j > 1 )
	    {
                str[j] = 0;
                write_to_buffer( d->snoop_by, str, 0 );
		}
*/
        return;
        }
 
        if ( txt[i] == '\r' )
        {
            j--;
            continue;
        }
 
        if ( j == 0 )
        {
            for ( j = 0; j < MAX_INPUT_LENGTH; j++ )
              str[j] = 0;
              
            if ( HAS_ANSI(d->snoop_by->character) )
            strcat( str, BOLD );
            
            strcat( str, STR(d->character, name) );
            str[4+3]=0;
            strcat( str, "* " );
            
            if ( HAS_ANSI(d->snoop_by->character) )
            strcat( str, ANSI_NORMAL );
            
            j = strlen( str );
        }
 
        if ( txt[i] == '\n' )
        {
            str[j] = '\n';
            j++;
            str[j] = '\r';
            j++;
            str[j] = 0;
            if ( j >= 10 )
              write_to_buffer( d->snoop_by, str, 0 );
            str[0] = 0;
            j = -1;
        }
        else str[j] = txt[i];
    }

    return;
}
 



/*
 * This scriptedure, an alterate to using the above, will NOT crash the
 * link by spamming the output buffer on the remote side.  This was
 * submitted for your approval by Kalgen of his mud, Zebesta.
 */
bool write_to_descr_nice( CONNECTION *d )
{
    int iStart;
    int nWrite;
    int nBlock;
    int length;
 
 
    length = d->outtop;
 
    iStart = 0;
    nBlock = UMIN( length - iStart, 4096 );
 
    if ( ( nWrite = write( d->connection, d->outbuf + iStart, nBlock ) ) < 0 )
    {
        byte_count += nBlock;
        perror( "Write_to_connection" );
        return FALSE;
    }
 
    if ( (length - nWrite) )
    {
        int i;

        for ( i = 0; (i + nWrite) < length; i++ )
           d->outbuf[i] = d->outbuf[i+nWrite];

        d->outtop -= nWrite;
        d->fpromptok = FALSE;
    }
    else
    {
        if ( d->snoop_by )
        {
          d->outbuf[d->outtop] = 0;
          parse_snoop( d, d->outbuf );
        }
 
        d->fpromptok = TRUE;
        d->outtop = 0;
    }

    packet[current_time % 60] += nWrite;
    packet_count += nWrite;
    return TRUE;
}



/*
 * Spawn counter.
 * Do anything else.
 */
void stop_idling( PLAYER *ch )
{
    if ( ch == NULL
    ||   ch->desc == NULL
    ||   ch->desc->connected > NET_PLAYING
    ||   ch->in_scene != get_scene( SCENE_VNUM_LIMBO ) )
	return;

    ch->timer = 0;
    return;
}




/*
 * Write to one char.
 */
void to_actor( const char *txt, PLAYER *ch )
{
    if ( QUIET_STC != TRUE
      && txt != NULL
      && ch != NULL
      && ch->desc != NULL ) display_interp( ch, txt );
//        write_to_buffer/*_interp*/( ch->desc, txt, strlen(txt) );
    return;
}



/*
 * Send a page to one char.
 */
void page_to_actor( const char *txt, PLAYER *ch )
{
    int fOldLen=ch->pagelen;

    if ( txt == NULL || ch->desc == NULL )
        return;

    /*
     * No pagers.
     */
    if ( ch->pagelen <= 0 ) 
    ch->pagelen=512;  // max single line output buffer, probably large than any used.       

    /*
     * If there is already some data being "paged" for this connection,
     * append the new string.
     */
    if ( !MTD(ch->desc->pager_head) )
    {
        char fub[strlen(txt)+strlen(ch->desc->pager_head)+2];
        int i;

        fub[0] = '\0';
        strcat( fub, ch->desc->pager_head );
        i = strlen(fub) - strlen(ch->desc->pager_point);
        strcat( fub, txt );

        free_string( ch->desc->pager_head );
        ch->desc->pager_head = str_dup( fub );
        ch->desc->pager_point = ch->desc->pager_head + i;
        return;
    }

    free_string( ch->desc->pager_head );
    ch->desc->pager_head = str_dup( txt );
    ch->desc->pager_point = ch->desc->pager_head;
    page_string( ch->desc, "" );
    ch->pagelen = fOldLen;
    return;
}



/*
 * Simplified pager.
 */
void page_string(struct connection *d, char *input)
{
  char buffer[ MAX_STRING_LENGTH * 3 ];
  char buf[ MAX_INPUT_LENGTH ];
  register char *scan, *chk;
  int lines = 0, toggle=1;

  one_argument(input, buf);

  switch( UPPER( buf[0] ) )
  {
  case '\0':
  case 'N': /* show next page of text */
     lines = 0;
    break;

  case 'R': /* refresh current page of text */
     lines = - 1 - (d->character->pagelen);
    break;

  case 'P': /* scroll back a page of text */
    lines = -(2*d->character->pagelen);
    break;

  case 'H': /* Show some help */
  case '?':
    write_to_buffer( d, "Pager help:\n\rN or Enter     next page\n\rR              redraw this page\n\r",
                          0 );
    write_to_buffer( d, "P              previous page\n\rH or ?         help\n\rAny other keys exit.\n\r\n\r<more> ",
                    0 );
/*    lines = - 1 - (d->character->pagelen); */
    break;

  default: /*otherwise, stop the text viewing */
    if ( d->pager_head )
    {
        free_string( d->pager_head );
        d->pager_head = str_dup( "" );
    }
    d->pager_point = NULL;
    return;

  }

  /* do any backing up necessary */
  if (lines < 0)
  {
    for ( scan = d->pager_point; scan > d->pager_head; scan-- )
         if ( ( *scan == '\n' ) || ( *scan == '\r' ) )
         {
             toggle = -toggle;
             if ( toggle < 0 )
                 if ( !( ++lines ) )
                     break;
         }
    d->pager_point = scan;
  }

  /* show a chunk */
  lines  = 0;
  toggle = 1;
  for ( scan = buffer; ; scan++, d->pager_point++ )
       if ( ( ( *scan = *d->pager_point ) == '\n' || *scan == '\r' )
           && ( toggle = -toggle ) < 0 )
           lines++;
       else
           if ( !*scan || ( d->character && !NPC( d->character )
                          && lines >= d->character->pagelen) )
           {

               *scan = '\0';
               write_to_buffer( d, buffer, strlen( buffer ) );

             /* See if this is the end (or near the end) of the string */
               for ( chk = d->pager_point; isspace( *chk ); chk++ );
               if ( !*chk )
               {
                   if ( d->pager_head )
                   {
                       free_string( d->pager_head );
                       d->pager_head = str_dup( "" );
                   }
                   d->pager_point = 0;
               }
               return;
           }

  return;
}



/*
 * Kludgy display check for ansi color.
 */
void ansi_color( const char *txt, PLAYER *ch )
{
    if ( !NPC(ch) && txt != NULL )
    {
     if ( !IS_SET(ch->flag2,PLR_ANSI) && !IS_SET(ch->flag2,PLR_VT100) ) return;
      else
      if ( IS_SET(ch->flag2, PLR_VT100) && !IS_SET( ch->flag2, PLR_ANSI ) )
      {
         if ( !str_cmp(txt, GREEN  )
           || !str_cmp(txt, RED    )
           || !str_cmp(txt, BLUE   )
           || !str_cmp(txt, BLACK  )
           || !str_cmp(txt, CYAN   )
           || !str_cmp(txt, GREY   )
           || !str_cmp(txt, YELLOW )
           || !str_cmp(txt, PURPLE ) ) return;
      }
     write_to_buffer( ch->desc, txt, strlen(txt) );
     return;
    }
}


/*
 * Kludgy display check for ansi color.
 */
void clrscr( PLAYER *ch )
{
    if ( NPC(ch) ) return;

    if ( !IS_SET(ch->flag2,PLR_ANSI) && !IS_SET(ch->flag2,PLR_VT100) ) return;

      if ( IS_SET(ch->flag2,PLR_CLRSCR ) )
      {
     to_actor( CLRSCR, ch );
     return;
      }
    return;
}


/*
 * The primary output interface for echoing to characters.
 */
void global( char *buf, int level, int toggler, int toggler2 )
{
    CONNECTION *d;
    
    for ( d = connection_list; d != NULL; d = d->next )
    {
      if ( CONNECTED(d)
        && get_trust( d->character ) >= level
        && IS_SET(d->character->flag, toggler)
        && IS_SET(d->character->flag, toggler2) )
      {
         to_actor( buf, d->character );
         to_actor( "\n\r", d->character );
      }
    }
}


/*
 * The primary output interface for echoing to connections.
 */
void write_global( char *buf )
{
    CONNECTION *d;
    
    for ( d = connection_list; d != NULL; d = d->next )
       write_to_buffer( d, buf, strlen(buf) );
}


void cmd_global( PLAYER *ch, char *argument )
{
    if ( argument[0] == '\0' )
    {
	to_actor( "Global what?\n\r", ch );
	return;
    }

    write_global( argument );
    return;
}    


/*
 * The primary output interface for formatted output.
 */
void act( const char *format, PLAYER *ch, const void *arg1, const void *arg2, int type )
{
    char buf[MAX_STRING_LENGTH];
    char fname[MAX_INPUT_LENGTH];
    PLAYER *to;
    PLAYER *vch = (PLAYER *) arg2;
    PROP *prop1 = (PROP  *) arg1;
    PROP *prop2 = (PROP  *) arg2;
    const char *str;
    const char *i;
    char *p;

    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' || QUIET_STC )
        return;
        
    if ( ch == NULL )
    {
       bug( "Act: null ch", 0 );
       return;
    }
        
    if ( ch->in_scene == NULL && type != TO_ACTOR && type != TO_VICT )
    {
       bug( "Act: null ch->in_scene", 0 );
       return;
    }

    to = ch->in_scene->people;
    if ( type == TO_VICT )
    {
	if ( vch == NULL )
	{
	    bug( "Act: null vch with TO_VICT.", 0 );
	    return;
	}
	to = vch->in_scene->people;
    }
    
    for ( ; to != NULL; to = to->next_in_scene )
    {
    if ( to->desc == NULL || !IS_AWAKE(to) )                 continue;
    if ( type == TO_ACTOR    && to != ch )                    continue;
    if ( type == TO_VICT    && ( to != vch || to == ch ) )   continue;
    if ( type == TO_SCENE    && to == ch )                    continue;
    if ( type == TO_NOTVICT && (to == ch || to == vch) )     continue;

    if ( type != TO_ALL
      && type != TO_ACTOR
      && type != TO_VICT
      && type != TO_SCENE
      && type != TO_NOTVICT )
    bug( "Act: incorrect type (%d) assuming TO_ALL", type );

    p  = buf;
	str	= format;
	while ( *str != '\0' )
	{
	    if ( *str != '$' )
	    {
		*p++ = *str++;
		continue;
	    }
	    ++str;

        if ( arg2 == NULL && *str >= 'A' && *str <= 'Z'
                          && *str >= '0' && *str <= '9' )
	    {
		bug( "Act: missing arg2 for code %d.", *str );
		i = " <@@@> ";
	    }
	    else
	    {
		switch ( *str )
		{
        default:  bug( "Act: bad code %d.", *str ); i = " <@@@> ";      break;
        case '$': i = "$"; break;

        case '1': i = ""; if ( IS_SET( to->flag2, PLR_ANSI ) ) i = BLUE; break;
        case '2': i = ""; if ( IS_SET( to->flag2, PLR_ANSI ) ) i = GREEN; break;
        case '3': i = ""; if ( IS_SET( to->flag2, PLR_ANSI ) ) i = CYAN; break;
        case '4': i = ""; if ( IS_SET( to->flag2, PLR_ANSI ) ) i = RED; break;
        case '5': i = ""; if ( IS_SET( to->flag2, PLR_ANSI ) ) i = PURPLE; break;
        case '6': i = ""; if ( IS_SET( to->flag2, PLR_ANSI ) ) i = YELLOW; break;
        case '7': i = ""; if ( IS_SET( to->flag2, PLR_ANSI ) ) i = GREY; break;
        case '0': i = ""; if ( IS_SET( to->flag2, PLR_ANSI ) ) i = BLACK; break;
        case 'B': i = ""; if ( IS_SET( to->flag2, PLR_ANSI )
                    || IS_SET( to->flag2, PLR_VT100 ) ) i = BOLD;     break;
        case 'I': i = ""; if ( IS_SET( to->flag2, PLR_ANSI )
                    || IS_SET( to->flag2, PLR_VT100 ) ) i = INVERSE;  break;
        case 'F': i = ""; if ( IS_SET( to->flag2, PLR_ANSI )
                    || IS_SET( to->flag2, PLR_VT100 ) ) i = FLASH;    break;
        case 'R': i = ""; if ( IS_SET( to->flag2, PLR_ANSI )
                    || IS_SET( to->flag2, PLR_VT100 ) ) i = NTEXT;    break;

        /* Thx alex for 't' idea */
        case 't': i = (arg1 != NULL) ? (char *) arg1 : "(null)"; break;
        case 'T': i = (arg2 != NULL) ? (char *) arg2 : "(null)"; break;

        case 'n': i = (ch == to)  ? "you"  : PERS(ch,  to); break;
        case 'N': i = (vch == to) ? "you"  : PERS(vch, to); break;
        case 'e': i = (ch == to)  ? "you"  : HE_SHE(ch);    break;
        case 'E': i = (vch == to) ? "you"  : HE_SHE(vch);   break;
        case 'm': i = (ch == to)  ? "you"  : HIM_HER(ch);   break;
        case 'M': i = (vch == to) ? "you"  : HIM_HER(vch);  break;
        case 's': i = (ch == to)  ? "your" : HIS_HER(ch);   break;
        case 'S': i = (vch == to) ? "your" : HIS_HER(vch);  break;
        case 'i': i = (ch == to)  ? "are"  : "is";          break;
        case 'v': i = (ch == to)  ? ""     : "s";           break;
        case 'V': i = (vch == to) ? ""     : "s";           break;
        case 'w': i = (ch == to)  ? ""     : "es";          break;
        case 'W': i = (vch == to) ? ""     : "es";          break;
        case 'x': i = (ch == to)  ? "y"    : "ies";         break;
        case 'X': i = (vch == to) ? "y"    : "ies";         break;
        case 'y': i = (ch == to)  ? "r"    : "'s";          break;
        case 'Y': i = (vch == to) ? "r"    : "'s";          break;
        case 'k': i = (ch == to)  ? "are"  : "is";          break;
        case 'K': i = (vch == to) ? "are"  : "is";          break;

		case 'p':
		    i = can_see_prop( to, prop1 )
                ? STR(prop1, short_descr)
			    : "something";
		    break;

		case 'P':
		    i = can_see_prop( to, prop2 )
                ? STR(prop2, short_descr)
			    : "something";
		    break;

		case 'd':
            if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
            {
                i = "door";
            }
            else
            {
                one_argument( (char *) arg2, fname );
                i = fname;
            }
		    break;
		}
	    }
		
	    ++str;
	    while ( ( *p = *i ) != '\0' )
		++p, ++i;
	}

        *p++ = '\n';
        *p++ = '\r';                
        *p++ = '\0'; 
    {
        char *z;
        z = format_string( str_dup( ansi_uppercase( buf ) ) );
        to_actor( z, to ); 
        free_string( z );
    }

/* old, see above
        write_to_buffer( to->desc, ansi_uppercase( buf ), p - buf );
 */

    }

    return;
}




/*
 * Macintosh support functions.
 */
#if defined(macintosh)
int gettimeofday( struct timeval *tp, void *tzp )
{
    tp->tv_sec  = time( NULL );
    tp->tv_usec = 0;
}
#endif




/*
 * email <player> <filename>
 *
 * or: email <player>
 *  <- string editor
 */
void cmd_email( PLAYER *ch, char *argument ) {
}




void wtf_logf (char * fmt, ...)
{
        char buf [2*MSL];
        va_list args;
        va_start (args, fmt);
        vsprintf (buf, fmt, args);
        va_end (args);

        log_string (buf);
}



void var_to_actor (char * fmt, PLAYER *ch, ...)
{
        char buf [2*MSL];
        va_list args;
        if ( NPC(ch) ) return;
        va_start (args, ch);
        vsprintf (buf, fmt, args);
        va_end (args);

        to_actor (buf, ch);
}
