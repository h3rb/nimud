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
#include <string.h>
#include <time.h>

#include <zlib.h>

#include "nimud.h"
#include "script.h"
#include "skills.h"
#include "defaults.h"

char *	const	dir_name	[]		=
{
    "north", "east", "south", "west", "up", "down", 
    "northwest", "northeast", "southwest", "southeast"
};

char *  const   dir_rev     []      =
{
    "the south", "the west", "the north", "the east", "below", "above",
    "the southeast", "the southwest", "the northeast", "the northwest"
};

char *  const   dir_letter     []      =
{
    "n", "e", "s", "w", "u", "d", "NW", "NE", "SW", "SE"
};

const   int  rev_dir     []      =
{
    DIR_SOUTH,
    DIR_WEST,
    DIR_NORTH,
    DIR_EAST,
    DIR_DOWN,
    DIR_UP,
    DIR_SE,
    DIR_SW,
    DIR_NE,
    DIR_NW   
};

const	int	movement_loss	[MOVE_MAX]	=
{
    1,   /* MOVE_INSIDE       */
    1,   /* MOVE_CITY         */
    2,   /* MOVE_FIELD        */
    4,   /* MOVE_FOREST       */
    5,   /* MOVE_HILLS        */
    8,   /* MOVE_MOUNTAIN     */
    4,   /* MOVE_WATER_SWIM   */
    1,   /* MOVE_WATER_NOSWIM */
    6,   /* MOVE_UNDERWATER   */
   10,   /* MOVE_AIR          */
    6,   /* MOVE_DESERT       */
   10,   /* MOVE_ICELAND      */
    5    /* MOVE_CLIMB        */
};


void flee( PLAYER *ch, PLAYER *fighting, int d );



void hide_check( PLAYER *ch, SCENE *in_scene )
{
    PLAYER *rch;

    if ( !IS_AFFECTED(ch,BONUS_HIDE) )
    return;

    for ( rch = in_scene->people;  rch != NULL;  rch = rch->next_in_scene )
    {
        if ( rch == ch ) continue;

        if ( skill_check( rch, skill_lookup("camoflage"), 25 ) )
        {
            act( "You notice $N trying to hide $S presence.", rch, NULL, ch, TO_ACTOR );
            if ( skill_check( ch, skill_lookup("camoflage"), 25 ) )
            {
            act( "$N notices you trying to hide your presence.", ch, NULL, rch, TO_ACTOR );
            }
        }
    }

    return;
}



/*
 * Do flee checks.
void flee( PLAYER *ch, PLAYER *fighting, int d )
{
    PLAYER *fch;
    int penalty = 0, fdex, chdex, chstr, fstr;


    chdex = number_range( 0, get_curr_dex(ch) );
    fdex  = number_range( 0, get_curr_dex(fighting) );

    chstr = number_range( 0, get_curr_str(ch) );
    fstr  = number_range( 0, get_curr_str(fighting) );

    for( fch = ch->in_scene->people; fch != NULL; fch = fch->next_in_scene )
    {
        if ( fch->fighting != ch ) continue;

        penalty++;
        if ( chdex < fdex - (fch == fighting ? -1 : penalty) )
        {
            act( "$4You attack $N as $E turns to run!$R", fch, NULL, ch, 
TO_ACTOR );
            act( "$4$n attacks you as you turn to run!$R", fch, NULL, ch, 
TO_VICT );
            act( "$4$n attacks $N as $e turns to run!$R", fch, NULL, ch, 
TO_NOTVICT );
            fch->fbits = 0;
            oroc( fch, ch );
        }
        else
        if ( chstr < fstr - penalty
          || skill_check( ch, skill_lookup("evasion"), 0 ) )
        {
            act( "$B$N block$V $n as $e tries to escape $t!$N",
                 ch, dir_name[d], fch, TO_ALL );
            return;
        }
    }

    stop_fighting( ch, TRUE );
    ch->position = POS_FIGHTING;
}
 */


