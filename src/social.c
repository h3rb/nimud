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

#if defined(BSD)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <zlib.h>

#include "nimud.h"
#include "script.h"
#include "skills.h"
#include "net.h"

/*
 * Syntax:  rp [text]
 *          in text, #<keyword> gets replaced with "smote", e.g.
 *          smote hello #greet have a nice day -->
 *          "Hello," you greet, "Have a nice day"
 *          if no #<keyword> is found, it defaults to "say"
 */
void cmd_smote( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    PLAYER *actor;
    char *c, *p = "";
    char *message, *tailer;
    int smote = 0;	/* Defaults to "say" */

    const char * const smote_table[] =
    {
    "say",      "state",     "declare",    "greet",     "hail",
    "speak",    "report",    "remark",     "comment",   "continue",
    "chuckle",  "laugh",     "sneer",      "grin",      "smile",
    "exclaim",  "shout",     "yell",       "scream",    "grimace",
    "boom",     "thunder",   "roar",       "storm",     "blare",
    "bellow",   "demand",    "warn",       "advise",    "suggest",
    "observe",  "whisper",   "sob",        "sigh",      "coo",
    "groan",    "croak",     "rasp",       "agree",     "decide",
    "disagree", "prop",      "decline",    "utter",     "mumble",
    "mutter",   "murmur",    "grunt",      "stutter",   "ask",
    "respond",  "muse",      "ponder",     "wonder",    "ruminate",
    "grumble",  "growl",     "snap",       "snarl",     "spit",
    "retort",   "squeak",    "whimper",    "whine",     "beg",
    "pray",     "explain",   "offer",      "inquire",   "sing",
	""
    };


    if ( MTD(argument) )
    {
        send_to_actor( "Rp what?\n\r", ch );
        return;
    }

    /* Grab the '#' keyword */
    for ( c = argument; *c; c++ )
    {
	if ( *c == '#' )
	{
	    char keyword[ MAX_INPUT_LENGTH ];
	    register int i;
	    *c++ = '\0';
	    for( i = 0, keyword[0] = '\0'; *c && !isspace(*c); keyword[i++] = *c++ );
	    keyword[i] = '\0';
	    p = c;
	    if ( keyword[0] != '\0' )
	    {
		for( i = 0; smote_table[i][0] != '\0'; i++ )
		{
		    if ( !str_prefix(keyword, smote_table[i]) )
		    {
			smote = i;
			break;
		    }
		}
	    }
	    break;
	}
    }
    /* Strip leading blanks */
    while( *argument && isspace(*argument) ) argument++;
    while( *p && isspace(*p) ) p++;

    if ( MTD(argument) && MTP(p) )
    {
	send_to_actor( "Invalid or missing argument.\n\r", ch );
	return;
    }
	    
    /* Strip tailing blanks from the first part... */
    for( c = &argument[strlen(argument)-1]; *c && isspace(*c) ; c-- )
	*c = '\0';
    tailer = ispunct(*c) ? "" : ",";

    if ( !MTD(argument) )
    	snprintf( buf, MAX_STRING_LENGTH, "\"%s%s\" you ", capitalize(argument), tailer );
    else
	snprintf( buf, MAX_STRING_LENGTH, "You " );
    strcat( buf, smote_table[smote] );
    if ( !MTP(p) )
    	sprintf( &buf[strlen(buf)], ",  \"%s\"", capitalize(p) );
    else
	strcat( buf, "." );
    message = format_indent( str_dup(buf), "   ", 74, FALSE );
    snprintf( buf, MAX_STRING_LENGTH, "%sIn %s:\n\r  $t%s",
             color_table[GET_PC(ch,colors[COLOR_COMM],7)].actor_code,
             lang_table[ch->speaking].name,
             HAS_ANSI(ch) ? ANSI_NORMAL : "" );
    act( buf, ch, message, NULL, TO_ACTOR  );
    free_string( message );

    for ( actor = ch->in_scene->people;  actor != NULL;  actor = actor->next_in_scene )
    {
        if ( actor != ch && actor->desc != NULL )
        {
	    if ( !MTD(argument) )
	    {
	        snprintf( buf, MAX_STRING_LENGTH, "\"%s%s\" %s ",
		capitalize(garble_text( argument,
                  check_speech( actor, ch, lang_table[ch->speaking].pgsn )) ),
		tailer,
		PERS(ch, actor) );
	    }
	    else
	        snprintf( buf, MAX_STRING_LENGTH, "%s ", capitalize(PERS(ch, actor)) );

    	    strcat( buf, smote_table[smote] );
	    if ( !MTP(p) )
	    {
    	    	sprintf( &buf[strlen(buf)], "s,  \"%s\"",
		capitalize(garble_text( p,
                  check_speech( actor, ch, lang_table[ch->speaking].pgsn)) ) );
	    }
	    else
		strcat( buf, "s." );
    	    message = format_indent( str_dup(buf), "   ", 74, FALSE );

            snprintf( buf, MAX_STRING_LENGTH, "%sIn %s:\n\r  $t%s",
                      color_table[GET_PC(actor,colors[COLOR_COMM],7)].actor_code,
                      !NPC(actor) && learned(ch,lang_table[ch->speaking].pgsn) <= 0 ?
                      "a strange, foreign tongue" : lang_table[ch->speaking].name,
            	      HAS_ANSI(actor) ? ANSI_NORMAL : "" );
                      
	    act( buf, actor, message, ch, TO_ACTOR );
    	    free_string( message );
	}

        if ( NPC(ch) && NPC(actor)
          && actor->pIndexData->dbkey == ch->pIndexData->dbkey ) 
            continue;

        script_update( actor, TYPE_ACTOR, TRIG_SAY, ch, NULL, argument, NULL );
    }

    trigger_list( ch->in_scene->contents, TRIG_SAY, ch, NULL, argument, NULL );
    trigger_list( ch->carrying, TRIG_SAY, ch, NULL, argument, NULL );
    return;
}


/*
 * Syntax:  immtalk
 *          immtalk [text]
 */
