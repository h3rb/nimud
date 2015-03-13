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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <zlib.h>

#include "nimud.h"
#include "script.h"
#include "skills.h"
#include "defaults.h"

#define IS_FULL         75
#define IS_DRUNK        40


#define INV(loc)       ( (loc == WEAR_NONE || loc == WEAR_HOLD_1 ||     \
                         loc == WEAR_HOLD_2 || loc == WEAR_WIELD_1 ||   \
                         loc == WEAR_WIELD_2 ) )

#define NOTPET(wb)        ( (NPC(ch) \
                          && IS_SET(ch->flag, ACTOR_PET)           \
                          && wb == WEAR_BELT_1                  \
                          && wb == WEAR_BELT_2                  \
                          && wb == WEAR_BELT_3                  \
                          && wb == WEAR_BELT_4                  \
                          && wb == WEAR_BELT_5 ) )
#define CAN_WEAR_LOC(ch,wb)   ( TRUE )




/*
 * Kludgy check for the "hidden in robes" inventory
 * catch versus "hand-empty".
 *
 * Called at the cmd_ level
 */
void inv_to_hand_check( PLAYER *ch ) {
   PROP *prop;
   int empty = hand_empty( ch );;

   if ( empty == WEAR_NONE )
   return;

   for ( prop = ch->carrying;  prop != NULL; prop=prop->next_content ) {
         if ( prop->wear_loc == WEAR_NONE ) {
               prop->wear_loc = empty;
               return;
         }
   }
  
   return;
}




bool can_wield( PLAYER *ch, PROP *prop, bool fSilent )
{
    int weight;
    PROP *w1, *w2, *sh;

    if ( prop->item_type == ITEM_WEAPON )
    {
        if ( wield_free( ch, prop ) != WEAR_WIELD_1
          && IS_SET(prop->wear_flags, ITEM_TWO_HANDED) )
        {
            if ( !fSilent ) {
	    if ( prop->wear_loc != WEAR_HOLD_1 )
            act( "You need to have $p in your primary hand, and two hands free.", ch, prop, NULL, TO_ACTOR );
 	    else
            act( "$p requires two hands to be wielded correctly.",
                 ch, prop, NULL, TO_ACTOR );
            }
            return FALSE;
        }


        if ( wield_free( ch, prop ) == WEAR_NONE )
        return FALSE;

    }
    else
    if ( IS_SET(prop->wear_flags,ITEM_WEAR_SHIELD) )
    {
        if ( get_eq_char( ch, WEAR_SHIELD ) != NULL )
        {
            if ( !fSilent ) to_actor( "You are already using a shield.", ch );
            return FALSE;
        }
    }
    else
    {
        if ( !fSilent && prop->item_type == ITEM_RANGED_WEAPON ) 
        act( "Just holding a $p in your hands is the same as wielding it.", 
             ch, prop, NULL, TO_ACTOR );
        else
        if ( !fSilent ) act( "$p is not a weapon.", ch, prop, NULL, TO_ACTOR );

        return FALSE;
    }


    if ( str_app[get_curr_str(ch)].wield < prop->weight )
    {
        if ( !fSilent ) act( "You are not strong enough to wield $p.", ch, prop, NULL, TO_ACTOR );
        return FALSE;
    }

    w1 = get_eq_char(ch, WEAR_WIELD_1);
    w2 = get_eq_char(ch, WEAR_WIELD_2);
    sh = get_eq_char(ch, WEAR_SHIELD);

    weight = 0;
    if ( w1 ) weight += w1->weight;
    if ( w2 ) weight += w2->weight;
    if ( sh ) weight += sh->weight;

    if ( str_app[get_curr_str(ch)].wield < weight + prop->weight )
    {
        if ( (w1 || w2) && sh && !fSilent )
        act( "You are not strong enough to use a shield, a weapon and $p.",
             ch, prop, NULL, TO_ACTOR );
        else
        if ( (w1 || w2) && !fSilent )
        act( "You are too weak to wield $p and $P at the same time.",
             ch, prop, (w1==NULL)?w2:w1, TO_ACTOR );
        else if ( !fSilent )
        act( "You lack the strength to wield $p.", ch, prop, NULL, TO_ACTOR );
        return FALSE;
    }

    return TRUE;
}


bool draw_prop( PLAYER *ch, PROP *prop )
{
   if ( prop->wear_loc != WEAR_BELT_1
     && prop->wear_loc != WEAR_BELT_2
     && prop->wear_loc != WEAR_BELT_3
     && prop->wear_loc != WEAR_BELT_4
     && prop->wear_loc != WEAR_BELT_5 )
   return FALSE;

   if ( hand_empty( ch ) == WEAR_NONE )
   return FALSE;

   if ( !can_wield(ch, prop, FALSE) )
   return FALSE;

   equip_char( ch, prop, wield_free( ch, prop ) );

   act( "$n draws $p from $s belt.", ch, prop, NULL, TO_SCENE );
   act( "You draw $p from your belt.", ch, prop, NULL, TO_ACTOR );
   return TRUE;
}


bool sheath_prop( PLAYER *ch, PROP *prop, bool fSilent )
{
   if ( prop == NULL )
   return FALSE;
   
   if ( !CAN_WEAR_LOC(ch,WEAR_BELT_1) )
   return FALSE;

   if ( !IS_SET( prop->wear_flags, ITEM_WEAR_BELT ) )
   return FALSE;

   if ( get_eq_char( ch, WEAR_WAIST ) == NULL )
   {
       if ( !fSilent ) to_actor( "You need a belt to hold your weapons.\n\r", ch );
       return TRUE;
   }

   if ( prop->wear_loc != WEAR_NONE
     && prop->wear_loc != WEAR_HOLD_1
     && prop->wear_loc != WEAR_HOLD_2
     && prop->wear_loc != WEAR_WIELD_1
     && prop->wear_loc != WEAR_WIELD_2 )
   return FALSE;

   if ( belt_empty( ch ) == WEAR_NONE )
   {
       if ( !fSilent ) to_actor( "Remove something from your belt first.\n\r", ch );
       return TRUE;
   }

   if ( prop->wear_loc == WEAR_WIELD_1
     || prop->wear_loc == WEAR_WIELD_2
     || IS_SET(prop->extra_flags, ITEM_USED) ) unequip_char( ch, prop );
   prop->wear_loc = belt_empty( ch );

   /* Note: even silent calls show this message */
   act( "$n sheathes $p at $s belt.", ch, prop, NULL, TO_SCENE );
   act( "You sheathe $p at your belt.", ch, prop, NULL, TO_ACTOR );
   return TRUE;

}

/*
 * Remove a prop.
 */
bool remove_prop( PLAYER *ch, int iWear, bool fReplace, bool fMsg )
{
    PROP *prop;
    
    if ( ( prop = get_eq_char( ch, iWear ) ) == NULL )
	return TRUE;

    if ( !fReplace )
	return FALSE;

    if ( NOT_WORN( prop ) )
    {
    if ( fMsg ) act( "$p is not being worn.", ch, prop, NULL, TO_ACTOR );
    return FALSE;
    }

    if ( hand_empty( ch ) == WEAR_NONE && !fReplace )
    {
        to_actor( "Your hands are full.\n\r", ch );
        return FALSE;
    }

    if ( IS_SET(prop->extra_flags, ITEM_NOREMOVE) && !IS_IMMORTAL(ch) )
    {
	act( "It is impossible to remove $p.", ch, prop, NULL, TO_ACTOR );
	return FALSE;
    }

    if ( prop->wear_loc == WEAR_WAIST && iWear != WEAR_WAIST )
    {
        PROP *prop2;
        bool fFound = FALSE;
        int curwear;

        for ( curwear = WEAR_BELT_1; curwear <= WEAR_BELT_5; curwear++ )
        {
           prop2 = get_eq_char( ch, curwear );
           if (prop2 != NULL)
           {
                 fFound = TRUE;
                 prop_from_actor( prop2 );
                 prop_to_scene( prop2, ch->in_scene );
           }
        }

        if (fFound)
        {
        act( "As $n removes $p, everything on it clatters to the floor!", ch,
                               prop, NULL, TO_SCENE );
        act( "The items hung from your $t clatter to the floor.", ch,
                       smash_article( STR(prop,short_descr) ), NULL, TO_ACTOR );
        }
    }

    if ( unequip_char( ch, prop ) )
    {
        act( "$n stops using $p.", ch, prop, NULL, TO_SCENE );
        act( "You stop using $p.", ch, prop, NULL, TO_ACTOR );
    }
    else
    {
        to_actor( "Your hands are full.\n\r", ch );
        return FALSE;
    }

    return TRUE;
}


int get_wear_location( int loc )
{
    if ( IS_SET( loc, ITEM_WEAR_SHIELD ) )
    return WEAR_SHIELD;

    if ( IS_SET( loc, ITEM_WEAR_FINGER ) )
    return WEAR_FINGER_L;

    if ( IS_SET( loc, ITEM_WEAR_NECK ) )
    return WEAR_NECK_1;

    if ( IS_SET( loc, ITEM_WEAR_BODY ) )
    return WEAR_BODY;

    if ( IS_SET( loc, ITEM_WEAR_HEAD ) )
    return WEAR_HEAD;

    if ( IS_SET( loc, ITEM_WEAR_LEGS ) )
    return WEAR_LEGS;

    if ( IS_SET( loc, ITEM_WEAR_FEET ) )
    return WEAR_FEET;

    if ( IS_SET( loc, ITEM_WEAR_HANDS ) )
    return WEAR_HANDS;

    if ( IS_SET( loc, ITEM_WEAR_ARMS ) )
    return WEAR_ARMS;

    if ( IS_SET( loc, ITEM_WEAR_ABOUT ) )
    return WEAR_ABOUT;

    if ( IS_SET( loc, ITEM_WEAR_WAIST ) )
    return WEAR_WAIST;

    if ( IS_SET( loc, ITEM_WEAR_WRIST ) )
    return WEAR_WRIST_L;

    if ( IS_SET( loc, ITEM_WEAR_SHOULDER  ) )
    return WEAR_SHOULDER_L;

    if ( IS_SET( loc, ITEM_WEAR_ANKLE ) )
    return WEAR_ANKLE_L;

    if ( IS_SET( loc, ITEM_WEAR_EAR ) )
    return WEAR_EAR_L;

    if ( IS_SET( loc, ITEM_WEAR_HELM ) )
    return WEAR_HELM;

    if ( IS_SET( loc, ITEM_WEAR_SHIRT ) )
    return WEAR_SHIRT;

    if ( IS_SET( loc, ITEM_WEAR_STOCKING ) )
    return WEAR_STOCKING;

    if ( IS_SET( loc, ITEM_WEAR_FACE ) )
    return WEAR_FACE;

    if ( IS_SET( loc, ITEM_WEAR_BACK ) )
    return WEAR_BACK;

    if ( IS_SET( loc, ITEM_WEAR_LOIN ) )
    return WEAR_LOIN;

    if ( IS_SET( loc, ITEM_WEAR_FOREHEAD ) )
    return WEAR_FOREHEAD;

    if ( IS_SET( loc, ITEM_WEAR_NOSE ) )
    return WEAR_NOSE;

    if ( IS_SET( loc, ITEM_WEAR_PANTS ) )
    return WEAR_PANTS;

    if ( IS_SET( loc, ITEM_WEAR_PIN ) )
    return WEAR_PIN;
    
    if ( IS_SET( loc, ITEM_WEAR_ASBELT ) )
    return WEAR_ASBELT_1;

    return WEAR_NONE;
}

/*
 * Wear one prop.
 * Optional replacement of existing props.
 * Big repetitive code, ick.
 */
