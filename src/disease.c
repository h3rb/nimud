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
 ******************************************************************************
 *   n a m e l e s s i n c a r n a t e          dedicated to chris cool       *
 ******************************************************************************/

/* Lycanthropics and vampirism */
/* possible future idea: jeckle-n-hyde type changlings */

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
#include "skills.h"
#include "defaults.h"

#define CON(x)    ( URANGE(0,get_curr_con(x),25) )
#define DEX(x)    ( URANGE(0,get_curr_dex(x),25) )



/*
 * Anyone (PC or NPC) may attempt to bite another.
 */
void cmd_bite( PLAYER *ch, char *argument ) {

    PLAYER *vict;

    if ( !IS_LYCANTHROPIC(ch) 
      && !IS_VAMPIRE(ch) ) {
       to_actor( "You bite your nails, silly you.\n\r", ch );
       return;
    }

    vict = get_actor_scene( ch, argument );

    if ( !vict ) {
       to_actor( "Who?\n\r", ch );
       return;
    }

    if ( IS_VAMPIRE(ch) && IS_VAMPIRE(vict) ) {
       to_actor( "Ah, but you are both already vampires...\n\r", ch );
       return;
    }

    if ( !NPC(vict) && vict->position != POS_SLEEPING ) {
       to_actor( "You may only bite other players while they are sleeping.\n\r", ch );
       return;
    }

    /* no skill for this, just a percentage chance based on dex */
    if ( number_range( 0, 100 ) >= 50+dex_app[DEX(ch)].defensive
      && number_range( 0, 100 ) >= 50+dex_app[DEX(vict)].defensive ) {
    
        display_interp( ch, "^2" );
        display_interp( vict, "^2" );
        act("$n bite$v $N hard on the neck!", ch, NULL, vict, TO_ALL );

        if ( number_range( 0, 100 ) >= con_app[CON(vict)].shock
        && number_range( 0, 100 ) >= con_app[CON(vict)].resistance ) {
   
               char buf[MAX_STRING_LENGTH];

            display_interp( vict, "^B" ); 
            to_actor( "You feel different.\n\r", ch );

            /* Transfer of the problem.. */
            if ( IS_VAMPIRE(ch) ) {
               if ( NPC(vict) ) SET_BIT(ch->flag, ACTOR_VAMPIRE);
               else SET_BIT(ch->flag2, PLR_VAMPIRE);
               sprintf( buf, "%s was bitten and has joined the brood.", NAME(vict) );
               add_history( vict, buf );
            }
            else {
               if ( NPC(vict) ) SET_BIT(ch->flag, ACTOR_LYCANTHROPE);
               else SET_BIT(ch->flag2, PLR_LYCANTHROPE);
               sprintf( buf, "%s contracts the disease of lycanthropy.", NAME(vict) );
               add_history( vict, buf );
            }
            
        }
        else {
            BONUS af;

            display_interp( ch, "^2" );
            act( "$n chokes and gags.", vict, NULL, NULL, TO_SCENE );
            to_actor( "You choke and gag.\n\r", vict );
            af.type      = skill_dbkey(skill_lookup("poison"));
            af.duration  = 5;
            af.location  = APPLY_CON;
            af.modifier  = -1;
            af.bitvector = BONUS_POISON;
            bonus_join( vict, &af );
        }
        to_actor( "Your teeth sink deep into tasty flesh.\n\r", ch );

        if ( IS_VAMPIRE(ch) && !IS_VAMPIRE(vict) ) {

	BONUS af;

	af.type      = skill_dbkey(skill_lookup("lycanthropy"));
	af.duration  = 5;
	af.location  = APPLY_STR;
	af.modifier  = 1;
	af.bitvector = BONUS_METAMORPH;
	bonus_join( ch, &af );

        to_actor( "You draw your life's energy from the blood you taste.\n\r", ch );
        }
        
    }
    else {
         act( "$N slip$v away before $n can bite $M.", ch, NULL, vict, TO_NOTVICT );
         act( "You avoid being bitten by $N.", vict, NULL, ch, TO_ACTOR );
    }

    display_interp( ch, "^N" );
    display_interp( vict, "^N" );
    return;
};



/*
 * Routine called by update.c when the moon turns full.
 * Changes descripts and augments player statistics temporarily 
 * until moon changes phase.
 *
 * Phase-based changes effect only PCs.  Use scripts
 * to create the illusion for NPCs.
 */
void change_lycanthropes( bool fWere ) {

    PLAYER *ch;

    for ( ch = actor_list;  ch != NULL; ch = ch->next ) {
    
     if ( !IS_LYCANTHROPIC(ch) )
         continue;

     if( fWere ) {
		BONUS af;

		display_interp( ch, "^B" );
		act( "$n grows claws and teeth because of $s disease.", ch, NULL, NULL, TO_SCENE );
		to_actor( "You growl and snarl, growing long teeth and fur.\n\r", ch );

		af.type      = skill_dbkey(skill_lookup("lycanthropy"));
		af.duration  = 10;
		af.location  = APPLY_STR;
		af.modifier  = 10;
		af.bitvector = BONUS_METAMORPH;
		bonus_join( ch, &af );
                display_interp( ch, "^N" );

           free_string( ch->short_descr );
           ch->short_descr = str_dup( "a werewolf" );
           free_string( ch->long_descr );
           ch->long_descr = str_dup( "A long-haired werewolf growls and snarls.\n\r" );           
     }
     else {
        if ( ch->short_descr != NULL ) {
	free_string( ch->short_descr );
        free_string( ch->long_descr );
        ch->short_descr = NULL;
	ch->long_descr = NULL;

        to_actor( "You return to human form.\n\r", ch );
  	act( "$n returns to human form.", ch, NULL, NULL, TO_SCENE );
        }
     }
        
    }

    return;
};
