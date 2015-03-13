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
#include <ctype.h>
#include <string.h>
#include <time.h>

#include <zlib.h>

#include "nimud.h"
#include "skills.h"
#include "script.h"
#include "fight.h"
#include "defaults.h"
#include "net.h"



#define FGT_HIT        1
#define FGT_WOUNDED    2
#define FGT_STUNNED    4
#define FGT_CMP_HIT    8
#define FGT_DODGE     16
#define FGT_PARRY     32
#define FGT_PRIMARY   64





struct fight_mode
{
    const char * name;
    int tohit;
    int dodge;
    int todam;
};

const struct fight_mode fight_mode_table[] =
{
    { "elusive",       -15, 15, -5 },
    { "cautious",      -10, 10,  0 },
    { "normal",          0,  0,  0 },
    { "bold",           10,-10,  0 },
    { "berserk",        15,-15,  5 },
    { "",                0,  0,  0 }
};

char * const damage_strings [] =
{
     " lightly."      ,  /* 0 - 10 */
     " with finesse!" ,      /* 20 */
     " moderately."   ,     
     "."              ,
     "!"              ,      /* 50 */
     " hard."         ,
     " heavily!"      ,
     "!"              ,
     " critically!"   ,
     " mortally!"     ,
     " with a pulverizing blow." ,  /* 100 */
     " beyond recognition."      ,
     " with colossal might!"     ,
     ", tearing him asunder!"       /* 130 */
};

const int combat_int[26] =
{ 0,                         /* 0 */
  0,
  0,
/*
 * Creatures with int > 2 know when they've been injured
 */
  FGT_HIT,
  FGT_HIT,
/*
 * Creatures with int > 4 know when they've been seriously injured
 */
  FGT_HIT|FGT_WOUNDED,   /* 5 */
  FGT_HIT|FGT_WOUNDED,
  FGT_HIT|FGT_WOUNDED,
  FGT_HIT|FGT_WOUNDED,
/*
 * Creatures with int > 8 know when their opponent is unable to
 * attack them
 */
  FGT_HIT|FGT_WOUNDED|FGT_STUNNED,
  FGT_HIT|FGT_WOUNDED|FGT_STUNNED,   /* 10 */
  FGT_HIT|FGT_WOUNDED|FGT_STUNNED,
  FGT_HIT|FGT_WOUNDED|FGT_STUNNED,
/*
 * Creatures with int > 12 know when to change combat tactic due to damage
 */
  FGT_HIT|FGT_WOUNDED|FGT_STUNNED|FGT_CMP_HIT,
  FGT_HIT|FGT_WOUNDED|FGT_STUNNED|FGT_CMP_HIT,
  FGT_HIT|FGT_WOUNDED|FGT_STUNNED|FGT_CMP_HIT,   /* 15 */
  FGT_HIT|FGT_WOUNDED|FGT_STUNNED|FGT_CMP_HIT,
  FGT_HIT|FGT_WOUNDED|FGT_STUNNED|FGT_CMP_HIT,
  FGT_HIT|FGT_WOUNDED|FGT_STUNNED|FGT_CMP_HIT,
  FGT_HIT|FGT_WOUNDED|FGT_STUNNED|FGT_CMP_HIT,
  FGT_HIT|FGT_WOUNDED|FGT_STUNNED|FGT_CMP_HIT, /* 20 */
  FGT_HIT|FGT_WOUNDED|FGT_STUNNED|FGT_CMP_HIT,
  FGT_HIT|FGT_WOUNDED|FGT_STUNNED|FGT_CMP_HIT,
  FGT_HIT|FGT_WOUNDED|FGT_STUNNED|FGT_CMP_HIT,
  FGT_HIT|FGT_WOUNDED|FGT_STUNNED|FGT_CMP_HIT,
  FGT_HIT|FGT_WOUNDED|FGT_STUNNED|FGT_CMP_HIT /* 25 */
};

    

/*
 * Local functions.
 */

bool flee               args( ( PLAYER *ch ) );
void retreat            args( ( PLAYER *ch ) );
bool disarm             args( ( PLAYER *ch, PLAYER *victim ) );
bool bash               args( ( PLAYER *ch, PLAYER *victim, bool fArmed ) );
bool grapple            args( ( PLAYER *ch, PLAYER *victim ) );
bool kick               args( ( PLAYER *ch, PLAYER *victim, bool fArmed ) );
bool sweep              args( ( PLAYER *ch ) );
bool trip               args( ( PLAYER *ch, PLAYER *victim, bool fArmed ) );
int  number_fighting    args( ( PLAYER *ch ) );
void auto_fight		args( ( PLAYER *ch, PLAYER *victim) );
void auto_flee		args( ( PLAYER *ch, PLAYER *victim ) );
bool armed_attack	args( ( PLAYER *ch, PLAYER *victim) );
bool unarmed_attack	args( ( PLAYER *ch, PLAYER *victim) );


int fight_mode_lookup( const char *name )
{
    int sn;

    if ( name == NULL ) return -1;

    for (sn = 0; fight_mode_table[sn].name[0] != '\0'; sn++ )
    {
	if ( LOWER(name[0]) == LOWER(fight_mode_table[sn].name[0])
	&&   !str_prefix( name, fight_mode_table[sn].name ) )
	    return sn;
    }

    return -1;
}

void MSG_weapon_hit( PLAYER *ch, PROP *prop, PLAYER *vi, int dam )
{
    char buf[MAX_STRING_LENGTH];
    char w[MAX_STRING_LENGTH];
    char *i;

    if ( prop->item_type != ITEM_WEAPON )
    {
        bug( "MSG_weapon_hit: Supplied prop is not a weapon.", 0 );
        return;
    }

    sprintf( w, "%s", smash_article(STR(prop,short_descr)) );
    i = "$n successfully hit$v $N with $s $t.";

    if ( !MTP(STR(prop,action_descr)) ) {
        int lines=0;

        char *p;
        p = STR(prop,action_descr);
        while ( *p != '\0' ) {
            if (*p == '\n') lines++;
        }
        lines--;  /* most action descs have a trailing linebreak */
        p = STR(prop,action_descr);

        lines = number_range(0,lines);
        while ( lines-- >= 0 ) {
         p = one_line( p, buf );   
        }
        i = buf;
        
    } else {
    switch ( prop->value[3] )
    {
    default:
    if ( prop->value[3] > MAX_ATTACK )
    bug("MSG_weapon_hit: Invalid attack (dbkey %d).", prop->pIndexData->dbkey);
    if ( dam >= 100 ) i = "$4$B$n massacre$v $N with $s powerful attack!$R";
    else
    if ( dam < 5 )
    {
        switch( number_range(0, 9) )
        {
        case 0: i = "$n$y $t barely grazes $N.";                                         break;
        case 1: i = "$n$y badly placed hit merely scratches $N.";                        break;
        case 2: i = "$n attack$v $N with $s $t, but it does little damage.";             break;
        case 3: i = "$N $K clipped by $n$y $t.";                                         break;
        case 4: i = "$n hit$v $N lightly.";                                              break;
        case 5: i = "$n manage$v to hit $N, though the damage is slight.";               break;
        case 6: i = "The $t that $n $k wielding hardly hits $N at all.";                 break;
        case 7: i = "$N$Y $K distracted, but $n fail$v to hit $M with any real force.";  break;
        case 8: i = "$n pounce$v on $N$Y weak spot, but $e fail$v to do much damage.";   break;
        case 9: i = "$n only skim$v $n$y weapon across $N$Y body.";                      break;
        }
    }
    else
    if ( dam < 40 )
    {
        switch( number_range(0, 9) )
        {
        case 0: i = "$n hit$v $N with $s $t.";                                                break;
        case 1: i = "$N fail$V to escape $n$y $t.";                                           break;
        case 2: i = "$n hit$v $N.";                                                           break;
        case 3: i = "$N $K unable to dodge $n$y attack; $e hurt$v $M.";                       break;
        case 4: i = "Having surprised $N, $n bound$v forward and hit$v $M.";                  break;
        case 5: i = "$n lunge$v at $N with $s $t at the ready, hitting $M.";                  break;
        case 6: i = "$N $K hit by $n$y $t when $e attack$v.";                                 break;
        case 7: i = "$n hit$v $N.";                                                           break;
        case 8: i = "$N $K suddenly hit by $n$y $t.";                                         break;
        case 9: i = "With $s $t, $n succeed$v to harm $N.";                                   break;
        }
    }
    else
    {
        switch( number_range(0, 9) )
        {
        case 0: i = "$4$n$y $t shakes $N with a powerful hit.$R";                                 break;
        case 1: i = "$N contort$V in pain as $n$y $t connects with $M.";                      break;
        case 2: i = "$n deal$v a heavy blow to $N with $s $t.";                               break;
        case 3: i = "$N $K overcome with a pained look as $n hit$v $M.";                      break;
        case 4: i = "$N $K surprised by $n$y ferocious attack.";                              break;
        case 5: i = "$n lunge$v at $N, dealing a heavy blow with $s $t.";                     break;
        case 6: i = "Raising $s weapon, $n hit$v $N with the full force of $s attack.";       break;
        case 7: i = "$N double$V over with the strength of $n$y attack!";                     break;
        case 8: i = "$n quite spontaneously leap$v forward, hitting $N with $s $t!";          break;
        case 9: i = "With skill and finesse, $N offer$V $Mself to $n$y awaiting $t.";         break;
        }
    }
    break;


    case 0:  /* "hit" */
    if ( dam >= 100 ) i = "$n massacre$v $N with $s powerful hit!";
    else
    if ( dam < 5 )
    {
        switch( number_range(0, 9) )
        {
        case 0: i = "$n$y $t barely grazes $N.";                                         break;
        case 1: i = "$n$y badly placed hit merely scratches $N.";                        break;
        case 2: i = "$n attack$v $N with $s $t, but it does little damage.";             break;
        case 3: i = "$N $K clipped by $n$y $t.";                                         break;
        case 4: i = "$n hit$v $N lightly.";                                              break;
        case 5: i = "$n manage$v to hit $N, though the damage is slight.";               break;
        case 6: i = "The $t that $n $k wielding hardly hits $N at all.";                 break;
        case 7: i = "$N$Y $K distracted, but $n fail$v to hit $M with any real force.";  break;
        case 8: i = "$n pounce$v on $N$Y weak spot, but $e fail$v to do much damage.";   break;
        case 9: i = "$n only skim$v $n$y weapon across $N$Y body.";                      break;
        }
    }
    else
    if ( dam < 40 )
    {
        switch( number_range(0, 9) )
        {
        case 0: i = "$n hit$v $N with $s $t.";                                                break;
        case 1: i = "$N fail$V to escape $n$y $t.";                                           break;
        case 2: i = "$n hit$v $N.";                                                           break;
        case 3: i = "$N $K unable to dodge $n$y attack; $e hurt$v $M.";                       break;
        case 4: i = "Having surprised $N, $n bound$v forward and hit$v $M.";                  break;
        case 5: i = "$n lunge$v at $N with $s $t at the ready, hitting $M.";                  break;
        case 6: i = "$N $K hit by $n$y $t when $e attack$v.";                                 break;
        case 7: i = "$n hit$v $N.";                                                           break;
        case 8: i = "$N $K suddenly hit by $n$y $t.";                                         break;
        case 9: i = "With $s $t, $n succeed$v to harm $N.";                                   break;
        }
    }
    else
    {
        switch( number_range(0, 9) )
        {
        case 0: i = "$n$y $t shakes $N with a powerful hit.";                                 break;
        case 1: i = "$N contort$V in pain as $n$y $t connects with $M.";                      break;
        case 2: i = "$n deal$v a heavy blow to $N with $s $t.";                               break;
        case 3: i = "$N $K overcome with a pained look as $n hit$v $M.";                      break;
        case 4: i = "$N $K surprised by $n$y ferocious attack.";                              break;
        case 5: i = "$n lunge$v at $N, dealing a heavy blow with $s $t.";                     break;
        case 6: i = "Raising $s weapon, $n hit$v $N with the full force of $s attack.";       break;
        case 7: i = "$N double$V over with the strength of $n$y attack!";                     break;
        case 8: i = "$n quite spontaneously leap$v forward, hitting $N with $s $t!";          break;
        case 9: i = "With skill and finesse, $N offer$V $Mself to $n$y awaiting $t.";         break;
        }
    }
    break;
    case 1: /* "slice" */
    if ( dam >= 100 )
    {
        switch( number_range(0, 4) )
        {
        case 0: i = "$n shred$v $N with $s powerful slice!";                                   break;
        case 1: i = "$n swing$v $s $t, bisecting $N$Y body with incredible feriocity!";        break;
        case 2: i = "$N $K vivisected by $n$y $t!";                                           break;
        case 3: i = "$n slice$v $N$Y body to shreds with $s $t!";                 break;
        case 4: i = "$n$y $t slices through $N$Y vital organs!";           break;
        }
    }
    else
    if ( dam < 5 )
    {
        switch( number_range(0, 9) )
        {
        case 0: i = "$n$y $t barely slices $N.";                                              break;
        case 1: i = "$n$y badly placed attack merely scratches $N.";                          break;
        case 2: i = "$n slice$v $N with $s $t, but it does little damage.";                   break;
        case 3: i = "$n$y $t slices away a small piece of $N$Y flesh.";                       break;
        case 4: i = "$n leave$v a small wound in $N with $s $t.";                             break;
        case 5: i = "$n manage$v to slice $N, though the damage is slight.";                  break;
        case 6: i = "$N seem$V unaffected by the slice $n$y $t has left.";                    break;
        case 7: i = "$N $K caught off guard, but $n fail$v to use $s $t effectively.";        break;
        case 8: i = "$N$Y flesh is only slightly marred by $n$y slice.";                      break;
        case 9: i = "$n swing$v $n$y $t short, leaving only a scrape along $N$Y body.";       break;
        }
    }
    else
    if ( dam < 40 )
    {
        switch( number_range(0, 9) )
        {
        case 0: i = "$n make$v a jagged rocking motion with $s $t, tearing $N$Y flesh!";      break;
        case 1: i = "$n$y $t leaves a long, curved wound on $N$Y body.";                      break;
        case 2: i = "With $s $t, $n slice$v $N gruesomely.";                                  break;
        case 3: i = "$N $K unable to avoid $n$y attack; $e slice$v $M.";                      break;
        case 4: i = "With a skillful execution, $n rend$v $N$Y flesh with $s $t.";            break;
        case 5: i = "The blade of $n$y $t slides smoothly along $N$Y body.";                  break;
        case 6: i = "$n$y $t leaves a small wound in $N$Y flesh.";                            break;
        case 7: i = "$n bring$v $s weapon's blade in contact with $N!";                       break;
        case 8: i = "Lunging forward, $n manage$v to slice $N.";                              break;
        case 9: i = "$N lose$V a bit of flesh to $n$y deadly $t!";                            break;
        }
    }
    else
    {
        switch( number_range(0, 9) )
        {
        case 0: i = "$n tear$v a chunk of flesh from $N with $s $t!";                         break;
        case 1: i = "$N shriek$V as $n$y $t connects with $S flailing body!";                 break;
        case 2: i = "$n deal$v a mighty wound to $N with $s $t.";                             break;
        case 3: i = "$N $K momentarily stunned as $n$y $t streaks across $S flesh.";          break;
        case 4: i = "$N $K about to duck $n$y attack when $s $t connects!";                   break;
        case 5: i = "$n lunge$v at $N, slicing away with deadly accuracy at $N$Y frame.";     break;
        case 6: i = "$n swing$v $s $t in a lethal arc, rending $N$Y flesh horridly!";         break;
        case 7: i = "$N frantically reach$W for the wound that $n$y $t created.";             break;
        case 8: i = "$n suddenly find$v $s $t buried in $N.";                                 break;
        case 9: i = "$n slice$v easily through the hard and soft parts of $N$Y anatomy.";     break;
        }
    }
    break;
    case 2: /* "stab" */
    if ( dam >= 100 )
    {
        switch( number_range(0, 4) )
        {
        case 0: i = "$n perforate$v $N with a series of lightning stabs!";              break;
        case 1: i = "$n bur$x $s $t into $N, slaughtering $M!";                     break;
        case 2: i = "$n rearrange$v $N$Y anatomy with $s final stabs!";                       break;
        case 3: i = "With a vicious stab, $n$y $t punctures $N$Y body!";                                 break;
        case 4: i = "$N is impaled on $n$y weapon!";                                          break;
        }
    }
    else
    if ( dam < 5 )
    {
        switch( number_range(0, 9) )
        {
        case 0: i = "$n stab$v $N, but fail$v to penetrate deeply.";                          break;
        case 1: i = "$n arc$v $s weapon a trifle high, missing $N$Y vitals.";                 break;
        case 2: i = "$n drive$v $s $t toward $N, stabbing $M only slightly.";                 break;
        case 3: i = "Fumbling with $s $t, $n ineptly graze$v $N.";                            break;
        case 4: i = "$n$y $t meets heavy resistance from $N$Y body.";                         break;
        case 5: i = "$N sidestep$V, but $n$y attack hits $M lightly anyway.";                  break;
        case 6: i = "The $t $n $k wielding only scratches $N.";                               break;
        case 7: i = "Using only the tip of $s $t, $n lightly cut$v $N.";                      break;
        case 8: i = "$N$Y flesh is only slightly marred by $n$y slice.";                      break;
        case 9: i = "$n swing$v $n$y $t short, leaving only a scrape along $N$Y body.";       break;
        }
    }
    else
    if ( dam < 40 )
    {
        switch( number_range(0, 9) )
        {
        case 0: i = "$n bur$x $s $t up to $s hand in $N$Y body!";                             break;
        case 1: i = "With $s $t, $n open$v a gaping wound in $N.";                            break;
        case 2: i = "$n stab$v $N gruesomely.";                                               break;
        case 3: i = "$n leave$v a gory wound in $N with $s attack.";                          break;
        case 4: i = "$n poke$v a large hole in $N$Y flesh!";                                  break;
        case 5: i = "$N suddenly find$V $n$y $t tearing through $S vitals!";                  break;
        case 6: i = "$n stab$v $N with $s $t!";                                               break;
        case 7: i = "$n stick$v $s weapon far into $N.";                                      break;
        case 8: i = "With a sudden lunge, $n drive$v $s $t into $N$Y awaiting flesh!";          break;
        case 9: i = "$n$y $t tears a bit of flesh from $N, leaving behind a sickly wound!";   break;
        }
    }
    else
    {
        switch( number_range(0, 9) )
        {
        case 0: i = "$n suddenly find$v $s $t covered in $N$Y bodily fluids.";                break;
        case 1: i = "$N let$V out a small scream as $n stab$v $M!";                           break;
        case 2: i = "$n whirl$v $s weapon in the air, then wound$v $N with vicious ferver!";  break;
        case 3: i = "$N $K taken by surprise as $n tr$x to shred $M to pieces!";              break;
        case 4: i = "$n fumble$v with $s $t, but manage$v to eviscerate $N anyway!";          break;
        case 5: i = "$N meet$V $n$y attack with full force.";                                 break;
        case 6: i = "In a powerful maneuver, $n arc$v $s weapon perfectly into $N$Y vitals!"; break;
        case 7: i = "A sickening sound fills your ears as $n horribly stab$v $N.";            break;
        case 8: i = "$n suddenly find$v $s $t buried in $N.";                                 break;
        case 9: i = "With grotesque ease, $n slides $s $t into $N$Y gullet.";                 break;
        }
    }
    break;
    }
    }
    display_interp( ch, "^4" );

    if ( number_range( 0, 50 ) > 25 )
    act( i, ch, w, vi, TO_ALL );
    else act( i, ch, w, vi, TO_ACTOR );
    display_interp( ch, "^N" );
    return;
}



