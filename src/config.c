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
 ******************************************************************************
 *   n a m e l e s s i n c a r n a t e          dedicated to chris cool       *
 ******************************************************************************/

#if defined(BSD)
#include <types.h>
#include <crypt.h>
#include <unistd.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <zlib.h>

#include "net.h"
#include "nimud.h"

char *default_color_variable = "^N";
char *default_color_variable_di = NTEXT;

/*
 * Syntax:  clear
 */
void cmd_clear ( PLAYER_DATA *ch, char *argument )
{
    if ( !HAS_ANSI(ch) )
    send_to_actor( "You do not have ansi support enabled.\n\r", ch );
    else
    send_to_actor( CLRSCR, ch );

    return;
}

void cmd_tips( PLAYER_DATA *ch, char *argument )
{
    cmd_config( ch, "tips" );     
};

void cmd_client( PLAYER_DATA *ch, char *argument )
{
    cmd_config( ch, "client" );
}

void cmd_blank ( PLAYER_DATA *ch, char *argument )
{
    cmd_config( ch, "blank" );
}

void cmd_brief ( PLAYER_DATA *ch, char *argument )
{
    cmd_config( ch, "-brief" );
}

void cmd_compact ( PLAYER_DATA *ch, char *argument )
{
    cmd_config( ch, "compact" );
}

void cmd_holylight ( PLAYER_DATA *ch, char *argument )
{
    cmd_config( ch, "holylight" );
}

/*
 * Syntax:  pager
 *          pager [number]
 */
void cmd_pager ( PLAYER_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int lines;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        if ( ch->pagelen == 0 )
        {
            ch->pagelen = 22;
            send_to_actor( "Pager on; defaulted to 22 lines per page.\n\r", ch );
            return;
        }

        send_to_actor( "Pager off.\n\r", ch );
        ch->pagelen = 0;
        return;
    }

	lines = atoi( arg );

    if ( lines < 0 || lines > 500 )
    {
    send_to_actor( "Invalid page length value.\n\r", ch );
	return;
    }

    ch->pagelen = lines;
    snprintf( buf, MAX_STRING_LENGTH, "Pager set to %d lines.\n\r", lines );
    send_to_actor( buf, ch );
    return;
}


/*
 * Originally coded by Morgenes for Aldara Mud
 * Syntax:  prompt
 *          prompt all
 *          prompt [text]
 */
void cmd_prompt( PLAYER_DATA *ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];

   if( argument[0] == '\0' )
   {
      if (!IS_SET(ch->act2, PLR_PROMPT))
      {
         send_to_actor( "Prompt ON.\n\r", ch );
         SET_BIT(ch->act2, PLR_PROMPT);
      }
      else
      {
         REMOVE_BIT( ch->act2, PLR_PROMPT );
         send_to_actor( "Prompt OFF.\n\r", ch );
      }
      return;
   }

   if( !str_cmp( argument, "all" ) )
      strcpy( buf, "< ^h ^m ^e > " );
else
   if( !str_cmp( argument, "immortal" ) ) 
      strcpy( buf, "^c^I^C^N> " );
else
   if( !str_cmp( argument, "visual" ) )
      strcpy( buf, "^f^n^N< ^B^4^h^N ^B^3^m^N ^5^e^N > " );
else
   if( !str_cmp( argument, "numbers" ) )
      strcpy( buf, "^B< ^N^4^H ^M ^E ^N^B>^N " );
else
   if( !str_cmp( argument, "colors" ) || !str_cmp( argument, "color" ) )
      strcpy( buf, "^B< ^N^4^h ^5^m ^3^e ^N^B>^N " );