void wear_prop( PLAYER *ch, PROP *prop, bool fReplace, int loc )
{
    PROP *prop2;

    if ( NOTPET(loc) ) return;

    if ( CAN_WEAR( prop, ITEM_WEAR_SHIELD ) )
    {
        if ( !remove_prop( ch, WEAR_SHIELD, fReplace, TRUE )
          || !can_wield( ch, prop, FALSE ) )
            return;
        act( "$n straps $p to $s arm as a shield.", ch, prop, NULL, TO_SCENE );
        act( "You strap $p to your arm as a shield.", ch, prop, NULL, TO_ACTOR );
        equip_char( ch, prop, WEAR_SHIELD );
        return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_FINGER ) )
    if ( loc == 0 || loc == ITEM_WEAR_FINGER )
    {

	if ( get_eq_char( ch, WEAR_FINGER_L ) != NULL
	&&   get_eq_char( ch, WEAR_FINGER_R ) != NULL
	&&   !remove_prop( ch, WEAR_FINGER_L, fReplace, TRUE )
	&&   !remove_prop( ch, WEAR_FINGER_R, fReplace, TRUE ) )
	    return;

	if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL )
	{
        act( "$n slips $p onto $s left finger.",    ch, prop, NULL, TO_SCENE );
        act( "You slip $p onto your left finger.",  ch, prop, NULL, TO_ACTOR );
	    equip_char( ch, prop, WEAR_FINGER_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL )
	{
        act( "$n slips $p onto $s right finger.",   ch, prop, NULL, TO_SCENE );
        act( "You slip $p onto your right finger.", ch, prop, NULL, TO_ACTOR );
	    equip_char( ch, prop, WEAR_FINGER_R );
	    return;
	}

	bug( "Wear_prop: no free finger.", 0 );
	to_actor( "You already wear two rings.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_ANKLE ) )
    if ( loc == 0 || loc == ITEM_WEAR_ANKLE )
    {

    if ( get_eq_char( ch, WEAR_ANKLE_L ) != NULL
    &&   get_eq_char( ch, WEAR_ANKLE_R ) != NULL
    &&   !remove_prop( ch, WEAR_ANKLE_L, fReplace, TRUE )
    &&   !remove_prop( ch, WEAR_ANKLE_R, fReplace, TRUE ) )
	    return;

    if ( get_eq_char( ch, WEAR_ANKLE_L ) == NULL )
	{
        act( "$n fastens $p around $s left ankle.",    ch, prop, NULL, TO_SCENE );
        act( "You fasten $p around your left ankle.",  ch, prop, NULL, TO_ACTOR );
        equip_char( ch, prop, WEAR_ANKLE_L );
	    return;
	}

    if ( get_eq_char( ch, WEAR_ANKLE_R ) == NULL )
	{
        act( "$n fastens $p around $s right ankle.",   ch, prop, NULL, TO_SCENE );
        act( "You fasten $p around your right ankle.", ch, prop, NULL, TO_ACTOR );
        equip_char( ch, prop, WEAR_ANKLE_R );
	    return;
	}

    bug( "Wear_prop: no free ankle.", 0 );
    to_actor( "You already wear two items on your ankles.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_EAR ) )
    if ( loc == 0 || loc == ITEM_WEAR_EAR )
    {

    if ( get_eq_char( ch, WEAR_EAR_L ) != NULL
    &&   get_eq_char( ch, WEAR_EAR_R ) != NULL
    &&   !remove_prop( ch, WEAR_EAR_L, fReplace, TRUE )
    &&   !remove_prop( ch, WEAR_EAR_R, fReplace, TRUE ) )
	    return;

    if ( get_eq_char( ch, WEAR_EAR_L ) == NULL )
	{
        act( "$n wears $p on $s left ear.",    ch, prop, NULL, TO_SCENE );
        act( "You wear $p on your left ear.",  ch, prop, NULL, TO_ACTOR );
        equip_char( ch, prop, WEAR_EAR_L );
	    return;
	}

    if ( get_eq_char( ch, WEAR_EAR_R ) == NULL )
	{
        act( "$n wears $p on $s right ear.",   ch, prop, NULL, TO_SCENE );
        act( "You wear $p on $s right ear.", ch, prop, NULL, TO_ACTOR );
        equip_char( ch, prop, WEAR_EAR_R );
	    return;
	}

    bug( "Wear_prop: no free ear.", 0 );
    to_actor( "You already wear two earrings.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_NECK ) )
    if ( loc == 0 || loc == ITEM_WEAR_NECK )
    {

	if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
	&&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
	&&   !remove_prop( ch, WEAR_NECK_1, fReplace, TRUE )
	&&   !remove_prop( ch, WEAR_NECK_2, fReplace, TRUE ) )
	    return;

	if ( get_eq_char( ch, WEAR_NECK_1 ) == NULL )
	{
        act( "$n clasps $p around $s neck.",   ch, prop, NULL, TO_SCENE );
        act( "You clasp $p around your neck.", ch, prop, NULL, TO_ACTOR );
	    equip_char( ch, prop, WEAR_NECK_1 );
	    return;
	}

	if ( get_eq_char( ch, WEAR_NECK_2 ) == NULL )
	{
        act( "$n clasps $p around $s neck.",   ch, prop, NULL, TO_SCENE );
        act( "You clasp $p around your neck.", ch, prop, NULL, TO_ACTOR );
	    equip_char( ch, prop, WEAR_NECK_2 );
	    return;
	}

    bug( "Wear_prop: no free neck.", 0 );
	to_actor( "You already wear two neck items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_NOSE ) )
    if ( loc == 0 || loc == ITEM_WEAR_NOSE )
    {

    if ( !remove_prop( ch, WEAR_NOSE, fReplace, TRUE ) )
	    return;
    act( "$n pierces $s nose with $p.",   ch, prop, NULL, TO_SCENE );
    act( "You pierce your nose with $p.", ch, prop, NULL, TO_ACTOR );
    equip_char( ch, prop, WEAR_NOSE );
	return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_BODY ) )
    if ( loc == 0 || loc == ITEM_WEAR_BODY )
    {

	if ( !remove_prop( ch, WEAR_BODY, fReplace, TRUE ) )
	    return;
	act( "$n wears $p on $s body.",   ch, prop, NULL, TO_SCENE );
	act( "You wear $p on your body.", ch, prop, NULL, TO_ACTOR );
	equip_char( ch, prop, WEAR_BODY );
	return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_SHIRT ) )
    if ( loc == 0 || loc == ITEM_WEAR_SHIRT )
    {

    if ( !remove_prop( ch, WEAR_SHIRT, fReplace, TRUE ) )
	    return;
    act( "$n pulls $p over $s body.",   ch, prop, NULL, TO_SCENE );
    act( "You pull $p over your body.", ch, prop, NULL, TO_ACTOR );
    equip_char( ch, prop, WEAR_SHIRT );
	return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_HEAD ) )
    if ( loc == 0 || loc == ITEM_WEAR_HEAD )
    {

	if ( !remove_prop( ch, WEAR_HEAD, fReplace, TRUE ) )
	    return;
    act( "$n dons $p.",   ch, prop, NULL, TO_SCENE );
    act( "You don $p.", ch, prop, NULL, TO_ACTOR );
	equip_char( ch, prop, WEAR_HEAD );
	return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_HELM )
     && ( prop2 = get_eq_char( ch, WEAR_HEAD ) ) != NULL )
    if ( loc == 0 || loc == ITEM_WEAR_HELM )
    {

    if ( !remove_prop( ch, WEAR_HELM, fReplace, TRUE ) )
	    return;
    act( "$n places $p onto $P.",   ch, prop, prop2, TO_SCENE );
    act( "You place $p onto $P.", ch, prop, prop2, TO_ACTOR );
    equip_char( ch, prop, WEAR_HELM );
	return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_FACE ) )
    if ( loc == 0 || loc == ITEM_WEAR_FACE )
    {

    if ( !remove_prop( ch, WEAR_FACE, fReplace, TRUE ) )
	    return;
    act( "$n covers $s face with $p.",   ch, prop, NULL, TO_SCENE );
    act( "You cover $s face with $p.", ch, prop, NULL, TO_ACTOR );
    equip_char( ch, prop, WEAR_FACE );
	return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_LEGS ) )
    if ( loc == 0 || loc == ITEM_WEAR_LEGS )
    {

	if ( !remove_prop( ch, WEAR_LEGS, fReplace, TRUE ) )
	    return;
    act( "$n straps $p onto $s legs.",   ch, prop, NULL, TO_SCENE );
    act( "You strap $p onto your legs.", ch, prop, NULL, TO_ACTOR );
	equip_char( ch, prop, WEAR_LEGS );
	return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_PANTS ) )
    if ( loc == 0 || loc == ITEM_WEAR_PANTS )
    {

        if ( !remove_prop( ch, WEAR_PANTS, fReplace, TRUE ) )
            return;
        act( "$n wear $p on $s legs.",   ch, prop, NULL, TO_SCENE );
        act( "You wear $p on your legs.", ch, prop, NULL, TO_ACTOR );
        equip_char( ch, prop, WEAR_PANTS );
        return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_STOCKING ) )
    if ( loc == 0 || loc == ITEM_WEAR_STOCKING )
    {

        if ( !remove_prop( ch, WEAR_STOCKING, fReplace, TRUE ) )
            return;
        act( "$n pull $p onto $s legs.",   ch, prop, NULL, TO_SCENE );
        act( "You pull $p onto your legs.", ch, prop, NULL, TO_ACTOR );
        equip_char( ch, prop, WEAR_STOCKING );
        return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_FEET ) )
    if ( loc == 0 || loc == ITEM_WEAR_FEET )
    {
         if ( !remove_prop( ch, WEAR_FEET, fReplace, TRUE ) )
               return;
         act( "$n slips $s feet into $p.",   ch, prop, NULL, TO_SCENE );
         act( "You slip your feet into $p.", ch, prop, NULL, TO_ACTOR );
         equip_char( ch, prop, WEAR_FEET );
         return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_HANDS ) )
    if ( loc == 0 || loc == ITEM_WEAR_HANDS )
    {

        if ( !remove_prop( ch, WEAR_HANDS, fReplace, TRUE ) )
            return;
        act( "$n wears $p on $s hands.",   ch, prop, NULL, TO_SCENE );
        act( "You wear $p on your hands.", ch, prop, NULL, TO_ACTOR );
        equip_char( ch, prop, WEAR_HANDS );
        return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_ARMS ) )
    if ( loc == 0 || loc == ITEM_WEAR_ARMS )
    {

        if ( !remove_prop( ch, WEAR_ARMS, fReplace, TRUE ) )
             return;
        act( "$n wears $p on $s arms.",   ch, prop, NULL, TO_SCENE );
        act( "You wear $p on your arms.", ch, prop, NULL, TO_ACTOR );
        equip_char( ch, prop, WEAR_ARMS );
        return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_ABOUT ) )
    if ( loc == 0 || loc == ITEM_WEAR_ABOUT )
    {
        if ( !remove_prop( ch, WEAR_ABOUT, fReplace, TRUE ) )
            return;
        act( "$n wraps $p about $s body.",   ch, prop, NULL, TO_SCENE );
        act( "You wrap $p about your body.", ch, prop, NULL, TO_ACTOR );
        equip_char( ch, prop, WEAR_ABOUT );
        return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_WAIST ) )
    if ( loc == 0 || loc == ITEM_WEAR_WAIST )
    {

        if ( !remove_prop( ch, WEAR_WAIST, fReplace, TRUE ) )
            return;
        act( "$n fastens $p around $s waist.",   ch, prop, NULL, TO_SCENE );
        act( "You fasten $p around your waist.", ch, prop, NULL, TO_ACTOR );
        equip_char( ch, prop, WEAR_WAIST );
        return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_LOIN ) )
    if ( loc == 0 || loc == ITEM_WEAR_LOIN )
    {

        if ( !remove_prop( ch, WEAR_LOIN, fReplace, TRUE ) )
            return;
        act( "$n wraps $p about $s waist.",   ch, prop, NULL, TO_SCENE );
        act( "You wrap $p about your waist.", ch, prop, NULL, TO_ACTOR );
        equip_char( ch, prop, WEAR_LOIN );
        return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_WRIST ) )
    if ( loc == 0 || loc == ITEM_WEAR_WRIST )
    {

	if ( get_eq_char( ch, WEAR_WRIST_L ) != NULL
	&&   get_eq_char( ch, WEAR_WRIST_R ) != NULL
	&&   !remove_prop( ch, WEAR_WRIST_L, fReplace, TRUE )
	&&   !remove_prop( ch, WEAR_WRIST_R, fReplace, TRUE ) )
	    return;

	if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL )
	{
	    act( "$n clips $p around $s left wrist.",
		ch, prop, NULL, TO_SCENE );
	    act( "You clip $p around your left wrist.",
		ch, prop, NULL, TO_ACTOR );
	    equip_char( ch, prop, WEAR_WRIST_L );
	    return;
    }

	if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL )
	{
	    act( "$n clips $p around $s right wrist.",
		ch, prop, NULL, TO_SCENE );
	    act( "You clip $p around your right wrist.",
		ch, prop, NULL, TO_ACTOR );
	    equip_char( ch, prop, WEAR_WRIST_R );
	    return;
	}

	bug( "Wear_prop: no free wrist.", 0 );
	to_actor( "You already wear two wrist items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_SHOULDER ) )
    if ( loc == 0 || loc == ITEM_WEAR_SHOULDER )
    {

    if ( get_eq_char( ch, WEAR_SHOULDER_L ) != NULL
      && get_eq_char( ch, WEAR_SHOULDER_R ) != NULL
      && !remove_prop( ch, WEAR_SHOULDER_L, fReplace, TRUE )
      && !remove_prop( ch, WEAR_SHOULDER_R, fReplace, TRUE ) )
            return;

    if ( get_eq_char( ch, WEAR_SHOULDER_L ) == NULL )
        {
             act( "$n slings $p over $s left shoulder.",
                     ch, prop, NULL, TO_SCENE );
             act( "You sling $p over your left shoulder.",
                     ch, prop, NULL, TO_ACTOR );
             equip_char( ch, prop, WEAR_SHOULDER_L );
                 return;
        }

    if ( get_eq_char( ch, WEAR_SHOULDER_R ) == NULL )
        {
             act( "$n slings $p over $s right shoulder.",
                     ch, prop, NULL, TO_SCENE );
             act( "You sling $p over your right shoulder.",
                     ch, prop, NULL, TO_ACTOR );
             equip_char( ch, prop, WEAR_SHOULDER_R );
                 return;
        }

    bug( "Wear_prop: no free shoulder.", 0 );
    to_actor( "You already wear an items on each shoulder.\n\r", ch );
        return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_ASBELT ) )
    if ( loc == 0 || loc == ITEM_WEAR_ASBELT )
    {

    if ( get_eq_char( ch, WEAR_ASBELT_1 ) != NULL
      && get_eq_char( ch, WEAR_ASBELT_2 ) != NULL
      && get_eq_char( ch, WEAR_ASBELT_3 ) != NULL
      && !remove_prop( ch, WEAR_ASBELT_1, fReplace, TRUE )
      && !remove_prop( ch, WEAR_ASBELT_2, fReplace, TRUE )
      && !remove_prop( ch, WEAR_ASBELT_3, fReplace, TRUE ) )
            return;

    if ( get_eq_char( ch, WEAR_ASBELT_1 ) == NULL )
        {
             act( "$n fastens $p around $s waist.",  ch, prop, NULL, TO_SCENE );
             act( "You fasten $p around your waist.", ch, prop, NULL, TO_ACTOR );
             equip_char( ch, prop, WEAR_ASBELT_1 );
                 return;
        }

    if ( get_eq_char( ch, WEAR_ASBELT_2 ) == NULL )
        {
             act( "$n fastens $p around $s waist.",  ch, prop, NULL, TO_SCENE );
             act( "You fasten $p around your waist.", ch, prop, NULL, TO_ACTOR );
             equip_char( ch, prop, WEAR_ASBELT_2 );
                 return;
        }

    if ( get_eq_char( ch, WEAR_ASBELT_3 ) == NULL )
        {
            act( "$n fastens $p around $s waist.",   ch, prop, NULL, TO_SCENE );
            act( "You fasten $p around your waist.",  ch, prop, NULL, TO_ACTOR );
            equip_char( ch, prop, WEAR_ASBELT_3 );
            return;
        }

    bug( "Wear_prop: no free as_belt.", 0 );
    to_actor( "You already wear enough items there.\n\r", ch );
        return;
    }


    if ( CAN_WEAR( prop, ITEM_WEAR_BACK ) )
    if ( loc == 0 || loc == ITEM_WEAR_BACK )
    {
        if ( !remove_prop( ch, WEAR_BACK, fReplace, TRUE ) )
            return;
        act( "$n hefts $p onto $s back.",   ch, prop, NULL, TO_SCENE );
        act( "You heft $p onto your back.", ch, prop, NULL, TO_ACTOR );
        equip_char( ch, prop, WEAR_BACK );
        return;
    }

    if ( CAN_WEAR( prop, ITEM_WEAR_SHOULDERS ) )
    {
        if ( !remove_prop( ch, WEAR_SHOULDERS, fReplace, TRUE )
          || !can_wield( ch, prop, FALSE ) )
            return;
        act( "$n places $p on $s shoulders.", ch, prop, NULL, TO_SCENE );
        act( "You place $p on your shoulders.", ch, prop, NULL, TO_ACTOR );
        equip_char( ch, prop, WEAR_SHOULDERS );
        return;
    }

    if ( CAN_WEAR( prop, ITEM_HOLD )  && hand_empty( ch ) != WEAR_NONE )
    {
        if ( !remove_prop( ch, prop->wear_loc, fReplace, TRUE ) )
            return;
        act( "$n holds $p in $s hand.", ch, prop, NULL, TO_SCENE );
        act( "You hold $p in your hand.", ch, prop, NULL, TO_ACTOR );
        equip_char( ch, prop, hand_empty( ch ) );
        return;
    }

    if ( fReplace )
    to_actor( "It is impossible to wear that there.\n\r", ch );

    return;
}



