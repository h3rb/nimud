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
 * To use any part of NiMUD, you must comply with the Merc, Diku and NiMUD    *
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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <zlib.h>

#include "nimud.h"
#include "client.h"
#include "defaults.h"
#include "net.h"
#include "skills.h"

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
#if     defined(MSDOS)
const char    echo_off_str    [] = { '\0' };
const char    echo_on_str     [] = { '\0' };
#endif

#if     defined(unix) && !defined(ECHO_STR)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/telnet.h>

const char    echo_off_str    [] = { IAC, WILL, TELOPT_ECHO, '\0' };
const char    echo_on_str     [] = { IAC, WONT, TELOPT_ECHO, '\0' };
#endif

#include "network.h"

/*
 * Local defines.
 */
#define DC(desc)   ( desc->connected )
#define MT(str, d) ( str == NULL || str[0] == '\0' ? d : str )
#define SMT(str)   ( str == NULL || str[0] == '\0' )

/*
 * Other local functions (OS-independent).
 */
void    newbie              args( ( CONNECTION_DATA *d, char *argument ) );
void    newbie_check        args( ( CONNECTION_DATA *d ) );
bool    check_parse_name   args( ( char *name ) );
bool    check_playing      args( ( CONNECTION_DATA *d, char *name ) );
bool    check_reconnect    args( ( CONNECTION_DATA *d, char *name,
                                   bool fConn ) );
void    stop_idling        args( ( PLAYER_DATA *ch ) );
bool    apply_ok           args( ( PLAYER_DATA *ch ) );
void    print_doc_menu     args( ( PLAYER_DATA *ch ) );
void    print_login_menu   args( ( PLAYER_DATA *ch ) );
void    print_stat_menu    args( ( PLAYER_DATA *ch ) );

/*
 * Guest character support.
 */
int       guestnumber = 0;
PLAYER_DATA *generate_guest  args( ( void ) );




/*
 * Setup new player racial information, as well as default info.
 */
void setup_race( PLAYER_DATA *ch )
{
    int race;
/*    char buf[MAX_STRING_LENGTH]; */

    ch->position         = POS_STANDING;

    ch->act              = 0;
    SET_BIT(ch->act2, PLR_BLANK | PLR_COMBINE | PLR_PROMPT);
    ch->pagelen          = 30;
    ch->hit              = MAXHIT(ch);
    ch->move             = MAXMOVE(ch);

    PC(ch,birth_day)   = weather_info.day;
    PC(ch,birth_month) = weather_info.month;
    PC(ch,stat_points) = 5;

    /*
     * Starting skills and groups.
     */
    update_skill(ch, skill_vnum(skill_lookup("offense")), 25);
    update_skill(ch, skill_vnum(skill_lookup("defense")), 50);
    update_skill(ch, skill_vnum(skill_lookup("wp")),      25);
    update_skill(ch, skill_vnum(skill_lookup("survival")),30);
    update_skill(ch, skill_vnum(skill_lookup("swimming")),10);
    update_skill(ch, skill_vnum(skill_lookup("climb")),   10);
    update_skill(ch, skill_vnum(skill_lookup("riding")),  10);
    update_skill(ch, skill_vnum(skill_lookup("dodge")),   25);
    update_skill(ch, skill_vnum(skill_lookup("parry")),   15);
    update_skill(ch, skill_vnum(skill_lookup("language")),100);
    update_skill(ch, skill_vnum(skill_lookup("stark")),   100);
    update_skill(ch, skill_vnum(skill_lookup("evasion")), 35);
    update_skill(ch, skill_vnum(skill_lookup("hafted")),  10);

    race = race_lookup( ch->race );

    PC(ch,birth_year)  = weather_info.year - RACE(race,start_age);

    ch->bonuses      = RACE(race,bonus_bits);

    /*
     * Race information.
     */
    ch->perm_str      = number_fuzzy( RACE(race,start_str) );
    ch->perm_int      = number_fuzzy( RACE(race,start_int) );
    ch->perm_wis      = number_fuzzy( RACE(race,start_wis) );
    ch->perm_dex      = number_fuzzy( RACE(race,start_dex) );
    ch->perm_con      = number_fuzzy( RACE(race,start_con) );
    ch->size          = RACE(race,size);

    PC(ch,home)       = RACE(race,start_scene);

  /*
   * Racial skills for language.
   */
    {
      int x;

      update_skill(ch,RACE(race,primary),100);

        for ( x = 0;  x < MAX_LANGUAGE;  x++ )
           if ( lang_table[x].pgsn == RACE(race,primary) )
            ch->speaking = x;

      if ( RACE(race,secondary) ) 
      update_skill(ch,RACE(race,secondary),60);
    }

    return;
}

/*
 * Assign default new-character information, props, etc.
 * Due to nature of props, do not call twice.
 */