else
   if( !str_cmp( argument, "inverse" ) )
      strcpy( buf, "^2^I^0^4 ^h ^6 ^m ^3 ^e ^N " );
   else
   if( !str_cmp( argument, "invnum" ) )
      strcpy( buf, "^2^I^0^4 ^H ^6 ^M ^3 ^E ^N " );
   else
   {
      if ( strlen(argument) > 100 )
         argument[100] = '\0';
      strcpy( buf, argument );
      smash_tilde( buf );
   }

   free_string( ch->prompt );
   ch->prompt = str_dup( buf );
   if ( ch->exp_level > 0 ) 
   sprintf( buf2, "Prompt set to '%s'.\n\r", ch->prompt );
   else sprintf( buf2, "Prompt set.\n\r" );
   if ( ch->desc->connected <= NET_PLAYING ) 
   send_to_actor( buf2, ch );

   if (!IS_SET(ch->act2, PLR_PROMPT))
         SET_BIT(ch->act2, PLR_PROMPT);

   return;
}
 

/*
 * Syntax:  password [old] [new]
 */
void cmd_password( PLAYER_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( IS_NPC(ch) )
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_actor( "Syntax: password <old> <new>.\n\r", ch );
	return;
    }

    if ( !str_cmp( crypt(arg1, ch->name), PC(ch,pwd) ) )
    {
	WAIT_STATE( ch, 40 );
	send_to_actor( "Wrong password.  Wait 10 seconds.\n\r", ch );
	return;
    }

    if ( strlen(arg2) < 5 )
    {
	send_to_actor(
	    "New password must be at least five characters long.\n\r", ch );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
	    send_to_actor(
		"New password not acceptable, try again.\n\r", ch );
	    return;
	}
    }

    free_string( PC(ch,pwd) );
    PC(ch,pwd) = str_dup( pwdnew );
    save_actor_prop( ch );
    send_to_actor( "Ok.\n\r", ch );
    return;
}


/*
 * Syntax:  ansi
 *          ansi colors
 *          ansi [keyword] [#]
 *          ansi [keyword] [color]
 */
void cmd_ansi( PLAYER_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int c;
    int c2;

    if ( IS_NPC(ch) || !HAS_ANSI(ch) )
    {
        cmd_help( ch, "ansi" );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_actor( "[Keyword  ]  Color Sample\n\r", ch );

        snprintf( buf, MAX_STRING_LENGTH, "[Scenes    ]  Scene titles appear %s%s%s.\n\r",
                  color_table[PC(ch,colors[COLOR_SCENE_TITLE])].code,
                  color_table[PC(ch,colors[COLOR_SCENE_TITLE])].name,
                  color_table[19].code );
        send_to_actor( buf, ch );
        if ( IS_IMMORTAL(ch) )
        {
        snprintf( buf, MAX_STRING_LENGTH, "[Immtalk  ]  The immortal channel appears in %s%s%s.\n\r",
                  color_table[PC(ch,colors[COLOR_IMMTALK])].code,
                  color_table[PC(ch,colors[COLOR_IMMTALK])].name,
                  color_table[19].code );
        send_to_actor( buf, ch );
        }
        snprintf( buf, MAX_STRING_LENGTH, "[Spoken   ]  Spoken communications are shown in %s%s%s.\n\r",
                  color_table[PC(ch,colors[COLOR_COMM])].code,
                  color_table[PC(ch,colors[COLOR_COMM])].name,
                  color_table[19].code );
        send_to_actor( buf, ch );
        snprintf( buf, MAX_STRING_LENGTH, "[Chats    ]  Chat channel communications are %s%s%s.\n\r",
                  color_table[PC(ch,colors[COLOR_CHAT])].code,
                  color_table[PC(ch,colors[COLOR_CHAT])].name,
                  color_table[19].code );
        send_to_actor( buf, ch );
        snprintf( buf, MAX_STRING_LENGTH, "[Groups   ]  Group info and speech appears %s%s%s.\n\r",
                  color_table[PC(ch,colors[COLOR_GROUPS])].code,
                  color_table[PC(ch,colors[COLOR_GROUPS])].name,
                  color_table[19].code );
        send_to_actor( buf, ch );
        snprintf( buf, MAX_STRING_LENGTH, "[Lists    ]  Object lists appear %s%s%s.\n\r",
                  color_table[PC(ch,colors[COLOR_PROP_LIST])].code,
                  color_table[PC(ch,colors[COLOR_PROP_LIST])].name,
                  color_table[19].code );
        send_to_actor( buf, ch );
        return;
    };

    if ( !str_prefix( arg1, "colors" ) )
    {
        send_to_actor( "Colors:\n\r", ch );
        for ( c = 0; c < 20; c++ )
        {
            snprintf( buf, MAX_STRING_LENGTH, "[%2d] %s%s%s\n\r",
                          c,
                          c != 0 ? color_table[c].code : "",
                          c == 0 ? "(black)" : color_table[c].name,
                          NTEXT );
            send_to_actor( buf, ch );
        }
        return;
    }

    c = -1;
    if ( !str_prefix( arg1, "scenes" ) )      c = COLOR_SCENE_TITLE;
    if ( !str_prefix( arg1, "immtalk" ) )    c = COLOR_IMMTALK;
    if ( !str_prefix( arg1, "spoken" ) )     c = COLOR_COMM;
    if ( !str_prefix( arg1, "groups" ) )     c = COLOR_GROUPS;
    if ( !str_prefix( arg1, "chats" ) )      c = COLOR_CHAT;
    if ( !str_prefix( arg1, "lists" ) )      c = COLOR_PROP_LIST;
    
    if ( c < 0 )
    {
        send_to_actor( "Configure which color?\n\r", ch );
        return;
    }

    if ( arg2[0] == '\0' )
    {
        send_to_actor( "Set it to what color?\n\r", ch );
        return;
    }

    if ( !is_number( arg2 ) )
    {
        for ( c2 = 0; color_table[c2].name[0] != ' '; c2++ )
        {
            if ( !str_prefix( arg2, color_table[c2].name ) )
            break;
        }
    }
    else c2 = atoi( arg2 );

    snprintf( buf, MAX_STRING_LENGTH, "Invalid color index.\n\r" );

    if ( c2 < 20
      && c2 >= 0 )
    {
        PC(ch,colors[c]) = c2;
        snprintf( buf, MAX_STRING_LENGTH, "Set to %s%s%s.\n\r",
                 color_table[c2].code,
                 color_table[c2].name, NTEXT );
    };

    send_to_actor( buf, ch );
    return;
}