/*
 * Syntax:  wear [prop]
 *          wear [prop] [location]
 *          wear all
 */
void cmd_wear( PLAYER *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    PROP *prop;
    int oldwear;

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
    to_actor( "Wear what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	PROP *prop_next;

	for ( prop = ch->carrying; prop != NULL; prop = prop_next )
	{
	    prop_next = prop->next_content;
        if ( (prop->wear_loc == WEAR_NONE || prop->wear_loc == WEAR_HOLD_1 ||
              prop->wear_loc == WEAR_HOLD_2)
           && can_see_prop( ch, prop )
           && !TOO_SMALL(ch, prop)
           && !TOO_BIG(ch, prop) )
        wear_prop( ch, prop, FALSE, 0 );
	}
        inv_to_hand_check( ch );
	return;
    }
    else
    {
    if ( ( prop = get_prop_carry( ch, arg1 ) ) == NULL )
	{
            to_actor( "You do not have that item in your hand.\n\r", ch );
	    return;
	}

    if ( (prop->size || ch->size) && (TOO_BIG(ch,prop) || TOO_SMALL(ch,prop)) )
    {
        to_actor( "It doesn't fit you.\n\r", ch );
        return;
    }

    oldwear = prop->wear_loc;
    wear_prop( ch, prop, TRUE, wear_name_bit( arg2 ) );
    if ( prop->size && ch->size && oldwear != prop->wear_loc )
    {
        if ( prop->size < ch->size )
        to_actor( "It looks a bit snug.\n\r", ch );
        else
        if ( prop->size > ch->size )
        to_actor( "It looks a bit loose.\n\r", ch );
    }
    }

    inv_to_hand_check( ch );
    return;
}



/*
 * Syntax:  sheath
 *          sheath [prop]
 */
void cmd_sheath( PLAYER *ch, char *argument )
{
    char arg1[MAX_STRING_LENGTH];
    PROP *prop1;
    PROP *prop2;

    argument = one_argument( argument, arg1 );

    if ( ( prop1 = get_prop_carry( ch, arg1 ) ) == NULL )
    {
    prop1 = get_eq_char( ch, WEAR_HOLD_1 );
    prop1 = prop1 ? prop1 : get_eq_char( ch, WEAR_WIELD_1 );
    prop2 = get_eq_char( ch, WEAR_HOLD_2 );
    prop2 = prop2 ? prop2 : get_eq_char( ch, WEAR_WIELD_2 );
        sheath_prop( ch, prop1, FALSE );
        sheath_prop( ch, prop2, FALSE );
        return;
    }

    if (!sheath_prop( ch, prop1, FALSE ))  
    to_actor( "It is impossible to do that!\n\r", ch );
    return;
}



/*
 * Syntax:  draw
 *          draw [prop]
 */
void cmd_draw( PLAYER *ch, char *argument )
{
    char arg1[MAX_STRING_LENGTH];
    PROP *prop;
 
    argument = one_argument( argument, arg1 );
 
    if ( ( prop = get_prop_wear( ch, arg1 ) ) == NULL )
      {    
       if ( ( prop = get_eq_char( ch, WEAR_BELT_1 ) ) != NULL ) 
          draw_prop( ch, prop );
       if ( ( prop = get_eq_char( ch, WEAR_BELT_2 ) ) != NULL ) 
          draw_prop( ch, prop );                 
       if ( ( prop = get_eq_char( ch, WEAR_BELT_3 ) ) != NULL ) 
          draw_prop( ch, prop );
       if ( ( prop = get_eq_char( ch, WEAR_BELT_4 ) ) != NULL ) 
          draw_prop( ch, prop );
       if ( ( prop = get_eq_char( ch, WEAR_BELT_5 ) ) != NULL ) 
          draw_prop( ch, prop );
          
       return;
     }
 
    if (!draw_prop( ch, prop ))  to_actor( "You are unable to draw an item from your belt.\n\r", ch );
    return;
}
 


/*
 * Syntax:  remove [prop]
 */
void cmd_remove( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PROP *prop;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	to_actor( "Remove what?\n\r", ch );
	return;
    }

    if ( ( prop = get_prop_wear( ch, arg ) ) == NULL )
    {
        to_actor( "You are not wearing that item.\n\r", ch );
	return;
    }

    remove_prop( ch, prop->wear_loc, TRUE, TRUE );
    inv_to_hand_check( ch );
    return;
}



/*
 * Syntax:  wield [prop]
 */
void cmd_wield( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PROP *prop;

    one_argument( argument, arg );

    if ( ( prop = get_prop_carry( ch, arg ) ) == NULL )
    {
        to_actor( "You are not holding that.\n\r", ch );
        return;
    }

    if ( prop->wear_loc == WEAR_WIELD_1
      || prop->wear_loc == WEAR_WIELD_2 )
    {
	to_actor( "It is already being wielded.\n\r", ch );
    	return;
    }

    if ( can_wield( ch, prop, FALSE ) )
    {
	equip_char( ch, prop, wield_free( ch, prop ) );

    	act( "$n wields $p.", ch, prop, NULL, TO_SCENE );
    	act( "You wield $p.", ch, prop, NULL, TO_ACTOR );
    }
    else
    {
	act( "It is impossible to wield $p now.", ch, prop, NULL, TO_ACTOR );
    }

    inv_to_hand_check( ch );
    return;
}


/*
 * Syntax:  hold [prop]
 */
