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
#include <string.h>
#include <time.h>

#include <zlib.h>

#include "nimud.h"
#include "defaults.h"



void update_money( PROP *prop )
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];

    if ( prop->item_type != ITEM_MONEY )
    {
        bug( "update_money: item is not of type money.", 0 );
        return;
    }

    if ( prop->value[1] >= MAX_COIN ) prop->value[1] = MAX_COIN-1;
    if ( prop->value[1] < 0 ) prop->value[1] = 0;

    if ( prop->value[0] > 1  )
    {
    prop->pIndexData = get_prop_template( PROP_VNUM_MONEY_SOME );
    if ( prop->value[0] <= 10 )       strcpy( buf, "few"               ); else
    if ( prop->value[0] <= 100 )      strcpy( buf, "small pile of"     ); else
    if ( prop->value[0] <= 1000 )     strcpy( buf, "pile of"           ); else
    if ( prop->value[0] <= 10000 )    strcpy( buf, "large pile of"     ); else
    if ( prop->value[0] <= 100000 )   strcpy( buf, "heaping mound of"  ); else
    if ( prop->value[0] <= 1000000 )  strcpy( buf, "small hill of"     ); else
    if ( prop->value[0] <= 10000000 ) strcpy( buf, "mountain of"       );
                                else strcpy( buf, "whole shitload of" );

    buf1[0] ='\0';
    strcat(buf1, numberize(prop->value[0]));
    strcat(buf1, " ");
    strcat(buf1, coin_table[prop->value[1]].long_name );
    strcat(buf1, " coin" );
    if ( prop->value[0] > 1 ) strcat(buf1,"s");

    sprintf( buf2, prop->pIndexData->description, buf,
                   coin_table[prop->value[1]].long_name );
    free_string( prop->name );        prop->name        = str_dup( buf1 );
    free_string( prop->description ); prop->description = str_dup( buf2 );
    free_string( prop->short_descr ); prop->short_descr = str_dup( buf1 );
    prop->weight         = (prop->value[0] * coin_table[prop->value[1]].weight) / 20;
    prop->cost           = prop->value[0] * coin_table[prop->value[1]].convert;
    }
    else
    if ( prop->value[0] == 1 )
    {
    prop->pIndexData = get_prop_template( PROP_VNUM_MONEY_ONE );
    sprintf( buf1, prop->pIndexData->short_descr,
                   coin_table[prop->value[1]].long_name );
    sprintf( buf2, prop->pIndexData->description,
                   coin_table[prop->value[1]].long_name );
    sprintf( buf3, prop->pIndexData->name, coin_table[URANGE(0,prop->value[1],MAX_COIN-1)].long_name);
    free_string( prop->name );           prop->name        = str_dup( buf3 );
    free_string( prop->description );    prop->description = str_dup( buf2 );
    free_string( prop->short_descr );    prop->short_descr = str_dup( buf1 );
    prop->weight         = coin_table[prop->value[1]].weight;
    prop->cost           = coin_table[prop->value[1]].convert;
    }

    return;
}

PROP *create_money( int amount, int type )
{
    PROP *prop;

    if ( amount <= 0 )
    {
    bug( "Create_money: zero or negative money %d.", amount );
    amount = 1;
    }

    if ( amount == 1 )
    {
    prop = create_prop( get_prop_template( PROP_VNUM_MONEY_ONE ), 0 );
    prop->item_type      = ITEM_MONEY;
    prop->value[0]       = 1;
    prop->value[1]       = URANGE(0,type,MAX_COIN-1);
    prop->wear_loc       = -1;
    prop->weight         = coin_table[type].weight;
    prop->cost           = coin_table[type].convert;
    update_money( prop );
    }
    else
    {
	prop = create_prop( get_prop_template( PROP_VNUM_MONEY_SOME ), 0 );
    prop->value[0]       = amount;
    prop->value[1]       = type;
    prop->item_type      = ITEM_MONEY;
    prop->wear_loc       = -1;
    prop->weight         = amount * coin_table[type].weight / 16;
    prop->cost           = amount * coin_table[type].convert;
    update_money( prop );
    }

    return prop;
}

