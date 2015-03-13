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
#include "net.h"


/*
 * Syntax:  mload [dbkey]
 */
void cmd_mload( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ACTOR_TEMPLATE *pActorIndex;
    PLAYER *victim;
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
    send_to_actor( "Syntax: mload <dbkey>.\n\r", ch );
	return;
    }

    if ( ( pActorIndex = get_actor_template( atoi( arg ) ) ) == NULL )
    {
    send_to_actor( "No actor has that dbkey.\n\r", ch );
	return;
    }

    victim = create_actor( pActorIndex );
    actor_to_scene( victim, ch->in_scene );
    act( "$n has created $N!", ch, NULL, victim, TO_SCENE );
    act( "You create $N!", ch, NULL, victim, TO_ACTOR );
    return;
}



/*
 * Syntax:  oload [dbkey]
 *          oload [dbkey] [level]
 *          oload comp [num]
 *          oload good [num]
 */
void cmd_oload( PLAYER *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    PROP_TEMPLATE *pPropIndex;
    PROP *prop;
    int level;
 
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
 
    if ( arg1[0] == '\0' )
    {
        send_to_actor( "Syntax: pload <dbkey> <level>.\n\r", ch );
        send_to_actor( "        pload good <number>.\n\r", ch );
        send_to_actor( "        pload comp <number>.\n\r", ch );
        return;
    }
 
    if ( arg2[0] == '\0' )
    {
	level = get_trust( ch );
    }
    else
    {
	/*
	 * New feature by Locke.
	 */
        if ( !is_number( arg1 ) )
        {

            level = atoi( arg2 );

            if ( !str_prefix( arg1, "good" ) ) {
                prop = create_good( level );                 
         	prop_to_actor( prop, ch );
                prop->wear_loc     = hand_empty( ch );
                act( "$n has created $p!", ch, prop, NULL, TO_SCENE );
                act( "You create $p!", ch, prop, NULL, TO_ACTOR );
                return;
            }

            if ( !str_prefix( arg1, "comp" ) ) {
                prop = create_comp( level );                 
           	prop_to_actor( prop, ch );
                prop->wear_loc     = hand_empty( ch );
                act( "$n has created $p!", ch, prop, NULL, TO_SCENE );
                act( "You create $p!", ch, prop, NULL, TO_ACTOR );
                return;
            }

            send_to_actor( "Syntax: oload <dbkey> <level>.\n\r", ch );
            send_to_actor( "        oload good <number>.\n\r", ch );
            send_to_actor( "        oload comp <number>.\n\r", ch );
	    return;
        }

              level = atoi( arg2 );

	if ( level < 0 || level > get_trust( ch ) )
        {
        send_to_actor( "Limited to your trust level.\n\r", ch );
	    return;
        }
    }

    if ( ( pPropIndex = get_prop_template( atoi( arg1 ) ) ) == NULL )
    {
    send_to_actor( "No prop has that dbkey.\n\r", ch );
	return;
    }

    prop = create_prop( pPropIndex, level );
    if ( CAN_WEAR(prop, ITEM_TAKE) )
    {
	prop_to_actor( prop, ch );
        prop->wear_loc     = hand_empty( ch );
    }
    else
    {
	prop_to_scene( prop, ch->in_scene );
    }
    act( "$n has created $p!", ch, prop, NULL, TO_SCENE );
    act( "You create $p!", ch, prop, NULL, TO_ACTOR );
    return;
}



/*
 * Syntax:  load actor [dbkey]
 *          load prop [dbkey]
 *          load prop [dbkey] [level]
 */
void cmd_load( PLAYER *ch, char *argument )
{
    char arg[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg );
    
     if ( !str_prefix( arg, "actor" ) ) cmd_mload( ch, argument );
else if ( !str_prefix( arg, "prop" ) ) cmd_oload( ch, argument );
else send_to_actor( "Syntax: Load <actor|prop> <dbkey>\n\r", ch );

     return;
}





/*
 * Syntax:  sset [person] ['skill'] [value/max]
 *          sset [person] all
 */