void leave_strings( PLAYER *ch, PROP *prop, int sect, int door, bool fWindow )
{
    char buf[MAX_STRING_LENGTH];
    PLAYER *fch;
    PLAYER *fch_next;

    if ( ch->position == POS_FIGHTING )
    sprintf( buf, "You turn tail and flee %s", dir_name[door] );
    else
    if ( prop != NULL )
        sprintf( buf, "You use %s to %s %s",
                      can_see_prop( ch, prop ) ? STR(prop, short_descr) : "something",
                      sect == MOVE_MOUNTAIN       ? "climb" :
                      sect == MOVE_CLIMB         ? "climb" :
                      sect == MOVE_WATER_NOSWIM   ? "sail" :
                      sect == MOVE_AIR            ? "fly" :
                      sect == MOVE_UNDERWATER     ? "dive" : "move",
                      dir_name[door] );
    else
    if ( ch->rider != NULL )
        sprintf( buf, "You wander %s, rode by %s", dir_name[door],
                         PERS(ch->rider, ch) );
    else
    if ( ch->riding != NULL ) {
        if ( ch->desc ) sendcli( ch->desc, "PLAYSOUND hooves.wav");
        sprintf( buf, "You ride %s %s", PERS(ch->riding, ch),
                         dir_name[door] );
        }
    else
    if ( IS_AFFECTED(ch, BONUS_FLYING) )
        sprintf( buf, "You fly %s", dir_name[door] );
    else
    if ( IS_AFFECTED(ch, BONUS_SNEAK) )
        sprintf( buf, "You try to sneak %s", dir_name[door] );
    else
    if ( sect == MOVE_WATER_SWIM || sect == MOVE_UNDERWATER ) {
        if ( ch->desc ) sendcli( ch->desc, "PLAYSOUND swim.wav" );
        sprintf( buf, "You swim %s", dir_name[door] );
      }
    else
    if ( sect == MOVE_CLIMB || fWindow == TRUE ) {
        if ( ch->desc ) sendcli( ch->desc, "PLAYSOUND pick.wav" );
        sprintf( buf, "You climb %s", dir_name[door] );
    }
    else
    if ( ch->hitched_to == NULL ) {
        if ( ch->desc ) sendcli( ch->desc, "PLAYSOUND footsteps.wav" );
        sprintf( buf, "You walk %s",  dir_name[door] );
     }
    else
    if ( ch->position == POS_STANDING ) 
        sprintf( buf, "You drag %s %s",
                      PERSO(ch->hitched_to, ch),
                      dir_name[door] );

    buf[0] = UPPER(buf[0]);
    to_actor( buf, ch );

    if ( ch->in_scene->exit[door]
      && !MTD(ch->in_scene->exit[door]->keyword) )
    {
        if ( !str_cmp( ch->in_scene->exit[door]->keyword, "debris" ) )
        to_actor( " through ", ch );
        else {
         if ( !str_infix( ch->in_scene->exit[door]->keyword, "path" ) )
         to_actor( " down a", ch );        
         else
         if ( !str_infix( ch->in_scene->exit[door]->keyword, "ledge" ) )
         to_actor( " along a", ch );
         else
         to_actor( " through a", ch );

         if ( IS_SET(ch->in_scene->exit[door]->exit_flags, EXIT_ISDOOR) ) 
         to_actor( "n open ", ch );
         else {if ( IS_VOWEL(ch->in_scene->exit[door]->keyword[0]) )
                to_actor( "n ", ch ); 
           else to_actor( " ", ch );}
        }
        to_actor( ch->in_scene->exit[door]->keyword, ch );
    }
    to_actor( ch->position == POS_FIGHTING ? "!\n\r" : ".\n\r", ch );

    for ( fch = ch->in_scene->people; fch != NULL; fch = fch_next )
    {
        fch_next = fch->next_in_scene;

        if ( ch == fch
          || ch->riding == fch
          || ch->rider == fch)
        continue;

        if ( !can_see( fch, ch )
          || !IS_AWAKE(fch) )
        continue;

        if ( ch->rider != NULL )
        break;

        if ( ch->position == POS_FIGHTING ) {
         sprintf( buf, "%s turns and flees %s", PERS(ch, fch), dir_name[door] );
         if ( ch->desc ) sendcli( ch->desc, "PLAYSOUND flee.wav" );
        }
   else if ( prop != NULL )
        sprintf( buf, "%s uses %s to %s %s",
                      PERS(ch, fch),
                      can_see_prop(fch, prop) ? STR(prop, short_descr) : "something",
                      sect == MOVE_MOUNTAIN ? "climb" :
                      sect == MOVE_WATER_NOSWIM ? "sail" :
                      sect == MOVE_AIR ? "fly" :
                      sect == MOVE_UNDERWATER ? "dive" : "move",
                      dir_name[door] );
   else if ( ch->riding != NULL &&
        (can_see(fch, ch) || can_see(fch, ch->riding)) )
        {
        if ( ch->riding->hitched_to != NULL )
        {
              sprintf( buf, "%s rides %s upon %s, pulling %s behind them",
                            PERS(ch, fch), dir_name[door],
                            PERS(ch->riding, fch),
                            PERSO(ch->riding->hitched_to, fch) );
        }
        else
        {
              sprintf( buf, "%s rides %s %s", PERS(ch, fch),
                            PERS(ch->riding, fch), dir_name[door] );
        }
        }
   else if ( IS_AFFECTED(ch, BONUS_FLYING) && can_see(fch, ch) )
           sprintf( buf, "%s flies %s", PERS(ch, fch), dir_name[door] );
   else if ( IS_AFFECTED(ch, BONUS_SNEAK) )
        {
            if ( IS_SET(fch->flag2, WIZ_HOLYLIGHT)
              || skill_check(fch, skill_lookup("stealth"), 25) )
           sprintf( buf, "%s tries to sneak %s", PERS(ch, fch), dir_name[door] );
           else buf[0] = '\0';
        }
   else if ( sect == MOVE_WATER_SWIM || sect == MOVE_UNDERWATER )
         sprintf( buf, "%s swims %s",  PERS(ch, fch), dir_name[door] );
   else if ( sect == MOVE_CLIMB )
            sprintf( buf, "%s climbs %s",  PERS(ch, fch), dir_name[door] );
   else if ( ch->position == POS_STANDING )
        {
        if ( ch->hitched_to == NULL || !can_see_prop(fch, ch->hitched_to) )
        sprintf( buf, "%s walks %s",  PERS(ch, fch), dir_name[door] );
   else sprintf( buf, "%s walks %s dragging %s",
                      PERS(ch, fch), dir_name[door],
                      STR(ch->hitched_to,short_descr) );
        }

        buf[0] = UPPER(buf[0]);
        to_actor( buf, fch );
        if ( buf[0] != '\0' )
        {
            if ( ch->in_scene->exit[door]
             && !MTD(ch->in_scene->exit[door]->keyword) )
            {
                to_actor( " through an open ", fch );
                to_actor( ch->in_scene->exit[door]->keyword, fch );
            }
            to_actor( ch->position == POS_FIGHTING ? "!\n\r" : ".\n\r", fch );
        }

        sprintf( buf, "%d", door );
        if (script_update( fch, TYPE_ACTOR, TRIG_MOVES, ch, NULL, buf, NULL ) == 2)
        return;
    }
}


