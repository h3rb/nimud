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
#include "net.h"
#include "board.h"
#include "skills.h"
#include "mem.h"
#include "script.h"

/*
 * Local variables.
 */

int         FORCE_LEVEL;


/*
 *  Syntax:  wizhelp
 */
void cmd_wizhelp( PLAYER_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;
 
    col = 0;
    for ( cmd = 0; command_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( command_table[cmd].level >= LEVEL_HERO
        &&   command_table[cmd].level <= get_trust( ch )
        &&   command_table[cmd].position >= POS_DEAD )
	{
        if ( command_table[cmd].cmd_fun == NULL )
        {
             send_to_actor( "\n\r", ch );

             if ( (++col) % 5 != 0 )
             send_to_actor( "\n\r", ch );
             display_interp( ch, "^B" );
             send_to_actor( command_table[cmd].name, ch );
             send_to_actor( "\n\r", ch );
             display_interp( ch, "^N" );
             col = 0;
        } else {

        snprintf( buf, MAX_STRING_LENGTH, "%10s [%2d]", command_table[cmd].name, command_table[cmd].level );
	    send_to_actor( buf, ch );
        if ( ++col % 5 == 0 )
        send_to_actor( "\n\r", ch );
        }
	}
    }
 
    if ( col % 4 != 0 )
    send_to_actor( "\n\r", ch );
    return;
}



/*
 *  Syntax:  rstat
 *           rstat [name/vnum]
 */
void cmd_rstat( PLAYER_DATA *ch, char *argument )
{
    extern char * const dir_name[];
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    INSTANCE_DATA *pTrig;
    VARIABLE_DATA *pVar;
    SCENE_INDEX_DATA *location;
    PROP_DATA *prop;
    PLAYER_DATA *rch;
    int door;

    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_scene : find_location( ch, arg );
    if ( location == NULL )
    {
    send_to_actor( "No such location.\n\r", ch );
	return;
    }

    snprintf( buf, MAX_STRING_LENGTH, "Name:  [%s]\n\rVnum: [%5d]   Zone:  [%5d] %s\n\r",
             location->name,
             location->vnum,
             location->zone->vnum,
             location->zone->name );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Reference:  [%5d]  Wagon: [%5d]   Sector[%s]\n\rLight: [%5d]  Max People: [%5d]\n\r",
             location->template,
             location->wagon,
             sector_name( location->sector_type ),
             location->light,
             location->max_people );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Flags: [%s]\n\rDescription:\n\r%s",
             scene_bit_name( location->scene_flags ),
             location->description );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Client:\n\r%s", location->client );
    send_to_actor( buf, ch ); 

    if ( location->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

    send_to_actor( "ED keywords: ", ch );
	for ( ed = location->extra_descr; ed; ed = ed->next )
    {
            send_to_actor( "'", ch );
            send_to_actor( ed->keyword, ch );
            send_to_actor( "'", ch );
	}
    send_to_actor( "\n\r", ch );
    }

    send_to_actor( "Chars: [", ch );
    for ( rch = location->people; rch; rch = rch->next_in_scene )
    {
        send_to_actor( " ", ch );
        one_argument( STR(rch, name), buf );
        send_to_actor( buf, ch );
    }

    send_to_actor( "]\n\rObjs:  [", ch );
    for ( prop = location->contents; prop; prop = prop->next_content )
    {
    send_to_actor( " ", ch );
    one_argument( STR(prop, name), buf );
	send_to_actor( buf, ch );
    }
    send_to_actor( "]\n\r", ch );

    for ( door = 0; door < MAX_DIR; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = location->exit[door] ) != NULL )
	{
        snprintf( buf, MAX_STRING_LENGTH, "%5s to [%5d], key [%5d], fl %3d/%3d [%s] '%s'\n\r",
                 capitalize( dir_name[door] ),
                 pexit->to_scene != NULL ? pexit->to_scene->vnum : 0,
                 pexit->key,
                 pexit->exit_info, pexit->rs_flags,
                 exit_bit_name( pexit->rs_flags ),
                 !MTD(pexit->keyword) ? pexit->keyword : "" );
	    send_to_actor( buf, ch );
        if ( pexit->description != NULL && pexit->description[0] != '\0' )
            send_to_actor( pexit->description, ch );
    }
    }

    if ( location->globals )
    send_to_actor( "Scene's variables:\n\r", ch );
    for ( pVar = location->globals;  pVar != NULL;  pVar = pVar->next )
    {
        snprintf( buf, MAX_STRING_LENGTH, "  [%2d] %s ", pVar->type, pVar->name );
        send_to_actor( buf, ch );

        switch ( pVar->type )
        {
            case TYPE_STRING: snprintf( buf, MAX_STRING_LENGTH, " = \"%s\"\n\r", (char *)pVar->value ); break;
            case TYPE_ACTOR:    snprintf( buf, MAX_STRING_LENGTH, " = %s\n\r", STR(((PLAYER_DATA *)pVar->value),name) ); break;
            case TYPE_PROP:    snprintf( buf, MAX_STRING_LENGTH, " = prop %d (%s)\n\r", 
                                        ((PROP_DATA *)(pVar->value))->pIndexData->vnum, 
                                        STR(((PROP_DATA *)(pVar->value)), short_descr) ); break;
            case TYPE_SCENE:   snprintf( buf, MAX_STRING_LENGTH, " = scene %d\n\r", ((SCENE_INDEX_DATA *)pVar->value)->vnum ); break;
            default:          snprintf( buf, MAX_STRING_LENGTH, " = <unknown>\n\r" ); break;
        }
        send_to_actor( buf, ch );
    }
    for ( pTrig = location->instances; pTrig != NULL; pTrig = pTrig->next )
    {
        snprintf( buf, MAX_STRING_LENGTH, "[%5d] %s (%d,%d)\n\r", pTrig->script->vnum, pTrig->script->name, pTrig->wait, pTrig->autowait );
        send_to_actor( buf, ch );

        for ( pVar = pTrig->locals;  pVar != NULL;  pVar = pVar->next )
        {
            snprintf( buf, MAX_STRING_LENGTH, "  [%2d] %s ", pVar->type, pVar->name );
            send_to_actor( buf, ch );

            switch ( pVar->type )
            {
                case TYPE_STRING: snprintf( buf, MAX_STRING_LENGTH, " = \"%s\"\n\r", (char *)pVar->value ); break;
                case TYPE_ACTOR:    snprintf( buf, MAX_STRING_LENGTH, " = %s\n\r", STR(((PLAYER_DATA *)pVar->value),name) ); break;
                case TYPE_PROP:    snprintf( buf, MAX_STRING_LENGTH, " = prop %d\n\r", ((PROP_DATA  *)pVar->value)->pIndexData->vnum ); break;
                case TYPE_SCENE:   snprintf( buf, MAX_STRING_LENGTH, " = scene %d\n\r", ((SCENE_INDEX_DATA *)pVar->value)->vnum ); break;
                default:          snprintf( buf, MAX_STRING_LENGTH, " = <unknown>\n\r" ); break;
            }
            send_to_actor( buf, ch );
        }
    }

    if ( location->spawn_first == NULL )
    {
        send_to_actor( "Scene has no spawns.\n\r", ch );
        return;
    }


    return;
}


/*
 * Syntax:  index actor [vnum]
 *          index prop [vnum]
 *          index scene   [vnum]
 *          index [vnum]
 */
void cmd_index( PLAYER_DATA *ch, char *argument )
{
    char arg[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg );
    
     if ( !str_prefix(arg, "actor") ) cmd_aindex( ch, argument );
else if ( !str_prefix(arg, "prop") ) cmd_pindex( ch, argument );
else if ( !str_prefix(arg, "scene"  ) ) cmd_rstat( ch, argument );
else cmd_rstat( ch, arg );

     return;
}
    
  