void MSG_weapon_partial_dodge( PLAYER *ch, PROP*prop, PLAYER *victim )
{
/*    bug( "MSG_weapon_partial_dodge: msg",0 );    */
    return;
}



void MSG_weapon_parry( PLAYER *ch, PROP *prop, PROP *vo,
                       PLAYER *victim )
{
    char w[MAX_STRING_LENGTH];
    char *i = NULL;

    sprintf( w, "%s", smash_article(STR(vo,short_descr)) );
    switch( number_range(0,7) )
    {
    case 0: i = "$N parr$X $n$y attack with $S $t."; break;
    case 1: i = "$N block$V $n$y strike with $S $t."; break;
    case 2: i = "$N bring$V $S $t up to parry $n$y blow."; break;
    case 3: i = "$N lift$V $S $t and manage$V to parry $S strike."; break;
    case 4: i = "$n$y attack is brushed aside by $N$Y parry."; break;
    case 5: i = "$N deflect$V $n$y attack with $S $t."; break;
    case 6: i = "$n$y blow glances harmlessly off from $N$Y $t."; break;
    case 7: i = "$n$y strike thuds into $N$Y $t, causing no damage."; break;
    }
    act( i, ch, w, victim, TO_ALL );
    return;
}



void MSG_weapon_dodge( PLAYER *ch, PROP *prop, PLAYER *victim )
{
    char w[MAX_STRING_LENGTH];
    char *i = NULL;

    sprintf( w, "%s", smash_article(STR(prop,short_descr)) );
    switch(number_range(0,7))
    {
	case 0: i = "$N dodge$V $n$y $t."; break;
	case 1: i = "$N feint$V and manage$V to avoid being hit by $n."; break;
	case 2: i = "$N step$V back just in time to avoid $n$y blow."; break;
	case 3: i = "$N leap$V to the left, barely dodging $n$y $t."; break;
	case 4: i = "$N leap$V to the right, avoiding $n$y strike."; break;
	case 5: i = "$N duck$V under $n$y $t."; break;
	case 6: i = "$N swerve$V to the left, avoiding easily $n$y $t."; break;
	case 7: i = "$N swerve$V to the right, dodging $n$y $t with little effort."; break;
    }
    act( i, ch, w, victim, TO_ALL );
    return;
}

void MSG_weapon_miss( PLAYER *ch, PROP *prop, PLAYER *victim )
{
    char w[MAX_STRING_LENGTH];
    char *i = NULL;

    sprintf( w, "%s", smash_article(STR(prop,short_descr)) );
    switch(number_range(0,9))
    {
        case 0: i = "$n swing$v $s $t wide and miss$w $N."; break;
	case 1: i = "$n$y haphazard lunge misses $N."; break;
	case 2: i = "$n thrust$v $s $t at $N but miss$w."; break;
	case 3: i = "$n$y badly aimed attack misses $N."; break;
	case 4: i = "$n$y feeble strike misses $N."; break;
	case 5: i = "$n nearly lose$v $s balance, missing $s blow."; break;
	case 6: i = "$n stretch$w forward to strike and fall$v out of balance."; break;
	case 7: i = "$n$y blow falls short of $N."; break;
        case 8: i = "$n$y strike connects only with thin air."; break;
        case 9: i = "$n$y poor attack fails."; break;
    }

    if ( number_range( 0, 50 ) > 25 ) {
    act( i, ch, w, victim, TO_ALL ); } else
      act( i, ch, w, victim, TO_ACTOR );
    return; 
}


void MSG_actor_hit( PLAYER *ch, int idx, PLAYER *victim, int dam )
{
    char w[MAX_STRING_LENGTH];
    char *i = NULL;

    sprintf( w, "%s",   damage_strings[URANGE(0,dam/10,12)] );

    switch( idx )
    {
        case 0: i = "$4$n $Bhit$v$R$4 $N$t$R"; break;
        case 1: i = "$4$n $Bslice$v$R$4 $N$t$R"; break;
        case 2: i = "$4$n $Bstab$v$R$4 $N$t$R"; break;
        case 3: i = "$4$n $Bslash$w$R$4 $N$t$R"; break;
        case 4: i = "$4$n $Bwhip$v$R$4 $N$t$R"; break;
        case 5: i = "$4$n $Bcleave$v$R$4 $N$t$R"; break;
        case 6: i = "$4$n $Bvorpal$v$R$4 $N$t$R"; break;
        case 7: i = "$4$n $Bpound$v$R$4 $N$t$R"; break;
        case 8: i = "$4$n $Bpierce$v$R$4 $N$t$R"; break;
        case 9: i = "$4$n $Blash$w$R$4 $N$t$R"; break;
        case 10:i = "$4$n $Bclaw$v$R$4 $N$t$R"; break;
        case 11:i = "$4$n $Bblast$v$R$4 $N$t$R"; break;
        case 12:i = "$4$n $Bcrush$w$R$4 $N$t$R"; break;
        case 13:i = "$4$n $Bgore$v$R$4 $N$t$R"; break;
        case 14:i = "$4$n $Bbite$v$R$4 $N$t$R"; break;
        case 15:i = "$4$n $Btear$v$R$4 $N's flesh with slippery suctions.$R"; break;
     default: {bug( "MSG_actor_hit: msg",0 ); return;}     break;
    }

    act( i, ch, w, victim, TO_ALL );
    return; 
}



void MSG_partial_dodge( PLAYER *ch, int idx, PLAYER *victim )
{
    return;
}

void MSG_hand_hit( PLAYER *ch, PLAYER *victim, int dam )
{
    char *i = NULL;

    if ( dam >= 100 ) i = "$n slay$v $N with $s powerful hit!";
    else
    if ( dam < 5 )
    {
        switch( number_range(0, 9) )
        {
	case 0: i = "$n$y punch barely grazes $N.";                                         break;
	case 1: i = "$n$y badly placed punch thuds harmlessly on $N.";                        break;
	case 2: i = "$n punch$w $N, but does little damage.";             break;
	case 3: i = "$n$y punch grazes $N, but the damage is slight.";                                         break;
	case 4: i = "$n punch$w $N lightly.";                                              break;
	case 5: i = "$n manage$v to punch $N, though the damage is slight.";               break;
	case 6: i = "$n$y feeble punch hardly hits $N at all.";                 break;
	case 7: i = "$N$Y $K distracted, but $n fail$v to do $M with any real damage.";  break;
	case 8: i = "$n punch$w $N$Y in a weak spot, but fail$v to do much damage.";   break;
	case 9: i = "$n$y pound connects $N, but without visible effect.";                      break;
	}
    }
    else
    if ( dam < 12 )
    {
        switch( number_range(0, 9) )
        {
	case 0: i = "$n punch$w $N viciously.";                                                break;
	case 1: i = "$N fail$V to escape $n$y hard uppercut.";                                           break;
	case 2: i = "$n pound$v $N.";                                                           break;
	case 3: i = "$N $K unable to dodge $n$y attack; $e hurt$v $M.";                       break;
	case 4: i = "Having surprised $N, $n step$v forward and punch$w $M.";                  break;
	case 5: i = "$n$y left hook thuds into $N.";                  break;
	case 6: i = "$N $K winded by $n$y right hook.";                                 break;
	case 7: i = "$n$y hard straight connects $N.";                                                           break;
	case 8: i = "$N $K suddenly hit by $n$y fast jab.";                                         break;
	case 9: i = "$n$y sudden knee jerk bruises $N.";                                   break;
        }
    }
    else
    {
        switch( number_range(0, 9) )
        {
	case 0: i = "$n$y fast left thuds painfully into $N.";                                 break;
	case 1: i = "$N contort$V in pain as $n$y uppercut connects with $M.";                      break;
	case 2: i = "$n deal$v a heavy right hook to $N.";                               break;
	case 3: i = "$N $K overcome with a pained look as $n punch$w $M.";                      break;
	case 4: i = "$N $K surprised by $n$y ferocious attack.";                              break;
	case 5: i = "$n feint$v and throw$v a forceful blow into $N.";                     break;
	case 6: i = "$N groan$V in pain as $n$y heavy punch hits $M.";       break;
	case 7: i = "$N double$V over with the strength of $n$y hard jab!";                     break;
	case 8: i = "$n quite spontaneously leap$v forward, delivering a knee jerk into $N$Y groin!";          break;
	case 9: i = "$N leap$v forward, straight into $n$y outstretched fist!";         break;
        }
    }
    act( i, ch, NULL, victim, TO_ALL );
    return;
}

void MSG_hand_miss( PLAYER *ch, PLAYER *victim )
{
    char *i = NULL;
    switch(number_range(0,9))
    {
	case 0: i = "$n swing$v $s uppercut wide and miss$w $N."; break;
	case 1: i = "$n$y haphazard straight misses $N."; break;
	case 2: i = "$n jab$v at $N but miss$w."; break;
	case 3: i = "$n$y badly aimed attack misses $N."; break;
	case 4: i = "$n$y feeble punch misses $N."; break;
	case 5: i = "$n nearly lose$v $s balance!"; break;
	case 6: i = "$n lean$v forward to punch and nearly fall$v on $s nose."; break;
	case 7: i = "$n$y blow falls short of $N."; break;
        case 8: i = "$n$y balance is off, the attack does no damage."; break;
        case 9: i = "$N remains untouched by $n$y attempt."; break;
    }
    if ( number_range( 0, 50 ) > 25 )
    act( i, ch, NULL, victim, TO_ALL );
    else act( i, ch, NULL, victim, TO_ACTOR );
    return;
}