void cmd_sset( PLAYER *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    PLAYER *victim;
    SKILL *pIndex=NULL;
    int value;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        send_to_actor( "Syntax: sset <victim> '<skill>' <value|max>\n\r",  ch );
        send_to_actor( "or:     sset <victim> all       <value|max>\n\r",  ch );
        send_to_actor( "Skill being any skill or spell.\n\r",              ch );
        return;
    }

    if ( ( victim = get_actor_world( ch, arg1 ) ) == NULL )
    {
        send_to_actor( "They aren't here.\n\r", ch );
        return;
    }

    if ( NPC(victim) )
    {
        send_to_actor( "Not on NPC's.\n\r", ch );
        return;
    }

    fAll = !str_cmp( arg2, "all" );
    if ( !fAll && !( pIndex = skill_lookup( arg2 ) ) )
    {
        send_to_actor( "No such skill.\n\r", ch );
        return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
        if ( !str_cmp( arg3, "max" ) )
        {
            if ( fAll )
            {
                int sn;

                for ( sn = 0; sn <= top_dbkey_skill; sn++ )
                {
                    SKILL *pSkill;

                    if ( (pSkill=get_skill_index(sn)) != NULL )
                    update_skill(victim,sn,pSkill->max_learn);
                }
                send_to_actor( NAME(ch), victim );
                send_to_actor( " bestows you with great knowledge.\n\r", victim );
                send_to_actor( "You bestow great knowledge.\n\r", ch );
            }
            else update_skill(victim,pIndex->dbkey,pIndex->max_learn);

            send_to_actor( "Skill(s) set to max_learn value.\n\r", ch );
            return;
        }

        send_to_actor( "Value must be numeric.\n\r", ch );
        return;
    }

    value = atoi( arg3 );
    if ( value <= -1 || value > 100 )
    {
        send_to_actor( "Value range is 0 (unlearned) to 100.\n\r", ch );
        return;
    }

    if ( fAll )
    {
        int sn;

        for ( sn = 0; sn <= top_dbkey_skill; sn++ )
        {
            SKILL *pSkill;

            if ( (pSkill=get_skill_index(sn)) )
            update_skill(victim,sn,value);
        }
    }
    else update_skill(victim,pIndex->dbkey,value);
    if ( !fAll) send_to_actor( pIndex->name, ch );
    send_to_actor( " adjusted.\n\r", ch );
    return;
}



/*
 * Syntax:  mset [person] [field] [value]
 */