bool ostat( PLAYER_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    BONUS_DATA *paf;
    PROP_DATA *prop;
    INSTANCE_DATA *pTrig;
    VARIABLE_DATA *pVar;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
    send_to_actor( "Syntax:  stat <target> [description]\n\r", ch );
    return TRUE;
    }

    if ( ( prop = get_prop_world( ch, arg ) ) == NULL )
    {
    return FALSE;
    }

    snprintf( buf, MAX_STRING_LENGTH, "Name:   [%s]          zone: [%3d] %s\n\r",
    STR(prop, name),
    prop->pIndexData->zone->vnum,
    prop->pIndexData->zone->name );
    send_to_actor( buf, ch );


    snprintf( buf, MAX_STRING_LENGTH, "Vnum:   [%5d]   Size:  [%5d]  Type: [%s]\n\r",
             prop->pIndexData->vnum,  prop->size,
             item_type_name( prop->item_type ) );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Short:  [%s]\n\rPlural: [%s]\n\r",
             STR(prop, short_descr), pluralize( STR(prop, short_descr) ) );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Long:\n\r%s", STR(prop,description) );
    send_to_actor( buf, ch );

    if ( MTD(STR(prop,description_plural)) )
    snprintf( buf, MAX_STRING_LENGTH, "Plural:\n\r%s %s are here.\n\r",
             capitalize( numberize( number_range( 0, 100 ) ) ),
             pluralize( STR(prop, short_descr) ) );
    else
    {
        send_to_actor( "Plural:\n\r", ch );
        snprintf( buf, MAX_STRING_LENGTH, STR(prop,description_plural),
                      numberize( number_range( 0, 100 ) ) );
        buf[0] = UPPER(buf[0]);
    }
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Action:\n\r%s", STR(prop, action_descr) );
    send_to_actor( buf, ch );

    if ( *argument != '\0' && !str_prefix( argument, "description" ) )
    {
        snprintf( buf, MAX_STRING_LENGTH, "Description:\n\r%s", STR(prop,real_description) );
        send_to_actor( buf, ch );
    }

    snprintf( buf, MAX_STRING_LENGTH, "Wear bits:  [%s]\n\rExtra bits: [%s]\n\r",
    wear_bit_name( prop->wear_flags ),
    extra_bit_name( prop->extra_flags ) );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Number: [%d/%d]  Weight: [%d/%d]  Occupants: [%5d]\n\r",
             1,           get_prop_number( prop ),
             prop->weight, get_prop_weight( prop ),
             count_occupants( prop ) );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Cost:   [%5d]   Timer: [%5d]  Level: [%5d]\n\r",
             prop->cost, prop->timer, prop->level );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Scene:   [%5d]  Object: [%s] Contents[%s]  Carried: [%s]\n\rHitched: [%s]   Wear_loc: [%s]\n\r",
             prop->in_scene    == NULL    ?        0 : prop->in_scene->vnum,
             prop->in_prop     == NULL    ? "(none)" : STR(prop->in_prop, short_descr),
             prop->contains     == NULL    ? "(none)" : STR(prop->contains, short_descr),
             prop->carried_by == NULL    ? "(no one)" : NAME(prop->carried_by),
             hitched(prop)    == NULL    ? "(no one)" : NAME(hitched(prop)),
             wear_loc_name( prop->wear_loc ) );
    send_to_actor( buf, ch );

    show_occupants_to_actor( prop, ch );
    
    snprintf( buf, MAX_STRING_LENGTH, "Values: [%5d] [%5d] [%5d] [%5d]\n\r",
    prop->value[0],
    prop->value[1],
    prop->value[2],
    prop->value[3] );
    send_to_actor( buf, ch );

    value_breakdown( prop->item_type,
                     prop->value[0],
                     prop->value[1],
                     prop->value[2],
                     prop->value[3],
                     ch );

    if ( prop->extra_descr != NULL || prop->pIndexData->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

    send_to_actor( "ED keywords: [", ch );

	for ( ed = prop->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_actor( ed->keyword, ch );
	    if ( ed->next != NULL )
        send_to_actor( " ", ch );
	}

	for ( ed = prop->pIndexData->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_actor( ed->keyword, ch );
	    if ( ed->next != NULL )
        send_to_actor( " ", ch );
	}

    send_to_actor( "]\n\r", ch );
    }

    for ( paf = prop->bonus; paf != NULL; paf = paf->next )
    {
       snprintf( buf, MAX_STRING_LENGTH, "Bonuses %s by %d.\n\r", bonus_loc_name( paf->location ),
                                              paf->modifier );
       send_to_actor( buf, ch );
    }

    for ( paf = prop->pIndexData->bonus; paf != NULL; paf = paf->next )
    {
       snprintf( buf, MAX_STRING_LENGTH, "Bonuses %s by %d.\n\r", bonus_loc_name( paf->location ),
                                              paf->modifier );
       send_to_actor( buf, ch );
    }

    /*
     * Globals.
     */
    if ( prop->globals )
    send_to_actor( "Prop's variables:\n\r", ch );
    for ( pVar = prop->globals;  pVar != NULL;  pVar = pVar->next )
    {
         snprintf( buf, MAX_STRING_LENGTH, "  [%2d] %s ", pVar->type, pVar->name );
         send_to_actor( buf, ch );

         switch ( pVar->type )
         {
             case TYPE_STRING: snprintf( buf, MAX_STRING_LENGTH, " = \"%s\"\n\r", (char *)pVar->value ); break;
             case TYPE_ACTOR:    snprintf( buf, MAX_STRING_LENGTH, " = %s\n\r", STR(((PLAYER_DATA *)pVar->value),name) ); break;
             case TYPE_PROP:    snprintf( buf, MAX_STRING_LENGTH, " = %d\n\r", ((PROP_DATA  *)pVar->value)->pIndexData->vnum ); break;
             case TYPE_SCENE:   snprintf( buf, MAX_STRING_LENGTH, " = %d\n\r", ((SCENE_INDEX_DATA *)pVar->value)->vnum ); break;
             default:          snprintf( buf, MAX_STRING_LENGTH, " = <unknown>\n\r" ); break;
         }
         send_to_actor( buf, ch );
     }
    for ( pTrig = prop->instances; pTrig != NULL; pTrig = pTrig->next )
    {
        snprintf( buf, MAX_STRING_LENGTH, "[%5d] %s (%d,%d)\n\r", pTrig->script->vnum, pTrig->script->name, pTrig->wait, pTrig->autowait );
        send_to_actor( buf, ch );

        for ( pVar = pTrig->locals;  pVar != NULL;  pVar = pVar->next )
        {
            snprintf( buf, MAX_STRING_LENGTH, "  [%2d] %s ", pVar->type, pVar->name );
            send_to_actor( buf, ch );

            switch ( pVar->type )
            {
                case TYPE_STRING: snprintf( buf, MAX_STRING_LENGTH, " = \"%s\"\n\r", (char *)pVar->value ); break;
                case TYPE_ACTOR:    snprintf( buf, MAX_STRING_LENGTH, " = %s\n\r", STR(((PLAYER_DATA *)pVar->value),name) ); break;
                case TYPE_PROP:    snprintf( buf, MAX_STRING_LENGTH, " = %d\n\r", ((PROP_DATA  *)pVar->value)->pIndexData->vnum ); break;
                case TYPE_SCENE:   snprintf( buf, MAX_STRING_LENGTH, " = %d\n\r", ((SCENE_INDEX_DATA *)pVar->value)->vnum ); break;
                default:          snprintf( buf, MAX_STRING_LENGTH, " = <unknown>\n\r" ); break;
            }
            send_to_actor( buf, ch );
        }
    }

    return TRUE;
}