void cmd_immtalk( PLAYER *ch, char *argument )
{
    PLAYER *och;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char *message;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        TOGGLE_BIT(ch->flag2, CHANNEL_IMMTALK);

        if ( IS_SET(ch->flag2, CHANNEL_IMMTALK) )
             send_to_actor( "Immtalk on.\n\r", ch );
        else send_to_actor( "Immtalk off.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "list" ) || !str_cmp( arg, "record" ) || !str_cmp( arg, "history" ) ) {
        page_to_actor( "Things said you witnessed:\n\r", ch );
        if ( NPC(ch) ) page_to_actor( PC(ch,immt_buf), ch );
        return;
    }

    message = format_indent( str_dup(argument), "   ", 74, FALSE );

    for ( och = actor_list;  och != NULL;  och = och->next )
    {
        if ( !IS_IMMORTAL(och)
          || !IS_SET(och->flag2,CHANNEL_IMMTALK)
          || NPC(och) )
        continue;
      
      strjoin(PC(och,immt_buf), NAME(ch));
      strjoin(PC(och,immt_buf), ": ");
      strjoin(PC(och,immt_buf), message);
      strjoin(PC(och,immt_buf), "\n\r" );

      snprintf( PC(och,immt_last), MSL, "%s: %s\n\r", NAME(ch), message );

        if ( och->desc != NULL )
        {
            snprintf( buf, MAX_STRING_LENGTH, "%s%s--\n\r   %s%s\n\r",
                   HAS_ANSI(och) ?
                       color_table[PC(och,colors)[COLOR_IMMTALK]].code : "",
                   och==ch ? "You" :
                     can_see(och, ch) ? STR(ch,name) : "Someone", message,
                   HAS_ANSI(och) ? ANSI_NORMAL : "" );
            send_to_actor( buf, och );
        }
    }

    free_string( message );
    return;
}


/*
 * Syntax:  chat
 *          chat [text]
 */
void cmd_chat( PLAYER *ch, char *argument )
{
    PLAYER *och;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char *message;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        TOGGLE_BIT(ch->flag2, CHANNEL_CHAT);

        if ( IS_SET(ch->flag2, CHANNEL_CHAT) )
             send_to_actor( "Chat on.\n\r", ch );
        else send_to_actor( "Chat off.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "list" ) || !str_cmp( arg, "record" ) || !str_cmp( arg, "history" ) ) {
        page_to_actor( "Things people said in the chat channel:\n\r", ch );
        if ( NPC(ch) ) page_to_actor( PC(ch,chat_buf), ch );
        return;
    }

    if ( !IS_SET(ch->flag2, CHANNEL_CHAT) ) cmd_chat( ch, "" );

    message = format_indent( str_dup(argument), "   ", 74, FALSE );

    for ( och = actor_list;  och != NULL;  och = och->next )
    {
        if ( !IS_SET(och->flag2,CHANNEL_CHAT)
          || NPC(och) )
        continue;

      strjoin(PC(och,chat_buf), NAME(ch));
      strjoin(PC(och,chat_buf), ": ");
      strjoin(PC(och,chat_buf), message);
      strjoin(PC(och,chat_buf), "\n\r" );

      snprintf( PC(och,chat_last), MSL, "%s -- %s\n\r", NAME(ch), argument );

        if ( och->desc != NULL )
        {
            snprintf( buf, MAX_STRING_LENGTH, "%s%s--\n\r   %s%s\n\r",
                   HAS_ANSI(och) ?
                       color_table[PC(och,colors)[COLOR_CHAT]].code : "",
                   och==ch ? "You" :
                     can_see(och, ch) ? STR(ch,name) : "Someone", message,
                   HAS_ANSI(och) ? ANSI_NORMAL : "" );
            send_to_actor( buf, och );
        }
    }

    free_string( message );
    return;
}



/*
 * Syntax:  reply [text]
 */
void cmd_reply(PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    PLAYER *victim;
    char *message;
    int position;

    message=argument; argument=one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        if ( ch->reply ) {
        send_to_actor( "Reply what to ", ch );
        send_to_actor( NAME(ch->reply), ch );
        send_to_actor( "?\n\r", ch );
        } else 
        send_to_actor( "There is no one to reply to.  Use 'tell' to establish communication.\n\r", ch );
        return;
    }

    if ( ch->reply == NULL )
    {
        send_to_actor( "There is no one to reply to.\n\r", ch );
        return;
    }

    message = format_indent( str_dup(message), "   ", 74, FALSE );

    victim = ch->reply;
    if ( victim != ch )
    {
    snprintf( buf, MAX_STRING_LENGTH, "%sYou reply to $N:\n\r   \"$t\"$R",
             color_table[GET_PC(ch,colors[COLOR_COMM],7)].actor_code );
    act( buf, ch, message, victim, TO_ACTOR );
    }

    snprintf( buf, MAX_STRING_LENGTH, "%sYour thoughts unfocus as a message from\n\r$N echos in your mind:\n\r   \"$t\"$R",
             color_table[GET_PC(victim,colors[COLOR_COMM],7)].actor_code );

    position            = victim->position;
    victim->position    = POS_STANDING;
    act( buf, victim, message, ch, TO_ACTOR );
    victim->position    = position;

    ch->mana--;
    victim->mana--;
    free_string( message );
    return;
}   


/*
 * Syntax:  tell [person] [text]
 */
void cmd_tell( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    PLAYER *victim;
    char *message;
    int position;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        page_to_actor( "Syntax:    tell [target] [message]\n\r", ch );
        page_to_actor( "People told you:\n\r", ch );
        if ( NPC(ch) ) page_to_actor( PC(ch,tell_buf), ch );
        return;
    }

    if ( ( victim = get_actor_world( ch, arg ) ) == NULL  )
    {
        send_to_actor( "They aren't here.\n\r", ch );
        return;
    }

    message = format_indent( str_dup(argument), "   ", 74, FALSE );

    if ( victim != ch )
    {
    ch->reply = victim;
    victim->reply = ch;
    snprintf( buf, MAX_STRING_LENGTH, "%sYour message echoes in $N's mind:\n\r   \"$t\"$R",
             color_table[GET_PC(ch,colors[COLOR_COMM],7)].actor_code );
    act( buf, ch, message, victim, TO_ACTOR );
    }

    snprintf( buf, MAX_STRING_LENGTH, "%sA message from $N echoes in your mind:\n\r   \"$t\"$R",
             color_table[GET_PC(victim,colors[COLOR_COMM],7)].actor_code );

    if ( !NPC(victim) ) {
      strjoin(PC(victim,tell_buf), NAME(ch));
      strjoin(PC(victim,tell_buf), ": ");
      strjoin(PC(victim,tell_buf), message);
      strjoin(PC(victim,tell_buf), "\n\r" );

      snprintf( PC(victim,tell_last), MSL, "%s told you: %s\n\r", NAME(ch), message );
    }

    position            = victim->position;
    victim->position    = POS_STANDING;
    act( buf, victim, message, ch, TO_ACTOR );
    victim->position    = position;
    ch->mana--;
    victim->mana--;

    free_string(message);
    return;
}



