/****************************************************************************
******************************************************************************
* Locke's   __ -based on merc v5.0-____        NIM Server Software           *
* ___ ___  (__)__    __ __   __ ___|  | v5.1a  Version 5.1a                  *
* |  /   \  __|  \__/  |  | |  |      |        documentation release         *
* |       ||  |        |  \_|  | ()   |        Valentines Day Massacre 2003  *
* |    |  ||  |  |__|  |       |      |                                      *
* |____|__|___|__|  |__|\___/__|______|        http://www.nimud.org/nimud    *
*   n a m e l e s s i n c a r n a t e          dedicated to chris cool       *
******************************************************************************
 *                                                                          *
 *      This is the server software for The Isles, called NiMUD 5.1a        *
 *                                                                          *
 *    Portions of this code are copyrighted to Herb Gilliland.              * 
 *    Copyright (c) 1994-2003 by Herb Gilliland.  All rights reserved.      *
 *                                                                          *
 *  Much time and thought has gone into this software and you are           *
 *  benefitting.  We hope that you share your changes too.  What goes       *
 *  around, comes around.                                                   *
 *                                                                          *
 ****************************************************************************
 *   Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *   Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 ******************************************************************************
 * NiMUD copyright (c) 1993-2009 by Herb Gilliland                            *
 * Includes improvements by Chris Woodward (c) 1993-1994                      *
 * Based on Merc 2.1c / 2.2                                                   *
 ******************************************************************************
 * To use this software you must comply with its license.                     *
 * licenses.  See the file 'docs/COPYING' for more information about this.    *
 ******************************************************************************
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
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <zlib.h>

#include "nimud.h"
#include "script.h"
#include "skills.h"



char * get_direction( char *arg )
{
    switch ( LOWER(arg[0]) )
      {
      case 'n': 
      {
            if ( LOWER(arg[1]) == 'w' ) return "northwest";
            if ( LOWER(arg[1]) == 'e' ) return "northeast";
            if ( LOWER(arg[5]) == 'w' ) return "northwest";
            if ( LOWER(arg[5]) == 'e' ) return "northeast";
            return "north";
	  }
      case 's':
      {
            if ( LOWER(arg[1]) == 'w' ) return "southwest";
            if ( LOWER(arg[1]) == 'e' ) return "southeast";
            if ( LOWER(arg[5]) == 'w' ) return "southwest";
            if ( LOWER(arg[5]) == 'e' ) return "southeast";
            return "south";
	  }
      case 'e': return "east";
      case 'w': return "west";
      case 'u': return "up";
      case 'd': return "down";
      }
 
    return "anywhere";
}


int find_door( PLAYER *ch, char *arg )
{
    EXIT *pexit;
    int door;

    if ( ( door = get_dir( arg ) ) == MAX_DIR )
    {
       for ( door = 0; door < MAX_DIR; door++ )
       {
           if ( ( pexit = ch->in_scene->exit[door] ) != NULL
             && IS_SET(pexit->exit_flags, EXIT_ISDOOR)
             && !MTD(pexit->keyword)
             && is_name( arg, pexit->keyword ) )
           return door;
       }

       act( "I see no $T here.", ch, NULL, arg, TO_ACTOR );
       return MAX_DIR;
    }
    
    pexit = ch->in_scene->exit[door];

    if ( pexit != NULL 
      && IS_SET( pexit->exit_flags, EXIT_SECRET )
      && IS_SET( pexit->exit_flags, EXIT_CLOSED ) )
    {
        act( "I see no door $T here.", ch, NULL, get_direction(arg), TO_ACTOR );
        return MAX_DIR;
    }

    if ( pexit != NULL 
      && IS_SET( pexit->exit_flags, EXIT_CONCEALED ) )
    {
        act( "I see no door $T here.", ch, NULL, get_direction(arg), TO_ACTOR );
        return MAX_DIR;
    }

    if ( pexit == NULL )
    {
       act( "I see no door $T here.", ch, NULL, get_direction(arg), TO_ACTOR );
       return MAX_DIR;
    }

    if ( !IS_SET(pexit->exit_flags, EXIT_ISDOOR) )
    {
       to_actor( "You cannot do that.\n\r", ch );
       return MAX_DIR;
    }

    return door;
}



/*
 * Syntax:  open [door]
 *          open [direction]
 *          open [prop]
 */