void MSG_dodge( PLAYER *ch, int idx, PLAYER *victim )
{
    const char *i;
    i = "$N dodge$V $n$y attack.";
    act( i, ch, NULL, victim, TO_ALL );
    return;
}



/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
    PLAYER *ch;
    PLAYER *ch_next;
    PLAYER *victim;

    for ( ch = actor_list; ch != NULL; ch = ch->next )
    {
        ch_next = ch->next;

        ch->fbits = 0;
        cmd_hunt( ch, "continue" );

        if ( ( victim = ch->fighting ) == NULL || ch->in_scene == NULL )
            continue;
 
	if ( ch->position < POS_SLEEPING )
	{
	    stop_fighting( ch, FALSE );
	    continue;
	}

	else if ( ch->position < POS_FIGHTING )
	{
	    if ( ch->position == POS_SLEEPING && !IS_AFFECTED(ch, BONUS_SLEEP) )
	    	to_actor( "You jolt awake and rejoin the fray.\n\r", ch );
	    cmd_stand( ch, "" );
	}

        if ( NPC(ch) )
        {
            if ( IS_SET(ch->flag, ACTOR_GOOD) ) cmd_peace(ch,""); 
            if( get_eq_char( ch, WEAR_WIELD_1 ) == NULL
             || get_eq_char( ch, WEAR_WIELD_2 ) == NULL ) {
            cmd_draw( ch, "" );
	    /* If disarmed, try to find a weapon? */
            cmd_get( ch, "all" );
             }
        }

        /*
         *  This code worked once but causes major problems.
         *  (Memory corruption) -- not really sure why; it
         *  wasn't a major performance hog.
         *
         *  I removed it.  Combat scripts are now triggered
         *  at the beginning of combat.  Loops are required
         *  to make a script loop until the end of combat.
         */

        /*
        script_update( ch, TYPE_ACTOR,           
                           TRIG_COMBAT, victim, NULL, NULL, NULL );
        trigger_list( ch->carrying,            
                      TRIG_COMBAT, 
                      victim, NULL, NULL, NULL );

        if ( ch->in_scene ) {
        script_update( ch->in_scene, TYPE_SCENE, 
                       TRIG_COMBAT, 
                       ch,     NULL, NULL, NULL );
        trigger_list( ch->in_scene->contents,   
                      TRIG_COMBAT, 
                      victim, NULL, NULL, NULL ); 
        } */

        /* No actions for chars in WAIT_STATE. Handle wait for actors */
	if (ch->wait > 0)
        {
	    if ( ch->desc == NULL )
	       ch->wait -= PULSE_VIOLENCE;
	    continue;
        }

        if ( IS_AWAKE(ch)
          && ch->in_scene == victim->in_scene
	  && ch != victim )
	{
            /* Maybe give status report */
            if ( !NPC(ch) ) {
                if ( number_range( 0, 100 ) < 25 ) {
                    act( "$N looks $t.", ch, STRING_HITS(victim), victim, TO_ACTOR);
                }
            }

	    /* Exchange blows */
	    oroc( ch, victim );

	    /* Maybe change combat tactic? */
	    if ( NPC(ch) )
		auto_fight( ch, victim );

	    /* Maybe flee? -- This has been moved here from damage(),
	     * because fleeing may cause damage to the victim and we 
	     * don't want to be caught in an infinite loop
	     */
	    if ( NPC(victim) && victim->hit <= 50 )
		auto_flee( ch, victim );
	}
        else
            stop_fighting( ch, FALSE );
    }

    return;
}



/*
 * Do one group of attacks.
 * (One Round One Char)
 */
void oroc( PLAYER *ch, PLAYER *victim )
{
    PROP *primary_wpn, *secondary_wpn;

    /*
     * Add combat script trigger here, or add to cmd_fight().
     */

    primary_wpn = get_eq_char( ch, WEAR_WIELD_1 );
    secondary_wpn = get_eq_char( ch, WEAR_WIELD_2 );

    weapon_attack( ch, primary_wpn, victim );

    /*
     * Weapon_attack() does a hand attack if no weapon found. However,
     * we want to make a dual wield attack only if ch is actually
     * wielding a weapon, so we must make a check here...
     * Also, if the victim died, there's no point attacking...
     */
    if (  ch->fighting != NULL
     && skill_check( ch, skill_lookup( "dual wield" ), 25 )
     && secondary_wpn != NULL
     && secondary_wpn->item_type == ITEM_WEAPON )
    	weapon_attack( ch, secondary_wpn, victim );

    if ( ch->fighting != NULL
      && skill_check( ch, skill_lookup( "second attack" ), 40 )
      && !IS_SET(ch->fbits, FGT_PARRY) )
    	weapon_attack( ch, primary_wpn, victim );

    if ( ch->fighting != NULL
      && skill_check( ch, skill_lookup( "third attack" ), 65 )
      && !IS_SET(ch->fbits, FGT_PARRY) )
    	weapon_attack( ch, primary_wpn, victim );

    if ( ch->fighting != NULL
      && skill_check( ch, skill_lookup( "fourth attack" ), 65 )
      && !IS_SET(ch->fbits, FGT_PARRY) )
    	weapon_attack( ch, primary_wpn, victim );

    if ( ch->fighting != NULL
      && skill_check( ch, skill_lookup( "fifth attack" ), 65 )
      && !IS_SET(ch->fbits, FGT_PARRY) )
    	weapon_attack( ch, primary_wpn, victim );

    if ( NPC(ch) && ch->fighting != NULL )
	actor_attack( ch, victim );

    return;
}

/*----------------------
 * Attack modifier funcs
 *----------------------*/

/*
 * Calculates effects of hit to the armor: armor wear and
 * deflection. Note that beat_armor() has been moved here (it's
 * more logical that the wearing out occurs simultaneously to
 * the blow...
 */
int armor_deflect( PLAYER *victim, int idx )
{

    PROP *armor;
    int deflect;

    /* Add Check for Material Later */

    armor = get_eq_char( victim, number_range( 0, MAX_WEAR-1 ) );

    if ( armor != NULL && armor->item_type == ITEM_ARMOR )
    {
    	if ( armor->value[1] > 0
          && armor->value[2] > 0 )
           --armor->value[1];

    	/* Gradually worsen the armor condition... */
    	if ( armor->value[1] == 0 )
        armor->value[1] = (--armor->value[0] > 0) ? armor->value[2] : -1;
    }
    /*
     * INTERPOLATE() does not work, because victim_ac can be negative
     * Here we convert victim_ac to a percentage value and reverse
     * it so that 0 (no deflection) - 100 (complete deflection)
     */

    deflect = 100 - (GET_AC(victim) + 100)/2 + (15-number_range(0, 30));
    return( URANGE(0,deflect,100) );
}

/*
 * Calculates whether a hit really connects the victim
 */
int chance_to_hit( PLAYER *ch, PLAYER *victim )
{
    int chance;
    chance = 25;

    /*
     * Size adjustment.
     */
    if ( ch->size + 2 < victim->size
    && (ch->size > 0 && victim->size > 0) )
    chance += 10;

    /*
     * STR vs. DEX adjustment
     */
    if ( get_curr_str( ch ) > get_curr_dex( victim ) )
	chance += 5;

    /*
     * Fight mode adjustment
     * Victim position adjustment
     */
    chance += fight_mode_table[ch->fmode].tohit;
    if ( victim->position < POS_FIGHTING )
	chance += 40;
    else
       chance -= fight_mode_table[victim->fmode].dodge;

    /*
     * Drunkeness adjustment
     */
    if ( !NPC(ch) )
	chance -= PC(ch, condition)[COND_DRUNK];
    if ( !NPC(victim) && PC(victim, condition)[COND_DRUNK] > 0 )
	chance += PC(victim, condition)[COND_DRUNK];
    /*
     * Blindness/invis adjustment
     */
    if ( !can_see(ch,victim) )
	chance -= 40;

    /*
     * Health adjustment.
     * Hurt characters have less chance to hit, and are hit easier.
     */
    chance += ((PERCENTAGE(ch->hit, MAXHIT(ch))+25)/2)-25;
    chance += ((PERCENTAGE(victim->hit, MAXHIT(victim))+25)/2)-25;

    return( chance );
}

/*
 * Calculates modifiers to the damage
 */
int damage_adjust( PLAYER *ch, int dam, int deflected )
{
    /*
     * Deflected is again the percentage of damage deflected,
     * so to find out damage applied we must subtract the
     * value from 100 and convert to percentage.
     */
    dam = dam * (100 - deflected) / 100 
	+ fight_mode_table[ch->fmode].todam
	+ str_app[get_curr_str(ch)].todam;
    return UMAX(0,dam);
}

void actor_attack( PLAYER *ch, PLAYER *victim )
{
    int count;
    int dam;
    int oldhit;

    if ( NPC(ch) )
    {
    for ( count = 0;  count < MAX_ATTACK;  count++ )
    {
        ATTACK *attack;

        if ( (attack = ch->pIndexData->attacks[count]) != NULL )
        {
        /*
         * Hit or Miss?
	 */
	if ( number_percent( ) < chance_to_hit( ch, victim ) )
	{
	    dam = number_range( attack->dam1, attack->dam2 );

    if ( attack_table[attack->idx].hit_fun != NULL
     && attack_table[attack->idx].hit_fun( ch, victim, dam, victim->hit) 
)
    continue;

	}
        else
	{
	    damage( ch, victim, 0 );
	    continue;
	}

        if ( can_see(victim,ch) && IS_AWAKE(victim) )
        {
        if ( skill_check( victim, skill_lookup( "dodge" ), 50 ) )
        {
            MSG_dodge( ch, attack->idx, victim );
	    SET_BIT( victim->fbits, FGT_DODGE );
            continue;
        }

        if ( skill_check( victim, skill_lookup( "dodge" ), 75 ) )
        {
            MSG_partial_dodge( ch, attack->idx, victim );
            damage( ch, victim, dam / 2 );
	    SET_BIT( victim->fbits, FGT_PARRY );
            continue;
        }
        }

        oldhit = victim->hit;
        MSG_actor_hit( ch, attack->idx, victim, PERCENTAGE(dam,oldhit) );
        damage( ch, victim, dam );
	}
    }
    }

    return;
}


/*
 * Hand-to-Hand Routines
 */
void hand_attack( PLAYER *ch, PLAYER *victim )
{
    int dam = 0, oldhit;
    /*
     * Can't beat a dead char!
     * Guard against weird scene-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_scene != victim->in_scene )
    return;

    /*
     * If we a used special skill, do nothing more
     */
    if ( unarmed_attack( ch, victim ) )
	return;
    /*
     * Hit or Miss?
     */

    if ( number_percent( ) < chance_to_hit( ch, victim ) )
    {
	/* Hit, check armor. */
	dam = damage_adjust( ch, 1, armor_deflect( victim, 0 ) );
    }
    else
    {
	MSG_hand_miss( ch, victim );
	if ( !NPC(ch) && NPC( victim ) )
	    damage( ch, victim, 0 );
        return;
    }

    if ( skill_check( victim, skill_lookup( "dodge" ), 50 )
     && can_see(victim,ch) && IS_AWAKE(victim) )
    {
	MSG_dodge( ch, 0, victim );
	SET_BIT( victim->fbits, FGT_DODGE );
        return;
    }

    oldhit = victim->hit;
    MSG_hand_hit( ch, victim, PERCENTAGE(dam,oldhit) );
    damage( ch, victim, dam );
    tail_chain( );
    return;
}


/*
 * Hit one guy once.
 */
void weapon_attack( PLAYER *ch, PROP *prop, PLAYER *victim )
{
    int dam = -1;   /* miss */
    int oldhit;
    int chance;
    int deflected;
    int wp;

    if  ( prop == NULL
      || victim == NULL
      || prop->item_type != ITEM_WEAPON ) 
    {
        bool ATTACKS = FALSE;

        if ( NPC(ch) ) {
        for ( wp=0;  wp < MAX_ATTACK;  wp++ ) {
            ATTACKS = ATTACKS || ch->pIndexData->attacks[wp] != NULL;
        }
        }
        if (!ATTACKS) hand_attack( ch, victim );
        return;
    }

    /*
     * Can't beat a dead char!
     * Guard against weird scene-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_scene != victim->in_scene )
    return;

    /*
     * If we a used special skill, do nothing more
     */
    if ( armed_attack( ch, victim ) )
	return;

    /*
     * Hit or Miss?
     */
    chance = chance_to_hit( ch, victim );
    /*
     * Weapon proficiencies.
     */
    wp = attack_table[URANGE(0,prop->value[3],MAX_ATTACK)].wpgsn;
    /*
     * Rather than use a flat modifier, increase the chance in
     * relation to the weapon skill
     */
    chance += (learned(ch,wp)*30)/100;

    if ( number_percent( ) < chance )
    {
	/* Hit, check armor. */
	deflected = armor_deflect( victim, prop->value[3] );
        if ( skill_check(ch, get_skill_index(wp), 0) )
            deflected /= 2;

	dam = number_range( UMIN(prop->value[1],prop->value[2]),
			    UMAX(prop->value[1],prop->value[2])  );
	dam = damage_adjust( ch, dam, deflected );
        SET_BIT( ch->fbits, FGT_PRIMARY );
    }
    else
    {
        MSG_weapon_miss( ch, prop, victim );
	if ( !NPC(ch) && NPC( victim ) )
	    damage( ch, victim, 0 );
        return;
    }

    if ( attack_table[prop->value[3]].hit_fun != NULL
     && attack_table[prop->value[3]].hit_fun( ch, victim, dam, victim->hit) )
    return;


    if ( victim->wait <= 0 && can_see( victim, ch ) && IS_AWAKE( victim ) )
    {
    /*
     * Shield parry
     */
    if ( skill_check( victim, skill_lookup( "parry" ), 50 )
      && !IS_SET(victim->fbits, FGT_PRIMARY) )
    {
        PROP *vo;

	vo = get_eq_char( victim, WEAR_SHIELD );
        if ( vo == NULL )
        vo = get_eq_char( victim, WEAR_HOLD_1 );
        if ( vo == NULL )
        vo = get_eq_char( victim, WEAR_HOLD_2 );

        if ( vo != NULL && vo->item_type == ITEM_ARMOR )
        {
        MSG_weapon_parry( ch, prop, vo, victim );
	SET_BIT( victim->fbits, FGT_PARRY );
        return;
        }
    }

    /*
     * Weapon parry, lower chance
     */
    if ( skill_check( victim, skill_lookup( "parry" ), 30 )
      && !IS_SET(victim->fbits, FGT_PRIMARY) )
    {
        PROP *vo;

        vo = get_eq_char( victim, WEAR_WIELD_1 );
        if ( vo == NULL )
        vo = get_eq_char( victim, WEAR_WIELD_2 );

        if ( vo != NULL )
        {
        MSG_weapon_parry( ch, prop, vo, victim );
	SET_BIT( victim->fbits, FGT_PARRY );
        SET_BIT( victim->fbits, FGT_PRIMARY );
        return;
        }
    }

    /*
     * Full dodge
     */
    if ( skill_check( victim, skill_lookup( "parry" ), 50 ) )
    {
        MSG_weapon_dodge( ch, prop, victim );
	SET_BIT( victim->fbits, FGT_DODGE );
        return;
    }

    /*
     * Partial dodge
     */
    if ( skill_check( victim, skill_lookup( "parry" ), 75 ) )
    {
        MSG_weapon_partial_dodge( ch, prop, victim );
        damage( ch, victim, dam / 2 );
	SET_BIT( victim->fbits, FGT_PARRY );
        SET_BIT( victim->fbits, FGT_DODGE );
        return;
    }
    }


    oldhit = victim->hit;
    MSG_weapon_hit( ch, prop, victim, PERCENTAGE(dam,oldhit) );
    damage( ch, victim, dam );
    tail_chain( );
    return;
}