void cmd_hold( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PROP *prop;

    one_argument( argument, arg );

    if ( ( prop = get_prop_carry( ch, arg ) ) == NULL )
    {
        to_actor( "You are not holding that.\n\r", ch );
        return;
    }

    if ( prop->wear_loc == WEAR_HOLD_1
      || prop->wear_loc == WEAR_HOLD_2 )
    return;

    if ( prop->wear_loc == WEAR_WIELD_1
      || prop->wear_loc == WEAR_WIELD_2
      || IS_SET(prop->extra_flags, ITEM_USED) )
    {
        unequip_char( ch, prop );
        act( "$n stops wielding $p.", ch, prop, NULL, TO_SCENE );
        act( "You stop wielding $p.", ch, prop, NULL, TO_ACTOR );
    }

    prop->wear_loc   = hand_empty( ch );
    act( "$n holds $p.", ch, prop, NULL, TO_SCENE );
    act( "You hold $p.", ch, prop, NULL, TO_ACTOR );
    return;
}


/*
 * Tools for fools.
 * Syntax:  use [tool] <target>
 */
void cmd_use( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
/*    char buf[MAX_STRING_LENGTH]; */
    PLAYER *victim;
    PROP *prop;
/*    PROP *target; */

    if ( ch->position == POS_FIGHTING ) { to_actor("Not while fighting!\n\r", ch ); return; }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	to_actor( "Use what (on what)?\n\r", ch );
	return;
    }

    if ( ( prop = get_prop_inv( ch, arg ) ) == NULL )
    {
        to_actor( "You are not holding that item.\n\r", ch );
	return;
    }

    if ( prop->item_type != ITEM_TOOL ) { to_actor( "How?\n\r", ch ); 
        return; }

    argument = one_argument( argument, arg );

    /*
     * Bandages.
     */
    if ( IS_SET(prop->value[0], TOOL_BANDAGES) ) {
           
    if ( ( victim = get_actor_world( ch, arg ) ) == NULL ) victim = ch;

    if ( victim->position == POS_FIGHTING ) { to_actor("Not while fighting!\n\r", ch ); return; }

    /*
     * Item use decay.
     */
    if ( prop->value[1]-- <= 0 ) { to_actor( "It's all used up.\n\r", ch ); return; }

    ch->hit = ch->hit + prop->value[3];
    if ( ch->hit > MAXHIT(ch) ) ch->hit = MAXHIT(ch);

    if ( ch == victim ) 
        act( "$n uses $p.", ch, prop, NULL, TO_SCENE);

    if ( ch != victim ) {
        act( "$n heals $N with $p.", ch, prop, NULL, TO_ALL );
    }
    to_actor( STR(prop,action_descr), ch );
    to_actor( "You feel better.\n\r", victim );
    if ( prop->value[1] == 0 ) extractor_prop(prop);

    }


}

/*
 * Similar to wield [prop] command
 */
void wield_prop( PROP *prop, PLAYER *ch )
{
    PROP *prop1;
    PROP *prop2;

    prop1 = get_eq_char( ch, WEAR_HOLD_1 );
    prop2 = get_eq_char( ch, WEAR_HOLD_2 );

    if ( prop1 && can_wield( ch, prop1, TRUE ) )
    equip_char( ch, prop1, wield_free( ch, prop1 ) );

    if ( prop2 && can_wield( ch, prop2, TRUE ) )
    equip_char( ch, prop2, wield_free( ch, prop2 ) );

    inv_to_hand_check( ch );
    return;
}


/*
 * Kludgy work-around.
 * find_pack() determines target packs for get_prop()
 */
bool fFound;

PROP *find_pack( PLAYER *ch, PROP *prop ) {

    PROP *pack;

        /*
         * Locate open packs that still have space available in them.
         */
    fFound=FALSE;
    for ( pack = ch->carrying; pack != NULL; pack = pack->next_content ) 
    {
       if ( pack->item_type == ITEM_CONTAINER 
          && ( get_prop_weight( prop ) + 
             ( get_prop_weight( pack ) - 
               pack->weight) < pack->value[0] ) ) 
         {
              if ( !IS_SET(pack->value[3],CONT_CLOSED) ) {
                    fFound = TRUE;
                    break;
                }
         }
    }
    return pack;
}


/*
 * Used by cmd_get.
 */
int get_prop( PLAYER *ch, PROP *prop, 
              PROP *container )
{
    char buf[MAX_STRING_LENGTH];
    PROP *pack;
    PROP *pProp;
    bool fMoney;

    if ( !prop ) return FALSE;

    fMoney = prop->item_type == ITEM_MONEY;

    if ( !CAN_WEAR(prop, ITEM_TAKE) )
    {
        act( "You cannot carry $p.", ch, prop, NULL, TO_ACTOR );
	return TRUE;
    }

    if ( ch->carry_weight + get_prop_weight( prop ) > can_carry_w( ch )
     && container == NULL )
    {
        act( "$d: you aren't able to carry that much weight.",
             ch, NULL, STR(prop, name), TO_ACTOR );
        return FALSE;
    }

    pack = find_pack( ch, prop );

    /*
     * Player's hands are full.  Attempt to sheath weapons
     * to free up resources.
     */
    if ( hand_empty( ch ) == WEAR_NONE )
    {
    PROP *prop1, *prop2;

    prop1 = get_eq_char( ch, WEAR_HOLD_1 );
    prop1 = prop1 ? prop1 : get_eq_char( ch, WEAR_WIELD_1 );
    prop2 = get_eq_char( ch, WEAR_HOLD_2 );
    prop2 = prop2 ? prop2 : get_eq_char( ch, WEAR_WIELD_2 );
    sheath_prop( ch, prop1, TRUE );
    sheath_prop( ch, prop2, TRUE );
    }
  
    /*
     * Did the game sheath the weapons?  (Does the player
     * have a belt to sheath these weapons?  If not...)
     */
    if ( hand_empty( ch ) == WEAR_NONE )  /* attempt to free a hand */
    {
        /*
         * If there is a target pack, attempt to put one or more
         * items from the player's hands into the pack.
         *
         * If the player is holding the target pack, do nothing.
         * Future Improvement? if a player is holding a pack,
         * default to that pack as the target pack.
         */
        if ( pack != NULL )
        {
           pProp = get_eq_char( ch, WEAR_HOLD_2 );
           if ( !pProp || pProp == pack ) pProp = get_eq_char( ch, WEAR_HOLD_1 );
           if ( !pProp || pProp == pack ) pProp = get_eq_char( ch, WEAR_WIELD_2 );
           if ( !pProp || pProp == pack ) pProp = get_eq_char( ch, WEAR_WIELD_1 );

           if ( pProp && pProp != pack ) {

/*           unequip_char( ch, pProp );*/
           prop_from_actor( pProp );
           prop_to_prop( pProp, pack );
           snprintf( buf, MAX_STRING_LENGTH, "You stuff %s into %s.\n\r", 
                 STR(pProp,short_descr),
                 STR(pack,short_descr) );
           act( buf, ch, NULL, NULL, TO_ACTOR );
           } 
           else {
              to_actor( "Your hands are full.\n\r", ch );
              return FALSE;
           }
        }
        else {
          /*
           * The player has no applicable target pack.
           * Let them know they why they can't pick the item up.
           */
           if ( fFound ) 
           {
              to_actor( "Your packs are closed.\n\r", ch );
              return FALSE;
           }
           else {
              to_actor( "Your hands are full.\n\r", ch );
              return FALSE;
           }
        }
    }

          /*
           * There's no target pack, the player has no free
           * hands, and there is no pack that's closed.
           */
        if ( hand_empty( ch ) == WEAR_NONE && !pack && !fFound ) {
             to_actor( "Your hands are full.\n\r", ch );
             return FALSE;
        }
          /*
           * The player has no free hands, there's no target
           * pack, but it found a pack that's closed. 
           */
        if ( hand_empty( ch ) == WEAR_NONE && !pack && fFound ) {
             act( "Your $p is closed.", ch, prop, NULL, TO_ACTOR );
             return FALSE;
        } 

     /*
      * Try to pick something up from a container.
      */
    if ( container )  /* from a corpse or container */
    {
          /*
           * Ok, there's a target pack but no free hands.
           * The player has to stow an item directly.
           */
        if ( pack && hand_empty( ch ) == WEAR_NONE ) {

            snprintf( buf, MAX_STRING_LENGTH, "You pick up %s from %s and stow it into %s.", 
                      STR(prop,short_descr),
                      STR(container,short_descr),
                      STR(pack,short_descr) );
            act( buf, ch, NULL, NULL, TO_ACTOR );

            snprintf( buf, MAX_STRING_LENGTH, "%s gets %s from %s and puts it into %s.", 
                      capitalize(NAME(ch)),
                      STR(prop,short_descr),
                      STR(container,short_descr),
                      STR(pack,short_descr) );
            act( buf, ch, NULL, NULL, TO_SCENE );

            prop_from_prop( prop );
            prop_to_prop( prop, pack );
            return TRUE;
          }

         /*
          * The player has a free hand.
          */
        if ( hand_empty( ch ) != WEAR_NONE ) {
             act( "You get $p from $P.", ch, prop, container, TO_ACTOR );
             act( "$n gets $p from $P.", ch, prop, container, TO_SCENE );

             prop_from_prop( prop );
             if ( !fMoney ) {
                 prop_to_actor( prop, ch );
                 prop->wear_loc = hand_empty( ch );
                 wield_prop( prop, ch );
             }
             else  prop_to_actor_money( prop, ch );
          
             return TRUE;
        } 

         /*
          * The player has no free hands.
          */
        to_actor( "Your hands are full.\n\r", ch );
        return FALSE;

    }
    else  /* from the scene */
    {
         /*
          * There's a target pack, but no free hands.
          * Player attempts to stow the item directly.
          */
        if ( pack && hand_empty( ch ) == WEAR_NONE ) {


            snprintf( buf, MAX_STRING_LENGTH, "You pick up %s and stow it into %s.", 
                      STR(prop,short_descr),
                      STR(pack,short_descr) );
            act( buf, ch, NULL, NULL, TO_ACTOR );

            snprintf( buf, MAX_STRING_LENGTH, "%s gets %s and puts it into %s.", 
                      capitalize(NAME(ch)),
                      STR(prop,short_descr),
                      STR(pack,short_descr) );
            act( buf, ch, NULL, NULL, TO_SCENE );

            prop_from_scene( prop );
            prop_to_prop( prop, pack );
            return TRUE;
          }

         /*
          * The player has a free hand.
          */
        if ( hand_empty( ch ) != WEAR_NONE  ) {
             act( "You get $p.", ch, prop, container, TO_ACTOR );
             act( "$n gets $p.", ch, prop, container, TO_SCENE );

             prop_from_scene( prop );
             if ( !fMoney ) {
                 prop_to_actor( prop, ch );
                 prop->wear_loc = hand_empty( ch );
                 wield_prop( prop, ch );
             }
             else  prop_to_actor_money( prop, ch );
          
             return TRUE;
        } 

         /*
          * The player has no free hand.
          */
        to_actor( "Your hands are full.\n\r", ch );
        return FALSE;
     }

    return FALSE;
}



/*
 * Syntax: get [prop]
 *         get all
 *         get all [prop]
 *         get all.[prop]
 *         get all.[prop] [prop]
 */