void cmd_open( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PROP *prop;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	to_actor( "Open what?\n\r", ch );
	return;
    }

    if ( ( prop = get_prop_here( ch, arg ) ) != NULL )
    {
       if ( prop->item_type != ITEM_CONTAINER
         && prop->item_type != ITEM_FURNITURE
         && !IS_SET(prop->value[1], FURN_EXIT) )
       {
           to_actor( "You cannot open that.\n\r", ch );
           return;
       }

       if ( !IS_SET(prop->value[1], CONT_CLOSED) )
       {
           to_actor( "It is not closed.\n\r", ch );
           return;
       }

       if ( !IS_SET(prop->value[1], CONT_CLOSEABLE) )
       {
           to_actor( "You cannot do that.\n\r", ch );
           return;
       }

       if ( IS_SET(prop->value[1], CONT_LOCKED) )
       {
           act( "$p is locked.", ch, prop, NULL, TO_ACTOR );
           return;
       }

       REMOVE_BIT(prop->value[1], CONT_CLOSED);
       act( "You open $p.", ch, prop, NULL, TO_ACTOR );
       act( "$n opens $p.", ch, prop, NULL, TO_SCENE );
       return;
    }

    if ( ( door = find_door( ch, arg ) ) != MAX_DIR )
    {
        SCENE *to_scene;
        EXIT *pexit;
        EXIT *pexit_rev;

        pexit = ch->in_scene->exit[door];

        if ( !IS_SET(pexit->exit_flags, EXIT_CLOSED) )
        {
            to_actor( "It is already open.\n\r",  ch );
            return;
        }

        if ( IS_SET(pexit->exit_flags, EXIT_LOCKED) )
        {
            to_actor( "It is locked.\n\r", ch );
            return;
        }

        if ( IS_SET(pexit->exit_flags, EXIT_JAMMED) )
        {
            to_actor( "It is jammed shut.\n\r", ch );
            return;
        }

        REMOVE_BIT(pexit->exit_flags, EXIT_CLOSED);
        act( "$n open$v the $t.", ch, pexit->keyword, NULL, TO_ALL );

        if ( ( to_scene   = pexit->to_scene               ) != NULL
          && ( pexit_rev = to_scene->exit[rev_dir[door]] ) != NULL
          && pexit_rev->to_scene == ch->in_scene )
        {
            PLAYER *rch;

            REMOVE_BIT( pexit_rev->exit_flags, EXIT_CLOSED );
            for ( rch = to_scene->people; rch != NULL; rch = rch->next_in_scene )
            act( "The $T opens.", rch, NULL, pexit_rev->keyword, TO_ACTOR );
        }
    }

    return;
}



/*
 * Syntax:  close [door]
 *          close [direction]
 *          close [prop]
 */