void cmd_mset( PLAYER *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    PLAYER *victim;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
    send_to_actor( "Syntax: mset <victim> <field>  <value>\n\r", ch );
    send_to_actor( "or:     mset <victim> <string> <value>\n\r", ch );
    send_to_actor( "\n\r", ch );
    send_to_actor( "Field being one of:\n\r",                ch );
    send_to_actor( "  sex    class   race\n\r",              ch );
    send_to_actor( "  hp     move    timer\n\r",             ch );
    send_to_actor( "  master  practice\n\r",                 ch );
    send_to_actor( "  act    affect\n\r",                    ch );
    send_to_actor( "--For Players:\n\r",                     ch );
    send_to_actor( "  str int wis dex con security\n\r",     ch );
    send_to_actor( "  race size app age\n\r",                ch );
    send_to_actor( "  thirst drunk full\n\r",                ch );
    send_to_actor( "\n\r", ch );
    send_to_actor( "String being one of:\n\r",               ch );
    send_to_actor( "  name short long description\n\r",      ch );
    send_to_actor( "--For Players:\n\r",                     ch );
    send_to_actor( "  keywords constellation\n\r",           ch );
    return;
    }

    if ( ( victim = get_actor_world( ch, arg1 ) ) == NULL )
    {
    send_to_actor( "They aren't here.\n\r", ch );
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "sex" ) && arg3[0] != '\0' )
    {
        if ( value < 0 || value > 2 )
        {
            send_to_actor( "Sex range is 0 to 2.\n\r", ch );
            return;
        }
        victim->sex = value;
        return;
    }

    if ( !str_cmp( arg2, "size" ) && arg3[0] != '\0' )
    {
    victim->size = value;
    return;
    }


    if ( !str_cmp( arg2, "timer" ) && arg3[0] != '\0' )
    {
        if ( value < 0 || value > 30000 )
        {
            send_to_actor( "Timer range is 0 to 30,000.\n\r", ch );
            return;
        }
        victim->timer = value;
        return;
    }

    if ( !str_cmp( arg2, "app" ) && arg3[0] != '\0' )
    {
        if ( value < 0 || value > 30000 )
        {
            send_to_actor( "Application time range is from 0 to 30,000.\n\r", ch );
            return;
        }
        else if ( NPC(victim) )
        {
            send_to_actor( "PCs only.\n\r", ch );
            return;
        }
        PC(victim,app_time) = value;
        return;
    }

    if ( !str_cmp( arg2, "master" ) && arg3[0] != '\0' )
    {
        if ( !str_cmp( arg3, "none" ) )
        {
        victim->master = NULL;
        return;
        }

        if ( ( get_actor_world( ch, arg3 ) ) == NULL )
        {
        send_to_actor( "They aren't here.\n\r", ch );
        return;
        }

        victim->master = get_actor_world( ch, arg3 );
        return;
    }


    if ( !str_cmp( arg2, "act" ) && arg3[0] != '\0' )
    {
        victim->flag = value;
        if ( !IS_SET( victim->flag, ACTOR_NPC ) )
             SET_BIT( victim->flag, ACTOR_NPC );
        return;
    }

    if ( !str_cmp( arg2, "affect" ) && arg3[0] != '\0' )
    {
        victim->bonuses = value;
        return;
    }

    if ( !str_cmp( arg2, "name" ) && arg3[0] != '\0' )
    {
        free_string( victim->name );
        victim->name = str_dup( arg3 );
        return;
    }

    if ( !str_cmp( arg2, "short" ) && arg3[0] != '\0' )
    {
	free_string( victim->short_descr );
	victim->short_descr = str_dup( arg3 );
	return;
    }

    if ( !str_cmp( arg2, "long" ) )
    {
        char buf[MAX_STRING_LENGTH];

        snprintf( buf, MAX_STRING_LENGTH, "%s\n\r", arg3 );
        free_string( victim->long_descr );
        victim->long_descr = str_dup( buf );
        return;
    }

    if ( !str_cmp( arg2, "description" ) )
    {
        send_to_actor( "Entering line editing mode.\n\r", ch );
        send_to_actor( "Terminate with a ~ on a blank line.\n\r", ch );
        ch->desc->pString = &victim->description;
        return;
    }

    if ( NPC(victim) )
    {
        cmd_mset( ch, "" );
        return;
    }

    /*
     * Players only.
     */
    if ( get_trust( victim ) > get_trust( ch ) )
    {
    send_to_actor( "They are trusted at a higher level than you.\n\r", ch );
    return;
    }

    if ( !str_cmp( arg2, "str" )
      && arg3[0] != '\0' )
    {
        if ( value < 3 || value > 25 )
        {
        send_to_actor( "Strength range is 3 to 25.\n\r", ch );
	    return;
        }

        victim->perm_str = value;
        return;
    }

    if ( !str_cmp( arg2, "int" ) && arg3[0] != '\0' )
    {
        if ( value < 3 || value > 25 )
	{
        send_to_actor( "Intelligence range is 3 to 25.\n\r", ch );
	    return;
	}

    victim->perm_int = value;
	return;
    }

    if ( !str_cmp( arg2, "wis" ) && arg3[0] != '\0' )
    {
        if ( value < 3 || value > 25 )
	{
        send_to_actor( "Wisdom range is 3 to 25.\n\r", ch );
	    return;
	}

    victim->perm_wis = value;
	return;
    }

    if ( !str_cmp( arg2, "dex" ) && arg3[0] != '\0' )
    {
        if ( value < 3 || value > 25 )
	{
        send_to_actor( "Dexterity range is 3 to 25.\n\r", ch );
	    return;
	}

    victim->perm_dex = value;
	return;
    }

    if ( !str_cmp( arg2, "con" ) && arg3[0] != '\0' )
    {
        if ( value < 3 || value > 25 )
	{
        send_to_actor( "Constitution range is 3 to 25.\n\r", ch );
	    return;
	}

    victim->perm_con = value;
	return;
    }

    if ( !str_cmp( arg2, "race" ) && arg3[0] != '\0' )
    {
    if ( value < 0 || value > MAX_RACE )
    {
        char buf[MAX_STRING_LENGTH];

        snprintf( buf, MAX_STRING_LENGTH, "Race range is 0 to %d.\n", MAX_RACE );
        send_to_actor( buf, ch );
        return;
    }
    victim->race = value;
    return;
    }

    if ( !str_cmp( arg2, "age" ) && arg3[0] != '\0' )
    {
    if ( NPC(ch) ) return;

    if ( value < 0 )
    {
        char buf[MAX_STRING_LENGTH];

        snprintf( buf, MAX_STRING_LENGTH, "Race range is 0 to %d.\n", MAX_RACE );
        send_to_actor( buf, ch );
        return;
    }
     PC(ch,birth_year)  = weather.year - value;
    return;
    }


    if ( !str_cmp( arg2, "security" ) && arg3[0] != '\0' )
    {
        if ( value < 1 || value > 9 )
	{
        send_to_actor( "Security level is between 1 and 9.\n\r", ch );
	    return;
	}

        if ( PC(victim,security) < PC(ch,security) )
        {
        send_to_actor( "They have a lower security level than you!\n\r", ch );
        return;
        }
    
        PC(victim,security) = UMAX(PC(ch,security), value);
        return;
    }

    if ( !str_cmp( arg2, "thirst" ) && arg3[0] != '\0' )
    {
    if ( value < -100 || value > 100 )
	{
        send_to_actor( "Thirst range is -100 to 100.\n\r", ch );
	    return;
	}

    if ( victim->userdata ) PC(victim,condition[COND_THIRST]) = value;
	return;
    }

    if ( !str_cmp( arg2, "drunk" ) && arg3[0] != '\0' )
    {
    if ( value < 0 || value > 100 )
	{
        send_to_actor( "Drunk range is 0 to 100.\n\r", ch );
	    return;
	}

    if ( victim->userdata ) PC(victim,condition[COND_DRUNK]) = value;
	return;
    }

    if ( !str_cmp( arg2, "full" ) && arg3[0] != '\0')
    {
    if ( value < -100 || value > 100 )
	{
        send_to_actor( "Full range is -100 to 100.\n\r", ch );
	    return;
	}

    if ( victim->userdata ) PC(victim,condition[COND_FULL]) = value;
	return;
    }

    if ( !str_cmp( arg2, "keywords" ) && arg3[0] != '\0' )
    {
        free_string( victim->keywords );
        victim->keywords = str_dup( arg3 );
	return;
    }

    if ( !str_cmp( arg2, "constellation" ) )
    {
        free_string( PC(victim,constellation) );
        PC(victim,constellation) = str_dup( arg3 );
        return;
    }

    /*
     * Generate usage message.
     */
    cmd_mset( ch, "" );
    return;
}