void cmd_get( PLAYER *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    PROP *prop;
    PROP *prop_next;
    PROP *container;
    bool found;              /* NEEDS TO SEARCH 1 LEVEL DEEP + SACK FIX */

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( !str_cmp( arg2, "from" ) )
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	to_actor( "Get what?\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
        /* get all scene */

        if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
        {
            prop = get_prop_list( ch, arg1, ch->in_scene->contents );
            if ( prop == NULL )
            {
                act( "I see no $T here.", ch, NULL, arg1, TO_ACTOR );
                return;
            }

            if ( get_prop( ch, prop, NULL ) )
            script_update( prop, TYPE_PROP, TRIG_GETS, ch, NULL, NULL, NULL );
        }
        else
        {
            found = FALSE;
            for ( prop = ch->in_scene->contents; prop != NULL; prop = prop_next )
            {
                PROP *prop2, *prop2_next;

                prop_next = prop->next_content;

                if ( ( arg1[3] == '\0' || is_name( &arg1[4], STR(prop, name) ) )
                &&   can_see_prop( ch, prop ) )
                {
                    found = TRUE;
                    if ( !get_prop( ch, prop, NULL ) ) break;
                    script_update( prop, TYPE_PROP, TRIG_GETS, ch, NULL, NULL, NULL );
                }

                /* stuff from tables, containers and corpses */
                if ( prop->contains != NULL && prop->carried_by != ch && !IS_SET(prop->value[3],CONT_CLOSED) )
                for ( prop2 =  prop->contains;  prop2 != NULL;  prop2 = prop2_next ) {
                 prop2_next = prop2->next_content;
                 if ( ( arg1[3] == '\0' || is_name( &arg1[4], STR(prop2, name) ) )
                 &&   can_see_prop( ch, prop2 ) )
                  {
                    found = TRUE;
                    if ( !get_prop( ch, prop2, prop ) ) break;
                    script_update( prop2, TYPE_PROP, TRIG_GETS, ch, NULL, NULL, NULL );
                  }
		}

            }

            if ( !found )
            {
                if ( arg1[3] == '\0' )
                to_actor( "I see nothing here.\n\r", ch );
                else
                act( "I see no $T here.", ch, NULL, &arg1[4], TO_ACTOR );
            }
        }
    }
    else
    {
        if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
        {
            to_actor( "It is impossible to do that.\n\r", ch );
            return;
        }

        if ( ( container = get_prop_here( ch, arg2 ) ) == NULL ) 
        {
            /* check in packs of mount */
            PLAYER *mount =  get_actor_scene( ch, arg2 );
            if ( !mount ) {
            act( "I see no $T here.", ch, NULL, arg2, TO_ACTOR );
            return;
            } else {
 
            if ( !NPC(mount) ||  mount->master != ch ) { 
               act( "$N is not yours to command.", ch, NULL, mount, TO_ACTOR );
               return;
            }
 
            prop = get_prop_list( ch, arg1, mount->carrying );
            if ( prop == NULL )
            {
            act( "I see nothing like that carried by $N.", ch, NULL, mount, TO_ACTOR );
            return;
            }
 
         /*
          * The player has a free hand?
          */
        if ( hand_empty( ch ) == WEAR_NONE ) { 
         /*
          * The player has no free hands.
          */
        to_actor( "Your hands are full.\n\r", ch );
        return;
        }


            prop_from_actor( prop );
            prop_to_scene( prop, ch->in_scene );
            if ( !get_prop( ch, prop, NULL ) ) {
             act( "$n drops $p.", ch, prop, NULL, TO_SCENE );
             to_actor( "You couldn't retreive the item and ended up dropping it.\n\r", ch );
            }
            else {
             return;
            }

            } /* end mount checks */
        }

        switch ( container->item_type )
        {
            default: to_actor( "That's not a container.\n\r", ch );
                     return;

            case ITEM_FURNITURE:
             if ( !VAL_SET(container, 1, FURN_PUT) )
             {
                to_actor( "That's not a container.\n\r", ch );
                return;
             }
            case ITEM_CONTAINER:
            case ITEM_CORPSE_NPC:
            case ITEM_CORPSE_PC:
            break;
        }

        if ( IS_SET(container->value[1], CONT_CLOSED) )
        {
            act( "The $t is closed.",
                 ch, smash_article(STR(container, name)), NULL, TO_ACTOR );
            return;
        }

        if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
        {
            /* 'get prop container' */
            prop = get_prop_list( ch, arg1, container->contains );
            if ( prop == NULL )
            {
            act( "I see nothing like that in the $T.", ch, NULL, arg2, TO_ACTOR );
            return;
            }
        if ( get_prop( ch, prop, container /* container->carried_by != ch */ ) )
        script_update( prop, TYPE_PROP, TRIG_GETS, ch, NULL, NULL, NULL );
        }
        else
        {
            /* 'get all container' or 'get all.prop container' */
            found = FALSE;
            for ( prop = container->contains; prop != NULL; prop = prop_next )
            {
                prop_next = prop->next_content;
                if ( ( arg1[3] == '\0' || is_name( &arg1[4], STR(prop, name) ) )
                    &&   can_see_prop( ch, prop ) )
            {
                found = TRUE;
                cmd_sheath( ch, "" );
                if ( !get_prop( ch, prop, container 
                   /* container->carried_by != ch */ ) ) break;
                script_update( prop, TYPE_PROP, TRIG_GETS, ch, NULL, NULL, NULL );
            }
            }

            if ( !found )
            {
            if ( arg1[3] == '\0' )
                act( "I see nothing in the $T.", ch, NULL, arg2, TO_ACTOR );
            else
                act( "I see nothing like that in the $T.",
                     ch, NULL, arg2, TO_ACTOR );
            }
        }
    }

    return;
}




/*
 * Syntax:  put [prop] [prop]
 *          put all.[prop] [prop]
 */