void exp_gain( PLAYER *ch, int gain, bool fMessage )
{
    char buf[MAX_STRING_LENGTH];
    int gained_levels=0;
    int xp=0;
    int needed=0;

    if ( gain == 0 ) return;

    gain = gain + URANGE(0,number_range( gain/4, gain/2 ),gain); 

    snprintf( buf, MSL, "Experience> %s gains %d exp\n\r", NAME(ch), gain );
    NOTIFY( buf, LEVEL_IMMORTAL, WIZ_NOTIFY_DEATH );

    /*
     * The Experience Equation
     */

    xp= (ch->exp + gain); 
    ch->exp +=gain;
    if ( fMessage == TRUE ) {
        display_interp( ch, "^3" ); 
        snprintf( buf, MAX_STRING_LENGTH, "You gain %d experience point%s.\n\r", gain, gain > 1 ? "s" : "" );
        to_actor( buf, ch );
        display_interp( ch, "^N" ); 
    }       

    needed=(ch->exp_level+1)*EXP_PER_LEVEL;
     while ( 1 ) {
      if ( xp > needed ) {
        gained_levels++;
        xp-=needed; 
        needed=EXP_PER_LEVEL;
      }
      else break;
    }
    
    if ( gained_levels > 0 ) {
        bool fgain=0;  int ogained=gained_levels;
        while ( gained_levels-- > 0 ) { ch->exp_level++;
         if (ch->exp_level % 5 == 0) fgain++; }
        gained_levels=ogained;
        display_interp( ch, "^3" ); 
        display_interp( ch, "^B" );

        snprintf( buf, MAX_STRING_LENGTH, "You raised %s level%s", gained_levels == 1 ? "a" : 
                         capitalize(numberize(gained_levels)), 
                      gained_levels > 1 ? "s!" : "!" );
        to_actor( buf, ch );

        display_interp( ch, "^N" );
        display_interp( ch, "^B" );
        to_actor( " You are now level ", ch );
        to_actor( capitalize(numberize(ch->exp_level)), ch );
        to_actor( ".\n\r", ch );
      
        if ( !NPC(ch) && !IS_HERO(ch) && ch->exp_level >= MAX_MORTAL_LEVEL ) {
          snprintf( buf, MAX_STRING_LENGTH, "A new star shines in the heavens!\n\r%s has become immortal.", NAME(ch) );
          add_history( ch, buf );
          write_global( buf );
          to_actor( "You are now a hero and may commission a castle.\n\r", ch );
          PC(ch,level) = LEVEL_HERO;         
        } else if ( fgain > 0 && !NPC(ch) ) {
            if (fgain == 1)
            to_actor( "You gain a hero point.\n\r", ch ); 
            else { to_actor( "You gain ", ch ); 
                   to_actor( itoa(fgain), ch );
                   to_actor( " hero points.\n\r", ch ); 
                 }
           if ( PC(ch,stat_points) > 0 ) {
            snprintf( buf, MAX_STRING_LENGTH, "%s gains new status amongst his peers.\n\r",
                     NAME(ch) );
            } else {
            snprintf( buf, MAX_STRING_LENGTH, "%s is given heroic stature.  %s is known as a %sfamous adventurer.\n\r",
                     NAME(ch), ch->sex == SEX_MALE ? "He" : 
                             ( ch->sex == SEX_FEMALE ? "She" : "It" ),
                     ch->karma > 0 ? "" : "in" );
            }
            add_history( ch, buf );
            PC(ch,stat_points)++;
        }
        display_interp( ch, "^N" );
    }   

    return;      
}

/*
 * Inflict damage from a hit.
 */
void damage( PLAYER *ch, PLAYER *victim, int dam )
{
    if ( dam != 0 )
    {
        PLAYER *rch;
        char buf[12];

        for( rch = ch->in_scene->people; rch != NULL; rch = rch->next_in_scene )
        {
            if ( IS_IMMORTAL(rch) && IS_SET(rch->flag,WIZ_NOTIFY_DAMAGE))
            {
                snprintf( buf, MAX_STRING_LENGTH, "[%c%c%3d->%c%c] ",
                         rch != ch ? STR(ch,name)[0] : '*',
                         rch != ch ? STR(ch,name)[1] : '*',
                         dam,
                         rch != victim ? STR(victim,name)[0] : '*',
                         rch != victim ? STR(victim,name)[1] : '*' );
                to_actor( buf, rch );
            }
        }
 
   }

    if ( IS_IMMORTAL(victim) && victim->hit < dam )
    return;

    if ( victim->position == POS_DEAD )
        return;

    /*
     * Stop up any residual loopholes.
     */
    if ( dam > 10000 )
    {
    wtf_logf( "Damage: %d: more than 10000 points!", dam );
    dam = 10;
    }

    if ( victim != ch )
    {
        /*
         * Certain attacks are forbidden.
         * Most other attacks are returned.
         */
        if ( is_safe( ch, victim ) )
            return;

        if ( victim->position > POS_STUNNED )
        {
            if ( victim->fighting == NULL )
                set_fighting( victim, ch );
            victim->position = POS_FIGHTING;
            if ( ch->fighting == NULL )
                set_fighting( ch, victim );
        }

        /*
         * More charm stuff.
         */
        if ( victim->master == ch )
            stop_follower( victim );

        /*
         * Inviso attacks ... not.
         */
        if ( IS_AFFECTED(ch, BONUS_INVISIBLE) )
        {
            SKILL *pSkill = skill_lookup( "invisibility" );
            SKILL *pMass = skill_lookup( "mass invisibility" );
            bonus_strip( ch, pSkill->dbkey );
            bonus_strip( ch, pMass->dbkey );
            REMOVE_BIT( ch->bonuses, BONUS_INVISIBLE );
            act( "$n fades into existence.", ch, NULL, NULL, TO_SCENE );
        }

        /*
         * Damage modifiers.
         */
        if ( IS_AFFECTED(victim, BONUS_SANCTUARY) )
            dam /= 2;

        if ( IS_AFFECTED(victim, BONUS_PROTECT) )
            dam -= dam / 4;

/*
 * Fix lightly bug.
 */
        if ( dam < 0 )
            dam = 1;
    }
    if ( dam == 0 )
	return;

    /*
     * Hurt the victim.
     */
    if ( PERCENTAGE( victim->hit, MAXHIT(victim) ) <= 40 )
	SET_BIT(victim->fbits,FGT_WOUNDED);
    SET_BIT(victim->fbits,FGT_HIT);
    victim->hit -= dam;
    if ( IS_IMMORTAL(victim) && victim->hit < 1 ) victim->hit = 1;

    update_pos( victim );

    /*
     * These messages have to be here instead of violence_update()
     * because stop_fighting() is called below...
     */
    if ( victim->position == POS_STUNNED )
    {
        act( "$n is mortally wounded!", victim, NULL, NULL, TO_SCENE );
        to_actor("You are badly wounded, but will probably recover.\n\r", victim );
    }
    else if ( victim->position == POS_DEAD )
    {
        to_actor( "** You have been killed. **\n\r\n\r", victim );
        if ( IS_IMMORTAL(victim) )
        {
            to_actor( "But you are immortal, so you rise again.\n\r", victim );
            victim->hit = 50;
            victim->position = POS_STANDING;
            return;
        } else victim->exp=victim->exp - victim->exp/4;   /* EXPERIENCE EQUATION */
    }

    /*
     * Sleep spells and extremely wounded folks.
     * If both ch and victim are PCs, stop the combat.
     */
    if ( !IS_AWAKE(victim) )
	stop_fighting( victim, !NPC(ch) && !NPC(victim) ? TRUE : FALSE );

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
        if ( !NPC(victim) )
        {
        char  buf[MAX_STRING_LENGTH];

        sprintf( log_buf, "%s killed by %s at %d",
                NAME(victim),
                NAME(ch),
                victim->in_scene->dbkey );
            log_string( log_buf );

        snprintf( buf, MAX_STRING_LENGTH, "Notify> %s", log_buf );
        NOTIFY( buf, LEVEL_IMMORTAL, WIZ_NOTIFY_DEATH );
        }

   /*     if ( IS_SET(victim->bounty == 0 )   ch->bounty += 500;    Crime */

        if ( (NPC(ch) && IS_SET(victim->flag, ACTOR_BOUNTY))
        || victim->bounty > 0 )
        {
            ch->owed   += victim->bounty;
            victim->bounty = 0;
        }

        /*
         * Karma, credits+bucks, and experience.
         */  
{
        PLAYER *pGroup;

        ch->karma += victim->karma;

        if ( !NPC(ch) && NPC(victim) ) {
         ch->credits += victim->credits;
         ch->bucks += victim->bucks;
         if ( victim->credits > 0 ) 
          var_to_actor( "Your ^6St^1^Bar^N^Bcard^N amount available increases by ^B%d^N.\n\r", ch, victim->credits );
         if ( victim->bucks > 0 ) 
          var_to_actor( "You got ^2%d bucks^N.\n\r", ch, victim->bucks );
        }
        exp_gain( ch, victim->exp, TRUE );

        /* Group gain */
        for ( pGroup = actor_list;  pGroup != NULL;  pGroup = pGroup->next )
        {
               if ( ( in_group( pGroup, ch ) 
                   || ch->master == pGroup 
                   || (ch->master && in_group( pGroup, ch->master ))
                    )
                 && ch != pGroup
                 && pGroup->in_scene == ch->in_scene  ) 
               { //

        if ( !NPC(pGroup) && NPC(victim) ) {
         pGroup->credits += victim->credits/2;
         pGroup->bucks += victim->bucks/2;
         if ( victim->credits > 0 ) 
          var_to_actor( "Your ^6St^1^Bar^N^Bcard^N amount available increases by ^B%d^N.\n\r", pGroup, victim->credits/2 );
         if ( victim->bucks > 0 ) 
          var_to_actor( "You got ^2%d bucks^N.\n\r", pGroup, victim->bucks/2 );
        }

               exp_gain( pGroup, ((NPC(victim) ? victim->exp : 
                                                   victim->exp/4) / 2)
                               + ((NPC(victim) ? victim->exp 
                                                 : victim->exp/4) / 4), 
                             TRUE );

         if ( !NPC(pGroup) && pGroup->leader && number_range(0,100) <= 2 ) {
                   int amt; amt = number_range(0,10+(MAX_MORTAL_LEVEL-pGroup->exp_level)/7);
                   if ( amt > 0 ) {
                    var_to_actor( "^3Group leader bonus: ^N^B%d experience!^N\n\r", pGroup->leader, amt );
                    exp_gain( pGroup->leader, amt, TRUE );
                   }
               }
          } //
        }
}
        script_update( ch, TYPE_ACTOR, TRIG_KILLS, victim, NULL, NULL, NULL );
        script_update( victim, TYPE_ACTOR, TRIG_DIES, ch, NULL, NULL, NULL );
        script_update( victim->in_scene, TYPE_SCENE, TRIG_DIES, victim, NULL, NULL, NULL );
        trigger_list( victim->carrying, TRIG_DIES, victim, NULL, NULL, NULL );
        trigger_list( ch->carrying, TRIG_DIES, victim, NULL, NULL, NULL );
        trigger_list( ch->in_scene->contents, TRIG_DIES, victim, NULL, NULL, NULL );
        raw_kill( victim );
        return;
    }

    if ( victim == ch )
        return;

    /*
     * Take care of link dead people.
     */
    if ( !NPC(victim) && victim->desc == NULL )
	retreat( victim );
    tail_chain( );
    return;
}



bool is_safe( PLAYER *ch, PLAYER *victim )
{
    if ( NPC(ch) || NPC(victim) )
        return FALSE;

    /* Thx Josh! */
    if ( victim->fighting == ch )
        return FALSE;

    if ( IS_SET( ch->in_scene->scene_flags, SCENE_SAFE ) )
    return TRUE;

    if ( IS_SET( victim->in_scene->scene_flags, SCENE_SAFE ) )
    return TRUE;

    return FALSE;
}



/*
 * Set position of a victim.
 */
void update_pos( PLAYER *victim )
{
    if ( victim->hit > 0 )
    {
	if ( victim->position <= POS_STUNNED ) victim->position = POS_STANDING;
	return;
    }

	victim->hit = 0;
	victim->position = POS_DEAD;
    return;
}



/*
 * Start fights.
 */