bool mstat( PLAYER_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    BONUS_DATA *paf;
    PLAYER_DATA *victim;
    INSTANCE_DATA *pTrig;
    VARIABLE_DATA *pVar;
    int days, hours;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
    send_to_actor( "Syntax: stat <target> [description]\n\r", ch );
    return TRUE;
    }

    if ( ( victim = get_actor_world( ch, arg ) ) == NULL )
    {
    return FALSE;
    }

    snprintf( buf, MAX_STRING_LENGTH, "Name:  [%s]\n\r",   STR(victim, name) );
    send_to_actor( buf, ch );


    if ( victim->pIndexData != NULL )
    {
    snprintf( buf, MAX_STRING_LENGTH, "Vnum:  [%5d]  ", victim->pIndexData->vnum );
    send_to_actor( buf, ch );
    }

    if ( victim->pIndexData != NULL && victim->pIndexData->zone != NULL )
    {
    snprintf( buf, MAX_STRING_LENGTH, "zone:  [%3d] %s\n\r",
             victim->pIndexData->zone->vnum,
             victim->pIndexData->zone->name );
    send_to_actor( buf, ch );
    }

    snprintf( buf, MAX_STRING_LENGTH, "Hp:  [%3d/%3d]  Mana: [%3d/%3d]  Move: [%3d/%3d]  Karma: [%d] Exp: [%d]\n\rDrunk: [%5d/100]  Thirst: [%4d/100]  Full: [%d/100]\n\r",
    victim->hit,         MAXHIT(victim),
    victim->mana,        MAXMANA(victim),
    victim->move,        MAXMOVE(victim),
    victim->karma,       victim->exp,
    GET_PC(victim,condition[COND_DRUNK],0),
    GET_PC(victim,condition[COND_THIRST],0),
    GET_PC(victim,condition[COND_FULL],0) );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Size:  [%5d]   Race:  [%5d]   Sex:  [%s]    Scene:  %s [%d]\n\r",
             victim->size,
             victim->race,
             victim->sex == SEX_MALE    ? "male"   :
             victim->sex == SEX_FEMALE  ? "female" : "neutral",
             victim->in_scene == NULL    ? "None"   :
                 trunc_fit( victim->in_scene->name, 15 ),
             victim->in_scene == NULL    ?        0 : victim->in_scene->vnum );
    send_to_actor( buf, ch );


    snprintf( buf, MAX_STRING_LENGTH, "Str:   [%5d]   Int:   [%5d]   Wis:  [%5d]   Dex:    [%5d] Con: [%5d]\n\r",
                  get_curr_str( victim ),
                  get_curr_int( victim ),
                  get_curr_wis( victim ),
                  get_curr_dex( victim ),
                  get_curr_con( victim ) );
    send_to_actor( buf, ch );
	
    snprintf( buf, MAX_STRING_LENGTH, "Lv: [%5d]  AC: [%5d]  Coins: [%5d] $:[%d] MU:[%d]  Fight:  [%d, %d]\n\r",
             GET_PC(victim,level,-1),
             GET_AC(victim),      tally_coins ( victim ),  victim->bucks, victim->credits,
             victim->fmode, victim->fbits );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Wait:  [%5d]   Invis: [%5d] Bounty: [%5d]   Owed:   [%5d]\n\r",
             victim->wait,        GET_PC(victim,wizinvis,0),
             victim->bounty,      victim->owed );
    send_to_actor( buf, ch );


    days  = ((int) GET_PC(victim,played,0)) / 24;
    hours = ((int) GET_PC(victim,played,0)) % 24;
    snprintf( buf, MAX_STRING_LENGTH, "Age:   [%5d]   Timer: [%5d]  Trace: [%5d]   Played: [%d, %d]\n\r",
                  GET_AGE(victim), victim->timer,
                  IS_NPC(ch) ? -1 :
                    (ch->userdata->trace != NULL ? ch->userdata->trace->vnum : -1),
                  days, hours );
    send_to_actor( buf, ch );

    if ( !IS_NPC(victim) ) {
        snprintf( buf, MAX_STRING_LENGTH, "%d Total Logins- Constellation:\n\r%s\n\r", 
             PC(victim,logins), PC(victim,constellation) );
        send_to_actor( buf, ch );
    }
 
    snprintf( buf, MAX_STRING_LENGTH, "Act:     [%s] %d\n\r",
             IS_NPC(victim) ? act_bit_name( victim->act )
                            : plr_bit_name( victim->act ),
            !IS_NPC(victim) ? PC(victim,app_time) : 0 );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Bonuses: [%s]\n\r", bonus_bit_name(victim->bonuses));
    send_to_actor( buf, ch );


    for ( paf = victim->bonus; paf != NULL; paf = paf->next )
    {
       snprintf( buf, MAX_STRING_LENGTH, "Bonuses %s by %d.\n\r", bonus_loc_name( paf->location ),
                                              paf->modifier );
       send_to_actor( buf, ch );
    }

    snprintf( buf, MAX_STRING_LENGTH, "Wielding: [%d/%d]  Hitroll/Damroll: [%d/%d]  [%s",
             wield_weight(victim),
             str_app[get_curr_str(victim)].wield,
             GET_HITROLL(victim), GET_DAMROLL(victim),
             position_name( victim->position ) );
    send_to_actor( buf, ch );
	
    if ( victim->furniture )
    {
        snprintf( buf, MAX_STRING_LENGTH, " on %s", STR(victim->furniture, short_descr) );
        send_to_actor( buf, ch );
    }

    if ( victim->position == POS_FIGHTING )
    {
        snprintf( buf, MAX_STRING_LENGTH, " with %s",
                      victim->fighting ? NAME(victim->fighting) : "nobody" );
        send_to_actor( buf, ch );
    }

    send_to_actor( "]\n\r", ch );

    snprintf( buf, MAX_STRING_LENGTH, "Carrying %d items of weight %d.\n\r",
	victim->carry_number, victim->carry_weight );
    send_to_actor( buf, ch );

    buf[0] = '\0';

    if ( victim->master || !MTD(victim->tracking) )
    {
    snprintf( buf, MAX_STRING_LENGTH, "Master: %s  Hunting: %s\n\r",
             victim->master      ? NAME(victim->master)   : "Nobody",
             MTD(victim->tracking) ? "Nobody" : victim->tracking );
    send_to_actor( buf, ch );
    }

    if ( victim->riding
      || victim->rider )
    {
    snprintf( buf, MAX_STRING_LENGTH, "Riding: [%s]  Mounted by:  [%s]",
             victim->riding      ? NAME(victim->riding)     : "Nobody",
             victim->rider       ? NAME(victim->rider)      : "Nobody" );
    send_to_actor( buf, ch );
    }

    if ( buf[0] != '\0' ) send_to_actor( "\n\r", ch );

    snprintf( buf, MAX_STRING_LENGTH, "Short: [%s]\n\rLong:\n\r%s",
             STR(victim, short_descr),  STR(victim, long_descr) );
    send_to_actor( buf, ch );
    
    if ( STR(victim, long_descr)[strlen(STR(victim, long_descr))-2] != '\n' )
    send_to_actor( "\n\r", ch );

    if ( *argument != '\0' && !str_prefix( argument, "description" ) )
    {
        snprintf( buf, MAX_STRING_LENGTH, "Description:\n\r%s", STR(victim,description) );
        send_to_actor( buf, ch );
    }

    if ( !MTD(victim->keywords) )
    {
        snprintf( buf, MAX_STRING_LENGTH, "Extended Keywords: [%s]\n\r", victim->keywords );
        send_to_actor( buf, ch );
    }

    if ( victim->userdata )
    {
    if ( !MTD(PC(victim,email)) )
    {
        snprintf( buf, MAX_STRING_LENGTH, "Email: %s\n\r", PC(victim,email) );
        send_to_actor( buf, ch );
    }

    if ( !MTD(PC(victim,denial)) )
    {
        snprintf( buf, MAX_STRING_LENGTH, "Rejection notice:\n\r%s", PC(victim,denial) );
        send_to_actor( buf, ch );
    }
    }

    if ( victim->globals )
    send_to_actor( "Variables:\n\r", ch );
    for ( pVar = victim->globals;  pVar != NULL;  pVar = pVar->next )
    {
       snprintf( buf, MAX_STRING_LENGTH, "  [%2d] %s ", pVar->type, pVar->name );
       send_to_actor( buf, ch );

       switch ( pVar->type )
       {
           case TYPE_STRING: snprintf( buf, MAX_STRING_LENGTH, "[str] = \"%s\"\n\r", (char *)pVar->value ); break;
           case TYPE_ACTOR:    snprintf( buf, MAX_STRING_LENGTH, "[actor] = %s\n\r", STR(((PLAYER_DATA *)pVar->value),name) ); break;
           case TYPE_PROP:    snprintf( buf, MAX_STRING_LENGTH, "[prop] = %d\n\r", ((PROP_DATA  *)pVar->value)->pIndexData->vnum); break;
           case TYPE_SCENE:   snprintf( buf, MAX_STRING_LENGTH, "[rm ] = %d\n\r", ((SCENE_INDEX_DATA *)pVar->value)->vnum ); break;
             default:          snprintf( buf, MAX_STRING_LENGTH, " = <unknown>\n\r" ); break;
       }
       send_to_actor( buf, ch );
    }

    for ( pTrig = victim->instances; pTrig != NULL; pTrig = pTrig->next )
    {
        snprintf( buf, MAX_STRING_LENGTH, "[%5d] %s (%d,%d)\n\r", pTrig->script->vnum, pTrig->script->name, pTrig->wait, pTrig->autowait );
        send_to_actor( buf, ch );

        for ( pVar = pTrig->locals;  pVar != NULL;  pVar = pVar->next )
        {
            snprintf( buf, MAX_STRING_LENGTH, "  [%2d] %s ", pVar->type, pVar->name );
            send_to_actor( buf, ch );

            switch ( pVar->type )
            {
                case TYPE_STRING: snprintf( buf, MAX_STRING_LENGTH, "[str] = \"%s\"\n\r", (char *)pVar->value ); break;
                case TYPE_ACTOR:    snprintf( buf, MAX_STRING_LENGTH, "[actor] = %s\n\r", STR(((PLAYER_DATA *)pVar->value),name) ); break;
                case TYPE_PROP:    snprintf( buf, MAX_STRING_LENGTH, "[prop] = %d\n\r", ((PROP_DATA  *)pVar->value)->pIndexData->vnum); break;
                case TYPE_SCENE:   snprintf( buf, MAX_STRING_LENGTH, "[rm ] = %d\n\r", ((SCENE_INDEX_DATA *)pVar->value)->vnum ); break;
                default:          snprintf( buf, MAX_STRING_LENGTH, " = <unknown>\n\r" ); break;
            }
            send_to_actor( buf, ch );
        }
    }

    for ( paf = victim->bonus; paf != NULL; paf = paf->next )
    {
	sprintf( buf,
        "[Spell %s on %s by %d for %d with %s]\n\r",
	    skill_name((int) paf->type),
	    bonus_loc_name( paf->location ),
	    paf->modifier,
	    paf->duration,
        bonus_bit_name( paf->bitvector )        );
	send_to_actor( buf, ch );
    }

    return TRUE;

}




