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
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <zlib.h>

#include <unistd.h>	/* For execl in hotboot() */

#include "nimud.h"
#include "net.h"
#include "defaults.h"


/*
 * Local functions.
 */

SCENE *	find_location	args( ( PLAYER *ch, char *arg ) );



/*
 * Syntax:  advance [person] [level]
 *          advance [person] [level] [constellation]
 */
void cmd_advance( PLAYER *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char embuf[MAX_STRING_LENGTH];
    PLAYER *victim;
    int level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
        to_actor( "Syntax: advance <char> <level> [constellation].\n\r", ch );
        return;
    }

    if ( ( victim = get_actor_world( ch, arg1 ) ) == NULL )
    {
        to_actor( "That player is not here.\n\r", ch);
        return;
    }

    if ( NPC(victim) )
    {
        to_actor( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( ( level = atoi( arg2 ) ) < 1 || level > MAX_LEVEL )
    {
        snprintf( buf, MAX_STRING_LENGTH, "Level must be 1 to %d.\n\r", MAX_LEVEL );
        to_actor( buf, ch );
        return;
    }

    if ( PC(ch,level) != MAX_LEVEL )
    to_actor( "Limited to your trust level, changing constellation only.\n\r", ch );
    else
    {
        PC(victim,level)    = level;

        if ( MTD(PC(victim,constellation)) )
        PC(victim,constellation) = str_dup( "%s" );

        switch ( PC(victim,level) )
        {
            case LEVEL_NEW:
            case LEVEL_MORTAL:
             if ( IS_SET(victim->flag, PLR_APPLIED) ) {
             snprintf( buf, MAX_STRING_LENGTH, "$n $k now an unvalidated player on this mud.\n\r" );
             }
             else {
             snprintf( buf, MAX_STRING_LENGTH, "$n$y character has been validated.\n\r" ); 
             sprintf( embuf, "Dear %s,\n\r\n\rThis message is being sent from %s %s %d,\n\rletting you know your character is validated and ready for play!\n\r\n\r               Sincerely,\n\r            %s Management\n\r", victim->name, MUD_NAME, MUD_ADDRESS, port, MUD_NAME );
             EMAIL( PC(victim,email), "Your character has been validated!",
                embuf );
             }
             break;
            case LEVEL_HERO:
             snprintf( buf, MAX_STRING_LENGTH, "$n $k now an Avatar.\n\r" ); break;
            case MAX_LEVEL:
             snprintf( buf, MAX_STRING_LENGTH, "$n $k now the highest level available on this mud.\n\r" ); break;
            default:
             if (IS_IMMORTAL(victim)) snprintf( buf, MAX_STRING_LENGTH, "$n $k now immortal.\n\r" );
                                 else snprintf( buf, MAX_STRING_LENGTH, "$n $k raised a level.\n\r" );
            break;
        }

        act( buf, victim, NULL, ch, TO_VICT );
        act( buf, victim, NULL, ch, TO_ACTOR );
        if ( IS_IMMORTAL(victim) ) {
             add_history(victim, "Immortalized in the book of Saints.\n\r");
             sprintf( embuf, "Dear %s,\n\r\n\rThis message is being sent to inform you that your character\n\ris now an immortal on %s at %s %d.\n\r\n\r              Congratulations, %s!\n\r\n\r                 %s Management\n\r", victim->name, MUD_NAME, MUD_ADDRESS, port, victim->name, MUD_NAME );
             EMAIL( PC(victim,email), "Your character is now immortal!",
              embuf );
        }

        if ( !IS_SET(ch->flag2, CHANNEL_IMMTALK) && IS_IMMORTAL(ch) )
        cmd_immtalk( ch, "" );
    }


    if ( !IS_IMMORTAL(victim) )
    {
        free_string( PC(victim, constellation) );
        PC(victim,constellation) = NULL;
    }
    else
    if ( *argument != '\0' )
    {
        free_string( PC(victim, constellation) );
        PC(victim,constellation) = str_dup( argument );
    }

    return;
}




/*
 * Syntax:  restore [person]
 */
void cmd_restore( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PLAYER *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        to_actor( "Restore whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_actor_world( ch, arg ) ) == NULL )
    {
        to_actor( "They aren't here.\n\r", ch );
        return;
    }

    victim->hit  = MAXHIT(victim);
    victim->move = MAXMOVE(victim);
    victim->mana = MAXMANA(victim);
    update_pos( victim );
    act( "You touch $S head and restore $M.", ch, NULL, victim, TO_ACTOR );
    act( "$n touches your head and you feel energized.", ch, NULL, victim, TO_VICT );
    return;
}




/*
 * Syntax:  freeze [person]
 */
void cmd_freeze( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PLAYER *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
    to_actor( "Freeze whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_actor_world( ch, arg ) ) == NULL )
    {
    to_actor( "They aren't here.\n\r", ch );
	return;
    }

    if ( NPC(victim) )
    {
    to_actor( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
    to_actor( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->flag, PLR_FREEZE) )
    {
	REMOVE_BIT(victim->flag, PLR_FREEZE);
    to_actor( "You can play again.\n\r", victim );
    to_actor( "FREEZE removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->flag, PLR_FREEZE);
    to_actor( "The gods have revoked your freedoms, you are frozen!\n\r", victim );
    to_actor( "FREEZE set.\n\r", ch );
    }

    save_actor_prop( victim );

    return;
}




/*
 * Syntax:  log [person]
 *          log list
 *          log #
 *          log all
 */
void cmd_log( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PLAYER *victim;
    extern bool fLogAll;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
    to_actor( "Log whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	if ( fLogAll )
	{
	    fLogAll = FALSE;
        to_actor( "Log ALL off.\n\r", ch );
	}
	else
	{
	    fLogAll = TRUE;
        to_actor( "Log ALL on.\n\r", ch );
	}
	return;
    }

    if ( is_number( arg ) ) {
        char fname[MAX_STRING_LENGTH];

        sprintf( fname, "cat ../log/%s.log > .shellout", arg );
        cmd_shell( ch, fname );
        return;
    }

    if ( !str_cmp( arg, "list" ) )
        {
        cmd_shell( ch, "ls ../log > .shellout" );
        return;
        }

    if ( ( victim = get_actor_world( ch, arg ) ) == NULL )
    {
    to_actor( "They aren't here.\n\r", ch );
	return;
    }

    if ( NPC(victim) )
    {
    to_actor( "Not on NPC's.\n\r", ch );
	return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->flag, PLR_LOG) )
    {
	REMOVE_BIT(victim->flag, PLR_LOG);
    to_actor( "LOG removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->flag, PLR_LOG);
    to_actor( "LOG set.\n\r", ch );
    }

    return;
}




/*
 * Syntax:  noemote [person]
 */
void cmd_noemote( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PLAYER *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
    to_actor( "Noemote whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_actor_world( ch, arg ) ) == NULL )
    {
    to_actor( "They aren't here.\n\r", ch );
	return;
    }

    if ( NPC(victim) )
    {
    to_actor( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
    to_actor( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->flag2, PLR_NO_EMOTE) )
    {
    REMOVE_BIT(victim->flag2, PLR_NO_EMOTE);
    to_actor( "You can emote again.\n\r", victim );
    to_actor( "NO_EMOTE removed.\n\r", ch );
    }
    else
    {
    SET_BIT(victim->flag2, PLR_NO_EMOTE);
    to_actor( "You are no longer allowed to emote!\n\r", victim );
    to_actor( "NO_EMOTE set.\n\r", ch );
    }

    return;
}




/*
 * Syntax:  peace
 */
void cmd_peace( PLAYER *ch, char *argument )
{
    PLAYER *rch;

    for ( rch = ch->in_scene->people; rch != NULL; rch = rch->next_in_scene )
    {
        if ( rch->fighting != NULL )
        {
            if ( !str_cmp( argument, "silent" ) )
            to_actor( "You stop fighting.\n\r", ch );
            else
            {
                act( "$n waves a white flag.", rch, 0, 0, TO_SCENE );
                to_actor( "You wave a white flag.\n\r", rch );
            }

            rch->fighting = NULL;
            rch->position = POS_STANDING;
            update_pos( rch );
        }
    }

    return;
}



BAN *		ban_free;
BAN *		ban_list;


/*
 * Syntax:  ban [site]
 */
void cmd_ban( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    BAN *pban;

    if ( NPC(ch) )
	return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
    strcpy( buf, "Banned sites:\n\r" );
	for ( pban = ban_list; pban != NULL; pban = pban->next )
	{
	    strcat( buf, pban->name );
        strcat( buf, "\n\r" );
	}
	to_actor( buf, ch );
	return;
    }

    for ( pban = ban_list; pban != NULL; pban = pban->next )
    {
	if ( !str_cmp( arg, pban->name ) )
	{
        to_actor( "That site is already banned!\n\r", ch );
	    return;
	}
    }

    if ( ban_free == NULL )
    {
	pban		= alloc_perm( sizeof(*pban) );
    }
    else
    {
	pban		= ban_free;
	ban_free	= ban_free->next;
    }

    pban->name	= str_dup( arg );
    pban->next	= ban_list;
    ban_list	= pban;
    to_actor( "Ok.\n\r", ch );
    return;
}




/*
 * Syntax:  allow [site]
 */
void cmd_allow( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    BAN *prev;
    BAN *curr;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
    to_actor( "Remove which site from the ban list?\n\r", ch );
	return;
    }

    prev = NULL;
    for ( curr = ban_list; curr != NULL; prev = curr, curr = curr->next )
    {
	if ( !str_cmp( arg, curr->name ) )
	{
	    if ( prev == NULL )
		ban_list   = ban_list->next;
	    else
		prev->next = curr->next;

	    free_string( curr->name );
	    curr->next	= ban_free;
	    ban_free	= curr;
        to_actor( "Ok.\n\r", ch );
	    return;
	}
    }

    to_actor( "Site is not banned.\n\r", ch );
    return;
}




/*
 * Syntax:  wizlock
 */
void cmd_wizlock( PLAYER *ch, char *argument )
{
    extern bool wizlock;
    wizlock = !wizlock;

    if ( wizlock )
    to_actor( "Game wizlocked.\n\r", ch );
    else
    to_actor( "Game un-wizlocked.\n\r", ch );

    return;
}


/*
 * Thanks to Grodyn for pointing out bugs in this function.
 * Syntax:  force [person] [action]
 *          force all [action]
 */
void cmd_force( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
    to_actor( "Force whom to do what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	PLAYER *vch;
	PLAYER *vch_next;

	for ( vch = actor_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( !NPC(vch) && get_trust( vch ) < get_trust( ch ) )
	    {
        act( "$n forces everyone to '$t'.", ch, argument, vch, TO_VICT );
        if (!NPC(ch))
          FORCE_LEVEL = get_trust( ch );
          else
          FORCE_LEVEL = MAX_LEVEL;
		interpret( vch, argument );
	    }
	}
    }
    else
    {
	PLAYER *victim;

	if ( ( victim = get_actor_world( ch, arg ) ) == NULL )
	{
        to_actor( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
        to_actor( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if ( !NPC(ch) && get_trust( victim ) >= get_trust( ch ) )
	{
        to_actor( "Do it yourself!\n\r", ch );
	    return;
	}

    act( "$n types '$t' for you.", ch, argument, victim, TO_VICT );
    if (!NPC(ch))
      FORCE_LEVEL = get_trust( ch );
      else
      FORCE_LEVEL = MAX_LEVEL;

	interpret( victim, argument );
    }

    to_actor( "Ok.\n\r", ch );
    return;
}



/*
 * New routines by Locke.
 * Syntax:  invis [level]
 */
void cmd_invis( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int oldinvis;

    one_argument( argument, arg );

    if ( NPC(ch) )
	return;

    oldinvis = PC(ch,wizinvis);
    if ( arg[0] == '\0' )
    {
         PC(ch,wizinvis) = PC(ch,wizinvis) == 0 ? PC(ch,level) : 0;
    }
    else PC(ch,wizinvis) = atoi( arg );


    PC(ch,wizinvis) = PC(ch,wizinvis) <= PC(ch,level)
                      ? PC(ch,wizinvis) : PC(ch,level);

    if ( PC(ch,wizinvis) <= LEVEL_IMMORTAL && PC(ch,wizinvis) != 0 )
         PC(ch,wizinvis) = 0;

    if ( PC(ch,wizinvis) == oldinvis ||
      ( PC(ch,wizinvis) >= LEVEL_IMMORTAL && oldinvis >= LEVEL_IMMORTAL ) )
        return;

    if ( PC(ch,wizinvis) < oldinvis && PC(ch,wizinvis) == 0 )
    {
    act( "$n slowly fades into existence.", ch, NULL, NULL, TO_SCENE );
    to_actor( "You slowly fade back into existence.\n\r", ch );
    }
    else
    if ( PC(ch,wizinvis) >= LEVEL_IMMORTAL && PC(ch,wizinvis) > oldinvis )
    {
    act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_SCENE );
    to_actor( "You slowly vanish into thin air.\n\r", ch );
    }

    return;
}




/*
 * Syntax:  bounty [person] [bounty]
 */
void cmd_bounty( PLAYER *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    PLAYER *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
    to_actor( "Syntax: bounty <character> <bounty>.\n\r", ch );
	return;
    }

    if ( ( victim = get_actor_world( ch, arg1 ) ) == NULL )
    {
    to_actor( "They aren't here.\n\r", ch );
	return;
    }

    if ( NPC(victim) )
    {
    to_actor( "Not on NPC's.\n\r", ch );
	return;
    }

    victim->bounty =  atoi( arg2 );

    to_actor( "Bounty set.\n\r", ch );
    return;
}



/*
 * Wizify and Wizbit sent in by M. B. King
 * Syntax:  wizify [person]
 */
void cmd_wizify( PLAYER *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    PLAYER *victim;
  
    argument = one_argument( argument, arg1  );
    if ( arg1[0] == '\0' )
    {
	to_actor( "Syntax: wizify <name>\n\r" , ch );
	return;
    }
    if ( ( victim = get_actor_world( ch, arg1 ) ) == NULL )
    {
	to_actor( "They aren't here.\n\r" , ch );
	return;
    }
    if ( NPC( victim ) )
    {
	to_actor( "Not on actors.\n\r", ch );
	return;
    }

    TOGGLE_BIT( victim->flag2, PLR_WIZBIT );
    if ( IS_SET(victim->flag2, PLR_WIZBIT ) )
    {
	act( "$N wizified.\n\r", ch, NULL, victim, TO_ACTOR );
	act( "$n has wizified you!\n\r", ch, NULL, victim, TO_VICT );
    }
    else
    {
	act( "$N dewizzed.\n\r", ch, NULL, victim, TO_ACTOR );
	act( "$n has dewizzed you!\n\r", ch, NULL, victim, TO_VICT ); 
    }

    cmd_save( victim, "internal");
    return;
}



/*
 * Syntax:  bamfin [emote]
 */
void cmd_bamfin( PLAYER *ch, char *argument )
{
    if ( !NPC(ch) )
    {
	smash_tilde( argument );
    free_string( PC(ch,bamfin) );
    PC(ch,bamfin) = str_dup( argument );
    to_actor( "Ok.\n\r", ch );
    }
    return;
}



/*
 * Syntax:  bamfout [emote]
 */
void cmd_bamfout( PLAYER *ch, char *argument )
{
    if ( !NPC(ch) )
    {
	smash_tilde( argument );
    free_string( PC(ch,bamfout) );
    PC(ch,bamfout) = str_dup( argument );
    to_actor( "Ok.\n\r", ch );
    }
    return;
}



/*
 * Syntax:  deny [person]
 */
void cmd_deny( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PLAYER *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
    to_actor( "Deny whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_actor_world( ch, arg ) ) == NULL )
    {
    to_actor( "They aren't here.\n\r", ch );
	return;
    }

    if ( NPC(victim) )
    {
    to_actor( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
    to_actor( "You failed.\n\r", ch );
	return;
    }

    SET_BIT(victim->flag, PLR_DENY);
    to_actor( "You are denied access!\n\r", victim );
    to_actor( "OK.\n\r", ch );
    cmd_quit( victim, "" );

    return;
}




/*
 * Syntax:  disconnect [person]
 */
void cmd_disconnect( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CONNECTION *d;
    PLAYER *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
    to_actor( "Disconnect whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_actor_world( ch, arg ) ) == NULL )
    {
    to_actor( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
    act( "$N doesn't have a connection.", ch, NULL, victim, TO_ACTOR );
	return;
    }

    for ( d = connection_list; d != NULL; d = d->next )
    {
	if ( d == victim->desc )
	{
	    close_socket( d );
        to_actor( "Ok.\n\r", ch );
	    return;
	}
    }

    bug( "Cmd_disconnect: desc not found.", 0 );
    to_actor( "Connection not found!\n\r", ch );
    return;
}


/*
 * Syntax:  system [message]
 */
void cmd_system( PLAYER *ch, char *argument )
{
    CONNECTION *d;
    
    if ( argument[0] == '\0' )
    {
    to_actor( "Echo what?\n\r", ch );
	return;
    }

    for ( d = connection_list; d; d = d->next )
    {
        if ( d->connected <= NET_PLAYING )
	{
        to_actor( "SYSTEM:\07 ", d->character );
	    to_actor( argument, d->character );
        to_actor( "\n\r",   d->character );
	}
    }

    return;
}


/*
 * Syntax:  echo [message]
 */
void cmd_echo( PLAYER *ch, char *argument )
{
    CONNECTION *d;
    
    if ( argument[0] == '\0' )
    {
    to_actor( "Echo what?\n\r", ch );
	return;
    }

    for ( d = connection_list; d; d = d->next )
    {
	if ( d->connected == NET_PLAYING )
	{
	    to_actor( argument, d->character );
        to_actor( "\n\r",   d->character );
	}
    }

    return;
}




/*
 * Syntax:  recho [message]
 *          recho [dbkey] [message]
 */
void cmd_recho( PLAYER *ch, char *argument )
{
    CONNECTION *d;
    char arg[MAX_INPUT_LENGTH];
    SCENE *pOldScene;
        
    if ( argument[0] == '\0' )
    {
    to_actor( "Recho what?\n\r", ch );
	return;
    }

    one_argument( argument, arg );
    if ( is_number( arg ) ){
       argument = one_argument( argument, arg );
       pOldScene = get_scene(  atoi(arg) );
       if ( pOldScene == NULL ) pOldScene = ch->in_scene;
    }
    else pOldScene = ch->in_scene;

    for ( d = connection_list; d; d = d->next )
    {
	if ( d->connected == NET_PLAYING
	&&   d->character->in_scene == pOldScene )
	{
	    to_actor( argument, d->character );
        to_actor( "\n\r",   d->character );
	}
    }

    return;
}



SCENE *find_location( PLAYER *ch, char *arg )
{
    PLAYER *victim;
    PROP *prop;
    SCENE *pScene;
    int iVnum;

    if ( is_number(arg) )
	return get_scene( atoi( arg ) );

    if ( ( victim = get_actor_world( ch, arg ) ) != NULL )
	return victim->in_scene;

    if ( ( prop = get_prop_world( ch, arg ) ) != NULL )
        return prop->in_scene;

    for ( iVnum = 0;  iVnum <= top_dbkey_scene;  iVnum++ )
    {
    if ( (pScene = get_scene( iVnum )) == NULL ) continue;
    if ( is_prename( arg, pScene->name ) ) return pScene;
    }
    
    return NULL;
}



/*
 * Syntax:  transfer [person] [place]
 */
void cmd_transfer( PLAYER *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    SCENE *location;
    CONNECTION *d;
    PLAYER *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
    to_actor( "Transfer whom (and where)?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	for ( d = connection_list; d != NULL; d = d->next )
	{
	    if ( d->connected == NET_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_scene != NULL
	    &&   can_see( ch, d->character ) )
	    {
		char buf[MAX_STRING_LENGTH];
                snprintf( buf, MAX_STRING_LENGTH, "%s %s", STR(d->character,name), arg2 );
                                       cmd_transfer( ch, buf );
	    }
	}
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_scene;
    }
    else
    {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
        to_actor( "No such location.\n\r", ch );
	    return;
	}

	if ( scene_is_private( location ) )
	{
        to_actor( "That scene is private right now.\n\r", ch );
	    return;
	}
    }

    if ( ( victim = get_actor_world( ch, arg1 ) ) == NULL )
    {
    to_actor( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->in_scene == NULL )
    {
    to_actor( "They are in limbo.  [extracting]\n\r", ch );
     }

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );
    if ( victim->in_scene != NULL )
    {
    act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_SCENE );
    actor_from_scene( victim );
    }
    actor_to_scene( victim, location );

    act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_SCENE );
    if ( ch != victim )
    act( "$n has transferred you.", ch, NULL, victim, TO_VICT );
    cmd_look( victim, "auto" );
    if ( ch->in_scene != victim->in_scene )
    to_actor( "Transferred.\n\r", ch );
}



/*
 * Syntax:  at [location/person/thing] [action]
 */
void cmd_at( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    SCENE *location;
    SCENE *original;
    PLAYER *wch;
    
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
    to_actor( "At where what?\n\r", ch );
        return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
    to_actor( "No such location.\n\r", ch );
        return;
    }

#if defined(IMMORTAL_PRIVACY)
    if ( scene_is_private( location ) )
    {
    to_actor( "That scene is private right now.\n\r", ch );
    return;
    }
#endif

    original = ch->in_scene;
    actor_from_scene( ch );
    actor_to_scene( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = actor_list; wch != NULL; wch = wch->next )
    {
        if ( wch == ch )
        {
            actor_from_scene( ch );
            actor_to_scene( ch, original );
            break;
        }
    }

    return;
}


/*
 * Syntax:  as [person] [action]
 */
void cmd_as( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PLAYER *wch;
    CONNECTION *olddesc;
    
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
    to_actor( "As who what?\n\r", ch );
        return;
    }
    
    if ( (wch = get_actor_world( ch, arg )) == NULL )
    {
        to_actor( "They aren't here.\n\r", ch );
        return;
    }


    if ( NPC(wch) || IS_IMMORTAL(wch) )
    {
        to_actor( "Only mortal PCs.\n\r", ch );
        return;
    }
    
    olddesc = wch->desc;
    wch->desc = ch->desc;
    ch->desc->character = wch;
    
    if ( strstr( argument, "quit" )
      || !str_prefix( argument, "quit" ) )
    {
        to_actor( "No thanks.\n\r", ch );
        return;
    }
    
    interpret( wch, argument );
    
    ch->desc = olddesc;
    olddesc->character = ch;

    return;
}



/*
 * Syntax:  goto [person/place/thing]
 */
void cmd_goto( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    SCENE *location;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
    to_actor( "Goto where?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
    to_actor( "No such location.\n\r", ch );
	return;
    }

#if defined(IMMORTAL_PRIVACY)
    if ( scene_is_private( location ) )
    {
    to_actor( "That scene is private right now.\n\r", ch );
	return;
    }
#endif

    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );

    if ( PC(ch,wizinvis) < LEVEL_IMMORTAL )
    {
    cmd_emote( ch, (GET_PC(ch,bamfout[0],'\0') != '\0')
        ? PC(ch,bamfout) : "leaves in a swirling mist" );
    }

    actor_from_scene( ch );
    actor_to_scene( ch, location );

    if ( PC(ch,wizinvis) < LEVEL_IMMORTAL  )
    {
    cmd_emote( ch, (GET_PC(ch,bamfin[0],'\0') != '\0')
        ? PC(ch,bamfin) : "appears in a swirling mist" );
    }

    cmd_look( ch, "auto" );
    return;
}


void cmd_reboo( PLAYER *ch, char *argument )
{
    to_actor( "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}



/*
 * Syntax:  reboot
 */
void cmd_reboot( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern int num_hour;
    extern bool fCopyOverride;

    fCopyOverride = FALSE;
    one_argument( argument, buf );
    if ( buf[0] == '\0' || atoi(buf) == 0 )
    {
        num_hour = (PULSE_PER_SECOND*2);  /* 2 second delay */
        snprintf( buf, MAX_STRING_LENGTH, "Reboot initiated by %s.\n\r ", STR(ch,name) );
        write_global( buf );
        return;
    }

    num_hour = ((PULSE_PER_SECOND * 60) * atoi(buf)) + 1;
    return;
}



void cmd_shutdow( PLAYER *ch, char *argument )
{
    to_actor( "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}



/*
 * Syntax:  shutdown
 */
void cmd_shutdown( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool shut_down;
    extern bool fCopyOverride;

    snprintf( buf, MAX_STRING_LENGTH, "Shutdown by %s. ", STR(ch,name) );
    append_file( ch, SHUTDOWN_FILE, buf );
    strcat( buf, "\n\r" );
    write_global( buf );
    
    fCopyOverride = TRUE;
    shut_down = TRUE;
    return;
}




/*
 * Syntax:  snoop [person]
 */
void cmd_snoop( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CONNECTION *d;
    PLAYER *victim;

    one_argument( argument, arg );

    if ( NPC(ch) )
    {
        to_actor( "Bad actor!  Don't ever do that again!\n\r", ch );
        return;
    }

    if ( arg[0] == '\0' ) sprintf( arg, "self" );

    if ( ( victim = get_actor_world( ch, arg ) ) == NULL )
    {
        to_actor( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
        to_actor( "No connection to snoop.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
    to_actor( "Cancelling all snoops.\n\r", ch );
	for ( d = connection_list; d != NULL; d = d->next )
	{
            if ( d->snoop_by == ch->desc )
                d->snoop_by = NULL;
	}
	return;
    }
    
    if ( victim->desc->snoop_by == ch->desc )
    {
        victim->desc->snoop_by = NULL;
        to_actor( "Cancelled.\n\r", ch );
        return;
    }

    if ( victim->desc->snoop_by != NULL )
    {
        to_actor( "Busy already.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        victim->desc->snoop_by = NULL;
        act( "$B$n is snooping you.$R", ch, NULL, victim, TO_VICT );
    }

    if ( ch->desc != NULL )
    {
	for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
	{
	    if ( d->character == victim || d->original == victim )
	    {
                to_actor( "No snoop loops.\n\r", ch );
		return;
	    }
	}
    }

    victim->desc->snoop_by = ch->desc;
    to_actor( "Snooped.\n\r", ch );
    return;
}




/*
 * Syntax:  switch [person]
 */
void cmd_switch( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PLAYER *victim;

    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
        to_actor( "Switch into whom?\n\r", ch );
	return;
    }

    if ( ch->desc == NULL )
	return;
    
    if ( ch->desc->original != NULL )
    {
        to_actor( "You are already switched.\n\r", ch );
	return;
    }

    if ( ( victim = get_actor_world( ch, arg ) ) == NULL )
    {
        to_actor( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
        to_actor( "Ok.\n\r", ch );
	return;
    }

    if ( victim->desc != NULL || !NPC(victim) )
    {
        to_actor( "Character in use.\n\r", ch );
	return;
    }

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    to_actor( "Ok.\n\r", victim );
    return;
}



/*
 * Syntax:  return
 */
void cmd_return( PLAYER *ch, char *argument )
{
    if ( ch->desc == NULL )
	return;

    if ( ch->desc->original == NULL )
    {
        to_actor( "You aren't switched.\n\r", ch );
	return;
    }

    to_actor( "You return to your original body.\n\r", ch );
    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc; 
    ch->desc                  = NULL;
    return;
}




/*
 * Syntax:  charload [name]
 *          charload [name] deny
 *          charload [name] allow
 *          charload [name] approve
 *          charload [name] reject
 *          charload [name] show
 *
 * Why does this not load the associated actors?
 */
void cmd_charload( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CONNECTION *d;
    PLAYER *nch;
    bool fOld;
    
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' )
    {
        to_actor( "cload <character> <action>\n\r", ch );
        to_actor( "deny allow approve reject show unload save\n\r", ch );
        to_actor( "Characters not logged in are loaded into the scene they were last in.\n\r", ch );
        return;
    }

    if ( ( nch = get_actor_world( ch, arg ) ) == NULL )
    {
        d = ch->desc;
        d->character = NULL;
        fOld = load_actor_prop( d, capitalize( arg ) );
        nch = d->character;
        nch->desc = NULL;       /* juggling desc */
        d->character = ch;
        ch->desc = d;
    
        if ( !fOld )
        {
            free_char( nch );
            to_actor( "That character does not exist.\n\r", ch );
            return;
        }

        if ( nch->in_scene == NULL )
        actor_to_scene( nch, get_scene( SCENE_VNUM_APPLY ) );
        else
        actor_to_scene( nch, nch->in_scene );
        
        nch->next = actor_list;
        actor_list = nch;
        to_actor( "Character loaded.\n\r", ch );
    }
    else
    {
        if ( NPC(nch) )
        return;
    }

    if ( !str_cmp( arg2, "unload" ) )
    {
        to_actor( "Saved and unloaded.\n\r", ch );
        cmd_quit( nch, "" );
        return;
    }

    if ( !str_cmp( arg2, "save" ) )
    {
        to_actor( "Saved.\n\r", ch );
        cmd_save( nch, "internal" );
        return;
    }

/* OLD
    if ( !str_cmp( arg2, "approve" ) )
    {
        RACE *pRace;

        PC(nch,level) = LEVEL_MORTAL;
        REMOVE_BIT( nch->flag, PLR_DENY );

        pRace = race_lookup( ch->race );
        if ( pRace == NULL )
        {
            to_actor( "Invalid race.\n\r", ch );
            return;
        }

        actor_from_scene( nch );
        actor_to_scene( nch, get_scene( pRace->start_scene ) );
        to_actor( "Approved.\n\r", ch );
        free_string( PC(nch,denial) );
        PC(nch,denial) = str_dup( "" );
        return;
    }
*/
    if ( !str_cmp( arg2, "approve" ) ) {

        REMOVE_BIT( nch->flag, PLR_APPLIED );
        REMOVE_BIT( nch->flag, PLR_DENY );
        ch->userdata->app_time = 0;
        STC( "Approved.\n\r", ch );

        free_string( PC(nch,denial) );
        PC(nch,denial) = str_dup( "" );
        return;
    }

/* OLD
    if ( !str_cmp( arg2, "reject" ) )
    {
        PC(nch,level) = LEVEL_APPLIED;
        actor_from_scene( nch );
        actor_to_scene( nch, get_scene( SCENE_VNUM_APPLY ) );
        string_append( ch, &PC(nch,denial)  );
        PC(nch,denial) = str_dup( argument );
        to_actor( "Rejecting.\n\r", ch );
        return;
    }
 */
    if ( !str_cmp( arg2, "reject" ) ) {
        SET_BIT( nch->flag, PLR_APPLIED );
        actor_from_scene( nch );
        actor_to_scene( nch, get_scene( SCENE_VNUM_APPLY ) );

        string_append( ch, &PC(nch,denial)  );
        PC(nch,denial) = str_dup( argument );
        STC( "Rejecting.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "deny" ) )
    {
        SET_BIT( nch->flag, PLR_DENY );
        to_actor( "Denied.\n\r", ch );
        save_actor_prop( nch );
        cmd_quit( nch, "" );
        return;
    }

    if ( !str_cmp( arg2, "allow" ) )
    {
        REMOVE_BIT( nch->flag, PLR_DENY );
        to_actor( "Allowed.\n\r", ch );
        return;
    }

    return;
}
    



/*
 * Syntax:  shell [command]
 */
void cmd_shell( PLAYER *ch, char *argument )
{
#if defined(unix)
    char buf[MAX_INPUT_LENGTH];
    char * temp;
    
    if ( !str_cmp( argument, "last" ) )
    {
        FILE *out;
        char *pf;

        out = fopen( "ispell -a -S < .scenedesc", "r" );
        
        while (fgets(buf, MAX_STRING_LENGTH-1, out) != NULL)
        {
            if ( buf[0] != '&' ) continue;
            pf = format_string( str_dup( buf ) );
            to_actor( pf, ch );
            free_string( pf );
        }
            
        pclose( out );
        return;
    }

    snprintf( buf, MAX_STRING_LENGTH, "%s > .shellout &", argument );
    system( buf );

    temp = fread_file( ".shellout" );
    page_to_actor( temp, ch );
    free_string(temp);

#else
    system( argument );
#endif
    return;
}



/*
 * Syntax:  purge
 *          purge [prop]
 *          purge [person]
 *          purge [scene dbkey]
 */
void cmd_purge( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PLAYER *victim;
    PROP *prop = NULL;

    one_argument( argument, arg );

    if ( arg[0] == '\0' || is_number(arg) )
    {
	/* 'purge' */
	PLAYER *vnext;
	PROP  *prop_next;
        SCENE *pScene;

        pScene = get_scene( atoi(arg) );

        if ( !pScene ) victim = ch->in_scene->people;
        else
        victim = pScene->people;

	for ( ; victim != NULL; victim = vnext )
	{
	    vnext = victim->next_in_scene;
	        if ( NPC(victim) && victim->desc == NULL 
                     && victim != ch )
			extractor_char( victim, TRUE );
	}

        if ( !pScene ) prop = ch->in_scene->contents;
        else
        prop = pScene->contents;

	for ( ; prop != NULL; prop = prop_next )
	{
	    prop_next = prop->next_content;
	    extractor_prop( prop );
	}

        if ( NPC(ch) ) return;

        act( "$n cleanses the scene with righteous fire!", ch, NULL, NULL, TO_SCENE);
        to_actor( "You cleanse the scene with righteous fire!\n\r", ch );
	return;
    }

    if ( ( victim = get_actor_scene( ch, arg ) ) == NULL
      && ( prop    = get_prop_here( ch, arg ) ) == NULL )
    {
        to_actor( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim )
    {
    if ( !NPC(victim) || victim->desc != NULL )
    {
    to_actor( "Not on PC's or switched PC's.\n\r", ch );
	return;
    }

    act( "You purge $N.", ch, NULL, victim, TO_ACTOR );
    act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    extractor_char( victim, TRUE );
    }

    if ( prop )
    {
    act( "You purge $P.", ch, NULL, prop, TO_ACTOR );
    act( "$n purges $P.", ch, NULL, prop, TO_SCENE );
    extractor_prop( prop );
    }

    return;
}



#define CH(connection)  ((connection)->original ? \
(connection)->original : (connection)->character)

#if defined(NEVER)
/*  Hotboot - Original idea: Fusion of MUD++
 *  Adapted to Diku by Erwin S. Andreasen, <erwin@andreasen.org>
 *  Adapted for NiMUD by Niksa
 *  http://www.andreasen.org
 *  Changed into a ROM patch after seeing the 100th request for it :)
 */
void cmd_hotboot (PLAYER *ch, char * argument)
{
	FILE *fp;
	CONNECTION *d, *d_next;
	char buf [100], buf2[100];
	extern int port,control; /* db.c */
	
	fp = fopen (HOTBOOT_FILE, "w");
	
	if (!fp)
	{
		to_actor ("Hotboot file not writeable, aborted.\n\r",ch);
		wtf_logf ("Could not write to hotboot file: %s", HOTBOOT_FILE);
		perror ("cmd_hotboot:fopen");
		return;
	}
	
	/* Consider changing all saved zones here, if you use OLC */
	
	/* cmd_zsave (NULL, ""); - autosave changed zones */
	
	
	sprintf (buf, "\n\r *** HOTBOOT by %s - please remain seated!\n\r", ch->name);
	
	/* For each playing connection, save its state */
	for (d = connection_list; d ; d = d_next)
	{
		PLAYER * och = CH (d);
		d_next = d->next; /* We delete from the list , so need to save this */
		
		if (!d->character || d->connected > NET_PLAYING) /* drop those logging on */
		{
			write_to_connection (d->connection, "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r", 0);
			close_socket (d); /* throw'em out */
		}
		else
		{
			fprintf (fp, "%d %s %s\n", d->connection, och->name, d->host);

#if 0			 /* This is not necessary for ROM */
			if (och->level == 1)
			{
				write_to_connection (d->connection, "Since you are level one, and level one characters do not save, you gain a free level!\n\r", 0);
				advance_level (och);
				och->level++; /* Advance_level doesn't do that */
			}
#endif			
			save_actor_prop (och);
			
			write_to_connection (d->connection, buf, 0);
		}
	}
	
	fprintf (fp, "-1\n");
	fclose (fp);
	
	/* Close reserve and other always-open files and release other resources */
	
	fclose (fpReserve);
	
	/* exec - connections are inherited */
	
	sprintf (buf, "%d", port);
	sprintf (buf2, "%d", control);
	execl (EXEC_FILE, "merc", buf, "hotboot", buf2, (char *) NULL);

	/* Failed - sucessful exec will not return */
	
	perror ("cmd_hotboot: execl");
	to_actor ("Hotboot FAILED!\n\r",ch);
	
	/* Here you might want to reopen fpReserve */
	fpReserve = fopen (NULL_FILE, "r");
}

/* Recover from a hotboot - load players */
void hotboot_recover ( int control )
{
	CONNECTION *d;
	FILE *fp;
	char name [100];
	char host[MSL];
	int desc;
	bool fOld;
	
	wtf_logf ("Hotboot recovery initiated");
	
	fp = fopen (HOTBOOT_FILE, "r");
	
	if (!fp) /* there are some connections open which will hang forever then ? */
	{
		perror ("hotboot_recover:fopen");
		wtf_logf ("Hotboot file not found. Exitting.\n\r");
		exit (1);
	}

	unlink (HOTBOOT_FILE); /* In case something crashes - doesn't prevent reading	*/
	
	for (;;)
	{
		fscanf (fp, "%d %s %s\n", &desc, name, host);
		if (desc == -1)
			break;

		/* Write something, and check if it goes error-free */		
		if (!write_to_connection (desc, "\n\rRestoring from hotboot...\n\r",0))
		{
			close (desc); /* nope */
			continue;
		}
		
		d = new_connection(  );
		d->connection = desc;
		
		d->host = str_dup (host);
		d->next = connection_list;
		connection_list = d;
		d->connected = NET_HOTBOOT_RECOVER; /* -15, so close_socket frees the char */
		
	
		/* Now, find the pfile */
		
		fOld = load_actor_prop (d, name);
		
		if (!fOld) /* Player file not found?! */
		{
			write_to_connection (desc, "\n\rSomehow, your character was lost in the hotboot. Sorry.\n\r", 0);
			close_socket (d);			
		}
		else /* ok! */
		{
			write_to_connection (desc, "\n\rHotboot recovery complete.\n\r",0);
	
			/* Just In Case */
			if (!d->character->in_scene)
				d->character->in_scene = get_scene (SCENE_VNUM_START);

			/* Insert in the actor_list */
			d->character->next = actor_list;
			actor_list = d->character;

			actor_to_scene (d->character, d->character->in_scene);
			cmd_look (d->character, "auto");
			act ("$n materializes!", d->character, NULL, NULL, TO_SCENE);
			d->connected = NET_PLAYING;

			/* Pets are not used in NiMUD like this, so this is not needed. */
			/*
			if (d->character->pet != NULL)
			{
			    actor_to_scene(d->character->pet,d->character->in_scene);
			    act("$n materializes!.",d->character->pet,NULL,NULL,TO_SCENE);
			} 
			*/
		}
		
	}
   fclose (fp);
	
	
}
#endif