void set_fighting( PLAYER *ch, PLAYER *victim )
{
    INSTANCE *pTrig;

    if ( ch->fighting != NULL )
    {
//        bug( "Set_fighting: already fighting", 0 );
        return;
    }

    cmd_draw( ch, "" );

    /*
     * Halt non-combat and non-spell scripts temporarily.
     * Can be triggered immediately after this.
     */
    for ( pTrig = ch->instances; pTrig != NULL; pTrig = pTrig->next )
    {
        if ( pTrig->script->type != TRIG_COMBAT
          && pTrig->script->type != TRIG_SPELL )
        {
            VARIABLE *pVar, *pVar_next;

            for ( pVar = pTrig->locals;  pVar != NULL; pVar = pVar_next )
            {
                pVar_next = pVar->next;
                free_variable( pVar );
            }

            pTrig->locals = NULL;
            pTrig->location = NULL;
        }
    }

    if ( IS_AFFECTED(ch, BONUS_SLEEP) )
        bonus_strip( ch, skill_dbkey(skill_lookup("sleep")) );

    if ( ch->furniture ) set_furn( ch, NULL );
    ch->fighting = victim;
    ch->position = POS_FIGHTING;
    ch->fbits     = 0;
    return;
}



/*
 * Stop fights.
 */
void stop_fighting( PLAYER *ch, bool fBoth )
{
    PLAYER *fch;

    for ( fch = actor_list; fch != NULL; fch = fch->next )
    {
        if ( fch == ch || ( fBoth && fch->fighting == ch ) )
        {
            if ( NPC(fch) )
            {
                /* restart born scripts
                script_update( fch, TYPE_ACTOR, TRIG_BORN, NULL, NULL, NULL, NULL );
                */
                if ( fBoth )
                {
                    if ( number_fighting(fch) < 2 )
                    fch->wait = 0;
                    cmd_sheath( fch, "" );
                }
            }

            fch->fighting  = NULL;
            fch->position  = POS_STANDING;
            fch->fbits     = 0;
            update_pos( fch );
        }
    }

    return;
}


/*
 * Make a corpse out of character.
 */