bool pstat( PLAYER_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    SCRIPT_DATA *pScript;
    int vnum;
    int search;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
    send_to_actor( "Syntax: stat <script vnum>\n\r", ch );
    return TRUE;
    }

    search = atoi( arg );

    pScript = NULL;
    for ( vnum = 0; vnum <= top_vnum_script; vnum++ )
    {
        if ( vnum == search )
        {
            pScript = get_script_index( vnum );
            break;
        }
    }

    if ( pScript == NULL )
    return FALSE;


    snprintf( buf, MAX_STRING_LENGTH, "Vnum:  [%5d]  Name:  [%s]\n\r", pScript->vnum,
             pScript->name  );
    page_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Type:  [%5d]         [%s]\n\r", pScript->type,
             show_script_type( pScript->type ) );
    page_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Script:\n\r%s", pScript->commands );
    page_to_actor( buf, ch );

    return TRUE;            
}


/*
 * Syntax:  stat [person/thing/num]
 */
void cmd_stat( PLAYER_DATA *ch, char *argument )
{
    if ( *argument == '\0' )
    {
        send_to_actor( "Usage:   stat [person|thing|vnum]\n\r", ch );
        return;
    }

    if ( !mstat( ch, argument )
      && !ostat( ch, argument )
      && !pstat( ch, argument ) )
        send_to_actor( "Nothing like that to stat.\n\r", ch );

     return;
}



    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_actor_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */

/*
 * Enhancements by Locke
 * Syntax:  afind all
 *          afind [name]
 *          afind [name] [lower [upper]]
 *          afind zone [lower [upper]]
 *          afind [lower] [upper]
 *          afind exp [lower [upper]]
 */
void cmd_afind( PLAYER_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    ACTOR_INDEX_DATA *pActorIndex;
    bool fRange = FALSE;
    bool fzone = FALSE;
    bool fName = FALSE;
    int range1 = 0;
    int range2 = 0;
    ZONE_DATA *pZone = ch->in_scene->zone;
    char name[MAX_INPUT_LENGTH];
    int vnum;
    bool found = FALSE;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' )
    {
    send_to_actor( "Syntax: afind all\n\r",               ch );
    send_to_actor( "        afind [name, zone] [num1 num2] [zone]\n\r", ch );
    send_to_actor( "        afind [lower] [upper] exp\n\r", ch );
	return;
    }

    if ( is_number( arg1 ) && is_number( arg2 ) )
    {
        fRange = TRUE;
        range1 = atoi( arg1 );
        range2 = atoi( arg2 );
        if ( !str_cmp(arg3, "zone") )     fzone = TRUE;
        else
        {
            if ( !str_cmp( arg3, "all" ) )
            {
                fRange = FALSE;
                fName  = FALSE;
                fzone  = FALSE;
            }
            else
            {
                fName = TRUE;
                strcpy( name, arg3 );
            }
        }
    }
    else
    {
        if ( is_number( arg2 ) && is_number( arg3 ) )
        {
            fRange = TRUE;
            range1 = atoi( arg2 );
            range2 = atoi( arg3 );
        }
        if ( !str_cmp(arg1, "zone") )     fzone = TRUE;
        else
        {
            if ( !str_cmp( arg1, "all" ) )
            {
                fRange = FALSE;
                fName  = FALSE;
                fzone  = FALSE;
            }
            else
            {
                fName = TRUE;
                strcpy( name, arg1 );
            }
        }
    }


    if ( range1 > range2 && fRange )
    {
        vnum = range1;
        range1 = range2;
        range2 = vnum;
    }


    if ( !str_cmp( arg3, "exp" ) ) {
    for ( vnum = 0; vnum <= top_vnum_actor; vnum++ )
    {
        if ( ( pActorIndex = get_actor_index( vnum ) ) != NULL
          && ( pActorIndex->exp > range1 && pActorIndex->exp < range2 ) )
        {
            found = TRUE;
            snprintf( buf, MAX_STRING_LENGTH, "[%5d] %-42s  %s %d xp %d karma\n\r", vnum, pActorIndex->short_descr,
                                             !fzone ? trunc_fit(pActorIndex->name,20) : "", pActorIndex->exp, pActorIndex->karma );
            send_to_actor( buf, ch );
        }
    } }
    else {
    if ( range2 > top_vnum_actor || !fRange ) range2 = top_vnum_actor;
    for ( vnum = range1; vnum <= range2; vnum++ )
    {
        if ( ( pActorIndex = get_actor_index( vnum ) ) != NULL
          && ( is_prename( name, pActorIndex->name )
            || is_prename( name, pActorIndex->short_descr ) || !fName )
          && ( pActorIndex->zone == pZone || !fzone ) )
        {
            found = TRUE;
            snprintf( buf, MAX_STRING_LENGTH, "[%5d] %-42s  %s %d xp %d karma\n\r", vnum, pActorIndex->short_descr,
                                             !fzone ? trunc_fit(pActorIndex->name,20) : "", pActorIndex->exp, pActorIndex->karma );
            send_to_actor( buf, ch );
        }
    }
    }

    if ( !found )
    send_to_actor( "Nothing like that in hell, earth, or heaven.\n\r", ch );

    return;
}


