/******************************************************************************
 * Locke's   __                      __         NIM Server Software           *
 * ___ ___  (__)__    __ __   __ ___|  | v5     Version 5 (ALPHA)             *
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
#include <string.h>
#include <time.h>

#include <zlib.h>

#include "nimud.h"
#include "defaults.h"
#include "script.h"


/*
 * From mem.c
 */
extern VARIABLE_DATA * mud_var_list;


/*
 * Find a race.
 */
int race_lookup( int vnum )
{
    if ( vnum >= MAX_RACE || vnum <= 0 ) return 0; 
    return vnum;
}


/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust( PLAYER_DATA *ch )
{
    if ( ch == NULL ) return -1;

    if ( ch->userdata )
    return PC(ch,level);

    /*
     * Switch characters are trusted at their original level.
     * Doesn't count switching into players (which isn't allowed)
     */
    if ( ch->desc != NULL )
    {
       if ( ch->desc->original != NULL )
       ch = ch->desc->original;
    }

    if ( IS_NPC(ch) )
    return LEVEL_MORTAL;

    return -1;
}






/*
 * Retrieve character's current strength.
 */
int get_curr_str( PLAYER_DATA *ch )
{
    int max;

	max = 25;

    return URANGE( 3, ch->perm_str + ch->mod_str, max );
}




/*
 * Retrieve character's current intelligence.
 */
int get_curr_int( PLAYER_DATA *ch )
{
    int max;

	max = 25;

    return URANGE( 3, ch->perm_int + ch->mod_int, max );
}



/*
 * Retrieve character's current wisdom.
 */
int get_curr_wis( PLAYER_DATA *ch )
{
    int max;

	max = 25;

    return URANGE( 3, ch->perm_wis + ch->mod_wis, max );
}


/*
 * Retrieve character's current dexterity.
 */
int get_curr_dex( PLAYER_DATA *ch )
{
    int max;

	max = 25;

    return URANGE( 3, ch->perm_dex + ch->mod_dex, max );
}



/*
 * Retrieve character's current constitution.
 */
int get_curr_con( PLAYER_DATA *ch )
{
    int max;

	max = 25;

    return URANGE( 3, ch->perm_con + ch->mod_con, max );
}




/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( PLAYER_DATA *ch )
{
    if ( IS_IMMORTAL(ch) || (IS_NPC(ch) && ch->pIndexData->pShop) )
	return 1000000;

    return str_app[get_curr_str(ch)].carry;
}


/*
 * Apply or remove an affect to a character.
 */
void bonus_modify( PLAYER_DATA *ch, BONUS_DATA *paf, bool fAdd )
{
    PROP_DATA *prop, *prop_next;
    int mod;
    static int depth;

    mod = paf->modifier;

    if ( fAdd )
    {
	SET_BIT( ch->bonuses, paf->bitvector );
    }
    else
    {
        int race = race_lookup( ch->race );
        PROP_DATA *pr;

        REMOVE_BIT( ch->bonuses, paf->bitvector );

        SET_BIT(ch->bonuses, RACE(race,bonus_bits) );

        /*
         * Make check if your wearing two affects of the same bitvector.
         */
        for ( pr = ch->carrying;   pr != NULL;  pr = pr->next_content )
        {
            BONUS_DATA *af;

            if ( NOT_WORN(pr) )
             continue;

            for ( af = pr->bonus; af != NULL;  af = af->next )
            {
                if (IS_SET( af->bitvector, paf->bitvector ) )
                SET_BIT(ch->bonuses, af->bitvector);
            }
        }

        mod = 0 - mod;
    }

    if ( IS_NPC(ch) )
	return;

    switch ( paf->location )
    {
    default:
        {
            SKILL_DATA *pSkill;

            pSkill = find_skill_pc(ch, paf->location);
            if ( !pSkill ) {

                bug( "Bonus_modify: unknown location %d.", paf->location );
                paf->location = APPLY_NONE;
                return;
            }

            pSkill->skill_level += mod;
        }
     break;
    case APPLY_NONE:                                   break;
    case APPLY_STR:           ch->mod_str      += mod; break;
    case APPLY_DEX:           ch->mod_dex      += mod; break;
    case APPLY_INT:           ch->mod_int      += mod; break;
    case APPLY_WIS:           ch->mod_wis      += mod; break;
    case APPLY_CON:           ch->mod_con      += mod; break;
    case APPLY_SEX:           ch->sex          += mod; break;
    case APPLY_AGE:           PC(ch,age)       += mod; break;
    case APPLY_SIZE:          ch->size         += mod; break;
    case APPLY_AC:            ch->armor        += mod; break;
    case APPLY_HITROLL:       ch->hitroll      += mod; break;
    case APPLY_DAMROLL:       ch->damroll      += mod; break;
    case APPLY_SAVING_THROW:  ch->saving_throw += mod; break;
    }

    if ( depth == 0 )    /* prop_from/to_actor recurses */
    {
    depth++;
    for ( prop = ch->carrying;
          prop != NULL && can_carry_w( ch ) < ch->carry_weight;
          prop = prop_next )
    {
    prop_next = prop->next_content;
    
    act( "You drop $p.", ch, prop, NULL, TO_ACTOR );
    act( "$n drops $p.", ch, prop, NULL, TO_SCENE );
    unequip_char( ch, prop );
    prop_from_actor( prop );
    prop_to_scene( prop, ch->in_scene );
    }
    depth--;
    }
    return;
}