/*
 * Syntax:  whisper [person] [text]
 */
void cmd_whisper( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    PLAYER *victim;
    char *message;
    int position;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_actor( "Whisper to whom what?\n\r", ch );
        return;
    }

    if ( ( victim = get_actor_scene( ch, arg ) ) == NULL )
    {
        send_to_actor( "They aren't here.\n\r", ch );
        return;
    }

    message = format_indent( str_dup(argument), "   ", 74, FALSE );

    if ( victim != ch )
    {
    snprintf( buf, MAX_STRING_LENGTH, "%sYou whisper to $N in %s:\n\r   \"$t\"$R",
             color_table[GET_PC(ch,colors[COLOR_COMM],7)].actor_code,
             lang_table[ch->speaking].name );
    act( buf, ch, message, victim, TO_ACTOR );
    }

    act( "$n whispers something to $N.", ch, NULL, victim, TO_SCENE );

    snprintf( buf, MAX_STRING_LENGTH, "%s$N whispers to you in %s:\n\r   \"$t\"$R",
             color_table[GET_PC(victim,colors[COLOR_COMM],7)].actor_code,
             lang_table[ch->speaking].name );

    position            = victim->position;
    victim->position    = POS_STANDING;
    act( buf, victim,
         garble_text( message,
           check_speech( victim, ch, lang_table[ch->speaking].pgsn ) ),
         ch, TO_ACTOR );
    victim->position    = position;

    free_string(message);
    return;
}



/*
 * Syntax:  wish
 *          wish [text]
 */
void cmd_wish( PLAYER *ch, char *argument )
{
    PLAYER *och;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char *message;

    one_argument( argument, arg );

    if ( IS_IMMORTAL(ch) && arg[0] == '\0' )
    {
        TOGGLE_BIT(ch->flag2, CHANNEL_WISHES);

        if ( IS_SET(ch->flag2, CHANNEL_WISHES) )
             send_to_actor( "Wishes on.\n\r", ch );
        else send_to_actor( "Wishes off.\n\r", ch );
        return;
    }

    message = format_indent( str_dup(argument), "   ", 74, FALSE );

    for ( och = actor_list;  och != NULL;  och = och->next )
    {
        if ( och->desc != NULL )
        {
            if ( ch != och )
            if ( NPC(ch)
              || NPC(och)
              || (!IS_IMMORTAL(ch) && !IS_IMMORTAL(och))
              || !IS_SET(och->flag2,CHANNEL_WISHES) )
            continue;

	            snprintf( buf, MAX_STRING_LENGTH, "%s%s wish%s:\n\r%s   %s\n\r",
        	           HAS_ANSI(och) ?
               	           color_table[PC(och,colors)
                           [IS_IMMORTAL(och) ? COLOR_IMMTALK : COLOR_COMM]].code : "",
                 	   och==ch ? "You" : NAME(ch),
                       och==ch ? ""    : "es",
                           message,
                    HAS_ANSI(och) ? ANSI_NORMAL : "" );
                    send_to_actor( buf, och );
        }
    }

    free_string( message );
    return;
}




void shout( PLAYER *ch, PLAYER *rch, char *message, int dir )
{
    char buf[MAX_STRING_LENGTH];

    if ( ch == rch )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%sYou shout, in %s:\n\r  \"$t\"$R",
                 color_table[GET_PC(ch,colors[COLOR_COMM],7)].actor_code,
                 lang_table[ch->speaking].name );
        act( buf, ch, message, NULL, TO_ACTOR );
        return;
    }

    if ( rch->in_scene != ch->in_scene )
        snprintf( buf, MAX_STRING_LENGTH, "%sYou hear %s shout in %s from %s:\n\r   \"%s\"%s\n\r",
                 HAS_ANSI(rch) ? color_table[PC(rch,colors[COLOR_COMM])].code : "",
                 (ch->sex == SEX_MALE)   ? "a man"       :
                 (ch->sex == SEX_FEMALE) ? "a woman"     : "someone",
                 learned(rch,lang_table[ch->speaking].pgsn) > 0 ?
                   lang_table[ch->speaking].name : "a strange, foreign tongue",
                 dir_rev[rev_dir[dir]],
                 garble_text( message,
                     check_speech( rch, ch, lang_table[ch->speaking].pgsn ) ),
                 HAS_ANSI(rch) ? ANSI_NORMAL : "" );
        else
        snprintf( buf, MAX_STRING_LENGTH, "%s%s shouts, in %s:\n\r   \"%s\"%s\n\r",
                 HAS_ANSI(rch) ? color_table[PC(rch,colors[COLOR_COMM])].code : "",
                 capitalize(PERS(ch, rch)),
                 learned(rch,lang_table[ch->speaking].pgsn) > 0 ?
                   lang_table[ch->speaking].name : "a strange, foreign tongue",
                 garble_text( message,
                     check_speech( rch, ch, lang_table[ch->speaking].pgsn ) ),
                 HAS_ANSI(rch) ? ANSI_NORMAL : "" );

    send_to_actor( buf, rch );

    return;
}



/*
 * Syntax:  shout [text]
 */
void cmd_shout( PLAYER *ch, char *argument )
{
    int door;
    SCENE *pScene;
    PLAYER *rch;
    char *message;

    if ( *argument == '\0' )
    {
        send_to_actor( "Shout what?\n\r", ch );
        return;
    }

    message = format_indent( str_dup(argument), "   ", 74, FALSE );

    pScene = ch->in_scene;

    for ( rch = actor_list;  rch != NULL;  rch = rch->next )
    {
        if ( (door=find_first_step(rch->in_scene, pScene, 3)) >= 0
          || rch->in_scene == pScene )
            shout( ch, rch, message, door );
    }

    free_string( message );
    return;
}