void new_char( PLAYER_DATA *ch )
{
    PROP_DATA *prop;

    if ( ch == NULL )
    return;    
       
    /*
     * Newbie equipment.
     */
    if ( ch->carrying == NULL )
    {
    prop = create_prop( get_prop_index( PROP_VNUM_DEFAULT_LIGHT ), 1 );
    prop_to_actor( prop, ch );
    prop->wear_loc = WEAR_HOLD_2;
    prop = create_prop( get_prop_index( PROP_VNUM_DEFAULT_WEAPON ), 1 );
    prop_to_actor( prop, ch );
    prop->wear_loc = WEAR_BELT_1;
    prop = create_prop( get_prop_index( PROP_VNUM_DEFAULT_VEST ), 1 );
    prop_to_actor( prop, ch );
    prop->size = ch->size;
    equip_char( ch, prop, WEAR_BODY );
    prop = create_prop( get_prop_index( PROP_VNUM_DEFAULT_BELT ), 1 );
    prop_to_actor( prop, ch );
    prop->size = ch->size;
    equip_char( ch, prop, WEAR_WAIST );
    prop = create_prop( get_prop_index( PROP_VNUM_DEFAULT_PACK ), 1 );
    prop_to_actor( prop, ch );
    prop->size = ch->size;
    equip_char( ch, prop, WEAR_SHOULDER_L );

    prop_to_prop( create_prop( get_prop_index( PROP_VNUM_DEFAULT_TINDERBOX ), 1),
                prop );

     if ( ch->race <= 3 ) 
    prop_to_prop( create_prop( get_prop_index( PROP_VNUM_DEFAULT_LETTER ), 1),
                prop );

    prop_to_prop( create_prop( get_prop_index( PROP_VNUM_DEFAULT_FOOD ), 1),
                prop );
    prop_to_prop( create_prop( get_prop_index( PROP_VNUM_DEFAULT_DRINK ), 1),
                prop );

    create_amount( number_range( 5, 10 ) * 10, ch, NULL, NULL );
    }

    if ( ch->userdata == NULL ) ch->userdata = new_user_data( );

    if ( ch->userdata->level != LEVEL_BUILDER )
    ch->userdata->level         = LEVEL_MORTAL;
    SET_BIT(ch->act2, PLR_COMBINE);
    SET_BIT(ch->act2, PLR_BLANK);
    SET_BIT(ch->act2, PLR_TIPS);

    ch->userdata->app_time = 230; /* I think thats 24 hours if its in 5's anyways close enough */

    return;
}



void print_stat_menu( PLAYER_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];

    if ( ch->exp_level > 0 && PC(ch,stat_points) == 0 ) {
        send_to_actor( "You have spent all of your hero points.\n\rHit return: ", ch );
        DC(ch->desc) = NET_HERO_RECON;
        return;
    }
   
    if ( ch->exp_level == 0 ) cmd_help( ch, "NEWBIE_STAT_MENU_HEADER" ); 
    else 
    send_to_actor( "You have hero points left unspent!\n\r\n\r", ch );
   
    snprintf( buf, MAX_STRING_LENGTH, "[S]trength:     %s\n\r", name_stat_range(get_curr_str(ch)) );
    send_to_actor( buf, ch );
    snprintf( buf, MAX_STRING_LENGTH, "[I]ntelligence: %s\n\r", name_stat_range(get_curr_int(ch)) );
    send_to_actor( buf, ch );
    snprintf( buf, MAX_STRING_LENGTH, "[W]isdom:       %s\n\r", name_stat_range(get_curr_wis(ch)) );
    send_to_actor( buf, ch );
    snprintf( buf, MAX_STRING_LENGTH, "[D]exterity:    %s\n\r", name_stat_range(get_curr_dex(ch)) );
    send_to_actor( buf, ch );
    snprintf( buf, MAX_STRING_LENGTH, "[C]onstitution: %s\n\r", name_stat_range(get_curr_con(ch)) );
    send_to_actor( buf, ch );
    if ( ch->exp_level == 0 ) {
    snprintf( buf, MAX_STRING_LENGTH, "[A]ge:          %d years\n\r", GET_AGE(ch) );
    send_to_actor( buf, ch );
    snprintf( buf, MAX_STRING_LENGTH, "[H]eight:       %d units\n\r", ch->size );
    send_to_actor( buf, ch );
    }

    snprintf( buf, MAX_STRING_LENGTH, "You have %d point%s to distribute.\n\r", 
          PC(ch,stat_points), PC(ch,stat_points) == 1 ? "" : "s" );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Hit return to continue, or, %s, type a letter: ",
             capitalize(ch->name) );
    send_to_actor( buf, ch );

    return;
}