/*
 * Conglomerate several money items.
 */
void merge_money( PLAYER *ch )
{
    PROP *prop;
    PROP *prop_next;
    int sub_totals[MAX_COIN];
    int type;

    for ( type = 0; type < MAX_COIN; type++ )
    {
        sub_totals[type] = tally_one_coin( ch->carrying, type, FALSE );
    }

    for ( prop = ch->carrying;  prop != NULL;  prop = prop_next )
    {
        prop_next = prop->next_content;
        if ( prop->item_type == ITEM_MONEY )
        {
            prop_from_actor( prop );
            extractor_prop( prop );
        }
    }

    for ( type = 0; type < MAX_COIN; type++ )
    {
        if ( sub_totals[type] > 0 )
        {
            prop_to_actor( (prop = create_money( sub_totals[type], type )), ch );
            prop->wear_loc = hand_empty( ch );
        }
    }
    return;
}

void merge_money_prop( PROP *prop )
{
    PROP *iprop;
    PROP *prop_next;
    int sub_totals[MAX_COIN];
    int type;

    for ( type = 0; type < MAX_COIN; type++ )
    {
        sub_totals[type] = tally_one_coin( prop->contains, type, FALSE );
    }

    for ( iprop = prop->contains;  iprop != NULL;  iprop = prop_next )
    {
        prop_next = iprop->next_content;
        if ( iprop->item_type == ITEM_MONEY )
        {
//            bug( "merge_money_prop: here", 0 );
            prop_from_prop( iprop );
            extractor_prop( iprop );
        }
    }

    for ( type = 0; type < MAX_COIN; type++ )
    {
        if ( sub_totals[type] > 0 )
          prop_to_prop( create_money( sub_totals[type], type ), prop );
    }
    return;
}


void merge_money_scene( SCENE *scene )
{
    PROP  *iprop;
    PROP *prop_next;
    int sub_totals[MAX_COIN];
    int type;

    for ( type = 0; type < MAX_COIN; type++ )
    {
        sub_totals[type] = tally_one_coin( scene->contents, type, FALSE );
    }

    for ( iprop = scene->contents;  iprop != NULL;  iprop = prop_next )
    {
        prop_next = iprop->next_content;
        if ( iprop->item_type == ITEM_MONEY )
        {
            prop_from_scene( iprop );
            extractor_prop( iprop );
        }
    }

    for ( type = 0; type < MAX_COIN; type++ )
    {
        if ( sub_totals[type] > 0 )
          prop_to_scene( create_money( sub_totals[type], type ), scene );
    }
    return;
}

/*
 * Create a set of 'money' props.
 */
void create_amount( int amount, PLAYER *pActor, SCENE *pScene,
                    PROP *pProp )
{
    PROP *prop;
    int type;

    if ( amount <= 0 )
    return;

    if ( amount == 1 )
    {
    prop = create_money( 1, MAX_COIN-1 );
    if ( pActor ) prop_to_actor_money( prop, pActor );
    if ( pScene ) prop_to_scene( prop, pScene );
    if ( pProp ) prop_to_prop( prop, pProp );
    }
    else
    if ( amount < 30 && (amount % 10 != 0) )
    {
    prop = create_money( amount, MAX_COIN-1 );
    if ( pActor ) prop_to_actor_money( prop, pActor );
    if ( pScene ) prop_to_scene( prop, pScene );
    if ( pProp ) prop_to_prop( prop, pProp );
    }
    else
    {
        for ( type = 0; type < MAX_COIN; type++ )
        {

        if ( amount/coin_table[type].convert <= 0 )
            continue;

        prop = create_money( amount/coin_table[type].convert, type );
        amount %= coin_table[type].convert;

        if ( pActor ) prop_to_actor_money( prop, pActor );
        if ( pScene ) prop_to_scene( prop, pScene );
        if ( pProp ) prop_to_prop( prop, pProp );
        }
    }

    if ( pActor ) merge_money( pActor );
    if ( pProp ) merge_money_prop( pProp );
    return;
}