void say_to( PLAYER *ch, PLAYER *victim, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char *verb;
    char *message;

    if ( argument[0] == '\0' )
    {
        send_to_actor( "Say what?\n\r", ch );
        return;
    }

    switch ( argument[strlen(argument)-1] )
    {
        default: verb = "say";     break;
       case '!': verb = "exclaim"; break;
       case '?': verb = "ask";     break;
    }

    message = format_indent( str_dup(argument), "   ", 74, FALSE );

    snprintf( buf, MAX_STRING_LENGTH, "%sYou %s to %s in %s:\n\r  \"$t\"%s",
             color_table[GET_PC(victim,colors[COLOR_COMM],7)].actor_code,
             verb,
             NAME(victim),
             lang_table[ch->speaking].name,
             HAS_ANSI(ch) ? ANSI_NORMAL : "" );
    act( buf, ch, message, NULL, TO_ACTOR  );

    snprintf( buf, MAX_STRING_LENGTH, "%s%s %ss %syou in %s:\n\r   \"$t\"%s",
                  color_table[GET_PC(victim,colors[COLOR_COMM],7)].actor_code,
                  capitalize(PERS(ch, victim)),
                  verb,
                  verb[0] != 'a' ? "to " : "",
                  learned(victim,lang_table[ch->speaking].pgsn) > 0 ?
                   lang_table[ch->speaking].name : "a strange, foreign tongue",
                  HAS_ANSI(victim) ? ANSI_NORMAL : "" );
                      
    if ( victim != ch && victim->desc != NULL )
    act( buf, victim,
         garble_text( message,
           check_speech( victim, ch, lang_table[ch->speaking].pgsn ) ),
         NULL, TO_ACTOR );

    script_update( victim, TYPE_ACTOR, TRIG_SAY, ch, NULL, argument, NULL );
    free_string( message );
    return;
}





/*
 * Syntax:  say [text]
 */
void cmd_say( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    PLAYER *actor;
    char *verb;
    char *message;

    if ( argument[0] == '\0' )
    {
        page_to_actor( "Things said near you:\n\r", ch );
        if ( NPC(ch) ) page_to_actor( PC(ch,say_buf), ch );
        return;
    }
    /*
     * Guest character support.
     * Force guests to speak OOC.
     */
    if ( !NPC(ch) && !str_cmp( ch->name, "guest" ) )
    {
        cmd_ooc( ch, argument );
        return;
    }

    switch ( argument[strlen(argument)-1] )
    {
        default: verb = "say";     break;
       case '!': verb = "exclaim"; break;
       case '?': verb = "ask";     break;
    }

    message = format_indent( str_dup(argument), "   ", 74, FALSE );

    snprintf( buf, MAX_STRING_LENGTH, "%sYou %s, in %s:\n\r  \"$t\"%s",
             color_table[GET_PC(ch,colors[COLOR_COMM],7)].actor_code,
             verb,
             lang_table[ch->speaking].name,
             HAS_ANSI(ch) ? ANSI_NORMAL : "" );
    act( buf, ch, message, NULL, TO_ACTOR  );

    for ( actor = ch->in_scene->people;  actor != NULL;  actor = actor->next_in_scene )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%s%s %ss, in %s:\n\r   \"$t\"%s",
                      color_table[GET_PC(actor,colors[COLOR_COMM],7)].actor_code,
                      capitalize(PERS(ch, actor)),
                      verb,
                 learned(actor,lang_table[ch->speaking].pgsn) > 0 ?
                   lang_table[ch->speaking].name : "a strange, foreign tongue",
                      HAS_ANSI(actor) ? ANSI_NORMAL : "" );
                      
        if ( actor != ch && actor->desc != NULL )
            act( buf, actor,
                 garble_text( message,
                  check_speech( actor, ch, lang_table[ch->speaking].pgsn ) ),
                 NULL, TO_ACTOR );

        if ( NPC(ch) && NPC(actor)
          && actor->pIndexData->dbkey == ch->pIndexData->dbkey ) 
            continue;

    if ( !NPC(actor) ) {
      strjoin(PC(actor,say_buf), NAME(ch));
      strjoin(PC(actor,say_buf), ": ");
      strjoin(PC(actor,say_buf),
                 garble_text( message,
                  check_speech( actor, ch, lang_table[ch->speaking].pgsn ) ) );
      strjoin(PC(actor,say_buf), "\n\r" );
      snprintf( PC(actor,say_last), MSL, "%s said: %s\n\r", NAME(ch), message );

    }

        if ( ch != actor )
        script_update( actor, TYPE_ACTOR, TRIG_SAY, ch, NULL, argument, NULL );
    }

    free_string( message );

    script_update( ch->in_scene, TYPE_SCENE, TRIG_SAY, ch, NULL, argument, NULL );
    trigger_list( ch->in_scene->contents, TRIG_SAY, ch, NULL, argument, NULL );
    trigger_list( ch->carrying, TRIG_SAY, ch, NULL, argument, NULL );
    return;
}

/*
 * Syntax:  talk [text]
 * Only to those sitting at your furniture.
 */
void cmd_talk( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    PLAYER *actor;
    char *verb;
    char *message;

    if ( argument[0] == '\0' )
    {
        send_to_actor( "Talk by saying what?\n\r", ch );
        return;
    }
    /*
     * Guest character support.
     * Force guests to speak OOC.
     */
    if ( !NPC(ch) && !str_cmp( ch->name, "guest" ) )
    {
        cmd_ooc( ch, argument );
        return;
    }

    if ( ch->furniture == NULL )
    {
        send_to_actor( "You aren't sitting at or on anything.\n\r", ch );
        cmd_say( ch, argument );
        return;
    }

    switch ( argument[strlen(argument)-1] )
    {
        default: verb = "say";     break;
       case '!': verb = "exclaim"; break;
       case '?': verb = "ask";     break;
    }

    message = format_indent( str_dup(argument), "   ", 74, FALSE );

    snprintf( buf, MAX_STRING_LENGTH, "%sFrom %s, you %s, in %s:\n\r  \"$t\"%s",
             color_table[GET_PC(ch,colors[COLOR_COMM],7)].actor_code,
             STR(ch->furniture, short_descr),
             verb,
             lang_table[ch->speaking].name,
             HAS_ANSI(ch) ? ANSI_NORMAL : "" );
    act( buf, ch, message, NULL, TO_ACTOR  );

    for ( actor = ch->in_scene->people;  actor != NULL;  actor = actor->next_in_scene )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%sFrom %s, %s %ss, in %s:\n\r   \"$t\"%s",
                      STR(ch->furniture, short_descr),
                      color_table[GET_PC(actor,colors[COLOR_COMM],7)].actor_code,
                      capitalize(PERS(ch, actor)),
                      verb,
                 learned(actor,lang_table[ch->speaking].pgsn) > 0 ?
                   lang_table[ch->speaking].name : "a strange, foreign tongue",
                      HAS_ANSI(actor) ? ANSI_NORMAL : "" );
                      
        if ( actor != ch && actor->desc != NULL && actor->furniture == ch->furniture )
            act( buf, actor,
                 garble_text( message,
                  check_speech( actor, ch, lang_table[ch->speaking].pgsn ) ),
                 NULL, TO_ACTOR );

        if ( NPC(ch) && NPC(actor)
          && actor->pIndexData->dbkey == ch->pIndexData->dbkey ) 
            continue;

        script_update( actor, TYPE_ACTOR, TRIG_SAY, ch, NULL, argument, NULL );
    }

    free_string( message );

    script_update( ch->in_scene, TYPE_SCENE, TRIG_SAY, ch, NULL, argument, NULL );
    trigger_list( ch->in_scene->contents, TRIG_SAY, ch, NULL, argument, NULL );
    trigger_list( ch->carrying, TRIG_SAY, ch, NULL, argument, NULL );
    return;
}