void arrive_strings( PLAYER *ch, PROP *prop, int sect, int door, bool fWindow )
{
    char buf[MAX_STRING_LENGTH];
    PLAYER *fch;
    PLAYER *fch_next;

    for ( fch = ch->in_scene->people; fch != NULL; fch = fch_next )
    {
        fch_next = fch->next_in_scene;

        if (ch == fch
         || ch->riding == fch
         || ch->rider == fch)
        continue;

        if ( !can_see( fch, ch )
          || !IS_AWAKE(fch) )
        continue;

        if ( ch->rider != NULL )
        break;

        if ( ch->position == POS_FIGHTING )
        sprintf( buf, "%s flees from %s", PERS(ch, fch), dir_rev[door] );
   else if ( prop != NULL )
        sprintf( buf, "%s uses %s to %s %s",
                 PERS(ch, fch),
                 PERSO(prop, fch),
                 sect == MOVE_MOUNTAIN     ? "climb" :
                 sect == MOVE_WATER_NOSWIM ? "sail" :
                 sect == MOVE_AIR          ? "fly" :
                 sect == MOVE_UNDERWATER   ? "dive" : "move",
                 dir_name[door] );
   else if ( ch->riding != NULL )
        sprintf( buf, "%s rides %s in from %s", PERS(ch, fch),
                 PERS(ch->riding, fch), dir_rev[door] );
   else if ( IS_AFFECTED(ch, BONUS_FLYING) )
        sprintf( buf, "%s flies in from %s", PERS(ch, fch), dir_rev[door] );
   else if ( IS_AFFECTED(ch, BONUS_SNEAK) && skill_check( fch, skill_lookup("stealth"), 75 ) )
        {
        if ( IS_SET(fch->flag2, WIZ_HOLYLIGHT)
          || skill_check( fch, skill_lookup("stealth"), 25 ) )
        sprintf( buf, "%s tries to sneak in from %s", PERS(ch, fch), dir_rev[door] );
        else buf[0] = '\0';
        }
   else if ( sect == MOVE_WATER_SWIM || sect == MOVE_UNDERWATER )
        sprintf( buf, "%s swims in from %s",  PERS(ch, fch), dir_rev[door] );
   else if ( sect == MOVE_CLIMB )
        sprintf( buf, "%s climbs in from %s",  PERS(ch, fch), dir_rev[door] );
   else if ( ch->position == POS_STANDING )
    {
        if ( fWindow ) 
        sprintf( buf, "%s climbs in through an open %s", PERS(ch,fch),
             ch->in_scene && ch->in_scene->exit[door] ? 
             ch->in_scene->exit[door]->keyword : "window" );
   else if ( ch->hitched_to == NULL || !can_see_prop(fch, ch->hitched_to) )
        sprintf( buf, "%s walks in from %s",  PERS(ch, fch), dir_rev[door] );
   else sprintf( buf, "%s walks in from %s dragging %s",
                 PERS(ch, fch), dir_rev[door],
                 STR(ch->hitched_to,short_descr) );
    }

        buf[0] = UPPER(buf[0]);
        to_actor( buf, fch );
        if ( buf[0] != '\0' )
        {
            if ( ch->in_scene->exit[rev_dir[door]]
              && !MTD(ch->in_scene->exit[rev_dir[door]]->keyword) )
            {
                to_actor( " through the ", fch );
                if ( IS_SET(ch->in_scene->exit[rev_dir[door]]->exit_flags,EXIT_ISDOOR)
                 && !IS_SET(ch->in_scene->exit[rev_dir[door]]->exit_flags,EXIT_CLOSED) )
                to_actor( "open ", ch );               
                to_actor( cut_to( ch->in_scene->exit[rev_dir[door]]->keyword ), fch );
            }
            to_actor( ch->position == POS_FIGHTING ? "!\n\r" : ".\n\r", fch );
        }

        sprintf( buf, "%d", rev_dir[door] );
        script_update( fch, TYPE_ACTOR, TRIG_ENTER, ch, NULL, buf, NULL );
    }
}



bool lose_movement( PLAYER *ch, SCENE *in_scene,
                    SCENE *to_scene )
{
    int move;

	move = movement_loss[UMIN(MOVE_MAX-1, in_scene->move)]
         + movement_loss[UMIN(MOVE_MAX-1, to_scene->move)]
         ;

    if ( NPC(ch) && IS_SET(ch->flag, ACTOR_MOUNT) ) move/=3;

    if ( move <= 0 ) move = 1;

    if ( ch->riding )
	{
        if ( ch->riding->hitched_to != NULL )
        move += get_prop_weight( ch->riding->hitched_to ) / 4;

        if ( ch->riding->move < move )
        {
            to_actor( "Your mount is too exhausted.\n\r", ch );
            cmd_emote( ch->riding, "pants and gasps from exhaustion." );
            return FALSE;
        }
	}

    if ( ch->hitched_to != NULL )
    move += get_prop_weight( ch->hitched_to ) / 4;

    if ( ch->riding )   move -= get_curr_con(ch->riding)/2;

    if ( ch->move < move )
	{
	    to_actor( "You are too exhausted.\n\r", ch );
        return FALSE;
	}

    ch->move -= move;
    return TRUE;
}



