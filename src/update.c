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

// historic tag:

/******************************************************************************
 * Locke's   __ -based on merc v5.0-____        NIM Server Software           *
 * ___ ___  (__)__    __ __   __ ___|  | v5.1a  Version 5.1a                  *
 * |  /   \  __|  \__/  |  | |  |      |        documentation release         *
 * |       ||  |        |  \_|  | ()   |        Valentines Day Massacre 2003  *
 * |    |  ||  |  |__|  |       |      |                                      *
 * |____|__|___|__|  |__|\___/__|______|        http://www.nimud.org/nimud    *
 *   n a m e l e s s i n c a r n a t e          dedicated to chris cool       *
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

#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#include <zlib.h>


#include "nimud.h"
#include "script.h"
#include "skills.h"
#include "defaults.h"
#include "net.h"
#include "board.h"

/* Default value for reboots = pulses per second x sec x min x hours */
int     num_hour              = PULSE_PER_SECOND * 60 * 60 * 4;

int     pulse_zone;
int     pulse_actor;
int     pulse_violence;
int     pulse_point;
int     autosave_counter;

int     uptime;

/*
 * Local functions.
 */
void    aggr_update     args( ( void ) );


void    save_config     args( ( void ) );


/*
 * From board.c
 */

void save_board( int b );

extern struct note_data     *note_list[MAX_BOARD];

     


/*
 * Regeneration stuff.
 */
int hit_gain( PLAYER_DATA *ch )
{
    int gain;

    if ( IS_NPC(ch) )
    {
    gain = get_curr_con(ch) * 3 / 2;
    }
    else
    {
    gain = UMIN( 5, get_curr_con(ch) ) * 2;

	switch ( ch->position )
	{
	case POS_SLEEPING: gain += get_curr_con(ch);		break;
	case POS_RESTING:  gain += get_curr_con(ch) / 2;	break;
	}

    if ( PC(ch,condition[COND_FULL])   <= 0 )
	    gain /= 2;

    if ( PC(ch,condition[COND_THIRST]) <= 0 )
	    gain /= 2;

    }

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    /*
     * Furniture Bonuses
     */
    if ( ch->furniture ) 
    gain += gain / 4;  

    return UMIN(gain, MAXHIT(ch) - ch->hit);
}


/*
 * Regeneration stuff.
 */
int mana_gain( PLAYER_DATA *ch )
{
    int gain;

    if ( IS_NPC(ch) )
    {
    gain = get_curr_wis(ch) * 3 / 2;
    }
    else
    {
    gain = UMIN( 5, get_curr_wis(ch) ) * 2;

	switch ( ch->position )
	{
	case POS_SLEEPING: gain += get_curr_wis(ch);break;
	case POS_RESTING:  gain += get_curr_wis(ch) / 2;break;
	}

    if ( PC(ch,condition[COND_FULL])   <= 0 )
	    gain /= 2;

    if ( PC(ch,condition[COND_THIRST]) <= 0 )
	    gain /= 2;

    }

    /*
     * Furniture Bonuses
     */
    if ( ch->furniture ) 
    gain += gain / 4;  

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    return UMIN(gain, MAXMANA(ch) - ch->mana);
}




int move_gain( PLAYER_DATA *ch )
{
    int gain;

    if ( IS_NPC(ch) )
    {
    gain = get_curr_con(ch);
    }
    else
    {
    gain = UMAX( 15, 2 * get_curr_con(ch) );

	switch ( ch->position )
	{
	case POS_SLEEPING: gain += get_curr_dex(ch);		break;
	case POS_RESTING:  gain += get_curr_dex(ch) / 2;	break;
	}

    if ( PC(ch,condition[COND_FULL])   <= 0 )
	    gain /= 2;

    if ( PC(ch,condition[COND_THIRST]) <= 0 )
	    gain /= 2;
    }

    /*
     * Furniture Bonuses
     */
    if ( ch->furniture ) 
    gain += gain / 4;  

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    return UMIN(gain, MAXMOVE(ch) - ch->move);
}



void gain_condition( PLAYER_DATA *ch, int iCond, int value )
{
    int condition;

    if ( IS_NPC(ch) || IS_HERO(ch) )
        return;

    /*
     * Nasty desert.
     * Nasty.
     */
    if ( iCond == COND_THIRST 
      && value < 0
      && ch->in_scene 
      && ch->in_scene->sector_type == SECT_DESERT )   value *= 2;

    condition               = PC(ch,condition[iCond]);
    PC(ch,condition[iCond]) = URANGE( -100, condition + value, 100 );

    if ( iCond == COND_DRUNK && PC(ch,condition[iCond]) < 0 )
        PC(ch,condition[iCond]) = 0;

    condition = PC(ch,condition[iCond]);

/*
    if ( PC(ch,condition[iCond]) < 10 )
    {
      switch ( iCond )
      {
      case COND_FULL:   send_to_actor( "You are hungry.\n\r",  ch );   break;
      case COND_THIRST: send_to_actor( "You are thirsty.\n\r", ch );   break;
      }
    }
*/

    if ( iCond == COND_DRUNK
      && condition == 5 )
        send_to_actor( "You sober up.\n\r", ch );

    if ( PC(ch,condition[iCond]) < 0 )
    {
      switch ( iCond )
      {
      case COND_FULL:
          if ( condition < -80 )
          send_to_actor( "You are dying of starvation; you require sustinence.\n\r", ch );
     else if ( condition < -60 )
          send_to_actor( "You begin to feel the effects of lack of food; you are weak and dizzy.\n\r", ch );
     else if ( condition < -40 )
          send_to_actor( "You feel faint from lack of food, you are starving.\n\r",  ch );
     else if ( condition < -20 )
          send_to_actor( "Your stomach is growling, you should really eat something.\n\r", ch );

          if ( PC(ch,condition[iCond]) <= -100
            && ch->position > POS_SLEEPING )
          {
              send_to_actor(  "You collapse due to lack of food...", ch );
              act( "$n collapses!", ch, NULL, NULL, TO_SCENE );
              ch->position = POS_SLEEPING;
          }
          break;

      case COND_THIRST:
          if ( condition < -80 )
          send_to_actor( "You are dying of dehydration; you need fluid.\n\r", ch );
     else if ( condition < -60 )
          send_to_actor( "Having gone without liquid, you are now feeling weak and dehydrated.\n\r", ch );
     else if ( condition < -40 )
          send_to_actor( "You feel faint and sickly, you are dehydrating.\n\r",  ch );
     else if ( condition < -20 )
          send_to_actor( "You are really thirsty and your lips are quite parched.\n\r", ch );

          if ( PC(ch,condition[iCond]) <= -100 )
          {
              send_to_actor(  "You are dying of dehydration!", ch );
              act( "$n are dying of dehydration!", ch, NULL, NULL, TO_SCENE );
              ch->hit = ch->hit - ch->hit/4;
              if ( ch->hit <= 0 ) ch->hit = 1; 
          }
          break;
      }
    }

    return;
}


/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL NiMUD cpu time.
 * -- Furey
 */
void actor_update( void )
{
    PLAYER_DATA *ch;
    PLAYER_DATA *ch_next;
    EXIT_DATA *pexit;
    int door;

    /* Examine all actors. */
    for ( ch = actor_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

    if ( ch->in_scene == NULL )
        continue;

    /* Add drowning here... */

    if ( !IS_NPC(ch) )
        continue;

    if ( ch->timer > 0 )
    {
        if ( --ch->timer == 0 )
        {
        act( "$n has left.", ch, NULL, NULL, TO_SCENE );
        act( "Your time has run out.", ch, NULL, NULL, TO_ACTOR );
        raw_kill( ch );
        continue;
        }
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
	    continue;

	/* Examine call for special proc */
	/* That's all for sleeping / busy monster */
	if ( ch->position != POS_STANDING )
	    continue;

	/* Wander */
	if ( !IS_SET(ch->act, ACT_SENTINEL)
    && ( door = number_bits( MAX_DIR ) ) < MAX_DIR
	&& ( pexit = ch->in_scene->exit[door] ) != NULL
	&&   pexit->to_scene != NULL
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->to_scene->scene_flags, SCENE_NO_ACTOR)
	&& ( !IS_SET(ch->act, ACT_STAY_ZONE)
	||   pexit->to_scene->zone == ch->in_scene->zone ) )
	{
	    move_char( ch, door );
	    if ( ch->position < POS_STANDING )
	        continue;
	}

	/* Flee */
    if ( ch->hit < MAXHIT(ch) / 2
    && ( door = number_bits( MAX_DIR ) ) < MAX_DIR
	&& ( pexit = ch->in_scene->exit[door] ) != NULL
	&&   pexit->to_scene != NULL
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->to_scene->scene_flags, SCENE_NO_ACTOR) )
	{
	    PLAYER_DATA *rch;
	    bool found;

	    found = FALSE;
	    for ( rch  = pexit->to_scene->people;
		  rch != NULL;
		  rch  = rch->next_in_scene )
	    {
		if ( !IS_NPC(rch) )
		{
		    found = TRUE;
		    break;
		}
	    }
	    if ( !found )
        move_char( ch, door );
	}

    }

    return;
}