/*
 * Tally the number of coins a character has.
 */
int tally_coins( PLAYER *actor )
{
    PROP *prop;
    int total = 0;

    for ( prop = actor->carrying;  prop != NULL; prop = prop->next_content )
    {
        if ( prop->item_type == ITEM_MONEY
          && prop->value[1] >= 0
          && prop->value[1] < MAX_COIN )
        total += prop->value[0] * coin_table[prop->value[1]].convert;

        if ( ( prop->value[1] < 0 || prop->value[1] >= MAX_COIN ) 
          && prop->item_type == ITEM_MONEY )
        {
        bug( "Tally_coins: prop (dbkey %d) bad coinage",
             prop->pIndexData->dbkey );
        prop->value[1] = URANGE( 0, prop->value[1], MAX_COIN-1 );
        }

        if ( prop->item_type == ITEM_CONTAINER )
        {
            PROP *iprop;

            for ( iprop = prop->contains; iprop != NULL; iprop = iprop->next_content )
            {
                if ( iprop->item_type == ITEM_MONEY
                  && iprop->value[1] >= 0
                  && iprop->value[1] < MAX_COIN )
                total += iprop->value[0] * coin_table[iprop->value[1]].convert;

                if ( ( iprop->value[1] < 0 || iprop->value[1] >= MAX_COIN )
                  && iprop->item_type == ITEM_MONEY )
                {
                bug( "Tally_coins: iprop (dbkey %d) bad coinage",
                     iprop->pIndexData->dbkey );
                iprop->value[1] = URANGE( 0, iprop->value[1], MAX_COIN-1 );
                }
            }
        }
    }

    return total;
}

/*
 * Tally the number of A SINGLE TYPE OF coin a character has.
 */
int tally_one_coin( PROP *plist, int type, bool fContents )
{
    PROP *prop;
    int total = 0;

    for ( prop = plist;  prop != NULL; prop = prop->next_content )
    {
        if ( prop->item_type == ITEM_MONEY
          && prop->value[1] >= 0
          && prop->value[1] < MAX_COIN
          && prop->value[1] == type )
        total += prop->value[0];

        if ( ( prop->value[1] < 0 || prop->value[1] >= MAX_COIN ) 
           && prop->item_type == ITEM_MONEY )
        {
        bug( "Tally_one_coin: prop (dbkey %d) bad coinage",
             prop->pIndexData->dbkey );
        prop->value[1] = URANGE( 0, prop->value[1], MAX_COIN-1 );
        }

        if ( prop->item_type == ITEM_CONTAINER
           && fContents )
        {
            PROP *iprop;

            for ( iprop = prop->contains; iprop != NULL;
                  iprop = iprop->next_content )
            {
                if ( iprop->item_type == ITEM_MONEY
                  && iprop->value[1] >= 0
                  && iprop->value[1] < MAX_COIN
                  && iprop->value[1] == type )
                total += iprop->value[0];

                if ( ( iprop->value[1] < 0 || iprop->value[1] >= MAX_COIN )
                  && iprop->item_type == ITEM_MONEY )
                {
                bug( "Tally_one_coin: iprop (dbkey %d) bad coinage",
                     iprop->pIndexData->dbkey );
                iprop->value[1] = URANGE( 0, iprop->value[1], MAX_COIN-1 );
                }
            }
        }
    }

    return total;
}


/*
 * Create a cost out of the blue.
 */
char *name_amount( int amount )
{
    static char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int type;

    if ( amount < 20 && amount != 0 )
    {
    snprintf( buf, MAX_STRING_LENGTH, "%d copper", amount );
    return buf;
    }

    buf[0] = '\0';

    for ( type = 0; type<MAX_COIN; type++ )
    {
    if ( amount == 0 )
        break;

    if ( amount/coin_table[type].convert <= 0 )
        continue;

    sprintf( buf2, "%s%s %s", buf[0] != '\0' ? ", " : "",
                              numberize( (amount/coin_table[type].convert) ),
                              coin_table[type].long_name );
    strcat( buf, buf2 );
    amount %= coin_table[type].convert;
    }
    if ( buf[0] == '\0' ) snprintf( buf, MAX_STRING_LENGTH, "nothing" );

    return buf;
}