bool check_move( PLAYER *ch, int door, int depth, SCENE *in_scene,
                 SCENE *to_scene, PROP **prop )
{
    bool found, fRiding = ch->riding != NULL;
    int sect;
    EXIT *pexit;
    PROP *fprop = NULL;

    // Dying players sometimes are dying before their script is done being, prior to dispose
    if ( in_scene == NULL || to_scene == NULL ) return FALSE;

   /*
    * NO_ACTORS and STAY_ZONE
    */
    if ( !ch->instances && NPC(ch) && ( IS_SET(to_scene->scene_flags,SCENE_NO_ACTOR)
                                     || (to_scene->zone != in_scene->zone && IS_SET(ch->flag,ACTOR_STAY_ZONE) )))
    {
        to_actor( "You are an NPC, and restricted from going in this direction due to flag NO_ACTOR or flag STAY_ZONE", ch );
        if ( !ch->master )
        return FALSE;
    }

    /*
     * Dragging -- rider.
     */
    if ( ch->hitched_to != NULL
      && !IS_SET(ch->hitched_to->wear_flags, ITEM_TAKE) )
    {
        act( "It is impossible to budge $p.", ch, ch->hitched_to, NULL, TO_ACTOR );
        act( "$n tries to pull $p $T away and fails.", ch, ch->hitched_to, dir_name[door], TO_SCENE );
        return FALSE;
    }

    if ( ch->hitched_to != NULL
      && get_prop_weight( ch->hitched_to ) > can_carry_w( ch ) * 2 )
    {
        act( "It is impossible to budge $p.", ch, ch->hitched_to, NULL, TO_ACTOR );
        act( "$n tries to pull $p $T away and fails.", ch, ch->hitched_to, dir_name[door], TO_SCENE );
        return FALSE;
    }

    /*
     * Holding.
     */
    if ( IS_AFFECTED(ch, BONUS_HOLD) && depth == 0 )
    {
        to_actor( "It is impossible to move.\n\r", ch );
        return FALSE;
    }

    if ( (fRiding && IS_AFFECTED(ch->riding, BONUS_HOLD)) && depth == 0  )
    {
        to_actor( "Your ride aren't able move.\n\r", ch );
        return FALSE;
    }

    in_scene = ch->in_scene;
    if ( ( pexit   = in_scene->exit[door] ) == NULL
      || ( to_scene = pexit->to_scene      ) == NULL )
    {
    to_actor( "You cannot go that way.\n\r", ch );
    return FALSE;
    }

    if ( IS_SET(pexit->exit_flags, EXIT_CLOSED)
      && !IS_AFFECTED(ch, BONUS_PASS_DOOR) )
    {
        if ( !IS_SET(pexit->exit_flags, EXIT_SECRET) || IS_IMMORTAL(ch) )
        {
            if ( scene_is_dark( in_scene ) )
            {
                act( "In the darkness, $n stumbles into a closed $T.",ch,NULL,pexit->keyword,TO_SCENE);
                act( "In the darkness, you bump into a closed $T.",ch,NULL,pexit->keyword,TO_ACTOR );
            }
            else
            act( "The $T is closed.", ch, NULL, pexit->keyword, TO_ACTOR );
        }
        else to_actor( "You cannot go that way.\n\r", ch );

        if ( !IS_IMMORTAL(ch) ) 
        return FALSE;
    }

    if ( IS_SET(pexit->exit_flags, EXIT_WINDOW)
      && pexit->key == -1 )
    {
        to_actor( "You cannot go that way.\n\r", ch );
        return FALSE;
    }

    if ( IS_AFFECTED(ch, BONUS_CHARM)
      && ch->master != NULL
      && in_scene == ch->master->in_scene )
    {
    to_actor( "And leave your beloved master?\n\r", ch );
    return FALSE;
    }

    if ( fRiding && ch->riding->position < POS_STANDING )
    {
        to_actor( "Your ride doesn't want to move right now.\n\r", ch );
        return FALSE;
    }

    if ( scene_is_private( to_scene ) )
    {
    to_actor( "It is impossible to enter that scene right now.\n\r", ch );
    return FALSE;
    }

    sect = to_scene->move;

    /*
     * Look for a movement device (not really a vehicle).
     */
    found = FALSE;
    for ( fprop = ch->carrying; fprop != NULL;  fprop = fprop->next_content )
    {
    if ( fprop->item_type == ITEM_VEHICLE )
    {
        int t;

        for ( t = 0; t < 4; t++ )
        {
            if (fprop->value[t] == sect)
            {
                found = TRUE;
                break;
            }
        }
        if ( found ) break;
    }
    }
    
    if ( NPC(ch) && IS_SET(ch->flag, ACTOR_PET) ) found = FALSE;

    if ( sect == MOVE_INSIDE
      || sect == MOVE_CITY
      || sect == MOVE_FIELD
      || sect == MOVE_FOREST
      || sect == MOVE_HILLS )
    {
    fprop = NULL;
    found = TRUE;
    }
    else
    if ( fprop == NULL )
    {
    if ( sect == MOVE_WATER_SWIM && !IS_AFFECTED( ch, BONUS_FLYING ) )
    {
        fprop = NULL;
        found = TRUE;
        if ( ch->rider != NULL || ch->riding != NULL )
        {
            to_actor( "It is impossible to swim while riding.\n\r", ch );
            return FALSE;
        }

        if ( !skill_check( ch, skill_lookup("swimming"), 2 )
          && !IS_AFFECTED( ch, BONUS_BREATHING ) )
        {
            damage( ch, ch, number_fuzzy( 5 ) );
            to_actor( "You gurgle as water laps into your face.\n\r", ch );
            act( "$n gurgles as $e takes a lung full of water.\n\r", ch, NULL, NULL, TO_SCENE );
            return FALSE;
        }
    }
    else
    if ( sect == MOVE_CLIMB && !IS_AFFECTED( ch, BONUS_FLYING ) )
    {
        int multiplier;

        fprop = NULL;
        found = TRUE;
        if ( fRiding )
        {
            to_actor( "It is impossible to climb while riding.\n\r", ch );
            return FALSE;
        }

        if ( hand_empty( ch ) == WEAR_NONE )
        {
            to_actor( "You need atleast one free hand to climb.\n\r", ch );
            return FALSE;
        }

        if ( get_eq_char( ch, WEAR_HOLD_1 ) != NULL
          || get_eq_char( ch, WEAR_HOLD_2 ) != NULL )
            multiplier = 50;
       else multiplier = 0;


        if ( !skill_check( ch, skill_lookup("climbing"), multiplier ) )
        {
            if ( !skill_check( ch, skill_lookup("climbing"), multiplier ) )
            {
            SET_BIT( ch->bonuses, BONUS_FALLING );
            to_actor( "You lose your grip.\n\r", ch );
            }
            else
            {
            to_actor( "Your hold slips and you nearly fall!\n\r", ch );
            act( "$n's hold slips!", ch, NULL, NULL, TO_SCENE );
            }
            return FALSE;
        }
    }
    }
            
    if ( !found        /* no vehicle */
    && ((sect == MOVE_MOUNTAIN && !IS_AFFECTED(ch, BONUS_FLYING) )
     || (sect == MOVE_AIR       && !IS_AFFECTED(ch, BONUS_FLYING) )
     || (sect == MOVE_AIR       && fRiding
                                && !IS_AFFECTED(ch->riding, BONUS_FLYING) )
     || (sect == MOVE_UNDERWATER   && !IS_AFFECTED(ch, BONUS_BREATHING) )
     || (sect == MOVE_WATER_NOSWIM && !IS_AFFECTED(ch, BONUS_FLYING) ))     )
    {
        char buf[MAX_STRING_LENGTH];
            
        switch( sect )
        {
               default: sprintf( buf, "You need another form of transportation to go there\n\r" ); break;
  case MOVE_UNDERWATER: sprintf( buf, "It is impossible to hold your breath for long enough.\n\r" ); break;
    case MOVE_MOUNTAIN: sprintf( buf, "Its too steep for you to climb unaided.\n\r" ); break;
case MOVE_WATER_NOSWIM: sprintf( buf, "You need a boat to go there.\n\r" ); break;
         case MOVE_AIR: sprintf( buf, "It is impossible to fly!\n\r" ); break;
        }
            
        to_actor( buf, ch );
        return FALSE;
    }
      /* End cant move checks */

    *prop = fprop;
    return TRUE;
}