void stat_menu( PLAYER_DATA *ch, char *argument )
{
    CONNECTION_DATA *d;
    char buf[MAX_STRING_LENGTH];

    d= ch->desc;
    if ( d==NULL )
    {
        bug( "Stat_menu: NULL connection.", 0 );
        return;
    }

    switch( LOWER(argument[0]) )
    {
    case 's':
        cmd_help( ch, "NEWBIE_STRENGTH" );
        write_to_buffer( d, "Do you wish to increase or decrease this stat? ", 0 );
        DC(d) = NET_STAT_STR;
      break;
    case 'i':
        cmd_help( ch, "NEWBIE_INTELLIGENCE" );
        write_to_buffer( d, "Do you wish to increase or decrease this stat? ", 0 );
        DC(d) = NET_STAT_INT;
      break;
    case 'w':
        cmd_help( ch, "NEWBIE_WISDOM" );
        write_to_buffer( d, "Do you wish to increase or decrease this stat? ", 0 );
        DC(d) = NET_STAT_WIS;
      break;
    case 'd':
        cmd_help( ch, "NEWBIE_DEXTERITY" );
        write_to_buffer( d, "Do you wish to increase or decrease this stat? ", 0 );
        DC(d) = NET_STAT_DEX;
      break;
    case 'c':
        cmd_help( ch, "NEWBIE_CONSTITUTION" );
        write_to_buffer( d, "Do you wish to increase or decrease this stat? ", 0 );
        DC(d) = NET_STAT_CON;
      break;
    case 'a':
    if ( ch->exp_level == 0 )
    {
        int race;

        cmd_help( ch, "NEWBIE_AGE" );
/***/   DC(d) = NET_STAT_AGE;

        race = race_lookup( ch->race );
        snprintf( buf, MAX_STRING_LENGTH, "%s live to be a maximum of %d years.",     
                 RACE(race,race_name), RACE(race,base_age) );
        send_to_actor( buf, ch );

        snprintf( buf, MAX_STRING_LENGTH, "%s is currently %d years old.\n\r",
                 NAME(ch), GET_AGE(ch) );
        send_to_actor( buf, ch );

        write_to_buffer( d, "What age would you like to be? ", 0 );
    }
      break;
    case 'h':
    if ( ch->exp_level == 0 )
    {
        int race;

        cmd_help( ch, "NEWBIE_HEIGHT" );

        race = race_lookup( ch->race );
/***/   DC(d) = NET_STAT_SIZE;

        snprintf( buf, MAX_STRING_LENGTH, "Members of your race are from %d to %d units tall.\n\r",
                RACE(race,size)-4,
                RACE(race,size)+4 );
        send_to_actor( buf, ch );

        snprintf( buf, MAX_STRING_LENGTH, "You are currently %d units in size.\n\r[Units are six inches or approximately 18cm]\n\r",
                 ch->size );
        send_to_actor( buf, ch );

        write_to_buffer( d, "How many units tall? ", 0 );
    }
      break;
   default:
        if ( PC(ch,stat_points) != 0 && ch->exp_level <1 )
        {
            send_to_actor( "NOTE: You still have hero points to distribute.\n\r", ch );
            print_stat_menu( ch );
            break;
        }
        if ( ch->exp_level < 1 ) { 

        free_string( PC(ch, denial) );
        PC(ch, denial) = NULL;

    free_string( PC(ch,denial) );
    PC(ch,denial) = NULL;
    ch->in_scene = get_scene_index( SCENE_VNUM_START );

    new_char( ch );    
    save_actor_prop( ch );

    snprintf( buf, MAX_STRING_LENGTH, "%s is born in the year %d.", ch->name,
               PC(ch,birth_year) );  
    add_history( ch, buf );

    ch->hit = MAXHIT(ch);
    snprintf( buf, MAX_STRING_LENGTH, "Notify> New player %s@%s.", ch->name, d->host );
    NOTIFY( buf, LEVEL_IMMORTAL, WIZ_NOTIFY_LOGIN );
    sprintf( log_buf, "%s@%s new player.", ch->name, d->host );
    log_string( log_buf );

    write_to_buffer( d, "\n\r", 2 );
    show_player_statistics( ch );
    cmd_help( ch, "WELCOME" );
    DC(d) = NET_READ_MOTD;
          } else {  DC(d) = NET_HERO_RENET_ABORT;  
           send_to_actor( "Congratulations!  You may spend any additional hero points\n\rnext time you log in. Hit return to continue: ", ch ); 
           }

       break;

    }

    return;
}


void stat_menu_choice( PLAYER_DATA *ch, char *argument )
{
    CONNECTION_DATA *d;

    d = ch->desc;

    if ( d == NULL )
    {
        bug( "Stat_menu_choice:  NULL connection.", 0 );
        return;
    }

    while ( isspace( *argument) ) argument++;     

    switch( DC(d) )
    {   
   case NET_STAT_STR:
   case NET_STAT_INT:
   case NET_STAT_WIS:
   case NET_STAT_DEX:
   case NET_STAT_CON:
      {
          switch ( LOWER(argument[0]) )
          {
             case 'i':
                {
                    if ( PC(ch,stat_points) == 0 )
                    {
                        send_to_actor( "You don't have a stat point to distribute.\n\r", ch );

                        if ( ch->exp_level == 0 )
                        write_to_buffer( d, "Do you want to decrease (-) this stat? ", 0 );
                        else 
                        write_to_buffer( d, "Press return: ", 0 );
                        
                        return;
                    }

                    if (((DC(d) == NET_STAT_STR && ch->perm_str >= 25)
                      || (DC(d) == NET_STAT_INT && ch->perm_int >= 25)
                      || (DC(d) == NET_STAT_WIS && ch->perm_wis >= 25)
                      || (DC(d) == NET_STAT_DEX && ch->perm_dex >= 25)
                      || (DC(d) == NET_STAT_CON && ch->perm_con >= 25)) && (ch->exp_level == 0) )
                    {
                        send_to_actor( "It is impossible to add any more to that stat.\n\r", ch );
                        DC(d) = NET_STAT_MENU;
                        return;
                    }

                    PC(ch,stat_points)--;
                    if ( DC(d) == NET_STAT_STR ) ch->perm_str++;
                    if ( DC(d) == NET_STAT_INT ) ch->perm_int++;
                    if ( DC(d) == NET_STAT_WIS ) ch->perm_wis++;
                    if ( DC(d) == NET_STAT_DEX ) ch->perm_dex++;
                    if ( DC(d) == NET_STAT_CON ) ch->perm_con++;
                    send_to_actor( "Added.\n\r", ch );
                    print_stat_menu( ch );
                    DC(d) = NET_STAT_MENU;
                }
               break;
             case 'd':
                if ( ch->exp_level < 1 )
                {
                    if ( (DC(d) == NET_STAT_STR && ch->perm_str <= 3)
                      || (DC(d) == NET_STAT_INT && ch->perm_int <= 3)
                      || (DC(d) == NET_STAT_WIS && ch->perm_wis <= 3)
                      || (DC(d) == NET_STAT_DEX && ch->perm_dex <= 3)
                      || (DC(d) == NET_STAT_CON && ch->perm_con <= 3) )
                    {
                        send_to_actor( "It is impossible to subtract any more from that stat.\n\r", ch );
                        DC(d) = NET_STAT_MENU;
                        return;
                    }

                    PC(ch,stat_points)++;
                    if ( DC(d) == NET_STAT_STR ) ch->perm_str--;
                    if ( DC(d) == NET_STAT_INT ) ch->perm_int--;
                    if ( DC(d) == NET_STAT_WIS ) ch->perm_wis--;
                    if ( DC(d) == NET_STAT_DEX ) ch->perm_dex--;
                    if ( DC(d) == NET_STAT_CON ) ch->perm_con--;
                    print_stat_menu( ch );
                    DC(d) = NET_STAT_MENU;
                } else {
                   send_to_actor( "You cannot decrease a stat, only increase (+).\n\r", ch );
                   print_stat_menu( ch );
                   DC(d) = NET_STAT_MENU;
                }
               break;
              default:  print_stat_menu( ch ); DC(d) = NET_STAT_MENU;  break;
          }
      }
     break;
   case NET_STAT_AGE:
      {
          int value = atoi(argument);
          int race = race_lookup(ch->race);

          PC(ch,birth_year) = weather_info.year - 
     URANGE(15,value,RACE(race,base_age));
         
          if ( PC(ch,birth_year) != weather_info.year - value )
              send_to_actor( "Invalid age.\n\r", ch );

          print_stat_menu( ch );
          DC(d) = NET_STAT_MENU;
      }
     break;
   case NET_STAT_SIZE:
      {
          int value = atoi(argument);
          int race = race_lookup(ch->race);

          ch->size = URANGE(RACE(race,size)-4,value,RACE(race,size+4));
          if ( ch->size !=  value )
          send_to_actor( "Invalid height.\n\r", ch );

          print_stat_menu( ch );
          DC(d) = NET_STAT_MENU;
      }
     break;
        default: print_stat_menu( ch ); DC(d)=NET_STAT_MENU; break;
    }

    return;
}