/*
 * Nail unused money items.
 */
void strip_empty_money( PLAYER *ch )
{
    PROP *prop;
    PROP *prop_next;
    PROP *iprop;
    PROP *iprop_next;

    for ( prop = ch->carrying;  prop != NULL;  prop = prop_next )
    {
        prop_next = prop->next_content;
        if ( prop->item_type == ITEM_MONEY  && prop->value[0] <= 0 )
        {
            prop_from_actor( prop );
            extractor_prop( prop );
        }

        for ( iprop = prop->contains;  iprop != NULL; iprop = iprop_next )
        {
        iprop_next = iprop->next_content;
        if ( iprop->item_type == ITEM_MONEY  && iprop->value[0] <= 0 )
        {
            prop_from_prop( iprop );
            extractor_prop( iprop );
        }
        }
    }
    return;
}
     

/*
 * Ok, here's the tricky part... managing two money totals at once,
 * this is the routine that makes the change if you give the dude
 * too much.  This is for shops mainly, but can be used elsewhere.
 */
char *sub_coins( int amount, PLAYER *ch )
{
    PROP *prop;
    PROP *iprop;
    int money = 0;

    if ( tally_coins( ch ) < amount )
    {
        bug( "Sub_coins: ch doesn't have enough money (%d)", amount );
        return "nothing";
    }

    for ( prop = ch->carrying;  prop != NULL;  prop = prop->next_content )
    {
        if ( money >= amount ) break;
        if ( prop->item_type == ITEM_MONEY )
        {
            while ( prop->value[0] > 0 && money < amount )
            {
                 money += coin_table[prop->value[1]].convert;
                 prop->value[0]--;
            }
            if ( prop->value[0] > 0 ) update_money( prop );
        }

        for ( iprop = prop->contains;  iprop != NULL;  iprop = iprop->next_content )
        {
        if ( money >= amount ) break;
        if ( iprop->item_type == ITEM_MONEY )
        {
            while ( iprop->value[0] > 0 && money < amount )
            {
                 money += coin_table[iprop->value[1]].convert;
                 iprop->value[0]--;
            }
            if ( iprop->value[0] > 0 ) update_money( iprop );
        }
        }
    }

    strip_empty_money( ch );
    if ( money > amount ) create_amount( money - amount, ch, NULL, NULL );
    return name_amount( money - amount );
}



void cmd_money( PLAYER *ch, char *argument )
{
     PLAYER *och;
     int worth=tally_coins(ch);

     if ( worth > 0 ) {
      send_to_actor( "You have ", ch );
      send_to_actor( name_amount( worth ), ch );
      send_to_actor( ".\n\r", ch );
     }
     if ( ch->credits > 0 )
     var_to_actor( "Your ^6St^1^Bar^1^N^Bcard^N still has %d credits on it.\n\r", ch, ch->credits );
     if ( ch->bucks > 0 )
     var_to_actor( "You've got ^2$^B%d^N.\n\r", ch, ch->bucks );

  if ( argument != NULL )
 for( och=actor_list; och!=NULL; och=och->next ) if ( och->master == ch && NPC(och) ) {
     worth=tally_coins(och);
     if ( worth > 0 ) 
     var_to_actor( "%s has %s.\n\r", ch, NAME(och), name_amount( worth ) );
     
     if ( och->credits > 0 )
     var_to_actor( "%s's ^6St^1^Bar^1^N^Bcard^N still has %d credits on it.\n\r", ch, NAME(och), och->credits );
     if ( och->bucks > 0 )
     var_to_actor( "%s has ^2$^B%d^N.\n\r", ch, NAME(och), och->bucks );
 }

     return;
}