/*
 * Enhancements by Locke
 * Syntax:  pfind all
 *          pfind [name]
 *          pfind [name] [lower [upper]]
 *          pfind zone [lower [upper]]
 *          pfind [lower] [upper]
 */
void cmd_pfind( PLAYER_DATA *ch, char *argument )
{
    char buf2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];

    PROP_INDEX_DATA *pPropIndex;

    bool fRange = FALSE;
    bool fzone = FALSE;
    bool fName = FALSE;

    int range1 = 0;
    int range2 = 0;

    ZONE_DATA *pZone = ch->in_scene->zone;
    char name[MAX_INPUT_LENGTH];

    int vnum;
    bool found = FALSE;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' )
    {
    send_to_actor( "Syntax: pfind all\n\r",               ch );
    send_to_actor( "        pfind [name, zone] [num1 num2]\n\r", ch );
	return;
    }

    if ( is_number( arg1 ) && is_number( arg2 ) )
    {
        fRange = TRUE;
        range1 = atoi( arg1 );
        range2 = atoi( arg2 );
        if ( !str_cmp(arg3, "zone") )     fzone = TRUE;
        else
        {
            if ( !str_cmp( arg3, "all" ) )
            {
                fRange = FALSE;
                fName  = FALSE;
                fzone  = FALSE;
            }
            else
            {
                fName = TRUE;
                strcpy( name, arg3 );
            }
        }
    }
    else
    {
        if ( is_number( arg2 ) && is_number( arg3 ) )
        {
            fRange = TRUE;
            range1 = atoi( arg2 );
            range2 = atoi( arg3 );
        }
        if ( !str_cmp(arg1, "zone") )     fzone = TRUE;
        else
        {
            if ( !str_cmp( arg1, "all" ) )
            {
                fRange = FALSE;
                fName  = FALSE;
                fzone  = FALSE;
            }
            else
            {
                fName = TRUE;
                strcpy( name, arg1 );
            }
        }
    }


    if ( range1 > range2 && fRange )
    {
        vnum = range1;
        range1 = range2;
        range2 = vnum;
    }

    if ( range2 > top_vnum_prop || !fRange ) range2 = top_vnum_prop;

    buf2[0] = '\0';
    for ( vnum = range1; vnum <= range2; vnum++ )
    {
        if ( ( pPropIndex = get_prop_index( vnum ) ) != NULL
          && ( is_prename( name, pPropIndex->name )
            || is_prename( name, pPropIndex->short_descr ) || !fName )
          && ( pPropIndex->zone == pZone || !fzone ) )
        {
            found = TRUE;
            snprintf( buf, MAX_STRING_LENGTH, "[%5d %10s] %-42s  %s\n\r", vnum,
                                             item_type_name( pPropIndex->item_type ),
                                             pPropIndex->short_descr,
                                             !fzone ? trunc_fit(pPropIndex->zone->name,20) : "" );
            if ( strlen(buf) + strlen(buf2) < MSL )
            strcat( buf2, buf );
        }
    }
    if ( buf2[0] != '\0' ) send_to_actor( buf2, ch );


    if ( !found )
    send_to_actor( "Nothing like that in hell, earth, or heaven.\n\r", ch );

    return;
}


/*
 * Syntax:  rfind all
 *          rfind [name]
 *          rfind [name] [lower [upper]]
 *          rfind zone [lower [upper]]
 *          rfind [lower] [upper]
 */
void cmd_rfind( PLAYER_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    SCENE_INDEX_DATA *pSceneIndex;
    bool fRange = FALSE;
    bool fzone = FALSE;
    bool fName = FALSE;
    int range1 = 0;
    int range2 = 0;
    ZONE_DATA *pZone = ch->in_scene->zone;
    char name[MAX_INPUT_LENGTH];
    int vnum;
    bool found = FALSE;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' )
    {
    send_to_actor( "Syntax: rfind all\n\r",               ch );
    send_to_actor( "        rfind [name, zone] [num1 num2]\n\r", ch );
	return;
    }

    if ( is_number( arg1 ) && is_number( arg2 ) )
    {
        fRange = TRUE;
        range1 = atoi( arg1 );
        range2 = atoi( arg2 );

        if ( !str_cmp(arg3, "zone") )     fzone = TRUE;
        else
        {
            if ( !str_cmp( arg3, "all" ) )
            {
                fRange = FALSE;
                fName  = FALSE;
                fzone  = FALSE;
            }
            else
            {
                fName = TRUE;
                strcpy( name, arg3 );
            }
        }
    }
    else
    {
        if ( is_number( arg2 ) && is_number( arg3 ) )
        {
            fRange = TRUE;
            range1 = atoi( arg2 );
            range2 = atoi( arg3 );
        }

        if ( !str_cmp(arg1, "zone") )     fzone = TRUE;
        else
        {
            if ( !str_cmp( arg1, "all" ) )
            {
                fRange = FALSE;
                fName  = FALSE;
                fzone  = FALSE;
            }
            else
            {
                fName = TRUE;
                strcpy( name, arg1 );
            }
        }
    }


    if ( range1 > range2 && fRange )
    {
        vnum = range1;
        range1 = range2;
        range2 = vnum;
    }

    if ( range2 > top_vnum_scene || !fRange ) range2 = top_vnum_scene;

    for ( vnum = range1; vnum <= range2; vnum++ )
    {
        if ( ( pSceneIndex = get_scene_index( vnum ) ) != NULL
          && ( is_prename( name, pSceneIndex->name ) || !fName )
          && ( pSceneIndex->zone == pZone         || !fzone ) )
        {
            found = TRUE;
            snprintf( buf, MAX_STRING_LENGTH, "[%5d] %-42s  %s\n\r", vnum,
                                       trunc_fit( pSceneIndex->name, 42 ),
                                       !fzone ? pSceneIndex->zone->name : "" );
            send_to_actor( buf, ch );
        }
    }


    if ( !found )
    send_to_actor( "Nothing like that in hell, earth, or heaven.\n\r", ch );

    return;
}


/*
 * Syntax:  scfind all
 *          scfind [name]
 *          scfind [name] [lower [upper]]
 *          scfind zone [lower [upper]]
 *          scfind [lower] [upper]
 */