void move_char( PLAYER *ch, int door )
{
    static int depth = 0;
    SCENE *in_scene;
    SCENE *to_scene;
    PROP *prop;
    char buf[MAX_STRING_LENGTH];
    int sect;
    bool fWindow;

    if ( door < 0 || door >= MAX_DIR )
    {
	wtf_logf( "Cmd_move: bad door %d.", door );
	return;
    }

    /*
     * Fleeing
     */
    if ( ch->fighting != NULL )
    {
        flee( ch, ch->fighting, door );
        if ( ch->fighting != NULL )
        return;
    }


    /*
     * Blind staggering -- rider or mount.
     * Also, drunkeness.
     */
    if ( IS_AFFECTED(ch, BONUS_BLIND)
      || ( !NPC(ch) && PC(ch,condition)[COND_DRUNK] > 30 )
      || (ch->riding
       && IS_AFFECTED(ch->riding, BONUS_BLIND)
       && skill_check(ch, skill_lookup("riding"), 50)) )
    {
        if ( door != DIR_UP && door != DIR_DOWN )
        door = depth != 0 ? door : number_fuzzy( door );

        if ( door == DIR_UP || door == DIR_DOWN )
        door = number_range( 0, DIR_NW );
    }

    in_scene = ch->in_scene;
    to_scene = in_scene && in_scene->exit[door] ? in_scene->exit[door]->to_scene : NULL;

    if ( !NPC(ch) && in_scene->exit[door] == NULL ) { to_actor( "There is no way to travel in that direction from here.\n\r", ch );
       return; }

    if ( !check_move(ch, door, depth, in_scene, to_scene, &prop) )
    return;

    sect = to_scene->move;

    fWindow = in_scene->exit[door] && IS_SET(in_scene->exit[door]->exit_flags, EXIT_WINDOW) ? 1 : 0;

    /*
     * Exhaustion.
     */
    if ( !lose_movement(ch, in_scene, to_scene) )
    return;

    if ( in_scene->exit[door] 
      && IS_SET(in_scene->exit[door]->exit_flags, EXIT_NOMOVE) ) {
      to_actor( "You cannot go that way.\n\r", ch );
      return;
    }

    WAIT_STATE( ch, 2 );

    sprintf( buf, "%d", door );
    if (script_update( ch->in_scene, TYPE_SCENE, TRIG_MOVES, ch, NULL, buf, NULL ) == 2)
    return;

    clrscr(ch);
    leave_strings( ch, prop, sect, door, fWindow );

    actor_from_scene( ch );
    actor_to_scene( ch, to_scene );

/*    client_send_map( ch ); */

    arrive_strings( ch, prop, sect, door, fWindow );
    hide_check( ch, to_scene );

    if (script_update( to_scene, TYPE_SCENE, TRIG_ENTER, ch, NULL, buf, NULL ) == 2)
    return;

    if ( ch->hitched_to != NULL )
    {
        prop_from_scene( ch->hitched_to );
        prop_to_scene( ch->hitched_to, to_scene );
    }

    /*
     * Guard against rider-mount inconsistencies.
     */
    if ( ch->riding != NULL
      && ch->in_scene != ch->riding->in_scene
      && depth == 0 )
    {
        depth++;
        move_char( ch->riding, door );
        if ( ch->riding->in_scene != ch->in_scene )
        {
        ch->riding->rider = NULL;
        ch->riding = NULL;
        to_actor( "You are forced to leave your mount behind.\n\r", ch );
        }
        depth--;
    }
    else
    if ( ch->rider != NULL
      && ch->in_scene != ch->rider->in_scene
      && depth == 0 )
    {
        depth++;
        move_char( ch->rider, door );
        if ( ch->rider->in_scene != ch->in_scene )
        {
        ch->rider->riding = NULL;
        ch->rider = NULL;
        to_actor( "You are forced to leave your rider behind.\n\r", ch );
        }
        depth--;
    }

    if ( !NPC(ch) && IS_SET(ch->flag2, PLR_CLRSCR) ) {
      to_actor( "+--------------------------------------------------------------------+\n\r", ch );
      to_actor( PC(ch,say_last), ch );
      to_actor( PC(ch,tell_last), ch );
      to_actor( PC(ch,chat_last), ch );
      if ( IS_IMMORTAL(ch) ) to_actor( PC(ch,immt_last), ch );
      to_actor( "+--------------------------------------------------------------------+\n\r", ch );
    }
    cmd_look( ch, "auto" );

    if ( !MTD(ch->tracking) )
    cmd_track( ch, "continue" );

    {
        PLAYER *fch;
        PLAYER *fch_next;

        for ( fch = in_scene->people; fch != NULL; fch = fch_next )
        {
            fch_next = fch->next_in_scene;

            if ( fch->master == ch && fch->riding == NULL && fch->position == POS_STANDING )
            {
            act( "You follow $N $t.", fch, dir_name[door], ch, TO_ACTOR );
            move_char( fch, door );
            }
            else
            if ( prop && fch->furniture == prop && fch->in_scene != prop->in_scene )
            {
            act( "$p moves $t.", fch, prop, dir_name[door], TO_ACTOR );
            move_char( fch, door );
            }
        }
    }

/*    if ( IS_SET(ch->in_scene->scene_flags, SCENE_GENERATED) )*/
/*    gen_scene( ch->in_scene );  */

    if ( ch->position == POS_FIGHTING ) ch->position = POS_STANDING;
    return;
}





void cmd_north ( PLAYER *ch, char *argument ) { move_char( ch, DIR_NORTH ); return; }
void cmd_east  ( PLAYER *ch, char *argument ) { move_char( ch, DIR_EAST );  return; }
void cmd_south ( PLAYER *ch, char *argument ) { move_char( ch, DIR_SOUTH ); return; }
void cmd_west  ( PLAYER *ch, char *argument ) { move_char( ch, DIR_WEST );  return; }
void cmd_up    ( PLAYER *ch, char *argument ) { move_char( ch, DIR_UP );    return; }
void cmd_down  ( PLAYER *ch, char *argument ) { move_char( ch, DIR_DOWN );  return; }
void cmd_nw    ( PLAYER *ch, char *argument ) { move_char( ch, DIR_NW );    return; }
void cmd_ne    ( PLAYER *ch, char *argument ) { move_char( ch, DIR_NE );    return; }
void cmd_sw    ( PLAYER *ch, char *argument ) { move_char( ch, DIR_SW );    return; }
void cmd_se    ( PLAYER *ch, char *argument ) { move_char( ch, DIR_SE );    return; }