/*
 * Syntax:  say [text]
 */
void cmd_ooc( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    PLAYER *actor;
    char *verb;
    char *message;

    if ( argument[0] == '\0' )
    {
        page_to_actor( "Out-of-character comments recently:\n\r", ch );
        if ( NPC(ch) ) page_to_actor( PC(ch,ooc_buf), ch );
        return;
    }

    switch ( argument[strlen(argument)-1] )
    {
        default: verb = "say";     break;
       case '!': verb = "exclaim"; break;
       case '?': verb = "ask";     break;
    }

    message = format_indent( str_dup(argument), "   ", 74, FALSE );

    snprintf( buf, MAX_STRING_LENGTH, "%sYou %s, out-of-character:\n\r  \"$t\"%s",
             color_table[GET_PC(ch,colors[COLOR_COMM],7)].actor_code,
             verb,
             HAS_ANSI(ch) ? ANSI_NORMAL : "" );
    act( buf, ch, message, NULL, TO_ACTOR  );

    for ( actor = ch->in_scene->people;  actor != NULL;  actor = actor->next_in_scene )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%s%s %ss, out-of-character:\n\r   \"$t\"%s",
                      color_table[GET_PC(actor,colors[COLOR_COMM],7)].actor_code,
                      capitalize(PERS(ch, actor)),
                      verb,
                      HAS_ANSI(actor) ? ANSI_NORMAL : "" );
                      
        if ( actor != ch && actor->desc != NULL )
            act( buf, actor, message, NULL, TO_ACTOR );

        if ( NPC(ch) && NPC(actor)
          && actor->pIndexData->dbkey == ch->pIndexData->dbkey ) 
            continue;

    if ( !NPC(actor) ) {
      if ( strlen(PC(actor,ooc_buf)) > MSL - 1024 ) PC(actor,ooc_buf)[0]='\0';
      strcat(PC(actor,ooc_buf), NAME(ch));
      strcat(PC(actor,ooc_buf), ": ");
      strcat(PC(actor,ooc_buf), message );
      strcat(PC(actor,ooc_buf), "\n\r" );
    }

    }

    free_string( message );
    return;
}



/*
 * Syntax: speak [language]
 */
void cmd_speak( PLAYER *ch, char *argument )
{
    int x;

    if ( MTD(argument) )
    {
        send_to_actor( "You know of the following languages:\n\r", ch );
        for ( x = 0;  x < MAX_LANGUAGE; x++ )
        {
              if ( learned(ch,lang_table[x].pgsn) > 0 )
{              send_to_actor( lang_table[x].name, ch );
              send_to_actor( "\n\r", ch );}
        }
        return;
    }

    for ( x = 0; x < MAX_LANGUAGE; x++ )
    {
        if ( !str_prefix( argument, lang_table[x].name )
          && (NPC(ch) || learned(ch,lang_table[x].pgsn) > 0) )
        {
            ch->speaking = x;
            send_to_actor( "You now speak ", ch );
            send_to_actor( lang_table[x].name, ch );
            send_to_actor( ".\n\r", ch );
            return;
        }
    }

    send_to_actor( "You know not of that language.\n\r", ch );
    return;
}


/*
 * Syntax: emote [action]
 */