void cmd_scfind( PLAYER_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    SCRIPT_DATA *pScript;
    bool fRange = FALSE;
    bool fzone = FALSE;
    bool fName = FALSE;
    int range1 = 0;
    int range2 = 0;
    ZONE_DATA *pZone = ch->in_scene->zone;
    char name[MAX_INPUT_LENGTH];
    int vnum;
    bool found = FALSE;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' )
    {
    send_to_actor( "Syntax: scfind all\n\r",               ch );
    send_to_actor( "        scfind [name, zone] [num1 num2]\n\r", ch );
	return;
    }

    if ( is_number( arg1 ) && is_number( arg2 ) )
    {
        fRange = TRUE;
        range1 = atoi( arg1 );
        range2 = atoi( arg2 );

        if ( !str_cmp(arg3, "zone") )     fzone = TRUE;
        else
        {
            if ( !str_cmp( arg3, "all" ) )
            {
                fRange = FALSE;
                fName  = FALSE;
                fzone  = FALSE;
            }
            else
            {
                fName = TRUE;
                strcpy( name, arg3 );
            }
        }
    }
    else
    {
        if ( is_number( arg2 ) && is_number( arg3 ) )
        {
            fRange = TRUE;
            range1 = atoi( arg2 );
            range2 = atoi( arg3 );
        }

        if ( !str_cmp(arg1, "zone") )     fzone = TRUE;
        else
        {
            if ( !str_cmp( arg1, "all" ) )
            {
                fRange = FALSE;
                fName  = FALSE;
                fzone  = FALSE;
            }
            else
            {
                fName = TRUE;
                strcpy( name, arg1 );
            }
        }
    }


    if ( range1 > range2 && fRange )
    {
        vnum = range1;
        range1 = range2;
        range2 = vnum;
    }

    if ( range2 > top_vnum_script || !fRange ) range2 = top_vnum_script;

    for ( vnum = range1; vnum <= range2; vnum++ )
    {
        if ( ( pScript = get_script_index( vnum ) ) != NULL
          && ( is_prename( name, pScript->name ) || !fName )
          && ( pScript->zone == pZone         || !fzone ) )
        {
            found = TRUE;
            snprintf( buf, MAX_STRING_LENGTH, "[%5d %2d] %-32s  %s\n\r", vnum, pScript->type,
                                             pScript->name,
                                             !fzone ? pScript->zone->name : "" );
            send_to_actor( buf, ch );
        }
    }


    if ( !found )
    send_to_actor( "Nothing like that in hell, earth, or heaven.\n\r", ch );

    return;
}



 
bool mwhere( PLAYER_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    PLAYER_DATA *victim;
    bool found, fAll = FALSE;

    one_argument( argument, arg );
    if ( arg[0] == '\0' ) return FALSE;

    if ( !str_cmp( arg, "all" ) ) fAll = TRUE;

    found = FALSE;
    for ( victim = actor_list; victim != NULL; victim = victim->next )
    {
    if ( can_see( ch, victim )
	&&   victim->in_scene != NULL
    && ( is_name(arg, NAME(victim)) || fAll ) )
	{
	    found = TRUE;
        snprintf( buf, MAX_STRING_LENGTH, "[%5d] %-28s at [%5d] %s\n\r",
                 IS_NPC(victim) ? victim->pIndexData->vnum : 0,
                 IS_NPC(victim) ? STR(victim, short_descr) : STR(victim,name),
                 victim->in_scene->vnum,
                 victim->in_scene->name );
        send_to_actor( buf, ch );
	}
    }

    return found;
}


/* Idea from Talen of Vego's cmd_where command */

bool owhere( PLAYER_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    bool found = FALSE;
    PROP_DATA *prop;
    PROP_DATA *in_prop;
    int prop_counter = 1;

    one_argument( argument, arg );

    if( arg[0] == '\0' )
    return FALSE;

	for ( prop = prop_list; prop != NULL; prop = prop->next )
	{
        if ( !can_see_prop( ch, prop ) || !is_name( arg, STR(prop, name) ) )
	        continue;

	    found = TRUE;

	    for ( in_prop = prop; in_prop->in_prop != NULL;
		 in_prop = in_prop->in_prop )
	        ;

	    if ( in_prop->carried_by != NULL )
	    {
		snprintf( buf, MAX_STRING_LENGTH, "[%2d] %s carried by %s.\n\r", prop_counter,
            STR(prop, short_descr), PERS( in_prop->carried_by, ch ) );
	    }
	    else
	    {
        snprintf( buf, MAX_STRING_LENGTH, "[%2d] %s in %s [%d].\n\r", prop_counter,
            STR(prop, short_descr), ( in_prop->in_scene == NULL ) ?
            "somewhere" : in_prop->in_scene->name, (in_prop->in_scene == NULL) ?
            0 : in_prop->in_scene->vnum );
	    }
	    
	    prop_counter++;
	    buf[0] = UPPER( buf[0] );
	    send_to_actor( buf, ch );
	}

    return found;
}

void pwhere( PLAYER_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    PLAYER_DATA *victim;
    bool found;

	send_to_actor( "Players near you:\n\r", ch );
	found = FALSE;
    for ( victim = actor_list;  victim != NULL;  victim = victim->next )
	{
       if ( !IS_NPC(victim)
         && victim->in_scene != NULL
         && can_see( ch, victim ) )
       {
        found = TRUE;
        snprintf( buf, MAX_STRING_LENGTH, "%-18s %28s  [%5d]  %s\n\r", STR(victim,name),
                                               victim->in_scene->name,
                                               victim->in_scene->vnum,
                                               trunc_fit(victim->in_scene->zone->name,20) );
        send_to_actor( buf, ch );
       }
	}

    if ( !found ) send_to_actor( "No players can be found.\n\r", ch );

    return;
}



/*
 * Syntax:  where [person/thing]
 */
void cmd_where( PLAYER_DATA *ch, char *argument )
{
    char arg[MAX_STRING_LENGTH];
    bool found1, found2;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || !IS_IMMORTAL(ch) )
    {
        pwhere( ch );
        return;
    }

    found1 = mwhere( ch, arg );
    found2 = owhere( ch, arg );

    if ( !found1 && !found2 )
    {
        send_to_actor( "Nothing like that anywhere.\n\r", ch );
        return;
    }
    return;
}



char *connect_string( int c )
{
    switch ( c )
    {
            case CON_ZEDITOR: return "EDITING: zones";
            case CON_REDITOR: return "EDITING: scenes";
            case CON_AEDITOR: return "EDITING: actors";
            case CON_OEDITOR: return "EDITING: objects";
            case CON_SEDITOR: return "EDITING: scripts";
            case CON_HEDITOR: return "EDITING: helps";
            case CON_PLAYING: return "Playing the game.";
         case CON_SHOW_TITLE: return "At the title screen.";
          case CON_READ_MOTD: return "Reading the login message.";
          case CON_STAT_MENU: return "At the Stat Mod menu.";
           case CON_STAT_STR: return "STAT: Modifying str.";
           case CON_STAT_INT: return "STAT: Modifying int.";
           case CON_STAT_WIS: return "STAT: Modifying wis.";
           case CON_STAT_DEX: return "STAT: Modifying dex.";
           case CON_STAT_CON: return "STAT: Modifying con.";
           case CON_STAT_AGE: return "STAT: Selecting new age.";
          case CON_STAT_SIZE: return "STAT: Selecting a height.";
   case CON_CONFIRM_NEW_NAME: return "Confirming a new name.";
   case CON_GET_NEW_PASSWORD: return "Choosing a new password.";
                        case
    CON_CONFIRM_NEW_PASSWORD: return "Confirming a new password.";
         case CON_SHOW_INTRO: return "Reading the gen intro.";
      case CON_CHAR_GEN_NAME: return "Entering a name.";
     case CON_CHAR_GEN_EMAIL: return "Entering an email.";
          case CON_GEN_RACES: return "Choosing race.";
       case CON_CHAR_GEN_SEX: return "Choosing sex.";
           case CON_GET_NAME: return "Typing a name.";
   case CON_GET_OLD_PASSWORD: return "Entering a password.";
    case CON_CHAR_GEN_RACE_CONFIRM: return "Confirming a race.";
                     default: return "?? Connection ??";
    }
}



/*
 *  Syntax:  users
 */