/*
 * Update the time table.
 */
void time_update( void )
{
    char buf[MAX_STRING_LENGTH];
    int rstr = number_range( 1, 3 );

    switch ( ++weather_info.hour )
    {
    case  4:
        if ( weather_info.sunlight != MOON_RISE )
        break;

        weather_info.sunlight = MOON_SET;
        weather_info.temperature -= number_fuzzy( 5 );

        if ( rstr == 1 )
        strcat( buf, "The moon sets.\n\r" );
        else
        if ( rstr == 2 )
        strcat( buf, "The moon disappears beneath the horizon.\n\r" );
        else
        strcat( buf, "The sky grows dark as the moon sets.\n\r" );
       break;

    case  5:
        weather_info.sunlight = SUN_LIGHT;

        if ( rstr == 1 )
        strcat( buf, "Dawn blushes the sky with hues of orange and magenta.\n\r" );
        else
        if ( rstr == 2 )
        strcat( buf, "The day has begun.\n\r" );
        else
        strcat( buf, "It is dawn.\n\r" );

        if ( weather_info.month == 0
          || weather_info.month == 1
          || weather_info.month == 11 )
        weather_info.temperature = number_fuzzy( 20 );   /* Winter */
        else
        if ( weather_info.month == 2
          || weather_info.month == 3
          || weather_info.month == 4 )
        weather_info.temperature = number_fuzzy( 50 );   /* Spring */
        else
        if ( weather_info.month == 5
          || weather_info.month == 6
          || weather_info.month == 7 )
        weather_info.temperature = number_fuzzy( 80 );   /* Summer */
        else
        weather_info.temperature = number_fuzzy( 45 );   /* Autumn */
       break;

    case  6:
        weather_info.sunlight = SUN_RISE;
        weather_info.temperature += number_fuzzy( 15 );

        if ( rstr == 1 )
        strcat( buf, "The first shafts of sunlight streak along their western path.\n\r" );
        else
        if ( rstr == 2 )
        strcat( buf, "The sun rises in the east.\n\r" );
        else
        strcat( buf, "The sun illuminates the lands.\n\r" );
       break;

    case  12:
        weather_info.temperature += number_fuzzy( 20 );

        if ( rstr == 1 )
        strcat( buf, "It is noon.\n\r" );
        else
        if ( rstr == 2 )
        strcat( buf, "The sun reaches its apex in the sky.\n\r" );
        else
        strcat( buf, "Sunlight beats down upon you from directly above.\n\r" );
       break;

    case 19:
        weather_info.sunlight = SUN_SET;
        weather_info.temperature -= number_fuzzy( 20 );

        if ( rstr == 1 )
        strcat( buf, "The sun slowly disappears in the west.\n\r" );
        else
        if ( rstr == 2 )
        strcat( buf, "The sun dips below the western horizon.\n\r" );
        else
        strcat( buf, "The sun sets with a glorious twilight.\n\r" );
       break;

    case 20:
        weather_info.sunlight = SUN_DARK;
        weather_info.temperature -= number_fuzzy( 10 );

        if ( rstr == 1 )
        strcat( buf, "The night has begun.\n\r" );
        else
        if ( rstr == 2 )
        strcat( buf, "The last bit of twilight fades away to blackness.\n\r" );
        else
        strcat( buf, "The darkness of night engulfs the realm.\n\r" );
       break;

    case 24:
        if ( weather_info.moon_phase != MOON_NEW )
        weather_info.sunlight = MOON_RISE;
        weather_info.temperature -= number_fuzzy( 10 );
        weather_info.hour = 0;
        weather_info.day++;              /* New Day */
        save_config( );


        if ( ++weather_info.next_phase % 3 == 0 )
        {
            weather_info.moon_phase++;
            if ( weather_info.moon_phase >= MOON_MAX )
            {
                weather_info.moon_phase = MOON_NEW;
                weather_info.next_phase = 0;
            }
        }

        if ( weather_info.moon_phase == MOON_NEW )
        strcat( buf, "A black disc, devoid of stars, is the only moon on this night.\n\r" );
   else if ( weather_info.moon_phase == MOON_WAXING_CRESCENT )
        strcat( buf, "The short sliver of moon offers its jagged edge to the night sky.\n\r" );
   else if ( weather_info.moon_phase == MOON_WAXING_HALF )
        strcat( buf, "The waxing half moon rises for its night journey across the heavens.\n\r" );
   else if ( weather_info.moon_phase == MOON_WAXING_THREE_QUARTERS )
        strcat( buf, "The waxing three quarter moon rises.\n\r" );
   else if ( weather_info.moon_phase == MOON_FULL ) {
        strcat( buf, "The full moon rises, casting a silver glow over the night.\n\r" );
        change_lycanthropes( TRUE );
        }
   else if ( weather_info.moon_phase == MOON_WANING_THREE_QUARTERS ) {
        change_lycanthropes( FALSE );
        strcat( buf, "The grey-silver waning three-quarter moon begins its nocturnal trek.\n\r" );
        }
   else if ( weather_info.moon_phase == MOON_WANING_HALF )
        strcat( buf, "Amidst an oceanic starry night sky, the waning half moon floats,\n\rits silvery ship adrift on celestial currents.\n\r" );
   else if ( weather_info.moon_phase == MOON_WANING_CRESCENT )
        strcat( buf, "A curved waning crescent moon graces the night with a few rays of moonlight.\n\r" );
       break;
    }

    if ( weather_info.day   >= 30 )     /* New Month */
    {
        weather_info.day = 0;
        weather_info.month++;
    }

    if ( weather_info.month >= 12 )     /* New Year  */
    {
        weather_info.month = 0;
        weather_info.year++;
    }
}



/*
 * Update the weather.
 * (mmhg=Millimeters of NiMUDury)
 */
