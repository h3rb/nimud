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
 *                           *
 * The Isles OLC copyright   *
 * 1993-1994 Herb Gilliland  *
 * and Chris Woodward        *
 *                           *  Who was Surreal "Chris Cool" Woodward?
 * Nimud OLC - copywrited to *  
 * Herb Gilliland and        *
 * Christopher Woodward 1993 *  Christopher Woodward was a student at Penn State in 1994.  A freshman in computer science,
 *                           *  he developed this online creation software in 1991 for The Isles, a NiMUD-derived MUD hosted
 * THE ORIGINAL ONLINE       *  by the Hacks users group at the University of Arizona.  It was first released publically in 1994.
 * CREATION SYSTEM.          *
 *                           *  Originally developed as an answer to a lack of readily accessible zone editors, the OLC has been 
 * Locke's additions 93-2003 *  expanded to include new features over the years while still retaining core functionality.
 * Krayy's HEDIT version 1.0 *
 *                           *  The Isles and NiMUD software has been dedicated to his memory.  Chris died on December 13, 1995. 
 *****************************/ 

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
#include "net.h"
#include "edit.h"
#include "mem.h"
#include "script.h"


/*
 * Macros to increase the speed at which editors are coded.
 */

/* Without zone checking for skills, spells and help data.
 * create <dbkey>
 */
#define CREATE_COMMAND(var,hash,new,get,tdbkey) \
    if ( !str_cmp( arg1, "create" ) )\
    {  int iHash;  value = atoi( arg2 ); if ( arg2[0] == '\0'  || value == 0 )\
        { send_to_actor( "Syntax:  create [dbkey]\n\r", ch ); return; }\
        if ( get( value ) != NULL )\
        {  send_to_actor( "Vnum is already in use.\n\r", ch );  return;  }\
        var = new(); var->dbkey = value; if ( value > tdbkey ) tdbkey = value;\
        iHash = value % MAX_KEY_HASH; var->next = hash[iHash];\
        hash[iHash]  = var; ch->desc->pEdit = (void *)var;\
        send_to_actor( "Created.\n\r", ch ); return;    }

/*
 * With zone checking.
 * create <dbkey>
 */
#define CREATE_COMMANDZ(var,hash,new,get,tdbkey) \
if ( !str_cmp( arg1, "create" ) )\
 {value = atoi( arg2 );  if ( arg2[0] == '\0'  || value == 0 ) {\
  send_to_actor( "Syntax:  create [dbkey]\n\r", ch ); return; }\
  pZone = get_dbkey_zone( value ); if ( pZone == NULL ) { \
  send_to_actor( "That dbkey is not assigned an zone.\n\r", ch ); return; }\
  if ( !IS_BUILDER( ch, pZone ) ) { send_to_actor( "Vnum is outside your zone.\n\r", ch ); return; }\
  if ( get( value ) != NULL ) { send_to_actor( "Vnum already exists.\n\r", ch ); return; }\
  var = new(); var->zone = pZone; var->dbkey = value; if ( value > tdbkey ) tdbkey = value;  iHash = value % MAX_KEY_HASH;  \
  var->next = hash[iHash]; hash[iHash]  = var; ch->desc->pEdit = (void *)var;\
  SET_BIT( pZone->zone_flags, ZONE_CHANGED ); send_to_actor( "Created.\n\r", ch ); return;  }



/*
 * var <value>
 */
#define EDIT_VALUE(sname,desc,var,conv)\
    if ( !str_cmp( arg1, sname )  )\
    {  value = conv( arg2 ); var = value;\
       send_to_actor( desc, ch );\
       send_to_actor( "\n\r", ch );\
       return;   }

/*
 * <bit-name>
 */
#define DEF_VALUE(bitconv, var, notfound, desc)\
    if ( bitconv( arg1 ) != notfound )    {\
        TOGGLE_BIT(var, bitconv( arg1 ));\
        send_to_actor( desc, ch );\
        send_to_actor( "\n\r", ch );\
        return;    }

/*
 * var <string>
 */
#define STR_EDIT_VALUE(sdesc,var)\
    if ( !str_cmp( arg1, sdesc ) )    {\
        if ( arg2[0] == '\0' )        {\
            send_to_actor( "Syntax:  ", ch );\
            send_to_actor( sdesc, ch );\
            send_to_actor(" [", ch );\
            send_to_actor( sdesc, ch );\
            send_to_actor( "]\n\r", ch );\
            return;        }\
        free_string( var );  var = str_dup( arg2 );\
        send_to_actor( capitalize(sdesc), ch );\
        send_to_actor( " set.\n\r", ch );\
        return;    }

/*------------------------------------------------------------*/

/*
 * Spell Editor
 */