void cmd_users( PLAYER_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    CONNECTION_DATA *d;
    int count;

    count	= 0;
    buf[0]	= '\0';
    for ( d = connection_list; d != NULL; d = d->next )
    {
    if ( d->character != NULL && can_see( ch, d->character ) )
	{
        count++;
        sprintf( buf2, "[%2d %2d %2d] %s@%s",
		d->connection,
		d->connected,
        d->character ? d->character->timer : 0,
        d->original  ? STR(d->original,name)  :
        d->character ? STR(d->character,name) : "(none)",
		d->host
		);

        sprintf( buf + strlen(buf), "%-53s%-25s\n\r",
                 buf2, connect_string( d->connected ) );
	}
    }

    sprintf( buf2, "%d user%s\n\r", count, count == 1 ? "" : "s" );
    send_to_actor( buf2, ch );
    send_to_actor( buf, ch );
    return;
}


/*
 *  Syntax:  sockets
 */
void cmd_sockets( PLAYER_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    PLAYER_DATA *tch;
    CONNECTION_DATA *d;
    int count;

    count	= 0;
    buf[0]	= '\0';
    for ( tch = actor_list; tch != NULL; tch = tch->next )
    {
    if ( !IS_NPC(tch) && can_see( ch, tch ) )
	{
        count++;
        if ( (d = tch->desc) != NULL )
        {
        sprintf( buf + strlen(buf), "[%2d %2d %2d] %s@%s\n\r",
		d->connection,
		d->connected,
        d->character ? d->character->timer : 0,
        d->original  ? STR(d->original,name)  :
        d->character ? STR(d->character,name) : "(none)",
        d->host  );
        }
        else
        {
        sprintf( buf + strlen(buf), "[      %2d] %16s %5d\n\r",
        tch->timer,
        STR(tch,name),
        tch->in_scene != NULL ? tch->in_scene->vnum : 0 );
        }
	}
    }

    sprintf( buf2, "%d user%s\n\r", count, count == 1 ? "" : "s" );
    send_to_actor( buf2, ch );
    send_to_actor( buf, ch );
    return;
}




void count_scenes( ZONE_DATA *pZone, int *scenes, int *unfinished )
{
     SCENE_INDEX_DATA *pSceneIndex;
     int iHash;

     for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	for ( pSceneIndex = scene_index_hash[iHash]; pSceneIndex; pSceneIndex = pSceneIndex->next )
	    if ( pSceneIndex->zone == pZone )
	    {
		if ( pSceneIndex->description[0] == '\0' 
                  && pSceneIndex->template == 0 )
		    (*unfinished)++;
		(*scenes)++;
	    }
}

/*
 * Syntax:  zones
 */
void cmd_zones( PLAYER_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    ZONE_DATA *pZone;
    int total_scenes = 0, total_unfinished = 0, s_scenes = 0, s_unfinished = 0;

    snprintf( buf, MAX_STRING_LENGTH, "[%2s] %-15s S %-6s-%6s %-12s %-3s/%-3s %-18.18s\n\r",
    "#N", "zone Name", "lvnum", "uvnum", "Filename", "Rm", "NoD", "Builders" );
    send_to_actor( buf, ch );

    for ( pZone = zone_first; pZone != NULL; pZone = pZone->next )
    {
	
    count_scenes( pZone, &s_scenes, &s_unfinished );
    snprintf( buf, MAX_STRING_LENGTH, "[%2d] %-15s %1d %6d-%-6d %-12s %3d/%-3d %-15s\n\r",
        pZone->vnum,
        trunc_fit(pZone->name,15),
        pZone->security,
        pZone->lvnum,
        pZone->uvnum,
        pZone->filename,
	s_scenes, s_unfinished,
        pZone->builders );
	send_to_actor( buf, ch );
   	total_scenes 		+= s_scenes;
	total_unfinished 	+= s_unfinished;
	s_scenes = s_unfinished = 0;
    }
    snprintf( buf, MAX_STRING_LENGTH, "\n\rTotal: %d scenes, %d unfinished (%d%%)\n\r",
	total_scenes, total_unfinished,
        total_unfinished * 100 / UMAX(1,total_scenes) );
    send_to_actor( buf, ch );

    return;
}



extern const struct board_data board_table [MAX_BOARD];

/*
 * Syntax:  table
 *          table [type]
 */