void cmd_close( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PROP *prop;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	to_actor( "Close what?\n\r", ch );
	return;
    }

    if ( ( prop = get_prop_here( ch, arg ) ) != NULL )
    {
       if ( prop->item_type != ITEM_CONTAINER
         && prop->item_type != ITEM_FURNITURE
         && !IS_SET(prop->value[1], FURN_EXIT) )
       {
           to_actor( "You cannot close that.\n\r", ch );
           return;
       }

       if ( IS_SET(prop->value[1], CONT_CLOSED) )
       {
           to_actor( "It is already closed.\n\r", ch );
           return;
       }

       if ( !IS_SET(prop->value[1], CONT_CLOSEABLE) )
       {
           to_actor( "You cannot do that.\n\r", ch );
           return;
       }

       SET_BIT(prop->value[1], CONT_CLOSED);
       act( "You close $p.", ch, prop, NULL, TO_ACTOR );
       act( "$n closes $p.", ch, prop, NULL, TO_SCENE );
       return;
    }

    if ( ( door = find_door( ch, arg ) ) != MAX_DIR )
    {
	SCENE *to_scene;
	EXIT *pexit;
	EXIT *pexit_rev;

	pexit	= ch->in_scene->exit[door];

	if ( IS_SET(pexit->exit_flags, EXIT_CLOSED) )
    {
        to_actor( "It is already closed.\n\r", ch );
        return;
    }

    if ( IS_SET(pexit->exit_flags, EXIT_JAMMED) )
    {
        to_actor( "It is jammed open.\n\r", ch );
        return;
    }

	SET_BIT(pexit->exit_flags, EXIT_CLOSED);
    act( "$n close$v the $t.", ch, pexit->keyword, NULL, TO_ALL );

	if ( ( to_scene   = pexit->to_scene               ) != NULL
	&&   ( pexit_rev = to_scene->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->to_scene == ch->in_scene )
	{
	    PLAYER *rch;

	    SET_BIT( pexit_rev->exit_flags, EXIT_CLOSED );
	    for ( rch = to_scene->people; rch != NULL; rch = rch->next_in_scene )
        act( "The $T closes.", rch, NULL, pexit_rev->keyword, TO_ACTOR );
	}
    }

    return;
}



PROP *has_key( PLAYER *ch, int key )
{
   PROP *prop;

   for ( prop = ch->carrying; prop != NULL; prop = prop->next_content )
   if ( prop->pIndexData->dbkey == key ) return prop;

   return NULL;
}



/*
 * Syntax:  lock [door]
 *          lock [direction]
 *          lock [prop]
 */
void cmd_lock( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PROP *prop;
    int door;
    PROP *key;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
       to_actor( "Lock what?\n\r", ch );
       return;
    }

    if ( ( prop = get_prop_here( ch, arg ) ) != NULL )
    {
       if ( prop->item_type != ITEM_CONTAINER
         && prop->item_type != ITEM_FURNITURE
         && !IS_SET(prop->value[1], FURN_EXIT) )
       {
           to_actor( "You cannot do that.\n\r", ch );
           return;
       }

       if ( !IS_SET(prop->value[1], CONT_CLOSED) )
       {
           to_actor( "It is not closed.\n\r", ch );
           return;
       }

       if ( prop->value[2] <= 0 )
       {
           to_actor( "It cannot be locked.\n\r", ch );
           return;
       }

       if ( (key = has_key( ch, prop->value[2] )) == NULL )
       {
           to_actor( "You lack the key.\n\r", ch );
           return;
       }

       if ( IS_SET(prop->value[1], CONT_LOCKED) )
       {
           to_actor( "It is already locked.\n\r", ch );
           return;
       }

       SET_BIT(prop->value[1], CONT_LOCKED);
       if ( key != NULL )
       act( "You lock $p with $P.", ch, prop, key, TO_ACTOR );
       else
       act( "You lock $p.", ch, prop, NULL, TO_ACTOR );
       act( "$n locks $p.", ch, prop, NULL, TO_SCENE );
       return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
       SCENE *to_scene;
       EXIT *pexit;
       EXIT *pexit_rev;


       pexit   = ch->in_scene->exit[door];

       if ( pexit == NULL )
       {
           to_actor( "There is no door there.\n\r", ch );
           return;
       }

       if ( !IS_SET(pexit->exit_flags, EXIT_CLOSED) )
       {
           to_actor( "It is not closed.\n\r", ch );
           return;
       }

       if ( pexit->key <= 0 )
       {
           to_actor( "It cannot be locked.\n\r", ch );
           return;
       }

       if ( (key = has_key( ch, pexit->key)) == NULL )
       {
           to_actor( "You lack the key.\n\r", ch );
           return;
       }

       if ( IS_SET(pexit->exit_flags, EXIT_LOCKED) )
       {
           to_actor( "It is already locked.\n\r", ch );
           return;
       }

       SET_BIT(pexit->exit_flags, EXIT_LOCKED);
       if ( key != NULL )
       act( "You lock it with $P.", ch, prop, key, TO_ACTOR );
       else
       act( "You lock it.", ch, prop, NULL, TO_ACTOR );
       act( "$n locks the $T.", ch, NULL, pexit->keyword, TO_SCENE );

       if ( ( to_scene   = pexit->to_scene               ) != NULL
       &&   ( pexit_rev = to_scene->exit[rev_dir[door]] ) != 0
       &&   pexit_rev->to_scene == ch->in_scene )
       {
           PLAYER *rch;

           SET_BIT( pexit_rev->exit_flags, EXIT_LOCKED );
           for ( rch = to_scene->people; rch != NULL; rch = rch->next_in_scene )
           act( "The $T clicks.", rch, NULL, pexit_rev->keyword, TO_ACTOR );
       }
    }

    return;
}