void spedit( PLAYER *ch, char *argument )
{
    SPELL *pSpell=NULL;
    char arg[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int  value;

    strcpy( arg, argument );
    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    strcpy( arg2, argument );

    pSpell = (SPELL *)(ch->desc->pEdit);

    if ( !str_cmp( arg1, "done" ) || !pSpell )
    {
        ch->desc->pEdit = NULL;
        ch->desc->connected = NET_PLAYING;
        cmd_zsave( ch, "world" );
        return;
    }


    if ( !str_cmp( arg1, "show" ) )
    {
        clrscr(ch);
        snprintf( buf, MAX_STRING_LENGTH, "%d", pSpell->dbkey );
        cmd_spedit( ch, "show" );
        return;
    }


    if ( !str_cmp( arg1, "?" ) )
    {
        cmd_help( ch, "spedit" );
        return;
    }

    if ( is_number( arg1 ) )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%d", atoi(arg1) );
        cmd_spedit( ch, buf );
        return;
    }

    DEF_VALUE(mana_name_bit, pSpell->mana_type, MANA_NONE,
              "Mana type toggled." );

    EDIT_VALUE("position", "Minimum position set.",
               pSpell->minimum_position, name_position );

    EDIT_VALUE("delay", "Delay set.",
               pSpell->delay, atoi);

    EDIT_VALUE("mana",  "Mana requirement set.",
               pSpell->mana_cost, mana_name_bit);

    STR_EDIT_VALUE("name",pSpell->name);

    EDIT_VALUE("target", "Target type set.",    
               pSpell->target,    target_name_bit);

    EDIT_VALUE("position", "Minimum position set.", 
               pSpell->minimum_position,  name_position);

    EDIT_VALUE("level",    "Level set.",
               pSpell->level, atoi);

    EDIT_VALUE("slot",    "Slot set.",
               pSpell->slot, atoi);

    CREATE_COMMAND(pSpell,spell__hash,
                          new_spell,
                          get_spell_index,
                          top_dbkey_spell);

    if ( !str_cmp( arg1, "group" ) ) {
        SKILL *pGroup;

        pGroup = skill_lookup( arg2 );
        if ( !pGroup ) {
        send_to_actor( "Invalid group.\n\r", ch );
        return; 
        }

        pSpell->group_code = pGroup->dbkey;
        send_to_actor("Group set.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "script" ) )
    {
        INSTANCE *pTrig;
        SCRIPT *script;

        if ( !str_cmp( arg2, "clear" ) || !str_cmp( arg2, "kill" ) )
        {
            INSTANCE *pNext;

            for ( pTrig = pSpell->instances; pTrig != NULL; pTrig = pNext )
            {
                pNext     = pTrig->next;
                free_instance( pTrig );
            }

            pSpell->instances = NULL;
            send_to_actor( "Scripts cleared.\n\r", ch );
            return;
        }

        if ( (script = get_script_index(atoi(arg2))) == NULL )
        {
            send_to_actor( "Syntax:  script [dbkey]\n\r", ch );
            return;
        }

        pTrig            = new_instance();
        pTrig->script    = script;

        pTrig->next    = pSpell->instances;
        pSpell->instances = pTrig;

        send_to_actor( "Script added.\n\r", ch );
        return;
    }

    interpret( ch, arg );
    return;
}



/*
 * Syntax:  spedit [name]
 *          spedit [dbkey]
 *          spedit create [dbkey]
 *         *spedit create 
 */
void cmd_spedit( PLAYER *ch, char *argument )
{
    SPELL *pSpell=NULL;
    INSTANCE *pInstance;
    int value;
    char arg1[MSL];
    char arg2[MSL];
    char buf[MSL];

    if ( NPC(ch) ) return;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( is_number( arg1 ) ) {
        pSpell = get_spell_index( atoi(arg1) ); 
        if ( !pSpell ) {
            send_to_actor( "Spell not exist.\n\r", ch );
            return;
        }
        ch->desc->connected = NET_SPEDITOR;
        ch->desc->pEdit = (void *)pSpell;
        return;
    }
  
    if ( !str_cmp( arg1, "show" ) )
    {

        SKILL *pGroup;

        if ( ( pSpell = (SPELL *)ch->desc->pEdit ) == NULL )
        {
        send_to_actor( "Spells:  You are not editing that spell.\n\r", ch );
        return;
        }

            pGroup = get_skill_index( pSpell->group_code );

            snprintf( buf, MAX_STRING_LENGTH, "[^B%5d^N] Level [^B%5d^N]  Name [^B%-12s^N]  Target [^B%s^N]\n\r", 
                     pSpell->dbkey,
                     pSpell->level,
                     pSpell->name,
                     target_bit_name( pSpell->target ) ); 
            send_to_actor( buf, ch );
            snprintf( buf, MAX_STRING_LENGTH, "        Mana  [^B%5d^N]  %s\n\r", 
                     pSpell->mana_cost, 
                     mana_bit_name( pSpell->mana_type ) );
            send_to_actor( buf, ch );

            snprintf( buf, MAX_STRING_LENGTH, "        Group [^B%5d^N]  ^B%s^N - Delay [^B%d^N] Position [^B%s^N]\n\r",
                     pSpell->group_code, 
                     pGroup ? pGroup->name : "INVALID GROUP",
                     pSpell->delay, 
                     position_name(pSpell->minimum_position) );

            send_to_actor( buf, ch );

            for ( pInstance = pSpell->instances; pInstance != NULL;  
                  pInstance = pInstance->next ) {
              snprintf( buf, MAX_STRING_LENGTH, "Script [^B%s^N] %d\n\r",
                       pInstance->script->name, 
                       pInstance->script->dbkey );
              send_to_actor( buf, ch );
            }
        return;

    }
    else
    {

        if ( !str_cmp( arg1, "create" ) )
        {
            value = atoi( arg2 );
            if ( arg2[0] == '\0' || value == 0 )
            {
                send_to_actor( "Syntax:  spedit create [dbkey]\n\r", ch );
                return;
            }
  CREATE_COMMAND(pSpell,spell__hash,
                        new_spell,
                        get_spell_index,
                        top_dbkey_spell);
            ch->desc->pEdit = (void *)pSpell;
            if ( ch->desc->pEdit != NULL )
            ch->desc->connected = NET_SPEDITOR;
        } 

    if ( !str_prefix( arg1, "list" ) )
    {
        int tdbkey;

        for ( tdbkey=0; tdbkey <= top_dbkey_spell; tdbkey++ )
        { 
            pSpell = get_spell_index( tdbkey );
            if ( !pSpell ) continue;

            snprintf( buf, MAX_STRING_LENGTH, "[^B%5d^N] Level [^B%5d^N]  Name [^B%-12s^N] ", 
                     pSpell->dbkey,
                     pSpell->level,
                     pSpell->name);
            send_to_actor( buf, ch );
            snprintf( buf, MAX_STRING_LENGTH, "Group [^B%5d^N]  Target [^B%s^N]\n\r",
  pSpell->group_code, 
  target_bit_name( pSpell->target ) ); 

            send_to_actor( buf, ch );
        }
        return;
    }

    }

    ch->desc->connected = NET_SPEDITOR;
    return;
}




/*---------------------------------------------------------------*/

/*
 * Skill Editor
 */

void skedit( PLAYER *ch, char *argument )
{
    SKILL *pSkill=NULL;
    char arg[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int  value;

    strcpy( arg, argument );
    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    strcpy( arg2, argument );

    pSkill = (SKILL *)(ch->desc->pEdit);

    if ( !str_cmp( arg1, "done" ) || !pSkill )
    {
        ch->desc->pEdit = NULL;
        ch->desc->connected = NET_PLAYING;
        cmd_zsave( ch, "world" );
        return;
    }


    if ( !str_cmp( arg1, "show" ) )
    {
        clrscr(ch);
        snprintf( buf, MAX_STRING_LENGTH, "%d", pSkill->dbkey );
        cmd_skedit( ch, "show" );
        return;
    }

    if ( !str_cmp( arg1, "list" ) ) {
        clrscr(ch);
        cmd_skedit( ch, "list" );
        return;
    }

    if ( !str_cmp( arg1, "?" ) )
    {
        cmd_help( ch, "skedit" );
        return;
    }

    if ( is_number( arg1 ) )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%d", atoi(arg1) );
        cmd_skedit( ch, buf );
        return;
    }

    EDIT_VALUE("position", "Minimum position set.",
               pSkill->minimum_position, 
               name_position );

    EDIT_VALUE("delay", "Beat delay set.",
               pSkill->delay, atoi);

    EDIT_VALUE("slot", "Slot.  Do not duplicate a spell dbkey.",
               pSkill->slot, atoi);

    STR_EDIT_VALUE("name",pSkill->name);

    EDIT_VALUE("target", "Target type set.",    
               pSkill->target,    target_name_bit);

    EDIT_VALUE("str", "Required strength set.",
               pSkill->req_str, atoi );
 
    EDIT_VALUE("int", "Required intelligence set.",
               pSkill->req_int, atoi );


    EDIT_VALUE("wis", "Required wisdom set.",
               pSkill->req_wis, atoi );


    EDIT_VALUE("dex", "Required dexterity set.",
               pSkill->req_dex, atoi);


    EDIT_VALUE("con", "Required constitution set.",
               pSkill->req_con, atoi);

    EDIT_VALUE("position", "Minimum position set.", 
               pSkill->minimum_position,  name_position);

    EDIT_VALUE("delay",    "Delay set.",
               pSkill->level, atoi);

    EDIT_VALUE("learn",    "Learn rate set.",
               pSkill->learn_rate, atoi);

    EDIT_VALUE("cost",    "Cost set.",
               pSkill->cost, atoi);

    EDIT_VALUE("practice",    "Max % gained in a practice.",
               pSkill->max_prac, atoi);

    EDIT_VALUE("level",    "Level set.",
               pSkill->level, atoi);

    EDIT_VALUE("slot",     "Slot set.",
               pSkill->slot, atoi)

    EDIT_VALUE("required", "Required percent set.",
               pSkill->required_percent, atoi);
    
/*--*/

    if ( !str_cmp( arg1, "group" ) ) {
        SKILL *pGroup;

        pGroup = skill_lookup( arg2 );
        if ( !pGroup ) {
        send_to_actor( "Invalid group.\n\r", ch );
        return; 
        }

        pSkill->group_code = pGroup->dbkey;
        send_to_actor("Group set.\n\r", ch );
        return;
    }

    CREATE_COMMAND(pSkill,skill__hash,
                          new_skill,
                          get_skill_index,
                          top_dbkey_skill);

    interpret( ch, arg );
    return;
}



/*
 * Syntax:  skedit [name]
 *          skedit [dbkey]
 *          skedit create [dbkey]
 *         *skedit create 
 */
void cmd_skedit( PLAYER *ch, char *argument )
{
    SKILL *pSkill;
    int value;
    char arg1[MSL];
    char arg2[MSL];
    char buf[MSL];

    if ( NPC(ch) ) return;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( !str_cmp( arg1, "show" ) )
    {
        SKILL *pGroup;

        if ( ( pSkill = (SKILL *)ch->desc->pEdit ) == NULL )
        {
        send_to_actor( "Skills:  You are not editing that skill.\n\r", ch );
        return;
        }

        pGroup = get_skill_index( pSkill->group_code );

        snprintf( buf, MAX_STRING_LENGTH, "[^B%5d^N] Level [^B%5d^N]  Name [^B%-12s^N]  Target [^B%s^N]\n\r", 
                 pSkill->dbkey,
                 pSkill->level,
                 pSkill->name,
                 target_bit_name( pSkill->target ) ); 
        send_to_actor( buf, ch );
 
        snprintf( buf, MAX_STRING_LENGTH, "        Str [^B%3d^N] Int [^B%3d^N] Wis [^B%3d^N] Dex [^B%3d^N] Con [^B%3d^N]\n\r", 
                 pSkill->req_str, 
                 pSkill->req_int, 
                 pSkill->req_wis,
                 pSkill->req_dex, 
                 pSkill->req_con );
        send_to_actor( buf, ch );

        snprintf( buf, MAX_STRING_LENGTH, "        Group [^B%5d^N]  %s - Delay [^B%d^N] Position [^B%s^N]\n\r",
                 pSkill->group_code, 
                 pGroup ? pGroup->name : (pSkill->group_code == 0 ? "GROUP" : "INVALID GROUPING"),
                 pSkill->delay, 
                 position_name(pSkill->minimum_position) );
        send_to_actor( buf, ch );

        snprintf( buf, MAX_STRING_LENGTH, "        Cost  [^B%3d^N]  Rate [^B%3d^N] Practice [^B%3d^N] Learn [^B%3d^N]  Required [^B%3d^N]\n\r", 
                 pSkill->cost, pSkill->learn_rate, pSkill->max_prac,
                 pSkill->max_learn, pSkill->required_percent );
        send_to_actor( buf, ch );
        return;

    }
    else
    if ( !str_cmp( arg1, "create" ) )
        {
            value = atoi( arg2 );
            if ( arg2[0] == '\0' || value == 0 )
            {
                send_to_actor( "Syntax:  skedit create [dbkey]\n\r", ch );
                return;
            }
    CREATE_COMMAND(pSkill,skill__hash,
                          new_skill,
                          get_skill_index,
                          top_dbkey_skill);
        }
    else
    if ( !str_cmp( arg1, "list" ) )
    {
        int tdbkey;

        for ( tdbkey=0; tdbkey <= top_dbkey_skill; tdbkey++ )
        { 
            pSkill = get_skill_index( tdbkey );
            if ( !pSkill ) continue;

            snprintf( buf, MAX_STRING_LENGTH, "[^B%5d^N] Level [^B%5d^N]  Name [^B%-12s^N] ", 
                     pSkill->dbkey,
                     pSkill->level,
                     pSkill->name);
            send_to_actor( buf, ch );
            snprintf( buf, MAX_STRING_LENGTH, "Group [^B%5d^N]\n\r", pSkill->group_code ); 

            send_to_actor( buf, ch );
        }
        return;
    }
    else
    if ( (pSkill = skill_lookup(arg1)) != NULL ) {
        send_to_actor( "Editing ", ch ); 
        send_to_actor( pSkill->name, ch );
        send_to_actor( ".\n\r", ch );
        ch->desc->connected = NET_SKEDITOR;
        ch->desc->pEdit = (void *)pSkill;
        return;        
    }
    else
    if ( (pSkill = get_skill_index( atoi(arg1) )) ) { 
        if ( !pSkill ) {
            send_to_actor( "Skill not exist.\n\r", ch );
            return;
        }
        ch->desc->connected = NET_SKEDITOR;
        ch->desc->pEdit = (void *)pSkill;
        return;
    }

    send_to_actor( "No such skill.\n\r", ch );
    return;
}



/*-----------------------------------------------------------------*/

ZONE *get_zone( int dbkey )
{
    ZONE *pZone;

    for (pZone = zone_first; pZone != NULL; pZone = pZone->next )
    {
        if (pZone->dbkey == dbkey)
            return pZone;
    }

    return NULL;
}



ZONE *get_dbkey_zone( int dbkey )
{
    ZONE *pZone;

    for ( pZone = zone_first; pZone != NULL; pZone = pZone->next )
    {
        if ( dbkey >= pZone->ldbkey
          && dbkey <= pZone->udbkey )
            return pZone;
    }

    return NULL;
}



char *zone_bit_name( int zone_flags )
{
    static char buf[512];

    buf[0] = '\0';
    if ( zone_flags & ZONE_STATIC       )   strcat( buf, " static" );
    if ( zone_flags & ZONE_CHANGED      )   strcat( buf, " changed" );
    if ( zone_flags & ZONE_ADDED        )   strcat( buf, " added" );
    return buf+1;
}



int get_zone_flags_number( char *argument )
{
    if ( !str_cmp( argument, "static"  ) )      return ZONE_STATIC;
    if ( !str_cmp( argument, "changed" ) )      return ZONE_CHANGED;
    if ( !str_cmp( argument, "added" ) )        return ZONE_ADDED;
    return ZONE_NONE;
}




void zedit( PLAYER *ch, char *argument )
{
    ZONE *pZone;
    char arg[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf [MAX_STRING_LENGTH];
    int  value;

    pZone = (ZONE *)ch->desc->pEdit;
    strcpy( arg, argument );
    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    strcpy( arg2, argument );


    if ( !IS_BUILDER( ch, pZone ) )
    {
        send_to_actor( "ZEdit:  Insufficient security to modify zone.\n\r", ch );
    }


    if ( !str_cmp( arg1, "show" ) || arg1[0] == '\0' )
    {
        clrscr(ch);
        snprintf( buf, MAX_STRING_LENGTH, "%d", pZone->dbkey );
        cmd_astat( ch, buf );
        return;
    }


    if ( !str_cmp( arg1, "done" ) )
    {
        ch->desc->pEdit = NULL;
        ch->desc->connected = NET_PLAYING;
        cmd_zsave( ch, "changed" );
        return;
    }


    if ( !str_cmp( arg1, "?" ) )
    {
        cmd_help( ch, "zedit" );
        return;
    }

    if ( !str_cmp( arg1, "generate" ) )
    {
        if ( !str_cmp( arg2, "overwrite" ) )
        generate( ch, pZone->ldbkey, pZone->udbkey, TRUE );
        else
        generate( ch, pZone->ldbkey, pZone->udbkey, FALSE );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        return;
    };


    if ( is_number( arg1 ) )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%d", atoi(arg1) );
        cmd_zedit( ch, buf );
        return;
    }

    if ( !IS_BUILDER( ch, pZone ) )
    {
        interpret( ch, arg );
        return;
    }


    if ( ( value = get_zone_flags_number( arg1 ) ) != ZONE_NONE )
    {
        TOGGLE_BIT(pZone->zone_flags, value);

        send_to_actor( "Flag toggled.\n\r", ch );
        return;
    }


    if ( !str_cmp( arg1, "create" ) )
    {
        ZONE *pNewzone;

        if ( zone_first == NULL )
        {
            zone_first = new_zone();
            pNewzone   = zone_first;
        }
        else
        {
            for ( pNewzone = zone_first;
                  pNewzone != NULL;
                  pNewzone = pNewzone->next )
            {
                if ( pNewzone->next == NULL )
                {
                    send_to_actor( pNewzone->name, ch );
                    pNewzone->next = new_zone();          /* Clip on end */
                    pNewzone       = pNewzone->next;      /* Goto end    */
                    break;
                }
            }
        }

        ch->desc->pEdit    =   (void *)pNewzone;

        SET_BIT( pNewzone->zone_flags, ZONE_ADDED );
        send_to_actor( "zone Created.\n\r", ch );
        return;
    }


    if ( !str_cmp( arg1, "name" ) )
    {
        if ( arg2[0] == '\0' )
        {
               send_to_actor( "Syntax:   name [$name]\n\r", ch );
               return;
        }

        free_string( pZone->name );
        pZone->name = str_dup( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Name set.\n\r", ch );
        return;
    }


    if ( !str_cmp( arg1, "filename" ) || !str_cmp( arg1, "file" ) )
    {
        if ( argument[0] == '\0' )
        {
            send_to_actor( "Syntax:  filename [$file]\n\r", ch );
            send_to_actor( "or       file     [$file]\n\r", ch );
            return;
        }

        if ( argument[0] == '$' )
        return;

        free_string( pZone->filename );
        pZone->filename = str_dup( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Filename set.\n\r", ch );
        return;
    }


    if ( !str_cmp( arg1, "age" ) )
    {
        if ( !is_number( arg2 ) || arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  age [#age]\n\r", ch );
            return;
        }

        pZone->age = atoi( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Age set.\n\r", ch );
        return;
    }


    if ( !str_cmp( arg1, "repop" ) )
    {
        if ( arg2[0] == '\0' )
        {
            string_append( ch, &pZone->repop );
            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            return;
        }

        if ( arg2[0] == '+' )
        {
            string_append( ch, &pZone->repop );
            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            return;
        }

        send_to_actor( "Syntax:  repop    - line edit\n\r", ch );
        send_to_actor( "         repop +  - line append\n\r",ch );
        return;
    }


    if ( !str_cmp( arg1, "security" ) )
    {
        if ( !is_number( arg2 ) || arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  security [#level]\n\r", ch );
            return;
        }

        value = atoi( arg2 );

        if ( NPC(ch) || value < GET_PC(ch,security,0) || value > 9 )
        {
            if ( GET_PC(ch,security,9) != 9 )
            {
                snprintf( buf, MAX_STRING_LENGTH, "Valid security range is %d-9.\n\r", PC(ch,security) );
                send_to_actor( buf, ch );
            }
            else
            {
                send_to_actor( "Valid security is 9 only.\n\r", ch );
            }
            return;
        }

        pZone->security = value;
        SET_BIT( pZone->zone_flags, ZONE_CHANGED );

        send_to_actor( "Security set.\n\r", ch );
        return;
    }


    if ( !str_cmp( arg1, "builder" ) )
    {
        argument = one_argument( argument, arg2 );

        if ( arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  builder [$name]\n\r", ch );
            return;
        }

        arg2[0] = UPPER( arg2[0] );

        if ( strstr( pZone->builders, arg2 ) != NULL )
        {
            pZone->builders = string_replace( pZone->builders, arg2, "\0" );
            pZone->builders = string_unpad( pZone->builders );

            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            send_to_actor( "Builder removed.\n\r", ch );
            return;
        }
        else
        {
            buf[0] = '\0';
            if (pZone->builders[0] != '\0' )
            {
                strcat( buf, pZone->builders );
                strcat( buf, " " );
            }
            strcat( buf, arg2 );
            free_string( pZone->builders );
            pZone->builders = string_proper( str_dup( buf ) );

            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            send_to_actor( "Builder added.\n\r", ch );
            return;
        }
    }


    if ( !str_cmp( arg1, "ldbkey" ) )
    {
        if ( !is_number( arg2 ) || arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  ldbkey [#lower]\n\r", ch );
            return;
        }
        value = atoi( arg2 );

        if ( get_dbkey_zone( value ) != NULL
          && get_dbkey_zone( value ) != pZone )
        {
            send_to_actor( "ZEdit:  Vnum range already assigned.\n\r", ch );
            return;
        }

        pZone->ldbkey = value;
        SET_BIT( pZone->zone_flags, ZONE_CHANGED );

        send_to_actor( "Lower dbkey set.\n\r", ch );
        return;
    }


    if ( !str_cmp( arg1, "udbkey" ) )
    {
        if ( !is_number( arg2 ) || arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  udbkey [#upper]\n\r", ch );
            return;
        }

        value = atoi( arg2 );

        if ( get_dbkey_zone( value ) != NULL
          && get_dbkey_zone( value ) != pZone )
        {
            send_to_actor( "ZEdit:  Vnum range already assigned.\n\r", ch );
            return;
        }

        pZone->udbkey = value;
        SET_BIT( pZone->zone_flags, ZONE_CHANGED );

        send_to_actor( "Upper dbkey set.\n\r", ch );
        return;
    }


    if ( !str_cmp( arg1, "dbkey" ) )
    {
       argument = one_argument( argument, arg1 );
       strcpy( arg2, argument );

       if ( !is_number( arg1 ) || arg1[0] == '\0'
         || !is_number( arg2 ) || arg2[0] == '\0' )
       {
            send_to_actor( "Syntax:  dbkey [#lower] [#upper]\n\r", ch );
            return;
        }

        value = atoi( arg1 );

        if ( get_dbkey_zone( value ) != NULL
          && get_dbkey_zone( value ) != pZone )
        {
            send_to_actor( "ZEdit:  Lower dbkey already assigned.\n\r", ch );
            return;
        }

        pZone->ldbkey = value;
        send_to_actor( "Lower dbkey set.\n\r", ch );

        value = atoi( arg2 );

        if ( get_dbkey_zone( value ) != NULL
          && get_dbkey_zone( value ) != pZone )
        {
            send_to_actor( "ZEdit:   Upper dbkey already assigned.\n\r", ch );
            return;
        }

        pZone->udbkey = value;
        send_to_actor( "Upper dbkey set.\n\r", ch );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        return;
    }

    interpret( ch, arg );
    return;
}



bool redit_exit( PLAYER *ch, SCENE *pScene, int door,
                 char *arg1, char *arg2 )
{
    ZONE *pZone, *ozone = NULL;
    EXIT *pexit;
    EXIT *oexit = NULL;
    int value;

    pexit = pScene->exit[door];
    pZone = pScene->zone;
    if ( pexit != NULL && pexit->to_scene != NULL )
    {
        oexit = pexit->to_scene->exit[rev_dir[door]];
        ozone = pexit->to_scene->zone;
    }

    if ( !str_cmp( arg1, "unlink" )
      || !str_cmp( arg1, "delete" )
      || !str_cmp( arg1, "kill" )  )
    {
        if ( pScene->exit[door] == NULL )
        {
            send_to_actor( "Cannot delete a non-existant exit.\n\r", ch );
            return TRUE;
        }

        if ( oexit != NULL && IS_BUILDER( ch, ozone ) )
        {
            free_exit( oexit );
            pexit->to_scene->exit[rev_dir[door]] = NULL;
            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        }

        free_exit( pexit );
        pScene->exit[door] = NULL;

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Exit unlinked.\n\r", ch );
        return TRUE;
    }

    if ( is_number( arg1 ) )
    {
        SCENE *to_scene;

        value = atoi( arg1 );

        if ( (to_scene = get_scene( value )) == NULL )
        {
            if ( (ozone = get_dbkey_zone( value )) == NULL )
            {
                send_to_actor( "Vnum does not fall within an zone define.\n\r", ch );
                return TRUE;
            }

            if ( !IS_BUILDER( ch, ozone ) )
            {
            send_to_actor( "Cannot dig a scene in a zone of which you are not a builder.\n\r", ch );
            return TRUE;
            }
            else
            {
                if ( ( IS_SET(ozone->zone_flags, ZONE_STATIC)
                    || IS_SET(pZone->zone_flags, ZONE_STATIC) )
                  && to_scene->zone != pZone )
                {
                send_to_actor( "You cannot link static zones to other zones.\n\r", ch );
                return TRUE;
                }
                else
                {
                char buf[MAX_INPUT_LENGTH];

                snprintf( buf, MAX_STRING_LENGTH, "create %d", value );
                redit( ch, buf );
                to_scene = get_scene( value );
                if ( to_scene == NULL )
                {
                    bug( "Redit_exit: NULL after attempted create, dbkey %d.", value );
                    return TRUE;
                }
                to_scene->move = pScene->move;
                to_scene->scene_flags  = pScene->scene_flags;
                }
            }
        }

        if ( !IS_BUILDER( ch, to_scene->zone ) )
        {
            send_to_actor( "Return exit not created.\n\r", ch );
            ozone = NULL;
        }
        else ozone = to_scene->zone;

        if ( to_scene->exit[rev_dir[door]] != NULL )
        {
            send_to_actor( "Return exit already exists.\n\r", ch );
            return TRUE;
        }

        if ( pScene->exit[door] != NULL )
            redit_exit( ch, pScene, door, "unlink", "" );

        pScene->exit[door] = new_exit();

        pScene->exit[door]->to_scene = to_scene;
        pScene->exit[door]->dbkey = value;

        if ( ozone != NULL )
        {
        door                            = rev_dir[door];
        to_scene->exit[door]             = new_exit();
        to_scene->exit[door]->to_scene    = pScene;
        to_scene->exit[door]->dbkey       = pScene->dbkey;
        SET_BIT( ozone->zone_flags, ZONE_CHANGED );
        }

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Exit linked.\n\r", ch );
        return TRUE;
    }
        
    if ( !str_cmp( arg1, "to" ) )
    {
        if ( arg2[0] == '\0' || !is_number( arg2 ) )
        {
            send_to_actor( "Syntax:  [direction] scene [dbkey]\n\r", ch );
            return TRUE;
        }

        if ( pScene->exit[door] == NULL )
        {
            pScene->exit[door] = new_exit();
        }

        value = atoi( arg2 );

        if ( get_scene( value ) == NULL )
        {
            send_to_actor( "Cannot link to non-existant scene.\n\r", ch );
            return TRUE;
        }

        pScene->exit[door]->to_scene = get_scene( value );
        pScene->exit[door]->dbkey = value;

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Exit destination set.\n\r", ch );
        return TRUE;
    }


    if ( !str_cmp( arg1, "key" ) )
    {
        if ( arg2[0] == '\0' || !is_number( arg2 ) )
        {
            send_to_actor( "Syntax:  [direction] key [dbkey]\n\r", ch );
            return TRUE;
        }

        if ( pScene->exit[door] == NULL )
        {
            pScene->exit[door] = new_exit();
        }

        value = atoi( arg2 );

        if ( get_prop_template( value ) == NULL )
        {
            send_to_actor( "Cannot use a non-existant prop as a key.\n\r", ch );
            return TRUE;
        }

        pScene->exit[door]->key = value;

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Exit key set.\n\r", ch );
        return TRUE;
    }


    if ( !str_cmp( arg1, "name" ) )
    {
        if ( arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  [direction] name [string]\n\r", ch );
            return TRUE;
        }

        if ( pScene->exit[door] == NULL )
        {
            pScene->exit[door] = new_exit();
        }

        free_string( pScene->exit[door]->keyword );
        pScene->exit[door]->keyword = str_dup( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Exit name set.\n\r", ch );
        return TRUE;
    }


    if ( !str_cmp( arg1, "desc" ) || !str_cmp( arg1, "description" ) )
    {
        if ( pScene->exit[door] == NULL )
        {
            pScene->exit[door]          = new_exit();
            pScene->exit[door]->dbkey    = pScene->dbkey;
            pScene->exit[door]->to_scene = pScene;
        }

        string_append( ch, &pScene->exit[door]->description );
        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        return TRUE;
    }

    if ( !str_cmp( arg1, "door" ) )
    {
        if ( (pexit = pScene->exit[door]) == NULL )
        {
            send_to_actor( "Exit does not exist.\n\r", ch );
            return TRUE;
        }

        if ( pScene->exit[door]->to_scene == NULL )
        {
            send_to_actor( "Exit does not lead anywhere.\n\r", ch );
            return TRUE;
        }

        ozone = pScene->exit[door]->to_scene->zone;
        if ( (oexit = pScene->exit[door]->to_scene->exit[rev_dir[door]]) == NULL )
        {
            send_to_actor( "No return exit.\n\r", ch );
            return TRUE;
        }

        if ( !IS_BUILDER( ch, ozone ) )
        {
            send_to_actor( "You are not a builder of the return exit.\n\r", ch );
            return TRUE;
        }

        SET_BIT( pexit->rs_flags, EXIT_ISDOOR );
        SET_BIT( pexit->exit_flags, EXIT_ISDOOR );
        SET_BIT( oexit->rs_flags, EXIT_ISDOOR );
        SET_BIT( oexit->exit_flags, EXIT_ISDOOR );
        send_to_actor( "Door copied to reverse side.\n\r", ch );

        if ( arg2[0] != '\0' )
        {
            free_string( pexit->keyword );
            pexit->keyword = str_dup( arg2 );

            free_string( oexit->keyword );
            oexit->keyword = str_dup( arg2 );
        }
        else
        {
            free_string( pexit->keyword );
            pexit->keyword = str_dup( "door" );

            free_string( oexit->keyword );
            oexit->keyword = str_dup( "door" );
        }
        return TRUE;
    }

    if ( !str_cmp( arg1, "flag" ) )
    {
        int flag = exit_name_bit( arg2 );

        if ( flag == EXIT_NONE )
        {
            send_to_actor( "Syntax:  [direction] flag [exit flag]\n\r", ch );
            return TRUE;
        }

        if ( pScene->exit[door] == NULL )
        {
            send_to_actor( "Exit does not exist.\n\r", ch );
            return TRUE;
        }

        TOGGLE_BIT(pScene->exit[door]->rs_flags,  flag);
        TOGGLE_BIT(pScene->exit[door]->exit_flags, flag);

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Exit flag toggled.\n\r", ch );
        return TRUE;
    }

    if ( (value = exit_name_bit( arg1 )) != EXIT_NONE )
    {
        if ( (pexit = pScene->exit[door]) == NULL )
        {
            send_to_actor( "Exit does not exist.\n\r", ch );
            return TRUE;
        }

        if ( pScene->exit[door]->to_scene == NULL )
        send_to_actor( "Exit does not lead anywhere.\n\r", ch );
        else
        {
            ozone = pScene->exit[door]->to_scene->zone;
            if ( (oexit = pScene->exit[door]->to_scene->exit[rev_dir[door]]) == NULL )
            send_to_actor( "No return exit.\n\r", ch );
        }

        TOGGLE_BIT(pexit->rs_flags, value);
        TOGGLE_BIT(pexit->exit_flags, value);

        if ( oexit != NULL && IS_BUILDER( ch, ozone ) )
        {
            TOGGLE_BIT(oexit->rs_flags, value);
            TOGGLE_BIT(oexit->exit_flags, value);
        }

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Exit flag toggled.\n\r", ch );
        return TRUE;
    }

    return FALSE;
}




void redit( PLAYER *ch, char *argument )
{
    ZONE *pZone;
    SCENE *pScene;
    EXTRA_DESCR *ed;
    char arg[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int  value;
    int  iHash;
    int  door;

    strcpy( arg, argument );
    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    strcpy( arg2, argument );

    pScene = ch->in_scene;
    pZone = pScene->zone;


    if ( !IS_BUILDER( ch, pZone ) )
    {
        send_to_actor( "ZEdit:  Insufficient security to modify scene.\n\r", ch );
    }


    if ( !str_cmp( arg1, "show" ) || arg1[0] == '\0' )
    {
        clrscr(ch);
        snprintf( buf, MAX_STRING_LENGTH, "%d", pScene->dbkey );
        cmd_rstat( ch, buf );
        return;
    }


    if ( !str_cmp( arg1, "done" ) )
    {
        ch->desc->pEdit = NULL;
        ch->desc->connected = NET_PLAYING;
        cmd_zsave( ch, "changed" );
        cmd_zsave( ch, "list" );
        return;
    }


    if ( !str_cmp( arg1, "?" ) )
    {
        cmd_help( ch, "aedit" );
        return;
    }

   if ( !str_cmp( arg1, "cli" ) || !str_cmp( arg1, "client" ) )
   {
        string_append( ch, &pScene->client );
        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        return;
   }

   if ( !str_cmp( arg1, "copy" ) ) {
        SCENE *pScene_from;
        argument = one_argument( argument, arg1 );

        if ( !is_number( arg1 ) ) {
            send_to_actor( "Copies basic item information TO this record:\n\rcopy [dbkey-to-copy-from]\n\r", ch );
            return;
        }

        pScene_from = get_scene( atoi( arg1 ) );
        if ( !pScene_from ) {
            send_to_actor( "Invalid scene dbkey to copy from.\n\r", ch );
            return;
        }

        free_string( pScene->name );
        free_string( pScene->description );

        pScene->name        = str_dup( pScene_from->name );
        pScene->description = str_dup( pScene_from->description );

        pScene->position    = pScene_from->position;
        pScene->template    = pScene_from->template;
        pScene->scene_flags  = pScene_from->scene_flags;
        pScene->max_people  = pScene_from->max_people;
        pScene->light       = pScene_from->light;
        pScene->move = pScene_from->move;
        pScene->terrain     = pScene_from->terrain;
        pScene->wagon       = pScene_from->wagon;

        send_to_actor( "Basic information copied to this scene.\n\r", ch );
        return;        
    }

    if ( is_number( arg1 ) )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%d", atoi(arg1) );
        cmd_redit( ch, buf );
        return;
    }

    if ( !IS_BUILDER( ch, pZone ) )
    {
        interpret( ch, arg );
        return;
    }


    if ( scene_name_bit( arg1 ) != SCENE_NONE )
    {
        TOGGLE_BIT(pScene->scene_flags, scene_name_bit( arg1 ));

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Scene flag toggled.\n\r", ch );
        return;
    }


    if ( move_number( arg1 ) != MOVE_MAX )
    {
        pScene->move  = move_number( arg1 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Sector type set.\n\r", ch );
        return;
    }


    if ( ( door = get_dir( arg1 ) ) != MAX_DIR && arg2[0] != '\0' )
    {
        argument = one_argument( argument, arg1 );
        strcpy( arg2, argument );

        if ( redit_exit( ch, pScene, door, arg1, arg2 ) ) return;
    }


    if ( !str_cmp( arg1, "ed" ) )
    {
        if ( arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  ed add [keyword]\n\r", ch );
            send_to_actor( "         ed delete [keyword]\n\r", ch );
            send_to_actor( "         ed edit [keyword]\n\r", ch );
            send_to_actor( "         ed format [keyword]\n\r", ch );
            return;
        }

        argument = one_argument( argument, arg1 );
        strcpy( arg2, argument );

        if ( !str_cmp( arg1, "add" ) )
        {
            if ( arg2[0] == '\0' )
            {
                send_to_actor( "Syntax:  ed add [keyword]\n\r", ch );
                return;
            }

            for ( ed = pScene->extra_descr; ed != NULL; ed = ed->next )
            {
                if ( is_name( arg2, ed->keyword ) )
                    break;
            }

            if ( ed != NULL )
            {
                send_to_actor( "Extra description keyword exists.\n\r", ch );
                return;
            }

            ed                  =   new_extra_descr();
            ed->keyword         =   str_dup( arg2 );
            ed->description     =   str_dup( "" );
            ed->next            =   pScene->extra_descr;
            pScene->extra_descr  =   ed;

            string_append( ch, &ed->description );

            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            return;
        }


        if ( !str_cmp( arg1, "edit" ) )
        {
            if ( arg2[0] == '\0' )
            {
                send_to_actor( "Syntax:  ed edit [keyword]\n\r", ch );
                return;
            }

            for ( ed = pScene->extra_descr; ed != NULL; ed = ed->next )
            {
                if ( is_name( arg2, ed->keyword ) )
                    break;
            }

            if ( ed == NULL )
            {
                send_to_actor( "Extra description keyword not found.\n\r", ch );
                return;
            }

            string_append( ch, &ed->description );

            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            return;
        }


        if ( !str_cmp( arg1, "delete" ) )
        {
            EXTRA_DESCR *ped;

            if ( arg2[0] == '\0' )
            {
                send_to_actor( "Syntax:  ed delete [keyword]\n\r", ch );
                return;
            }

            ped = NULL;

            for ( ed = pScene->extra_descr; ed != NULL; ed = ed->next )
            {
                if ( is_name( arg2, ed->keyword ) )
                    break;
                ped = ed;
            }

            if ( ed == NULL )
            {
                send_to_actor( "Extra description keyword not found.\n\r", ch );
                return;
            }

            if ( ped == NULL )
            {
                pScene->extra_descr = ed->next;
            }
            else
            {
                ped->next = ed->next;
            }

            free_extra_descr( ed );

            send_to_actor( "Extra description deleted.\n\r", ch );
            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            return;
        }

    }
   
    if ( !str_cmp( arg1, "reference" ) 
     ||  !str_cmp( arg1, "ref" ) )
    {
        SCENE *template;
  
        value = atoi( arg2 );

        if ( value == 0 )
        {
             send_to_actor( "Cleared template reference.\n\r", ch);
             pScene->template = 0;
             return;
        }

        template = get_scene( value );
   
        if ( template == NULL 
          || !IS_SET(template->scene_flags, SCENE_TEMPLATE) )
        {
              send_to_actor( "Invalid dbkey for template.\n\r", ch);
              return;
        }
 
        pScene->template = value;

        send_to_actor( "Scene template set.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "script" ) )
    {
        INSTANCE *pTrig;
        SCRIPT *script;

        if ( !str_cmp( arg2, "clear" ) || !str_cmp( arg2, "kill" ) )
        {
            INSTANCE *pNext;

            for ( pTrig = pScene->instances; pTrig != NULL; pTrig = pNext )
            {
                pNext     = pTrig->next;
                free_instance( pTrig );
            }

            pScene->instances = NULL;
            send_to_actor( "Scripts cleared.\n\r", ch );
            return;
        }

        if ( (script = get_script_index(atoi(arg2))) == NULL )
        {
            send_to_actor( "Syntax:  script [dbkey]\n\r", ch );
            return;
        }
        SET_BIT( pZone->zone_flags, ZONE_CHANGED );

        pTrig            = new_instance();
        pTrig->script    = script;

        pTrig->next    = pScene->instances;
        pScene->instances = pTrig;

        send_to_actor( "Script set.\n\r", ch );
        return;
    }

    CREATE_COMMANDZ(pScene,scene_hash,
                           new_scene,
                           get_scene,
                           top_dbkey_scene);

    if ( !str_cmp( arg1, "name" ) )
    {
        if ( arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  name [name]\n\r", ch );
            return;
        }

        free_string( pScene->name );
        pScene->name = str_dup( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Name set.\n\r", ch );
        return;
    }


    if ( !str_cmp( arg1, "strcpy" ) )
    {
        SCENE *fScene;

        fScene = get_scene( atoi( arg2 ) );

        if ( fScene == NULL )
        {
        send_to_actor( "Syntax:  strcpy [dbkey]\n\r", ch );
        return;
        }

        if ( !IS_BUILDER(ch, fScene->zone) )
        {
        send_to_actor( "You are not allowed to copy from other zones.\n\r", ch );
        return;
        }

        free_string( pScene->description );
        pScene->description = str_dup( fScene->description );
        send_to_actor( "Description copied.\n\r *** This is frowned upon by zone reviewers! ***\n\r", ch );
        return;
    }


    if ( !str_cmp( arg1, "desc" ) || !str_cmp( arg1, "description" ) )
    {
        string_append( ch, &pScene->description );
        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        return;
    }


    if ( !str_cmp( arg1, "format" ) )
    {
        pScene->description = format_string( pScene->description );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "String formatted.\n\r", ch );
        return;
    }


    if ( !str_cmp( arg1, "light" ) )
    {
        if ( arg2[0] == '\0' || !is_number( arg2 ) )
        {
            send_to_actor( "Syntax:  light [number]\n\r", ch );
            return;
        }

        value = atoi( arg2 );
        pScene->light = value;

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Light set.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg1, "wagonref" ) )
    {
        if ( arg2[0] == '\0' || !is_number( arg2 ) )
        {
            send_to_actor( "Syntax:  wagonref [prop dbkey]\n\r", ch );
            return;
        }

        value = atoi( arg2 );
        pScene->wagon = value;

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Wagon prop dbkey reference set.\n\r", ch);
        return;
    }


    interpret( ch, arg );
    return;
}



void oedit( PLAYER *ch, char *argument )
{
    ZONE *pZone;
    PROP_TEMPLATE *pProp;
    EXTRA_DESCR *ed;
    BONUS *pAf;
    char arg[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int  value;
    int  iHash;

    strcpy( arg, argument );
    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    strcpy( arg2, argument );

    pProp = (PROP_TEMPLATE *)ch->desc->pEdit;
    pZone = pProp->zone;

    if ( !IS_BUILDER( ch, pZone ) )
    {
        send_to_actor( "Prop: Edit:  Insufficient security to modify prop.\n\r", ch );
    }

    if ( !str_cmp( arg1, "show" ) || arg1[0] == '\0' )
    {
        clrscr(ch);
        snprintf( buf, MAX_STRING_LENGTH, "%d", pProp->dbkey );
        cmd_pindex( ch, buf );
        return;
    }


    if ( !str_cmp( arg1, "done" ) )
    {
        ch->desc->pEdit = NULL;
        ch->desc->connected = NET_PLAYING;
        cmd_zsave( ch, "changed" );
        return;
    }


    if ( !str_cmp( arg1, "?" ) )
    {
        cmd_help( ch, "pedit" );
        return;
    }
    

    if ( is_number( arg1 ) )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%d", atoi(arg1) );
        cmd_oedit( ch, buf );
        return;
    }

    if ( !IS_BUILDER( ch, pZone ) )
    {
        interpret( ch, arg );
        return;
    }

    if ( !str_cmp( arg1, "copy" ) ) {
        PROP_TEMPLATE *pProp_from;
        argument = one_argument( argument, arg1 );

        if ( !is_number( arg1 ) ) {
            send_to_actor( "Copies basic item information TO this record:\n\rcopy [dbkey-to-copy-from]\n\r", ch );
            return;
        }

        pProp_from = get_prop_template( atoi( arg1 ) );
        if ( !pProp_from ) {
            send_to_actor( "Invalid prop to copy from.\n\r", ch );
            return;
        }

        free_string( pProp->name );
        free_string( pProp->short_descr );
        free_string( pProp->short_descr_plural );
        free_string( pProp->action_descr );
        free_string( pProp->description );
        free_string( pProp->description_plural );
        free_string( pProp->real_description );

        pProp->name        = str_dup( pProp_from->name );
        pProp->short_descr = str_dup( pProp_from->short_descr );
        pProp->short_descr_plural = str_dup( pProp_from->short_descr_plural );
        pProp->description        = str_dup( pProp_from->description );
        pProp->description_plural = str_dup( pProp_from->description_plural );
        pProp->action_descr     = str_dup( pProp_from->action_descr );
        pProp->real_description = str_dup( pProp_from->real_description );

        pProp->value[0]  = pProp_from->value[0];
        pProp->value[1]  = pProp_from->value[1];
        pProp->value[2]  = pProp_from->value[2];
        pProp->value[3]  = pProp_from->value[3];
        pProp->item_type = pProp_from->item_type;
        pProp->weight    = pProp_from->weight;
        pProp->cost      = pProp_from->cost;
        pProp->count     = pProp_from->count;
        pProp->extra_flags = pProp_from->extra_flags;
        pProp->wear_flags  = pProp_from->wear_flags;
        send_to_actor( "Basic information copied to this prop.\n\r", ch );
        return;        
    }

    if ( !str_cmp( arg1, "bonus" ) )
    {
        char arg3[MAX_STRING_LENGTH];

        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );
        strcpy( arg3, argument );

        if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 )
          || arg3[0] == '\0' || !is_number( arg3 ) )
        {
            send_to_actor( "Syntax:  bonus [location] [#mod] [#duration]\n\r", ch );
            return;
        }

        pAf             =   new_bonus();
        pAf->location   =   bonus_name_loc( arg1 );
        pAf->modifier   =   atoi( arg2 );
        pAf->type       =   atoi( arg3 );
        pAf->duration   =   0;
        pAf->bitvector  =   0;
        pAf->next       =   pProp->bonus;
        pProp->bonus  =   pAf;

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Bonus added.\n\r", ch);
        return;
    }


    if ( !str_cmp( arg1, "delbonus" ) || !str_cmp( arg1, "rembonus" ) )
    {
        pAf             =   pProp->bonus;

        if ( pAf == NULL )
        {
            send_to_actor( "Prop: Edit:  No affect to remove.\n\r", ch );
            return;
        }

        pProp->bonus  =   pProp->bonus->next;
        free_bonus( pAf );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Bonus removed.\n\r", ch);
        return;
    }


    if ( !str_cmp( arg1, "timer" ) )
    {
        if ( arg2[0] == '\0' || !is_number( arg2 ) )
        {
            send_to_actor( "Syntax:  timer [number]\n\r", ch );
            return;
        }

        pProp->timer = atoi( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Timer set.\n\r", ch);
        return;
    }


    if ( ( value = item_name_type( arg1 ) ) != ITEM_NONE )
    {
        pProp->item_type = value;

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Type set.\n\r", ch);
        return;
    }


    if ( ( value = extra_name_bit( arg1 ) ) != EXTRA_NONE )
    {
        TOGGLE_BIT(pProp->extra_flags, value);

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Extra flag toggled.\n\r", ch);
        return;
    }


    if ( ( value = wear_name_bit( arg1 ) ) != ITEM_WEAR_NONE )
    {
        TOGGLE_BIT(pProp->wear_flags, value);

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Wear flag toggled.\n\r", ch);
        return;
    }



    if ( !str_cmp( arg1, "name" ) )
    {
        if ( arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  name [string]\n\r", ch );
            return;
        }

        free_string( pProp->name );
        pProp->name = str_dup( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Name set.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg1, "plural" ) )
    {
        if ( arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  plural [string]\n\r", ch );
            return;
        }

        free_string( pProp->short_descr_plural );
        pProp->short_descr_plural = str_dup( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Plural set.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg1, "plurallong" ) )
    {
        if ( arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  plurallong [string]\n\r", ch );
            return;
        }

        strcat( arg2, "\n\r" );
        if ( !strstr( arg2, "%s" ) )
        {
            send_to_actor( "String must contain %s in place of numeric.\n\r", ch );
            return;
        }

        free_string( pProp->description_plural );
        pProp->description_plural = str_dup( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Plural long description set.\n\r", ch);
        return;
    }


    if ( !str_cmp( arg1, "short" ) )
    {
        if ( arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  short [string]\n\r", ch );
            return;
        }

        free_string( pProp->short_descr );
        pProp->short_descr = str_dup( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Short description set.\n\r", ch);
        return;
    }


    if ( !str_cmp( arg1, "action" ) )
    {
        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        if ( arg2[0] == '\0' )
        string_append( ch, &pProp->action_descr );
        else
        {
            free_string( pProp->action_descr );
            pProp->action_descr = str_dup( arg2 );
        }
        return;
    }


    if ( !str_cmp( arg1, "description" )  ||  !str_cmp( arg1, "desc" ) )
    {
        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        string_append( ch, &pProp->real_description );
        return;
    }


    if ( !str_cmp( arg1, "long" ) )
    {
        if ( arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  long [string]\n\r", ch );
            return;
        }
        
        strcat( arg2, "\n\r" );

        free_string( pProp->description );
        pProp->description = str_dup( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Long description set.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg1, "level" ) )
    {
        if ( arg2[0] == '\0' || !is_number( arg2 ) )
        {
            send_to_actor( "Syntax:  level [number]\n\r", ch );
            return;
        }

        pProp->level = atoi( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Level set.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg1, "value1") || !str_cmp( arg1, "v1" ) )
    {
        if ( arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  value1 [number|flag]\n\r", ch );
            send_to_actor( "or       v1 [number|flag]\n\r", ch );
            return;
        }

        if ( is_number( arg2 ) )
        pProp->value[0] = atoi( arg2 );
        else TOGGLE_BIT(pProp->value[0],item_valflag( arg2 ));

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Value 1 set.\n\r", ch);
        return;
    }


    if ( !str_cmp( arg1, "value2") || !str_cmp( arg1, "v2" ) )
    {
        if ( arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  value2 [number|flag]\n\r", ch );
            send_to_actor( "or       v1 [number|flag]\n\r", ch ); 
            return;
        }

        if ( is_number( arg2 ) )
        pProp->value[1] = atoi( arg2 );
        else TOGGLE_BIT( pProp->value[1], item_valflag( arg2 ) );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Value 2 set.\n\r", ch);
        return;
    }


    if ( !str_cmp( arg1, "value3") || !str_cmp( arg1, "v3" ) )
    {
        if ( arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  value3 [number|flag]\n\r", ch );
            send_to_actor( "or       v3 [number|flag]\n\r", ch );
            return;
        }

        
        if ( is_number( arg2 ) )
        pProp->value[2] = atoi( arg2 );
        else TOGGLE_BIT( pProp->value[2], item_valflag( arg2 ));

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Value 3 set.\n\r", ch);
        return;
    }


    if ( !str_cmp( arg1, "value4") || !str_cmp( arg1, "v4" ) )
    {
        if ( arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  value4 [number|flag]\n\r", ch );
            send_to_actor( "or       v4 [number|flag]\n\r", ch );
            return;
        }

        if ( is_number( arg2 ) )
        pProp->value[3] = atoi( arg2 );
        else TOGGLE_BIT(pProp->value[3], item_valflag( arg2 ));

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Value 4 set.\n\r", ch);
        return;
    }
                           

    if ( !str_cmp( arg1, "material" ) )
    {
        if ( arg2[0] == '\0' || !is_number( arg2 ) )
        {
            send_to_actor( "Syntax:  material [type]\n\r", ch );
            return;
        }

        value = atoi( arg2 );   /* Change THIS */
/*        pProp->material = value;  */

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Material set.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg1, "size" ) )
    {
        if ( arg2[0] == '\0' || !is_number( arg2 ) )
        {
            send_to_actor( "Syntax:  size [number]\n\r", ch );
            return;
        }

        value = atoi( arg2 );
        pProp->size = value;

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Size set.\n\r", ch);
        return;
    }



    if ( !str_cmp( arg1, "weight" ) )
    {
        if ( arg2[0] == '\0' || !is_number( arg2 ) )
        {
            send_to_actor( "Syntax:  weight [number]\n\r", ch );
            return;
        }

        value = atoi( arg2 );
        pProp->weight = value;

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Weight set.\n\r", ch);
        return;
    }


    if ( !str_cmp( arg1, "cost" ) )
    {
        if ( arg2[0] == '\0' || !is_number( arg2 ) )
        {
            send_to_actor( "Syntax:  cost [number]\n\r", ch );
            return;
        }

        value = atoi( arg2 );
        pProp->cost = value;

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Cost set.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg1, "script" ) )
    {
        INSTANCE *pTrig;
        SCRIPT *script;

        if ( !str_cmp( arg2, "clear") || !str_cmp( arg2, "kill" ) )
        {
            INSTANCE *pNext;

            for ( pTrig = pProp->instances; pTrig != NULL; pTrig = pNext )
            {
                pNext     = pTrig->next;
                free_instance( pTrig );
            }

            pProp->instances = NULL;
            send_to_actor( "Scripts cleared.\n\r", ch );
            return;
        }

        if ( (script = get_script_index(atoi(arg2))) == NULL )
        {
            send_to_actor( "Syntax:  script [dbkey]\n\r", ch );
            return;
        }
        SET_BIT( pZone->zone_flags, ZONE_CHANGED );

        pTrig            = new_instance();
        pTrig->script    = script;

        pTrig->next    = pProp->instances;
        pProp->instances = pTrig;

        send_to_actor( "Script set.\n\r", ch );
        return;
    }

CREATE_COMMANDZ(pProp,prop_template_hash,
                      new_prop_template,
                      get_prop_template,
                      top_dbkey_prop);

    if ( !str_cmp( arg1, "ed" ) )
    {
        if ( arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  ed add [keyword]\n\r", ch );
            send_to_actor( "         ed delete [keyword]\n\r", ch );
            send_to_actor( "         ed edit [keyword]\n\r", ch );
            send_to_actor( "         ed format [keyword]\n\r", ch );
            return;
        }

        argument = one_argument( argument, arg1 );
        strcpy( arg2, argument );

        if ( !str_cmp( arg1, "add" ) )
        {
            if ( arg2[0] == '\0' )
            {
                send_to_actor( "Syntax:  ed add [keyword]\n\r", ch );
                return;
            }

            ed                  =   new_extra_descr();
            ed->keyword         =   str_dup( arg2 );
            ed->next            =   pProp->extra_descr;
            pProp->extra_descr   =   ed;

            string_append( ch, &ed->description );

            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            return;
        }


        if ( !str_cmp( arg1, "edit" ) )
        {
            if ( arg2[0] == '\0' )
            {
                send_to_actor( "Syntax:  ed edit [keyword]\n\r", ch );
                return;
            }

            for ( ed = pProp->extra_descr; ed != NULL; ed = ed->next )
            {
                if ( is_name( arg2, ed->keyword ) )
                    break;
            }

            if ( ed == NULL )
            {
                send_to_actor( "Prop: Edit:  Extra description keyword not found.\n\r", ch );
                return;
            }

            string_append( ch, &ed->description );

            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            return;
        }


        if ( !str_cmp( arg1, "append" ) )
        {
            if ( arg2[0] == '\0' )
            {
                send_to_actor( "Syntax:  ed edit [keyword]\n\r", ch );
                return;
            }

            for ( ed = pProp->extra_descr; ed != NULL; ed = ed->next )
            {
                if ( is_name( arg2, ed->keyword ) )
                    break;
            }

            if ( ed == NULL )
            {
                send_to_actor( "Prop: Edit:  Extra description keyword not found.\n\r", ch );
                return;
            }

            string_append( ch, &ed->description );

            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            return;
        }


        if ( !str_cmp( arg1, "delete" ) )
        {
            EXTRA_DESCR *ped;

            if ( arg2[0] == '\0' )
            {
                send_to_actor( "Syntax:  ed delete [keyword]\n\r", ch );
                return;
            }

            ped = NULL;

            for ( ed = pProp->extra_descr; ed != NULL; ed = ed->next )
            {
                if ( is_name( arg2, ed->keyword ) )
                    break;
                ped = ed;
            }

            if ( ed == NULL )
            {
                send_to_actor( "Prop: Edit:  Extra description keyword not found.\n\r", ch );
                return;
            }

            if ( ped == NULL )
            {
                pProp->extra_descr = ed->next;
            }
            else
            {
                ped->next = ed->next;
            }

            free_extra_descr( ed );

            send_to_actor( "Extra description deleted.\n\r", ch );
            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            return;
        }


        if ( !str_cmp( arg1, "format" ) )
        {
            EXTRA_DESCR *ped;

            if ( arg2[0] == '\0' )
            {
                send_to_actor( "Syntax:  ed format [keyword]\n\r", ch );
                return;
            }

            ped = NULL;

            for ( ed = pProp->extra_descr; ed != NULL; ed = ed->next )
            {
                if ( is_name( arg2, ed->keyword ) )
                    break;
                ped = ed;
            }

            if ( ed == NULL )
            {
                send_to_actor( "Prop: Edit:  Extra description keyword not found.\n\r", ch );
                return;
            }

            ed->description = format_string( ed->description );

            send_to_actor( "Extra description formatted.\n\r", ch );
            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            return;
        }
    }


    interpret( ch, arg );
    return;
}




void aedit( PLAYER *ch, char *argument )
{
    ZONE *pZone;
    ACTOR_TEMPLATE *pActor;
    char arg[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int  value;
    int  iHash;

    strcpy( arg, argument );
    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    strcpy( arg2, argument );
    value = atoi( arg2 );

    pActor = (ACTOR_TEMPLATE *)ch->desc->pEdit;
    pZone = pActor->zone;
 
    if ( !IS_BUILDER( ch, pZone ) )
    {
         send_to_actor( "Actor: Edit:  Insufficient security to modify actor.\n\r", ch );
    }

    if ( !str_cmp( arg1, "show" ) || arg1[0] == '\0' )
    {
        clrscr(ch);
        snprintf( buf, MAX_STRING_LENGTH, "%d %s", pActor->dbkey, arg2 );
        cmd_aindex( ch, buf );
        return;
    }


    if ( !str_cmp( arg1, "done" ) )
    {
        ch->desc->pEdit = NULL;
        ch->desc->connected = NET_PLAYING;
        cmd_zsave( ch, "changed" );
        return;
    }

    if ( !str_cmp( arg1, "?" ) )
    {
        cmd_help( ch, "AEDIT" );
        return;
    }

    if ( !str_cmp( arg1, "copy" ) ) {
        ACTOR_TEMPLATE *pActor_from;
        argument = one_argument( argument, arg1 );

        if ( !is_number( arg1 ) ) {
            send_to_actor( "Copies basic actor information TO this record:\n\rcopy [dbkey-to-copy-from]\n\r", ch );
            return;
        }

        pActor_from = get_actor_template( atoi( arg1 ) );
        if ( !pActor_from ) {
            send_to_actor( "Invalid actor dbkey to copy from.\n\r", ch );
            return;
        }

        free_string( pActor->name );
        free_string( pActor->short_descr );
        free_string( pActor->long_descr );
        free_string( pActor->description );

        pActor->name        = str_dup( pActor_from->name        );
        pActor->short_descr = str_dup( pActor_from->short_descr );
        pActor->long_descr  = str_dup( pActor_from->long_descr  );
        pActor->description = str_dup( pActor_from->description );

        pActor->count       = pActor_from->count;
        pActor->race        = pActor_from->race;
        pActor->sex         = pActor_from->sex;
        pActor->exp         = pActor_from->exp;
        pActor->karma       = pActor_from->karma;
        pActor->flag         = pActor_from->flag;
        pActor->timer       = pActor_from->timer;
        pActor->money       = pActor_from->money;
        pActor->credits     = pActor_from->credits;
        pActor->bucks       = pActor_from->bucks;
        pActor->size        = pActor_from->size;
        pActor->cost        = pActor_from->cost;
        pActor->perm_str    = pActor_from->perm_str;
        pActor->perm_int    = pActor_from->perm_int;
        pActor->perm_wis    = pActor_from->perm_wis;
        pActor->perm_dex    = pActor_from->perm_dex;
        pActor->perm_con    = pActor_from->perm_con;
        pActor->fmode       = pActor_from->fmode;

        /*
         * Copy skills. (appends)
         */
        { SKILL *pSkill;
        for ( pSkill = pActor_from->learned; pSkill != NULL; pSkill = pSkill->next )
        {
           SKILL *pNew = skill_copy( pSkill );
           pNew->next = pActor->learned;
           pActor->learned = pNew;
        }
        }

        send_to_actor( "Basic information copied to this actor.\n\r", ch );
        return;        
    }

    if ( !str_cmp( arg1, "spells" ) || !str_cmp( arg1, "spells" ) ) {
             SPELL *pSpell;
        SPELL_BOOK *pSpellBook;

         if ( !str_cmp( arg2, "clear" ) ) {
         clear_spell_book( pActor->pSpells );
         pActor->pSpells = NULL;
         send_to_actor( "Actor spell list cleared.\n\r", ch );  
         return; 
         }  

         pSpell = get_spell_index( atoi(arg2) );
         if ( pSpell != NULL ) {
              pSpellBook = new_spell_book( );
              pSpellBook->dbkey = pSpell->dbkey;
              pSpellBook->next = pActor->pSpells; 
              pActor->pSpells = pSpellBook;
              send_to_actor( "Spell added to actor's book.\n\r", ch );
              return;
         }  

        snprintf( buf, MAX_STRING_LENGTH, "%s's spell knowledge:\n\r", pActor->short_descr );
        send_to_actor( buf, ch );
        for ( pSpellBook = pActor->pSpells;  pSpellBook != NULL;  pSpellBook = pSpellBook->next )
        {
             pSpell = get_spell_index ( pSpellBook->dbkey ); 
             if ( pSpell != NULL )   {
             send_to_actor( pSpell->name, ch );
             send_to_actor( "\n\r", ch );
             }
             else send_to_actor( "Invalid spell.\n\r", ch );
        }
        return; 
    }       

    if ( !str_cmp( arg1, "skills" ) )
    {
        SKILL *pSkill;
        int col=0;

        for ( pSkill=pActor->learned; pSkill != NULL; pSkill = pSkill->next )
        {
            col++;
            snprintf( buf, MAX_STRING_LENGTH, "%20s/%3d ",
                     trunc_fit( pSkill->name, 20 ),
                     pSkill->skill_level );
            send_to_actor( buf, ch );
            if ( col % 3 == 0 )
            {
                send_to_actor( "\n\r", ch );
                col = 0;
            }
        }

        if ( col % 3 != 0 )  send_to_actor( "\n\r", ch );
        return;
    }


    if ( is_number( arg1 ) )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%d", atoi(arg1) );
        cmd_aedit( ch, buf );
        return;
    }

    if ( !IS_BUILDER( ch, pZone ) )
    {
        interpret( ch, arg );
        return;
    }


    if ( !str_cmp( arg1, "shop" ) )
    {
        SHOP *pShop;

        argument = one_argument( argument, arg1 );
        strcpy( arg2, argument );

        if ( arg1[0] == '\0' )
        {
            send_to_actor( "Syntax:  shop hours [#opening] [#closing]\n\r", ch );
            send_to_actor( "         shop profit [#buying%] [#selling%]\n\r", ch );
            send_to_actor( "         shop type [#] [item type]\n\r", ch );
//            send_to_actor( "         shop buy #-goods-index\n\r", ch );
//            send_to_actor( "         shop sell #-goods-index\n\r", ch );
//            send_to_actor( "         shop comp #-components-index\n\r", ch );
            send_to_actor( "         shop rate [cost]\n\r", ch );
            send_to_actor( "         shop trades [number] [1=will buy]\n\r", ch );
            send_to_actor( "         shop [str1..str4] [string]\n\r", ch );
            send_to_actor( "         shop [trader|peddler|repair|components|credits|bucks]\n\r", ch );
            send_to_actor( "         shop delete\n\r", ch );
            return;
        }



        if ( pActor->pShop == NULL )
        {
            if ( !str_cmp( arg1, "delete" ) )
            {
                send_to_actor( "No shop to delete.\n\r", ch );
                return;
            }

            pActor->pShop         = new_shop();
            pActor->pShop->keeper = pActor->dbkey;
            shop_last->next     = pActor->pShop;
            send_to_actor( "Shop created.\n\r", ch );
            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        }

        pShop = pActor->pShop;

        if ( (value = shop_name_bit( arg1 )) != -1 )
        {
            TOGGLE_BIT(pShop->shop_flags, value);
            send_to_actor( "Shop flag toggled.\n\r", ch );
            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            return;
        }

        if ( !str_cmp( arg1, "str1" ) )
        {
            free_string( pShop->no_such_item );
            pShop->no_such_item = str_dup( arg2 );
            send_to_actor( "Str1 set.\n\r", ch );
            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            return;
        }

        if ( !str_cmp( arg1, "str2" ) )
        {
            free_string( pShop->cmd_not_buy );
            pShop->cmd_not_buy = str_dup( arg2 );
            send_to_actor( "Str2 set.\n\r", ch );
            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            return;
        }

        if ( !str_cmp( arg1, "str3" ) )
        {
            free_string( pShop->list_header );
            pShop->list_header = str_dup( arg2 );
            send_to_actor( "Str3 set.\n\r", ch );
            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            return;
        }

        if ( !str_cmp( arg1, "str4" ) )
        {
            free_string( pShop->hours_excuse );
            pShop->hours_excuse = str_dup( arg2 );
            send_to_actor( "Str4 set.\n\r", ch );
            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            return;
        }

        if ( !str_cmp( arg1, "buy" ) )
        {
            pShop->buy_index = atoi( arg2 );
            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            return;
        }

        if ( !str_cmp( arg1, "sell" ) )
        {
            pShop->sell_index = atoi( arg2 );
            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            return;
        }

        if ( !str_cmp( arg1, "comp" ) )
        {
            pShop->comp_index = atoi( arg2 );
            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            return;
        }


        if ( !str_cmp( arg1, "rate" ) )
        {
            if ( !is_number( arg2 ) )
            {
                send_to_actor( "Syntax:  shop rate [#]\n\r", ch );
                return;
            }

            pShop->repair_rate = atoi( arg2 );
            send_to_actor( "Shop repair rate set.\n\r", ch );
            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            return;
        }

        if ( !str_cmp( arg1, "hours" ) )
        {
            argument = one_argument( argument, arg1 );
            strcpy( arg2, argument );

            if ( arg1[0] == '\0' || !is_number(arg1)
              || arg2[0] == '\0' || !is_number(arg2) )
            {
                send_to_actor( "Syntax:  shop hours [#opening] [#closing]\n\r", ch );
                return;
            }

            pShop->open_hour = atoi( arg1 );
            pShop->close_hour = atoi( arg2 );

            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            send_to_actor( "Shop hours set.\n\r", ch);
            return;
        }


        if ( !str_cmp( arg1, "profit" ) )
        {
            argument = one_argument( argument, arg1 );
            strcpy( arg2, argument );

            if ( arg1[0] == '\0' || !is_number( arg1 )
              || arg2[0] == '\0' || !is_number( arg2 ) )
            {
                send_to_actor( "Syntax:  shop profit [#buying%] [#selling%]\n\r", ch );
                return;
            }

            pShop->profit_buy     = atoi( arg1 );
            pShop->profit_sell    = atoi( arg2 );

            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            send_to_actor( "Shop profit set.\n\r", ch);
            return;
        }


        if ( !str_cmp( arg1, "type" ) )
        {
            int iTrade;
            argument = one_argument( argument, arg1 );
            strcpy( arg2, argument );

            if ( arg1[0] == '\0' || !is_number( arg1 )
              || arg2[0] == '\0' )
            {
                send_to_actor( "Syntax:  shop type [#] [item type]\n\r", ch );
                return;
            }

            if ( atoi(arg1)-1 > MAX_TRADE )
            {
                snprintf( buf, MAX_STRING_LENGTH, "%d", MAX_TRADE );
                send_to_actor( "Shop keepers may only sell ", ch );
                send_to_actor( buf, ch );
                send_to_actor( " items max.\n\r", ch );
                return;
            }

            if ( ( value = item_name_type(arg2) ) == ITEM_NONE )
            {
                send_to_actor( "Actor: Edit:  That type of item is not known.\n\r", ch );
                return;
            }

            iTrade = atoi(arg1)-1;

            if ( iTrade < 0 || iTrade >= MAX_TRADE )
            {
                send_to_actor( "Syntax:  shop type [#] [item type]\n\r", ch );
                return;
            }

            pShop->buy_type[iTrade] = item_name_type( arg2 );

            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            send_to_actor( "Shop type set.\n\r", ch);
            return;
        }

        if ( !str_cmp( arg1, "trades" ) )
        {
            int iTrade;
            argument = one_argument( argument, arg1 );
            strcpy( arg2, argument );

            if ( arg1[0] == '\0' || !is_number( arg1 )
              || arg2[0] == '\0' )
            {
                send_to_actor( "Syntax:  shop trades [#] [1-100, 0=off]\n\r", ch );
                return;
            }

            if ( atoi(arg1)-1 > MAX_TRADE )
            {
                snprintf( buf, MAX_STRING_LENGTH, "%d", MAX_TRADE );
                send_to_actor( "Shop keepers may only sell ", ch );
                send_to_actor( buf, ch );
                send_to_actor( " goods types.\n\r", ch );
                return;
            }

            iTrade = atoi(arg1)-1;

            if ( iTrade < 0 || iTrade >= MAX_TRADE )
            {
                send_to_actor( "Syntax:  shop trades [#] [1-100+, 0=off]\n\r", ch );
                return;
            }

            pShop->trades[iTrade] = atoi( arg2 );

            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            send_to_actor( "Shop type set.\n\r", ch);
            return;
        }


        if ( !str_cmp( arg1, "delete" ) )
        {
            if ( pShop == NULL )
            {
                send_to_actor( "Cannot delete a shop that is non-existant.\n\r", ch );
                return;
            }

            free_shop( pActor->pShop );
            pActor->pShop = NULL;

            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            send_to_actor( "Shop deleted.\n\r", ch);
            return;
        }

        send_to_actor( "Type 'shop' alone for help.\n\r", ch );
        return;
    }


    if ( !str_cmp( arg1, "name" ) )
    {
        if ( arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  name [string]\n\r", ch );
            return;
        }

        free_string( pActor->name );
        pActor->name = str_dup( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Name set.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg1, "attributes" ) || !str_cmp( arg1, "attrib" ) )
    {
        pActor->perm_str = URANGE( 0, atoi( arg2 ), 25 );
        pActor->perm_con = URANGE( 0, atoi( arg2 ), 100 );
        pActor->perm_int = URANGE( 0, atoi( arg2 ), 100 );
        pActor->perm_wis = URANGE( 0, atoi( arg2 ), 100 );
        pActor->perm_dex = URANGE( 0, atoi( arg2 ), 25 );
        send_to_actor( "Attributes set.\n\r", ch );
        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        return;
    }

    if ( !str_cmp( arg1, "strength" ) || !str_cmp( arg1, "str" ) )
    {
        pActor->perm_str = URANGE( 0, atoi( arg2 ), 25 );
        send_to_actor( "Strength set.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "intelligence" ) || !str_cmp( arg1, "int" ) )
    {
        pActor->perm_int = URANGE( 0, atoi( arg2 ), 25 );
        send_to_actor( "Intelligence set.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "wisdom" ) || !str_cmp( arg1, "wis" ) )
    {
        pActor->perm_wis = URANGE( 0, atoi( arg2 ), 25 );
        send_to_actor( "Wisdom set.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "dexterity" ) || !str_cmp( arg1, "dex" ) )
    {
        pActor->perm_dex = URANGE( 0, atoi( arg2 ), 25 );
        send_to_actor( "Dexterity set.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "constitution" ) || !str_cmp( arg1, "con" ) )
    {
        pActor->perm_con = URANGE( 0, atoi( arg2 ), 25 );
        send_to_actor( "Constitution set.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "size" ) )
    {
        pActor->size = atoi( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Size set.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg1, "race" ) )
    {
        int rn;

        if ( arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  race [name]\n\r", ch );
            return;
        }

        if ( !is_number(arg2) )
        for ( rn = 0;  rn < MAX_RACE; rn++ )
        {
            if ( !str_prefix( arg2, RACE(rn,race_name) ) )
                break;
        }
        else
        {
            rn = atoi(arg1);
            rn = race_lookup( rn );
        }

        pActor->race = rn;

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Race set.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg1, "skill" ) )
    {
        SKILL *pSkill;

        argument = one_argument( argument, arg1 );
        strcpy( arg2, argument );

        if ( arg1[0] == '\0' || arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  skill [skill] [percentage]\n\r", ch );
            send_to_actor( "Once a skill is added it can only be manipulated.\n\rSkills of 0 don't save\n\r", ch );
            return;
        }

        pSkill = skill_lookup( arg1 );
        if ( !pSkill && str_cmp( arg1, "all" ) )
        {
            send_to_actor( "Invalid skill.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg1, "all" ) )
        {
            int sn;

            for ( sn = 0; sn < top_dbkey_skill; sn++ )
            {
                SKILL *pNewSkill;

                pSkill = get_skill_index( sn );
                if ( !pSkill ) continue;

                pNewSkill = skill_copy( pSkill );
                pNewSkill->next = pActor->learned;
                pActor->learned = pNewSkill;
                pNewSkill->skill_level = URANGE( 0, atoi( arg2 ), 100 );
            }
        }
        else    
        if ( pSkill )  {
           SKILL *pActorSkill;
  
           for ( pActorSkill = pActor->learned; pActorSkill != NULL;
                 pActorSkill = pActorSkill->next ) {
                if ( pSkill->dbkey == pActorSkill->dbkey ) break;
           }

           if ( !pActorSkill ) {
              pActorSkill = skill_copy( pSkill );
              pActorSkill->next = pActor->learned;
              pActor->learned = pActorSkill;
           }

           pActorSkill->skill_level = URANGE( 0, atoi( arg2 ), 100 );
        }

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Skill set.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg1, "short" ) )
    {
        if ( arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  short [string]\n\r", ch );
            return;
        }

        free_string( pActor->short_descr );
        pActor->short_descr = str_dup( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Short description set.\n\r", ch);
        return;
    }


    if ( !str_cmp( arg1, "long" ) )
    {
        if ( arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  long [string]\n\r", ch );
            return;
        }

        strcat( arg2, "\n\r" );
        free_string( pActor->long_descr );
        pActor->long_descr = str_dup( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Long description set.\n\r", ch);
        return;
    }


    if ( !str_cmp( arg1, "desc" ) || !str_cmp( arg1, "description" ) )
    {
        if ( arg2[0] == '\0' )
        {
            string_append( ch, &pActor->description );
            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            return;
        }

        if ( arg2[0] == '+' )
        {
            string_append( ch, &pActor->description );
            SET_BIT( pZone->zone_flags, ZONE_CHANGED );
            return;
        }

        send_to_actor( "Syntax:  desc    - line edit\n\r", ch );
        send_to_actor( "         desc +  - line append\n\r",ch );
        return;
    }


    if ( get_actor_sex_number( arg1 ) != SEX_NONE )
    {
        pActor->sex = get_actor_sex_number( arg1 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Sex set.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg1, "timer" ) )
    {
        if ( arg2[0] == '\0' || !is_number( arg2 ) )
        {
            send_to_actor( "Syntax:  timer [ticks]\n\r", ch );
            return;
        }

        pActor->timer = atoi( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Timer set.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg1, "exp" ) )
    {
        if ( arg2[0] == '\0' || !is_number( arg2 ) )
        {
            send_to_actor( "Syntax:  timer [ticks]\n\r", ch );
            return;
        }

        pActor->exp = atoi( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Exp set.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg1, "karma" ) )
    {
        if ( arg2[0] == '\0' || !is_number( arg2 ) )
        {
            send_to_actor( "Syntax:  karma [ticks]\n\r", ch );
            return;
        }

        pActor->karma = atoi( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Karma set.\n\r", ch);
        return;
    }


    if ( ( value = actor_name_bit( arg1 ) ) != ACTOR_NONE )
    {
        TOGGLE_BIT(pActor->flag, value);
        SET_BIT( pActor->flag, ACTOR_NPC );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Act flag toggled.\n\r", ch);
        return;
    }


    if ( ( value = bonus_name_bit( arg1 ) ) != AFFECT_NONE )
    {
        TOGGLE_BIT(pActor->bonuses, value);

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Bonus flag toggled.\n\r", ch);
        return;
    }


    if ( !str_cmp( arg1, "attack" ) )
    {
        int i;
        int iType;

        argument = one_argument( argument, arg1 );
        strcpy( arg2, argument );

        if ( !is_number( arg1 )  )
        {
            send_to_actor( "Syntax:  attack [num] [weapon type] [min dam] [max dam]\n\r", ch );
            send_to_actor( "         attack [num] delete\n\r",                            ch );
            return;
        }

        i = URANGE( 0, atoi( arg1 ), MAX_ATTACK-1 );

        argument = one_argument( argument, arg1 );
        strcpy( arg2, argument );

        if ( !str_cmp( arg1, "delete" ) )
        {
            if ( pActor->attacks[i] != NULL )
            {
                 free_attack( pActor->attacks[i] );
                 pActor->attacks[i] = NULL;
                 send_to_actor( "Attack deleted.\n\r", ch );
                 return;
            }
            else
            {
                send_to_actor( "Attack does not exist already.\n\r", ch );
                return;
            }
        }

        for ( iType = 0; iType < MAX_ATTACK; iType++ )
        {
            if ( !str_cmp( arg1, attack_table[iType].name ) )
            break;
        }

        if ( iType >= MAX_ATTACK )
        {
            send_to_actor( "Invalid weapon type (see 'table attack').\n\r", ch );
            return;
        }

        if ( pActor->attacks[i] == NULL )
        {
            send_to_actor( "Attack added.\n\r", ch );
            pActor->attacks[i] = new_attack( );
        }
        else
        {
            send_to_actor( "Attack modified.\n\r", ch );
        }

        pActor->attacks[i]->next = NULL;

        pActor->attacks[i]->idx     = iType;

        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );

        pActor->attacks[i]->dam1     = atoi( arg1 );
        pActor->attacks[i]->dam2     = atoi( arg2 );

        return;
    }

    if ( !str_cmp( arg1, "script" ) )
    {
        INSTANCE *pTrig;
        SCRIPT *script;

        if ( !str_cmp( arg2, "kill" ) )
        {
            INSTANCE *pNext;

            for ( pTrig = pActor->instances; pTrig != NULL;  )
            {
                pNext     = pTrig->next;
                free_instance( pTrig );
                pTrig = pNext;
            }

            pActor->instances = NULL;

            send_to_actor( "Scripts cleared.\n\r", ch );
            return;
        }

        if ( (script = get_script_index(atoi(arg2))) == NULL )
        {
            send_to_actor( "Syntax:  script [dbkey]\n\r", ch );
            return;
        }
        SET_BIT( pZone->zone_flags, ZONE_CHANGED );

        pTrig            = new_instance();
        pTrig->script    = script;

        pTrig->next    = pActor->instances;
        pActor->instances = pTrig;

        send_to_actor( "Script set.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "money" ) )
    {
        if ( arg2[0] == '\0' || !is_number( arg2 ) )
        {
            send_to_actor( "Syntax:  money [amount in copper]\n\r", ch );
            return;
        }

        pActor->money = atoi( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Monetary amount set.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg1, "credits" ) )
    {
        if ( arg2[0] == '\0' || !is_number( arg2 ) )
        {
            send_to_actor( "Syntax:  credits [#monetary units]\n\r", ch );
            return;
        }

        pActor->credits = atoi( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Credit (MU) amount set.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg1, "bucks" ) )
    {
        if ( arg2[0] == '\0' || !is_number( arg2 ) )
        {
            send_to_actor( "Syntax:  bucks [#dollars]\n\r", ch );
            return;
        }

        pActor->bucks = atoi( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Buck$ set.\n\r", ch);
        return;
    }
    

    if ( !str_cmp( arg1, "create" ) )
    {
        argument = one_argument( argument, arg2 );
        value = atoi( arg2 );
        if ( arg2[0] == '\0' || value == 0 )
        {
            send_to_actor( "Syntax:  aedit create [dbkey]\n\r", ch );
            return;
        }
            ch->desc->connected=NET_AEDITOR;
            CREATE_COMMANDZ(pActor,actor_template_hash,
                                   new_actor_template,
                                   get_actor_template,
                                   top_dbkey_actor);   return;

    }


    interpret( ch, arg );
    return;
}


void sedit( PLAYER *ch, char *argument )
{
    ZONE *pZone;
    SCRIPT *script;
    char arg[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int  value;
    int  iHash;

    strcpy( arg, argument );
    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    strcpy( arg2, argument );
    value = atoi( arg2 );

    script = (SCRIPT *)ch->desc->pEdit;
    pZone = !script ? NULL : get_dbkey_zone(script->dbkey);
 
    if ( pZone == NULL || !IS_BUILDER( ch, pZone ) )
    {
         send_to_actor( "Script: Edit:  Insufficient security to modify script.\n\r", ch );
    }

    /*
     * Set this script as your trace.
     */
    if ( !str_cmp( arg1, "trace" ) && !NPC(ch) )
    {
        if ( ch->userdata->trace != NULL )
        {
            send_to_actor( "Trace deactivated.\n\r", ch );
            ch->userdata->trace = NULL;
            ch->userdata->trace_wait = 0;
        }
        else
        {
            send_to_actor( "Trace activated.\n\r", ch );
            ch->userdata->trace = script;
            ch->userdata->trace_wait = value;
        }

        return;
    }

    if ( !str_cmp( arg1, "show" ) )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%d %s", script->dbkey, arg2 );
        cmd_sindex( ch, buf );
        return;
    }


    if ( !str_cmp( arg1, "done" ) )
    {
        ch->desc->pEdit = NULL;
        ch->desc->connected = NET_PLAYING;
        cmd_zsave( ch, "changed" );
        return;
    }

    if ( !str_cmp( arg1, "?" ) )
    {
        cmd_help( ch, "SCEDIT" );
        return;
    }

    if ( !pZone || !IS_BUILDER( ch, pZone ) )
    {
        interpret( ch, arg );
        return;
    }

    if ( !str_cmp( arg1, "name" ) )
    {
        if ( arg2[0] == '\0' )
        {
            send_to_actor( "Syntax:  name [string]\n\r", ch );
            return;
        }

        free_string( script->name );
        script->name = str_dup( arg2 );

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Name set.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg1, "script" ) || !str_cmp( arg1, "cmd" ) )
    {
        string_append( ch, &script->commands );
        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        return;
    }

    if ( (value = get_script_type( arg1 )) != -1  )
    {
        script->type  = value;

        SET_BIT( pZone->zone_flags, ZONE_CHANGED );
        send_to_actor( "Type set.\n\r", ch);
        return;
    }

CREATE_COMMANDZ(script,script__hash,
                       new_script,
                       get_script_index,
                       top_dbkey_script);

    interpret( ch, arg );
    return;
}



/*
 * Syntax: zedit [num]
 *         zedit create [dbkey]
 */
void cmd_zedit( PLAYER *ch, char *argument )
{
    ZONE *pZone;
    int value;

    if ( NPC(ch) ) return;

    pZone = ch->in_scene->zone;

    if ( is_number( argument ) )
    {
        value = atoi( argument );
        if ( ( pZone = get_zone( value ) ) == NULL )
        {
            send_to_actor( "That zone dbkey does not exist.\n\r", ch );
            return;
        }
    }
    else
    {
        if ( !str_cmp( argument, "create" ) )
        {
            pZone               =   new_zone();
            zone_last->next     =   pZone;
            zone_last           =   pZone;
            SET_BIT( pZone->zone_flags, ZONE_ADDED );
        }
    }

    ch->desc->pEdit = (void *)pZone;
    ch->desc->connected = NET_ZEDITOR;
    return;
}



/*
 * Syntax:  redit
 *          redit [dbkey]
 *          redit reset
 *          redit create [dbkey]
 */
void cmd_redit( PLAYER *ch, char *argument )
{
    SCENE *pScene;
    ZONE *pZone;
    int value;
    int iHash;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];

    if ( NPC(ch) ) return;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    pScene = ch->in_scene;

    if ( is_number( arg1 ) )
    {
        value = atoi( arg1 );
        if ( ( pScene = get_scene( value ) ) == NULL )
        {
            send_to_actor( "Scene Edit:  That dbkey does not exist.\n\r", ch );
            return;
        }
    }
    else
    {
        if ( !str_cmp( arg1, "reset" ) )
        {
            spawn_scene( pScene );
            send_to_actor( "Scene cued.\n\r", ch );
            return;
        }
        else 
        if ( !str_cmp( arg1, "create" ) )
        {
            value = atoi( arg2 );
            if ( arg2[0] == '\0' || value == 0 )
            {
                send_to_actor( "Syntax:  redit create [dbkey]\n\r", ch );
                return;
            }
CREATE_COMMANDZ(pScene,scene_hash,
                       new_scene,
                       get_scene,
                       top_dbkey_scene);
               if ( get_scene( value ) != NULL ) 
               { char b[MSL]; snprintf( b, MSL, "%d", value );
                 cmd_goto( ch,b ); }
               ch->desc->connected = NET_REDITOR;
        }
    }

   ch->desc->connected = NET_REDITOR;
    return;
}




/*
 * Syntax:  oedit [dbkey]
 *          oedit create [dbkey]
 */
void cmd_oedit( PLAYER *ch, char *argument )
{
    PROP_TEMPLATE *pProp;
    ZONE *pZone;
    int value;
    int iHash;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];

    if ( NPC(ch) ) return;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( is_number( arg1 ) )
    {
        value = atoi( arg1 );
        if ( ( pProp = get_prop_template( value ) ) == NULL )
        {
            send_to_actor( "Prop: Edit:  That dbkey does not exist.\n\r", ch );
            return;
        }

        ch->desc->pEdit = (void *)pProp;
        ch->desc->connected = NET_OEDITOR;
        return;
    }
    else
    {
CREATE_COMMANDZ(pProp,prop_template_hash,
                      new_prop_template,
                      get_prop_template,
                      top_dbkey_prop);
    }

    send_to_actor( "Prop: Edit:  There is no default prop to edit.\n\r", ch );
    return;
}




/*
 * Syntax:  aedit [dbkey]
 *          aedit create [dbkey]
 */
void cmd_aedit( PLAYER *ch, char *argument )
{
    ACTOR_TEMPLATE *pActor;
    ZONE *pZone;
    int value;
    int iHash;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];

    if ( NPC(ch) ) return;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( is_number( arg1 ) )
    {
        value = atoi( arg1 );
        if ( ( pActor = get_actor_template( value ) ) == NULL )
        {
            send_to_actor( "Actor: Edit:  That dbkey does not exist.\n\r", ch );
            return;
        }

        ch->desc->pEdit = (void *)pActor;
        ch->desc->connected = NET_AEDITOR;
        return;
    }
    else
    {
        if ( !str_cmp( arg1, "create" ) )
        {
            value = atoi( arg2 );
            if ( arg2[0] == '\0' || value == 0 )
            {
                send_to_actor( "Syntax:  aedit create [dbkey]\n\r", ch );
                return;
            }
            ch->desc->connected=NET_AEDITOR;
            CREATE_COMMANDZ(pActor,actor_template_hash,
                                   new_actor_template,
                                   get_actor_template,
                                   top_dbkey_actor);   return;
        }
    }

    send_to_actor( "Actor: Edit:  There is no default actor to edit.\n\r", ch );
    return;
}



/*
 * Syntax:  sedit [dbkey]
 *          sedit create [dbkey]
 */
void cmd_sedit( PLAYER *ch, char *argument )
{
    SCRIPT *script;
    int value;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];

    if ( NPC(ch) ) return;

    if ( MTD(argument) ) { cmd_scfind( ch, "zone" ); return; }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( is_number( arg1 ) )
    {
        value = atoi( arg1 );
        if ( ( script = get_script_index( value ) ) == NULL )
        {
            send_to_actor( "Script: Edit:  That dbkey does not exist.\n\r", ch );
            return;
        }

        ch->desc->pEdit = (void *)script;
        ch->desc->connected = NET_SEDITOR;
        return;
    }
    else
    {
            ch->desc->connected = NET_SEDITOR;
            CREATE_COMMAND(script,script__hash,
                                  new_script,
                                  get_script_index,
                                  top_dbkey_script);
            return;
    }

    send_to_actor( "Script: Edit:  There is no default script to edit.\n\r", ch );
    return;
}



/*
 * Syntax:  aindex [dbkey]
 */
void cmd_aindex( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ACTOR_TEMPLATE *victim;
    INSTANCE *pTrig;
    int iAtt;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' )
    {
    send_to_actor( "Mindex whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_actor_template( atoi( arg ) ) ) == NULL )
    {
    send_to_actor( "Invalid actor index VNUM.\n\r", ch );
        return;
    }

    snprintf( buf, MAX_STRING_LENGTH, "Name:   [^B%s^N]\n\r", victim->name );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Vnum:     [^B%5d^N]  Zone:   [^B%5d^N] %s\n\r",
            victim->dbkey,
    victim->zone == NULL ? -1        : victim->zone->dbkey,
    victim->zone == NULL ? "No zone" : victim->zone->name );
    send_to_actor( buf, ch );

    if ( !str_cmp( arg2, "skills" )  )
    {
        SKILL *pSkill;
        for ( pSkill=victim->learned; pSkill != NULL;  pSkill = pSkill->next )
        {
                snprintf( buf, MAX_STRING_LENGTH, "Skill: ^B%s^M at ^B%d%%^N\n\r", pSkill->name, 
                              pSkill->skill_level );
                send_to_actor( buf, ch );
        }
        return;
    }

    snprintf( buf, MAX_STRING_LENGTH, "Short:  [^B%s^N]\n\rLong:\n\r%s",
    victim->short_descr,
    victim->long_descr );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Act:    [^B%s^N]\n\r", actor_bit_name( victim->flag ) );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Bonus: [^B%s^N]\n\r",
                  bonus_bit_name( victim->bonuses ) );
    send_to_actor( buf, ch );

    {
        int race = race_lookup( victim->race );

        snprintf( buf, MAX_STRING_LENGTH, "Sz: [^B%3d^N]  Race: [^B%s^N]  Sex: [^B%s^N]  Karma: [^B%d^N]  Exp: [^B%d^N]\n\r",
            victim->size,
            RACE(race,race_name),
            victim->sex == SEX_MALE    ? "male"   :
            victim->sex == SEX_FEMALE  ? "female" : "neutral",
            victim->karma, victim->exp );
        send_to_actor( buf, ch );
    }

    snprintf( buf, MAX_STRING_LENGTH, "Money:  [^B%5d^N]  Credits:  [^B%5d^N]   Bucks:  [$^B%5d^N]\n\r", victim->money, victim->credits, victim->bucks );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Attrib { Str: [^B%2d^N]  Int: [^B%2d^N]  Wis: [^B%2d^N]  Dex: [^B%2d^N]  Con: [^B%2d^N]\n\r",
        victim->perm_str,
        victim->perm_int,
        victim->perm_wis,
        victim->perm_dex,
        victim->perm_con );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Description:\n\r%s", victim->description );
    send_to_actor( buf, ch );

    for ( iAtt = 0; iAtt < MAX_ATTACK; iAtt++ )
    {
        if ( victim->attacks[iAtt] != NULL )
        {
            snprintf( buf, MAX_STRING_LENGTH, "[^B%d^N] ^B%ss^N for [^B%d^N] to [^B%d^N] damage.\n\r", iAtt,
                          attack_table[victim->attacks[iAtt]->idx].name,
                          victim->attacks[iAtt]->dam1,
                          victim->attacks[iAtt]->dam2 );
            buf[0] = UPPER(buf[0]);
            send_to_actor( buf, ch );
        }
    }


    for ( pTrig = victim->instances; pTrig != NULL; pTrig = pTrig->next )
    {
        snprintf( buf, MAX_STRING_LENGTH, "[^4%5d^N] %s\n\r", pTrig->script->dbkey, pTrig->script->name );
        send_to_actor( buf, ch );
    }


    if ( victim->pShop != NULL )
    {
        SHOP *pShop;
        int iTrade;

        pShop = victim->pShop;

        snprintf( buf, MAX_STRING_LENGTH, "Shop data (for %d):\n\r    Will buy at %d%%, and sell at %d%%.  Opened %d-%d.\n\r",
                      pShop->keeper, pShop->profit_buy, pShop->profit_sell,
                      pShop->open_hour, pShop->close_hour );
        send_to_actor( buf, ch );

        snprintf( buf, MAX_STRING_LENGTH, "    Flags: [^B%s^N]\n\r", shop_bit_name(pShop->shop_flags) );
        send_to_actor( buf, ch );

        if ( IS_SET(pShop->shop_flags, SHOP_REPAIR) )
        {
        snprintf( buf, MAX_STRING_LENGTH, "    Repair Rate: [^B%5d^N]\n\r", pShop->repair_rate );
        send_to_actor( buf, ch );
        }

//        snprintf( buf, MAX_STRING_LENGTH, "Buy Index: ^B%d^N   Sell Index:  ^B%d^N   Component Index:  ^B%d^N\n\r",
//                 pShop->buy_index, pShop->sell_index, pShop->comp_index );
//        send_to_actor( buf, ch );

        if ( IS_SET(pShop->shop_flags, SHOP_PEDDLER)
          || IS_SET(pShop->shop_flags, SHOP_TRADER) )
        {
            if ( IS_SET(pShop->shop_flags, SHOP_PEDDLER) )
            snprintf( buf, MAX_STRING_LENGTH, " [##] Type %-24s", " " );
            else
            snprintf( buf, MAX_STRING_LENGTH, "^B%-35s^N", " " );
            send_to_actor( buf, ch );

            if ( IS_SET(pShop->shop_flags, SHOP_TRADER) )
            {
            snprintf( buf, MAX_STRING_LENGTH, "  [##] %15s", "Trading Item? 1=yes" );
            send_to_actor( buf, ch );
            }

            send_to_actor( "\n\r", ch );

            for ( iTrade = 0;
                  iTrade < UMAX(MAX_TRADE,
                     IS_SET(pShop->shop_flags,SHOP_TRADER) ? 10 : MAX_TRADE);
                  iTrade++ )
            {
                char buf2[MAX_STRING_LENGTH];
                bool fNothing = TRUE;

                if ( iTrade < MAX_TRADE
                  && IS_SET(pShop->shop_flags, SHOP_PEDDLER)
                  && pShop->buy_type[iTrade] != ITEM_NONE )
                {
                    snprintf( buf, MAX_STRING_LENGTH, " [^B%2d^N] Peddles ^B%s^N",  iTrade+1,
                             item_type_name( pShop->buy_type[iTrade] ) );
                    fNothing = FALSE;
                }
                else
                buf[0] = '\0';
                
                sprintf( buf2, "%-35s", buf );
                if ( !fNothing 
                  || (iTrade < 10 
                   && IS_SET(pShop->shop_flags, SHOP_TRADER)) )
                send_to_actor( buf2, ch );

                if ( iTrade < 10
                  && IS_SET(pShop->shop_flags, SHOP_TRADER) )
                {
                    snprintf( buf, MAX_STRING_LENGTH, "  [^N%2d^B] ^N%15s^B %3d",
                                  iTrade+1,
                                  name_good_code(iTrade), pShop->trades[iTrade] );
                    send_to_actor( buf, ch );
                    fNothing = FALSE;
                }

                if ( !fNothing ) send_to_actor( "\n\r", ch );
            }
        }


        snprintf( buf, MAX_STRING_LENGTH, "Str1 [no_such_item]:\n\r%s says, \"%s\"\n\r",
                      capitalize(victim->short_descr), pShop->no_such_item );
        send_to_actor( buf, ch );

        if ( IS_SET(pShop->shop_flags, SHOP_PEDDLER)
          || IS_SET(pShop->shop_flags, SHOP_TRADER) )
        {
            snprintf( buf, MAX_STRING_LENGTH, "Str2 [cmd_not_buy]:\n\r%s says, \"%s\"\n\r",
                           capitalize(victim->short_descr), pShop->cmd_not_buy );
            send_to_actor( buf, ch );
        }

        if ( IS_SET(pShop->shop_flags, SHOP_PEDDLER) )
        {
            snprintf( buf, MAX_STRING_LENGTH, "Str3 [list_header]:\n\r%s\n\r",
                     pShop->list_header );
            send_to_actor( buf, ch );
        }

        snprintf( buf, MAX_STRING_LENGTH, "Str4 [hours_excuse]:\n\r%s says, \"%s\"\n\r",
                      capitalize(victim->short_descr), pShop->hours_excuse );
        send_to_actor( buf, ch );

    }

    return;
}




/*
 * Syntax:  sindex [dbkey]
 */
void cmd_sindex( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    SCRIPT *script;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' )
    {
        send_to_actor( "Sindex what?\n\r", ch );
        return;
    }

    if ( ( script = get_script_index( atoi( arg ) ) ) == NULL )
    {
        send_to_actor( "Invalid script index VNUM.\n\r", ch );
        return;
    }

    snprintf( buf, MAX_STRING_LENGTH, "Vnum:   [^B%5d^N]  Name:   [^B%s^N]\n\r", script->dbkey, script->name );
    send_to_actor( buf, ch );

    if( !script->zone ) script->zone = get_dbkey_zone(script->dbkey);

    snprintf( buf, MAX_STRING_LENGTH, "zone:   [^B%5d^N] ^B%s^N\n\r",
             script->zone == NULL ? -1        : script->zone->dbkey,
             script->zone == NULL ? "No zone" : script->zone->name );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Type: ^B%s^N\n\r", show_script_type( script->type ) );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Script:\n\r%s", script->commands );
    send_to_actor( buf, ch );

    return;
}




void value_breakdown( int type, int v1, int v2, int v3, int v4, PLAYER *ch )
{
    char buf[MAX_STRING_LENGTH];

    switch ( type )
    {
          default: send_to_actor( "Values currently unused.\n\r", ch ); break;
  case ITEM_LIGHT:
        {
            if ( v2 >= 0 )
            {
                snprintf( buf, MAX_STRING_LENGTH, "^B%d^N (v1) of ^B%d^N (v2) hours of light.\n\r", v1, v2 );
                send_to_actor( buf, ch );
            }
            else
            send_to_actor( "Provides light infinitely.\n\r", ch );

            snprintf( buf, MAX_STRING_LENGTH, "Light bits (v4): [^B%s%s^N]\n\r",
                          IS_SET(v4, LIGHT_LIT) ? "lit " : "",
                          IS_SET(v4, LIGHT_FILLABLE) ? "fillable" : "" );
            send_to_actor( buf, ch );
        }
        break;
  case ITEM_POTION:
  case ITEM_FOUNTAIN:
  case ITEM_PILL:
  case ITEM_THROWN:
        {
            snprintf( buf, MAX_STRING_LENGTH, "Casts ^B%s^N (v2), ^B%s^N (v3) and ^B%s^N (v4) at level ^B%d^N (v1).\n\r",
                     v2 > 0 ? skill_name(v2) : "nothing",
                     v3 > 0 ? skill_name(v3) : "nothing",
                     v4 > 0 ? skill_name(v4) : "nothing",
                     v1 );
            send_to_actor( buf, ch );
        }
        break;
  case ITEM_WEAPON:
        {
       snprintf( buf, MAX_STRING_LENGTH, "Damages from ^B%d^N (v2) - %d (v3) (average ^B%d^N) of type ^B%s%s^N (v4).\n\r",
                     UMIN(v2,v3),
                     UMAX(v2,v3),
                     (UMIN(v2,v3)+UMAX(v2,v3))/2,
                     v4 >= 0 && v4 < MAX_ATTACK ? attack_table[v4].name : "INVALID",
                     v1 ? " and causes poison" : "" );
            send_to_actor( buf, ch );
        }
        break;
  case ITEM_RANGED_WEAPON: {
            snprintf( buf, MAX_STRING_LENGTH, "Damages from ^B%d^N (v2) - ^B%d^N (v3) (average ^B%d^N) using ammo dbkey ^B%d^N (v1).\n\r",
                     UMIN(v2,v3),
                     UMAX(v2,v3),
                     (UMIN(v2,v3)+UMAX(v2,v3))/2,
                     v1 );
            send_to_actor( buf, ch );
        } break;
  case ITEM_AMMO:
        {
            snprintf( buf, MAX_STRING_LENGTH, "Damage bonus ^B%d^N (v2) (quantity ^B%d^N (v1)) casts spell ^B%d^N (v3) level ^B%d^N (v4).\n\r",
                     v2,
                     v1, v3,
                     v4 );
            send_to_actor( buf, ch );
        }
        break;
  case ITEM_CLOTHING: 
        {
            snprintf( buf, MAX_STRING_LENGTH, "With an armor class bonus of ^B%d^N (v1).\n\r", v1 );
            STC( buf, ch );
            snprintf( buf, MAX_STRING_LENGTH, "%s", v2 == 1 ? "Possesses a hood (v2=1)\n\r" : "Set v2 to 1 for a hood\n\r");
            STC( buf, ch );
        } 
        break;
  case ITEM_ARMOR:
        {
            snprintf( buf, MAX_STRING_LENGTH, "Can be hit ^B%d^N/^B%d^N (v2/v3) times, with an armor class bonus of ^B%d^N (v1) .\n\r",
                     v2, v3, v1 );
            send_to_actor( buf, ch );
        }
        break;
  case ITEM_FURNITURE:
        {
            snprintf( buf, MAX_STRING_LENGTH, "Can maintain up to ^B%d^N (v1) halfstones (^B%d^N people)",
                     v1, v1/100 );
            send_to_actor( buf, ch );

            if ( get_prop_template( v3 ) != NULL )
            {
                snprintf( buf, MAX_STRING_LENGTH, ", requires key ^B%d^N (v3)", v3 );
                send_to_actor( buf, ch );
            }

            if ( get_scene( v4 ) != NULL )
            {
                snprintf( buf, MAX_STRING_LENGTH, ", and leads to %d (v4)", v4 );
                send_to_actor( buf, ch );
            }

            send_to_actor( ".\n\r", ch );

            snprintf( buf, MAX_STRING_LENGTH, "Furn bits (v2): [^B%s%s%s%s%s%s%s%s%s%s%s%s^N]\n\r",
                     IS_SET(v2,FURN_CLOSEABLE) ? "closeable " : "",
                     IS_SET(v2,FURN_PICKPROOF) ? "pickproof " : "",
                     IS_SET(v2,FURN_CLOSED)    ? "closed "    : "",
                     IS_SET(v2,FURN_LOCKED)    ? "locked "    : "",
                     IS_SET(v2,FURN_NOMOUNT)   ? "nomount "   : "",
                     IS_SET(v2,FURN_SIT)       ? "sit "       : "",
                     IS_SET(v2,FURN_SLEEP)     ? "sleep "     : "",
                     IS_SET(v2,FURN_EXIT)      ? "exit "      : "",
                     IS_SET(v2,FURN_PUT)       ? "put "       : "",
                     IS_SET(v2,FURN_MOVE)      ? "move"       : "",
                     IS_SET(v2,FURN_NOSHOW)    ? "noshow "    : "",
                     IS_SET(v2,FURN_HOME)      ? "home"       : "" );
 
            send_to_actor( buf, ch );
        }
        break;
  case ITEM_CONTAINER:
        {
            snprintf( buf, MAX_STRING_LENGTH, "Holds up to ^B%d^N (v1) halfstones of weight", v1 );
            send_to_actor( buf, ch );

            if ( get_prop_template( v3 ) != NULL )
            {
                snprintf( buf, MAX_STRING_LENGTH, ", and requires key %d (v3)", v3 );
                send_to_actor( buf, ch );
            }

            send_to_actor( ".\n\r", ch );

            snprintf( buf, MAX_STRING_LENGTH, "Container bits (v2): [^B%s%s%s%s^N]\n\r",
                     IS_SET(v2,CONT_CLOSEABLE) ? "closeable " : "",
                     IS_SET(v2,CONT_PICKPROOF) ? "pickproof " : "",
                     IS_SET(v2,CONT_CLOSED)    ? "closed "    : "",
                     IS_SET(v2,CONT_LOCKED)    ? "locked"    : "" );
            send_to_actor( buf, ch );
        }
        break;
  case ITEM_DRINK_CON:
        {
            snprintf( buf, MAX_STRING_LENGTH, "Contains ^B%d^N/^B%d^N (v1/v2) (about ^B%d^N-^B%d^M drinks (halfswils)) of (v3) ^B%s%s^N.\n\r",
                     v1, v2, v1/10, v1/3,
                     v3 >= 0 && v3 < LIQ_MAX ? liq_table[v3].liq_name : "INVALID LIQUID",
                     IS_SET(v4,DRINK_POISON) ? " and is laced with poison (v4)" : "" );
            send_to_actor( buf, ch );
            if ( IS_SET(v4,DRINK_TAVERN) )
            send_to_actor( "This drink container disappears when empty. (v4)\n\r", ch );
        }
        break;
  case ITEM_FOOD:
        {
            snprintf( buf, MAX_STRING_LENGTH, "Food lasts for ^B%d^N (v1) minutes (^B%d^N game hours).\n\r",
                     v1, v1 / 5 );
            send_to_actor( buf, ch );
        }
        break;
  case ITEM_MONEY:
        {
            snprintf( buf, MAX_STRING_LENGTH, "Worth ^B%d^N ^B%s^N coins. (v1)\n\r",
                     v1, v2 >= 0 && v2 < MAX_COIN ? coin_table[v2].long_name : "invalid" );
            send_to_actor( buf, ch );
        }
        break;
  case ITEM_GEM:
        {
            snprintf( buf, MAX_STRING_LENGTH, "Provides ^B%d^N/^B%d^N (v2/v3) mana for ^B%s%s%s%s^M (v1) elements.",
                     v2, v3,
                     IS_SET(v1,MA) || IS_SET(v1,ME) ? "earth " : "",
                     IS_SET(v1,MA) || IS_SET(v1,MF) ? "fire "  : "",
                     IS_SET(v1,MA) || IS_SET(v1,MW) ? "water " : "",
                     IS_SET(v1,MA) || IS_SET(v1,MA) ? "air"   : "" );
            send_to_actor( buf, ch );
        }
        break;
  case ITEM_VEHICLE:
        {
            snprintf( buf, MAX_STRING_LENGTH, "Provides access to ^B%s^N, ^B%s^N, ^B%s^M and ^B%s^M. (v1-4)\n\r",
                     move_name( v1 ),
                     move_name( v2 ),
                     move_name( v3 ),
                     move_name( v4 ) );
            send_to_actor( buf, ch );
        }
        break;
  case ITEM_TOOL:
        {
            snprintf( buf, MAX_STRING_LENGTH, "Has ^B%d^N of ^B%d^N uses remaining. (v2/v3)\n\r",
                     v2, v3 );
            send_to_actor( buf, ch );

            snprintf( buf, MAX_STRING_LENGTH, "Tool flags (v1): [^B%s%s%s%s%s^N]\n\r",
                     IS_SET(v1,TOOL_TINDERBOX) ? "tinderbox " : "",
                     IS_SET(v1,TOOL_LOCKPICK)  ? "lockpick "  : "",
                     IS_SET(v1,TOOL_BOUNTY)    ? "bounty"    : "",
                     IS_SET(v1,TOOL_BANDAGES)  ? "bandages" : "",
                     IS_SET(v1,TOOL_REPAIR)    ? "repair" : "" );
            send_to_actor( buf, ch );

            if ( IS_SET(v1,TOOL_BANDAGES) ) {
            snprintf( buf, MAX_STRING_LENGTH, "Heals for ^B%d^N (v4) worth of damage per use.\n\r", v4 );
            send_to_actor( buf, ch );
            }

            if ( IS_SET(v1,TOOL_REPAIR) ) {
            snprintf( buf, MAX_STRING_LENGTH, "Repairs at ^B%d%%^N (v4) efficiency.\n\r", v4 );
            send_to_actor( buf, ch );
            }
        }
        break;
  case ITEM_BOARD:
  case ITEM_COMPONENT:
  case ITEM_GOODS:
        {
            snprintf( buf, MAX_STRING_LENGTH, "Has ^B%s^N index of ^B%d^N (v1).\n\r", item_type_name(type), v1 );
            send_to_actor( buf, ch );
        }
        break;
    }

    return;
}





/*
 * Syntax:  pindex [dbkey]
 */
void cmd_pindex( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    BONUS *paf;
    PROP_TEMPLATE *prop;
    INSTANCE *pTrig;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
    send_to_actor( "Prop: index what?\n\r", ch );
        return;
    }

    if ( ( prop = get_prop_template( atoi( arg ) ) ) == NULL )
    {
    send_to_actor( "Invalid VNUM reference.\n\r", ch );
        return;
    }

    snprintf( buf, MAX_STRING_LENGTH, "Name:   [^B%s^N]\n\rzone:   [^B%5d^N] %s\n\r",
    prop->name,
    prop->zone == NULL ? -1        : prop->zone->dbkey,
    prop->zone == NULL ? "No zone" : prop->zone->name );
    send_to_actor( buf, ch );


    snprintf( buf, MAX_STRING_LENGTH, "Vnum:   [^B%5d^N]     Type:  [^B%s^N]\n\r",
    prop->dbkey,
    item_type_name( prop->item_type ) );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Short:  [^B%s^N]\n\r", prop->short_descr );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Plural: [^B%s^N]\n\r", pluralize( prop->short_descr ) );
    send_to_actor( buf, ch );

    {
        char buf2[MAX_STRING_LENGTH];
        snprintf( buf, MAX_STRING_LENGTH, "Long:\n\r%s", prop->description );
        send_to_actor( buf, ch );
        
        snprintf( buf2, MSL, "%s %s are here.\n\r", 
          numberize( 2 ), 
          pluralize( prop->short_descr ) );
        snprintf( buf, MAX_STRING_LENGTH, "Plural:\n\r%s",
             prop->description_plural == NULL
          || prop->description_plural[0] == '\0' ? capitalize(buf2)
                                                : prop->description_plural );
        send_to_actor( buf, ch );
    }
  
    snprintf( buf, MAX_STRING_LENGTH, "Action:\n\r%s", prop->action_descr );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Description:\n\r%s", prop->real_description );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Wear flags:  [^B%s^N]\n\rExtra flags: [^B%s^N]\n\r",
    wear_bit_name( prop->wear_flags ),
    extra_bit_name( prop->extra_flags ) );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Weight: [^B%5d^N]  Cost:     [^B%5d^N]  Timer: [^B%5d^N]  Level: [^B%5d^N]\n\r",
             prop->weight, prop->cost, prop->timer, prop->level );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Size:   [^B%5d^N]  Material: [^B%5d^N]\n\r",
             prop->size, prop->material );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Values: [^B%5d^N] [^B%5d^N] [^B%5d^N] [^B%5d^N]\n\r",
    prop->value[0],    prop->value[1],    prop->value[2],    prop->value[3] );
    send_to_actor( buf, ch );

    value_breakdown( prop->item_type,
                     prop->value[0],
                     prop->value[1],
                     prop->value[2],
                     prop->value[3],
                     ch );

    if ( prop->extra_descr != NULL )
    {
        EXTRA_DESCR *ed;

        send_to_actor( "ED keywords: [", ch );

        for ( ed = prop->extra_descr; ed != NULL; ed = ed->next )
        {
        char arg2[MAX_STRING_LENGTH];
        
        one_argument( ed->keyword, arg2 );
        send_to_actor( arg2, ch );
        if ( ed->next != NULL ) send_to_actor( " ", ch );
        }
  
        send_to_actor( "]\n\r", ch );
    }

    for ( paf = prop->bonus; paf != NULL; paf = paf->next )
    {
       snprintf( buf, MAX_STRING_LENGTH, "Bonuses ^B%s^N by ^B%d^N.\n\r", bonus_loc_name( paf->location ),
                                              paf->modifier );
       send_to_actor( buf, ch );
    }

    for ( pTrig = prop->instances; pTrig != NULL; pTrig = pTrig->next )
    {
        snprintf( buf, MAX_STRING_LENGTH, "[^B%5d^N] ^B%s^N\n\r", pTrig->script->dbkey, pTrig->script->name );
        send_to_actor( buf, ch );
    }

    return;
}


void display_spawns( PLAYER *ch )
{
    char final[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    SCENE *pScene = ch->in_scene;
    SPAWN *pSpawn;
    ACTOR_TEMPLATE        *pActor = NULL;
    ACTOR_TEMPLATE     *LastMob = NULL;
    PROP_TEMPLATE         *prop = NULL;
    PROP_TEMPLATE     *LastObj = NULL;
    ACTOR_TEMPLATE *pActorIndex;
    PROP_TEMPLATE *pPropIndex;
    bool last;
    int iSpawn = 0;
    int olevel = 2;

    if ( pScene == NULL ) return;

    pActor    = NULL;
    last    = TRUE;
    final[0]  = '\0';
    

    for ( pSpawn = pScene->spawn_first; pSpawn != NULL; pSpawn = pSpawn->next )
    {

    send_to_actor( final, ch );
    final[0] = '\0';
    sprintf( final, "%2d- ", ++iSpawn );

        switch ( pSpawn->command )
        {
        default:
        strcat( final, "Invalid Spawn Command\n\r" );
            break;

    case 'M':

        if ( ( pActorIndex = get_actor_template( pSpawn->rs_dbkey ) ) == NULL )
            {
            snprintf( buf, MAX_STRING_LENGTH, "Load Actor - Bad Vnum %d\n\r", pSpawn->rs_dbkey );
            strcat( final, buf );
            continue;
            }

        pActor = pActorIndex;
        snprintf( buf, MAX_STRING_LENGTH, "Loads %s (%d) in scene (max %d, %d%% chance) %d times\n\r",
                       pActor->short_descr, pSpawn->rs_dbkey,
                       pSpawn->loc, pSpawn->percent, pSpawn->num );
        strcat( final, buf );

        LastObj = NULL;
        LastMob = pActor;
        olevel  = 1;
            break;

        case 'O':
        if ( ( pPropIndex = get_prop_template( pSpawn->rs_dbkey ) ) == NULL )
            {
            snprintf( buf, MAX_STRING_LENGTH, "Load Object - Bad Vnum %d\n\r", pSpawn->rs_dbkey );
            strcat( final, buf );
            continue;
            }

        prop       = pPropIndex;

        if ( pSpawn->loc == SPAWN_LOC_INSIDE && LastObj != NULL )
        {
            snprintf( buf, MAX_STRING_LENGTH, " Loads %s (%d) inside %s", prop->short_descr,
                          pSpawn->rs_dbkey,
                          LastObj ? LastObj->short_descr : "!NO OBJ!" );
            strcat( final, buf );
        }
   else if ( pSpawn->loc == SPAWN_LOC_ONTOP && LastObj != NULL )
        {
            snprintf( buf, MAX_STRING_LENGTH, " Loads %s (%d) on top of %s",
                          prop->short_descr, pSpawn->rs_dbkey,
                          LastObj ? LastObj->short_descr : "!NO OBJ!" );
            strcat( final, buf );
        }
   else if ( pSpawn->loc == SPAWN_LOC_INSCENE )
        {
            snprintf( buf, MAX_STRING_LENGTH, "Loads %s (%d) in scene",
                          prop->short_descr, pSpawn->rs_dbkey );
            strcat( final, buf );
            LastObj = prop;
        }
   else if ( LastMob != NULL )
        {
            snprintf( buf, MAX_STRING_LENGTH, " Loads %s (%d) on %s of %s",
                          prop->short_descr, pSpawn->rs_dbkey,
                          wear_loc_name( pSpawn->loc ),
                          LastMob ? LastMob->short_descr : "!NO ACTOR!" );
            strcat( final, buf );
        }
        else strcat( final, "Incorrect Assignment of Object\n\r" );

        if ( !strstr( final, "Incorrect" ) && !strstr( final, "Drop" ) )
        {
            snprintf( buf, MAX_STRING_LENGTH, " %d%% chance %d times\n\r", pSpawn->percent,
                                                        pSpawn->num );
            strcat( final, buf );
        }

        if ( LastObj == NULL || prop->item_type == ITEM_CONTAINER )
                LastObj = prop;
        break;

        case 'C':
            if ( pSpawn->rs_dbkey >= MAX_COMPONENTS || pSpawn->rs_dbkey<0 )
            {
            snprintf( buf, MAX_STRING_LENGTH, "Load Component - Bad Component %d\n\r", pSpawn->rs_dbkey );
            strcat( final, buf );
            continue;
            }

        if ( pSpawn->loc == SPAWN_LOC_INSIDE && LastObj != NULL )
        {
            snprintf( buf, MAX_STRING_LENGTH, " Component (%d) inside %s", 
                          pSpawn->rs_dbkey,
                          LastObj ? LastObj->short_descr : "!NO OBJ!" );
            strcat( final, buf );
        }
   else if ( pSpawn->loc == SPAWN_LOC_ONTOP && LastObj != NULL )
        {
            snprintf( buf, MAX_STRING_LENGTH, " Component (%d) on top of %s",
                          pSpawn->rs_dbkey,
                          LastObj ? LastObj->short_descr : "!NO OBJ!" );
            strcat( final, buf );
        }
   else if ( pSpawn->loc == SPAWN_LOC_INSCENE )
        {
            snprintf( buf, MAX_STRING_LENGTH, " Component (%d) in scene",
                          pSpawn->rs_dbkey );
            strcat( final, buf );
            LastObj = prop;
        }
   else if ( LastMob != NULL )
        {
            snprintf( buf, MAX_STRING_LENGTH, " Component (%d) on %s of %s",
                          pSpawn->rs_dbkey,
                          wear_loc_name( pSpawn->loc ),
                          LastMob ? LastMob->short_descr : "!NO ACTOR!" );
            strcat( final, buf );
        }
        else strcat( final, "Incorrect Assignment of Object\n\r" );

        if ( !strstr( final, "Incorrect" ) && !strstr( final, "Drop" ) )
        {
            snprintf( buf, MAX_STRING_LENGTH, " %d%% chance %d times\n\r", pSpawn->percent,
                                                        pSpawn->num );
            strcat( final, buf );
        }

        break;

        case 'G': 
            if ( pSpawn->rs_dbkey >= MAX_GOODS || pSpawn->rs_dbkey<0 )
            {
            snprintf( buf, MAX_STRING_LENGTH, "Load Goods - Bad Good %d\n\r", pSpawn->rs_dbkey );
            strcat( final, buf );
            continue;
            }

        if ( pSpawn->loc == SPAWN_LOC_INSIDE && LastObj != NULL )
        {
            snprintf( buf, MAX_STRING_LENGTH, " Goods (%d) inside %s", 
                          pSpawn->rs_dbkey,
                          LastObj ? LastObj->short_descr : "!NO OBJ!" );
            strcat( final, buf );
        }
   else if ( pSpawn->loc == SPAWN_LOC_ONTOP && LastObj != NULL )
        {
            snprintf( buf, MAX_STRING_LENGTH, " Goods (%d) on top of %s",
                          pSpawn->rs_dbkey,
                          LastObj ? LastObj->short_descr : "!NO OBJ!" );
            strcat( final, buf );
        }
   else if ( pSpawn->loc == SPAWN_LOC_INSCENE )
        {
            snprintf( buf, MAX_STRING_LENGTH, "Goods (%d) in scene", pSpawn->rs_dbkey );
            strcat( final, buf );
            LastObj = prop;
        }
   else if ( LastMob != NULL )
        {
            snprintf( buf, MAX_STRING_LENGTH, " Loads (%d) on %s of %s",
                          pSpawn->rs_dbkey,
                          wear_loc_name( pSpawn->loc ),
                          LastMob ? LastMob->short_descr : "!NO ACTOR!" );
            strcat( final, buf );
        }
        else strcat( final, "Incorrect Assignment of Object\n\r" );

        if ( !strstr( final, "Incorrect" ) && !strstr( final, "Drop" ) )
        {
            snprintf( buf, MAX_STRING_LENGTH, " %d%% chance %d times\n\r", pSpawn->percent,
                                                        pSpawn->num );
            strcat( final, buf );
        }

        break;
       }
    }
    send_to_actor( final, ch );
    final[0] = '\0';

    return;
}



void add_spawn( SCENE *scene, SPAWN *pSpawn, int i )
{
    SPAWN *reset;
    int iSpawn = 0;

    SET_BIT( scene->zone->zone_flags, ZONE_CHANGED );

    if ( scene->spawn_first == NULL )
    {
        scene->spawn_first = pSpawn;
        scene->spawn_last  = pSpawn;
        pSpawn->next      = NULL;
        return;
    }

    i--;

    if ( i == 0 )
    {
        pSpawn->next = scene->spawn_first;
        scene->spawn_first = pSpawn;
        return;
    }

    for ( reset = scene->spawn_first; reset->next != NULL; reset = reset->next )
    {
        if ( ++iSpawn == i ) break;
    }

    pSpawn->next = reset->next;
    reset->next  = pSpawn;
    if ( pSpawn->next == NULL ) scene->spawn_last = pSpawn;
    return;
}
 


/*
 * Syntax:  resets [num] prop [dbkey] [location] [chance] [times]
 *          resets [num] actor [dbkey] [location] [chance] [times]
 *          resets [num] delete
 *          resets here
 *          cue here
 */
void cmd_spawns( PLAYER *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    char arg5[MAX_INPUT_LENGTH];
    char arg6[MAX_INPUT_LENGTH];
    SPAWN *pSpawn;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    argument = one_argument( argument, arg4 );
    argument = one_argument( argument, arg5 );
    argument = one_argument( argument, arg6 );

    if ( arg1[0] == '\0' )
    {
        if ( ch->in_scene->spawn_first != NULL )
        {
        send_to_actor( "Cues:\n\r", ch );
        display_spawns( ch );
        }
        else
        send_to_actor( "No cues in this scene.\n\r", ch );
    }

    if ( !str_cmp( arg1, "here" ) || !str_cmp( arg1, "scene" )  ) {
         spawn_scene( ch->in_scene );
         send_to_actor( "Scene cued.\n\r", ch );
  	return;
    }

    if ( !IS_BUILDER( ch, ch->in_scene->zone ) )
    {
        send_to_actor( "Warning: Invalid security for editing this scene.\n\r",
                      ch );
        return;
    }

    if ( is_number( arg1 ) )
    {
        SCENE *pScene = ch->in_scene;

        if ( !str_cmp( arg2, "delete" ) )
        {
        int insert_loc = atoi( arg1 );

        if ( ch->in_scene->spawn_first == NULL )
        {
            send_to_actor( "No resets in this scene.\n\r", ch );
            return;
        }

        if ( insert_loc-1 <= 0 )
        {
            pSpawn = pScene->spawn_first;
            pScene->spawn_first = pScene->spawn_first->next;
            if ( pScene->spawn_first == NULL )
                 pScene->spawn_last = NULL;
        }
        else
        {
            int iSpawn = 0;
            SPAWN *prev = NULL;

            for ( pSpawn = pScene->spawn_first; pSpawn != NULL; pSpawn = pSpawn->next )
            {
                if ( ++iSpawn == insert_loc )   break;
                prev = pSpawn;
            }

            if ( pSpawn == NULL )
            {
                send_to_actor( "Cue not defined.\n\r", ch );
                return;
            }

            if ( prev != NULL )  prev->next = prev->next->next;
                            else pScene->spawn_first = pScene->spawn_first->next;

            for ( pScene->spawn_last = pScene->spawn_first;
                  pScene->spawn_last->next != NULL;
                  pScene->spawn_last = pScene->spawn_last->next );
        }

        SET_BIT( pScene->zone->zone_flags, ZONE_CHANGED );
        free_spawn( pSpawn );
        send_to_actor( "Cue scratched.\n\r", ch );
        }
        else
        if ( (!str_prefix( arg2, "actor" ) && is_number( arg3 ))
          || (!str_prefix( arg2, "prop" ) && is_number( arg3 )) 
          || (!str_prefix( arg2, "component" ) && is_number( arg3 )) 
          || (!str_prefix( arg2, "good" ) && is_number( arg3 )) )
        {
            pSpawn = new_spawn();
            if ( !str_prefix( arg2, "actor" ) )
            {
            pSpawn->command = 'M';
            pSpawn->rs_dbkey = atoi( arg3 );
            pSpawn->loc     = is_number( arg4 ) ? atoi( arg4 ) : 1;
            pSpawn->percent = is_number( arg5 ) ? atoi( arg5 ) : 75;
            }
            else
            if ( !str_prefix( arg2, "prop" ) )
            {
            pSpawn->command = 'O';
            pSpawn->rs_dbkey = atoi( arg3 );
            if ( !str_cmp( arg4, "inside" ) || !str_cmp( arg4, "in" ) )
                                            pSpawn->loc     = SPAWN_LOC_INSIDE;
       else if ( !str_cmp( arg4, "on" ) )   pSpawn->loc     = SPAWN_LOC_ONTOP;
       else if ( !str_cmp( arg4, "scene" ) 
              || !str_cmp( arg4, "room" )
              || !str_cmp( arg4, "here" ) ) pSpawn->loc     = SPAWN_LOC_INSCENE;
       else pSpawn->loc     = wear_name_loc( arg4 );
            pSpawn->percent = is_number( arg5 ) ? atoi( arg5 ) : 75;
            }
            else
            if ( !str_prefix( arg2, "good" ) )
            {
            pSpawn->command = 'G';
            pSpawn->rs_dbkey = atoi( arg3 );
            if ( !str_cmp( arg4, "inside" ) || !str_cmp( arg4, "in" ) )
                                            pSpawn->loc     = SPAWN_LOC_INSIDE;
       else if ( !str_cmp( arg4, "on" ) )   pSpawn->loc     = SPAWN_LOC_ONTOP;
       else if ( !str_cmp( arg4, "scene") 
              || !str_cmp( arg4, "here") 
              || !str_cmp( arg4, "room") ) pSpawn->loc     = SPAWN_LOC_INSCENE;
       else pSpawn->loc     = wear_name_loc( arg4 );
            pSpawn->percent = is_number( arg5 ) ? atoi( arg5 ) : 75;
            }
            else
            if ( !str_prefix( arg2, "component" ) )
            {
            pSpawn->command = 'C';
            pSpawn->rs_dbkey = atoi( arg3 );
            if ( !str_cmp( arg4, "inside" ) || !str_cmp( arg4, "in" ) )
                                            pSpawn->loc     = SPAWN_LOC_INSIDE;
       else if ( !str_cmp( arg4, "on" ) )   pSpawn->loc     = SPAWN_LOC_ONTOP;
       else if ( !str_cmp( arg4, "scene" )
             || !str_cmp( arg4, "room" )
             || !str_cmp( arg4, "here" ) ) pSpawn->loc     = SPAWN_LOC_INSCENE;
       else pSpawn->loc     = wear_name_loc( arg4 );
            pSpawn->percent = is_number( arg5 ) ? atoi( arg5 ) : 75;
            }

            pSpawn->num     = !is_number(arg6) ? 1 : atoi( arg6 );

            add_spawn( ch->in_scene, pSpawn, atoi( arg1 ) );
            send_to_actor( "Cue added.\n\r", ch );
        }
        else
        {
        send_to_actor( "Syntax: CUE <number> GOOD <num> <location> [chance] [num to load]\n\r", ch );
        send_to_actor( "        CUE <number> GOOD <num> SCENE [chance] [num to load]\n\r", ch );
        send_to_actor( "        CUE <number> GOOD <num> IN|ON [chance] [num to load]\n\r", ch );
        send_to_actor( "Syntax: CUE <number> COMPONENT <num> <location> [chance] [num to load]\n\r", ch );
        send_to_actor( "        CUE <number> COMPONENT <num> SCENE [chance] [num to load]\n\r", ch );
        send_to_actor( "        CUE <number> COMPONENT <num> IN|ON [chance] [num to load]\n\r", ch );
        send_to_actor( "Syntax: CUE <number> PROP <dbkey> <location> [chance] [num to load]\n\r", ch );
        send_to_actor( "        CUE <number> PROP <dbkey> SCENE [chance] [num to load]\n\r", ch );
        send_to_actor( "        CUE <number> PROP <dbkey> IN|ON [chance] [num to load]\n\r", ch );
        send_to_actor( "        CUE <number> ACTOR <dbkey> [<max #> <chance> <num to load>]\n\r", ch );
        send_to_actor( "        CUE <number> DELETE\n\r", ch );
        send_to_actor( "        CUE HERE\n\r", ch );
        }
    }

    return;
}




/*
 * Syntax:  astat [num]
 */
void cmd_astat( PLAYER *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char buf  [MAX_STRING_LENGTH];
    ZONE *pZone;

    smash_tilde( argument );
    strcpy( arg1, argument );

    if ( is_number( arg1 ) )
        pZone = get_zone( atoi( arg1 ) );
    else
        pZone = ch->in_scene->zone;
        
    if (!pZone)
      pZone = ch->in_scene->zone;

    snprintf( buf, MAX_STRING_LENGTH, "Name:     [%5d] %s\n\r",
             pZone->dbkey, pZone->name );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "File:     [%s]\n\r",
             pZone->filename );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Age:      [%5d]\n\rPlayers:  [%5d]\n\r",
             pZone->age, pZone->nplayer );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Security: [%5d]\n\rBuilders: [%s]\n\r",
             pZone->security, pZone->builders );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Vnums:    [%d-%d]\n\r", pZone->ldbkey, pZone->udbkey );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Repop:\n\r%s", pZone->repop );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "Flags:    [%s]\n\r", zone_bit_name( pZone->zone_flags ) );
    send_to_actor( buf, ch );

    return;
}




void save_zone_list( void )
{
   FILE *fp, *f_from, *f_to;
   ZONE *pZone;
   char buf[MSL]; 
   char nfn[MIL];
   int cidx;

   if ( ( fp = fopen( ZONE_LIST, "w" ) ) == NULL ) {
      bug( "Save_zone_list: fopen", 0 );
      perror( "zone.lst" );
   } else {

      fprintf( fp, MUD_FILE "\n" );

      for ( cidx=0; cidx < MAX_HELP_CLASS; cidx++ )
         fprintf(fp, "%s\n", help_class_table[cidx].filename);

      fprintf( fp, SOCIALS_FILE "\n" );

      for( pZone = zone_first; pZone != NULL; pZone = pZone->next )
         if ( !str_infix(pZone->filename, ".db") ) 
  {
         fprintf( fp, "%s\n", pZone->filename );

// Create backups
     sprintf( nfn, "%s.bak", pZone->filename );
            /* open the source and the target files. */
    f_from = fopen(pZone->filename, "r");
    if (!f_from) {
	fprintf(stderr, "Cannot open source file: ");
	perror("");
	exit(1);
    }
    f_to = fopen(nfn, "w+");
    if (!f_from) {
	fprintf(stderr, "Cannot open target file: ");
	perror("");
	exit(1);
    }

    /* copy source file to target file, line by line. */
    while (fgets(buf, MSL-2, f_from)) {
	if (fputs(buf, f_to) == EOF) {  /* error writing data */
	    fprintf(stderr, "Error writing to target file: ");
	    perror("");
	    exit(1);
	}
    }
    if (!feof(f_from)) { /* fgets failed _not_ due to encountering EOF */
        fprintf(stderr, "Error reading from source file: ");
        perror("");
        exit(1);
    }

    /* close source and target file streams. */
    if (fclose(f_from) == EOF) {
	fprintf(stderr, "Error when closing source file: ");
	perror("");
    }
    if (fclose(f_to) == EOF) {
	fprintf(stderr, "Error when closing target file: ");
	perror("");
    }

  }


      for( pZone = zone_first; pZone != NULL; pZone = pZone->next )
         if ( str_infix(pZone->filename, ".db") ) 
  {
         fprintf( fp, "%s\n", pZone->filename );

// Create backups
     sprintf( nfn, "%s.bak", pZone->filename );
            /* open the source and the target files. */
    f_from = fopen(pZone->filename, "r");
    if (!f_from) {
	fprintf(stderr, "Cannot open source file: ");
	perror("");
	exit(1);
    }
    f_to = fopen(nfn, "w+");
    if (!f_from) {
	fprintf(stderr, "Cannot open target file: ");
	perror("");
	exit(1);
    }

    /* copy source file to target file, line by line. */
    while (fgets(buf, MSL-2, f_from)) {
	if (fputs(buf, f_to) == EOF) {  /* error writing data */
	    fprintf(stderr, "Error writing to target file: ");
	    perror("");
	    exit(1);
	}
    }
    if (!feof(f_from)) { /* fgets failed _not_ due to encountering EOF */
        fprintf(stderr, "Error reading from source file: ");
        perror("");
        exit(1);
    }

    /* close source and target file streams. */
    if (fclose(f_from) == EOF) {
	fprintf(stderr, "Error when closing source file: ");
	perror("");
    }
    if (fclose(f_to) == EOF) {
	fprintf(stderr, "Error when closing target file: ");
	perror("");
    }

  }

      fprintf( fp, SCENES_FILE "\n" );

      fprintf( fp, "$\n" );
      fclose( fp );
   }
   return;
}


                    

void save_string_to_file( char *fname, char *content )
{
   FILE *fp;

   if ( ( fp = fopen( fname, "w" ) ) == NULL ) {
      printf( "Save_string_to_file: fopen %s\n",  fname );      
      perror( fname );
   } else {
      fprintf( fp, "%s", content );
      fprintf( fp, "\n" );
      fclose( fp );
   }
   return;
}


/*
 * Saves skills, spells and calendar data as well as MUD flags and globals.
 * Helps keep the world persistent.
 */
void save_config( void )
{
    FILE *fp;
    TERRAIN *pTerrain;
    SKILL *pSkill;
    extern BAN *ban_list;
    BAN *pban;
    int tdbkey;

    if ( ( fp = fopen( MUD_FILE, "w" ) ) == NULL )
    {
    bug( "Save_config: fopen", 0 );
    perror( MUD_FILE );
    }
    else
    {
        fprintf( fp, "#CONFIG\n" );
        fprintf( fp, "Date %d %d %d %d\n", weather.hour, weather.day,
                     weather.month, weather.year );
        fprintf( fp, "Moon %d %d\n",
                     weather.moon_phase, weather.next_phase );

        for ( pban = ban_list; pban != NULL; pban = pban->next )
        {
            fprintf( fp, "Ban %s~\n", pban->name );
        }

        if ( wizlock )
        fprintf( fp, "WizLocked\n" );
        if ( newlock )
        fprintf( fp, "NewLocked\n" );

        fprintf( fp, "End\n\n" );

        /*
         * Output terrain information.
         */
        for ( pTerrain = terrain_list;  pTerrain != NULL;  pTerrain = pTerrain->next )
        {
             fprintf( fp, "#TERRAIN %d\n", pTerrain->dbkey );
             fprintf( fp, "N %s~\n",  pTerrain->name   );
             fprintf( fp, "Dsp %s~\n", fix_string( pTerrain->spring ) );
             fprintf( fp, "Dwi %s~\n", fix_string( pTerrain->winter ) );
             fprintf( fp, "Dsu %s~\n", fix_string( pTerrain->summer ) );
             fprintf( fp, "Dfa %s~\n", fix_string( pTerrain->fall   ) );
             fprintf( fp, "C %c\n",   pTerrain->map_char  );
             fprintf( fp, "S %d\nEnd\n", pTerrain->move );
        }

        /*
         * Output master skill list.
         */
        for( tdbkey=0; tdbkey <= top_dbkey_skill; tdbkey++ ) {
        {
             pSkill = get_skill_index( tdbkey );
             if ( !pSkill ) continue;
             fprintf( fp, "#SKILL %d\n", pSkill->dbkey );

             fprintf( fp, "N %s~\n",     pSkill->name         );
             fprintf( fp, "Lvl %d\n",    pSkill->skill_level  );
             fprintf( fp, "Target %d\n", pSkill->target       );
             fprintf( fp, "Delay %d\n",  pSkill->delay        );
             fprintf( fp, "Group %d\n",  pSkill->group_code   );
             fprintf( fp, "ReqPer %d\n", pSkill->required_percent );

             fprintf( fp, "Str %d\n", pSkill->req_str );
             fprintf( fp, "Int %d\n", pSkill->req_int );
             fprintf( fp, "Wis %d\n", pSkill->req_wis );
             fprintf( fp, "Dex %d\n", pSkill->req_dex );
             fprintf( fp, "Con %d\n", pSkill->req_con );

             if ( pSkill->msg_off )
             fprintf( fp, "MsgOff %s~\n",  pSkill->msg_off    );
             fprintf( fp, "MaxPrac %d\n", pSkill->max_prac   );
             fprintf( fp, "MaxLearn %d\n", pSkill->max_learn );
             fprintf( fp, "Rate %d\n", pSkill->learn_rate    );
             fprintf( fp, "Cost %d\n", pSkill->cost          );
             fprintf( fp, "Slot %d\n", pSkill->slot          );

             fprintf( fp, "End\n\n\n" );
        }
        }

        fprintf( fp, "\n#$\n" );
        fclose( fp );
    }

    return;
}




void save_contents( void )
{
    FILE *fp;

    if ( ( fp = fopen( SCENES_FILE, "w" ) ) == NULL )
    {
    bug( "Save_contents: fopen", 0 );
    perror( SCENES_FILE );
    }
    else
    {
        int iHash,tdbkey;
        SPELL *pSpell;
        SCENE *scene;


        /*
         * Output master spell list.
         * Must be done here because scene file is at the end
         * of the db.
         */
        for( tdbkey=0; tdbkey <= top_dbkey_spell; tdbkey++ ) {
        {
             INSTANCE *pInstance;
             pSpell = get_spell_index( tdbkey );
             if ( !pSpell ) continue;
             fprintf( fp, "#SPELL %d\n", pSpell->dbkey );
             fprintf( fp, "N %s~\n",  pSpell->name   );
             fprintf( fp, "Lvl %d\n", pSpell->level );
             fprintf( fp, "Target %d\n", pSpell->target );
             fprintf( fp, "Delay %d\n", pSpell->delay );
             fprintf( fp, "Mana %d\n", pSpell->mana_cost );
             fprintf( fp, "Type %d\n", pSpell->mana_type );
             fprintf( fp, "Group %d\n", pSpell->group_code );

             for ( pInstance = pSpell->instances;  pInstance != NULL;  
                      pInstance = pInstance->next )
              if ( get_script_index( pInstance->script->dbkey ) )
                    fprintf( fp, "Sc %d\n", pInstance->script->dbkey );

             fprintf( fp, "End\n" );
        }
        }


        fprintf( fp, "#CONTENTS\n" );

        for ( iHash = 0;  iHash < MAX_KEY_HASH;  iHash++ )
        {
        for ( scene = scene_hash[iHash]; scene != NULL;  scene = scene->next )
        {
            if ( !IS_SET(scene->scene_flags, SCENE_SAVE) )
            continue;

            fprintf( fp, "Scene %d\n", scene->dbkey );

            if ( IS_SET(scene->scene_flags, SCENE_WAGON) )
            {
                PROP_TEMPLATE *pPropIndex;

                pPropIndex = get_prop_template( scene->wagon );
                if ( pPropIndex != NULL )
                {
                    PROP *pProp;

                    for ( pProp = prop_list; pProp != NULL; pProp = pProp->next )
                    {
                        if ( pProp->pIndexData == pPropIndex
                          && pProp->in_scene != NULL )
                             break;
                    }

                    if ( pProp != NULL )
                    fprintf( fp, "Wagon %d\n", pProp->in_scene->dbkey );
                }
            }

            if ( scene->contents != NULL )
            fwrite_prop( scene->contents, fp, 0 );
        }
        }

        fprintf( fp, "Stop\n\n" );
        fprintf( fp, "#$\n" );
        fclose( fp );
    }


#if defined(NEVER)
    /*
     * To avoid prop duplication.
     */
    {
        PLAYER *ch;
        
        for ( ch = actor_list;  ch != NULL;  ch = ch->next )
            cmd_save( ch, "internal" );
    }         
#endif
   
    return;
}



void save_actors( FILE *fp, ZONE *pZone )
{
    int iHash;
    ACTOR_TEMPLATE *pActorIndex;
    INSTANCE *script;
    ATTACK *attack;
    SKILL *pSkill;
    int iTrade;
    int iAttack;

    fprintf( fp, "#ACTORS\n" );
    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( pActorIndex = actor_template_hash[iHash]; pActorIndex != NULL; pActorIndex = pActorIndex->next )
        {
            if ( pActorIndex != NULL && pActorIndex->zone == pZone )
            {
                fprintf( fp, "#%d\n",     pActorIndex->dbkey );
                fprintf( fp, "N %s~\n",   fix_string( pActorIndex->name ) );
                fprintf( fp, "SD %s~\n",  fix_string( pActorIndex->short_descr ) );
                fprintf( fp, "LD\n %s~\n", fix_string( pActorIndex->long_descr ) );
                fprintf( fp, "D\n %s~\n",  fix_string( pActorIndex->description ) );
                fprintf( fp, "A %d\n",    pActorIndex->flag );
                fprintf( fp, "AB %d\n",   pActorIndex->bonuses );
                fprintf( fp, "M %d\n",    pActorIndex->money );
                fprintf( fp, "Credits %d\n",    pActorIndex->credits );
                fprintf( fp, "Bucks %d\n",    pActorIndex->bucks );
                fprintf( fp, "S %d\n",    pActorIndex->sex );
                fprintf( fp, "Sz %d\n",   pActorIndex->size );
                fprintf( fp, "K %d E %d\n", pActorIndex->karma, pActorIndex->exp );
                fprintf( fp, "O %s~\n", pActorIndex->owner );
            
                {
                  SPELL_BOOK *pSpellBook;
                  for ( pSpellBook = pActorIndex->pSpells;  pSpellBook != NULL;  pSpellBook = pSpellBook->next )
                  fprintf( fp, "Spell %d\n", pSpellBook->dbkey );
                }     

                fprintf( fp, "AP %d %d %d %d %d\n",
                    pActorIndex->perm_str,
                    pActorIndex->perm_int,
                    pActorIndex->perm_wis,
                    pActorIndex->perm_dex,
                    pActorIndex->perm_con );

                if ( pActorIndex->pShop != NULL )
                {
                     fprintf( fp, "Shop\n" );

                     for( iTrade = 0; iTrade < MAX_TRADE; iTrade++ ) {
                     fprintf( fp, " T %d %d\n", iTrade, pActorIndex->pShop->buy_type[iTrade] );
                     fprintf( fp, " Tr %d %d\n", iTrade, pActorIndex->pShop->trades[iTrade] );
                     }
                     fprintf( fp, " P %d %d\n", pActorIndex->pShop->profit_buy,
                                                pActorIndex->pShop->profit_sell );
                     fprintf( fp, " H %d %d\n", pActorIndex->pShop->open_hour,
                                                pActorIndex->pShop->close_hour  );
                     fprintf( fp, " F %d\n",    pActorIndex->pShop->shop_flags  );
                     fprintf( fp, " R %d\n",    pActorIndex->pShop->repair_rate );
                     fprintf( fp, " BI %d ",  pActorIndex->pShop->buy_index   );
                     fprintf( fp, " CI %d ",  pActorIndex->pShop->comp_index  );
                     fprintf( fp, " SI %d\n", pActorIndex->pShop->sell_index  );
                     fprintf( fp, " Str1\n %s~\n", fix_string( pActorIndex->pShop->no_such_item ) );
                     fprintf( fp, " Str2\n %s~\n", fix_string( pActorIndex->pShop->cmd_not_buy ) );
                     fprintf( fp, " Str3\n %s~\n", fix_string( pActorIndex->pShop->list_header ) );
                     fprintf( fp, " Str4\n %s~\n", fix_string( pActorIndex->pShop->hours_excuse ) );
                     fprintf( fp, "EndShop\n" );
                }

                for ( script = pActorIndex->instances;  script != NULL;  script = script->next )
              if ( script->script && get_script_index( script->script->dbkey ) )
                    fprintf( fp, "Sc %d\n", script->script->dbkey );

                for ( iAttack = 0;  iAttack < MAX_ATTACK; iAttack++ )
                {
                    if ( pActorIndex->attacks[iAttack] != NULL )
                    {
                        attack = pActorIndex->attacks[iAttack];
                        fprintf( fp, "At %d %d %d %d\n",
                          iAttack,
                          attack->dam1,
                          attack->dam2,
                          attack->idx );
                    }
                }

                for ( pSkill = pActorIndex->learned;
                      pSkill != NULL;  pSkill = pSkill->next )
                {
                   if ( pSkill->skill_level > 0 ) 
                   fprintf( fp, "Sk %d '%s'\n",
                     pSkill->skill_level, pSkill->name ); 
                }

                fprintf( fp, "End\n\n" );
            }
        }
    }
    fprintf( fp, "#0\n\n\n\n" );
    return;
}



void save_scripts( FILE *fp, ZONE *pZone )
{
    int iHash;
    SCRIPT *pIndex;

    fprintf( fp, "#SCRIPTDATA\n" );
    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( pIndex = script__hash[iHash]; pIndex != NULL; pIndex = pIndex->next )
        {
            if ( pIndex != NULL && pIndex->zone == pZone )
            {
                FILE *ifp;
                char IFILE  [MAX_STRING_LENGTH];

                sprintf(IFILE, "%s%s_script.txt", SCRIPT_DIR, fix_string( pIndex->name ) );

	        if (! (ifp = fopen( IFILE, "w") ) )
		{
		     bug( "Export_script: script", 0);
		     perror( IFILE );
		}
                
                fprintf( ifp, "* %s\n", fix_string( pIndex->name ) );
                fprintf( ifp, "%s", fix_string(pIndex->commands ) );
                fclose( ifp );

                fprintf( fp, "#%d\n",     pIndex->dbkey );
                fprintf( fp, "N %s~\n",   fix_string( pIndex->name ) );
                fprintf( fp, "C\n%s~\n",  fix_string( pIndex->commands ) );
                fprintf( fp, "T %d\n",    pIndex->type );
                fprintf( fp, "End\n\n" );
            }
        }
    }
    fprintf( fp, "#0\n\n\n\n" );
    return;
}




void save_props( FILE *fp, ZONE *pZone )
{
    int iHash;
    PROP_TEMPLATE *pPropIndex;
    BONUS *pAf;
    EXTRA_DESCR *pEd;
    INSTANCE *script;

    fprintf( fp, "#OBJDATA\n" );
    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( pPropIndex = prop_template_hash[iHash]; pPropIndex != NULL; pPropIndex = pPropIndex->next )
        {
            if ( pPropIndex != NULL && pPropIndex->zone == pZone )
            {
                fprintf( fp, "#%d\n",     pPropIndex->dbkey );
                fprintf( fp, "N %s~\n",   fix_string( pPropIndex->name ) );
                fprintf( fp, "SD %s~\n",  fix_string( pPropIndex->short_descr ) );
                fprintf( fp, "P %s~\n",   fix_string( pPropIndex->short_descr_plural ) );
                fprintf( fp, "D\n%s~\n",  fix_string( pPropIndex->description ) );
                fprintf( fp, "A\n%s~\n",  fix_string( pPropIndex->action_descr ) );
                fprintf( fp, "PD\n%s~\n", fix_string( pPropIndex->description_plural ) );
                fprintf( fp, "DR\n%s~\n", fix_string( pPropIndex->real_description ) );
                fprintf( fp, "L %d\n",    pPropIndex->level );
                fprintf( fp, "T %d\n",    pPropIndex->item_type );
                fprintf( fp, "E %d\n",    pPropIndex->extra_flags );
                fprintf( fp, "W %d\n",    pPropIndex->wear_flags);
                fprintf( fp, "Sz %d\n",   pPropIndex->size );
                fprintf( fp, "Ti %d\n",   pPropIndex->timer );
                fprintf( fp, "Wt %d\n",   pPropIndex->weight );
                fprintf( fp, "C %d\n",    pPropIndex->cost );
                fprintf( fp, "O %s~\n",   pPropIndex->owner );

                for ( script = pPropIndex->instances;  script != NULL;  script = script->next )
              if ( get_script_index( script->script->dbkey ) )
                    fprintf( fp, "Sc %d\n", script->script->dbkey );

                switch ( pPropIndex->item_type )
                {
                default:
                fprintf( fp, "V %d %d %d %d\n",  pPropIndex->value[0],
                                                 pPropIndex->value[1],
                                                 pPropIndex->value[2],
                                                 pPropIndex->value[3] );
                    break;

                case ITEM_PILL:
                case ITEM_POTION:
                case ITEM_SCROLL:
                case ITEM_FOUNTAIN:
                fprintf( fp, "V %d %d %d %d\n",  pPropIndex->value[0],
                                                 pPropIndex->value[1],
                                                 pPropIndex->value[2],
                                                 pPropIndex->value[3] );
                    break;

                case ITEM_STAFF:
                case ITEM_WAND:
                fprintf( fp, "V %d %d %d %d\n",  pPropIndex->value[0],
                                                 pPropIndex->value[1],
                                                 pPropIndex->value[2],
                                                 pPropIndex->value[3] );
                    break;
                }

                for( pAf = pPropIndex->bonus; pAf != NULL; pAf = pAf->next )
                {
                fprintf( fp, "Af %d %d %d %d %d\n",  pAf->location,
                                                     pAf->modifier,
                                                     pAf->type,
                                                     pAf->duration,
                                                     pAf->bitvector );
                }

                for( pEd = pPropIndex->extra_descr; pEd != NULL; pEd = pEd->next )
                {
				fprintf( fp, "ED %s~\n%s~\n", pEd->keyword,
										 fix_string( pEd->description ) );
                }

                fprintf( fp, "End\n\n" );
            }
        }
    }
    fprintf( fp, "#0\n\n\n\n" );
    return;
}



void save_scenes( FILE *fp, ZONE *pZone )
{
    int iHash;
    SCENE *pSceneIndex;
    EXTRA_DESCR *pEd;
    SPAWN *pSpawn;
    EXIT *pExit;
    int door;

    fprintf( fp, "#SCENES\n" );
    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( pSceneIndex = scene_hash[iHash]; pSceneIndex != NULL; pSceneIndex = pSceneIndex->next )
        {
            if ( pSceneIndex->zone == pZone )
            {
                fprintf( fp, "#%d\n",    pSceneIndex->dbkey );
                fprintf( fp, "N %s~\n",  fix_string( pSceneIndex->name ) );
                fprintf( fp, "Ref %d\n", pSceneIndex->template );
                fprintf( fp, "D\n%s~\n", fix_string( pSceneIndex->description ) );
		fprintf( fp, "C\n%s~\n", fix_string( pSceneIndex->client ) );
                REMOVE_BIT(pSceneIndex->scene_flags, SCENE_MARK);
                fprintf( fp, "F %d\n",   pSceneIndex->scene_flags  );
                fprintf( fp, "S %d\n",   pSceneIndex->move );
                fprintf( fp, "M %d\n",   pSceneIndex->max_people  );
                fprintf( fp, "W %d\n",   pSceneIndex->wagon       );
                fprintf( fp, "T %d\n",   pSceneIndex->terrain     );
                fprintf( fp, "O %s~\n",  pSceneIndex->owner );

                for ( pEd = pSceneIndex->extra_descr; pEd != NULL;
                      pEd = pEd->next )
                {
					fprintf( fp, "ED %s~\n%s~\n",   pEd->keyword,
                                              fix_string( pEd->description ) );
                }

                for( pSpawn = pSceneIndex->spawn_first; pSpawn != NULL; pSpawn = pSpawn->next )
                {
                    if ( pSpawn != NULL )
                    {
                      fprintf( fp, "R %c %d %d %d %d\n", pSpawn->command,
                                                         pSpawn->rs_dbkey,
                                                         pSpawn->loc,
                                                         pSpawn->percent,
                                                         pSpawn->num );
                    }
                }

                for( door = 0; door < MAX_DIR; door++ )
                {
                    if ( (pExit = pSceneIndex->exit[door] ) != NULL
                          && pExit->dbkey > 0
                          && (fBootDb || get_scene( pExit->dbkey ) != NULL) )
                    {
                        fprintf( fp, "Dr %d %d %d %d\n", door,
                                                        pExit->rs_flags,
                                                        pExit->key,
                                                        pExit->dbkey );

                        fprintf( fp, "%s~\n", fix_string( pExit->description ) );
                        if ( pExit->keyword != NULL )
                        fprintf( fp, "%s~\n", fix_string( pExit->keyword ) );
                        else fprintf( fp, "~\n" );

                    }
                }
                fprintf( fp, "End\n\n" );
            }
        }
    }
    fprintf( fp, "#0\n\n\n\n" );
    return;
}



void save_zone( ZONE *pZone )
{
    FILE *fp;

    if (!pZone) return;

    REMOVE_BIT( pZone->zone_flags, ZONE_CHANGED );
    if ( ( fp = fopen( pZone->filename, "w" ) ) == NULL )
    {
    bug( "Open_zone: fopen", 0 );
    perror( pZone->filename );
    }

    fprintf( fp, "#ZONE \n" );
    fprintf( fp, "N %s~\n",        fix_string( pZone->name ) );
    if ( !MTD(pZone->repop) )
    fprintf( fp, "R %s~\n",        fix_string( pZone->repop ) );
    fprintf( fp, "B %s~\n",        fix_string( pZone->builders ) );
    fprintf( fp, "V %d %d\n",      pZone->ldbkey, pZone->udbkey );
    fprintf( fp, "S %d\n",         pZone->security );
    if ( IS_SET(pZone->zone_flags, ZONE_STATIC) )
    fprintf( fp, "Static\n" );
    fprintf( fp, "End\n\n\n\n" );

    save_scripts( fp, pZone );
    save_actors( fp, pZone );
    save_props( fp, pZone );
    save_scenes( fp, pZone );

    fprintf( fp, "#$\n" );

    fclose( fp );
    return;
}

void save_helps( void )
{
  HELP * pHelp;
  FILE * fp;
  char HFILE  [MAX_STRING_LENGTH];
  int cidx,dbkey;

  for ( cidx=0; cidx < MAX_HELP_CLASS; cidx++ ) {
     char buf [MAX_STRING_LENGTH];

     sprintf(HFILE, "%s", help_class_table[cidx].filename);

     if (! (fp = fopen( HFILE, "w") ) )
     {
         bug( "Open_help: fopen", 0);
         perror( HFILE );
     }

     for ( dbkey=0; dbkey <= top_dbkey_help; dbkey++ ) {
       pHelp = get_help_index( dbkey ); 
       if ( !pHelp || pHelp->class != cidx ) continue;

       one_argcase( pHelp->name, buf ); replace_char(buf, ' ', '_');
       fprintf(fp, "\n\n#DOC %d\nN %s~\n", dbkey, buf );

       fprintf(fp, "C %d L %d\nKW %s~\n",
               pHelp->class, pHelp->level, pHelp->keyword );
       if ( pHelp->syntax ) fprintf(fp, "SY %s~\n", fix_string(pHelp->syntax));

       if ( pHelp->text && pHelp->text[0] == ' ' ) fprintf(fp, ".\n%s~\n", fix_string(pHelp->text) );
       else fprintf(fp, "TX %s~\n", fix_string(pHelp->text)); 

       if ( pHelp->immtext ) fprintf(fp, "IT %s~\n", fix_string(pHelp->immtext));
       if ( pHelp->example ) fprintf(fp, "EX %s~\n", fix_string(pHelp->example));
       if ( pHelp->seealso ) fprintf(fp, "SA %s~\n", pHelp->seealso);
       fprintf(fp, "End" );
     }

     fprintf(fp,"\n#$\n#$\n");
     fclose(fp);
   }
   return;
}

void cmd_hstat( PLAYER *ch, char *argument )
{
   HELP *pHelp;
   char buf  [MAX_STRING_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
 
   argument = one_argument(argument,arg1);

   pHelp = get_help_index( atoi(arg1) );

   if ( pHelp ) {
         snprintf( buf, MAX_STRING_LENGTH, "Name:     %s\n\r", pHelp->name );
         send_to_actor( buf, ch );
         snprintf( buf, MAX_STRING_LENGTH, "Class:    %d (%s)\n\r", pHelp->class, help_class_table[pHelp->class].name );
         send_to_actor( buf, ch );
         snprintf( buf, MAX_STRING_LENGTH, "Level:    %d\n\r", pHelp->level );
         send_to_actor( buf, ch );
         snprintf( buf, MAX_STRING_LENGTH, "Keywords: %s\n\r", pHelp->keyword );
         send_to_actor( buf, ch );
         snprintf( buf, MAX_STRING_LENGTH, "Syntax:   %s\n\r", pHelp->syntax );
         send_to_actor( buf, ch );
         snprintf( buf, MAX_STRING_LENGTH, "Text:\n%s\n\r", pHelp->text );
         send_to_actor( buf, ch );
         snprintf( buf, MAX_STRING_LENGTH, "Immtext:\n%s\n\r", pHelp->immtext );
         send_to_actor( buf, ch );
         snprintf( buf, MAX_STRING_LENGTH, "Example:\n%s\n\r", pHelp->example );
         send_to_actor( buf, ch );
         snprintf( buf, MAX_STRING_LENGTH, "Seealso:  %s\n\r", pHelp->seealso );
         send_to_actor( buf, ch );
         return;
      }

   snprintf( buf, MAX_STRING_LENGTH, "There is no document for %s\n", arg1);
   send_to_actor( buf, ch );
}

/* Locke's improved documentation system.
 * Syntax: hedit [name]
 *         hedit create [name]
 */
void cmd_hedit( PLAYER *ch, char *argument )
{
    HELP *pHelp;
    int value;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];

    if ( NPC(ch) ) return;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( is_number( arg1 ) )
    {
        value = atoi( arg1 );
        if ( ( pHelp = get_help_index( value ) ) == NULL )
        {
            send_to_actor( "Edit: That dbkey does not exist.\n\r", ch );
            return;
        }

        ch->desc->pEdit = (void *)pHelp;
        ch->desc->connected = NET_HEDITOR;
        return;
    }
    else 
    {
            snprintf( arg1, MAX_STRING_LENGTH, "%s", argument );
            hedit( ch, arg1 );
            return;
    }

   send_to_actor( "Edit:  There is no default help to edit.\n\r", ch );
   return;
}


void hedit( PLAYER *ch, char *argument )
{
    HELP *pHelp;
    char arg[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char arg3[MAX_STRING_LENGTH];
    char buf [MAX_STRING_LENGTH];
    char *o=argument;
    int  value, tdbkey, enddbkey=top_dbkey_help;

    pHelp = (HELP *)ch->desc->pEdit;
    strcpy( arg, argument );
    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );


    if ( !IS_IMMORTAL( ch ) )
    {
        send_to_actor( "Help:  Insufficient security to modify help file.\n\r", ch );
    }

    if ( pHelp == NULL ) {
        int col=0;
           /* search for keywords that match */

        for ( tdbkey=0;tdbkey <= top_dbkey_help; tdbkey++ )
        { 
            pHelp = get_help_index( tdbkey );
            if ( !pHelp ) continue;

            if ( str_infix( o, pHelp->keyword )
             && str_cmp( o, pHelp->name ) 
             && str_prefix( o, pHelp->name ) )
            continue;   col++;

            snprintf( buf, MAX_STRING_LENGTH, "[%3d] Name [%-18s] ", 
                     pHelp->dbkey,
                     pHelp->name );

            if ( col % 2 == 0 ) page_to_actor("\n\r", ch );
            page_to_actor( buf, ch );
         }
         return;
    }
    else
    if ( !str_cmp( arg1, "list" ) )
    {
        tdbkey = atoi(arg2);
        enddbkey = atoi(arg3);  
        if (enddbkey < tdbkey) enddbkey=top_dbkey_help;
        
        for ( ;tdbkey <= enddbkey; tdbkey++ )
        { 
            pHelp = get_help_index( tdbkey );
            if ( !pHelp ) continue;

            snprintf( buf, MAX_STRING_LENGTH, "[%3d] Name [%-18s] ", 
                     pHelp->dbkey,
                     pHelp->name );

            if ( tdbkey % 2 == 0 ) send_to_actor("\n\r", ch );
            send_to_actor( buf, ch );
        }
        return;
    }


#if defined(NEVER)
    /*
     * Load a help file from old formats.
     */

    if ( !str_cmp( arg1, "import" ) ) {

       int cidx;
       FILE *fp = fopen( arg2, "r" );

       if ( !fp ) {
         send_to_actor( "File not found.\n\r", ch );
         return; }
       pHelp = NULL;

       fread_word(fp);

   for ( ; ; )
   {
      char *hname;
      HELP *hSort;
      int sdbkey;

      if ( fread_letter( fp ) != '#' ) {
         bug( "Load_helpdata: # not found.", 0 );
      }

      hname       = fread_word(fp);
      if ( hname[0] == '$' ) {
         break;
      }

      if ( !pHelp ) pHelp = new_help();

      pHelp->dbkey = top_dbkey_help;
      pHelp->name       = str_dup( hname );
      pHelp->class      = fread_number( fp );
      pHelp->level      = fread_number( fp );
      pHelp->keyword    = fread_string( fp );
      pHelp->syntax     = fread_string( fp );
      pHelp->text       = fread_string( fp );
      pHelp->immtext    = fread_string( fp );
      pHelp->example    = fread_string( fp );
      pHelp->seealso    = fread_string( fp );

            pHelp->next = help__hash[pHelp->dbkey % MAX_KEY_HASH];
            help__hash[pHelp->dbkey % MAX_KEY_HASH] = pHelp;
            pHelp = NULL;
   }

   fclose(fp);
   send_to_actor( "Imported.\n\r", ch );
   return;
       
    }
#endif

    if ( !str_cmp( arg1, "show" ) || arg1[0] == '\0' ) {
       clrscr(ch);
       snprintf( buf, MAX_STRING_LENGTH, "%d", pHelp->dbkey );
       cmd_hstat( ch, buf ); 
       return;
    }

   if ( !str_cmp( arg1, "done" ) )
   {
       ch->desc->pEdit = NULL;
       ch->desc->connected = NET_PLAYING;
       cmd_zsave( ch, "helps" );
       return;
   }

   if ( !str_cmp( arg1, "?" ) )
   {
      cmd_help( ch, "hedit" );
      return;
   }

   if ( !str_cmp( arg1, "level" ) ) {
      if ( !is_number( arg2 ) || arg2[0] == '\0' ) {
         send_to_actor( "Syntax:  level [number]\n\r", ch );
         return;
      }

      value = atoi( arg2 );

      if ( (value < -1) || (value > MAX_LEVEL) ) {
         snprintf( buf, MAX_STRING_LENGTH, "Hedit:  Valid level range is from -1 to %d.\n", MAX_LEVEL);
         send_to_actor( buf, ch );
         return;
      }

      pHelp->level = value;
      send_to_actor( "Level set.\n\r", ch );
      return;
   }

   if ( !str_cmp( arg1, "class" ) ) {
      if ( !is_number( arg2 ) || arg2[0] == '\0' ) {
         cmd_help( ch, "hclass" );
         return;
      }

      value = atoi( arg2 );

      if ( (value < 0) || (value > MAX_HELP_CLASS-1) ) {
         snprintf( buf, MAX_STRING_LENGTH, "Hedit:  Valid class range is from 0 to %d.\n", MAX_HELP_CLASS);
         send_to_actor( buf, ch );
         return;
      }

      pHelp->class = value;
      send_to_actor( "Help class has been set.\n\r", ch );
      return;
   }

   if ( !str_cmp( arg1, "text" )) {
      string_append( ch, &pHelp->text );
      return;
   }

   if ( !str_cmp( arg1, "immtext" )) {
      string_append( ch, &pHelp->immtext );
      return;
   }

   if ( !str_cmp( arg1, "example" )) {
      string_append( ch, &pHelp->example );
      return;
   }

   if ( !str_cmp( arg1, "syntax" )) {
      string_append( ch, &pHelp->syntax );
      return;
   }

   if ( !str_cmp( arg1, "keywords" )) {
      if(arg2[0] == '\0') {
         send_to_actor(" Syntax: keywords [keywords]\n\r",ch);
         return;
      }

      free_string( pHelp->keyword );
      pHelp->keyword = str_dup( arg2 );

      send_to_actor( "Keyword(s) Set.\n\r", ch);
      return;
   }

   if ( !str_cmp( arg1, "name" )) {
      if(arg2[0] == '\0') {
         send_to_actor(" Syntax: name [title]\n\r",ch);
         return;
      }

      free_string( pHelp->name );
      pHelp->name = str_dup( one_argument( arg, arg1 ) );

      send_to_actor( "Name set.\n\r", ch);
      return;
   }

   if ( !str_cmp( arg1, "seealso" )) {
      if(arg2[0] == '\0') {
         send_to_actor(" Syntax: seealso [list]\n\r",ch);
         return;
      }

      free_string( pHelp->seealso );
      pHelp->seealso = str_dup( one_argument( arg, arg1 ) );

      send_to_actor( "SeeAlso set.\n\r", ch);
      return;
   }

   interpret( ch, arg );
   return;
}



/*
 * Syntax:  zsave all
 *          zsave world
 *          zsave list
 *          zsave changed
 *          zsave zone
 */
void cmd_zsave( PLAYER *ch, char *argument )
{
    ZONE *pZone;

        save_zone_list();

        for( pZone = zone_first; pZone != NULL; pZone = pZone->next )
        {
            save_zone( pZone );
            REMOVE_BIT( pZone->zone_flags, ZONE_CHANGED );
        }

        save_config();
        save_contents();
        save_helps();
        save_zone_list();
        save_config();
        save_contents();
        save_helps();

    send_to_actor( "You saved the world.\n\r", ch );
    return;
}