void cmd_table( PLAYER_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int x;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_actor( "Tables to view:\n\r", ch );
        send_to_actor( "race coin color board attack liquid goods components\n\r", ch );
        send_to_actor( "str int wis dex con groups skills bits language spells\n\r", ch );
        send_to_actor( "positions NYI: spells abilities aliases\n\r", ch );
        return;
    }

    if ( !str_prefix( arg, "positions" ) )
    {
        for ( x = 0;  x <= MAX_POSITION;  x++ )
         {
                  snprintf( buf, MAX_STRING_LENGTH, "[%3d] %s\n\r", x, position_name( x ) );
                  send_to_actor( buf, ch );
         }
         return;
    }

    if ( !str_prefix( arg, "race" ) )
    {

        int race;

        send_to_actor( "[Num] Name         - Siz Age Bth Scene  Sks Sr In Ws Dx Co Bonuses\n\r", ch );

        for ( x = 0; x < MAX_RACE; x++ )
        {
            race = race_lookup( x );

            snprintf( buf, MAX_STRING_LENGTH, "[%3d] %-12s  %3d %3d %3d %5d %3d %2d %2d %2d %2d %2d %s\n\r",
                          x,
                          RACE(race,race_name),
                          RACE(race,size),
                          RACE(race,base_age),
                          RACE(race,start_age),
                          RACE(race,start_scene),
                          RACE(race,max_skills),
                          RACE(race,start_str),
                          RACE(race,start_int),
                          RACE(race,start_wis),
                          RACE(race,start_dex),
                          RACE(race,start_con),
                          bonus_bit_name( RACE(race,bonus_bits) ) );
            send_to_actor( buf, ch );
        }
        return;
    }

    if ( !str_prefix( arg, "spells" ) ) {
         send_to_actor( "Valid spell targets: offensive, defensive, ignore, prop, self\n\r\n\r", ch );
    }

    if ( !str_prefix( arg, "language" ) )
    {
        send_to_actor( "[Num] Language     GSN\n\r", ch );

        for ( x = 0; x < MAX_LANGUAGE; x++ )
        {
            snprintf( buf, MAX_STRING_LENGTH, "[%3d] %12s  %3d\n\r",
                          x,
                          lang_table[x].name,
                          lang_table[x].pgsn );
            send_to_actor( buf, ch );
        }
        return;
    }

    if ( !str_prefix( arg, "coin" ) )
    {
        send_to_actor( "[Num] Mult Convert  Wgt Short Long\n\r", ch );
        for ( x = 0; x < MAX_COIN; x++ )
        {
            snprintf( buf, MAX_STRING_LENGTH, "[%3d] %4d %7d %4d %5s %s\n\r",
                     x,
                     coin_table[x].multiplier,
                     coin_table[x].convert,
                     coin_table[x].weight,
                     coin_table[x].short_name,
                     coin_table[x].long_name   );
            send_to_actor( buf, ch );
        }
        return;
    }

    if ( !str_prefix( arg, "color" ) )
    {
        if ( !IS_SET(ch->act2, PLR_ANSI) )
        {
            send_to_actor( "You don't have ansi support activated.\n\r", ch );
            return;
        }

            send_to_actor( "[Num] Code   Code Name\n\r", ch );
        for ( x = 0; x < 20; x++ )
        {
            snprintf( buf, MAX_STRING_LENGTH, "[%3d] %sSAMPLE%s %4s %s\n\r",
                          x,
                          color_table[x].code,
                          NTEXT,
                          color_table[x].act_code,
                          color_table[x].name );
            send_to_actor( buf, ch );
        }
        return;
    }

    if ( !str_prefix( arg, "board" ) )
    {
        send_to_actor( "[Num] Read  Write Filename      Name\n\r", ch );
        for ( x = 0; x < MAX_BOARD; x++ )
        {
            snprintf( buf, MAX_STRING_LENGTH, "[%3d] %5d %5d %-13s %s\n\r",
                          x,
                          board_table[x].readlevel,
                          board_table[x].writelevel,
                          board_table[x].filename,
                          board_table[x].name );
            send_to_actor( buf, ch );
        }
        return;
    }

    if ( !str_prefix( arg, "attack" ) )
    {
        send_to_actor( "[Num] Slot WpSN Hit_fun Name\n\r", ch );
        for ( x = 0; x < MAX_ATTACK; x++ )
        {
            snprintf( buf, MAX_STRING_LENGTH, "[%3d] %4d %4d %6s  %s\n\r",
                          x,
                          attack_table[x].wpgsn,
                          attack_table[x].hit_type,
                          attack_table[x].hit_fun ? "Yes" : "No",
                          attack_table[x].name );
            send_to_actor( buf, ch );
        }
        return;
    }

    if ( !str_prefix( arg, "liquid" ) )
    {
        send_to_actor( "[Num] Drunk Full Thirst Name            Color\n\r", ch );
        for ( x = 0; x < LIQ_MAX; x++ )
        {
            snprintf( buf, MAX_STRING_LENGTH, "[%3d] %5d %4d %6d %-15s %s\n\r",
                     x,
                     liq_table[x].liq_bonus[0],
                     liq_table[x].liq_bonus[1],
                     liq_table[x].liq_bonus[2],
                     liq_table[x].liq_name,
                     liq_table[x].liq_color );
            send_to_actor( buf, ch );
        }
        return;
    }

    if ( !str_prefix( arg, "str" ) )
    {
        send_to_actor( "[Str] Hit Dam Carry Wield\n\r", ch );
        for ( x = 0; x < 26; x++ )
        {
            snprintf( buf, MAX_STRING_LENGTH, "[%3d] %3d %3d %5d %5d\n\r",
                          x,
                          str_app[x].tohit,
                          str_app[x].todam,
                          str_app[x].carry,
                          str_app[x].wield );
            send_to_actor( buf, ch );
        }
        return;
    }
               
    if ( !str_prefix( arg, "int" ) )
    {
        send_to_actor( "[Int] Learn\n\r", ch );
        for ( x = 0; x < 26; x++ )
        {
            snprintf( buf, MAX_STRING_LENGTH, "[%3d] %5d\n\r",
                          x,
                          int_app[x].learn );
            send_to_actor( buf, ch );
        }
        return;
    }

    if ( !str_prefix( arg, "wis" ) )
    {
        send_to_actor( "[Wis] Practice\n\r", ch );
        for ( x = 0; x < 26; x++ )
        {
            snprintf( buf, MAX_STRING_LENGTH, "[%3d] %8d\n\r",
                          x,
                          wis_app[x].practice );
            send_to_actor( buf, ch );
        }
        return;
    }

    if ( !str_prefix( arg, "dex" ) )
    {
        send_to_actor( "[Dex] Defensive\n\r", ch );
        for ( x = 0; x < 26; x++ )
        {
            snprintf( buf, MAX_STRING_LENGTH, "[%3d] %9d\n\r",
                          x,
                          dex_app[x].defensive );
            send_to_actor( buf, ch );
        }
        return;
    }

    if ( !str_prefix( arg, "con" ) )
    {
        send_to_actor( "[Con] HpDiff Hits Shock Res\n\r", ch );
        for ( x = 0; x < 26; x++ )
        {
            snprintf( buf, MAX_STRING_LENGTH, "[%3d] %6d %4d %5d %3d\n\r",
                          x,
                          con_app[x].hitp,
                          con_app[x].hitp + 100,
                          con_app[x].shock,
                          con_app[x].resistance );
            send_to_actor( buf, ch );
        }
        return;
    }

    if ( !str_prefix( arg, "groups" ) )
    {
        send_to_actor( "[Gro] Code GSN Name\n\r", ch );
        for ( x = 0; x < MAX_GROUP; x++ )
        {
            snprintf( buf, MAX_STRING_LENGTH, "[%3d] %4d %3d %s\n\r",
                          x,
                          group_table[x].group_code,
                          group_table[x].pgsn,
                          group_table[x].name );
            send_to_actor( buf, ch );
        }
        return;
    }

    if ( !str_prefix( arg, "bits" ) )
    {
        send_to_actor( "SEDIT/REDIT flags (not scene terrain or sector types):\n\r ", ch );
        send_to_actor( scene_bit_name( -1 ), ch );
        send_to_actor( "\n\r", ch );
        send_to_actor( "'Affects' also known as bonuses or penalties, AEDIT and bonus():\n\r", ch );
        send_to_actor( bonus_bit_name( -1 ), ch );
        send_to_actor( "\n\r", ch );
        send_to_actor( "AEDIT/MEDIT bits:\n\r", ch );
        send_to_actor( act_bit_name( -1 ), ch );
        send_to_actor( "\n\r", ch );
        send_to_actor( "Player bits settable using mset:\n\r", ch );
        send_to_actor( plr_bit_name( -1 ), ch );
        send_to_actor( "\n\r", ch );
        send_to_actor( "Extra flags used in PEDIT/OEDIT:\n\r", ch );
        send_to_actor( extra_bit_name( -1 ), ch );
        send_to_actor( "\n\r", ch );
        send_to_actor( "Bits which refer to wearable locations on props (PEDIT) including take flag:\n\r", ch );
        send_to_actor( wear_bit_name( -1 ), ch );
        send_to_actor( "\n\r", ch );
        send_to_actor( "Bits used on exits (used in SEDIT/REDIT):\n\r", ch );
        send_to_actor( exit_bit_name( -1 ), ch );
        send_to_actor( "\n\r", ch );
        send_to_actor( "Bits used in the shop command of the AEDIT/MEDIT:\n\r", ch );
        send_to_actor( shop_bit_name( -1 ), ch );
        send_to_actor( "\n\r", ch );
        return;
    }


    if ( !str_prefix( arg, "components" ) )
    {
        char final[MAX_STRING_LENGTH*8];

        sprintf( final, "[Num] %-23s %-21s [Tp]  Cost\n\r",
                 "Component Name", "Sell Unit (usage)" );

        for ( x = 0;  comp_table[x].type != -1; x++ )
        {
            snprintf( buf, MAX_STRING_LENGTH, "[%3d] %-23s %-21s [%2d] %5d\n\r",
                     x,
                     comp_table[x].name,
                     comp_table[x].usage,
                     comp_table[x].type,
                     comp_table[x].cost );
            strcat( final, buf );
        }

        page_to_actor( final, ch );
        return;
    }

    if ( !str_prefix( arg, "goods" ) )
    {
        char final[MAX_STRING_LENGTH*2];

        sprintf( final, "[Num] %-20s %-8s Wt [%-6s] [Lf]  Cost\n\r",
                 "Name", "Unit", "Type" );

        for ( x = 0;  goods_table[x].code >= 0; x++ )
        {
            snprintf( buf, MAX_STRING_LENGTH, "[%3d] %-20s %-8s %2d [%-6s] [%2d] %5d\n\r",
                     x,
                     goods_table[x].name,
                     goods_table[x].unit,
                     goods_table[x].weight,
                     name_good_code( goods_table[x].code ),
                     goods_table[x].life,
                     goods_table[x].cost );
            strcat( final, buf );
        }

        page_to_actor( final, ch );
        return;
    }

    cmd_table( ch, "" );
    return;
}



