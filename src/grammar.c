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





/*
 * Find a prop in a list.
 */
PROP *get_prop_list( PLAYER *ch, char *argument, PROP *list )
{
    char arg[MAX_INPUT_LENGTH];
    PROP *prop;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( prop = list; prop != NULL; prop = prop->next_content )
    {
    if ( can_see_prop( ch, prop ) && is_prename( arg, STR(prop, name) ) )
	{
	    if ( ++count == number )
		return prop;
	}
    }

    return NULL;
}


/*
 * Find a prop in a list.
 */
PROP *find_prop_list( PROP *list, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PROP *prop;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( prop = list; prop != NULL; prop = prop->next_content )
    {
    if ( is_prename( arg, STR(prop, name) ) )
	{
	    if ( ++count == number )
		return prop;
	}
    }

    return NULL;
}



/*
 * Find a prop in player's hands.
 */
PROP *get_prop_carry( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PROP *prop;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( prop = ch->carrying; prop != NULL; prop = prop->next_content )
    {
    if ( (prop->wear_loc == WEAR_NONE
      ||  prop->wear_loc == WEAR_HOLD_1
      ||  prop->wear_loc == WEAR_HOLD_2
      ||  prop->wear_loc == WEAR_WIELD_1
      ||  prop->wear_loc == WEAR_WIELD_2)
    &&   can_see_prop( ch, prop )
    &&   is_name( arg, STR(prop, name) ) )
	{
	    if ( ++count == number )
		return prop;
	}
    }


    return NULL;
}




/*
 * Find a prop in player's equipment.
 */
PROP *get_prop_wear( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PROP *prop;
    int number;
    int count;
    int iWear;

    number = number_argument( argument, arg );
    count  = 0;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
        if ( ( prop = get_eq_char( ch, iWear ) ) == NULL )
        continue;

    if ( prop->wear_loc != WEAR_NONE
    &&   prop->wear_loc != WEAR_HOLD_1
    &&   prop->wear_loc != WEAR_HOLD_2
	&&   can_see_prop( ch, prop )
    &&   is_name( arg, STR(prop, name) ) )
	{
	    if ( ++count == number )
		return prop;
	}
    }

    return NULL;
}


/*
 * Find a prop in the character's inventory top level.
 */
PROP *get_prop_inv( PLAYER *ch, char *argument )
{
    PROP *prop;
    char arg[MAX_INPUT_LENGTH];
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( prop = ch->carrying; prop != NULL; prop = prop->next_content )
    {
    if ( can_see_prop( ch, prop )  &&   is_name( arg, STR(prop, name) ) )
	{
	    if ( ++count == number )
		return prop;
	}
    }

    return NULL;
}


/*
 * Find a prop in the character's inventory top level, second level.
 */
PROP *get_prop_inv2( PLAYER *ch, char *argument )
{
    PROP *prop, *prop2;
    char arg[MAX_INPUT_LENGTH];
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;

    for ( prop = ch->carrying; prop != NULL; prop = prop->next_content )
    {
    if ( can_see_prop( ch, prop )  &&   is_name( arg, STR(prop, name) ) )
    {
        if ( ++count == number )
        return prop;
    }

    for ( prop2 = prop->contains; prop2 != NULL;  prop2 = prop2->next_content )
        {

        if ( can_see_prop( ch, prop2 )  &&   is_name( arg, STR(prop2, name) ) )
        {
        if ( ++count == number )
        return prop2;
        }

        }
    }

    return NULL;
}


/*
 * Find a prop in the scene or in inventory.
 */
PROP *get_prop_here( PLAYER *ch, char *argument )
{
    PROP *prop;

    if ( ( prop = get_prop_inv( ch, argument ) ) != NULL )
    return prop;

    prop = get_prop_list( ch, argument, ch->in_scene->contents );

    if ( prop != NULL )
    return prop;

    return NULL;
}



/*
 * Find a prop in the world.
 */