void cmd_emote( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;

    if ( !NPC(ch) && IS_SET(ch->flag2, PLR_NO_EMOTE) )
    {
        send_to_actor( "It is impossible to show your emotions.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_actor( "Emote what?\n\r", ch );
        return;
    }

    for ( plast = argument; *plast != '\0'; plast++ );

    strcpy( buf, argument );
    if ( isalpha(plast[-1]) )
        strcat( buf, "." );


    plast = format_indent( str_dup(buf), "", 76-strlen(NAME(ch)), FALSE );
    plast[0] = buf[0];

    act( "$n $T", ch, NULL, plast, TO_SCENE );

    if (ch->desc != NULL)
    act( "$t $T", ch, capitalize(NAME(ch)), plast, TO_ACTOR );

    free_string( plast );
    return;
}


/*
 * Syntax:  bug [report]
 */
void cmd_bug( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    snprintf( buf, MAX_STRING_LENGTH, "Notify> BUG: [%5d] %s- %s",
             ch->in_scene->dbkey, STR(ch,name), argument );
    NOTIFY( buf, LEVEL_IMMORTAL, WIZ_NOTIFY_BUG );

    append_file( ch, BUG_FILE, argument );
    send_to_actor( "Ok.  Thanks.\n\r", ch );
    return;
}


/*
 * Syntax:  idea [suggestion]
 */
void cmd_idea( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    snprintf( buf, MAX_STRING_LENGTH, "Notify> IDEA: [%5d] %s- %s",
             ch->in_scene->dbkey, STR(ch,name), argument );
    NOTIFY( buf, LEVEL_IMMORTAL, WIZ_NOTIFY_BUG );

    append_file( ch, IDEA_FILE, argument );
    send_to_actor( "Ok.  Thanks.\n\r", ch );
    return;
}


/*
 * Syntax:  typo [error]
 */
void cmd_typo( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    snprintf( buf, MAX_STRING_LENGTH, "Notify> TYPO: [%5d] %s- %s",
             ch->in_scene->dbkey, STR(ch,name), argument );
    NOTIFY( buf, LEVEL_IMMORTAL, WIZ_NOTIFY_BUG );

    append_file( ch, TYPO_FILE, argument );
    send_to_actor( "Ok.  Thanks.\n\r", ch );
    return;
}



void cmd_qui( PLAYER *ch, char *argument )
{
    send_to_actor( "If you want to QUIT, you have to spell it out.\n\r", ch );
    return;
}


/*
 * Syntax:  quit
 */
void cmd_quit( PLAYER *ch, char *argument )
{
    CONNECTION *d;
    char buf[MAX_STRING_LENGTH];

    if ( NPC(ch) )
    {
        cmd_return( ch, "" );
        return;
    }

    if ( ch->position == POS_FIGHTING )
    {
        send_to_actor( "No way! You are fighting.\n\r", ch );
        return;
    }

    if ( ch->position  < POS_STUNNED  )
    {
        send_to_actor( "You're not DEAD yet.\n\r", ch );
        return;
    }

    if ( !NPC(ch) )
        cmd_help( ch, "FAREWELL" );

    if ( IS_IMMORTAL( ch ) )
    {
        cmd_memory( ch, "" );
        send_to_actor( "Closing connection . . .\n\r", ch );
    }

    stop_idling( ch );

    sprintf( log_buf, "%s (%s) has left the game.", ch->name, ch->short_descr );
    log_string( log_buf );

    snprintf( buf, MAX_STRING_LENGTH, "Notify>  %s (%s) has left the game.", ch->name, ch->short_descr );
    NOTIFY( buf, LEVEL_IMMORTAL, WIZ_NOTIFY_LOGIN );

    act( "$n has left the game.", ch, NULL, NULL, TO_NOTVICT );

    snprintf( buf, MAX_STRING_LENGTH, "%s", STR(ch,name) );
    save_actor_prop( ch );
    d = ch->desc;

    extractor_char( ch, TRUE );

    /*
     * After extractor_char the ch is no longer valid!
     */
    if ( d != NULL ) close_socket( d );
    return;
}



/*
 * Syntax:  save
 */
void cmd_save( PLAYER *ch, char *argument )
{
    if ( NPC(ch) )
        return;
    save_copyover();
    save_contents();
    save_actor_prop( ch );
    if ( str_cmp( argument, "internal" ) ) {
      send_to_actor( "Saved ", ch );
      send_to_actor( NAME(ch), ch );
      send_to_actor( ".\n\r", ch );
      send_to_actor( "By the way, every time you type anything into the mud,\n\r", ch );
      send_to_actor( "your character file is saved.\n\r", ch );
     }
    return;
}


/*
 * Syntax:  follow [person]
 *          follow self
 */
void cmd_follow( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PLAYER *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_actor( "Follow whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_actor_scene( ch, arg ) ) == NULL )
    {
        send_to_actor( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED(ch, BONUS_CHARM) && ch->master != NULL )
    {
        act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_ACTOR );
        return;
    }

    if ( ch->master == victim ) {
       send_to_actor( "You are already following them.\n\r", ch );
       return;
    }

    if ( victim == ch )
    {
        if ( ch->master == NULL )
        {
            send_to_actor( "You already follow yourself.\n\r", ch );
            return;
        }
        stop_follower( ch );
        return;
    }
    
    if ( ch->master != NULL )
        stop_follower( ch );

    add_follower( ch, victim );
    return;
}



void add_follower( PLAYER *ch, PLAYER *master )
{
    if ( ch->master != NULL )
    {
        bug( "Add_follower: non-null master (already following).", 0 );
        return;
    }

    ch->master        = master;
    if ( (!NPC(ch)      && ch->desc      && !CONNECTED(ch->desc))
      || (!NPC(master)  && master->desc  && !CONNECTED(master->desc)) )
    return;

    if ( can_see( master, ch ) )
    act( "$n now follows you.", ch, NULL, master, TO_VICT );

    act( "You now follow $N.",  ch, NULL, master, TO_ACTOR );

    return;
}


bool in_group( PLAYER *ch, PLAYER *victim )
{
    if ( ch == NULL || victim == NULL ) return FALSE;

    if ( ch->leader == NULL 
      && victim->leader == NULL ) return FALSE;

    if ( ch->leader == victim->leader ) return TRUE;

    if ( victim->leader == ch ) return TRUE;

    if ( ch->leader == victim ) return TRUE;

    return FALSE;
}


void ungroup( PLAYER *ch )
{
    PLAYER *victim;
    PLAYER *newleader=NULL;

    if ( ch->leader == NULL ) {
       for ( victim = actor_list;  victim != NULL;  victim = victim->next )
       {
           if ( victim->leader == ch )
           { if ( !newleader ) newleader=victim;

        if ( !NPC(victim) )
        display_interp( victim, color_table[PC(victim,colors[COLOR_GROUPS])].di );

              act( "$N has parted company with the group.", victim, NULL, ch, TO_ACTOR );
        display_interp( victim, "^N" );
           if ( newleader != victim ) victim->leader = ch;
           else victim->leader = NULL;
           }
       }

        if ( !NPC(ch) )
        display_interp( ch, color_table[PC(ch,colors[COLOR_GROUPS])].di );
        act( "You left the group.", ch, NULL, NULL, TO_ACTOR );
        display_interp( ch, "^N" );
    }

    if ( ch->leader != NULL ) {
        victim = ch->leader;

        if ( !NPC(ch) )
        display_interp( ch, color_table[PC(ch,colors[COLOR_GROUPS])].di );
        if ( !NPC(victim) )
        display_interp( victim, color_table[PC(victim,colors[COLOR_GROUPS])].di );

              act( "$N has parted company with the group.", victim, NULL, ch, TO_ACTOR );
              act( "You left $N's group.", ch, NULL, victim, TO_ACTOR );
        display_interp( ch, "^N" );
        display_interp( victim, "^N" );

       victim->leader = NULL;
       ch->leader = NULL;
    }

    return;
}



void stop_follower( PLAYER *ch )
{
    if ( ch->master == NULL )
    {
        bug( "Stop_follower: null master.", 0 );
        return;
    }

    if ( IS_AFFECTED(ch, BONUS_CHARM) )
    {
        REMOVE_BIT( ch->bonuses, BONUS_CHARM );
        bonus_strip( ch, skill_dbkey(skill_lookup("charm")) );
    }

    if ( can_see( ch->master, ch ) )
    act( "$n stops following you.",     ch, NULL, ch->master, TO_VICT    );
    act( "You stop following $N.",      ch, NULL, ch->master, TO_ACTOR    );

    ungroup( ch );
    ch->master = NULL;
    return;
}



void die_follower( PLAYER *ch )
{
    PLAYER *fch;

    if ( ch->master != NULL )
        stop_follower( ch );

    for ( fch = actor_list; fch != NULL; fch = fch->next )
    {
        if ( fch->master == ch )
            stop_follower( fch );
    }

    return;
}



/*
 * Syntax:  order [person] [action]
 */
void cmd_order( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg_cmd[MAX_INPUT_LENGTH];
    PLAYER *victim;
    PLAYER *och;
    PLAYER *och_next;
    bool found;
    bool fAll;

    argument = one_argument( argument, arg );
    one_argument( argument, arg_cmd );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_actor( "Order whom to do what?\n\r", ch );
        return;
    }

    if ( IS_AFFECTED( ch, BONUS_CHARM ) )
    {
        send_to_actor( "You feel like taking, not giving, orders.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        fAll   = TRUE;
        victim = NULL;
    }
    else
    {
        fAll   = FALSE;
        if ( ( victim = get_actor_scene( ch, arg ) ) == NULL )
        {
            send_to_actor( "They aren't here.\n\r", ch );
            return;
        }

        if ( victim == ch )
        {
            send_to_actor( "Aye aye, right away!\n\r", ch );
            return;
        }

        if ( ( !IS_AFFECTED(victim, BONUS_CHARM) && !IS_SET(victim->flag,ACTOR_PET) )
           || victim->master != ch )
        {
            send_to_actor( "Do it yourself!\n\r", ch );
            return;
        }
    }

    if ( str_prefix( arg_cmd, "get"    )
      && str_prefix( arg_cmd, "drop"   )
      && str_prefix( arg_cmd, "wear"   )
      && str_prefix( arg_cmd, "sheath" )
      && str_prefix( arg_cmd, "assist" )
      && str_prefix( arg_cmd, "swap"   )
      && str_prefix( arg_cmd, "dump"   )
      && str_prefix( arg_cmd, "eat"    )
      && str_prefix( arg_cmd, "quaff"  )
      && str_prefix( arg_cmd, "zap"    )
      && str_prefix( arg_cmd, "smote"    )
      && str_prefix( arg_cmd, "give"   )
      && str_prefix( arg_cmd, "recite" )
      && str_prefix( arg_cmd, "sit"    )
      && str_prefix( arg_cmd, "shoot"  )
      && str_prefix( arg_cmd, "tactic" )
      && str_prefix( arg_cmd, "report" )
      && str_prefix( arg_cmd, "light"  )
      && str_prefix( arg_cmd, "extinguish"  )
      && str_prefix( arg_cmd, "flee"   )
      && str_prefix( arg_cmd, "north"  )
      && str_prefix( arg_cmd, "south"  )
      && str_prefix( arg_cmd, "east"   )
      && str_prefix( arg_cmd, "west"   )
      && str_prefix( arg_cmd, "up"     )
      && str_prefix( arg_cmd, "ride"   )
      && str_prefix( arg_cmd, "mount"  )
      && str_prefix( arg_cmd, "down"   )
      && str_prefix( arg_cmd, "northeast"  )
      && str_prefix( arg_cmd, "southeast"  )
      && str_prefix( arg_cmd, "northwest"  )
      && str_prefix( arg_cmd, "southwest"  )
      && str_prefix( arg_cmd, "track"  )
      && str_prefix( arg_cmd, "pick"   )
      && str_prefix( arg_cmd, "steal"  )
      && str_prefix( arg_cmd, "sneak"  )
      && str_prefix( arg_cmd, "hide"   )
      && str_prefix( arg_cmd, "pour"   )
      && str_prefix( arg_cmd, "rest"   )
      && str_prefix( arg_cmd, "stand"  )
      && str_prefix( arg_cmd, "sleep"  )
      && str_prefix( arg_cmd, "group"  )
      && str_prefix( arg_cmd, "exit"   )
      && str_prefix( arg_cmd, "enter"  )
      && str_prefix( arg_cmd, "drink"  )
      && str_prefix( arg_cmd, "give"   )
      && str_prefix( arg_cmd, "put"    )
      && str_prefix( arg_cmd, "cast"   )
      && str_prefix( arg_cmd, "draw"   )
      && str_prefix( arg_cmd, "stand"  )
      && str_prefix( arg_cmd, "remove" )
      && str_prefix( arg_cmd, "wield"  )
      && str_prefix( arg_cmd, "hold"   ) )
    {
        send_to_actor( "It is impossible to order them to do that.\n\r", ch );
        return;
    }

    found = FALSE;
    for ( och = ch->in_scene->people; och != NULL; och = och_next )
    {
        och_next = och->next_in_scene;

        if ( ( IS_AFFECTED(och, BONUS_CHARM) || ( NPC(och) && IS_SET(och->flag,ACTOR_PET) ) )
        &&   och->master == ch
        && ( fAll || och == victim ) )
        {
            found = TRUE;
            act( "$n orders you to '$t'.", ch, argument, och, TO_VICT );
            interpret( och, argument );
        }
    }

    if ( found )
    send_to_actor( "Ok.\n\r", ch );
    else
    send_to_actor( "You have no followers here.\n\r", ch );
    return;
}


/*
 * Syntax: monitor [target]
 *         monitor
 */
void cmd_monitor( PLAYER *ch, char *argument )
{
   char arg[MAX_STRING_LENGTH];
   PLAYER *victim;

   one_argument( argument, arg );

   if ( NPC(ch) ) return;

   display_interp( ch, color_table[PC(ch,colors[COLOR_GROUPS])].di );

   if ( arg[0] == '\0' ) {
       if ( ch->monitor == NULL ) {
           send_to_actor( "You aren't monitoring anybody.\n\r", ch );
           display_interp( ch, "^N" );
           return;
       }
       else {
           victim = ch->monitor;
           ch->monitor = NULL;
         if ( !NPC(victim) )
        display_interp( victim, color_table[PC(victim,colors[COLOR_GROUPS])].di );
           act( "You stop monitoring $n.", ch, NULL, victim, TO_ACTOR );
           act( "$N is no longer monitoring you.", victim, NULL, ch, TO_ACTOR );
           display_interp( ch, "^N" );
           display_interp( victim, "^N" );
           return;
       }
   }

   victim = get_actor_scene( ch, arg );
   if ( victim == ch ) {
       send_to_actor( "You already monitor yourself.\n\r", ch );
       display_interp( ch, "^N" );
       return;
   }
   if ( victim == NULL )
   {
       send_to_actor( "Monitor who?\n\r", ch );
       display_interp( ch, "^N" );
       return;
   }
   if ( !in_group( ch, victim ) ) {
            send_to_actor( "You aren't grouped with them.\n\r", ch );
            display_interp( ch, "^N" );
            return;
        }
  

   if ( ch->monitor ) cmd_monitor( ch, "" );

   if ( !NPC(victim) ) {
   display_interp( victim, color_table[PC(victim,colors[COLOR_GROUPS])].di );
   act( "You begin monitoring $n.", ch, NULL, victim, TO_ACTOR );
   act( "$N is now monitoring your status.", victim, NULL, ch, TO_ACTOR );
   ch->monitor = victim;

   display_interp( ch, "^N" );
   display_interp( victim, "^N" );
   }
   return;
}



void cmd_gtell( PLAYER *ch, char *argument )
{
   PLAYER *victim;

   if ( *argument == '\0' )
   {
        send_to_actor( "Gtell what?\n\r", ch );
        return;
   }

   if ( NPC(ch) ) return;

   display_interp( ch, color_table[PC(ch,colors[COLOR_GROUPS])].di );

   act( "You tell the group:\n\r   \"$t\"",
                   ch, argument, NULL, TO_ACTOR );

   display_interp( ch, "^N" );

   for ( victim = actor_list;  victim != NULL;  victim = victim->next )
   {
      if ( in_group(ch, victim) && victim != ch )
      {

      if ( !NPC(victim) )
      display_interp( victim, color_table[PC(victim,colors[COLOR_GROUPS])].di );

      act( "$N tells the group:\n\r   \"$t\"", victim, argument, ch, TO_ACTOR );
      display_interp( victim, "^N" );

      }
   }

   return;
}


/*
 * Syntax:  group [person]
 *          group self
 *          group
 */
void cmd_group( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    PLAYER *victim;

    if ( NPC(ch) ) return;

    display_interp( ch, color_table[PC(ch,colors[COLOR_GROUPS])].di );

    one_argument( argument, arg );

    if ( !str_cmp( arg, "disband" )
      || !str_cmp( arg, "me" )
      || !str_cmp( arg, "self" ) ) 
    {  ungroup( ch );  return; }

    if ( arg[0] == '\0' )
    {
        bool fGroup = FALSE;

        for ( victim = actor_list;  victim != NULL;  victim = victim->next )
        {
            if ( NPC(victim) ) continue;

            if ( in_group( ch, victim ) )
            {
               snprintf( buf, MAX_STRING_LENGTH, "%12s:  %-10s | %-10s | %-10s | %s %s\n\r",
                        NAME(victim),
                        STRING_HITS(victim), 
                        STRING_MOVES(victim),
                        STRING_MANA(victim),
                        ( victim->riding != NULL ) ? "mounted" : "",
                        ""  );
               strcat( arg, buf );
               fGroup = TRUE;
            }
        }

        if ( fGroup ) {
            if ( ch->leader == NULL ) send_to_actor( "Your group:\n\r\n\r", ch );
            else act( "$n's group:\n\r", ch->leader, NULL, ch, TO_VICT );
            send_to_actor( arg, ch );
        }
        else send_to_actor( "You are grouped with no one.\n\r", ch );
        display_interp( ch, "^N" );
        return;
    }

    if ( ( victim = get_actor_scene( ch, arg ) ) == NULL )
    {
        send_to_actor( "They aren't here.\n\r", ch );
        display_interp( ch, "^N" );
        return;
    }

    if ( NPC(victim) ) 
    {
        act( "$N doesn't need to be grouped.  Grouping is for human players.", ch, NULL, victim, TO_ACTOR );
        display_interp( ch, "^N" );
        return;
    }

    if ( victim->master != ch  )
    {
        act( "$N isn't following you.", ch, NULL, victim, TO_ACTOR );
        display_interp( ch, "^N" );
        return;
    }

    if ( victim->leader == ch ) {
        /* already grouped */
        act( "$N is already in your group.", ch, NULL, victim, TO_ACTOR );
        display_interp( ch, "^N" );
        return;
    }

    if ( victim != ch )
    {
        if ( ch->leader != NULL )
        {
            act( "You are already in $N's group.\n\r", ch, NULL, ch->leader, TO_ACTOR );
            display_interp( ch, "^N" );
            return;
        }

        victim->leader = ch;

        if ( !NPC(victim) )
        display_interp( victim, color_table[PC(victim,colors[COLOR_GROUPS])].di );

        act( "You join $N's group.", victim, NULL, ch, TO_ACTOR );
        act( "$N joins your group.", ch, NULL, victim, TO_ACTOR );

        snprintf( buf, MAX_STRING_LENGTH, "%s begins adventuring with a group lead by %s.",
                NAME(victim), NAME(ch) );
        add_history( victim, buf );

        snprintf( buf, MAX_STRING_LENGTH, "%s takes %s under %s wing.",
                NAME(ch), NAME(victim), HIS_HER(ch) );
        add_history( ch, buf );

        display_interp( ch, "^N" );
        display_interp( victim, "^N" );
        return;
    }
    
    ungroup( ch );
    return;
}


/*
 * Dismisses a follower.
 */
void cmd_dismiss( PLAYER *ch, char *argument ) {
    char arg[MIL];
    PLAYER *victim;

    argument = one_argument(argument,arg);

    if ( arg[0] == '\0' )
    {
        send_to_actor( "Dismiss whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_actor_scene( ch, arg ) ) == NULL )
    {
        send_to_actor( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim->master != ch || ( !IS_AFFECTED(victim, BONUS_CHARM) && !IS_SET(victim->flag,ACTOR_PET) ) )
    {
        act( "You have no control over $N!", ch, NULL, victim, TO_ACTOR );
        return;
    }

    act( "$n dismisses $N.", ch, NULL, victim, TO_SCENE );
    act( "$n dismissed you.", ch, NULL, victim, TO_VICT );
    act( "You dismissed $N.", ch, NULL, victim, TO_ACTOR );
    cmd_drop( victim, "all" );
    extractor_char( victim, TRUE );    
    return;
}