/*
 * Syntax:  config
 *          config [keyword]
 *          config [+/-][keyword]
 */
void cmd_config( PLAYER_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg_map[MIL];
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
	return;

    argument = one_argument( argument, arg );
    one_argument( argument, arg_map );

    if ( arg[0] == '\0' )
    {
        send_to_actor( "[ Keyword  ] Option\n\r", ch );

    send_to_actor(  IS_SET(ch->act2, PLR_AUTOEXIT)
        ? "[*EXITS    ] You automatically see exits.\n\r"
        : "[ exits    ] You don't automatically see exits.\n\r"
        , ch );

    send_to_actor(  IS_SET(ch->act2, PLR_ALIASES)
        ? "[*ALIASES  ] You have activated use of aliases.\n\r"
        : "[ aliases  ] You aren't using any command aliases.\n\r"
        , ch );

    send_to_actor(  IS_SET(ch->act2, PLR_VERBOSE)
        ? "[*VERBOSE  ] You automatically see verbose exits.\n\r"
        : "[ verbose  ] You don't automatically see verbose exits.\n\r"
        , ch );

    if ( HAS_ANSI(ch) ) {
    send_to_actor(  IS_SET(ch->act2, PLR_TIMECOLOR)
        ? "[*TIMECOLOR] Room descriptions change color based on time.\n\r"
        : "[ timecolor] You don't see time-based color effects.\n\r"
        , ch );
    }

    send_to_actor(  IS_SET(ch->act2, PLR_AUTOMAP) 
        ? "[*MAP      ] You automatically see a map.\n\r"
        : "[ map      ] You don't automatically see a map.\n\r"
        , ch );

    send_to_actor(  IS_SET(ch->act2, PLR_BLANK)
        ? "[*BLANK    ] You have a blank line before your prompt.\n\r"
        : "[ blank    ] You have no blank line before your prompt.\n\r"
	    , ch );

    send_to_actor(  IS_SET(ch->act2, PLR_BRIEF)
        ? "[*BRIEF    ] You see brief descriptions.\n\r"
        : "[ brief    ] You see long descriptions.\n\r"
	    , ch );
         
    send_to_actor(  IS_SET(ch->act2, PLR_COMBINE)
        ? "[*COMPACT  ] You see prop lists in combined format.\n\r"
        : "[ compact  ] You see prop lists in single format.\n\r"
	    , ch );

    send_to_actor(  IS_SET(ch->act2, PLR_PROMPT)
        ? "[*PROMPT   ] You have a prompt.\n\r"
        : "[ prompt   ] You don't have a prompt.\n\r"
	    , ch );

    send_to_actor(  IS_SET(ch->act2, PLR_ANSI)
        ? "[*ANSI     ] You receive ansi color codes.\n\r"
        : "[ ansi     ] You don't receive ansi color codes.\n\r"
	    , ch );

    send_to_actor(  IS_SET(ch->act2, PLR_MXP)
        ? "[*MXP      ] You receive MXP color codes.\n\r"
        : "[ mxp      ] You don't receive MXP color codes.\n\r"
	    , ch );

    send_to_actor(  IS_SET(ch->act2, PLR_CLRSCR)
        ? "[*CLRSCR   ] You receive clear screen codes.\n\r"
        : "[ clrscr   ] You don't receive clear screen codes.\n\r"
	    , ch );

    send_to_actor(  IS_SET(ch->act2, PLR_CLRSCR)
        ? "[*AGE      ] You are using the Ascii Graphics Engine.\n\r"
        : "[ age      ] You have muted the Ascii Graphics Engine.\n\r"
	    , ch );

    send_to_actor(  IS_SET(ch->act2, PLR_TELNET_GA)
        ? "[*TELNETGA ] You receive a telnet GA sequence.\n\r"
        : "[ telnetga ] You don't receive a telnet GA sequence.\n\r"
	    , ch );

    send_to_actor(  HAS_CLIENT(ch)
        ? "[*CLIENT   ] You are using the web client.\n\r"
        : "[ client   ] You are not using the web client.\n\r"
            , ch );


    send_to_actor(  SHOW_TIPS(ch)
        ? "[*TIPS     ] Helpful tips are enabled.\n\r" : ""
            , ch );


    send_to_actor(  IS_SET(ch->act2, PLR_SILENCE)
        ? "[*SILENCE  ] You are silenced.\n\r"
	    : ""
	    , ch );

    send_to_actor( !IS_SET(ch->act2, PLR_NO_EMOTE)
        ? ""
        : "[ emote    ] It is impossible to emote.\n\r"
        , ch );

    send_to_actor( !IS_SET(ch->act2, PLR_IDLE)
        ? ""
        : "[ IDLE     ] It is possible for you to idle forever.\n\r"
        , ch );

    if ( ch->pagelen == 0 )
    send_to_actor( "[ pager    ] You are not using the pager.\n\r", ch );
    else
    {
        snprintf( buf, MAX_STRING_LENGTH, "[    %-3d   ] You are using the pager.\n\r", ch->pagelen );
        send_to_actor( buf, ch );
    }

    if ( PC(ch,mapsize) == 0 || !IS_SET(ch->act2, PLR_AUTOMAP) )
    send_to_actor( "[ mapping  ] Your automap is default resolution.\n\r", ch );
    else
    {
        snprintf( buf, MAX_STRING_LENGTH, "[    %-3d  ] Your automapping resolution.\n\r", 
                 PC(ch,mapsize) );
        send_to_actor( buf, ch );
    }

if ( IS_IMMORTAL(ch) )
{
    send_to_actor( IS_SET(ch->act2, CHANNEL_IMMTALK )
        ? "[*IMMTALK  ] You are listening to the immortal banter.\n\r"
        : "[ immtalk  ] You are deaf to the immortal banter.\n\r"
	    , ch );

    send_to_actor( IS_SET(ch->act2, CHANNEL_WISHES )
        ? "[*WISHES   ] You are listening to mortal wishes.\n\r"
        : "[ wishes   ] You are deaf to mortal wishes.\n\r"
	    , ch );

    send_to_actor( IS_SET(ch->act2, WIZ_EQUIPMENT )
        ? "[*EQUIPMENT] You see equipment lists when looking at someone.\n\r"
        : "[ equipment] You do not see equipment lists when looking at someone.\n\r"
	    , ch );

    send_to_actor( IS_SET(ch->act2, WIZ_HOLYLIGHT )
        ? "[*HOLYLIGHT] A holy light surrounds you.\n\r"
        : "[ holylight] You are not using holylight.\n\r"
	    , ch );

    send_to_actor( IS_SET(ch->act, WIZ_NOTIFY)
        ? "[*NOTIFY   ] You view immortal notifications.\n\r"
        : "[ notify   ] You do not view immortal notifications.\n\r"
	, ch );

    send_to_actor( IS_SET(ch->act2, WIZ_TRANSLATE)
	? "[*TRANSLATE] You understand all speech automatically.\n\r"
	: "[ translate] You do not understand all speech automatically.\n\r"
        , ch );
}

    }
    else
    {
    char arg2[MAX_STRING_LENGTH];
	bool fSet;
    int bit = 0;
    int *act_var = &ch->act2;

    fSet = arg[0] == '+';
    if ( arg[0] == '-' || arg[0] == '+' )
    {
                sprintf( arg2, "%c", arg[0] );
                sprintf( arg, "%s", arg+1 );
    }
    
         if ( !str_prefix( arg, "exits"    ) ) bit = PLR_AUTOEXIT;
    else if ( !str_prefix( arg, "map"      ) ) bit = PLR_AUTOMAP;
    else if ( !str_prefix( arg, "verbose"  ) ) bit = PLR_VERBOSE;
    else if ( !str_prefix( arg, "timecolor") ) bit = PLR_TIMECOLOR;
    else if ( !str_prefix( arg, "blank"    ) ) bit = PLR_BLANK;
    else if ( !str_prefix( arg, "brief"    ) ) bit = PLR_BRIEF;
    else if ( !str_prefix( arg, "tips"     ) ) bit = PLR_TIPS;
    else if ( !str_prefix( arg, "compact"  ) ) bit = PLR_COMBINE;
    else if ( !str_prefix( arg, "prompt"   ) ) bit = PLR_PROMPT;
    else if ( !str_prefix( arg, "color"    ) ) bit = PLR_ANSI;
    else if ( !str_prefix( arg, "ansi"     ) ) bit = PLR_ANSI;
    else if ( !str_prefix( arg, "mxp"      ) ) bit = PLR_MXP;
    else if ( !str_prefix( arg, "age"      ) ) bit = PLR_NAGE;
    else if ( !str_prefix( arg, "clrscr"   ) ) bit = PLR_CLRSCR;
    else if ( !str_prefix( arg, "telnetga" ) ) bit = PLR_TELNET_GA;
    else if ( !str_prefix( arg, "idle"     ) ) bit = PLR_IDLE;
    else if ( !str_prefix( arg, "mapping"  ) ) {
         int mapsize;
         mapsize = atoi( arg_map );
         if ( mapsize < 0 ) { send_to_actor("Invalid size.\n\r", ch ); return; }
         PC(ch,mapsize) = mapsize <= 50 ? mapsize : 50;
         sprintf( arg, "Mapping resolution set to %d.\n\r", PC(ch,mapsize) );
         send_to_actor( arg, ch );
         return;
    }
    else if ( is_number( arg ) )
    {
         int pagelen;

         pagelen = atoi( arg );
         if ( pagelen >= 0 && pagelen < 500 )
         {
              sprintf( arg, "Pager set to %d lines.\n\r", pagelen );
              send_to_actor( arg, ch );
              ch->pagelen = pagelen;
              return;
         }
         else
         {
              send_to_actor( "Pager can only pause up to 500 lines.\n\r", ch );
              return;
         }
    }
    else
    if ( IS_IMMORTAL(ch) )
    {
        if ( !str_prefix( arg, "notify" ) )
        {
            bit = WIZ_NOTIFY;
            act_var = &ch->act;
        }
        else if ( !str_prefix( arg, "holylight" ) ) bit = WIZ_HOLYLIGHT;
        else if ( !str_prefix( arg, "equipment" ) ) bit = WIZ_EQUIPMENT;
        else if ( !str_prefix( arg, "immtalk"   ) ) bit = CHANNEL_IMMTALK;
        else if ( !str_prefix( arg, "wishes"    ) ) bit = CHANNEL_WISHES;
	else if ( !str_prefix( arg, "translate" ) ) bit = WIZ_TRANSLATE;
	else
        {
            send_to_actor( "Set which option?\n\r", ch );
            return;
        }
    }
	else
	{
        send_to_actor( "Set which option?\n\r", ch );
	    return;
	}

        if ( fSet )
        {
            snprintf( buf, MAX_STRING_LENGTH, "Enabled.\n\r" );
            SET_BIT    (*act_var, bit);
        }
        else
        if ( !fSet && arg2[0] == '-' )
        {
            snprintf( buf, MAX_STRING_LENGTH, "Disabled.\n\r" );
            REMOVE_BIT (*act_var, bit);
        }
        else
        {
            if (IS_SET(ch->act2, bit))
            {
            snprintf( buf, MAX_STRING_LENGTH, "Disabled.\n\r" );
            REMOVE_BIT (*act_var, bit);
            }
            else
            {
            snprintf( buf, MAX_STRING_LENGTH, "Enabled.\n\r" );
            SET_BIT    (*act_var, bit);
            }
        }

            if ( ch->desc->connected <= NET_PLAYING ) 
    send_to_actor( buf, ch );
    }

    return;
}