void cmd_put( PLAYER *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    PROP *container;
    PROP *prop;
    PROP *prop_next;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( !str_cmp( arg2, "in" ) || !str_cmp( arg2, "on" ) )
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	to_actor( "Put what in what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
    {
	to_actor( "It is impossible to do that.\n\r", ch );
	return;
    }

    if ( ( container = get_prop_here( ch, arg2 ) ) == NULL )
    {
	act( "I see no $T here.", ch, NULL, arg2, TO_ACTOR );
	return;
    }

    if ( container->item_type != ITEM_CONTAINER
      && container->item_type != ITEM_FURNITURE )
    {
        to_actor( "That's not a container.\n\r", ch );
        return;
    }
    
    if ( container->item_type == ITEM_FURNITURE 
      && !IS_SET(container->value[1], FURN_PUT) )
    {
        to_actor( "It is impossible to put anything on that.\n\r", ch );
        return;
    }

    if ( IS_SET(container->value[1], CONT_CLOSED) )
    {
    act( "The $t is closed.",
         ch, smash_article(STR(container, name)), NULL, TO_ACTOR );
	return;
    }

    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
	/* 'put prop container' */
    if ( ( prop = get_prop_inv( ch, arg1 ) ) == NULL )
	{
	    to_actor( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( prop == container )
	{
	    to_actor( "It is impossible to fold it into itself.\n\r", ch );
	    return;
	}

	if ( !can_drop_prop( ch, prop ) )
	{
	    to_actor( "It is impossible to let go of it.\n\r", ch );
	    return;
	}

    if ( get_prop_weight( prop ) + (get_prop_weight(container) - container->weight) > container->value[0] )
	{
	    to_actor( "It won't fit.\n\r", ch );
	    return;
	}

	prop_from_actor( prop );
    prop_to_prop( prop, container );
    if ( container->item_type != ITEM_FURNITURE )
    {
    act( "$n puts $p in $P.", ch, prop, container, TO_SCENE );
    act( "You put $p in $P.", ch, prop, container, TO_ACTOR );
    }
    else
    {
    act( "$n places $p on $P.", ch, prop, container, TO_SCENE );
    act( "You place $p on $P.", ch, prop, container, TO_ACTOR );
    }
    merge_money_prop( container );
    }
    else
    {
    /* 'put all container' or 'put all.prop container' */
	for ( prop = ch->carrying; prop != NULL; prop = prop_next )
	{
	    prop_next = prop->next_content;

        if ( ( arg1[3] == '\0' || is_name( &arg1[4], STR(prop, name) ) )
	    &&   can_see_prop( ch, prop )
            &&   INV(prop->wear_loc)
	    &&   prop != container
	    &&   can_drop_prop( ch, prop )
	    &&   get_prop_weight( prop ) + get_prop_weight( container )
		 <= container->value[0] )
	    {
		prop_from_actor( prop );
        prop_to_prop( prop, container );
        if ( container->item_type != ITEM_FURNITURE )
        {
        act( "$n puts $p in $P.", ch, prop, container, TO_SCENE );
        act( "You put $p in $P.", ch, prop, container, TO_ACTOR );
        }
        else
        {
        act( "$n places $p on $P.", ch, prop, container, TO_SCENE );
        act( "You place $p on $P.", ch, prop, container, TO_ACTOR );
        }
        merge_money_prop( container );
	    }
	}
    }

    return;
}



/*
 * Syntax:  dump [prop]
 *          dump [prop] [prop]
 */
void cmd_dump( PLAYER *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    PROP *container1, *container2 = NULL;
    PROP *prop;
    PROP *prop_next;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
    to_actor( "Dump what (into what)?\n\r", ch );
	return;
    }

    if ( (container1 = get_prop_here( ch, arg1 )) == NULL )
    {
        to_actor( "It is impossible to find that here.\n\r", ch );
        return;
    }

    if ( arg2[0] != '\0' && (container2 = get_prop_here( ch, arg2 )) == NULL )
    {
        to_actor( "It is impossible to find that here to dump it into.\n\r", ch );
        return;
    }

    if ( container1->item_type != ITEM_CONTAINER )
    {
        to_actor( "That's not a container that you can dump out.\n\r", ch );
        return;
    }

    if ( VAL_SET(container1, 1, CONT_CLOSED) )
    {
        to_actor( "It is closed.\n\r", ch );
        return;
    }

    if ( container2 )
    {
        if ( container2->item_type != ITEM_CONTAINER )
        {
            to_actor( "That's not a container that you can dump out.\n\r", ch );
            return;
        }

        if ( VAL_SET(container2, 1, CONT_CLOSED) )
        {
            to_actor( "It is impossible to dump anything into it, it is closed.\n\r", ch );
            return;
        }

        for ( prop = container1->contains;  prop != NULL; prop = prop_next )
        {
        prop_next = prop->next_content;

        if ( get_prop_weight( prop ) + get_prop_weight( container2 ) > container2->value[0] )
        {
        to_actor( "It won't all fit.\n\r", ch );
        act( "$n dumps a portion of the contents of $p into $P.", ch, container1, container2, TO_SCENE );
	    return;
        } else {
           prop_from_prop( prop );
           prop_to_prop( prop, container2 );
        }
        }

        act( "$n dumps the contents of $p into $P.", ch, container1, container2, TO_SCENE );
        act( "You dump the contents of $p into $P.", ch, container1, container2, TO_ACTOR );
    }
    else
    {
        act( "$n dumps the contents of $p onto the ground.", ch, container1, NULL, TO_SCENE );
        act( "You dump the contents of $p onto the ground.", ch, container1, NULL, TO_ACTOR );

        for ( prop = container1->contains;  prop != NULL; prop = prop_next )
        {
            prop_next = prop->next_content;

            prop_from_prop( prop );
            prop_to_scene( prop, ch->in_scene );
        }
    }

    return;
}




/*
 * Syntax:  drop [prop]
 *          drop all
 *          drop all.[prop]
 *          drop [number] [money prop]
 */
void cmd_drop( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PROP *prop;
    PROP *prop_next;
    int amount = 0;
    bool found;

    argument = one_argument( argument, arg );
    if ( is_number( arg ) )
    {
        amount = atoi( arg );
        argument = one_argument( argument, arg );
    }

    if ( arg[0] == '\0' )
    {
	to_actor( "Drop what?\n\r", ch );
	return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
	/* 'drop prop' */
    if ( ( prop = get_prop_inv( ch, arg ) ) == NULL )
	{
        to_actor( "You do not have that item.\n\r", ch );
	    return;
	}
    else
    if ( ( prop->wear_loc != WEAR_WIELD_1
        && prop->wear_loc != WEAR_WIELD_2 )
        || !NOT_WORN(prop) )
    {
        if ( !remove_prop( ch, prop->wear_loc, FALSE, FALSE ) )
        unequip_char( ch, prop );
        /*to_actor( "You will have to remove it first.\n\r", ch );
        return;*/
    }

	if ( !can_drop_prop( ch, prop ) )
	{
	    to_actor( "It is impossible to let go of it.\n\r", ch );
	    return;
	}

    if ( prop->item_type == ITEM_MONEY )
    {
        PROP *pnewprop;

        if ( amount > prop->value[0] || amount == 0 )
        amount = prop->value[0];

        prop->value[0] -= amount;


        update_money( prop );
        prop_to_scene( (pnewprop = create_money( amount, prop->value[1] )), ch->in_scene );

        act( "$n drops $p.", ch, pnewprop, NULL, TO_SCENE );
        act( "You drop $p.", ch, pnewprop, NULL, TO_ACTOR );
        strip_empty_money( ch );
        merge_money( ch );
        merge_money_scene( ch->in_scene );
        return;
    }

	prop_from_actor( prop );
	prop_to_scene( prop, ch->in_scene );
	act( "$n drops $p.", ch, prop, NULL, TO_SCENE );
	act( "You drop $p.", ch, prop, NULL, TO_ACTOR );
    }
    else
    {
	/* 'drop all' or 'drop all.prop' */
	found = FALSE;
	for ( prop = ch->carrying; prop != NULL; prop = prop_next )
	{
	    prop_next = prop->next_content;

        if ( ( arg[3] == '\0' || is_name( &arg[4], STR(prop, name) ) )
	    &&   can_see_prop( ch, prop )
            && INV(prop->wear_loc)
	    &&   can_drop_prop( ch, prop ) )
	    {
		found = TRUE;
		prop_from_actor( prop );
		prop_to_scene( prop, ch->in_scene );
		act( "$n drops $p.", ch, prop, NULL, TO_SCENE );
		act( "You drop $p.", ch, prop, NULL, TO_ACTOR );
	    }
	}

	if ( !found )
	{
	    if ( arg[3] == '\0' )
		act( "You are not carrying anything.",
		    ch, NULL, arg, TO_ACTOR );
	    else
        act( "You are not carrying $T.",
		    ch, NULL, &arg[4], TO_ACTOR );
	}
    }

    return;
}



/*
 * Syntax:  give [prop] [person]
 *          give [number] [money prop] [person]
 */
void cmd_give( PLAYER *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    PLAYER *victim;
    PROP  *prop;
    int amount = 0;

    argument = one_argument( argument, arg1 );

    /*
     * give 3 gold target
     */
    if ( is_number( arg1 ) )
    {
        char buf[MSL];
        char *p;

        amount = atoi( arg1 );        
        argument = one_argument( argument, arg2 );
        argument = one_argument( argument, arg1 );
        if ( !str_cmp( arg1, "to" ) ) 
        argument = one_argument( argument, arg1 );

	if ( ( victim = get_actor_scene( ch, arg1 ) ) == NULL )
	{
	    to_actor( "They aren't here.\n\r", ch );
	    return;
	}

        if ( tally_coins( ch ) < amount ) {
            to_actor( "You don't have that much.\n\r", ch );
            cmd_money( ch, "" );
            return;
        }

        switch ( LOWER(arg2[0]) ) { 
                             case 'c': default: break;
                             case 'g': amount *= 100; break; 
                             case 'e': amount *= 1000;
                             case 'p': amount *= 10000; break;
                             case 's': amount *= 10; break;
        }

        p = sub_coins(amount, ch );
        snprintf( buf, MAX_STRING_LENGTH, "You pay %s %s.\n\r",
                 NAME(victim), name_amount( amount ) );
        to_actor( buf, ch );

        create_amount( amount, victim, NULL, NULL );   
        snprintf( buf, MAX_STRING_LENGTH, "%s pays you %s.\n\r",
                 NAME(ch), name_amount( amount ) );
        to_actor( buf, victim );

        script_update( victim, TYPE_ACTOR, TRIG_GETS, ch, NULL, "coins", p );
        return;
    }

   /*
    * give object target
    */
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	to_actor( "Give what to whom?\n\r", ch );
	return;
    }

	if ( ( victim = get_actor_scene( ch, arg2 ) ) == NULL )
	{
	    to_actor( "They aren't here.\n\r", ch );
	    return;
	}

    if ( ( prop = get_prop_inv( ch, arg1 ) ) == NULL )
    {
	to_actor( "You do not have that item.\n\r", ch );
	return;
    }

    if ( !NOT_WORN( prop ) )
    {
        if ( prop->wear_loc == WEAR_WIELD_1
          || prop->wear_loc == WEAR_WIELD_2 )
        {
            if ( IS_SET(prop->extra_flags, ITEM_USED) )
            {
                unequip_char( ch, prop );
                act( "$n stops wielding $p.", ch, prop, NULL, TO_SCENE );
                act( "You stop wielding $p.", ch, prop, NULL, TO_ACTOR );
            }
            prop->wear_loc   = hand_empty( ch );
        }
        else
        {
            to_actor( "You must remove it first.\n\r", ch );
            return;
        }
    }

    if ( ( victim = get_actor_scene( ch, arg2 ) ) == NULL )
    {
	to_actor( "They aren't here.\n\r", ch );
	return;
    }

    if ( !can_drop_prop( ch, prop ) )
    {
	to_actor( "It is impossible to let go of it.\n\r", ch );
	return;
    }

    if ( hand_empty( victim ) == WEAR_NONE  && !IS_IMMORTAL(victim)
      && prop->item_type != ITEM_MONEY )
    {
	act( "$N has $S hands full.", ch, NULL, victim, TO_ACTOR );
	return;
    }

    if ( prop->item_type == ITEM_MONEY )
    {
        int oldval = prop->value[1];

        if ( amount > prop->value[0] || amount == 0 )
        amount = prop->value[0];

        prop->value[0] -= amount;

        update_money( prop );
        prop = create_money( amount, oldval );

        strip_empty_money( ch );
        merge_money( ch );
    }                     

    if ( victim->carry_weight + get_prop_weight( prop ) > can_carry_w( victim ) )
    {
	act( "$N aren't able carry that much weight.", ch, NULL, victim, TO_ACTOR );
	return;
    }

    if ( !can_see_prop( victim, prop ) )
    {
	act( "$N aren't able see it.", ch, NULL, victim, TO_ACTOR );
	return;
    }

    act( "$n gives $p to $N.", ch, prop, victim, TO_NOTVICT );
    act( "$n gives you $p.",   ch, prop, victim, TO_VICT    );
    act( "You give $p to $N.", ch, prop, victim, TO_ACTOR    );

    if ( prop->carried_by != NULL ) prop_from_actor( prop );
    if ( prop->carried_by != NULL ) prop->wear_loc = hand_empty( victim );
    else                           prop->wear_loc = WEAR_NONE;
    prop_to_actor_money( prop, victim );

    script_update( victim, TYPE_ACTOR, TRIG_GETS, ch, prop, STR(prop,name), NULL );
    return;
}



/*
 * Syntax:  fill [prop]
 *          fill [lantern] [prop]
 */
void cmd_fill( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PROP *prop;
    PROP *fountain;
    bool found;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	to_actor( "Fill what?\n\r", ch );
	return;
    }

    if ( ( prop = get_prop_carry( ch, arg ) ) == NULL )
    {
    to_actor( "You do not have that item in your hands.\n\r", ch );
	return;
    }

    if ( prop->item_type == ITEM_DRINK_CON )
    {
         found = FALSE;
         for ( fountain = ch->in_scene->contents; fountain != NULL;
         fountain = fountain->next_content )
         {
         if ( fountain->item_type == ITEM_FOUNTAIN )
         {
             found = TRUE;
             break;
         }
         }

         if ( !found )
         {
         to_actor( "There is no fountain here!\n\r", ch );
         return;
         }

         if ( prop->value[2] != 0 && prop->value[0] > 0 )
         cmd_pour( ch, arg );

         if ( prop->value[0] >= prop->value[1] )
         {
         to_actor( "Your container is full.\n\r", ch );
         return;
         }

         act( "You fill $p with the water from $P.", ch, prop, fountain, TO_ACTOR );
         act( "$n fills $p with water from $P.", ch, prop, fountain, TO_SCENE );
         prop->value[2] = 0; /* fountains are water */
         prop->value[0] = prop->value[1];
    }
    else
    if ( prop->item_type == ITEM_LIGHT )
    {
        PROP *prop2;

        if ( ( prop2 = get_item_held( ch, ITEM_DRINK_CON ) ) == NULL )
        {
        to_actor( "You don't have anything in your hand to fill it with.\n\r", ch );
        return;
        }

        if ( !IS_SET(prop->value[3], LIGHT_FILLABLE) )
        {
        to_actor( "It aren't able be refueled.\n\r", ch );
        return;
        }

        if ( prop2->value[2] != LIQ_OIL )
        {
        to_actor( "There's no oil in that container.\n\r", ch );
        return;
        }

        if ( prop->value[0] != 0 )
        {
        to_actor( "There is already some oil in it.\n\r", ch );
        return;
        }

        if (prop2->value[0] >= prop->value[1] )
        {
            act( "You fill $p with $P.", ch, prop, prop2, TO_ACTOR );
            prop->value[0] = prop->value[1];
            prop2->value[1] += -(prop->value[0]);
        }
        else
        {
            act( "You empty $P into $p.", ch, prop, prop2, TO_ACTOR );
            prop->value[0] = prop2->value[1];
            prop2->value[0] = 0;
            REMOVE_BIT(prop2->extra_flags, ITEM_INVENTORY);
        }
    }
    else
    {
    to_actor( "It is impossible to fill that.\n\r", ch );
    return;
    }

    return;
}



/*
 * Syntax:  pour [prop]
 *          pour [prop] [prop]
 */
void cmd_pour( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    PROP *prop1;
    PROP *prop2 = NULL;

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || ( prop1 = get_prop_here( ch, arg1 ) ) == NULL )
	{
        to_actor( "Pour from what?\n\r", ch );
	    return;
	}
	
	if ( prop1->item_type != ITEM_DRINK_CON
	  || prop1->value[0] <= 0 )
	{
         to_actor( "It is impossible to pour anything from that.\n\r", ch );
         return;
    }		

    if ( arg2[0] == '\0')
	{
        prop1->value[0] = 0;
        act( "You pour $t from $P onto the ground.",
             ch, liq_table[URANGE(0,prop1->value[2],LIQ_MAX)].liq_name,
             prop1, TO_ACTOR );
        act( "$n pours $t from $P onto the ground.",
             ch, liq_table[URANGE(0,prop1->value[2],LIQ_MAX)].liq_name,
             prop1, TO_SCENE );
        prop1->value[3] = 0;
	    return;
	}
	
	if (( prop2 = get_prop_here( ch, arg2 ) ) == NULL)
	{
		act( "Pour $t into what?", ch, prop1, NULL, TO_ACTOR);
		return;
	}

    if ( prop1->item_type != ITEM_DRINK_CON )
    {
        to_actor( "It is impossible to pour anything from that.\n\r", ch );
        return;
    }

    if ( prop1->value[0] <= 0 )
    {
        to_actor( "There is nothing to pour from that.\n\r", ch );
        return;
    }


    if ( prop2->item_type != ITEM_DRINK_CON )
    {
        to_actor( "It is impossible to pour anything into that.\n\r", ch );
        return;
    }

	/*
	 * Avoid mixing.
	 * Careful, a bit of recursion here!
	 */
    if ( prop2->value[2] != prop1->value[2] && prop2->value[0] > 0 )
        cmd_pour( ch, arg2 );

	/*
	 * Make liquid types identical.
	 */
    prop2->value[2] = prop1->value[2];

    snprintf( buf, MAX_STRING_LENGTH, "$n pours %s from $p into $P.",
             liq_table[URANGE(0,prop2->value[2],LIQ_MAX)].liq_name );
    act( buf, ch, prop1, prop2, TO_SCENE );
    snprintf( buf, MAX_STRING_LENGTH, "You pour %s from $p into $P.",
             liq_table[URANGE(0,prop2->value[2],LIQ_MAX)].liq_name );
    act( buf, ch, prop1, prop2, TO_ACTOR );

    if ( prop2->value[0] + prop1->value[0] > prop2->value[1] )
    {
        prop1->value[0] -= (prop2->value[0] + prop1->value[0]) - prop2->value[1];
        prop2->value[0] = prop2->value[1];
        act( "You fill it as much as you can, but $p still contains some liquid.", ch, prop1, prop2, TO_ACTOR );
    }
    else
    {
        prop2->value[0] += prop1->value[0];
        prop1->value[0] = 0;
    }


    return;
}