/*
 * Give an affect to a char.
 */
void bonus_to_actor( PLAYER_DATA *ch, BONUS_DATA *paf )
{
    BONUS_DATA *paf_new;


    paf_new = new_bonus( );

    *paf_new		= *paf;
    paf_new->next	= ch->bonus;
    ch->bonus	= paf_new;

    bonus_modify( ch, paf_new, TRUE );
    return;
}



/*
 * Remove an affect from a char.
 */
void bonus_remove( PLAYER_DATA *ch, BONUS_DATA *paf )
{
    if ( ch->bonus == NULL )
    {
	bug( "Bonus_remove: no affect.", 0 );
	return;
    }

    bonus_modify( ch, paf, FALSE );

    if ( paf == ch->bonus )
    {
	ch->bonus	= paf->next;
    }
    else
    {
	BONUS_DATA *prev;

	for ( prev = ch->bonus; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == paf )
	    {
		prev->next = paf->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Bonus_remove: cannot find paf.", 0 );
	    return;
	}
    }

    free_bonus( paf );
    return;
}



/*
 * Strip all affects of a given sn.
 */
void bonus_strip( PLAYER_DATA *ch, int sn )
{
    BONUS_DATA *paf;
    BONUS_DATA *paf_next;

    for ( paf = ch->bonus; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;
	if ( paf->type == sn )
	    bonus_remove( ch, paf );
    }

    return;
}



/*
 * Return true if a char is bonus by a spell.
 */
