/******************************************************************************
 * Locke's   __ -based on merc v2.2-____        NIM Server Software           *
 * ___ ___  (__)__    __ __   __ ___|  | v5.1a  Version 5.1a (ALPHA)          *
 * |  /   \  __|  \__/  |  | |  |      |        unreleased+revamped 2004      *
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

/*
 * List occupants.
 */
void show_occupants_to_actor( PROP_DATA *prop, PLAYER_DATA *ch )
{
    PLAYER_DATA *och;

    for ( och = ch->in_scene->people; och != NULL; och = och->next_in_scene )
    {
    if ( och->furniture == prop && can_see( ch, och ) )
    act( "$N $K $t on it.", ch, position_name( och->position ), och, TO_ACTOR );
    }
    return;
}


/*
 * Count occupants.
 */
int count_occupants( PROP_DATA *prop )
{
    PLAYER_DATA *och;
    int count = 0;

    if ( prop->in_scene == NULL )
    return -1;

    for ( och = actor_list; och != NULL; och = och->next )
        if ( och->furniture == prop ) count++;

    return count;
}



bool has_occupant( PROP_DATA *prop )
{
    PLAYER_DATA *och;

    for ( och = actor_list; och != NULL; och = och->next )
    {
        if ( och->furniture == prop )
          return TRUE;
    }
    return FALSE;
}



bool occupant( PROP_DATA *prop )
{
    PLAYER_DATA *och;

    for ( och = actor_list; och != NULL; och = och->next )
    {
        if ( och->furniture == prop  )
          return TRUE;
    }
    return FALSE;
}


PLAYER_DATA *hitched( PROP_DATA *prop )
{
    PLAYER_DATA *och;

    for ( och = actor_list; och != NULL; och = och->next )
    {
        if ( och->hitched_to == prop )
          return och;
    }
    return NULL;
}




PROP_DATA *get_furn_here( PLAYER_DATA *ch, char *argument )
{
    PROP_DATA *prop;
    PROP_DATA *saved = NULL;

    for ( prop = ch->in_scene->contents;  prop != NULL;  prop = prop->next_content )
    {
        if ( is_name(argument,STR(prop,name))
          && prop->item_type == ITEM_FURNITURE
          && ( saved == NULL || !OCCUPADO(prop) ) )
            saved = prop;
    }       
    return saved;
}




void set_furn( PLAYER_DATA *ch, PROP_DATA *prop )
{
    ch->furniture = NULL;

    if ( prop == NULL ) return;

    if ( OCCUPADO(prop) )
    {
        bug( "set_furn: Occupado.", 0 );
        return;
    }

    ch->furniture = prop;
    return;
}