/*
 * Syntax:  drag [prop] [direction]
 */
void cmd_drag( PLAYER *ch, char *argument )
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    PLAYER *actor;
    PROP *prop;
    int door;
    
    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( (prop = get_prop_list(ch, arg1, ch->in_scene->contents)) == NULL )
    {
        to_actor( "Drag what where?\n\r", ch );
        return;
    }
    
    if ( ( door = get_dir( arg2 ) ) == MAX_DIR )
    {   
        to_actor( "Drag what where?\n\r", ch );
        return;
    }

    for ( actor = ch->in_scene->people; actor != NULL; actor = actor->next_in_scene )
    {
        if ( actor->hitched_to == prop )
        {
            act( "$p is already hitched to $N.", ch, prop, actor, TO_ACTOR );
            return;
        }
    }
    
    ch->hitched_to = prop;
    move_char( ch, door );
    ch->hitched_to = NULL;

    return;
}



/*
 * Syntax:  hitch [person] [prop]
 */
void cmd_hitch( PLAYER *ch, char *argument )
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    PROP *prop;
    PLAYER *actor;
    PLAYER *vict;
    
    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || (vict = get_actor_scene(ch, arg1)) == NULL )
    {
        to_actor( "Hitch whom to what?\n\r", ch );
        return;
    }

    if ( arg2[0] == '\0' || (prop = get_prop_list(ch, arg2, ch->in_scene->contents)) == NULL )
    {
        to_actor( "Hitch whom to what?\n\r", ch );
        return;
    }

    if ( !IS_SET(prop->extra_flags, ITEM_HITCH) )
    {
        to_actor( "It is impossible to hitch anything to that.\n\r", ch );
        return;
    }

    for ( actor = ch->in_scene->people; actor != NULL; actor = actor->next_in_scene )
    {
        if ( actor->hitched_to == prop )
        {
            act( "$p is already hitched to $N.", ch, prop, actor, TO_ACTOR );
            return;
        }
    }

    if ( vict->hitched_to != NULL )
    cmd_unhitch( ch, arg2 );

    vict->hitched_to = prop;
    act( "You hitch $N to $p.", ch, prop, vict, TO_ACTOR );
    act( "$n hitches $N to $p.", ch, prop, vict, TO_NOTVICT );
    act( "$n hitches you to $p.", ch, prop, vict, TO_VICT );
    return;
}



/*
 * Syntax:  unhitch [person]
 */
void cmd_unhitch( PLAYER *ch, char *argument )
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    PROP *prop;
    PLAYER *vict;
    
    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || (vict = get_actor_scene(ch, arg1)) == NULL )
    {
        to_actor( "Unhitch whom?\n\r", ch );
        return;
    }

    if ( vict->hitched_to == NULL )
    {
        to_actor( "They aren't hitched to anything.", ch );
        return;
    }

    prop = vict->hitched_to;
    vict->hitched_to = NULL;
    act( "You unhitch $N from $p.", ch, prop, vict, TO_ACTOR );
    act( "$n unhitches $N from $p.", ch, prop, vict, TO_NOTVICT );
    act( "$n unhitches you from $p.", ch, prop, vict, TO_VICT );
    return;
}
 


/*
 * Syntax:  enter [prop]
 */
void cmd_enter( PLAYER *ch, char *argument )
{
    char arg[MAX_STRING_LENGTH];
    PLAYER *actor;
    PROP *prop;
    SCENE *dest;
    int ddbkey;
    
    one_argument( argument, arg );
    
    if ( ( prop = get_prop_here( ch, arg ) ) == NULL )
    {
        to_actor( "There is nothing like that to enter.\n\r", ch );
        return;
    }
    
    if ( ( prop->item_type != ITEM_FURNITURE )
      || !IS_SET(prop->value[1], FURN_EXIT) )
    {
        to_actor( "Now how do you expect to do that.\n\r", ch );
        return;
    }
    
    ddbkey = IS_SET(prop->value[1],FURN_HOME) ? 
                (NPC(ch) ? prop->value[3] : PC(ch,home)) : prop->value[3];

    if ( ( dest = get_scene( ddbkey ) ) == NULL )
    {
        to_actor( "It is impossible to go that way.\n\r", ch );
        return;
    }

    if ( scene_is_private( dest ) )
    {
        to_actor( "It is impossible to enter that scene right now.\n\r", ch );
        return;
    }
      
    if ( IS_SET(prop->value[1], EXIT_CLOSED) )
    {
        to_actor( "It's closed.\n\r", ch );
        return;
    }

    if ( IS_SET(prop->value[1], FURN_HOME) ) {
    act( "You return home.", ch, prop, NULL, TO_ACTOR );
    act( "$n returns home.", ch, prop, NULL, TO_SCENE ); 
    } else
    {    
    act( "$n enters $p.", ch, prop, NULL, TO_SCENE );

    if ( STR(prop,action_descr) != NULL )
    to_actor( STR(prop,action_descr), ch );
    else
    act( "You enter $p.", ch, prop, NULL, TO_ACTOR );

    }
    script_update( prop, TYPE_PROP, TRIG_ENTER, ch, NULL, NULL, NULL );

    actor_from_scene( ch );
    actor_to_scene( ch, dest );

    cmd_look( ch, "auto" );

    if ( IS_SET(ch->in_scene->scene_flags, SCENE_WAGON) 
      || IS_SET(prop->value[1], FURN_NOSHOW) )
    act( "$n arrives.", ch, NULL, NULL, TO_SCENE );
    else
    act( "$n arrives from $p.", ch, prop, NULL, TO_SCENE );


    if ( ch->rider != NULL )
    {
        actor_from_scene( ch->rider );
        actor_to_scene( ch->rider, dest );
        cmd_look( ch->rider, "auto" );
    }

    if ( ch->riding != NULL )
    {
        actor_from_scene( ch->riding );
        actor_to_scene( ch->riding, dest );
        cmd_look( ch->riding, "auto" );
    }

    for ( actor = ch->in_scene->people;
          actor != NULL;
          actor = actor->next_in_scene )
    {
    if (script_update( actor, TYPE_ACTOR, TRIG_ENTER, ch, NULL, "99", NULL ))
    break;
    }

    return;
}