/*
 * Syntax:  notify
 *          notify [keyword]
 */
void cmd_notify( PLAYER_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );

    if ( !str_cmp(arg1, "death")
      || !str_cmp(arg1, "deaths") )
    {
       if ( IS_SET(ch->act, WIZ_NOTIFY_DEATH) )
       {
          send_to_actor( "Death notification is now OFF.\n\r", ch );
          REMOVE_BIT(ch->act, WIZ_NOTIFY_DEATH);
          return;
       }
       else
       {
          send_to_actor( "Death notification is now ON.\n\r", ch );
          SET_BIT(ch->act, WIZ_NOTIFY_DEATH);
          return;
       }
    }
    else
    if ( !str_cmp(arg1, "damage") )
    {
       if ( IS_SET(ch->act, WIZ_NOTIFY_DAMAGE) )
       {
          send_to_actor( "Damage notification is now OFF.\n\r", ch );
          REMOVE_BIT(ch->act, WIZ_NOTIFY_DAMAGE);
          return;
       }
       else
       {
          send_to_actor( "Damage notification is now ON.\n\r", ch );
          SET_BIT(ch->act, WIZ_NOTIFY_DAMAGE);
          return;
       }
    }
    else
    if ( !str_cmp(arg1, "script")
      || !str_cmp(arg1, "scripts") )
    {
       if ( IS_SET(ch->act, WIZ_NOTIFY_SCRIPT) )
       {
          send_to_actor( "Script debug notification is now OFF.\n\r", ch );
          REMOVE_BIT(ch->act, WIZ_NOTIFY_SCRIPT);
          return;
       }
       else
       {
          send_to_actor( "Script debug notification is now ON.\n\r", ch );
          SET_BIT(ch->act, WIZ_NOTIFY_SCRIPT);
          return;
       }
    }

    if ( !str_cmp(arg1, "notify") )
    {
       if ( IS_SET(ch->act, WIZ_NOTIFY) )
       {
          send_to_actor( "Notify is now INACTIVE.\n\r", ch );
          REMOVE_BIT(ch->act, WIZ_NOTIFY);
          return;
       }
       else
       {
          send_to_actor( "Notify is now ACTIVE.\n\r", ch );
          SET_BIT(ch->act, WIZ_NOTIFY);
          return;
       }
    }
    else
    if ( !str_cmp(arg1, "bug")
      || !str_cmp(arg1, "bugs") )
    {
       if ( IS_SET(ch->act, WIZ_NOTIFY_BUG) )
       {
          send_to_actor( "Bug notification is now OFF.\n\r", ch );
          REMOVE_BIT(ch->act, WIZ_NOTIFY_BUG);
          return;
       }
       else
       {
          send_to_actor( "Bug notification is now ON.\n\r", ch );
          SET_BIT(ch->act, WIZ_NOTIFY_BUG);
          return;
       }
    }
    else
    if ( !str_cmp(arg1, "log")
      || !str_cmp(arg1, "logs") )
    {
       if ( IS_SET(ch->act, WIZ_NOTIFY_LOG) )
       {
          send_to_actor( "Log notification is now OFF.\n\r", ch );
          REMOVE_BIT(ch->act, WIZ_NOTIFY_LOG);
          return;
       }
       else
       {
          send_to_actor( "Log notification is now ON.\n\r", ch );
          SET_BIT(ch->act, WIZ_NOTIFY_LOG);
          if ( IS_SET( ch->act, WIZ_NOTIFY_BUG ) ) REMOVE_BIT(ch->act, WIZ_NOTIFY_BUG);
          return;

       }
    }
    else
    if ( !str_cmp(arg1, "event")
      || !str_cmp(arg1, "events") )
    {
       if ( IS_SET(ch->act, WIZ_NOTIFY_EVENT) )
       {
          send_to_actor( "Event notification is now OFF.\n\r", ch );
          REMOVE_BIT(ch->act, WIZ_NOTIFY_EVENT);
          return;
       }
       else
       {
          send_to_actor( "Event notification is now ON.\n\r", ch );
          SET_BIT(ch->act, WIZ_NOTIFY_EVENT);
          if ( IS_SET( ch->act, WIZ_NOTIFY_BUG ) ) REMOVE_BIT(ch->act, WIZ_NOTIFY_BUG);
          return;

       }
    }
    else
    if ( !str_cmp(arg1, "login")
      || !str_cmp(arg1, "logins") )
    {
       if ( IS_SET(ch->act, WIZ_NOTIFY_LOGIN) )
       {
          send_to_actor( "Login notify is now OFF.\n\r", ch );
          REMOVE_BIT(ch->act, WIZ_NOTIFY_LOGIN);
          return;
       }
       else
       {
          send_to_actor( "Login notify is now ON.\n\r", ch );
          SET_BIT(ch->act, WIZ_NOTIFY_LOGIN);
          return;
       }
    }
    else
    {
        send_to_actor( "[ Keyword  ] Option\n\r", ch );

    send_to_actor(  IS_SET(ch->act, WIZ_NOTIFY_LOGIN)
        ? "[*LOGINS   ] You are monitoring player logins.\n\r"
        : "[ logins   ] You don't monitor player logins.\n\r"
        , ch );

    send_to_actor(  IS_SET(ch->act, WIZ_NOTIFY_BUG)
        ? "[*BUGS     ] You see all bug reports.\n\r"
        : "[ bugs     ] You are currently ignoring bug reports.\n\r"
	    , ch );

    send_to_actor(  IS_SET(ch->act, WIZ_NOTIFY_SCRIPT)
        ? "[*SCRIPTS  ] You are debugging scripts.\n\r"
        : "[ scripts  ] You are not debugging scripts.\n\r"
	    , ch );
         
    send_to_actor(  IS_SET(ch->act, WIZ_NOTIFY_LOG)
        ? "[*LOGS     ] You are viewing log entries.\n\r"
        : "[ logs     ] You are not viewing log entries.\n\r"
	    , ch );

    send_to_actor(  IS_SET(ch->act, WIZ_NOTIFY_LOG)
        ? "[*EVENTS   ] You are viewing events.\n\r"
        : "[ event    ] You are not viewing events.\n\r"
	    , ch );

    send_to_actor(  IS_SET(ch->act, WIZ_NOTIFY_DEATH)
        ? "[*DEATHS   ] You are notified of player deaths.\n\r"
        : "[ deaths   ] You are not notified of player deaths.\n\r"
	    , ch );

    send_to_actor(  IS_SET(ch->act, WIZ_NOTIFY_DAMAGE)
        ? "[*DAMAGE   ] You are notified of combat damage.\n\r"
        : "[ damage   ] You are not notified of combat damage.\n\r"
	    , ch );

    send_to_actor(  IS_SET(ch->act, WIZ_NOTIFY)
        ? "[*NOTIFY   ] Notify is currently active.\n\r"
        : "[ notify   ] Notify has been rendered inactive.\n\r"
	    , ch );

        return;
    }

    return;
}