void actor_gen( PLAYER_DATA *ch, char *argument )
{
    CONNECTION_DATA *d;
    char arg[MAX_STRING_LENGTH];
    char *pwdnew;
    char *p;

    if ( ch == NULL )
	{
		bug( "Char_gen: NULL character at function call.", 0 );
		return;
	}

    d = ch->desc;
    one_argument( argument, arg );

    switch( DC(d) )
    {
    case NET_CONFIRM_NEW_NAME:
        switch ( *argument )
        {
        case 'y': case 'Y':
            write_to_buffer( d, "Enter a new password: ", 0 );
            DC(d) = NET_GET_NEW_PASSWORD;
            break;

        case 'n': case 'N':
            write_to_buffer( d, "Name? ", 0 );
            free_char( d->character );
            d->character = NULL;
            DC(d) = NET_GET_NAME;
            break;

        default:
            write_to_buffer( d, "Please type Yes or No.\n\rConfirm? ", 0 );
            break;
        }
       break;

    case NET_GET_NEW_PASSWORD:
#if defined(unix)
        write_to_buffer( d, "\n\r", 2 );
#endif

        if ( strlen(argument) < 5 )
        {
            write_to_buffer( d,
            "Password must be at least five characters long.\n\rPassword: ", 0 );
           write_to_buffer( d, echo_off_str, 0 );
            return;
        }

#if !defined(NOCRYPT)
        pwdnew = crypt( argument, ch->name );
#else
        pwdnew = argument;
#endif
        for ( p = pwdnew; *p != '\0'; p++ )
        {
            if ( *p == '~' )
            {
            write_to_buffer( d,
                   "New password not acceptable, please try again.\n\rPassword: ", 0 );
           write_to_buffer( d, echo_off_str, 0 );
            return;
            }
        }

        free_string( PC(ch,pwd) );
        PC(ch,pwd) = str_dup( pwdnew );
        write_to_buffer( d, "Please retype password for verification: ", 0 );
       write_to_buffer( d, echo_off_str, 0 );
        DC(d) = NET_CONFIRM_NEW_PASSWORD;
       break;

    case NET_CONFIRM_NEW_PASSWORD:
#if defined(unix)
        write_to_buffer( d, "\n\r", 2 );
#endif

        if ( strcmp( crypt( argument, PC(ch,pwd) ), PC(ch,pwd) ) )
        {
            write_to_buffer( d, "Passwords don't match.\n\rRetype password: ",
            0 );
           write_to_buffer( d, echo_off_str, 0 );
            DC(d) = NET_GET_NEW_PASSWORD;
            return;
        }

        cmd_help( ch, "NEWBIE_INTRO" );
	write_to_buffer( d, "Press enter to continue: ", 0 );
        DC(d) = NET_SHOW_INTRO;
	   break;

	case NET_SHOW_INTRO:
        cmd_help( ch, "NEWBIE_NAME" );
        write_to_buffer( d, "\n\r    ", 0 ); // <- better telnet fix, tx jain
        write_to_buffer( d, d->character->name, 0 );
        write_to_buffer( d, ", type your name again, or what is your name? ", 0 );
        DC(d) = NET_CHAR_GEN_NAME;
       break;

    case NET_CHAR_GEN_NAME:
        {
            bool fOld;
            char buf[MAX_STRING_LENGTH];

            d = ch->desc;
            fOld = load_actor_prop( d, fix_string( arg ) );
            free_char( d->character );
            ch->desc = d;
            d->character = ch;

            if ( !check_parse_name( fix_string( arg ) ) || fOld )
            {
                send_to_actor( "Invalid name, try again.\n\rName: ", ch );
                DC(d) = NET_CHAR_GEN_NAME;
                return;
            }
            
            buf[0] = '\0';
#if defined(MSDOS)
            snprintf( buf, MAX_STRING_LENGTH, "del %s%s", PLAYER_DIR, capitalize( ch->name ) );
#else
#if defined(unix)
            snprintf( buf, MAX_STRING_LENGTH, "rm -f %s%s &", PLAYER_DIR, capitalize( ch->name ) );
#endif
#endif
            if ( str_cmp( ch->name, "guest" ) ) system( buf );

            free_string( ch->name );
            ch->name = str_dup( capitalize( fix_string( arg ) ) );
        }

        cmd_help( ch, "NEWBIE_EMAIL" );
        write_to_buffer( d, "Please enter your email address (me@where.com): ", 0 
);
        DC(d) = NET_CHAR_GEN_EMAIL;
       break;

   case NET_CHAR_GEN_EMAIL:

        free_string( PC(ch,email) );
        PC(ch,email) = str_dup( fix_string( argument ) );
        send_to_actor( "Do you want to help write the world? ", ch );
        DC(d) = NET_CHAR_GEN_BUILDER;
        break;

    case NET_CHAR_GEN_BUILDER:
        if ( UPPER(*argument) == 'Y' )
        {
            PC(ch,constellation) = str_dup( "%s the Builder" );
#if !defined(APPLY_BUILDER) 
            PC(ch,level) = LEVEL_BUILDER;
            cmd_help( ch, "BUILDING" );
            send_to_actor( "\n\r", ch );
            send_to_actor( "You have been given limited building access.\n\r", ch );
            send_to-char( "Do you feel confident with programming languages? ", ch );
            DC(d) = NET_CHAR_GEN_WRITER;
#else
//            send_to_actor( "Ok.\n\r", ch );
            send_to_actor( "Do you feel confident with programming languages? ", ch );
            DC(d) = NET_CHAR_GEN_WRITER;
#endif
        }
        else {

        send_to_actor( "Please visit our website for documentation:\n\rhttp://nimud.divineright.org\n\r\n\r", ch );
        send_to_actor( "Do you want to use color (y/n)? ", ch );
        DC(d) = NET_CHAR_GEN_COLOR;
        }

        break;

    case NET_CHAR_GEN_WRITER:
            
        if ( UPPER(*argument) == 'Y' ) {
           PC(ch,constellation) = str_dup( "%s the Writer" );
#if !defined(APPLY_BUILDER)
           PC(ch,level) = LEVEL_WRITER;
           cmd_help( ch, "SCRIPTING" );
           send_to_actor( "\n\r", ch );
           send_to_actor( "Do you want to use color (y/n)? ", ch );
           DC(d) = NET_CHAR_GEN_COLOR;
        }
#else
        }
           DC(d) = NET_CHAR_GEN_APPLY_BUILDER;
        
send_to_actor("\n\r", ch );
send_to_actor("--------------------------------[Application]--------------------------------\n\r", ch );
send_to_actor("Enter a few lines describing your experience mudding and your intentions as a\n\r", ch );
send_to_actor("contributer to this software. Your message will be read as a resume of sorts.\n\r\n\r", ch );
actor_to_scene( ch, get_scene_index( SCENE_VNUM_APPLY ) );
cmd_note( ch, "enter" );
actor_from_scene( ch ); // must happen or else if the user drops link, bad juju
#endif
       

        break;

#if defined(APPLY_BUILDER) 
     case NET_CHAR_GEN_APPLY_BUILDER:
           if ( d->pString == NULL  ) {
           actor_to_scene( ch, get_scene_index( SCENE_VNUM_APPLY ) );
           cmd_note( ch, "subject New Builder or Writer" );
           cmd_note( ch, "to immortal" );
           cmd_note( ch, "post" );
           actor_from_scene( ch );
           send_to_actor("Do you want to use color (y/n)? ", ch );
           DC(d) = NET_CHAR_GEN_COLOR;
           }
        break;
#endif

    case NET_CHAR_GEN_COLOR:

        switch ( *argument )
        {
        case 'y': case 'Y':
            cmd_config( ch, "ansi" );
            cmd_config( ch, "timecolor" );

            if ( PC(ch,level) != LEVEL_BUILDER )
            cmd_prompt( ch, "colors" );
            else cmd_prompt( ch, "immortal" );

            send_to_actor( "What gender (m/f/n)? ", ch );
            DC(d) = NET_CHAR_GEN_SEX;
            break;
        case 'n': case 'N':
            send_to_actor( "What gender (m/f/n)? ", ch );
            DC(d) = NET_CHAR_GEN_SEX;
            break;

        default:
            write_to_buffer( d, "Please type Yes or No.\n\rConfirm? ", 0 );
            break;
        }
        break;

   case NET_CHAR_GEN_SEX:
        {
            switch ( UPPER(arg[0]) )
            {
                case 'F':
                   ch->sex = SEX_FEMALE;
                    cmd_help( ch, "RACES" );
              send_to_actor( "What race are you? ", ch );
                    DC(d) = NET_CHAR_GEN_RACE;
                  break;
                case 'M':
                   ch->sex = SEX_MALE;
                    cmd_help( ch, "RACES" );
              send_to_actor( "What race are you? ", ch );
                    DC(d) = NET_CHAR_GEN_RACE;
                  break;
                case 'N':
                   ch->sex = SEX_NEUTRAL;
                    cmd_help( ch, "RACES" );
              send_to_actor( "What race are you? ", ch );
                    DC(d) = NET_CHAR_GEN_RACE;
                  break;
                 default:
                   send_to_actor( "Invalid selection.\n\rWhich sex (m/f/n)? ", ch );
                  break;
            }
        }
       break;

   case NET_CHAR_GEN_RACE:
        {
              int race;

              for ( race = 0;  race < MAX_RACE;  race++ ) {
                  if ( !str_prefix( argument, RACE(race,race_name) ) ) 
                  break;
              }  

              ch->race = race_lookup( race );
              if ( race < MAX_RACE ) {
                   cmd_help( ch, RACE(race,race_name) );
                   send_to_actor( "Is this ok? ", ch );
              }
              else {
                  if ( *argument == '\0' ) { 
                     cmd_help( ch, "RACES" ); 
                     send_to_actor( "Which race? ", ch ); 
                     DC(d) = NET_CHAR_GEN_RACE;
                     return; 
                   }
                   send_to_actor( "I'm sorry, I don't know of that race.\n\rWhich? ", ch );
                   DC(d) = NET_CHAR_GEN_RACE;
                   return;
              }
              DC(d) = NET_CHAR_GEN_RACE_CONFIRM;
        }
       break; 
   case NET_CHAR_GEN_RACE_CONFIRM:
        {
              switch ( *argument ) {
                case 'n': case 'N':
                   cmd_help( ch, "RACES" );
                   send_to_actor( "Which race? ", ch );
                   DC(d) = NET_CHAR_GEN_RACE;
                  break;
                case 'y': case 'Y':
                    setup_race( ch );
                    print_stat_menu( ch );
                    DC(d) = NET_STAT_MENU;
                  break;
               default:  send_to_actor( "Please type Yes or No. Is this race ok? ", ch ); break;                              
              }
        }
       break;
   case NET_STAT_STR:
   case NET_STAT_INT:
   case NET_STAT_WIS:
   case NET_STAT_DEX:
   case NET_STAT_CON:
   case NET_STAT_AGE:
   case NET_STAT_SIZE:
     stat_menu_choice( ch, argument );
       break;

   case NET_STAT_MENU:
     stat_menu( ch, argument );
       break;

	}

	return;
}