void cmd_leave( PLAYER *ch, char *argument )
{
    SCENE *pScene;
    PROP_TEMPLATE *pPropIndex;
    PROP *pProp;

    if ( (pScene = ch->in_scene) == NULL )
    {
        to_actor( "There is nothing to get out of.\n\r", ch );
        return;
    }

    if ( IS_SET(pScene->scene_flags, SCENE_WAGON)
      && (pPropIndex = get_prop_template( pScene->wagon )) != NULL )
    {
        for ( pProp = prop_list; pProp != NULL; pProp = pProp->next )
        {
            if ( pProp->pIndexData == pPropIndex
              && pProp->in_scene != NULL )
             break;
        }

        if ( pProp == NULL )
        {
            wtf_logf( "Cmd_leave: pProp == NULL.", 0 );
            to_actor( "There is nothing to leave to.\n\r", ch );
            return;
        }

        act( "$n leaves.", ch, NULL, NULL, TO_SCENE );
        actor_from_scene( ch );
        actor_to_scene( ch, pProp->in_scene );
        act( "$n emerge$v from $p.", ch, pProp, NULL, TO_ALL );
        cmd_look( ch, "auto" );
        return;
    }

    to_actor( "There is nothing to leave to.\n\r", ch );
    return;
}



/*
 * Syntax:  stand
 */
void cmd_stand( PLAYER *ch, char *argument )
{
    if ( ch->riding != NULL )    { cmd_dismount( ch, "" ); return; }

    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, BONUS_SLEEP) )
	    { to_actor( "It is impossible to wake up!\n\r", ch ); return; }

	to_actor( "You wake and stand up.\n\r", ch );
	act( "$n wakes and stands up.", ch, NULL, NULL, TO_SCENE );
	ch->position = POS_STANDING;
    set_furn( ch, NULL );
	break;

    case POS_SITTING:
    case POS_RESTING:
	to_actor( "You stand up.\n\r", ch );
	act( "$n stands up.", ch, NULL, NULL, TO_SCENE );
	ch->position = POS_STANDING;
    set_furn( ch, NULL );
	break;

    case POS_STANDING:
	to_actor( "You are already standing.\n\r", ch );
    set_furn( ch, NULL );        /* just to be sure */
	break;

    case POS_FIGHTING:
    to_actor( "You are fighting!\n\r", ch );
	break;
    }

    return;
}



/*
 * Syntax:  rest
 */
void cmd_rest( PLAYER *ch, char *argument )
{

    if ( ch->riding != NULL )
    {
    cmd_rest( ch->riding, "" );
    cmd_dismount( ch, "" );
    }

    switch ( ch->position )
    {
    case POS_SLEEPING:
	to_actor( "You are already sleeping.\n\r", ch );
	break;

    case POS_RESTING:
	to_actor( "You are already resting.\n\r", ch );
	break;

    case POS_SITTING:
    to_actor( "You lean back and rest.\n\r", ch );
    act( "$n leans back and rests.", ch, NULL, NULL, TO_SCENE );
	ch->position = POS_RESTING;
	break;

    case POS_STANDING:
    to_actor( "You sit down and rest.\n\r", ch );
    if ( NPC(ch) && IS_SET(ch->flag, ACTOR_MOUNT) )
    act( "$n curls up on the ground.", ch, NULL, NULL, TO_SCENE );
    else
    act( "$n sits down and rests.", ch, NULL, NULL, TO_SCENE );
	ch->position = POS_RESTING;
	break;

    case POS_FIGHTING:
	to_actor( "You are already fighting!\n\r", ch );
	break;
    }

    return;
}



/*
 * Syntax:  sit
 *          sit [prop]
 */
void cmd_sit( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PROP *prop = NULL;

    if ( ch->riding != NULL )
    cmd_dismount( ch, "" );

    one_argument( argument, arg );
    if ( !MTD( arg ) )
    {
        if ( (prop = get_furn_here(ch, arg)) == NULL )
        {
            to_actor( "Sit on what?\n\r", ch );
            return;
        }

        if ( ch->furniture != NULL )
        {
            to_actor( "You are already using a piece of furniture.\n\r", ch );
            return;
        }

        if ( ch->position <= POS_SITTING )
        {
            to_actor( "Get up first.\n\r", ch );
            return;
        }

        if ( !VAL_SET(prop, 1, FURN_SIT) )
        {
            to_actor( "It is impossible to sit on that.\n\r", ch );
            return;
        }

        if ( OCCUPADO(prop) )
        {
            to_actor( "There is someone else already using it.\n\r", ch );
            return;
        }

        set_furn( ch, prop );
    }

    switch ( ch->position )
    {
    case POS_SLEEPING:
    to_actor( "You wake and sit up.\n\r", ch );
    act( "$n wakes and sits up.", ch, NULL, NULL, TO_SCENE );
    ch->position = POS_SITTING;
	break;

    case POS_RESTING:
    to_actor( "You sit up.\n\r", ch );
    act( "$n sits up.", ch, NULL, NULL, TO_SCENE );
    ch->position = POS_SITTING;
	break;

    case POS_SITTING:
    to_actor( "You are already sitting.\n\r", ch );
    break;

    case POS_STANDING:
    if ( !ch->furniture )
    to_actor( "You sit down.\n\r", ch );
    else
    act( "You sit down on $p.", ch, ch->furniture, NULL, TO_ACTOR );

    if ( !ch->furniture )
    act( "$n sits down.", ch, NULL, NULL, TO_SCENE );
    else
    act( "$n sits down on $p.", ch, ch->furniture, NULL, TO_SCENE );
    ch->position = POS_SITTING;
	break;

    case POS_FIGHTING:
    to_actor( "You are fighting!\n\r", ch );
	break;
    }

    return;
}



/*
 * Syntax:  sleep
 *          sleep [prop]
 */