/*
 * Syntax:  oset [prop] [field] [value]
 */
void cmd_oset( PLAYER *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    PROP *prop;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
    send_to_actor( "Syntax: oset <prop> <field>  <value>\n\r", ch );
    send_to_actor( "or:     oset <prop> <string> <value>\n\r", ch );
    send_to_actor( "\n\r",                                 ch );
    send_to_actor( "Field being one of:\n\r",              ch );
    send_to_actor( "  value1, 2, 3, 4               \n\r", ch );
    send_to_actor( "  extra   wear    level   weight\n\r", ch );
    send_to_actor( "  cost    type    timer   size\n\r",   ch );
    send_to_actor( "\n\r",                                 ch );
    send_to_actor( "String being one of:\n\r",             ch );
    send_to_actor( "  name    short   long\n\r",           ch );
	return;
    }

    if ( ( prop = get_prop_world( ch, arg1 ) ) == NULL )
    {
    send_to_actor( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( (!str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" )) && arg3[0] != '\0' )
    {
    prop->value[0] = value;
    return;
    }

    if ( (!str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" )) && arg3[0] != '\0' )
    {
	prop->value[1] = value;
	return;
    }

    if ( (!str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" )) && arg3[0] != '\0' )
    {
	prop->value[2] = value;
	return;
    }

    if ( (!str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" )) && arg3[0] != '\0' )
    {
	prop->value[3] = value;
	return;
    }


    if ( !str_cmp( arg2, "extra" ) && arg3[0] != '\0' )
    {
    prop->extra_flags ^= extra_name_bit( arg3 );
	return;
    }

    if ( !str_cmp( arg2, "wear" ) && arg3[0] != '\0' )
    {
    prop->wear_flags ^= wear_name_bit( arg3 );
	return;
    }

    if ( !str_cmp( arg2, "level" ) && arg3[0] != '\0' )
    {
	prop->level = value;
	return;
    }

    if ( !str_cmp( arg2, "size" ) && arg3[0] != '\0' )
    {
    prop->size = value;
	return;
    }
	
    if ( !str_cmp( arg2, "weight" ) && arg3[0] != '\0' )
    {
	prop->weight = value;
	return;
    }

    if ( !str_cmp( arg2, "cost" ) && arg3[0] != '\0' )
    {
	prop->cost = value;
	return;
    }

    if ( !str_cmp( arg2, "timer" ) && arg3[0] != '\0' )
    {
	prop->timer = value;
	return;
    }

    if ( !str_cmp( arg2, "type" ) && arg3[0] != '\0' )
    {
    prop->item_type = item_name_type ( arg3 );
    return;
    }

    if ( !str_cmp( arg2, "name" ) && arg3[0] != '\0' )
    {
	free_string( prop->name );
	prop->name = str_dup( arg3 );
	return;
    }

    if ( !str_cmp( arg2, "short" ) && arg3[0] != '\0' )
    {
	free_string( prop->short_descr );
	prop->short_descr = str_dup( arg3 );
	return;
    }

    if ( !str_cmp( arg2, "long" ) )
    {           
        send_to_actor( "Entering line editing mode.\n\r", ch );
        send_to_actor( "Terminate with a ~ on a blank line.\n\r", ch );
        ch->desc->pString = &prop->description;
        return;
    }


    /*
     * Generate usage message.
     */
    cmd_oset( ch, "" );
    return;
}