/*
 * Syntax:  unlock [door]
 *          unlock [direction]
 *          unlock [prop]
 */
void cmd_unlock( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PROP *prop;
    int door;
    PROP *key;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	to_actor( "Unlock what?\n\r", ch );
	return;
    }

    if ( ( prop = get_prop_here( ch, arg ) ) != NULL )
    {
       if ( prop->item_type != ITEM_CONTAINER
         && prop->item_type != ITEM_FURNITURE
         && !IS_SET(prop->value[1], FURN_EXIT) )
       {
           to_actor( "You cannot do that.\n\r", ch );
           return;
       }

       if ( !IS_SET(prop->value[1], CONT_CLOSED) )
       {
           to_actor( "It is not closed.\n\r", ch );
           return;
       }

       if ( prop->value[2] <= 0 )
       {
           to_actor( "It cannot be unlocked.\n\r", ch );
           return;
       }

       if ( (key = has_key( ch, prop->value[2] )) == NULL )
       {
           to_actor( "You lack the key.\n\r", ch );
           return;
       }

       if ( !IS_SET(prop->value[1], CONT_LOCKED) )
       {
           to_actor( "It is already unlocked.\n\r", ch );
           return;
       }

       REMOVE_BIT(prop->value[1], CONT_LOCKED);
       if ( key != NULL )
       act( "You unlock $p with $P.", ch, prop, key, TO_ACTOR );
       else
       act( "You unlock $p.", ch, prop, NULL, TO_ACTOR );
       act( "You unlock $p.", ch, prop, NULL, TO_ACTOR );
       return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
       /* 'unlock door' */
       SCENE *to_scene;
       EXIT *pexit;
       EXIT *pexit_rev;

       pexit = ch->in_scene->exit[door];
       if (pexit == NULL)
       {
             to_actor( "It is impossible to go that way.\n\r", ch );
             return;
       }

       if ( !IS_SET(pexit->exit_flags, EXIT_CLOSED) )
       {
           to_actor( "It is not closed.\n\r", ch );
           return;
       }

       if ( pexit->key <= 0 )
       {
           to_actor( "It cannot be unlocked.\n\r", ch );
           return;
       }

       if ( (key = has_key( ch, pexit->key)) == NULL )
       {
           to_actor( "You lack the key.\n\r", ch );
           return;
       }

       if ( !IS_SET(pexit->exit_flags, EXIT_LOCKED) )
       {
           to_actor( "It is already unlocked.\n\r", ch );
           return;
       }

       REMOVE_BIT(pexit->exit_flags, EXIT_LOCKED);
       if ( key != NULL )
       act( "You unlock the $t with $P.", ch, pexit->keyword, key, TO_ACTOR );
       else
       act( "You unlock $p.", ch, prop, NULL, TO_ACTOR );
       act( "$n unlocks the $T.", ch, NULL, pexit->keyword, TO_SCENE );

       if ( IS_SET(pexit->exit_flags, EXIT_EAT_KEY) )
       {
       act( "The lock of the $t eats $P.", ch, pexit->keyword, key, TO_ACTOR );
       extractor_prop( key );
       }

       /* unlock the other side */
       if ( ( to_scene   = pexit->to_scene               ) != NULL
       &&   ( pexit_rev = to_scene->exit[rev_dir[door]] ) != NULL
       &&   pexit_rev->to_scene == ch->in_scene )
       {
           PLAYER *rch;

           REMOVE_BIT( pexit_rev->exit_flags, EXIT_LOCKED );
           for ( rch = to_scene->people; rch != NULL; rch = rch->next_in_scene )
           act( "The $T clicks.", rch, NULL, pexit_rev->keyword, TO_ACTOR );
       }
    }

    return;
}