void cmd_sleep( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PROP *prop;

    if ( ch->riding != NULL )
    {
        to_actor( "You must dismount first.\n\r", ch );
        return;
    }

    one_argument( argument, arg );
    if ( !MTD( arg ) )
    {
        if ( (prop = get_prop_here(ch, arg)) == NULL )
        {
            to_actor( "Sleep on what?\n\r", ch );
            return;
        }

        if ( ch->furniture )
        {
            to_actor( "You are already on something else.\n\r", ch );
            return;
        }

        if ( VAL_SET(prop, 1, FURN_SLEEP) )
        {
            if ( OCCUPADO(prop) )
            {
                to_actor( "There is no more scene on it.\n\r", ch );
                return;
            }
            else
            {
                act( "$n lies down on $p.", ch, prop, NULL, TO_SCENE );
                act( "You lie down on $p.", ch, prop, NULL, TO_ACTOR );
                set_furn( ch, prop );
            }
        }
        else
        {
            to_actor( "It is impossible to sleep on that.\n\r", ch );
            return;
        }
    }
    else
    {
        prop = ch->furniture;
        if ( prop )
        {
            if ( prop->item_type == ITEM_FURNITURE
              && VAL_SET(prop, 1, FURN_SLEEP) )
            {
                act( "$n lies back on $p.", ch, prop, NULL, TO_SCENE );
                act( "You lie back on $p.", ch, prop, NULL, TO_ACTOR );
            }
            else
            {
            to_actor( "You roll onto the floor.\n\r", ch );
            set_furn( ch, NULL );
            }
        }
    }


    switch ( ch->position )
    {
    case POS_SLEEPING:
	to_actor( "You are already sleeping.\n\r", ch );
	break;

    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING:

    if ( !ch->furniture )
    to_actor( "You lie down and fall asleep.\n\r", ch );
    else
    to_actor( "You go to sleep.\n\r", ch );

    if ( !ch->furniture )
    act( "$n lies down and falls asleep.", ch, NULL, NULL, TO_SCENE );
    else
    act( "$n goes to sleep.", ch, NULL, NULL, TO_SCENE );

	ch->position = POS_SLEEPING;
	break;

    case POS_FIGHTING:
        to_actor( "You are fighting!\n\r", ch );
	break;
    }

    return;
}



/*
 * Syntax:  wake
 *          wake [person]
 */
void cmd_wake( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PLAYER *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{ cmd_stand( ch, argument ); return; }

    if ( !IS_AWAKE(ch) )
	{ to_actor( "You are asleep yourself!\n\r",       ch ); return; }

    if ( ( victim = get_actor_scene( ch, arg ) ) == NULL )
	{ to_actor( "They aren't here.\n\r",              ch ); return; }

    if ( IS_AWAKE(victim) )
	{ act( "$N is already awake.", ch, NULL, victim, TO_ACTOR ); return; }

    if ( IS_AFFECTED(victim, BONUS_SLEEP) )
	{ act( "It is impossible to wake $M!",   ch, NULL, victim, TO_ACTOR );  return; }

    act( "You wake $M.", ch, NULL, victim, TO_ACTOR );
    act( "$n wakes you.", ch, NULL, victim, TO_VICT );
    victim->position = POS_RESTING;
    return;
}



/*
 * Syntax:  sneak
 */
void cmd_sneak( PLAYER *ch, char *argument )
{
    if ( IS_AFFECTED(ch, BONUS_SNEAK) )
    {
    REMOVE_BIT(ch->bonuses, BONUS_SNEAK);
    to_actor( "You stop trying to move silently.\n\r", ch );
    return;
    }

    if ( ch->riding != NULL )
    {
        to_actor( "Dismount first.\n\r", ch );
        return;
    }
    to_actor( "You try to move silently.\n\r", ch );
    SET_BIT(ch->bonuses, BONUS_SNEAK);
    return;
}



/*
 * Syntax:  hide
 */
void cmd_hide( PLAYER *ch, char *argument )
{
    if ( IS_AFFECTED(ch, BONUS_HIDE) )
    {
    REMOVE_BIT(ch->bonuses, BONUS_HIDE);
    to_actor( "You stop trying to hide your presence.\n\r", ch );
    act( "$n stops trying to hide $s presence.", ch, NULL, NULL, TO_SCENE );
    return;
    }

    if ( ch->riding != NULL )
    {
        to_actor( "Dismount first.\n\r", ch );
        return;
    }

    to_actor( "You attempt to hide your presence.\n\r", ch );
    SET_BIT(ch->bonuses, BONUS_HIDE);
    hide_check( ch, ch->in_scene );
    return;
}




void cmd_home( PLAYER *ch, char *argument ) {
    SCENE *pScene;
    char buf[MAX_INPUT_LENGTH];

    if ( NPC(ch) ) return;

    argument = one_argument ( argument, buf );
    if (     !str_cmp( buf, "set" ) ) {
/*            if ( IS_SET( ch->in_scene, SCENE_SAVING ) ) { */
                 PC(ch,home) = ch->in_scene->dbkey;
                 to_actor( "Home set.\n\r", ch );
          }

    if (     !str_cmp( buf, "reset" ) ) {
/*            if ( IS_SET( ch->in_scene, SCENE_SAVING ) ) { */
                 PC(ch,home) = SCENE_VNUM_DEFAULT_HOME;
                 to_actor( "Home reset.\n\r", ch );
          }

    if ( ch->fighting != NULL || ch->position == POS_FIGHTING ) {
           to_actor( "You are in the middle of combat!\n\r", ch  );
           return; 
         }

    pScene = get_scene( PC(ch,home) );

    if ( pScene == NULL ) {
          PC(ch,home) = SCENE_VNUM_DEFAULT_HOME;
          pScene = get_scene( PC(ch,home) );
          to_actor( "Home recall location set.\n\r", ch );
          return;
    }


    /* 
     * Move all of your hirelings, mounts and conjured followers.
     */

    { PLAYER *pch;
      for ( pch = actor_list; pch!=NULL; pch=pch->next )
      if ( NPC(pch) && pch->master == ch ) {
       actor_from_scene( pch );
       actor_to_scene( pch, pScene );
       if ( pch->riding ) {
       actor_from_scene( pch->riding );
       actor_to_scene( pch->riding, pScene );
       }
      }
     if ( (pch=ch->riding) ) {
      actor_from_scene( pch );
      actor_to_scene( pch, pScene );
     }
    }

    display_interp( ch, "^B" );
    actor_from_scene( ch );
    to_actor( "You return home.\n\r", ch   );
    actor_to_scene( ch, pScene );
    display_interp( ch, "^N" );
    cmd_look( ch, "auto" );

    return;
}   