PROP *get_prop_world( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PROP *prop;
    int number;
    int count;

    if ( ( prop = get_prop_here( ch, argument ) ) != NULL )
	return prop;

    number = number_argument( argument, arg );
    count  = 0;
    for ( prop = prop_list; prop != NULL; prop = prop->next )
    {
    if ( can_see_prop( ch, prop ) && is_name( arg, STR(prop, name) ) )
	{
	    if ( ++count == number )
		return prop;
	}
    }

    return NULL;
}




#if defined(NEVER)
/*
 * Diku code - search a block of memory as if it were a string.
 * Slight incompatibility problem here on some machines I suspect.  
 * --Locke
 */
int search_block(char *arg, char **list, bool exact)
{
	register int i,l;

    /*
     * Make into lower case, and get length of string
     */
    for( l = 0; *(arg + l); l++ ) *(arg + l) = LOWER(*(arg + l));

    if (exact)
    {
        for( i = 0; **(list + i) != '\n'; i++ )
            if ( !strcmp(arg, *(list+i)) ) return(i);
    }
    else
    {
        /*
         * Avoid "" to match the first available string
         */

        if (!l) l = 1;

        for( i = 0; **(list+i) != '\n'; i++ )
           if ( !strncmp(arg, *(list + i), l) )
           return(i);
	}

	return(-1);
}
#endif



/*
 * Based on the Quinn argument interpreter for two-word parsing codes
 * from Diku.
 *
 * This code written by Locke.
 *
 * Returns information about the fill words (before and after first_arg).
 */
#define fill_word(argument) 	  ( is_number( argument )					\
                                    && fill_words(argument) )
#define MAX_FILL		10

char *	fill_words   [MAX_FILL] =
{
  "in",
  "from",
  "with",
  "the",
  "on",
  "at",
  "to",
  "here",
  "my",
  "a"
};

bool filler_words( char *argument )
{
	int x;

	for ( x = 0; x < MAX_FILL; x++ )
	{
		if ( !str_cmp( fill_words[x], argument ) ) return TRUE;
	}

	return FALSE;
}

#if defined(NEVER)
int argument_interpreter( char *argument, char *first_arg, char *second_arg,
						  int *x, int *y )
{
    int fills = 0;

    first_arg  = '\0';
    second_arg = '\0';
	x = 0;
	y = 0;

    while( *argument != '\0' && fill_word(first_arg) )
    {
        argument = one_argument( argument, first_arg );

        if ( *first_arg != '\0' )
        {
		if ( is_number( first_arg ) )		x = atoi( first_arg );
        if ( !str_cmp( first_arg, "in" )    SET_BIT(fills, FA_IN);
        if ( !str_cmp( first_arg, "on" )    SET_BIT(fills, FA_ON);
        if ( !str_cmp( first_arg, "here" )  SET_BIT(fills, FA_HERE);
        if ( !str_cmp( first_arg, "my" )    SET_BIT(fills, FA_MY);
        }
    }

    while( *argument != '\0' &&  fill_word(second_arg) )
    {
        argument = skip_spaces( argument );
        one_argument( argument, second_arg );

        if ( *second_arg != '\0' )
        {
		if ( is_number( second_arg ) )		y = atoi( first_arg );
        if ( !str_cmp( second_arg, "in" )   SET_BIT(fills, SA_IN);
        if ( !str_cmp( second_arg, "on" )   SET_BIT(fills, SA_ON);
        if ( !str_cmp( second_arg, "here" ) SET_BIT(fills, SA_HERE);
        if ( !str_cmp( second_arg, "my" )   SET_BIT(fills, SA_MY);
        }
    }

    return fills;
}



/*
 *	  "look 'at my sword in the pack here'"
 *	  "look 'sword'"
 *	  "look 'short sword'"
 *	  "look 'at a short sword'"

PROP *get_prop_nearby( PLAYER *ch, char *argument )
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
	int fBits;
	int x, y;

	fBits	  = argument_interpreter( argument, arg1, arg2, &x, &y );

	if ( fBits == 0 )
	{
		if ( arg2[0] == '\0' ) return get_prop_here( ch, arg1 );
		return get_prop_here( ch, argument );
	}



 */






#endif