/*
 * Syntax:  drink [prop]
 */
void cmd_drink( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PROP *prop;
    int amount;
    int liquid;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	for ( prop = ch->in_scene->contents; prop; prop = prop->next_content )
	{
	    if ( prop->item_type == ITEM_FOUNTAIN )
		break;
	}

	if ( prop == NULL )
	{
	    to_actor( "Drink what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( prop = get_prop_here( ch, arg ) ) == NULL )
	{
	    to_actor( "It is impossible to find it.\n\r", ch );
	    return;
	}
    }

    if ( !NPC(ch) && PC(ch,condition[COND_DRUNK]) > IS_DRUNK )
    {
        to_actor( "You fail to reach your mouth.\n\r", ch );
        act( "$n hiccups loudly.", ch, NULL, NULL, TO_SCENE );
	return;
    }

    switch ( prop->item_type )
    {
        default:
        to_actor( "It is impossible to drink from that.\n\r", ch );
        break;

        case ITEM_FOUNTAIN:
        if ( !NPC(ch) && PC(ch,condition)[COND_THIRST] >= IS_FULL/2 )
        {
            to_actor( "You are not thirsty enough.\n\r", ch );
            return;
        }

        if ( !NPC(ch) )
            PC(ch,condition)[COND_THIRST] = IS_FULL/2;
            
        act( "$n drinks from $p.", ch, prop, NULL, TO_SCENE );
        act( "You drink a mighty draught from $p.", ch,
             prop, NULL, TO_ACTOR );
        prop_cast_spell( prop, prop->value[1], ch, ch, NULL );
        prop_cast_spell( prop, prop->value[2], ch, ch, NULL );
        prop_cast_spell( prop, prop->value[3], ch, ch, NULL );
        break;

        case ITEM_DRINK_CON:
        if ( prop->value[0] <= 0 )
        {
            to_actor( "It is already empty.\n\r", ch );
            if ( IS_SET(prop->value[3], DRINK_TAVERN) )
            {
                to_actor( "You discard it.\n\r", ch );
                extractor_prop(prop);
            }
            return;
        }

        if ( ( liquid = prop->value[2]) >= LIQ_MAX || liquid < 0 )
        {
            bug( "Cmd_drink: bad liquid number %d.", prop->pIndexData->dbkey );
            liquid = prop->value[2] = 0;
        }

        if ( liquid == LIQ_OIL )
        {
        act( "$n drinks lamp oil from $p, and spits it out onto the ground in surprise.",
            ch, prop, NULL, TO_SCENE );
        act( "You drink oil from $p and spit it out in disgust.",
            ch, prop, NULL, TO_ACTOR );
        amount = 1;
        }
        else
        {
        act( "$n drinks $T from $p.",
            ch, prop, liq_table[liquid].liq_name, TO_SCENE );
        act( "You drink $T from $p.",
            ch, prop, liq_table[liquid].liq_name, TO_ACTOR );

        to_actor( STR(prop,action_descr), ch );

        amount = number_range(3, 10);
        amount = UMIN(amount, prop->value[0]);

        gain_condition( ch, COND_DRUNK,
            amount * liq_table[liquid].liq_bonus[COND_DRUNK  ] );
        gain_condition( ch, COND_FULL,
            amount * liq_table[liquid].liq_bonus[COND_FULL   ] );
        gain_condition( ch, COND_THIRST,
            amount * liq_table[liquid].liq_bonus[COND_THIRST ] );

        if ( !NPC(ch) && PC(ch,condition)[COND_DRUNK]  > IS_DRUNK )
            to_actor( "You feel drunk.\n\r", ch );
        if ( !NPC(ch) && PC(ch,condition)[COND_FULL]   > IS_FULL )
            to_actor( "You are full.\n\r", ch );
        if ( !NPC(ch) && PC(ch,condition)[COND_THIRST] > IS_FULL )
            to_actor( "You are no longer thirsty.\n\r", ch );

        if ( IS_SET(prop->value[3], DRINK_POISON) )
        {
            /* Poisoned! */
            BONUS af;

            act( "$n chokes and gags.", ch, NULL, NULL, TO_SCENE );
            to_actor( "You choke and gag.\n\r", ch );
            af.type      = skill_dbkey(skill_lookup("poison"));
            af.duration  = 3 * amount;
            af.location  = APPLY_NONE;
            af.modifier  = 0;
            af.bitvector = BONUS_POISON;
            bonus_join( ch, &af );
        }
    }
	
        prop->value[0] -= amount;
        if ( prop->value[0] <= 0 )
        {
            prop->value[0] = 0;
            REMOVE_BIT(prop->extra_flags, ITEM_INVENTORY);
            if ( IS_SET(prop->value[3], DRINK_TAVERN) )
            {
                to_actor( "You discard it.\n\r", ch );
                extractor_prop(prop);
                return;
            }
        }
        
        if ( prop->value[1] <= 0 )
        {
            act( "You discard $p.", ch, NULL, NULL, TO_ACTOR );
            extractor_prop( prop );
            return;
        }
    break;
    }

    return;
}






/*
 * Syntax:  eat [prop]
 */
void cmd_eat( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PROP *prop;
    PROP *prop2;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	to_actor( "Eat what?\n\r", ch );
	return;
    }

    if ( ( prop = get_prop_inv( ch, arg ) ) == NULL )
    {
	to_actor( "You do not have that item.\n\r", ch );
	return;
    }

    if ( !IS_IMMORTAL(ch) )
    {
        if ( prop->item_type != ITEM_FOOD && prop->item_type != ITEM_PILL )
	{
	    to_actor( "That's not edible.\n\r", ch );
	    return;
	}

        if ( !NPC(ch) && PC(ch,condition)[COND_FULL] > IS_FULL )
	{   
	    to_actor( "You are too full to eat more.\n\r", ch );
	    return;
	}
    }
    else
    {
        if ( prop->item_type == ITEM_LIGHT && IS_LIT(prop) )
            --ch->in_scene->light;
    }

    act( "$n eats $p.",  ch, prop, NULL, TO_SCENE );
    act( "You eat $p.", ch, prop, NULL, TO_ACTOR );

    if ( prop->item_type == ITEM_FOOD )
    to_actor( STR(prop,action_descr), ch );

    switch ( prop->item_type )
    {

    case ITEM_FOOD:
	if ( !NPC(ch) )
	{
	    int condition;

        condition = PC(ch,condition)[COND_FULL];
            gain_condition( ch, COND_FULL, prop->value[0] );
            
        if ( condition == 0 && PC(ch,condition)[COND_FULL] >= IS_FULL/2 )
                to_actor( "You are satisfied.\n\r", ch );
        else if ( PC(ch,condition)[COND_FULL] > IS_FULL )
                to_actor( "You are full.\n\r", ch );
        }
  
        if ( prop->value[3] != 0 )
        {
        /* Poisoned! */
	    BONUS af;

	    act( "$n chokes and gags.", ch, 0, 0, TO_SCENE );
	    to_actor( "You choke and gag.\n\r", ch );

	    af.type      = skill_dbkey(skill_lookup("poison"));
	    af.duration  = 2 * prop->value[0];
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = BONUS_POISON;
	    bonus_join( ch, &af );
	}

        /*
         * Slap the reference prop on 'em.
         */
        if ( get_prop_template( prop->value[2] ) != NULL )
        {
        prop2 = create_prop( get_prop_template( prop->value[2] ), 0 );
        prop_to_actor( prop2, ch );
        if ( IS_SET(prop->extra_flags, ITEM_USED) )
        SET_BIT(prop2->extra_flags, ITEM_USED);
        prop2->wear_loc = prop->wear_loc;
        }
	break;

    case ITEM_PILL:
	prop_cast_spell( prop, prop->value[1], ch, ch, NULL );
	prop_cast_spell( prop, prop->value[2], ch, ch, NULL );
	prop_cast_spell( prop, prop->value[3], ch, ch, NULL );
	break;
    }

    extractor_prop( prop );
    return;
}


/*
 * Syntax:  quaff [potion]
 */
void cmd_quaff( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PROP *prop;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        to_actor( "Quaff what?\n\r", ch );
        return;
    }

    if ( ( prop = get_prop_inv( ch, arg ) ) == NULL )
    {
        to_actor( "You do not have that potion.\n\r", ch );
        return;
    }

    if ( prop->item_type != ITEM_POTION )
    {
        cmd_drink( ch, argument );
        return;
    }

    act( "$n quaffs $p.", ch, prop, NULL, TO_SCENE );
    act( "You quaff $p.", ch, prop, NULL ,TO_ACTOR );

    to_actor( STR(prop,action_descr), ch );

    prop_cast_spell( prop, prop->value[1], ch, ch, NULL );
    prop_cast_spell( prop, prop->value[2], ch, ch, NULL );
    prop_cast_spell( prop, prop->value[3], ch, ch, NULL );

    extractor_prop( prop );
    return;
}


/*
 * Syntax:  steal [prop] [person]
 */
void cmd_steal( PLAYER *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    PLAYER *victim;
    PROP *prop;
    int percent;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        to_actor( "Steal what from whom?\n\r", ch );
        return;
    }

    if ( hand_empty( ch ) == WEAR_NONE )
    {
        to_actor( "You'll need to free a hand first.\n\r", ch );
        return;
    }

    if ( ( victim = get_actor_scene( ch, arg2 ) ) == NULL )
    {
        to_actor( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
    to_actor( "Like a dull sword, that's pointless.\n\r", ch );
        return;
    }

    { SKILL *pSkill = skill_lookup( "pickpocket" );
    WAIT_STATE( ch, pSkill ? pSkill->delay : 0 );
    percent  = number_percent( ) + ( IS_AWAKE(victim) ? 10 : -50 );
    }

    if ( victim->position == POS_FIGHTING
    || !skill_check( ch, skill_lookup( "pickpocket" ), 10 ) )
    {
        /*
         * Failure.
         */
    ch->bounty += 10;
        to_actor( "Oops.\n\r", ch );
        act( "$n tried to steal from you.\n\r", ch, NULL, victim, TO_VICT    );
        act( "$n tried to steal from $N.\n\r",  ch, NULL, victim, TO_NOTVICT );
    snprintf( buf, MAX_STRING_LENGTH, "%s is a bloody thief!", NAME(ch) );
    to_actor( buf, victim );

        if ( !NPC(ch) )
        {
            if ( NPC(victim) )
            {
        oroc( victim, ch );
            }
            else
            {
                log_string( buf );
            }
        }

        return;
    }

    if ( ( prop = get_prop_inv( victim, arg1 ) ) == NULL )
    {
        to_actor( "It is impossible to find it.\n\r", ch );
        return;
    }
        
    if ( !can_drop_prop( ch, prop ) || ( !skill_check( ch, skill_lookup( "pickpocket" ), 0 ) ) )
    {
    to_actor( "It is impossible to get it away.\n\r", ch );
        return;
    }

    if ( ch->carry_weight + get_prop_weight( prop ) > can_carry_w( ch ) )
    {
        to_actor( "It is impossible to carry that much weight.\n\r", ch );
        return;
    }

    if ( IS_SET(prop->extra_flags, ITEM_INVENTORY) )
    {
        ch->bounty += 30;
        prop = create_prop( prop->pIndexData, 0 );
    }
    else
    prop_from_actor( prop );

    prop_to_actor( prop, ch );
    prop->wear_loc = hand_empty( ch );
    to_actor( "Stolen.\n\r", ch );
    return;
}