/*
 * Sets the user's flag design.
 */
void cmd_flag( PLAYER_DATA *ch, char *argument ) {
  char arg[MSL];

  if ( IS_NPC(ch) ) return;
  argument = one_argcase( argument, arg );
  argument =arg;
  if ( *argument == '\0' ) {
     if ( PC(ch,flag) == NULL || strlen(PC(ch,flag)) == 0 ) {
        send_to_actor( "You have no kingdom-identifying flag currently set.\n\r", ch ); 
        send_to_actor( "To set your flag, you may enter 3 characters with many prompt color codes.\n\r", ch ); 
        send_to_actor( "To enter a code, use ^^4 for red (for example).  Try ^^ with one of these:\n\r", ch );
        send_to_actor( "^11^22^33^44^55^66^770^BB^NN^II^FF^N or ^0^==^--^<<^>>^??^##^::^+^7+^N\n\r", ch );
        send_to_actor( "You can use ^^^^ for ^^.\n\r", ch ); 
        send_to_actor( "Example: flag \"^^: ^^B*^^: \"\n\r", ch ); 
        send_to_actor( "Would set your flag to: ^: ^B*^: ^+^N\n\r", ch ); 
        return;
     } else {
        send_to_actor( "Your flag is set to: ", ch );   
        send_to_actor( PC(ch,flag), ch );
        send_to_actor( "^+^N\n\r", ch );
        return;
     }
  }

 /* Check to make sure there are 3 visible characters amidst many color codes. */
  {
    int count=0; 
    char *p; 
    p=arg; 
    while ( *p != '\0' ) { 
     if ( *p == '^' ) { p++; if ( *p != '\0' ) { 
                              if ( !char_isof( *p, "12345670BNIF=-<>?#:+^" ) ) {
                                 send_to_actor( "You used an invalid ^^ code.  Try ^^ with one of these:\n\r", ch );
                                 send_to_actor( "^11^22^33^44^55^66^770^BB^NN^II^FF^N or ^0^==^--^>>^??^##^::^+^7+^N\n\r", ch );
                                 send_to_actor( "You can use ^^^^ for ^^.\n\r", ch ); return;
                               } p++;
                              } 
                             }  // skip past codes
     else { p++; count++; } // count none-code characters
    }
    if ( count > 3 ) { send_to_actor( "Your flag has too many regular characters between its color codes,\n\r", ch );
                       send_to_actor( "reduce the number of characters to 3, interspliced with color codes.\n\r", ch );
                       return; 
                     }
    if ( count < 3 ) { send_to_actor( "Your flag has too few regular characters, it needs 3 between the color codes.\n\r", ch ); 
                       return; 
                     }
  }

  PC(ch,flag) = str_dup( argument );

  send_to_actor( "Your flag was set to: ", ch );
  send_to_actor( PC(ch,flag), ch );
  send_to_actor( "^+^N\n\r", ch );
  return;
}