bool is_bonused( PLAYER_DATA *ch, int sn )
{
    BONUS_DATA *paf;

    for ( paf = ch->bonus; paf != NULL; paf = paf->next )
    {
	if ( paf->type == sn )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Add or enhance an affect.
 */
void bonus_join( PLAYER_DATA *ch, BONUS_DATA *paf )
{
    BONUS_DATA *paf_old;
    bool found;

    found = FALSE;
    for ( paf_old = ch->bonus; paf_old != NULL; paf_old = paf_old->next )
    {
	if ( paf_old->type == paf->type )
	{
	    paf->duration += paf_old->duration;
	    paf->modifier += paf_old->modifier;
	    bonus_remove( ch, paf_old );
	    break;
	}
    }

    bonus_to_actor( ch, paf );
    return;
}


/*
 * Move a char out of a scene.
 */
void actor_from_scene( PLAYER_DATA *ch )
{
    PROP_DATA *prop;

    set_furn( ch, NULL );

    if ( ch->in_scene == NULL )
    {
	bug( "Char_from_scene: NULL.", 0 );
	return;
    }

    if ( !IS_NPC(ch) )
	--ch->in_scene->zone->nplayer;

    for ( prop = ch->carrying; prop != NULL;  prop = prop->next_content )
    {

    if ( prop->item_type == ITEM_LIGHT
      && IS_LIT(prop)
      && ch->in_scene->light > 0 )    --ch->in_scene->light;
    }

    if ( ch == ch->in_scene->people )
    {
	ch->in_scene->people = ch->next_in_scene;
    }
    else
    {
	PLAYER_DATA *prev;

	for ( prev = ch->in_scene->people; prev; prev = prev->next_in_scene )
	{
	    if ( prev->next_in_scene == ch )
	    {
		prev->next_in_scene = ch->next_in_scene;
		break;
	    }
	}

	if ( prev == NULL )
	    bug( "Char_from_scene: ch not found.", 0 );
    }

    ch->in_scene      = NULL;
    ch->next_in_scene = NULL;
    return;
}



/*
 * Move a char into a scene.
 */
void actor_to_scene( PLAYER_DATA *ch, SCENE_INDEX_DATA *pSceneIndex )
{
    PROP_DATA *prop;

    if ( pSceneIndex == NULL )
    {
	bug( "Char_to_scene: NULL.", 0 );
        pSceneIndex = get_scene_index( SCENE_VNUM_DEATH );
    }

    ch->in_scene         = pSceneIndex;
    ch->next_in_scene	= pSceneIndex->people;
    pSceneIndex->people	= ch;

    if ( !IS_NPC(ch) )
	++ch->in_scene->zone->nplayer;

    for ( prop = ch->carrying; prop != NULL;  prop = prop->next_content )
    {   
    if ( prop->item_type == ITEM_LIGHT && IS_LIT(prop) )   ++ch->in_scene->light;
    }

    return;
}




/*
 * Move (intelligently) a prop to a char.
 */
void prop_to_actor_money( PROP_DATA *prop, PLAYER_DATA *ch )
{
    PROP_DATA *fprop;

    if ( prop->item_type == ITEM_MONEY  )
    {
    for ( fprop = ch->carrying;  fprop != NULL; fprop = fprop->next_content )
    {
        if ( fprop->item_type == ITEM_CONTAINER )
        {
            PROP_DATA *iprop;

            for ( iprop = fprop->contains; iprop != NULL; iprop = iprop->next_content )
            {
                if ( iprop->item_type == ITEM_MONEY )
                {
                    break;
                }
            }
            if ( iprop == NULL )
            continue;

            prop_to_prop( prop, fprop );
            merge_money_prop( fprop );
            return;
        }
    }

    if ( fprop == NULL )
    {
    for ( fprop = ch->carrying;  fprop != NULL; fprop = fprop->next_content )
    {
        if ( fprop->item_type == ITEM_CONTAINER )
        {
            prop_to_prop( prop, fprop );
            merge_money_prop( fprop );
            return;
        }
    }
    }
    }

    if ( prop->item_type == ITEM_LIGHT
    &&   IS_LIT(prop)
    &&   ch->in_scene != NULL )
    ++ch->in_scene->light;

    prop->next_content = ch->carrying;
    ch->carrying      = prop;
    prop->carried_by     = ch;
    prop->in_scene        = NULL;
    prop->in_prop         = NULL;
    ch->carry_number   += get_prop_number( prop );
    ch->carry_weight   += get_prop_weight( prop );

}


/*
 * Give a prop to a char.
 */
void prop_to_actor( PROP_DATA *prop, PLAYER_DATA *ch )
{
    if ( prop->item_type == ITEM_LIGHT
    &&   IS_LIT(prop)
    &&   ch->in_scene != NULL )
    ++ch->in_scene->light;

    prop->next_content = ch->carrying;
    ch->carrying      = prop;
    prop->carried_by     = ch;
    prop->in_scene        = NULL;
    prop->in_prop         = NULL;
    ch->carry_number   += get_prop_number( prop );
    ch->carry_weight   += get_prop_weight( prop );
    REMOVE_BIT(prop->extra_flags, ITEM_USED);
    return;
}



/*
 * Take a prop from its character.
 */
void prop_from_actor( PROP_DATA *prop )
{
    PLAYER_DATA *ch;

    if ( ( ch = prop->carried_by ) == NULL )
    {
	bug( "Obj_from_char: null ch.", 0 );
	return;
    }

    if ( (prop->wear_loc != WEAR_NONE
       && prop->wear_loc != WEAR_HOLD_1
       && prop->wear_loc != WEAR_HOLD_2
       && prop->wear_loc != WEAR_BELT_1
       && prop->wear_loc != WEAR_BELT_2
       && prop->wear_loc != WEAR_BELT_3
       && prop->wear_loc != WEAR_BELT_4
       && prop->wear_loc != WEAR_BELT_5)
      || IS_SET(prop->extra_flags,ITEM_USED) )        unequip_char( ch, prop );

    if ( ch->carrying == prop )
    {
	ch->carrying = prop->next_content;
    }
    else
    {
	PROP_DATA *prev;

	for ( prev = ch->carrying; prev != NULL; prev = prev->next_content )
	{
	    if ( prev->next_content == prop )
	    {
		prev->next_content = prop->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	    bug( "Obj_from_char: prop not in list.", 0 );
    }

    if ( prop->item_type == ITEM_LIGHT
    &&   IS_LIT(prop)
    &&   ch->in_scene != NULL
    &&   ch->in_scene->light > 0 )
    --ch->in_scene->light;


    prop->carried_by	 = NULL;
    prop->next_content	 = NULL;
    ch->carry_number	-= get_prop_number( prop );
    ch->carry_weight	-= get_prop_weight( prop );
    return;
}




/*
 * Find the ac value of a prop, including position effect.
 */
int apply_ac( PROP_DATA *prop, int iWear )
{
    if ( prop->item_type != ITEM_ARMOR )
	return 0;

    return prop->value[0];
}


/*
 * Get a prop from a char using a type reference.
 * Goes 1 deep.  Returns 1st instance.
 */
PROP_DATA *get_item_char( PLAYER_DATA *ch, int itype )
{  
    PROP_DATA *prop, *prop2;

    for ( prop = ch->carrying;  prop != NULL; prop = prop->next_content )
    {
        if ( prop->item_type == itype ) return prop;

        for ( prop2 = prop->contains;  prop2 != NULL; prop2 = prop2->next_content )
        { if ( prop2->item_type == itype ) return prop2; }
    }
  
    return NULL;
}

/*
 * Get a prop from a char using a type reference.
 * Goes 1 deep.  Returns 1st instance.
 */
PROP_DATA *get_tool_char( PLAYER_DATA *ch, int nbit )
{  
    PROP_DATA *prop, *prop2;

    for ( prop = ch->carrying;  prop != NULL; prop = prop->next_content )
    {
        if ( prop->item_type == ITEM_TOOL 
          && IS_SET(prop->value[0], nbit) ) return prop;

        for ( prop2 = prop->contains;  prop2 != NULL; prop2 = prop2->next_content )
        if ( prop2->item_type == ITEM_TOOL
          && IS_SET(prop2->value[0], nbit) ) return prop2;
    }
  
    return NULL;
}


/*
 * Get a prop from a char using type referencing.
 */
PROP_DATA *get_item_held( PLAYER_DATA *ch, int itype )
{
    PROP_DATA *prop;

    if ( ( prop = get_eq_char( ch, WEAR_HOLD_1 ) ) != NULL
        && prop->item_type == itype )
        return prop;

    if ( ( prop = get_eq_char( ch, WEAR_WIELD_1 ) ) != NULL
        && prop->item_type == itype )
        return prop;

    if ( ( prop = get_eq_char( ch, WEAR_HOLD_2 ) ) != NULL
        && prop->item_type == itype )
        return prop;

    if ( ( prop = get_eq_char( ch, WEAR_WIELD_2 ) ) != NULL
        && prop->item_type == itype )
        return prop;

    return NULL;
}


/*
 * Find a piece of eq on a character.
 */
PROP_DATA *get_eq_char( PLAYER_DATA *ch, int iWear )
{
    PROP_DATA *prop;

    for ( prop = ch->carrying; prop != NULL; prop = prop->next_content )
    {
	if ( prop->wear_loc == iWear )
	    return prop;
    }

    return NULL;
}




/*
 *  Return if a prop can be put in their hands (hands empty?)
 */
int hand_empty( PLAYER_DATA *ch )
{
    PROP_DATA *h1, *h2;

    h1 = get_eq_char( ch, WEAR_HOLD_1 );
    if ( !h1 ) h1 = get_eq_char( ch, WEAR_WIELD_1 );

    h2 = get_eq_char( ch, WEAR_HOLD_2 );
    if ( !h2 ) h2 = get_eq_char( ch, WEAR_WIELD_2 );

    if ( (h1 != NULL && IS_SET( h1->wear_flags, ITEM_TWO_HANDED ))
      || (h2 != NULL && IS_SET( h2->wear_flags, ITEM_TWO_HANDED )) )
            return WEAR_NONE;

    if ( h1 == NULL ) return WEAR_HOLD_1;
    if ( h2 == NULL ) return WEAR_HOLD_2;

    return WEAR_NONE;
}



/*
 * Return the denominator to the empty wield slot on a character.
 */
int wield_free( PLAYER_DATA *ch, PROP_DATA *prop )
{
    PROP_DATA *h1, *h2, *sh;

    if ( !prop ) return WEAR_NONE;

    if ( IS_SET(prop->wear_flags,ITEM_WEAR_SHIELD) ) {
    sh = get_eq_char( ch, WEAR_SHIELD );
    return !sh ? WEAR_SHIELD : WEAR_NONE;
    }

    h1 = get_eq_char( ch, WEAR_HOLD_1 );
    if ( h1 == NULL ) h1 = get_eq_char( ch, WEAR_WIELD_1 );
    h2 = get_eq_char( ch, WEAR_HOLD_2 );
    if ( h2 == NULL ) h2 = get_eq_char( ch, WEAR_WIELD_2 );

    if ( (h1 != NULL 
        && IS_SET( h1->wear_flags, ITEM_TWO_HANDED ) 
        && h1->wear_loc == WEAR_WIELD_1)
      || (h2 != NULL 
        && IS_SET( h2->wear_flags, ITEM_TWO_HANDED ) 
        && h2->wear_loc == WEAR_WIELD_2) )
    return WEAR_NONE;

    if ( h1 == NULL || h1 == prop ) return WEAR_WIELD_1;
    if ( h2 == NULL || h2 == prop ) return WEAR_WIELD_2;

    return WEAR_NONE;
}


/*
 *  Return if a prop can be put in their belt (hands empty?)
 */
int belt_empty( PLAYER_DATA *ch )
{
    if ( get_eq_char( ch, WEAR_WAIST )  == NULL )        return WEAR_NONE;

    if ( get_eq_char( ch, WEAR_BELT_1 ) == NULL )        return WEAR_BELT_1;
    if ( get_eq_char( ch, WEAR_BELT_2 ) == NULL )        return WEAR_BELT_2;
    if ( get_eq_char( ch, WEAR_BELT_3 ) == NULL )        return WEAR_BELT_3;
    if ( get_eq_char( ch, WEAR_BELT_4 ) == NULL )        return WEAR_BELT_4;
    if ( get_eq_char( ch, WEAR_BELT_5 ) == NULL )        return WEAR_BELT_5;

    return WEAR_NONE;
}


/*
 * Unequip a char with a prop.
 */
bool unequip_char( PLAYER_DATA *ch, PROP_DATA *prop )
{
    BONUS_DATA *paf;

    if ( (prop->wear_loc == WEAR_NONE) )
    {
        prop->wear_loc = hand_empty( ch );
        return TRUE;
    }

    if ( (prop->wear_loc == WEAR_NONE
      || prop->wear_loc == WEAR_HOLD_1
      || prop->wear_loc == WEAR_HOLD_2
      || prop->wear_loc == WEAR_BELT_1
      || prop->wear_loc == WEAR_BELT_2
      || prop->wear_loc == WEAR_BELT_3
      || prop->wear_loc == WEAR_BELT_4
      || prop->wear_loc == WEAR_BELT_5)
      && !IS_SET(prop->extra_flags, ITEM_USED) )
    {
    return TRUE;
    }

    for ( paf = prop->pIndexData->bonus; paf != NULL; paf = paf->next )
    bonus_modify( ch, paf, FALSE );
    for ( paf = prop->bonus; paf != NULL; paf = paf->next )
    bonus_modify( ch, paf, FALSE );

    ch->armor       += prop->value[0];
    prop->wear_loc    = hand_empty( ch );
    REMOVE_BIT(prop->extra_flags, ITEM_USED);

    return TRUE;
}

/*
 * Equip a char with a prop.
 */
void equip_char( PLAYER_DATA *ch, PROP_DATA *prop, int iWear )
{
    BONUS_DATA *paf;

    if ( (prop->wear_loc != WEAR_NONE
       && prop->wear_loc != WEAR_HOLD_1
       && prop->wear_loc != WEAR_HOLD_2
       && prop->wear_loc != WEAR_BELT_1
       && prop->wear_loc != WEAR_BELT_2
       && prop->wear_loc != WEAR_BELT_3
       && prop->wear_loc != WEAR_BELT_4
       && prop->wear_loc != WEAR_BELT_5)
      || IS_SET(prop->extra_flags, ITEM_USED)
      || get_eq_char( ch, iWear ) != NULL )
    {
        bug( "Equip_char: already equipped (%d).", iWear );
        return;
    }

    if ( iWear == WEAR_NONE
      || iWear == WEAR_HOLD_1
      || iWear == WEAR_HOLD_2
      || iWear == WEAR_BELT_1
      || iWear == WEAR_BELT_2
      || iWear == WEAR_BELT_3
      || iWear == WEAR_BELT_4
      || iWear == WEAR_BELT_5)
    {
       bug( "Equip_char: equipped how? (%d)", iWear );
       return;
    }

    for ( paf = prop->pIndexData->bonus; paf != NULL; paf = paf->next )
	bonus_modify( ch, paf, TRUE );
    for ( paf = prop->bonus; paf != NULL; paf = paf->next )
	bonus_modify( ch, paf, TRUE );

    ch->armor           -= prop->value[0];
    prop->wear_loc	 = iWear;
    SET_BIT(prop->extra_flags, ITEM_USED);

    return;
}


/*
 * Count occurrences of a prop in a list.
 */
int count_prop_list( PROP_INDEX_DATA *pPropIndex, PROP_DATA *list )
{
    PROP_DATA *prop;
    int nMatch;

    nMatch = 0;
    for ( prop = list; prop != NULL; prop = prop->next_content )
    {
	if ( prop->pIndexData == pPropIndex )
	    nMatch++;
    }

    return nMatch;
}



/*
 * Move a prop out of a scene.
 */
void prop_from_scene( PROP_DATA *prop )
{
    SCENE_INDEX_DATA *in_scene;

    if ( ( in_scene = prop->in_scene ) == NULL )
    {
	bug( "prop_from_scene: NULL.", 0 );
	return;
    }

    if ( prop == in_scene->contents )
    {
	in_scene->contents = prop->next_content;
    }
    else
    {
	PROP_DATA *prev;

	for ( prev = in_scene->contents; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == prop )
	    {
		prev->next_content = prop->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_scene: prop not found.", 0 );
	    return;
	}
    }

    if ( prop->item_type == ITEM_LIGHT && IS_LIT(prop) )  --prop->in_scene->light;

    prop->in_scene      = NULL;
    prop->next_content = NULL;
    return;
}



/*
 * Move a prop into a scene.
 */
void prop_to_scene( PROP_DATA *prop, SCENE_INDEX_DATA *pSceneIndex )
{
    if ( prop->item_type == ITEM_LIGHT && IS_LIT(prop) )  ++pSceneIndex->light;

    prop->next_content		= pSceneIndex->contents;
    pSceneIndex->contents	= prop;
    prop->in_scene		= pSceneIndex;
    prop->carried_by		= NULL;
    prop->in_prop			= NULL;
    prop->wear_loc       = WEAR_NONE;

    return;
}



/*
 * Move a prop into a prop.
 */
void prop_to_prop( PROP_DATA *prop, PROP_DATA *prop_to )
{
    prop->next_content   = prop_to->contains;
    prop_to->contains    = prop;
    prop->in_prop			= prop_to;
    prop->in_scene		= NULL;
    prop->carried_by		= NULL;
    prop->wear_loc       = WEAR_NONE;

    for ( ; prop_to != NULL; prop_to = prop_to->in_prop )
    {
	if ( prop_to->carried_by != NULL )
	{
	    prop_to->carried_by->carry_number += get_prop_number( prop );
	    prop_to->carried_by->carry_weight += get_prop_weight( prop );
	}
    }

    return;
}



/*
 * Move a prop out of a prop.
 */
void prop_from_prop( PROP_DATA *prop )
{
    PROP_DATA *prop_from;

    if ( ( prop_from = prop->in_prop ) == NULL )
    {
	bug( "Prop_from_prop: null prop_from.", 0 );
	return;
    }

    if ( prop == prop_from->contains )
    {
	prop_from->contains = prop->next_content;
    }
    else
    {
	PROP_DATA *prev;

	for ( prev = prop_from->contains; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == prop )
	    {
		prev->next_content = prop->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Prop_from_prop: prop not found.", 0 );
	    return;
	}
    }

    prop->next_content = NULL;
    prop->in_prop       = NULL;

    for ( ; prop_from != NULL; prop_from = prop_from->in_prop )
    {
	if ( prop_from->carried_by != NULL )
	{
	    prop_from->carried_by->carry_number -= get_prop_number( prop );
	    prop_from->carried_by->carry_weight -= get_prop_weight( prop );
	}
    }

    return;
}



/*
 * Extract a prop from the world.
 */
void extract_prop( PROP_DATA *prop )
{
    PROP_DATA *prop_content;
    PROP_DATA *prop_next;
    extern PROP_DATA *prop_free;
    VARIABLE_DATA *pVar;
    VARIABLE_DATA *pVar_next;

    if ( !prop ) return;

    {
        PLAYER_DATA *actor;

        for ( actor = actor_list;  actor != NULL; actor = actor->next )
        {
            if ( actor->furniture == prop )
            actor->furniture = NULL;

            if ( actor->hitched_to == prop )
            actor->hitched_to = NULL;
        }
    }

    for ( pVar = mud_var_list;  pVar != NULL;  pVar = pVar_next )
    {
        pVar_next = pVar->next;
        if ( pVar->type == TYPE_PROP && (PROP_DATA *)pVar->value == prop )
        {
            pVar->type  = TYPE_STRING;
            pVar->value = str_dup( "" );
        }
    }


    if ( prop->in_scene != NULL )   prop_from_scene( prop );
    if ( prop->carried_by != NULL ) prop_from_actor( prop );
    if ( prop->in_prop != NULL )    prop_from_prop( prop );

    for ( prop_content = prop->contains; prop_content; prop_content = prop_next )
    {
	prop_next = prop_content->next_content;
	extract_prop( prop->contains );
    }

    if ( prop_list == prop )
    {
	prop_list = prop->next;
    }
    else
    {
	PROP_DATA *prev;

	for ( prev = prop_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == prop )
	    {
		prev->next = prop->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Extract_prop: prop %d not found.", prop->pIndexData->vnum );
	    return;
	}
    }

    {
	BONUS_DATA *paf;
	BONUS_DATA *paf_next;

	for ( paf = prop->bonus; paf != NULL; paf = paf_next )
	{
	    paf_next    = paf->next;

        free_bonus( paf );
	}
    }

    {
	EXTRA_DESCR_DATA *ed;
	EXTRA_DESCR_DATA *ed_next;

	for ( ed = prop->extra_descr; ed != NULL; ed = ed_next )
	{
	    ed_next		= ed->next;

        free_extra_descr( ed );
	}
    }

    free_string( prop->name        );
    free_string( prop->description );
    free_string( prop->description_plural );
    free_string( prop->short_descr );
    free_string( prop->short_descr_plural );
    free_string( prop->action_descr );
    free_string( prop->real_description );
    --prop->pIndexData->count;
    prop->next	= prop_free;
    prop_free	= prop;
    return;
}



/*
 * Extract a char from the world.
 */
void extract_char( PLAYER_DATA *ch, bool fPull )
{
    PLAYER_DATA *wch, *wch_next;
    PROP_DATA *prop;
    PROP_DATA *prop_next;
    VARIABLE_DATA *pVar;
    VARIABLE_DATA *pVar_next;

    if ( ch->in_scene == NULL )
    {
	bug( "Extract_char: NULL in_scene.", 0 );
	return;
    }

    if ( fPull )
    {
        PLAYER_DATA *pet;
        PLAYER_DATA *pet_next;

        for ( pet = actor_list; pet != NULL; pet = pet_next )
        {
            pet_next = pet->next;
            if ( ch != pet
              && ( pet->master == ch )
              && IS_SET(pet->act, ACT_PET) )
            extract_char( pet, TRUE );
        }
    }

    for ( pVar = mud_var_list;  pVar != NULL;  pVar = pVar_next )
    {
        pVar_next = pVar->next_master_var;
        if ( pVar->type == TYPE_ACTOR && (PLAYER_DATA *)pVar->value == ch )
        {
            pVar->type  = TYPE_STRING;
            pVar->value = str_dup( "" );
        }
    }

    {
        PLAYER_DATA *clist;

        for ( clist = actor_list;  clist != NULL;  clist = clist->next )
        {
            if ( clist->userdata != NULL )
            {
                if ( clist->userdata->trackscr == (void *)ch )
                    cmd_script( clist, "" );
            }
        }
    }

    if ( ch->haggling != NULL )
    {
        if ( ch->haggling->carried_by == NULL )
        extract_prop( ch->haggling );
        ch->haggling = NULL;
    }

    if ( fPull )
	die_follower( ch );

    stop_fighting( ch, TRUE );

    for ( prop = ch->carrying; prop != NULL; prop = prop_next )
    {
	prop_next = prop->next_content;
	extract_prop( prop );
    }
    
    actor_from_scene( ch );
    
    if ( ch->riding != NULL )
    {
       ch->riding->rider = NULL;
       ch->riding             = NULL;
    }
    
    if ( ch->rider != NULL )
    {
       ch->rider->riding = NULL;
       ch->rider         = NULL;
    }
    
    if ( !IS_NPC(ch) )
        PC(ch,condition)[COND_THIRST] = 100;

    
    if ( !fPull )
    {
    actor_to_scene( ch, get_scene_index( SCENE_VNUM_DEATH ) );
    cmd_look( ch, "auto" );
	return;
    }

    if ( IS_NPC(ch) )
	--ch->pIndexData->count;

    if ( ch->desc != NULL && ch->desc->original != NULL )
	cmd_return( ch, "" );

    if ( ch->desc != NULL ) ch->desc->snoop_by = NULL;


    if ( ch == actor_list )
    {
       actor_list = ch->next;
    }
    else
    {
	PLAYER_DATA *prev;

	for ( prev = actor_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == ch )
	    {
		prev->next = ch->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Extract_char: char not found.", 0 );
	    return;
	}
    }

    for ( wch = actor_list; wch != NULL; wch = wch_next ) { wch_next = wch->next;
     if ( wch->master == ch && IS_SET(wch->act,ACT_PET)  ) {

        /*
         * Clip from mount list (on first).
         */
        if ( wch == actor_list )
        {
           actor_list = actor_list->next;
        }
        else
        {
            PLAYER_DATA *prev;

            for ( prev = actor_list; prev != NULL; prev = prev->next )
            {
                /*
                 * Clip from list.
                 */
                if ( prev->next == wch )
                {
                    prev->next = wch->next;
                    break;
                }
            }

            if ( prev == NULL && wch_next != NULL )
            {
                bug( "Extract_char: mount not found.", 0 );
                return;
            }
        }
        /*
         * Watch for recursion.
         */
        extract_char( wch, TRUE );
      }
    }

    if ( ch->desc )
	ch->desc->character = NULL;
    free_char( ch );
    return;
}



/*
 * Find a char in the scene.
 */
PLAYER_DATA *get_actor_scene( PLAYER_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PLAYER_DATA *rch;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;

    if ( !str_cmp( arg, "self" ) || !str_cmp( arg, "me" ) )
	return ch;

    if ( !str_cmp( arg, "mount" ) )
    return ch->riding;

    for ( rch = ch->in_scene->people; rch != NULL; rch = rch->next_in_scene )
    {
        if ( ch == rch && count <= 1 )
        continue;
        
        if ( !can_see( ch, rch ) )
        continue;

        if ( !is_name( arg, STR(rch, name) ) )
        {
            if ( !IS_NPC(rch) && is_name( arg, rch->keywords ) )
            ;
            else continue;
        }

        if ( ++count == number )
            return rch;
    }
    
    if ( !is_name( arg, STR(ch, name) ) )
    {
        if ( !IS_NPC(ch) && is_name( arg, ch->keywords ) )
        return ch;
    }   

    return NULL;
}




/*
 * Find a char in the world.
 */
PLAYER_DATA *get_actor_world( PLAYER_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PLAYER_DATA *wch;
    int number;
    int count;

    if ( ( wch = get_actor_scene( ch, argument ) ) != NULL )
	return wch;

    number = number_argument( argument, arg );
    count  = 0;
    for ( wch = actor_list; wch != NULL ; wch = wch->next )
    {
        if ( !can_see( ch, wch ) )
        continue;

        if ( !is_name( arg, STR(wch, name) ) )
        {
            if ( !IS_NPC(wch) && is_name( arg, wch->keywords ) )
            ;
            else continue;
        }

        if ( ++count == number )
            return wch;
    }

    return NULL;
}



/*
 * Find some prop with a given index data.
 * Used by zone-reset 'P' command.
 */
PROP_DATA *get_prop_type( PROP_INDEX_DATA *pPropIndex )
{
    PROP_DATA *prop;

    for ( prop = prop_list; prop != NULL; prop = prop->next )
    {
	if ( prop->pIndexData == pPropIndex )
	    return prop;
    }

    return NULL;
}



      
/*
 * Return # of props which a prop counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_prop_number( PROP_DATA *prop )
{
    int number;

    if ( prop->item_type == ITEM_CONTAINER )
	number = 0;
    else
	number = 1;

    for ( prop = prop->contains; prop != NULL; prop = prop->next_content )
	number += get_prop_number( prop );

    return number;
}



/*
 * Return weight of a prop, including weight of contents.
 * Danger: Recursion ahead.
 */
int get_prop_weight( PROP_DATA *prop )
{
    int weight;

    weight = prop->weight;
    if ( prop->item_type == ITEM_DRINK_CON )
    weight += prop->value[0]/10;

    for ( prop = prop->contains; prop != NULL; prop = prop->next_content )
	weight += get_prop_weight( prop );

    return weight;
}



/*
 * True if scene is dark.
 */
int scene_is_dark( SCENE_INDEX_DATA *pSceneIndex )
{
    if ( pSceneIndex == NULL ) return FALSE;

    /*
     * Someone gotta light?
     */
    if ( pSceneIndex->light > 0 )
    return FALSE;

    /*
     * Is it supposed to be dark here?
     */
    if ( IS_SET(pSceneIndex->scene_flags, SCENE_DARK) )
    return TRUE;

    /*
     * Are we inside?
     */
    if ( pSceneIndex->sector_type == SECT_INSIDE )
    return FALSE;

    /*
     * If it is daylight.
     */
    if ( weather_info.sunlight == SUN_RISE
      || weather_info.sunlight == SUN_LIGHT )
    return FALSE;

    /*
     * If the moon is out and you're in the city or a field.
     */
    if ( weather_info.sunlight == MOON_RISE
      && weather_info.sky == SKY_CLOUDLESS
      && (pSceneIndex->sector_type == SECT_FIELD
       || pSceneIndex->sector_type == SECT_CITY) )
    return FALSE;

    /* 
     * If the sky is cloudy and you're in the forest..
     */
    if ( weather_info.sky != SKY_CLOUDLESS
      && pSceneIndex->sector_type == SECT_FOREST )
    return TRUE;
 

    return FALSE;
}



/*
 * True if scene is private.
 */
bool scene_is_private( SCENE_INDEX_DATA *pSceneIndex )
{
    PLAYER_DATA *rch;
    int count;

    if ( pSceneIndex->max_people == 0 )
    return FALSE;

    count = 0;
    for ( rch = pSceneIndex->people; rch != NULL; rch = rch->next_in_scene )
	count++;

    if ( count >= pSceneIndex->max_people )
	return TRUE;

    return FALSE;
}



/*
 * True if char can see victim.
 */
bool can_see( PLAYER_DATA *ch, PLAYER_DATA *victim )
{
    if ( ch == victim )
	return TRUE;
    
    if ( !IS_NPC(ch) && IS_SET(ch->act2, WIZ_HOLYLIGHT) )
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
	return FALSE;

    if ( scene_is_dark( ch->in_scene ) && !IS_AFFECTED(ch, AFF_INFRARED) )
	return FALSE;

/*
    if ( IS_NPC(ch) && HAS_SCRIPT(ch) && !IS_SET(ch->act, ACT_AGGRESSIVE ) )
    return TRUE;
 */

    if ( IS_AFFECTED(victim, AFF_INVISIBLE)
    &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
	return FALSE;

    if ( IS_AFFECTED(victim, AFF_HIDE)
    &&   !IS_AFFECTED(ch, AFF_DETECT_HIDDEN)
    &&   victim->fighting == NULL
    &&   ( IS_NPC(ch) ? !IS_NPC(victim) : IS_NPC(victim) ) )
	return FALSE;

    if (  get_trust( ch ) < GET_PC(victim,wizinvis,0)
       && get_trust( ch ) < get_trust( victim ) )
    return FALSE;

    return TRUE;
}



/*
 * True if char can see prop.
 */
bool can_see_prop( PLAYER_DATA *ch, PROP_DATA *prop )
{
    if ( !IS_NPC(ch) && IS_SET(ch->act2, WIZ_HOLYLIGHT) )
	return TRUE;

    if ( IS_AFFECTED( ch, AFF_BLIND ) )
    return FALSE;

    if ( IS_SET( prop->extra_flags, ITEM_BURNING ) )
    return TRUE;

    if ( prop->item_type == ITEM_LIGHT && IS_LIT( prop ) )
	return TRUE;

    if ( scene_is_dark( ch->in_scene ) && !IS_AFFECTED(ch, AFF_INFRARED) )
	return FALSE;

    if ( IS_SET(prop->extra_flags, ITEM_INVIS)
    &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
	return FALSE;

    return TRUE;
}



/*
 * True if char can drop prop.
 */
bool can_drop_prop( PLAYER_DATA *ch, PROP_DATA *prop )
{
    if ( !IS_SET(prop->extra_flags, ITEM_NODROP) )
	return TRUE;

    if ( IS_IMMORTAL(ch) )
	return TRUE;

    if ( NOT_WORN( prop ) )
    return TRUE;

    return FALSE;
}


/*
 * Get an extra description from a list.
 */
char *get_extra_descr( const char *name, EXTRA_DESCR_DATA *ed )
{
    for ( ; ed != NULL; ed = ed->next )
    {
	if ( is_name( name, ed->keyword ) )
        return ed->description;
    }
    return NULL;
}



/*
 * Use up some of a tool.
 */
bool use_tool( PROP_DATA *prop, int bit )
{
    if ( prop == NULL
      || prop->item_type != ITEM_TOOL
      || !VAL_SET(prop, 0, bit) )
    return FALSE;

    if ( prop->value[1] <= 0 )
    return FALSE;

    prop->value[1]--;
    return TRUE;
}



/*
 * Find a percentage, with error checking.
 */
int PERCENTAGE( int amount, int max )
{
    if (max > 0) return (amount*100)/max;

    return amount*100;
}



/*
 * Check the wield weight of a actor, for quick equations.
 */
int wield_weight( PLAYER_DATA *ch )
{
    int weight;
    PROP_DATA *w1, *w2, *sh;

    w1 = get_eq_char(ch, WEAR_WIELD_1);
    w2 = get_eq_char(ch, WEAR_WIELD_2);
    sh = get_eq_char(ch, WEAR_SHIELD);

    weight = 0;
    if ( w1 ) weight += w1->weight;
    if ( w2 ) weight += w2->weight;
    if ( sh ) weight += sh->weight;

    return weight;
}




bool is_hooded( PLAYER_DATA *ch )
{
    PLAYER_DATA *looker;
    PROP_DATA *hood;
    
    
    return FALSE;

    for ( looker = ch->in_scene->people; 
          looker != NULL; 
          looker = looker->next_in_scene ) {
        if( ( hood = get_eq_char( ch, WEAR_ABOUT ) ) != NULL
        && can_see_prop( looker, hood ) && hood->item_type == ITEM_CLOTHING
        && IS_SET( hood->value[1], HOODED )
        && IS_SET( hood->value[1], HOOD_UP ) )
            return TRUE;
    }
    return FALSE;
}