/*
 * Syntax:  light [prop]
 */
void cmd_light( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PROP *prop;
    PROP *prop2;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
        prop = get_item_held( ch, ITEM_LIGHT );
   else prop = get_prop_carry( ch, arg );

    prop2 = get_tool_char( ch, TOOL_TINDERBOX );

    /* Lamplighters     */
    if ( prop == NULL )
    prop = get_prop_here( ch, arg );

    if ( prop == NULL )
    {
        to_actor( "You don't have the light in your hands.\n\r", ch );
        return;
    }

    if ( IS_LIT( prop ) )
    {
        to_actor( "It's lit already!\n\r", ch );
        return;
    }

    if ( prop->value[0] <= 0 )
    {
        to_actor( "It's out of fuel.\n\r", ch );
        return;
    }

    if ( prop2 == NULL )
    {
        to_actor( "You don't have anything to light it with!\n\r", ch );
        return;
    }

    if ( use_tool( prop2, TOOL_TINDERBOX ) )
    {
        SET_BIT(prop->value[3], LIGHT_LIT);
        act( "$n lights $p with $P.", ch, prop, prop2, TO_SCENE );
        act( "You light $p with $P.", ch, prop, prop2, TO_ACTOR );

        ch->in_scene->light++;
        return;
    }
    else
    {
        act( "You cannot light $p with $P.", ch, prop, prop2, TO_ACTOR ); 
        if ( prop2->value[1] <= 0 ) { to_actor( "It's all used up.\n\r", ch ); }
        return;
    }

    return;
}



/*
 * Syntax:  extinguish [prop]
 */
void cmd_extinguish( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PROP *prop;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
        prop = get_item_held( ch, ITEM_LIGHT );
   else prop = get_prop_carry( ch, arg );

    /* Lamplighters     */
    if ( prop == NULL )
    prop = get_prop_here( ch, arg );

    if ( prop == NULL )
    {
        to_actor( "You don't have the light in your hands.\n\r", ch );
        return;
    }

    if ( !IS_LIT( prop ) )
    {
        to_actor( "It's extinguished already!\n\r", ch );
        return;
    }

    to_actor( STR(prop,action_descr), ch );

    REMOVE_BIT( prop->value[3], LIGHT_LIT );
    act( "$n extinguishes $p.", ch, prop, NULL, TO_SCENE );
    act( "You extinguish $p.", ch, prop, NULL, TO_ACTOR );

    ch->in_scene->light--;
    return;
}



void cmd_swap( PLAYER *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    PROP *prop1, *prop2;
    
    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );
    
    /*
     * Do the default -- swap between hands.
     */
    if ( arg1[0] == '\0' 
      || arg2[0] == '\0' )
    {
	    prop1 = get_eq_char( ch, WEAR_WIELD_1 );
    	if ( prop1 == NULL )
	    prop1 = get_eq_char( ch, WEAR_HOLD_1 );

	    prop2 = get_eq_char( ch, WEAR_WIELD_2 );
	    if ( prop2 == NULL )
	    prop2 = get_eq_char( ch, WEAR_HOLD_2 );

	    if ( prop1 && prop2 )
	    act( "$n swap$v $p and $P between $s two hands.", 
	         ch, prop1, prop2, TO_ALL );
	    else
	    if ( prop1 && !prop2 )
	    act( "$n swap$v $s $p into $s secondary hand.", 
	         ch, prop1, NULL, TO_ALL );
	    else
	    if ( !prop1 && prop2 )
	    act( "$n swap$v $s $p into $s primary hand.", 
	         ch, prop2, NULL, TO_ALL );
	    else
	    {
	        to_actor( "You have nothing in your hands.\n\r", ch );
    	    return;
	    }

    	if ( prop1 )
	    {
	        if ( prop1->wear_loc == WEAR_HOLD_1 )   prop1->wear_loc = WEAR_HOLD_2;
	        if ( prop1->wear_loc == WEAR_WIELD_1 )  prop1->wear_loc = WEAR_WIELD_2;
	    }

    	if ( prop2 )
	    {
	        if ( prop2->wear_loc == WEAR_HOLD_2 )   prop2->wear_loc = WEAR_HOLD_1;
	        if ( prop2->wear_loc == WEAR_WIELD_2 )  prop2->wear_loc = WEAR_WIELD_1;
	    }
	}
	else
	{
		PROP *con1 = NULL;
		PROP *con2 = NULL;
		
		prop1 = get_prop_carry( ch, arg1 );
		prop2 = get_prop_carry( ch, arg2 );
		
		/*
		 * Search containers on PC for first prop.
		 */
		if ( prop1 == NULL )
		{
		    for ( con1 = ch->carrying; con1 != NULL && prop1 == NULL;  con1 = con1->next_content )
		    {		           
		    	if ( can_see_prop( ch, con1 ) 
		    	  && ( ( con1->item_type == ITEM_CONTAINER
    		    	  || con1->item_type == ITEM_FURNITURE )
        	          && !IS_SET(con1->value[1], CONT_CLOSED) ) )
		    	prop1 = get_prop_list( ch, arg1, con1->contains );
		    }
		}
		
		if ( prop1 == NULL )
		{
			to_actor( "Swap what?\n\r", ch);
			return;
		}
				
		/*
		 * Search containers on PC for second prop.
		 */
		if ( prop2 == NULL )
		{
		    for ( con2 = ch->carrying; con2 != NULL && prop2 == NULL;  con2 = con2->next_content )
		    {		           
		    	if ( can_see_prop( ch, con2 ) 
		    	  && ( ( con2->item_type == ITEM_CONTAINER
    		    	  || con2->item_type == ITEM_FURNITURE )
        	          && !IS_SET(con2->value[1], CONT_CLOSED) ) )
		    	prop2 = get_prop_list( ch, arg2, con2->contains );
		    }
		}
		
		if ( prop2 == NULL )
		{
			to_actor( "Swap what?\n\r", ch );
			return;
		}
		
		if ( (con1 == NULL && con2 == NULL) 
		  || (con1 != NULL && con2 != NULL) )
		{
			to_actor( "Swap what with what?\n\r", ch );
			return;
		}
		
		/*
		 * CHECK FOR WEIGHT RESTRAINTS / can_drop_prop!
		 * NEEDS TO BE ADDED HERE!!!!!
		 */
		if ( con1 )
		{
			con1 = prop1->in_prop;
			prop_from_prop( prop1 );         /* grab first prop */
			prop_to_actor( prop1, ch );      /* give it to the char */
			prop1->wear_loc = prop2->wear_loc;
			
			prop_from_actor( prop2 ); 
			prop_to_prop( prop2, con1 );     /* put it in the original */
		}		
		else
		{
			con2 = prop2->in_prop;
			prop_from_prop( prop2 );         /* grab first prop */
			prop_to_actor( prop2, ch );      /* give it to the char */
			prop2->wear_loc = prop1->wear_loc;
			
			prop_from_actor( prop1 ); 
			prop_to_prop( prop1, con2 );     /* put it in the original */
		}

        snprintf( buf, MAX_STRING_LENGTH, "$n swap$v $p from %s with $P.",
               can_see_prop(ch, con1) ? STR(con1,short_descr) : "something" );
        act( buf, ch, prop1, prop2, TO_ALL );
	}
	
    return;
}




/*
 * cmd_raise and cmd_lower originally by Morgenes
 * Slightly modified by Daurven.  :)
 * raise [prop]
 */
void cmd_raise( PLAYER *ch, char *argument ) {
    PROP *hood;
    PROP *tmp;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( ( hood = get_prop_inv( ch, arg ) ) == NULL ) {
       STC( "You aren't wearing anything like that.\n\r", ch );
       return;
    }

    if ( hood->item_type != ITEM_CLOTHING || !IS_SET( hood->value[1],
HOODED ) ) {
       STC( "It is impossible to find the hood to raise.\n\r", ch );
       return;
    }

    if ( IS_SET( hood->value[1], HOOD_UP ) ) {
       STC( "The hood is already up!\n\r",ch );
       return;
    }

    if ( hood->wear_loc != WEAR_ABOUT
    && ( tmp = get_eq_char( ch, WEAR_ABOUT ) ) != NULL
    && tmp->item_type == ITEM_CLOTHING && IS_SET( tmp->value[1], HOODED )
    && IS_SET( tmp->value[1], HOOD_UP ) ) {
     snprintf( buf, MAX_STRING_LENGTH, "You must first lower the hood on your %s.\n\r",
        smash_article( format_prop_to_actor( tmp, ch, TRUE ) ) );
       STC( buf, ch );
       return;
    }

    act( "$n raises the hood of $p.", ch, hood, NULL, TO_SCENE );
    act( "You raise the hood of $p.", ch, hood, NULL, TO_ACTOR );
    SET_BIT( hood->value[1], HOOD_UP );
 free_string( ch->hkeywords );
 ch->hkeywords = str_dup( "hooded figure hood" );
    return;
}



/*
 * lower [prop]
 */
void cmd_lower( PLAYER *ch, char *argument ) {
    PROP *hood;
    PROP *tmp;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    one_argument( argument, arg );

    if ( ( hood = get_prop_inv( ch, arg ) ) == NULL ) {
       STC( "You aren't wearing anything like that.\n\r", ch );
       return;
    }

    if ( hood->item_type != ITEM_CLOTHING || !IS_SET( hood->value[1],
HOODED ) ) {
       STC( "It is impossible to find the hood to raise.\n\r", ch );
       return;
    }

    if ( !IS_SET( hood->value[1], HOOD_UP ) ) {
        STC( "The hood is already down!\n\r",ch );
        return;
    }

    if ( hood->wear_loc != WEAR_ABOUT
    && ( tmp = get_eq_char( ch, WEAR_ABOUT ) ) != NULL
    && tmp->item_type == ITEM_CLOTHING && IS_SET( tmp->value[1], HOODED )
    && IS_SET( tmp->value[1], HOOD_UP ) ) {
       snprintf( buf, MAX_STRING_LENGTH, "You must first lower the hood on your %s.\n\r",
        smash_article( format_prop_to_actor( tmp, ch, TRUE ) ) );
       STC( buf, ch );
       return;
    }

    REMOVE_BIT( hood->value[1], HOOD_UP );
    act( "$n lowers the hood of $p.", ch, hood, NULL, TO_SCENE );
    act( "You lower the hood of $p.", ch, hood, NULL, TO_ACTOR );
    free_string( ch->hkeywords );
    ch->hkeywords = str_dup( "" );
    return;
}



/*
 * Player-written books and notes.
 * Write [target]
 */
void cmd_write( PLAYER *ch, char *argument ) {
   PROP *quill;
   PROP *note;

   if ( NPC(ch) ) return;

   note = get_prop_carry( ch, argument );
  
   if ( !note ) { to_actor( "You cannot find that to write on.\n\r", ch );  return; }

   for ( quill = ch->carrying; quill != NULL; quill = quill->next_content ) {
      if ( quill->item_type == ITEM_QUILL ) break;
   }

   if ( !quill ) { to_actor( "You cannot find anything to write with.\n\r", ch ); return; }

   if ( !note->action_descr ) note->action_descr = str_dup("");
   act( "$n begins writing on $p with $P.", ch, note, quill, TO_SCENE );
   string_append( ch, &note->action_descr );
   
}