/*
 * Guest character support.
 *
 * Create a brand new character, with the name "guest"
 * note: guest characters are ignored in fwrite_char() in save.c
 */
PLAYER_DATA *generate_guest( void )
{
    PLAYER_DATA *nch;
    char buf[MAX_STRING_LENGTH];

    if (guestnumber < 0) guestnumber = 0;
    guestnumber++;

    nch = new_player_data( );
    nch->name        = str_dup( "Guest"          );

    snprintf( buf, MAX_STRING_LENGTH, "Guest #%d", guestnumber );
    nch->short_descr = str_dup( buf );

    nch->long_descr  = str_dup( "(OOC) A guest is observing.\n\r" );
    nch->description = str_dup( "OUT OF CHARACTER GUEST\n\r" );

    snprintf( buf, MAX_STRING_LENGTH, "guest %d", guestnumber );
    nch->keywords    = str_dup( buf              );

    /*
     * Setup_race creates the userdata for us.
     */
    setup_race( nch );
    nch->userdata->level = LEVEL_MORTAL;

    cmd_prompt(nch, "Guest> " );
    nch->act = ACT_NOSCAN | ACT_WIMPY;
    nch->in_scene = get_scene_index( SCENE_VNUM_GUEST );

    snprintf( buf, MAX_STRING_LENGTH, "GUEST CHARACTER CREATED: #%d", guestnumber );
    log_string( buf );

    return nch;
}