void weather_update( void )
{
    char buf[MAX_STRING_LENGTH];
    CONNECTION_DATA *d;
    int diff;

    buf[0] = '\0';

    /*
     * Weather change.
     */
    if ( number_bits( 4 ) == 0 )
    {
        int olddir = weather_info.winddir;

        weather_info.winddir += number_range( 0, 2 )-1;
        if ( weather_info.winddir % 3 != olddir )
        strcat( buf, "The wind changes direction.\n\r" );
    }

    if ( weather_info.month >= 9 && weather_info.month <= 16 )
	diff = weather_info.mmhg >  985 ? -2 : 2;
    else
	diff = weather_info.mmhg > 1015 ? -2 : 2;

    weather_info.change   += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
    weather_info.change    = UMAX(weather_info.change, -12);
    weather_info.change    = UMIN(weather_info.change,  12);

    weather_info.mmhg += weather_info.change;
    weather_info.mmhg  = UMAX(weather_info.mmhg,  960);
    weather_info.mmhg  = UMIN(weather_info.mmhg, 1040);

    switch ( weather_info.sky )
    {
    default: 
        bug( "Weather_update: bad sky %d.", weather_info.sky );
        weather_info.sky = SKY_CLOUDLESS;
       break;

    case SKY_CLOUDLESS:
        if ( weather_info.mmhg <  990
        || ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
        {
            if ( weather_info.month < 2 || weather_info.month == 11 )
            {
                strcat( buf, "A few flakes of snow are falling.\n\r" );
                weather_info.temperature -= 15;
            }
            else
            {
                if ( weather_info.sunlight == MOON_RISE )
                strcat( buf, "The moon passes behind a cloud.\n\r" );
                else
                strcat( buf, "The sky is getting cloudy.\n\r" );
                weather_info.temperature -= 5;
            }

            weather_info.sky = SKY_CLOUDY;
            weather_info.windspeed += 10;
        }
       break;

    case SKY_CLOUDY:
        if ( weather_info.mmhg <  970
        || ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) )
        {
            if ( weather_info.month < 2 || weather_info.month == 11 )
            {
                strcat( buf, "It starts to snow.\n\r" );
                weather_info.temperature -= 15;
            }
            else
            {
                strcat( buf, "It starts to rain.\n\r" );
                weather_info.temperature -= 5;
            }

            weather_info.sky = SKY_RAINING;
            weather_info.windspeed += 10;
        }

        if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
        {
            if ( weather_info.month < 2 || weather_info.month == 11 )
            {
                strcat( buf, "The snow lets up.\n\r" );
                weather_info.temperature += 15;
            }
            else
            {
                if ( weather_info.sunlight == MOON_RISE )
                strcat( buf, "The clouds disappear and the moon again shines into the night.\n\r" );
                else
                strcat( buf, "The clouds disappear.\n\r" );
                weather_info.temperature += 5;
            }

            weather_info.sky = SKY_CLOUDLESS;
            weather_info.windspeed -= 10;
        }
       break;

    case SKY_RAINING:
        if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 )
        {
            if ( weather_info.month < 2 || weather_info.month == 11 )
            {
                strcat( buf, "You are caught in a blizzard.\n\r" );
                weather_info.temperature -= 30;
            }
            else
            strcat( buf, "Lightning flashes in the sky.\n\r" );

            weather_info.sky = SKY_LIGHTNING;
            weather_info.windspeed += 10;
        }

        if ( weather_info.mmhg > 1030
        || ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
        {
            if ( weather_info.month < 2 || weather_info.month == 11 )
            {
                strcat( buf, "The snow is letting up.\n\r" );
                weather_info.temperature += 30;
            }
            else
            strcat( buf, "The rain stopped.\n\r" );

            weather_info.sky = SKY_CLOUDY;
            weather_info.windspeed -= 10;
        }
       break;

    case SKY_LIGHTNING:
        if ( weather_info.mmhg > 1010
        || ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
        {
            if ( weather_info.month < 2 || weather_info.month == 11 )
            {
                strcat( buf, "The blizzard subsides.\n\r" );
                weather_info.temperature += 10;
            }
            else
            strcat( buf, "The lightning has stopped.\n\r" );

            weather_info.sky = SKY_RAINING;
            weather_info.windspeed -= 10;
            break;
        }
       break;
    }

    if ( buf[0] != '\0' )
    {
        for ( d = connection_list; d != NULL; d = d->next )
        {
            if ( d->connected == CON_PLAYING
              && IS_OUTSIDE(d->character)
              && IS_AWAKE(d->character) )
            send_to_actor( buf, d->character );
        }
    }

    return;
}



/*
 * Update all chars, including actors.
 * This function is performance sensitive.
 */
void everybody_update( bool fEach )
{
    PLAYER_DATA *ch;
    PLAYER_DATA *ch_next;
    PLAYER_DATA *ch_save;
    PLAYER_DATA *ch_quit;
    time_t save_time;

    save_time	= current_time;
    ch_save	= NULL;
    ch_quit	= NULL;
    for ( ch = actor_list; ch != NULL; ch = ch_next )
    {
	BONUS_DATA *paf;
	BONUS_DATA *paf_next;

	ch_next = ch->next;

	script_update( ch, TYPE_ACTOR, fEach ? TRIG_EACH_PULSE : TRIG_TICK_PULSE,
				   NULL, NULL, NULL, NULL );

    if ( !fEach )
    {
        if ( !IS_NPC(ch) )
        {
            SKILL_DATA *pSkill;

            for ( pSkill=ch->learned; pSkill != NULL; pSkill=pSkill->next )
            {
                 /* ignore groups */
                if ( pSkill->group_code == 0 ) continue;

                if ( pSkill->skill_time > 0 )
                     pSkill->skill_time--;

                /*
                 * Forget skills when unused too long...
                 *          -- Newt
                 */

                /* possible extension to following if statement:
                   --------------------------------------------------------
                &&   pSkill->forget_time > 0
                                                 ^^^^^^^^^^^
                    Add "forget_time" field to skill_table!
                   --------------------------------------------------------- */
 
                if ( pSkill->skill_level > 50
                   && number_range(0,1000) == 0
                   && !IS_IMMORTAL(ch) )
                {
                     act( "Your understanding of $t dulls a bit.", ch, 
                          pSkill->name, NULL, TO_ACTOR );
                     pSkill->skill_level--;
                     pSkill->last_used = 0;
                }
                else {
                    pSkill->last_used++;
                    if ( pSkill->skill_level > 0 
                      && pSkill->skill_level < pSkill->max_learn 
                      && number_range( 0, 1000 ) == 0 ) {
                      pSkill->skill_level++;
                      act( "Upon thinking further of $t your understanding improves.",
                           ch, pSkill->name, NULL, TO_ACTOR );
                    }
                }

            }
            continue;
        }

        /*
         * Approval system.
         */

	    if ( !IS_NPC( ch ) ) {
	    char buf[MAX_STRING_LENGTH];
	    if ( ch->userdata->app_time == 1 ) {
	        ch->userdata->app_time = 0;

	       /*
	        * Right here you might want to set this PLR_APPLIED
	        * to ch->userdata->level == LEVEL_APPLY; or whatever it was.
	        */
	        SET_BIT( ch->act, PLR_APPLIED );
	        STC( "An immortal has yet to review your character.  It will get reviewed "
	             "within the next few\n\rhours up to 24 hours.\n\r",ch);
	        free_string( PC(ch,denial) );
	        PC(ch,denial) = NULL;
	        cmd_quit(ch,"menu");
	    }

	    if ( ch->userdata->app_time > 1 )
	        ch->userdata->app_time--;
	
	    if ( ch->userdata->app_time == 2 ) {
	        sprintf(buf, "Your character has not been approved.  Please post a note on the "
	                     "(OOC) Public board.\n\rAn immortal will review your character "
	                     "shortly after.  You have 10 minutes left of game "
	                     "play before you are disconnected.\n\r");
	        STC(buf,ch);
	    }
	}


    }

    if ( !IS_IMMORTAL(ch) )
    {
        if ( !IS_AWAKE(ch) )
        {
            gain_condition( ch, COND_DRUNK, -number_fuzzy(8) );

            if ( number_range(0,1) )
            {
            gain_condition( ch, COND_FULL,  -1 );
            }
        if ( ch->position >= POS_STUNNED )
        if ( ch->hit  < MAXHIT(ch) )
	     ch->hit  += hit_gain(ch);

        }
        else
        {
            gain_condition( ch, COND_DRUNK,  -number_fuzzy(4) );
            if ( number_range(0,50) > 30 ) {
             gain_condition( ch, COND_FULL, -number_range(0,1) );
             gain_condition( ch, COND_THIRST, -number_range(0,1) );
            }
        if ( ch->position >= POS_STUNNED )
        if ( ch->hit  < MAXHIT(ch) )
	     ch->hit  += hit_gain(ch);

        }
    }

	/*
	 * Find dude with oldest save time.
	 */
	if ( !IS_NPC(ch)
    && ( ch->desc == NULL || !CONNECTED(ch->desc) )
    &&   PC(ch,level) >= 2
    &&   PC(ch,save_time) < save_time )
	{
	    ch_save	= ch;
        save_time   = PC(ch,save_time);
	}

	if ( ch->position >= POS_STUNNED )
	{
        if ( ch->move < MAXMOVE(ch) )
		ch->move += move_gain(ch);
        if ( ch->mana < MAXMANA(ch) )
		ch->mana += mana_gain(ch);
	}

	if ( ch->position == POS_STUNNED )
	    update_pos( ch );

    if ( !IS_NPC(ch) ) ch->timer++;
    if ( !IS_IMMORTAL(ch) )
	{
        if ( ch->timer > 15 )
		ch_quit = ch;
	}

	for ( paf = ch->bonus; paf != NULL; paf = paf_next )
	{
	    paf_next	= paf->next;
	    if ( paf->duration > 0 )
		paf->duration--;
	    else if ( paf->duration < 0 )
		;
	    else
	    {
		if ( paf_next == NULL
		||   paf_next->type != paf->type
		||   paf_next->duration > 0 )
		{
		    if ( paf->type > 0 && paf->msg_off )
		    {
			send_to_actor( paf->msg_off, ch );
			send_to_actor( "\n\r", ch );
		    }
		}
	  
		bonus_remove( ch, paf );
	    }
	}

	/*
	 * Careful with the damages here,
	 *   MUST NOT refer to ch after damage taken,
	 *   as it may be lethal damage (on NPC).
	 */
	if ( IS_AFFECTED(ch, AFF_POISON) )
	{
	    act( "$n shivers and suffers.", ch, NULL, NULL, TO_SCENE );
	    send_to_actor( "You shiver and suffer.\n\r", ch );
        damage( ch, ch, 2 );
	}
	else if ( ch->position == POS_INCAP )
	{
        damage( ch, ch, 1 );
	}
	else if ( ch->position == POS_MORTAL )
	{
        damage( ch, ch, 2 );
	}
    }

    /*
     * Autosave and autoquit.
     * Check that these chars still exist.
     * Added support for helpful tips.
     */
    if ( ch_save != NULL || ch_quit != NULL )
    {
	for ( ch = actor_list; ch != NULL; ch = ch_next )
	{
	    ch_next = ch->next;
        if ( ch == ch_save )        save_actor_prop( ch );
        if ( ch == ch_quit && !IS_SET(ch->act2,PLR_IDLE) )        cmd_quit( ch, "" ); 
        tip( ch ); /* see tips.c */
	}
    }

    return;
}



/*
 * Update all props.
 * Called once a tick.	This function is performance sensitive.
 */
void prop_update_pulse( void )
{   
    PROP_DATA *prop;
    PROP_DATA *prop_next;
    SCENE_INDEX_DATA *pScene;

    for ( prop = prop_list; prop != NULL; prop = prop_next )
    {
        PLAYER_DATA *rch;
        bool Vehicle = FALSE;

        prop_next = prop->next;

    /*
     * Purge 'freed list' of props for Eat(this);
     */

        if ( prop->item_type == ITEM_DELETE ) {
           if ( prop->in_scene   != NULL ) prop_from_scene( prop );
      else if ( prop->carried_by != NULL ) prop_from_actor( prop );
      else if ( prop->in_prop    != NULL ) prop_from_prop( prop );

           extract_prop( prop );
           continue;
        }


        script_update( prop, TYPE_PROP, TRIG_EACH_PULSE, NULL, NULL, NULL, NULL );

/* consistency check */
        if ( prop->carried_by == NULL ) prop->wear_loc = WEAR_NONE;
        
        if ( prop->item_type == ITEM_VEHICLE && prop->in_scene != NULL )
        {
            int i;

            for ( i = 0; i < 4; i++ )
            {
                if ( prop->in_scene->sector_type == prop->value[i] )
                {
                    Vehicle = TRUE;
                    break;
                }
            }
        }


		if ( (pScene = prop->in_scene) != NULL
		  && (pScene->sector_type == SECT_WATER_NOSWIM
		   || pScene->sector_type == SECT_WATER_SWIM
		   || pScene->sector_type == SECT_AIR
		   || (pScene->sector_type == SECT_UNDERWATER
			&& pScene->exit[DIR_DOWN] != NULL
			&& pScene->exit[DIR_DOWN]->to_scene != NULL)
		   || pScene->sector_type == SECT_CLIMB)
		  && !Vehicle )
		{
			EXIT_DATA *pExit;
			char buf[MAX_STRING_LENGTH];

			snprintf( buf, MAX_STRING_LENGTH, "$p %s",
						  pScene->sector_type == SECT_WATER_NOSWIM ||
						  pScene->sector_type == SECT_WATER_SWIM ?
							  "splashes into the water and disappears." :
						  pScene->sector_type == SECT_AIR ||
						  pScene->sector_type == SECT_CLIMB ?
							  "plummets down to the earth below." :
							  "floats down into the murky waters below." );

            if ( (rch = pScene->people) != NULL )
			{
			act( buf, rch, prop, NULL, TO_SCENE );
			act( buf, rch, prop, NULL, TO_ACTOR );
			}

			if ( (pExit = pScene->exit[DIR_DOWN]) != NULL
			 &&   pExit->to_scene != pScene
			 &&   pExit->to_scene != NULL )
			{
				prop_from_scene( prop );
				pScene = pExit->to_scene;
				snprintf( buf, MAX_STRING_LENGTH, "$p %s",
						  pScene->sector_type == SECT_WATER_NOSWIM ||
						  pScene->sector_type == SECT_WATER_SWIM ?
							  "washes in from above." :
						  pScene->sector_type == SECT_CLIMB ?
							  "plummets down from above." :
							  "floats down from above." );
				prop_to_scene( prop, pExit->to_scene );
				rch = pExit->to_scene->people;
				if ( rch != NULL )
				{
				act( buf, rch, prop, NULL, TO_SCENE );
				act( buf, rch, prop, NULL, TO_ACTOR );
				}
			}
			else
			{
				extract_prop( prop );
				continue;
			}

		}
    }

    return;
}



void prop_update_tick( void )
{   
    PROP_DATA *prop;
    PROP_DATA *prop_next;
    SCENE_INDEX_DATA *pScene;

    for ( prop = prop_list; prop != NULL; prop = prop_next )
    {
		PLAYER_DATA *rch;
        PLAYER_DATA *ch;
		char *msg;

        prop_next = prop->next;

        script_update( prop, TYPE_PROP, TRIG_TICK_PULSE, NULL, NULL, NULL, NULL );

/* consistency check */
        if ( prop->carried_by == NULL ) prop->wear_loc = WEAR_NONE;

        ch = prop->carried_by;
        pScene = ch != NULL ? ch->in_scene : prop->in_scene;
    
        if ( prop->item_type == ITEM_LIGHT && IS_LIT(prop) )
        {
        if ( --prop->value[0] == 0 && pScene != NULL )
		{
            --pScene->light;
            if ( ch != NULL )
            {
			act( "$p goes out.", ch, prop, NULL, TO_SCENE );
            act( "$p that you are carrying goes out.", ch, prop, NULL, TO_ACTOR );
            }
            else if ( prop->in_scene != NULL
            &&      ( rch = prop->in_scene->people ) != NULL )
            {
            act( "$p goes out.", rch, prop, NULL, TO_SCENE );
            }
            REMOVE_BIT(prop->value[3], LIGHT_LIT);
            if (!IS_SET(prop->value[3], LIGHT_FILLABLE))    extract_prop( prop );
		}
        else
        if ( ch != NULL )
        {
            if (prop->value[0] > 0)
            {
                switch ( PERCENTAGE( prop->value[0], prop->value[1] )/10 )
                {
                    case 0:
                    act( "$p carried by $n flickers and sputters.", ch, prop, NULL, TO_SCENE );
                    act( "$p flickers and sputters.", ch, prop, NULL, TO_ACTOR );
                    break;
                    case 1:
                    act( "$p carried by $n flickers.", ch, prop, NULL, TO_SCENE );
                    act( "$p flickers.", ch, prop, NULL, TO_ACTOR );
                    break;
                    case 2:
                    act( "$p carried by $n flickers slightly.", ch, prop, NULL, TO_SCENE );
                    act( "$p flickers slightly.", ch, prop, NULL, TO_ACTOR );
                    break;
                }
            }
        }
		}

        if ( prop->timer <= 0 
          || --prop->timer > 0 
          || IS_SET(prop->extra_flags, ITEM_INVENTORY) )
        continue;

	if ( prop->timer == 0 && IS_SET(prop->extra_flags,ITEM_BURNING) ) 
	{ REMOVE_BIT(prop->extra_flags,ITEM_BURNING);
	  SET_BIT(prop->extra_flags,ITEM_BURNT);
	if ( number_range(3,5) == 4 ) { act( "$P is destroyed by fire.", 
ch, NULL, prop, TO_ACTOR );  extract_prop(prop );  continue;} 
  	  act( "$p smolders and is left charred by the flames that once engulfed it.", ch, prop, NULL, TO_ACTOR );
	  prop->timer = -1;
	  continue;
	}

		switch ( prop->item_type )
		{
		default:			  msg = "$p vanishes.";                      break;
		case ITEM_POTION:	  msg = "$p fades away.";                    break;
		case ITEM_WAND: 	  msg = "$p fizzles, pops and implodes.";    break;
		case ITEM_STAFF:	  msg = "$p folds up into oblivion.";        break;
		case ITEM_SCROLL:	  msg = "$p becomes unreadible.";            break;
		case ITEM_ARMOR:	  msg = "$p falls apart.";                   break;
		case ITEM_GEM:	  msg = "$p shatters into thousands of shards!"; break;
		case ITEM_PILL: 	  msg = "$p crumbles into dust.";            break;
		case ITEM_FOUNTAIN:   msg = "$p dries up.";                      break;
		case ITEM_CORPSE_NPC: msg = "$p rots away to nothing.";          break;
		case ITEM_CORPSE_PC:  msg = "$p decays into dust.";              break;
		case ITEM_FOOD: 	  msg = "$p goes bad.";                      break;
		case ITEM_DRINK_CON:  msg = "$p disappears in a puff of smoke."; break;
		}

		if ( prop->carried_by != NULL )
   	        act( msg, prop->carried_by, prop, NULL, TO_ACTOR );
		else
		if ( prop->in_scene != NULL
		  && ( rch = prop->in_scene->people ) != NULL )
		{
            act( msg, rch, prop, NULL, TO_SCENE );
            act( msg, rch, prop, NULL, TO_ACTOR );
		}

		extract_prop( prop );
    }

    return;
}



/*
 * Aggress.
 * Called once per pulse;
 *
 * for each mortal PC
 *     for each actor in scene
 *         aggress on some random PC
 * also take care of scramblers
 *
 * This function takes 25% to 35% of ALL NiMUD cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the actor to just attack the first PC
 *   who leads the party into the scene.
 *
 * -- Furey
 */
void aggr_update( void )
{
    PLAYER_DATA *wch;
    PLAYER_DATA *wch_next;
    PLAYER_DATA *ch;
    PLAYER_DATA *ch_next;
    PLAYER_DATA *vch;
    PLAYER_DATA *vch_next;
    PLAYER_DATA *victim;

    for ( wch = actor_list; wch != NULL; wch = wch_next )
    {
	wch_next = wch->next;

    script_update( wch, TYPE_ACTOR, TRIG_EACH_PULSE, NULL, NULL, NULL, NULL );

    if ( IS_NPC(wch)
    ||   IS_IMMORTAL(wch)
    ||   wch->in_scene == NULL )
        continue;

	for ( ch = wch->in_scene->people; ch != NULL; ch = ch_next )
	{
	    int count;

	    ch_next	= ch->next_in_scene;

	    if ( !IS_NPC(ch)
            ||  ( !IS_SET(ch->act, ACT_AGGRESSIVE) )
	    ||   ch->fighting != NULL
	    ||   IS_AFFECTED(ch, AFF_CHARM)
	    ||   !IS_AWAKE(ch)
	    ||   ( IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
	    ||   !can_see( ch, wch ) 
/* watch this */  || number_range(0, 5) < 5 )
		continue;

	    /*
	     * Ok we have a 'wch' player character and a 'ch' npc aggressor.
	     * Now make the aggressor fight a RANDOM pc victim in the scene,
	     *   giving each 'vch' an equal chance of selection.
	     */
	    count	= 0;
	    victim	= NULL;
	    for ( vch = wch->in_scene->people; vch != NULL; vch = vch_next )
	    {
		vch_next = vch->next_in_scene;

		if ( !IS_NPC(vch)
                &&   !IS_IMMORTAL(vch)
		&&   ( !IS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch) )
		&&   can_see( ch, vch ) )
		{
		    if ( number_range( 0, count ) == 0 )
			victim = vch;
		    count++;
		}
	    }

	    if ( victim == NULL )
	    {
		bug( "Aggr_update: null victim.", count );
		continue;
	    }

        oroc( ch, victim );
	}
    }

    return;
}





/*
 * Spawn one scene.
 */
bool spawn_scene( SCENE_INDEX_DATA *pScene )
{
    SPAWN_DATA *pSpawn;                /* Current Spawn                   */
    PLAYER_DATA *pActor;               /* Current Working Actor           */
    PROP_DATA *pProp;                  /* Current Working Object          */
    PLAYER_DATA    *LastMob;           /* Last Mob Created                */
    PROP_DATA     *LastObj;            /* Last Object Created             */
    int iExit;                         /* For exit resets                 */
    bool fOldBootDb = fBootDb;

    fBootDb = FALSE;                   /* Skirt get_xxx_index() exit()s   */

    if ( pScene == NULL ) return FALSE;
    if ( get_scene_index( pScene->vnum ) == NULL )
    {
          bug( "Spawn_scene: invalid scene %d", pScene->vnum);
          if ( fOldBootDb ) exit( 1 );
          return FALSE;
    }

    pActor    = NULL;
    LastMob = NULL;
    LastObj = NULL;
    
    for ( iExit = 0;  iExit < MAX_DIR;  iExit++ )
    {
       EXIT_DATA *pExit;
       EXIT_DATA *tExit;
       
       pExit = pScene->exit[iExit];
       if ( pExit == NULL ) continue;
       tExit = pExit->to_scene != NULL ? pExit->to_scene->exit[rev_dir[iExit]]
                                      : NULL;

       if ( pScene->people != NULL
         && !IS_SET(pExit->exit_info, EX_CONCEALED) )
       {
         if ( IS_SET(pExit->exit_info, EX_WINDOW) )
         {
         if ( !IS_SET(pExit->exit_info, EX_CLOSED)
            && IS_SET(pExit->rs_flags, EX_CLOSED) )
         {
         act( "The $t shuts.", pScene->people, pExit->keyword, NULL, TO_ACTOR );
         act( "The $t shuts.", pScene->people, pExit->keyword, NULL, TO_SCENE );
         }
         }
         else
         {
         if ( !IS_SET(pExit->exit_info, EX_CLOSED)
            && IS_SET(pExit->rs_flags, EX_CLOSED) )
         {
         act( "The $t closes.", pScene->people, pExit->keyword, NULL, TO_ACTOR );
         act( "The $t closes.", pScene->people, pExit->keyword, NULL, TO_SCENE );
         }
         }

         if ( !IS_SET(pExit->exit_info, EX_LOCKED)
            && IS_SET(pExit->rs_flags, EX_LOCKED) )
         {
         act( "The $t clicks.", pScene->people, pExit->keyword, NULL, TO_ACTOR );
         act( "The $t clicks.", pScene->people, pExit->keyword, NULL, TO_SCENE );
         }
       }

       pExit->exit_info = pExit->rs_flags;      /* set the reset flags   */
       if ( tExit != NULL )
        tExit->exit_info = tExit->rs_flags;     /* nail the other side */
    }

    for ( pSpawn = pScene->spawn_first; pSpawn != NULL; pSpawn = pSpawn->next )
    {
	ACTOR_INDEX_DATA *pActorIndex;
    PROP_INDEX_DATA *pPropIndex;
    int count;

    if ( pSpawn->num == -1 ) pSpawn->num=1;

    for ( count = 0; count < pSpawn->num; count++ )
    {
    if ( number_percent( ) > pSpawn->percent )
        continue;

	switch ( pSpawn->command )
	{
        default: bug( "Spawn_scene: bad command %c.", pSpawn->command );
	    break;

    case 'M':

        if ( ( pActorIndex = get_actor_index( pSpawn->rs_vnum ) ) == NULL )
        {
            bug( "Spawn_scene: 'M': bad vnum %d.", pSpawn->rs_vnum );
            if ( fOldBootDb ) exit( 1 );
            break;
        }

        if ( pActorIndex->count >= pSpawn->loc )
            break;

        pActor = create_actor( pActorIndex );

        actor_to_scene( pActor, pScene );
        script_update( pActor, TYPE_ACTOR, TRIG_BORN, NULL, NULL, NULL, NULL );

        LastObj = NULL;
        LastMob = pActor;
     break;

     case 'O':
        if ( ( pPropIndex = get_prop_index( pSpawn->rs_vnum ) ) == NULL )
        {
            bug( "Spawn_scene: 'O': bad vnum %d.", pSpawn->rs_vnum );
            if ( fOldBootDb ) exit( 1 );
            continue;
        }

        pProp = create_prop( pPropIndex, 0 );

        if ( pSpawn->loc == SPAWN_LOC_INSIDE && LastObj != NULL )
        {
            prop_to_prop( pProp, LastObj );
            script_update( pProp, TYPE_PROP, TRIG_BORN, NULL, NULL, NULL, NULL );
        }
   else if ( pSpawn->loc == SPAWN_LOC_ONTOP && LastObj != NULL )
        {
            prop_to_prop( pProp, LastObj );
            script_update( pProp, TYPE_PROP, TRIG_BORN, NULL, NULL, NULL, NULL );
        }
   else if ( pSpawn->loc == SPAWN_LOC_INSCENE )
        {
        if ( count_prop_list( pPropIndex, pScene->contents ) < pSpawn->num )
        {
            LastObj = pProp;
            prop_to_scene( pProp, pScene );
            script_update( pProp, TYPE_PROP, TRIG_BORN, NULL, NULL, NULL, NULL );
        }
        else count = pSpawn->num;
        }
   else if ( LastMob != NULL )
        {
            prop_to_actor( pProp, LastMob );
            pProp->wear_loc = URANGE( WEAR_NONE, pSpawn->loc, MAX_WEAR );

            /* For Shopkeepers */
            if ( LastMob->pIndexData->pShop != NULL
              && pProp->wear_loc == WEAR_NONE )
            {
                SET_BIT(pProp->extra_flags, ITEM_INVENTORY);
              /*  pProp->cost = pProp->pIndexData->cost; */
            }
            else
            script_update( pProp, TYPE_PROP, TRIG_BORN, NULL, NULL, NULL, NULL );
        }
        
        if ( pProp->carried_by == NULL
          && pProp->in_scene    == NULL
          && pProp->in_prop     == NULL )
        {
            extract_prop( pProp );
            pProp = NULL;
            continue;             
        }

        if ( pProp->item_type == ITEM_CONTAINER )
                LastObj = pProp;
        break;

	case 'G':  pPropIndex = get_prop_index( PROP_VNUM_TEMPLATE );
        if ( pSpawn->rs_vnum > MAX_GOODS-1 || pSpawn->rs_vnum < 0 )
        {
            bug( "Spawn_scene: 'G': bad good %d.", pSpawn->rs_vnum );
            if ( fOldBootDb ) exit( 1 );
            continue;
        }

        pProp = create_good( pSpawn->rs_vnum );

        if ( pSpawn->loc == SPAWN_LOC_INSIDE && LastObj != NULL )
        {
            prop_to_prop( pProp, LastObj );
            script_update( pProp, TYPE_PROP, TRIG_BORN, NULL, NULL, NULL, NULL );
        }
   else if ( pSpawn->loc == SPAWN_LOC_ONTOP && LastObj != NULL )
        {
            prop_to_prop( pProp, LastObj );
            script_update( pProp, TYPE_PROP, TRIG_BORN, NULL, NULL, NULL, NULL );
        }
   else if ( pSpawn->loc == SPAWN_LOC_INSCENE )
        {
        if ( count_prop_list( pPropIndex, pScene->contents ) < pSpawn->num )
        {
            LastObj = pProp;
            prop_to_scene( pProp, pScene );
            script_update( pProp, TYPE_PROP, TRIG_BORN, NULL, NULL, NULL, NULL );
        }
        else count = pSpawn->num;
        }
   else if ( LastMob != NULL )
        {
            prop_to_actor( pProp, LastMob );
            pProp->wear_loc = URANGE( WEAR_NONE, pSpawn->loc, MAX_WEAR );

            /* For Shopkeepers */
            if ( LastMob->pIndexData->pShop != NULL
              && pProp->wear_loc == WEAR_NONE )
            {
                SET_BIT(pProp->extra_flags, ITEM_INVENTORY);
              /*  pProp->cost = pProp->pIndexData->cost; */
            }
            else
            script_update( pProp, TYPE_PROP, TRIG_BORN, NULL, NULL, NULL, NULL );
        }
        
        if ( pProp->carried_by == NULL
          && pProp->in_scene    == NULL
          && pProp->in_prop     == NULL )
        {
            extract_prop( pProp );
            pProp = NULL;
            continue;             
        }

        break;

	case 'C': pPropIndex = get_prop_index( PROP_VNUM_TEMPLATE );
        if ( pSpawn->rs_vnum > MAX_COMPONENTS-1 || pSpawn->rs_vnum < 0 )
        {
            bug( "Spawn_scene: 'C': bad component %d.", pSpawn->rs_vnum );
            if ( fOldBootDb ) exit( 1 );
            continue;
        }

        pProp = create_comp( pSpawn->rs_vnum );

        if ( pSpawn->loc == SPAWN_LOC_INSIDE && LastObj != NULL )
        {
            prop_to_prop( pProp, LastObj );
            script_update( pProp, TYPE_PROP, TRIG_BORN, NULL, NULL, NULL, NULL );
        }
   else if ( pSpawn->loc == SPAWN_LOC_ONTOP && LastObj != NULL )
        {
            prop_to_prop( pProp, LastObj );
            script_update( pProp, TYPE_PROP, TRIG_BORN, NULL, NULL, NULL, NULL );
        }
   else if ( pSpawn->loc == SPAWN_LOC_INSCENE )
        {
        if ( count_prop_list( pPropIndex, pScene->contents ) < pSpawn->num )
        {
            LastObj = pProp;
            prop_to_scene( pProp, pScene );
            script_update( pProp, TYPE_PROP, TRIG_BORN, NULL, NULL, NULL, NULL );
        }
        else count = pSpawn->num;
        }
   else if ( LastMob != NULL )
        {
            prop_to_actor( pProp, LastMob );
            pProp->wear_loc = URANGE( WEAR_NONE, pSpawn->loc, MAX_WEAR );

            /* For Shopkeepers */
            if ( LastMob->pIndexData->pShop != NULL
              && pProp->wear_loc == WEAR_NONE )
            {
                SET_BIT(pProp->extra_flags, ITEM_INVENTORY);
              /*  pProp->cost = pProp->pIndexData->cost; */
            }
            else
            script_update( pProp, TYPE_PROP, TRIG_BORN, NULL, NULL, NULL, NULL );
        }
        
        if ( pProp->carried_by == NULL
          && pProp->in_scene    == NULL
          && pProp->in_prop     == NULL )
        {
            extract_prop( pProp );
            pProp = NULL;
            continue;             
        }

        break;
    }

    }


    }

    fBootDb = fOldBootDb;
    script_update( pScene, TYPE_SCENE, TRIG_TICK_PULSE, NULL, NULL, NULL, NULL );
    return TRUE;
}


/*
 * Repopulate scenes periodically (update 1/1024 of the scenes).
 */
void scene_update( void )
{
    static int iHash;
    SCENE_INDEX_DATA *pScene;

    if (iHash < 0) iHash = -1;
    if (++iHash >= MAX_KEY_HASH )
        iHash = 0;

    for (pScene = scene_index_hash[iHash]; pScene != NULL; pScene = pScene->next )
    {
        PLAYER_DATA *pch;

        if ( spawn_scene( pScene )
          && pScene->zone->repop != NULL
          && pScene->zone->repop[0] != '\0' )
        {
        for ( pch = pScene->people; pch != NULL; pch = pch->next_in_scene )
            send_to_actor( pScene->zone->repop, pch );
        }
    }

    return;
}


/*
 * Update all script parses mud-wide.
 */
void script_update_script( void )
{
    SCENE_INDEX_DATA *scene;
    PROP_DATA *prop;
    PLAYER_DATA *ch, *ch_next;
    int iHash;

    /* 
     * Scan through the scenes, the prop_list and actor_list,
     * locating all instance lists and starting or advancing any existing
     * parses contained in those lists, no matter what type of trigger.
     */


       /*
        * Scan scene hash updating all scene instances.
        */
    for ( iHash = 0;  iHash < MAX_KEY_HASH;  iHash++ )    {
    for ( scene = scene_index_hash[iHash]; scene != NULL;  scene = scene->next )
    {
        INSTANCE_DATA *trig;
        for ( trig = scene->instances;  trig != NULL;  trig = trig->next )
            parse_script( trig, scene, TYPE_SCENE );
    }
    }

       /*
        * Scan existing props.
        */
    for ( prop = prop_list;  prop != NULL;  prop = prop->next )    {
        INSTANCE_DATA *trig;
        for ( trig = prop->instances;  trig != NULL;  trig = trig->next )
            parse_script( trig, prop, TYPE_PROP );
    }

       /*
        * Scan through existing PCs and NPCs, 
        * updating parses of instances.
        */
    for ( ch = actor_list;  ch != NULL;  ch = ch_next )    {
        INSTANCE_DATA *trig;

        ch_next = ch->next;

        for ( trig = ch->instances;  trig != NULL;  trig = trig->next )
            parse_script( trig, ch, TYPE_ACTOR );
    }

    /*
     * Update events in the event queue.
     */
    update_event( );
    return;
}



/*
 * Automated reboots.
 */
void auto_reboot( void )
{
	if ( num_hour == (PULSE_PER_SECOND * 60) * 30 )
	{
//	write_global( "Reboot in 30 minutes.\n\r" );
//	cmd_echo( actor_list, "You don't need to quit, you'll be saved just before." );
	}
	else
	if ( num_hour == (PULSE_PER_SECOND * 60) * 15 )
	{
//	write_global( "Reboot in 15 minutes.\n\r" );
//	cmd_echo( actor_list, "You don't need to quit, you'll be saved just before." );
	}
	else
	if ( num_hour == (PULSE_PER_SECOND * 60) * 10 )
	{
//	write_global( "Reboot in 10 minutes.\n\r" );
//	cmd_echo( actor_list, "You don't need to quit, you'll be saved just before." );
	}
	else
	if ( num_hour == (PULSE_PER_SECOND * 60) * 5 )
	{
//	write_global( "Reboot in 5 minutes.\n\r" );
//      cmd_echo( actor_list, "You don't need to quit, you will be automatically saved just before." );
	}
	else
	if ( num_hour == (PULSE_PER_SECOND * 60) )
	{
//	write_global( "Reboot in 1 minute.\n\r" );
//	cmd_echo( actor_list, "You don't need to quit, you will be saved just before." );
	}
	else
	if ( num_hour <= 0 )
	{
		PLAYER_DATA *actor;
         
//		write_global( "Auto-rebooting; game will be back up shortly.\n\r" );

		for ( actor = actor_list; actor != NULL; actor = actor->next )
                if ( !IS_NPC(actor) ) cmd_save( actor, "internal" );

		cmd_zsave( NULL, "" );
                save_copyover();

                shut_down=TRUE;
	}
    return;
}


/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
void update_handler( void )
{
    num_hour--;
    num_pulse++;

    /*
     * Once per minute.
     */
    if ( --pulse_zone     <= 0 )
    {
		pulse_zone	= number_range( PULSE_ZONE / 2, 3 * PULSE_ZONE / 2 );
        everybody_update     ( TRUE );
        if ( number_range( 0, 1 ) != 0 ) weather_update  ( );
        prop_update_tick ( );
    }

    /*
     * Once per hour.
     */
    if ( --autosave_counter <= 0 )
    {
		extern int packet[60];
		int p;

		num_pulse = 0;
        for ( p = 0; p < 60; p++ )  packet[p] = -1;
		num_hour++;
/*
		write_global( "Please wait while the database is being saved..." );
 */
		connection_output( );
		autosave_counter = (PULSE_PER_SECOND * 60) * 60;
		cmd_zsave( NULL, "" ); 
/*
		write_global( "Database saved.\n\r" ); */
    }

    /*
     * Once every 4 seconds.
     */
    if ( --pulse_actor   <= 0 )
    {
		pulse_actor	= PULSE_MOV;
		actor_update	 ( );
		scene_update 	 ( );
		prop_update_pulse ( );
    }

    /*
     * Once every 3 seconds.
     */
    if ( --pulse_violence <= 0 )
    {
		pulse_violence	= PULSE_VIOLENCE;
		violence_update  ( );
    }


    /*
     * Once every five minutes.
     */
    if ( --pulse_point    <= 0 )
    {
        time_update     ( );
        pulse_point     = PULSE_TICK;
	everybody_update 	( FALSE );
    }

    if ( num_hour < (PULSE_PER_SECOND * 60) * 60 )
	auto_reboot( );
     
    aggr_update( );
    script_update_script( );
    tail_chain( );
    return;
}



/*
 * Player statistics module.
 *
 * Reads all of the player files in the player directory,
 * loads and sorts players determining:
 *
 *  - Highest Ranking Player on MUD
 *  - Biggest Landowner
 *  - Last 5 Players logged in today
 *  - Players with highest number of hours logged in
 */

char *ps_top_rank=NULL;
int ps_top_level=-1;

char *ps_top_landowner=NULL;
int ps_scene_count;

char *ps_last_1=NULL;
int ps_time_1=-1;
char *ps_last_2=NULL;
int ps_time_2=-1;
char *ps_last_3=NULL;
int ps_time_3=-1;
char *ps_last_4=NULL;
int ps_time_4=-1;
char *ps_last_5=NULL;
int ps_time_5=-1;

char *ps_longest=NULL;
int ps_length=-1;
int ps_logins=-1;

char *ps_top_mage=NULL;
int ps_magery=-1;
char *ps_top_skill=NULL;
int ps_learned=-1;

char *ps_top_exp=NULL;
int ps_exp=-1;

char *ps_top_wis=NULL;
int ps_wis=-1;
char *ps_top_dex=NULL;
int ps_dex=-1;
char *ps_top_int=NULL;
int ps_int=-1;
char *ps_top_str=NULL;
int ps_str=-1;
char *ps_top_con=NULL;
int ps_con=-1;


char *ps_top_bad=NULL;
int ps_hkarma=-1;
char *ps_top_good=NULL;
int ps_lkarma=-1;

int num_players=0;

void show_player_statistics( PLAYER_DATA *ch ) {
    char buf[MSL];

    display_interp( ch, "^B" );
    send_to_actor( "\n\r", ch );
    send_to_actor( "Game Statistics\n\r", ch );
    display_interp( ch, "^4" );
    send_to_actor( "---------------\n\r", ch );
    display_interp( ch, "^N" );

    if ( ps_top_rank ) {
       snprintf( buf, MSL, "^2Highest ranking player: ^N^B%s^N\n\r", ps_top_rank );
       display_interp( ch, buf );
    }

    if ( ps_top_landowner ) {
       snprintf( buf, MSL, "    ^2Greatest landowner:     ^N^B%s^N (%d)\n\r", ps_top_landowner, ps_scene_count );
       display_interp( ch, buf );
    }

    if ( ps_longest ) {
       snprintf( buf, MSL, "^2    Played the longest: ^N^B%s^N with ^B%d^N hours, ^B%d^N logins\n\r",
             ps_longest, ps_length % 24, ps_logins );
       display_interp( ch, buf );
    }

    if ( ps_top_skill ) {
       snprintf( buf, MSL, "^2 Most diverse skillset: ^N^B%s^N with ^B%d^N skills\n\r",
             ps_top_skill, ps_learned );
       display_interp( ch, buf );
    }

    if ( ps_top_mage ) {
       snprintf( buf, MSL, "^2    Most powerful mage: ^N^B%s^N with ^B%d^N spells\n\r",
             ps_top_mage, ps_magery );
       display_interp( ch, buf );
    }

    if ( ps_top_exp ) {
       snprintf( buf, MSL, "^2      Most experienced: ^N^B%s^N with ^B%d^N points\n\r",
             ps_top_exp, ps_exp );
       display_interp( ch, buf );
    }

    send_to_actor( "\n\r", ch );

    if ( ps_top_str ) {
       snprintf( buf, MSL, "^2 Strongest: ^N^B%s^N with ^B%d^N strength\n\r",
             ps_top_str, ps_str );
       display_interp( ch, buf );
    }

    if ( ps_top_int ) {
       snprintf( buf, MSL, "^2  Smartest: ^N^B%s^N with ^B%d^N intelligence\n\r",
             ps_top_int, ps_int );
       display_interp( ch, buf );
    }

    if ( ps_top_wis ) {
       snprintf( buf, MSL, "^2    Wisest: ^N^B%s^N with ^B%d^N wisdom\n\r",
             ps_top_wis, ps_wis );
       display_interp( ch, buf );
    }

    if ( ps_top_dex ) {
       snprintf( buf, MSL, "^2  Nimblest: ^N^B%s^N with ^B%d^N dexterity\n\r",
             ps_top_dex, ps_dex );
       display_interp( ch, buf );
    }

    if ( ps_top_con ) {
       snprintf( buf, MSL, "^2  Toughest: ^N^B%s^N with ^B%d^N constitution\n\r",
             ps_top_con, ps_con );
       display_interp( ch, buf );
    }

    send_to_actor( "\n\r", ch );

    if ( ps_top_bad && ps_top_good ) {
       snprintf( buf, MSL, "^2   Nicest: ^N^B%s^N with ^B%d^N karma\n\r",
             ps_top_good, ps_hkarma );
       display_interp( ch, buf );
       snprintf( buf, MSL, "^2  Meanest: ^N^B%s^N with ^B%d^N karma\n\r",
             ps_top_bad, ps_lkarma );
       display_interp( ch, buf );
    }


/*
    if ( ps_last_1 && ps_last_2 && ps_last_3 && ps_last_4 && ps_last_5 ) {
       snprintf( buf, MSL, "^2Last logins:     ^N^B%s^N, ^N^B%s^N, ^N^B%s^N, ^N^B%s^N, ^N^B%s^N\n\r", 
              ps_last_1, ps_last_2, ps_last_3, ps_last_4, ps_last_5 );
       display_interp( ch, buf );
    }
 */
}

void update_player_statistics( void ) {

  DIR *dir;
  struct dirent *dent;
  struct stat stbuf;
  char buf[MSL];
  char *f=NULL;
  extern bool fBootDb;

  if ( fBootDb ) wtf_logf( "Recalculating Player Statistics" );

  if(!(dir = opendir(PLAYER_DIR)))
  {
    perror("opendir()");
    return;
  }

  num_players=0;

  while((dent = readdir(dir)))
  {
    // Ignore ., .., .something, something.something because these files are not Players
    if(strstr(dent->d_name,".")) continue;

    snprintf(buf, MSL, "%s%s", PLAYER_DIR, dent->d_name);
    if((stat(buf, &stbuf)) == -1)
    {
      perror("stat()");
      continue;
    }

    f=fread_file( buf );        
    if ( f ) { 
      char *k=f;
      char arg[MSL];
      char name[MSL];
      int level=-1;  int immortal=-1;  int exp=-1;
      int logins=-1;
      int played=-1;
      int owns=0;
      int spells=0;
      int skills=0;
      int karma=0;
      int wis=0;
      int con=0;
      int dex=0;
      int _int=0;
      int str=0;
      name[0]='\0';
      while( *k!='\0' ) {
        k=one_argument(k,arg);
             if ( !str_cmp( arg, "n" ) ) { k=one_argument( k, name ); }
        else if ( !str_cmp( arg, "lv" ) ) { k=one_argument(k,arg); immortal=atoi(arg); }
        else if ( !str_cmp( arg, "mk" ) ) { k=one_argument(k,arg); k=one_argument(k,arg); karma=atoi(arg); }
        else if ( !str_cmp( arg, "exp" ) ) { k=one_argument(k,arg); exp=atoi(arg); }
        else if ( !str_cmp( arg, "explev" ) ) { k=one_argument(k,arg); level=atoi(arg); }
        else if ( !str_cmp( arg, "logins" ) ) { k=one_argument(k,arg); logins=atoi(arg); }
        else if ( !str_cmp( arg, "spell" ) ) { spells++; }
        else if ( !str_cmp( arg, "sk" ) )    { skills++; }
        else if ( !str_cmp( arg, "pl" ) ) { k=one_argument(k,arg); played=atoi(arg)/7200; }
        else if ( !str_cmp( arg, "ap" ) ) { k=one_argument(k,arg); str=atoi(arg); 
                                            k=one_argument(k,arg); _int=atoi(arg); 
                                            k=one_argument(k,arg); wis=atoi(arg); 
                                            k=one_argument(k,arg); dex=atoi(arg); 
                                            k=one_argument(k,arg); con=atoi(arg); }
        else if ( !str_cmp( arg, "end" ) ) break;
      }
      free_string( f );

      if ( immortal<0 || level<0 || played<0 || spells<0 || name[0]=='\0' ) continue;  // file was not valid

      num_players++;

      if ( immortal > 4 || !str_cmp( name, "vicker~" ) )  // it's an immortal, lets ignore this one
       continue;
      
      // remove the ~ from names
      { int i=0; for ( ; i<strlen(name); i++ ) if ( name[i] == '~' ) { name[i]='\0'; break; } }
//      wtf_logf( "Player: %s  Lv: %d, Logins: %d, Spells: %d, Played: %d", name, level, logins, spells, played );

      if ( played > ps_length ) {  
          ps_length = played;  ps_logins=logins;
          if ( ps_longest ) free_string(ps_longest);
          ps_longest = str_dup( capitalize(name) );
      } 

      if ( exp > ps_exp ) {
          ps_exp = exp; 
          if ( ps_top_exp ) free_string(ps_top_exp);
          ps_top_exp = str_dup( capitalize(name) );
      }

      if ( str > ps_str ) {
          ps_str = str; 
          if ( ps_top_exp ) free_string(ps_top_str);
          ps_top_str = str_dup( capitalize(name) );
      }

      if ( _int > ps_int ) {
          ps_int = _int; 
          if ( ps_top_int ) free_string(ps_top_int);
          ps_top_int = str_dup( capitalize(name) );
      }

      if ( wis > ps_wis ) {
          ps_wis = wis; 
          if ( ps_top_wis ) free_string(ps_top_wis);
          ps_top_wis = str_dup( capitalize(name) );
      }

      if ( dex > ps_dex ) {
          ps_dex = dex; 
          if ( ps_top_dex ) free_string(ps_top_dex);
          ps_top_dex = str_dup( capitalize(name) );
      }

      if ( con > ps_con ) {
          ps_con = con; 
          if ( ps_top_con ) free_string(ps_top_con);
          ps_top_con = str_dup( capitalize(name) );
      }

      if ( level > ps_top_level ) {
          ps_top_level = level;
          if ( ps_top_rank ) free_string(ps_top_rank);
          ps_top_rank = str_dup( capitalize(name) );
      }

      if ( spells > ps_magery ) {
          ps_magery = spells;
          if ( ps_top_mage ) free_string(ps_top_mage);
          ps_top_mage = str_dup( capitalize(name) );
      }

      if ( skills > ps_learned ) {
          ps_learned = skills;
          if ( ps_top_skill ) free_string(ps_top_skill);
          ps_top_skill = str_dup( capitalize(name) );
      }

      if ( karma < ps_lkarma ) {
          ps_lkarma = karma;
          if ( ps_top_bad ) free_string(ps_top_bad);
          ps_top_bad = str_dup( capitalize(name) );
      }

      if ( karma > ps_hkarma ) {
          ps_hkarma = karma;
          if ( ps_top_good ) free_string(ps_top_good);
          ps_top_good = str_dup( capitalize(name) );
      }


      /* Calculate Landownership */
      {
          int j; bool fOldBootDb=fBootDb;
          owns=0;  fBootDb=FALSE;
          for ( j =0; j <= top_vnum_scene; j++ ) {
             SCENE_INDEX_DATA *s = get_scene_index( j ); 
             if ( s && !str_cmp( s->owner, name ) ) owns++;
          } fBootDb=fOldBootDb;
          if ( owns > ps_scene_count ) {
          ps_scene_count = owns;
          if ( ps_top_landowner ) free_string(ps_top_landowner);
          ps_top_landowner = str_dup( capitalize(name) );
          }
      }
    }
  }

  closedir(dir);

}


void cmd_stats( PLAYER_DATA *ch, char *argument ) {
   update_player_statistics();
   show_player_statistics( ch );
   return;
}