/*
 * Syntax:  pick [direction]
 *          pick [door]
 *          pick [prop]
 */
void cmd_pick( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PLAYER *gch;
    PROP *prop;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	to_actor( "Pick what?\n\r", ch );
	return;
    }
 
    { SKILL *pSkill = skill_lookup( "pick" );
      if ( pSkill ) WAIT_STATE( ch, pSkill->delay );
    }

    for ( gch = ch->in_scene->people; gch; gch = gch->next_in_scene )
    {
    if ( NPC(gch) && IS_AWAKE(gch) && get_curr_int(gch) > 9 )
	{
        act( "$N notices you are trying to pick a lock.", ch, NULL, gch, TO_ACTOR );
        ch->bounty += 10;
	}
    }

    if ( !skill_check( ch, skill_lookup( "pick" ), 0 ) )
    {
	to_actor( "You failed.\n\r", ch);
	return;
    }

    if ( ( prop = get_prop_here( ch, arg ) ) != NULL )
    {
       if ( prop->item_type != ITEM_CONTAINER )
       {
           to_actor( "That's not a container.\n\r", ch );
           return;
       }

       if ( !IS_SET(prop->value[1], CONT_CLOSED) )
       {
           to_actor( "It is not closed.\n\r", ch );
           return;
       }

       if ( prop->value[2] <= 0 )
       {
           to_actor( "It cannot be unlocked.\n\r", ch );
           return;
       }

       if ( !IS_SET(prop->value[1], CONT_LOCKED) )
       {
           to_actor( "It is already unlocked.\n\r", ch );
           return;
       }

       if ( IS_SET(prop->value[1], CONT_PICKPROOF) )
       {
           to_actor( "You cannot decipher the lock.\n\r", ch );
           return;
       }

       REMOVE_BIT(prop->value[1], CONT_LOCKED);
       act( "You pick the lock of $p.", ch, prop, NULL, TO_ACTOR );
       act( "$n picks the lock of $p.", ch, prop, NULL, TO_SCENE );
       return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
       SCENE *to_scene;
       EXIT *pexit;
       EXIT *pexit_rev;

       pexit = ch->in_scene->exit[door];
   
       if ( !pexit ) {
           to_actor( "You cannot go that way.\n\r", ch );
           return;
       }

       if ( !IS_SET(pexit->exit_flags, EXIT_CLOSED) )
       {
           to_actor( "It is not closed.\n\r", ch );
           return;
       }

       if ( pexit->key < 0 )
       {
           to_actor( "It cannot be picked.\n\r", ch );
           return;
       }

       if ( !IS_SET(pexit->exit_flags, EXIT_LOCKED) )
       {
           to_actor( "It's already unlocked.\n\r", ch );
           return;
       }

       if ( IS_SET(pexit->exit_flags, EXIT_PICKPROOF) )
       {
           to_actor( "You failed.\n\r", ch );
           return;
       }

       REMOVE_BIT(pexit->exit_flags, EXIT_LOCKED);
       act( "You pick the lock of the $T.", ch, NULL, pexit->keyword, TO_ACTOR );
       act( "$n picks the lock of the $T.", ch, NULL, pexit->keyword, TO_SCENE );

       if ( ( to_scene   = pexit->to_scene               ) != NULL
       &&   ( pexit_rev = to_scene->exit[rev_dir[door]] ) != NULL
       &&   pexit_rev->to_scene == ch->in_scene )
       {
           PLAYER *rch;

           REMOVE_BIT( pexit_rev->exit_flags, EXIT_LOCKED );
           for ( rch = to_scene->people; rch != NULL; rch = rch->next_in_scene )
           act( "The $T clicks.", rch, NULL, pexit_rev->keyword, TO_ACTOR );
       }
    }

    return;
}