/*
 * Deal with sockets that haven't logged in yet.
 */
void newbie( CONNECTION_DATA *d, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    PLAYER_DATA *ch;
    bool fOld;
    extern bool wizlock;

    while ( *argument == ' ' )
	argument++;

    /*
     * This may not be neccessary.
     */
    if ( d->character != NULL )
        newbie_check( d );

/* Nor this */
//        write_to_buffer( d, echo_on_str, 0 );

    /*
     * Make sure connections are paired with characters.
     */
    if ( d->character == NULL )
    {
    ch = new_player_data( );

    d->character			= ch;
    ch->desc				= d;
    ch->name                = str_dup( "Guest" );
    ch->act                 = PLR_BLANK | PLR_COMBINE | PLR_PROMPT;
    }

    ch = d->character;

    /*
     * Jump past IAC/WILLWONT/DO crap that we ignore
     */

    while ( DC(d) > NET_PLAYING && !isalnum(*argument) && *argument != '\n' && *argument != '\r' && *argument != '\0'  ) { argument++; }
    switch ( DC(d) )
    {

    default:
    bug( "Newbie: bad DC(d) %d.", DC(d) );
	close_socket( d );
	return;

   case NET_SHOW_TITLE:
    case NET_GET_NAME: 

        if ( !str_cmp( argument, "CLIENT" ) ) {
           d->client = TRUE;
           return;
        }

        if ( !str_cmp( argument, "MSSP-REQUEST" ) ) {
           send_cleartext_mssp(d);
           return;
        }

#if defined(TRANSLATE)
        l = find_language( argument );
        if ( l != -1 ) {
             d->lingua = l;
             write_to_buffer( d, "Language set to ", 0);
             write_to_buffer( d, capitalize(lingua_table[l].name), 0 );
             write_to_buffer( d, ".\n\r", 0 );
             return;
        } 

        if ( argument[0] == '?' ) {
            write_to_buffer( d, "Supported languages:\n\r", 0 );
            for ( l = 0; l < MAX_LINGUA; l++ ) {
                write_to_buffer( d, lingua_table[l].name, 0 );
                write_to_buffer( d, ", ", 0 );
                if ( l % 6 == 0 ) write_to_buffer( d, "\n\r", 0 );
            }
            write_to_buffer( d, "which language? ", 0 ); return;
        }
#endif
	if ( argument[0] == '\0' )
	{
            write_to_buffer( d, "Name? ", 0 ); 
	    return;
	}

	argument[0] = UPPER(argument[0]);
	if ( !check_parse_name( argument ) )
	{
        if ( !sendcli( d, "BADNAME" ) )
        write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
	    return;
        }

    /*
     * Guest character support.
     */
    if ( !str_cmp(argument,"guest") || !str_cmp(argument,"onlooker") )
    {
        d->character        = generate_guest( );
        d->character->desc  = d;

        sendcli( d, "MOTD" );
        show_player_statistics( ch );
        cmd_help(d->character, "GUEST");
        write_to_buffer( d, "HIT RETURN TO OBSERVE: ", 0 );
        DC(d) = NET_READ_MOTD;
        return;
    }

    free_char( d->character );
    d->character = NULL;

    if ( check_reconnect( d, argument, TRUE ) )
    {
        fOld = TRUE;
    }
    else
    {
        free_char( d->character );
        fOld = load_actor_prop( d, argument );
        ch   = d->character;

        if ( IS_SET(ch->act, PLR_DENY) )
        {
            sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
            log_string( log_buf );

            sprintf( buf2, "Notify>  Denied %s from %s", argument, d->host );
            NOTIFY( buf2, LEVEL_IMMORTAL, WIZ_NOTIFY_LOGIN );

            write_to_buffer( d, "You are denied access.\n\r", 0 );
            close_socket( d );
            return;
        }

        if ( wizlock && (!IS_HERO(ch) || !IS_SET(ch->act, PLR_WIZBIT)) )
	    {
        write_to_buffer( d, "The game is currently closed to mortals.\n\r", 0 );
        DC(d) = NET_LOGIN_MENU;
		close_socket( d );
		return;
	    }
    }

	if ( fOld )
	{
	    /* Old player */
	    write_to_buffer( d, " Password: ", 0 );
///	    write_to_buffer( d, echo_off_str, 0 );
            DC(d) = NET_GET_OLD_PASSWORD;
	    return;
	}
	else
	{
	    /* New player */
        if ( !sendcli( d, "CREATE" ) ) {
        snprintf( buf, MAX_STRING_LENGTH, "\n\rThat character does not exist." );
	    write_to_buffer( d, buf, 0 );
          }

        DC(d) = NET_CONFIRM_NEW_NAME;
        write_to_buffer( d, " Create as new? ", 0 );

	    return;
	}
	break;

    case NET_GET_OLD_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

    if ( strcmp( crypt( argument, PC(ch,pwd) ), PC(ch,pwd) ) )
	{
            if ( !sendcli( d, "NOACCESS" ) )
            write_to_buffer( d, "Wrong password.\n\rClosing connection.\n\r", 0 );
	    close_socket( d );
	    return;
	}

///	write_to_buffer( d, echo_on_str, 0 );

	if ( check_reconnect( d, ch->name, TRUE ) )
	    return;
 
        if ( check_playing( d, ch->name ) )
        {
        check_reconnect( d, ch->name, TRUE );
        return;
        }

     case NET_HERO_RECON:            

            if ( PC(ch,stat_points) > 0 ) {
               print_stat_menu( ch );
               DC(d) = NET_STAT_MENU;
               break;
            } 
     case NET_HERO_RENET_ABORT:
               sendcli( d, "MOTD" );
               show_player_statistics( ch );
               if ( IS_IMMORTAL(ch) ) cmd_help( ch, "IMOTD" );
               cmd_help ( ch, "MOTD" );
               DC(d) = NET_READ_MOTD;
               PC(ch,logins)++;            
	break;


    case NET_CONFIRM_NEW_NAME:
         switch( *argument ) {

        case 'y': case 'Y':
             DC(d) = NET_GET_NEW_PASSWORD;
             write_to_buffer( d, "Enter a password: ", 0 );
///           write_to_buffer( d, echo_off_str, 0 );
            break;

	case 'n': case 'N':     
        free_char( d->character );
        d->character = NULL;
        DC(d) = NET_GET_NAME;
	    break;

	default:
        if ( !sendcli( d, "CREATE" ) )
        write_to_buffer( d, "Please type Yes or No.\n\rConfirm? ", 0 );
	    break;
         }
	break;

    case NET_GET_NEW_PASSWORD:
    case NET_CONFIRM_NEW_PASSWORD:
	case NET_SHOW_INTRO:
    case NET_CHAR_GEN_NAME:
    case NET_CHAR_GEN_EMAIL:
    case NET_CHAR_GEN_BUILDER: 
    case NET_CHAR_GEN_WRITER:
    case NET_CHAR_GEN_APPLY_BUILDER:
    case NET_CHAR_GEN_COLOR:
        case NET_CHAR_GEN_RACE:
        case NET_CHAR_GEN_RACE_CONFIRM:   
	case NET_CHAR_GEN_SEX:
	case NET_STAT_MENU:
	case NET_STAT_STR:
	case NET_STAT_INT:
	case NET_STAT_WIS:
	case NET_STAT_DEX:
	case NET_STAT_CON:
    case NET_STAT_AGE:
    case NET_STAT_SIZE:
      actor_gen( d->character, argument ); break;

    case NET_READ_MOTD:
    sendcli( d, "CONNECTED");
    sprintf( buf2, "\n\r\n\rWelcome to %s.\n\r\n\r",  VERSION_STR );
    save_actor_prop( ch );

    /*
     * Add to connected list.
     */
    ch->next      = actor_list;
    actor_list     = ch;
    DC(d)  = NET_PLAYING;

#if defined(NEVER)
    /*
     * Move mounts from waiting list to full game.
     */
    {
        PLAYER_DATA *wch;

        for ( wch = mount_list;  wch != NULL;  wch = wch->next )
        {
            PLAYER_DATA *prev;

            if ( wch->master != ch )
            continue;


            /*
             * Clip from list if its the first.
             */
            if ( wch == mount_list )
            {
                mount_list = mount_list->next;
                wch->next = actor_list;
                actor_list = wch;
                actor_to_scene( wch, wch->in_scene );
                continue;
            }

            for ( prev = mount_list; prev != NULL; prev = prev->next )
            {
                if ( prev->next == wch )
                {
                    prev->next = wch->next;
                    break;
                }
            }

            if ( prev == NULL ) bug( "Newbie: prev mount not found.", 0 );

            wch->next = actor_list;
            actor_list = wch;
            actor_to_scene( wch, wch->in_scene );
        }
    }
#endif

    /*
     * Figure out scene, if invalid, choose new scene.
     */
    if ( ch->in_scene == NULL )
    {
         if ( IS_IMMORTAL(ch) )
         ch->in_scene = get_scene_index( SCENE_VNUM_LIMBO );
    else ch->in_scene = get_scene_index( SCENE_VNUM_DEATH );
    }
     
    if ( ch->in_scene == NULL )
         ch->in_scene = get_scene_index( SCENE_VNUM_TEMPLATE );

    sprintf( log_buf, "%s@%s has connected.", ch->name, d->host );
    log_string( log_buf );
        
    actor_to_scene( ch, ch->in_scene );
    save_actor_prop( ch );
    act( "$n has entered the game.", ch, NULL, NULL, TO_NOTVICT );
    cmd_look( ch, "auto" );

	break;
    }

    return;
}