void make_corpse( PLAYER *ch )
{
    char buf[MAX_STRING_LENGTH];
    PROP *corpse;
    PROP *prop;
    PROP *prop_next;
    char *name;

    if ( NPC(ch) )
    {
        name          = NAME(ch);
        corpse        = create_prop(get_prop_template(PROP_VNUM_CORPSE_NPC), 0);
        corpse->timer = number_range( 20, 30 ) * 5;

    snprintf( buf, MAX_STRING_LENGTH, "corpse of %s", name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    snprintf( buf, MAX_STRING_LENGTH, "A corpse of %s lies here.\n\r", name );
    free_string( corpse->description );
    corpse->description = str_dup( buf );

    snprintf( buf, MAX_STRING_LENGTH, "corpses of %s", name );
    free_string( corpse->short_descr_plural );
    corpse->short_descr_plural = str_dup( buf );

    snprintf( buf, MAX_STRING_LENGTH, "%%s corpses of %s are laying in a heap.\n\r", name );
    free_string( corpse->description_plural );
    corpse->description_plural = str_dup( buf );

    }
    else
    {
        name            = NAME(ch);
        corpse          = create_prop(get_prop_template(PROP_VNUM_CORPSE_PC), 0);

    snprintf( buf, MAX_STRING_LENGTH, "%s's corpse", name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    snprintf( buf, MAX_STRING_LENGTH, "The marred body of %s lies here.\n\r", name );
    free_string( corpse->description );
    corpse->description = str_dup( buf );

    snprintf( buf, MAX_STRING_LENGTH, "%s's corpses", NAME(ch) );
    free_string( corpse->short_descr_plural );
    corpse->short_descr_plural = str_dup( buf );

    snprintf( buf, MAX_STRING_LENGTH, "%%s bodies of %s are laying in a heap.\n\t", name );
    free_string( corpse->description_plural );
    corpse->description_plural = str_dup( buf );
    }

    for ( prop = ch->carrying; prop != NULL; prop = prop_next )
    {
        prop_next = prop->next_content;
        prop_from_actor( prop );
        if ( IS_SET( prop->extra_flags, ITEM_INVENTORY ) )
        prop->item_type = ITEM_DELETE;
        else
        if ( IS_SET( prop->extra_flags, ITEM_BLESS ) && !NPC(ch) )
         prop_to_actor( prop, ch );
         else prop_to_prop( prop, corpse );
    }

    prop_to_scene( corpse, ch->in_scene );
    return;
}



/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( PLAYER *ch )
{
    SCENE *was_in_scene;
    char *msg;
    int door;

    display_interp( ch, "^B" );
    msg = "You hear $n's death cry.";
    switch ( number_range( 0, 12 ) )
    {
    case  0: msg  = "$n slumps onto the ground, dead.";         break;
    case  1: msg  = "$n collapses as the life flows from $m.";  break;
    case  2: msg  = "And then, suddenly, $n is dead.";          break;
    case  3: msg  = "The last ounce of life seeps from $n.";    break;
    case  4: msg  = "You fancy hearing the gods coming to claim $n's soul."; break;
    case  5:
        {
            PROP *prop = ch->carrying;
            char buf[MAX_STRING_LENGTH];

            msg  = "As $n dies, you notice the blood everywhere.";
            if ( prop != NULL && prop->item_type != ITEM_MONEY )
            {
            snprintf( buf, MAX_STRING_LENGTH, "a bloodied %s",
                          smash_article(STR(prop, short_descr)) );
            free_string( prop->short_descr );
            prop->short_descr = str_dup( buf );
            }
        }
     break;
    }

    act( msg, ch, NULL, NULL, TO_SCENE );

    if ( NPC(ch) )
    msg = "You hear the wail of something dying.";
    else
    msg = "You hear a shriek as someone dies.";

    was_in_scene = ch->in_scene;
    for ( door = 0; door < MAX_DIR; door++ )
    {
        EXIT *pexit;

        if ( ( pexit = was_in_scene->exit[door] ) != NULL
        &&   pexit->to_scene != NULL
        &&   pexit->to_scene != was_in_scene )
        {
            ch->in_scene = pexit->to_scene;
            act( msg, ch, NULL, NULL, TO_SCENE );
        }
    }
    ch->in_scene = was_in_scene;

    display_interp( ch, "^N" );
    return;
}



void raw_kill( PLAYER *victim )
{
    int i;
    PROP *pProp;

    stop_fighting( victim, TRUE );
    death_cry( victim );

    /*
     * Drop weapons and shields.
     */

    if ( (pProp = get_eq_char(victim, WEAR_SHIELD)) != NULL ) {
    prop_from_actor( pProp );
    prop_to_scene( pProp, victim->in_scene );
    }

    if ( (pProp = get_eq_char(victim, WEAR_WIELD_1 )) != NULL ) {
    prop_from_actor( pProp );
    prop_to_scene( pProp, victim->in_scene );
    }

    if ( (pProp = get_eq_char(victim, WEAR_WIELD_2 )) != NULL ) {
    prop_from_actor( pProp );
    prop_to_scene( pProp, victim->in_scene );
    }

    if ( (pProp = get_eq_char(victim, WEAR_HOLD_1 )) != NULL ) {
    prop_from_actor( pProp );
    prop_to_scene( pProp, victim->in_scene );
    }

    if ( (pProp = get_eq_char(victim, WEAR_HOLD_2 )) != NULL ) {
    prop_from_actor( pProp );
    prop_to_scene( pProp, victim->in_scene );
    }

    if ( victim->riding != NULL )
    {
        act( "As $n dies, you fall to the ground.",
             victim, NULL, victim->riding, TO_VICT );
        victim->riding->position   = POS_RESTING;
    }

    if ( victim->riding != NULL )
    act( "$n falls off you, dead.", victim, NULL, victim->riding, TO_VICT );
     
    dismount_char( victim );

    if ( NPC(victim) )
    {
        if ( !IS_SET(victim->flag,ACTOR_NOCORPSE) ) make_corpse( victim );
        extractor_char( victim, TRUE );
        return;
    }

    make_corpse( victim );
    extractor_char( victim, FALSE );
    actor_from_scene( victim );

  /* If rather new.. */
    {
    int p = PC(victim,played);
    int ct = (int) (current_time - PC(victim,logon));

    i = (p + ct)/7200;
    }

    if ( get_scene( PC(victim,death) ) == NULL ) PC(victim,death) = 
SCENE_VNUM_DEATH;

    if ( ( i < 10 ) )
    {
        int race;

        while ( victim->bonus )
        bonus_remove( victim, victim->bonus );

        race = race_lookup( victim->race );
        victim->bonuses = RACE(race,bonus_bits);            
        actor_to_scene( victim, get_scene( RACE(race,start_scene) ) );

        victim->armor       = 100;
        victim->position    = POS_RESTING;
        victim->hit         = UMAX( 1, victim->hit );
        victim->move        = UMAX( 1, victim->move );
        PC(victim,condition)[COND_DRUNK]   = 0;
        PC(victim,condition)[COND_FULL]    = 20;
        PC(victim,condition)[COND_THIRST]  = 20;
        cmd_restore( victim, "me" );
/*        to_actor( "For the first ten hours, you are given another 
chance.\n\r", victim ); */
    }
    else {
    actor_to_scene( victim, get_scene( PC(victim,death) ) );
    victim->position = POS_RESTING;
        victim->hit=1; victim->move=1; victim->armor=100; 
        PC(victim,condition)[COND_DRUNK]   = 0;
        PC(victim,condition)[COND_FULL]    = 20;
        PC(victim,condition)[COND_THIRST]  = 20;
    }

    save_actor_prop( victim );

    return;
}

/*
 * Syntax: kill [person]
 */
void cmd_kill( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PLAYER *victim;

    if ( NPC(ch)
      && IS_SET(ch->flag, ACTOR_PET) )  /* NOT */
       return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        to_actor( "Kill whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_actor_scene( ch, arg ) ) == NULL )
    {
        to_actor( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        to_actor( "You hit yourself.\n\r", ch );
        oroc( ch, ch );
        return;
    }

    if ( is_safe( ch, victim ) )
    return;

    if ( NPC(victim) && IS_SET(victim->flag, ACTOR_GOOD) )
    {
        act( "You don't want to fight $N.", ch, NULL, victim, TO_ACTOR );
        return;
    }

    if ( IS_AFFECTED(ch, BONUS_CHARM) && ch->master == victim )
    {
        act( "$N is your beloved master.", ch, NULL, victim, TO_ACTOR );
        return;
    }

    if ( ch->position == POS_FIGHTING )
    {
        to_actor( "You do the best you can!\n\r", ch );
        return;
    }

    cmd_draw( ch, "" );
    cmd_fight( ch, "" );
    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    oroc( ch, victim  );

    if ( ch->fighting != NULL ) {
      PLAYER *pGroup;
      for ( pGroup=ch->in_scene->people; pGroup!=NULL; pGroup=pGroup->next_in_scene ) 
       if ( !NPC(pGroup) && !IS_IMMORTAL(pGroup) && in_group( ch, pGroup ) ) cmd_assist( pGroup, "" );
    }
    return;
}

/*
 * Syntax: fight [fightmode]
 */
void cmd_fight( PLAYER *ch, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];
    int fmode;

    if ( argument[0] != '\0' )
    {
	one_argument( argument, arg );
	if ( ( fmode = fight_mode_lookup( arg ) ) < 0 )
	{
	    to_actor( "That is not a valid combat tactic.\n\r", ch );
	    return;
	}
	ch->fmode = fmode;
    }

    if ( NPC(ch) ) return;

    sprintf( arg, "Your combat tactic is %s.\n\r",
	fight_mode_table[ch->fmode].name );
    to_actor( arg, ch );
    return;
}


/*
 * Syntax: throw
 *         throw [direction|target]
 *         throw [direction] <target>
 */
void cmd_throw( PLAYER *ch, char *argument )
{
	to_actor( "Not yet implemented.\n\r", ch );
	return;
}


/*
 * Syntax: shoot
 *         shoot [direction]
 *         shoot [direction] <target>
 */
void cmd_shoot( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char w[ MAX_STRING_LENGTH ];
    PLAYER *victim;
    PROP_TEMPLATE *pPropIndex;
    PROP *prop, *prop2;

/*
 * Needs to have dual wield support
 */

    if ( ( prop = get_item_held( ch, ITEM_RANGED_WEAPON ) ) == NULL )
    {
    to_actor( "You need to wield a ranged weapon.\n\r", ch );
    return;
    }

    if ( prop->value[3] == 0 ) cmd_reload( ch, "" );
    if ( prop->value[3] == 0 )
       return;

    pPropIndex = get_prop_template( prop->value[0] );

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' )
    {
    to_actor( "Shoot who (where)?\n\r", ch );
    return;
    }

    if ( ( victim = get_actor_scene( ch, arg ) ) == NULL )
    {
        int dir = get_dir( arg );
        SCENE *pScene;
        SCENE *rch = ch->in_scene;

        /*
         * Shooting in a direction.
         */

        if ( ch->fighting != NULL ) {
             to_actor( "You cannot aim that far while fighting.\n\r", ch );
             return;
        }

        if ( rch->exit[dir] == NULL ) {
           to_actor( "That direction is blocked.\n\r", ch );
           return;
        }

        /*
         * Shooting too far (or accidentally misfiring).
         */
        if ( !skill_check( ch, skill_lookup( "ranged" ), 0 ) ) 
        pScene = rch->exit[dir]->to_scene;
        else {
             if ( rch->exit[dir]->to_scene && 
                  rch->exit[dir]->to_scene->exit[dir] )
             pScene = rch->exit[dir]->to_scene->exit[dir]->to_scene;
             else
             pScene = rch;
             }

        /*
         * Watch the setting of ch->in_scene here.
         */
        if ( !pScene ) {
           to_actor( "Shoot where?\n\r", ch );
           return;
        }

        actor_from_scene( ch );
        actor_to_scene(ch, pScene);
        if ( ( victim = get_actor_scene( ch, arg2 ) ) == NULL )
        {
            if ( pScene->exit[dir] && pScene->exit[dir]->to_scene )
            {
            actor_from_scene( ch );
            actor_to_scene( ch, pScene->exit[dir]->to_scene );
            }

            if ( ( victim = get_actor_scene( ch, arg2 ) ) == NULL 
		&& pScene && !NPC(ch) && !skill_check( ch, skill_lookup( "ranged" ), 0 ) )
            {

                /*
                 * Find random target.
                 */
                int count = 0;

                for ( victim = pScene->people;  victim != NULL; victim = victim->next_in_scene )
                count++;

                count = number_range( 0, count );
                for ( victim = pScene->people; 
                      victim != NULL && --count > 0;
                      victim = victim->next_in_scene )
                  ;
            } else { victim=ch; }
        }


    if ( victim == ch )
    {
    to_actor( "You misfire.\n\r", ch );
    if ( ch->in_scene != rch ) {
    actor_from_scene( ch );
    actor_to_scene( ch, rch );
    }
    return;
    }

    if ( victim != NULL && is_safe( ch, victim ) )
    {
      to_actor( "It is impossible to concentrate on fighting here.n\r", ch );
    if ( ch->in_scene != rch ) {
    actor_from_scene( ch );
    actor_to_scene( ch, rch );
    }
      return;
    }

    if ( victim != NULL &&  victim->fighting == ch )
    {
    if ( ch->in_scene != rch ) {
    actor_from_scene( ch );
    actor_to_scene( ch, rch );
    }
    to_actor( "It is impossible to shoot someone in melee with you!\n\r", ch );
    return;
    }
    if ( NPC(ch) && victim == NULL ) {
    if ( ch->in_scene != rch ) {
    actor_from_scene( ch );
    actor_to_scene( ch, rch );
    }
    return; 
}


    /*
     * The weapon fires..
     */

if (!NPC(ch))    prop->value[3]--;                  /* ammo decrease */
    {
    SKILL *pSkill = skill_lookup( "ranged" );
    WAIT_STATE( ch, pSkill ? pSkill->delay : 0 );
    }

    if ( victim == NULL || rch == NULL )
    {
    to_actor( "You miss.\n\r", ch );
    if ( pScene && pPropIndex ) 
	prop_to_scene( create_prop( pPropIndex, 0 ), pScene );
    if ( ch->in_scene != rch ) {
       actor_from_scene( ch );
       actor_to_scene( ch, rch );
    }
    return;
    }

    to_actor( STR(prop,action_descr), ch );


    pPropIndex = get_prop_template( prop->value[0] );
    if (pPropIndex == NULL) pPropIndex = prop->pIndexData;

    sprintf( w, "%s", pPropIndex->action_descr );

    if ( !IS_AWAKE(victim)
      || (NPC(ch) && NPC(victim))
      || skill_check( ch, skill_lookup( "ranged" ), 0 ) )
    {
	int deflected, dam;
    /*
     * Shield check.
     */
    if ( (prop2 = get_eq_char( victim, ITEM_ARMOR )) != NULL 
       && prop2->wear_loc == ITEM_WEAR_SHIELD
       && skill_check( victim, skill_lookup( "parry" ), 0 ) ) {
             act( "$N parries your attack.", ch, NULL, victim, TO_ACTOR );
             act( "You parry $t with your $T.", victim, w, 
                          smash_article(STR(prop2,short_descr)), TO_ACTOR );
             act( "$n blocks $t with $T.", victim, w, 
                          STR(prop2,short_descr), TO_SCENE );
          }
else {

	deflected = armor_deflect( victim, prop->value[3] );
	dam = number_range( UMIN(prop->value[1],prop->value[2]),
                            UMAX(prop->value[1],prop->value[2])  )
                + pPropIndex->value[1];

	if ( deflected == 0 ) deflected = 100;

    act( "$4$t $Bstrikes$R$4 $N!$R", ch, capitalize(w), victim, TO_SCENE );
    act( "$4You fire $p $Bstriking$R$4 $N!$R", ch, w, victim, TO_ACTOR );
    act( "$4You are struck by $p!$R", victim, w, ch, TO_ACTOR );
	dam = ((dam * deflected) / 100) * 2;
	damage( ch, victim, dam );
}
    }
    else
    {
    act( "$4$t misses $N completely.$R", ch, capitalize(w), victim, TO_ALL );
	damage( ch, victim, 0 );
    }
        if ( ch->in_scene != rch )
        {
        /*
         * Mobs hit may approach there attacker.
         */
        if ( ch->fighting && NPC(ch->fighting) && number_range( 0, 2 ) == 1)
           ch->fighting->tracking = str_dup( STR(ch,name) );

        stop_fighting( ch, TRUE );
        actor_from_scene( ch );
        actor_to_scene( ch, rch );
        act( "$n aims $p at $N and fires.", ch, prop, victim, TO_SCENE );
        }
        return;
    }

    if ( victim == ch )
    {
    to_actor( "You misfire.\n\r", ch );
    return;
    }

    if ( victim != NULL && is_safe( ch, victim ) )
    {
      to_actor( "It is impossible to concentrate on fighting here.n\r", ch );
      return;
    }

    if ( victim != NULL && victim->fighting == ch )
    {
    to_actor( "It is impossible to shoot someone in melee with you!\n\r", ch );
    return;
    }

    pPropIndex = get_prop_template( prop->value[0] );
    if (pPropIndex == NULL) pPropIndex = prop->pIndexData;

    /*
     * The weapon fires..
     */
    if ( NPC(ch) && victim == NULL ) {
     return; }

    prop->value[3]--;                            /* ammo decrease */
    to_actor( STR(prop,action_descr), ch );

    { SKILL *pSkill=skill_lookup( "ranged" );
    WAIT_STATE( ch, pSkill ? pSkill->delay : 0 );
    }

    if ( victim == NULL )
    {
    to_actor( "You miss and lose a round.\n\r", ch );
    return;
    }

    sprintf( w, "%s", pPropIndex->action_descr );

    if ( !IS_AWAKE(victim)
      || (NPC(ch) && NPC(victim))
      || skill_check( ch, skill_lookup( "ranged" ), 0 ) )
    {
	int deflected, dam;
    /*
     * Shield check.
     */
    if ( (prop2 = get_eq_char( victim, ITEM_ARMOR )) != NULL 
         && prop2->wear_loc == ITEM_WEAR_SHIELD
         && skill_check( victim, skill_lookup( "parry" ), 0 ) ) {
             act( "$N parries your attack.", ch, NULL, victim, TO_ACTOR );
             act( "You parry $t with your $T.", victim, w, 
                          smash_article(STR(prop2,short_descr)), TO_ACTOR );
             act( "$n blocks $t with $T.", victim, w, 
                          STR(prop2,short_descr), TO_SCENE );
          }

else {
	deflected = armor_deflect( victim, prop->value[3] );
	dam = number_range( UMIN(prop->value[1],prop->value[2]),
                            UMAX(prop->value[1],prop->value[2])  )
                + pPropIndex->value[1];

	if ( deflected == 0 ) deflected = 100;

    act( "$4$n fires $t $Bstriking$R$4 $N!$R", ch, w, victim, TO_SCENE );
    act( "$4You fire $t $Bstriking$R$4 $N!", ch, w, victim, TO_ACTOR );
    act( "$4You are struck by $t.", victim, w, NULL, TO_ACTOR );
	dam = ((dam * deflected) / 100) * 2;
	damage( ch, victim, dam );
}
    }
    else
    {
    act( "$4$t misses $N completely.$R", ch, capitalize(w), victim, TO_ALL );
	damage( ch, victim, 0 );
    }

    return;
}


void cmd_reload( PLAYER *ch, char *argument )
{
    PROP *prop;
    PROP *ammo, *cont;
    char buf[MAX_STRING_LENGTH];

    if ( ( prop = get_item_held( ch, ITEM_RANGED_WEAPON ) ) == NULL )
    {
    to_actor( "You need to wield a ranged weapon.\n\r", ch );
    return;
    }

    if ( prop->value[3] > 0 )
    {
        act( "$p is already loaded.\n\r", ch, prop, NULL, TO_ACTOR );
        return;
    }

    for ( ammo = ch->carrying;  ammo != NULL;  ammo = ammo->next_content )
        if ( ammo->pIndexData->dbkey == prop->value[0] ) break;

    if ( ammo == NULL )
    {

    for ( cont = ch->carrying;  cont != NULL;  cont = cont->next_content )
    {
        if ( cont->item_type == ITEM_CONTAINER )
        {
           for ( ammo = cont->contains;  ammo != NULL; ammo = ammo->next_content )
               if ( ammo->pIndexData->dbkey == prop->value[0] ) break;
           if ( ammo != NULL ) break;
        }
    }

    if ( ammo != NULL ) prop_from_prop( ammo );

    }
    else prop_from_actor( ammo );


    if ( ammo == NULL ) {
       to_actor( "You are out of ammunition.\n\r", ch );
       return;
    }

    snprintf( buf, MAX_STRING_LENGTH, "You ready your %s.\n\r", smash_article(STR(prop,short_descr)) );
    to_actor( buf, ch );
    prop->value[3] = ammo->value[0];
    extractor_prop( ammo );
    return;
}

/*
 * Syntax:  backstab [person]
 */
void cmd_backstab( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char w[ MAX_STRING_LENGTH ];
    PLAYER *victim;
    PROP *prop;
    int dt;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
    to_actor( "Backstab whom?\n\r", ch );
    return;
    }

    if ( ( victim = get_actor_scene( ch, arg ) ) == NULL )
    {
    to_actor( "They aren't here.\n\r", ch );
    return;
    }

    if ( victim == ch )
    {
    to_actor( "How can you sneak up on yourself?\n\r", ch );
    return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if ( ( prop = get_item_held( ch, ITEM_WEAPON ) ) == NULL )
    {
    to_actor( "You need to wield a weapon.\n\r", ch );
    return;
    }

    dt = prop->value[3];

    if ( attack_table[dt].hit_type != TYPE_PIERCE )
    {
    to_actor( "You need to wield a piercing weapon.\n\r", ch );
    return;
    }

    if ( victim->fighting != NULL )
    {
    to_actor( "It is impossible to backstab a fighting person.\n\r", ch );
    return;
    }

    if ( (victim->size - 4) > ch->size )
    {
	act( "It is impossible to reach $N's back!", ch, NULL, victim, TO_ACTOR );
	return;
    }
    if ( (victim->size + 4) < ch->size )
    {
	act( "$N is too small for you to backstab.", ch, NULL, victim, TO_ACTOR );
	return;
    }

    if ( victim->hit < MAXHIT(victim)/2 && victim->position > POS_SLEEPING )
    {
    act( "$N is too alert with pain to sneak up.", ch, NULL, victim, TO_ACTOR );
    return;
    }

    { SKILL *pSkill=skill_lookup( "backstab" );
    WAIT_STATE( ch, pSkill ? pSkill->delay : 0 );
    }
    sprintf( w, "%s", smash_article(STR(prop,short_descr)) );

    if ( !IS_AWAKE(victim)
      || (NPC(ch) && NPC(victim))
      || skill_check( ch, skill_lookup( "backstab" ), 0 ) )
    {
	int deflected, dam;

	deflected = armor_deflect( victim, prop->value[3] );
	dam = number_range( UMIN(prop->value[1],prop->value[2]),
                            UMAX(prop->value[1],prop->value[2])  );

	if ( deflected == 0 ) deflected = 100;

	/* double damage from backstabbing? */
	act( "$n thrust$v $t into $N back!", ch, w, victim, TO_ALL );
	dam = ((dam * deflected) / 100) * 2;
	damage( ch, victim, dam );
    }
    else
    {
	act( "$n tr$x to thrust $t into $N back!", ch, w, victim, TO_ALL );
	damage( ch, victim, 0 );
    }

    return;
}


void cmd_sla( PLAYER *ch, char *argument )
{
    to_actor( "If you want to SLAY, spell it out.\n\r", ch );
    return;
}



/*
 * Syntax: slay [person]
 */
void cmd_slay( PLAYER *ch, char *argument )
{
    PLAYER *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        to_actor( "Slay whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_actor_scene( ch, arg ) ) == NULL )
    {
        to_actor( "They aren't here.\n\r", ch );
        return;
    }

    if ( ch == victim )
    {
        to_actor( "Suicide is a mortal sin.\n\r", ch );
        return;
    }

    if ( get_trust(victim) > get_trust(ch) )
    {
        to_actor( "You failed.\n\r", ch );
        return;
    }

    script_update( victim, TYPE_ACTOR, TRIG_DIES, ch, NULL, NULL, NULL );
    act( "You grasp $S head and squeeze it until it explodes!",
                                            ch, NULL, victim, TO_ACTOR    );
    act( "$n grasps your head and squeezes until it explodes!",
                                            ch, NULL, victim, TO_VICT    );
    act( "$n grasps $N's head and squeezes until it explodes!",
                                            ch, NULL, victim, TO_NOTVICT );
    raw_kill( victim );
    return;
}



void retreat( PLAYER *ch )
{
    SKILL *pSkill = skill_lookup( "flee" );
    PLAYER *vch;
    int chance = 0;

    if ( ch->fighting == NULL )
	return;

    chance += get_curr_dex(ch);
    chance -= get_curr_dex(ch->fighting) * 3 / 2;

    if ( skill_check( ch, pSkill, 50 + chance ) )
    {
	WAIT_STATE( ch, pSkill ? pSkill->delay : 0 );
	for( vch = ch->in_scene->people; vch; vch = vch->next_in_scene )
	{
	    if ( vch->fighting == ch->fighting && vch != ch )
	    {
		act( "$n retreat$v from combat!", ch, NULL, NULL, TO_ALL );
		stop_fighting(ch,TRUE);
		act( "$N direct$v $S attack at $n!", vch, NULL, vch->fighting, TO_ALL );
		set_fighting(vch->fighting,vch);
		return;
	    }
	}
    }
    flee( ch );
    return;
}

/*
 * Syntax: retreat
 */
void cmd_retreat( PLAYER *ch, char *argument )
{
    if ( ch->fighting == NULL )
    {
	to_actor( "Calm down, you're not fighting!\n\r", ch );
	return;
    }
    if ( LEARNED(ch, "flee") <= 0 )
    {
	to_actor( "If you want out of combat, FLEE!\n\r", ch );
	return;
    }
    retreat(ch);
}

/*
 * Do flee checks.
 */
bool flee_check( PLAYER *ch, int d )
{
    PLAYER *fch, *fch_next;
    int penalty = 0, fdex, chdex, chstr, fstr;


    chdex = number_range( 0, get_curr_dex(ch) );
    chstr = number_range( 0, get_curr_str(ch) );

    for( fch = ch->in_scene->people; fch != NULL; fch = fch_next )
    {
	/* Careful here, our victim may die while fleeing... */
	fch_next = fch->next_in_scene;
	if ( fch->fighting != ch || fch->wait > 0 ) continue;
	fdex  = number_range( 0, get_curr_dex(fch) );
	fstr  = number_range( 0, get_curr_str(fch) );

	if ( chdex < fdex - penalty )
	{
	    act( "$B$4You attack $N as $E turns to run!$R", fch, NULL, ch, 
TO_ACTOR );
	    act( "$B$4$n attacks you as you turn to run!$R", fch, NULL, 
ch, 
TO_VICT );
	    act( "$B$4$n attacks $N as $e turns to run!$R", fch, NULL, ch, 
TO_NOTVICT );
	    fch->fbits = 0;
	    oroc( fch, ch );
	    /* If the poor bugger died... */
	    if ( ch->position < POS_STANDING )
		return( FALSE );
	}
	else
	if ( chstr < fstr - penalty
	  || skill_check( ch, skill_lookup( "flee" ), 0 ) )
        {
            act( "$B$N block$V $n as $e tries to escape $t!$R",
                 ch, dir_name[d], fch, TO_ALL );
	    return( FALSE );
	}
        penalty++;
    }
    /* Whew... got away! */
    return( TRUE );
}

/*
 * Flee from combat
 */
bool flee( PLAYER *ch )
{
    SCENE *was_in;
    int tried[ MAX_DIR ];
    int attempt;


    for ( attempt = 0; attempt < MAX_DIR; attempt++ ) tried[attempt] = FALSE;
    act( "$n panic$v and tr$x to flee!\n\r", ch, NULL, NULL, TO_ALL );
    was_in = ch->in_scene;
    for ( attempt = 0; attempt < MAX_DIR; attempt++ )
    {
        EXIT *pexit;
        int door;

        door = number_door();

	if ( tried[door]
        ||   ( pexit = was_in->exit[door] ) == NULL
	||   pexit->to_scene == NULL
	||   IS_SET(pexit->exit_flags, EXIT_CLOSED)
	||   IS_SET(pexit->exit_flags, EXIT_WINDOW)
	|| ( NPC(ch)
	  && IS_SET(pexit->to_scene->scene_flags, SCENE_NO_ACTOR) ) )
		continue;

        tried[door] = TRUE;
	if ( flee_check( ch, door ) ) 
	{
		stop_fighting( ch, TRUE );
		ch->position = POS_FIGHTING;
		move_char( ch, door );
		ch->position = POS_STANDING;
	}

    WAIT_STATE( ch, 1 );

	if ( ch->in_scene != was_in ) return( TRUE );
    }

    return( FALSE );
}



/*
 * Syntax:  flee
 */
void cmd_flee( PLAYER *ch, char *argument )
{

/*
    if ( ch->fighting == NULL )
    {
	to_actor( "Calm down, you're not fighting!\n\r", ch );
	return;
    }

    if ( LEARNED(ch, "flee") <= 0 )
    {
    to_actor( "It is impossible to to do that.\n\r", ch );
	return;
    }
 */

    flee( ch );    
    return;
}


/*
 * Syntax:  rescue [person]
 */
void cmd_rescue( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PLAYER *victim;
    PLAYER *fch;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        to_actor( "Rescue whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_actor_scene( ch, arg ) ) == NULL )
    {
        to_actor( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        to_actor( "What about fleeing instead?\n\r", ch );
        return;
    }

    if ( !NPC(ch) && NPC(victim) )
    {
        to_actor( "Doesn't need your help!\n\r", ch );
        return;
    }

    if ( ch->fighting == victim )
    {
        to_actor( "Too late.\n\r", ch );
        return;
    }

    if ( ( fch = victim->fighting ) == NULL )
    {
        to_actor( "That person is not fighting right now.\n\r", ch );
        return;
    }

    {
      SKILL *pSkill = skill_lookup( "rescue" );
    WAIT_STATE( ch, pSkill ? pSkill->delay : 0 );
    if ( skill_check( ch, pSkill, 0 ) )
    {
        to_actor( "You fail the rescue.\n\r", ch );
        return;
    }
    }

    act( "You rescue $N!",  ch, NULL, victim, TO_ACTOR    );
    act( "$n steps between you and $t!", ch, NAME(victim->fighting), victim, TO_VICT );
    act( "$n steps between $N and $t!", ch, NAME(victim->fighting), victim, TO_NOTVICT );

    stop_fighting( fch, FALSE );
    stop_fighting( ch, FALSE );
    stop_fighting( victim, FALSE );

    set_fighting( ch, fch );
    set_fighting( fch, ch );
    return;
}


bool hit_suck_disarm( PLAYER *ch, PLAYER *victim, int hit, int dam )
{
    PROP *prop;

    if ( skill_check( ch, skill_lookup( "disarm" ), 0 )
    && ( prop = get_item_held( victim, ITEM_WEAPON ) ) != NULL
    && hand_empty( ch ) != WEAR_NONE )
    {
        prop->wear_loc = hand_empty( ch );
        prop_from_actor( prop );
        prop_to_actor( prop, ch );
        act( "You pull $N's weapon right out of $s hand!",  ch, NULL, victim,  TO_ACTOR );
        act( "$n pulls your weapon right out of your hand!", ch, NULL, victim, TO_VICT );
        act( "$n pulls $N's weapon right out of $s hand!",   ch, NULL, victim, TO_NOTVICT );

        return TRUE;
    }

    return FALSE;
}



bool hit_vorpal( PLAYER *ch, PLAYER *victim, int hit, int dam )
{
    char buf[MAX_STRING_LENGTH];
    PROP *prop;
    char *name;

    if ( hit >= 17 )
    {
        act( "You slice $N's head clean off!",  ch, 0, victim, TO_ACTOR );
        act( "$n slices your head clean off!",  ch, 0, victim, TO_VICT );
        act( "$n slices $N's head clean off!",  ch, 0, victim, TO_NOTVICT );
        act( "$n's severed head plops on the ground.", victim, 0, 0, TO_SCENE );
        stop_fighting( victim, TRUE );

        name        = NAME(ch);
        prop         = create_prop( get_prop_template( PROP_VNUM_SEVERED_HEAD ), 0 );
        prop->timer  = number_range( 4, 7 );

        snprintf( buf, MAX_STRING_LENGTH, STR(prop, short_descr), NAME(victim) );
        free_string( prop->short_descr );
        prop->short_descr = str_dup( buf );

        snprintf( buf, MAX_STRING_LENGTH, STR(prop, description), NAME(victim) );
        free_string( prop->description );
        prop->description = str_dup( buf );

        prop_to_scene( prop, ch->in_scene );

        raw_kill( victim );
        return TRUE;
    }

    return FALSE;
}

bool hit_fire( PLAYER *ch, PLAYER *victim, int hit, int dam )
{
        PROP *pItem;

        act( "$B$4You breath fire at $N!$R",  ch, 0, victim, TO_ACTOR );
        act( "$B$4$n breathes fire at you!$R",  ch, 0, victim, TO_VICT );
        act( "$B$4$n chars $N with fire!$R",  ch, 0, victim, TO_NOTVICT );
        damage( ch, victim, dam ); 

        /*
         * Random item ignites and has its timer set.
         */
        for ( pItem = victim->carrying;  pItem != NULL; pItem = pItem->next_content ) {
            if (number_range(0,5) == 3 ) {

                SET_BIT(pItem->extra_flags, ITEM_BURNING);
                pItem->timer = number_range(5,10);  /* sets decay */

                act( "$4$p catches fire!$R", ch, pItem, victim, TO_ALL );
		return TRUE;
	    }
	}
        return TRUE;
}

bool hit_kick( PLAYER *ch, PLAYER *victim, int hit, int dam )
{
        act( "$B$4You kick $N!$R",  ch, 0, victim, TO_ACTOR );
        act( "$B$4$n kick you!$R",  ch, 0, victim, TO_VICT );
        act( "$B$4$n kicks $N with force$R",  ch, 0, victim, TO_NOTVICT );
        damage( ch, victim, dam ); 
        return TRUE;
}

bool hit_cold( PLAYER *ch, PLAYER *victim, int hit, int dam )
{
        act( "$B$1You chill $N!$R",  ch, 0, victim, TO_ACTOR );
        act( "$B$1$n chills you!$R",  ch, 0, victim, TO_VICT );
        act( "$B$1$n chills $N with an unearthly touch!$R",  ch, 0, victim, TO_NOTVICT );
        damage( ch, victim, dam ); 
        return TRUE;
}

bool hit_lightning( PLAYER *ch, PLAYER *victim, int hit, int dam )
{
        act( "$I$1$BYou electrify $N!$R",  ch, 0, victim, TO_ACTOR );
        act( "$I$1$B$n electrifies you!$R",  ch, 0, victim, TO_VICT );
        act( "$I$1$B$n electrifies $N with a bolt of lightning!$R",  ch, 0, victim, TO_NOTVICT );
        damage( ch, victim, dam ); 
        return TRUE;
}

bool hit_rot( PLAYER *ch, PLAYER *victim, int hit, int dam )
{
        act( "$I$6You rot $N!$R",  ch, 0, victim, TO_ACTOR );
        act( "$I$6$n rots your flesh!$R",  ch, 0, victim, TO_VICT );
        act( "$I$6$n rots $N's flesh!$R",  ch, 0, victim, TO_NOTVICT );
        damage( ch, victim, dam ); 
        return TRUE;
}

bool hit_acid( PLAYER *ch, PLAYER *victim, int hit, int dam )
{
        act( "$B$6You burn $N!$R",  ch, 0, victim, TO_ACTOR );
        act( "$B$6$n burns you!$R",  ch, 0, victim, TO_VICT );
        act( "$B$6$n burns $N with acid!$R",  ch, 0, victim, TO_NOTVICT );
        damage( ch, victim, dam ); 
        return TRUE;
}

bool hit_poison( PLAYER *ch, PLAYER *victim, int hit, int dam )
{
        BONUS af;

        act( "$B$2You bite $N!$R",  ch, 0, victim, TO_ACTOR );
        act( "$B$2$n bites you, releasing venom!$R",  ch, 0, victim, TO_VICT );
        act( "$B$2$n bites $N, releasing venom!$R",  ch, 0, victim, TO_NOTVICT );

            display_interp( ch, "^2" );
            display_interp( victim, "^2" );
            act( "$n chokes and gags.", victim, NULL, NULL, TO_SCENE );
            to_actor( "You choke and gag.\n\r", victim );
            display_interp( ch, "^N" );
            display_interp( victim, "^N" );

            af.type      = skill_dbkey(skill_lookup("poison"));
            af.duration  = 5;
            af.location  = APPLY_CON;
            af.modifier  = -1;
            af.bitvector = BONUS_POISON;
            bonus_join( victim, &af );

        damage( ch, victim, dam ); 
        return TRUE;
}

/*
 * Kick someone
 */
bool kick( PLAYER *ch, PLAYER *victim, bool fArmed )
{
    int chance = 0;

    if ( ch->fighting == NULL )
	return( FALSE );

    if (IS_AFFECTED(victim,BONUS_FLYING) || IS_AFFECTED(ch,BONUS_FLYING))
	return ( FALSE );

    chance  = fArmed ? 0 : 20;
    chance -= victim->size - ch->size;

    /* stats */
    chance += get_curr_dex(ch);
    chance -= get_curr_str(victim) * 4 / 3;

    if ( skill_check( ch, skill_lookup( "kicks" ), 30+chance ) )
    {
	char *i = NULL;
	int deflected, dam;

	deflected = armor_deflect( victim, 0 );
	dam = number_range( 2, UMAX(8,chance) );
        dam = damage_adjust( ch, dam, deflected );
	switch( number_range(0,4) )
	{
	case 0: i = "$n send$v a flying kick into $N!";
		break;
	case 1: i = "$n kick$v $N painfully!";
		break;
	case 2: i = "$n$y kick connects $N with an audible thud!";
		break;
	case 3: i = "$n$y swirl$v and kick$v $N with ferocity!";
		break;
	case 4: i = "$N $K surprised by $n$y sudden kick!";
		break;
        }
	act( i, ch, NULL, victim, TO_ALL );
	damage( ch, victim, dam );
	WAIT_STATE( victim, PULSE_VIOLENCE * 2 );
    }
    else
    {
	act( "$n tr$x to kick $N but miss$w!", ch, NULL, victim, TO_ALL );
        WAIT_STATE( ch, PULSE_VIOLENCE * 2 );
    }
    return ( TRUE );
}

/*
 * Disarm a creature.
 */
bool disarm( PLAYER *ch, PLAYER *victim )
{
    PROP *prop;


    if (victim->position < POS_FIGHTING)
	return ( FALSE );

    if ( ( prop = get_item_held( victim, ITEM_WEAPON ) ) == NULL )
        return ( FALSE );

    if ( get_item_held( ch, ITEM_WEAPON ) == NULL )
	return ( FALSE );

    if ( IS_SET( prop->extra_flags, ITEM_NOREMOVE ) && !IS_IMMORTAL(ch) )
    {
        act( "$B$n tries to disarm you, but $p won't come off your hand!$R",
             ch, prop, victim, TO_VICT );
        act( "You try to knock the weapon from $S hand, but it is useless!",
             ch, prop, victim,      TO_ACTOR );
        return ( TRUE );
    }

    {
    SKILL *pSkill=skill_lookup( "melee" );
    WAIT_STATE( ch, pSkill ? pSkill->delay : 0 );
    if ( skill_check( ch, pSkill, 70 - (get_curr_dex(victim) * 3) ) )
    {
	const char *i = NULL;
	switch( number_range(0,3) )
	{
	case 0: i = "$BWith a skillful twist, $n disarm$v $N!$R"; 
		break;
	case 1: i = "$B$n disarm$v $N and send$v $S weapon flying!$R";
		break;
	case 2: i = "$B$n feint$v and suddenly yank$v $N$Y weapon away!$R";
		break;
	case 3: i = "$B$n send$v $N$Y weapon flying, disarming $M!$R";
		break;
	}
	act( i,  ch, NULL, victim, TO_ALL );

	prop_from_actor( prop );
	prop_to_scene( prop, victim->in_scene );
    }
    else
    {
	act( "$n tr$x to disarm $N, but fail$v!", ch, NULL, victim, TO_ALL );
    }
    }
    return ( TRUE );
}


/*
 * Bash a creature.
 */
bool bash( PLAYER *ch, PLAYER *victim, bool fArmed )
{
    int chance = 0;

    if ( get_item_held(ch, ITEM_ARMOR) == NULL )
	return ( FALSE );

    if (victim->position < POS_FIGHTING)
	return ( FALSE );

    chance  = fArmed ? 0 : 10;

    chance += ch->carry_weight / 25;
    chance -= victim->carry_weight / 25;

    chance -= (victim->size - ch->size);

    /* stats */
    chance += get_curr_str(ch);
    chance -= get_curr_dex(victim) * 4 / 3;

    /* now the attack */
    if ( skill_check( ch, skill_lookup( "melee" ), 40 + chance ) )
    {
	const char *i = NULL;
	switch(number_range(0,3))
	{
	case 0: i = "$B$n send$v $N sprawling with a powerful bash!$R";
		break;
	case 1: i = "$B$n slam$v into $N, toppling $M off balance!$R";
		break;
	case 2: i = "$B$n$y forceful bash send$v $N sprawling!$R";
		break;
	case 3: i = "$BWith a vicious blow, $n send$v $N sprawling!$R";
		break;
	}
	act( i, ch, NULL, victim, TO_ALL );
	WAIT_STATE( victim, 2 * PULSE_VIOLENCE);
	victim->position = POS_RESTING;
	damage( ch, victim, number_range(2, UMAX(8,chance)) );	
    }
    else
    {
	act( "$n tri$x to bash $N, but fall$v flat on $s face!",
		ch, NULL, victim, TO_ALL );
	WAIT_STATE( ch,  2 * PULSE_VIOLENCE); 
	ch->position = POS_RESTING;
    }
    return ( TRUE );
}

/*
 * Grapple
 */
bool grapple( PLAYER *ch, PLAYER *victim )
{
    int chance = 0;

    if ( get_eq_char( ch, WEAR_HOLD_1 )
    ||   get_eq_char( ch, WEAR_HOLD_2 ) )
	return( FALSE );

    if (IS_AFFECTED(victim,BONUS_FLYING) || IS_AFFECTED(ch,BONUS_FLYING))
	return ( FALSE );

    chance -= victim->size - ch->size;

    chance += ch->carry_weight / 25;
    chance -= victim->carry_weight / 25;

    chance += get_curr_str(ch);
    chance -= get_curr_dex(victim) * 3 / 2;

    /* now the attack */
    if ( skill_check(ch, skill_lookup( "melee" ), 40 + chance) )
    {
	const char *i = NULL;
	switch(number_range(0,3))
	{
	case 0: i = "$B$n grab$v $N and throw$v $M to the ground!$R";
		break;
	case 1: i = "$B$n tackle$v $N, sending $M flat on $S face!$R";
		break;
	case 2: i = "$B$n seize$v a hold of $N and throw$v $M down!$R";
		break;
	case 3: i = "$B$n grasp$v $N and swing$v $M off balance!$R";
		break;
	}
	act( i, ch, NULL, victim, TO_ALL );
	WAIT_STATE(victim,3 * PULSE_VIOLENCE);
	victim->position = POS_RESTING;
	damage(ch,victim,number_range(2, 1 + 2 * victim->size / 20));
    }
    else
    {
	act( "$n tr$x to grapple $N, but fail$v.", ch, NULL, victim, TO_ALL );
	WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
    } 
    return ( TRUE );

}

/*
 * Trip
 */
bool trip( PLAYER *ch, PLAYER *victim, bool fArmed )
{
    int chance=0;

    if (IS_AFFECTED(victim,BONUS_FLYING) || IS_AFFECTED(ch,BONUS_FLYING))
	return ( FALSE );

    chance  = fArmed ? 0 : 20;
    chance -= victim->size - ch->size;

    /* dex */
    chance += get_curr_dex(ch);
    chance -= get_curr_dex(victim) * 3 / 2;

    /* now the attack */
    if ( skill_check(ch, skill_lookup( "melee" ), 30 + chance) )
    {
	const char *i = NULL;
	switch(number_range(0,3))
	{
	case 0: i = "$B$N feint$V, but $n trip$v $M off balance!$R";
		break;
	case 1: i = "$B$n trip$v $N, sending $M flat on $S face!$R";
		break;
	case 2: i = "$B$N lunge$V forward, but $n tackle$v $M down!$R";
		break;
	case 3: i = "$B$n anticipate$v $N$Y move and trip$v $M!$R";
		break;
	}
	act( i, ch, NULL, victim, TO_ALL );
	WAIT_STATE(victim,3 * PULSE_VIOLENCE);
	victim->position = POS_RESTING;
	damage(ch,victim,number_range(2, 1 + 2 * victim->size / 20));
    }
    else
    {
	act( "$n tr$x to trip $N, but fail$v.", ch, NULL, victim, TO_ALL );
    } 
    return ( TRUE );
}

/*
 * Sweep
 */
bool sweep( PLAYER *ch )
{
    PLAYER *vch, *vch_next;
    PROP *prop;
    char w[MAX_STRING_LENGTH];
    int count = 0;

    if ( ch->fighting == NULL )
	return ( FALSE );

    if ( ( prop = get_item_held( ch, ITEM_WEAPON ) ) == NULL )
	return ( FALSE );

    if ( attack_table[prop->value[3]].hit_type != TYPE_SLASH )
	return ( FALSE );


    sprintf( w, "%s", smash_article(STR(prop,short_descr)) );
    act( "$n swing$v $s $t around in a wide arc...", ch, w, NULL, TO_ALL);

    for ( vch = ch->in_scene->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_scene;
	if ( vch->fighting == ch )
	{
	    count++;
	    if ( skill_check(ch,skill_lookup("melee"),80/count) )
	       weapon_attack( ch, get_eq_char( ch, WEAR_WIELD_1 ), vch );
	}
    }
    if ( !count )
    {
	act( "$n$y sweep cuts only thin air!", ch, NULL, NULL, TO_ALL);
	count = number_range(1,3);
    }
    WAIT_STATE( ch, UMIN( 3, PULSE_VIOLENCE * count) ); 
    return ( TRUE );
}

int number_fighting( PLAYER *ch )
{
    PLAYER *vch;
    int count = 0;

    if ( ch->in_scene == NULL )
	return( 0 );
    for ( vch = ch->in_scene->people; vch; vch = vch->next_in_scene )
	if ( vch->fighting == ch ) count++;
    return( count );
}

 
/* This func tries to determine when a fighting party attempts to flee */
void auto_flee( PLAYER *ch, PLAYER *victim )
{
    int cond=0, wits = 0;

    /* We have no business here? */
    if ( victim->position != POS_FIGHTING
    ||   victim->wait > 0 )
	return;

    /* How smart am I? */
    cond = ch->fbits & combat_int[get_curr_int(victim)];
    wits = victim->fbits & combat_int[get_curr_int(victim)];

    /* Opponent stunned or I don't care if I've been hit */
    if ( !IS_SET(wits,FGT_HIT)
    ||    IS_SET(cond,FGT_STUNNED)
    ||   victim->fmode > fight_mode_lookup("normal") )
	return;


    /* Charmed folks won't go nowhere */
    if ( IS_AFFECTED(victim, BONUS_CHARM)
    &&   victim->master != NULL
    &&   victim->master->in_scene == victim->in_scene )
    {
	return;
    }

    /* Normal state of wimpiness */
    if ( !IS_SET( wits, FGT_WOUNDED )
    &&   victim->fmode > fight_mode_lookup("elusive")
    &&   victim->hit > MAXHIT(victim)/5 )
    {
       return;
    }
    else
    /* Cautious approach */
    if ( victim->fmode > fight_mode_lookup("cautious") 
    &&   victim->hit > MAXHIT(victim)/4 )
    {
	return;
    }

    /* Otherwise try to leave the scene */
    /* If they're outnumbered, try retreating gracefully */
    retreat( victim );
    return;
}

void auto_fight( PLAYER *ch, PLAYER *victim )
{
    int cond=0;
    int us=0, them=0;

    /* Not fighting, so bail out */
    if ( ch->position != POS_FIGHTING
    ||   ch->wait > 0 )
    {
	return;
    }

    /* Charmed folks are none too smart */
    if ( IS_AFFECTED(ch, BONUS_CHARM)
    &&   ch->master != NULL
    &&   ch->master->in_scene == ch->in_scene )
    {
	return;
    }

    /* If victim aren't able fight back, we'll pretend he's stunned */
    if ( victim->wait > 0 ) victim->fbits |= FGT_STUNNED;

    /* How smart we are? */
    cond = victim->fbits & combat_int[get_curr_int(ch)];

    /* Who's outnumbered? */
    us = number_fighting(victim);
    them = number_fighting(ch);

    /* If we're smart enough to know when to change tactic... */
    if ( IS_SET(combat_int[get_curr_int(ch)],FGT_CMP_HIT) )
    {

	/* If we're getting hurt and the opponent is not... */
    if ( (ch->hit < MAXHIT(ch)/3)
    &&  ((victim->hit > MAXHIT(victim)/2)
	  ||  us < them) )
	{
	    if ( ch->fmode > fight_mode_lookup("elusive") )
		--ch->fmode;
	    return;
	}
	else
	/* If the opponent is getting hurt and we're not... */
    if ( (ch->hit > MAXHIT(ch)/2)
    &&  ((victim->hit < MAXHIT(victim)/3)
	  ||  us > them) )
	{
	    if ( victim->fmode < fight_mode_lookup("berserk") )
		++ch->fmode;
	    return;
	}
    }

}

bool armed_attack( PLAYER *ch, PLAYER *victim )
{
    /* Self or victim not fighting, so bail out */
    if ( ch->position != POS_FIGHTING
    ||   ch->wait > 0 || victim->wait > 0 )
    {
	return( FALSE );
    }

    /* Attempt skill use */
    if ( number_bits(3) == 0 && LEARNED(ch,"melee") > number_percent() )
    {
	int i;
	i = number_range(0,4);
	switch ( i )
	{
	case 0:	/* BASH */
	    return( bash(ch, victim, TRUE) );
	case 1: /* KICK */
	    return( kick(ch, victim, TRUE) );
	case 2: /* SWEEP */
	    if ( number_fighting(ch) > 1 )
		return( sweep(ch) );
	    break;
	case 3: /* TRIP */
	    return( trip(ch, victim, TRUE) );
	case 4: /* DISARM */
	    return( disarm(ch, victim) );
	default: break;
	}
	return( TRUE );
    }
    return( FALSE );
}

bool unarmed_attack( PLAYER *ch, PLAYER *victim )
{
    /* Self or victim not fighting, so bail out */
    if ( ch->position != POS_FIGHTING
    ||   ch->wait > 0 || victim->wait > 0 )
    {
	return( FALSE );
    }

    if ( number_bits(3) == 0 && LEARNED(ch, "melee") > number_percent() )
    {
	
	switch ( number_range(0,3) )
	{
	case 0:	/* BASH */
	    return( bash(ch, victim, FALSE) );
	case 1: /* KICK */
	    return( kick(ch, victim, FALSE) );
	case 2: /* TRIP */
	    return( trip(ch, victim, FALSE) );
	case 3: /* GRAPPLE */
	    return( grapple(ch, victim) );
	default: break;
	}
	return( TRUE );
    }
    return( FALSE );
}



/*
 * Syntax:  assist [person]
 */
void cmd_assist( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PLAYER *victim;
    PLAYER *fch;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        if ( ch->master ) victim=ch->master;
        else
        for ( victim=ch->in_scene->people; 
              victim != NULL; victim=victim->next_in_scene )
        {
            if ( victim==ch ) continue;
            if ( in_group( ch, victim ) && victim->fighting != NULL )
             break;
        }
        if ( !victim ) { to_actor( "Assist whom?\n\r", ch ); return; }
    }
    else {
    if ( ( victim = get_actor_scene( ch, arg ) ) == NULL )
    {
        if ( ch->master ) victim=ch->master->fighting;
        if ( !victim ) {
        to_actor( "They aren't here.\n\r", ch );
        return;
        }
    }

    if ( victim == ch )
    {
        if ( ch->leader != NULL ) cmd_gtell( ch, "Please assist!" );
        return;
    }
    }

    if ( ch->in_scene != victim->in_scene ) {
        to_actor( "They aren't nearby.\n\r", ch );
        return; 
    }

    if ( ch->fighting == victim || ch->fighting != NULL )    {
        to_actor( "Too late.\n\r", ch );
        return;
    }

    if ( ( fch = victim->fighting ) == NULL )
    {
        to_actor( "That person is not fighting right now.\n\r", ch );
        return;
    }

    act( "You charge in to assist $N!",  ch, NULL, victim, TO_ACTOR );
    act( "$n assists you!", ch, NULL, victim, TO_VICT );
    act( "$n begins fighting alongside $N!", ch, NAME(victim->fighting), victim, TO_NOTVICT );

    set_fighting( ch, fch );
    return;
}




/*
 * Syntax:  mercy
 */
void cmd_mercy( PLAYER *ch, char *argument )
{
/*    char arg[MAX_INPUT_LENGTH];*/
    PLAYER *fch;

    if ( ch->fighting == NULL ) {
        to_actor( "You are not fighting right now.\n\r", ch );
        return;
    }

    if ( ch->fighting->fighting != ch ) {
        to_actor( "You cower in fear.", ch );
        act( "$n cowers in fear from $N.", ch, NULL, ch->fighting, TO_SCENE );
        stop_fighting( ch, TRUE );
        return;
    }

    fch = ch->fighting;

    act( "You beg for mercy $N!",  ch, NULL, fch, TO_ACTOR );
    act( "$n begs you for mercy!", ch, NULL, fch, TO_VICT );

    if ( NPC(ch->fighting) && IS_SET(ch->fighting->flag, ACTOR_MERCY) )
    {
        stop_fighting( ch, TRUE );
        stop_fighting( fch, TRUE );
        act( "$N spares $n.\n\r", ch, NULL, fch, TO_ALL );
    }

    return;
}




/*
 * Syntax:  stop
 */
void cmd_stop( PLAYER *ch, char *argument )
{
    PLAYER *fch;

    if ( ch->fighting == NULL ) {
        to_actor( "You are not fighting right now.\n\r", ch );
        return;
    }

    if ( !NPC(ch) && !NPC(ch->fighting))
    {
        fch = ch->fighting;
        stop_fighting( ch, TRUE );
        stop_fighting( fch, TRUE );
        act( "$N stops fighting $n.\n\r", ch, NULL, fch, TO_ALL );
    }

    return;
}