void newbie_check( CONNECTION_DATA *d )
{
    PLAYER_DATA *ch = d->character;

    if ( ch == NULL ) return;


    if ( DC(d) == NET_DOC_MENU && d->showstr_point == NULL )
    send_to_actor( "> ", d->character );

    return;
}



bool  apply_ok( PLAYER_DATA *ch )
{
    if ( !str_cmp( ch->name, "guest" ) )
	{
/*        send_to_actor( "You need to choose your name.\n\r", ch );*/
		return FALSE;
	}

    if ( SMT(ch->short_descr) )
	{
/*        send_to_actor( "You need to set your short description.\n\r", ch );*/
		return FALSE;
	}

    if ( SMT(ch->long_descr) )
	{
/*        send_to_actor( "You need to set your long description.\n\r", ch );*/
		return FALSE;
	}

    if ( SMT(ch->description) )
	{
/*        send_to_actor( "You need to set your description.\n\r", ch );*/
		return FALSE;
	}

    if ( SMT(PC(ch,email)) )
	{
/*        send_to_actor( "You need to set your email.\n\r", ch );*/
		return FALSE;
	}

    if ( SMT(ch->keywords) )
	{
/*        send_to_actor( "You need to set your extended keywords.\n\r", ch );*/
		return FALSE;
	}

    return TRUE;
}



/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
    /*
     * Reserved words or anti-lamer tactics.
     */
  if ( is_name( name, "all auto immortal self newbie god hero"
                      " someone you me the avatar here" ) )
  return FALSE;


    /*
     * Here comes the list of obsenities.  Not exactly the eloquent speech
     * I'm used to, but since the world isn't an eloquent place, neither is
     * this conditional.
     * Purpose?  To get rid of lamers who like to swear in their names,
     * usually at the expense of the hard working programmers and admins
     * who provide the service for them.   "Ass" is allowed as a substring
     * due to its popularity in words ("Bass, Mass, Assassin etc..") and its
     * rather low-degree of obsene-ness.
     */

    if ( !str_infix( name, "fuck" )      ||   !str_infix( name, "shit" )
    ||   !str_infix( name, "cunt" )      ||   !str_infix( name, "penis" )
    ||   !str_infix( name, "asshole" )   ||   !str_infix( name, "vagina" )
    ||   !str_cmp  ( name, "ass" )     )
        return FALSE;

    /*
     * Length restrictions.
     */
    if ( strlen(name) <  3 )
	return FALSE;

#if defined(MSDOS)
    if ( strlen(name) >  8 )
	return FALSE;
#endif

#if defined(macintosh) || defined(unix)
#ifdef NAME_LENGTH
    if ( strlen(name) > NAME_LENGTH )
#else
    if ( strlen(name) > 12 )
#endif
    return FALSE;
#endif

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha(*pc) )
		return FALSE;
	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;
    }

    /*
     * Prevent players from naming themselves after actors.
     */
#ifdef NOACTORJR
    {
	extern ACTOR_INDEX_DATA *actor_index_hash[MAX_KEY_HASH];
	ACTOR_INDEX_DATA *pActorIndex;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pActorIndex  = actor_index_hash[iHash];
		  pActorIndex != NULL;
		  pActorIndex  = pActorIndex->next )
	    {
        if ( is_name( name, pActorIndex->name ) )
		    return FALSE;
	    }
	}
    }
#endif

    return TRUE;
}




/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( CONNECTION_DATA *d, char *name, bool fConn )
{
    PLAYER_DATA *ch;
    char buf[MAX_STRING_LENGTH];

    if ( !d) return FALSE;

    for ( ch = actor_list; ch != NULL; ch = ch->next )
    {
	if ( !IS_NPC(ch)
	&& ( !fConn || ch->desc == NULL )
	&&  (d->character && !str_cmp( d->character->name, ch->name )) )
	{
        if ( fConn == FALSE )                            /* WTF? */
	    {
        free_string( PC(d->character,pwd) );
        PC(d->character,pwd) = str_dup( PC(ch,pwd) );
	    }
	    else
	    {
		free_char( d->character );
		d->character = ch;
		ch->desc	 = d;
		ch->timer	 = 0;
                if ( !sendcli( d, "CONNECTED RECONNECT" ) )
		send_to_actor( "Reconnecting.\n\r", ch );
		act( "$n has reconnected.", ch, NULL, NULL, TO_SCENE );
		sprintf( log_buf, "%s@%s reconnected.", ch->name, d->host );
		log_string( log_buf );
        DC(d) = NET_PLAYING;
	
        snprintf( buf, MAX_STRING_LENGTH, "Notify>  %s", log_buf );
        NOTIFY( buf, LEVEL_IMMORTAL, WIZ_NOTIFY_LOGIN );

	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( CONNECTION_DATA *d, char *name )
{
    CONNECTION_DATA *dold;

    for ( dold = connection_list; dold; dold = dold->next )
    {
        if ( dold != d
          && dold->character != NULL
          && dold->connected != NET_GET_NAME
          && dold->connected != NET_GET_OLD_PASSWORD
          && !str_cmp( name, dold->original
                           ? dold->original->name : dold->character->name ) )
        {
             write_to_buffer( dold, "Replacing with new player...Bye!\n\r", 0);
             close_socket( dold );
             return TRUE;
        }
    }

    return FALSE;
}

