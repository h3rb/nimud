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
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <zlib.h>

#include "nimud.h"
#include "net.h"
#include "skills.h"
#include "script.h"
#include "defaults.h"

/*
 * From newbie.c
 */

extern int guestnumber;

int num_str_dup = 0;           /* str_dup counter for cmd_mem */

/* Memory recycling */

BONUS              *   bonus_free;
ATTACK             *   attack_free;
CONNECTION         *   connection_free;
SPAWN              *   spawn_free;
INSTANCE           *   instance_free;
SCRIPT             *   script_free;
VARIABLE           *   variable_free;
ZONE               *   zone_free;
EXTRA_DESCR        *   extra_descr_free;
EXIT               *   exit_free;
SHOP               *   shop_free;
ACTOR_TEMPLATE        *   actor_template_free;
PROP_TEMPLATE         *   prop_template_free;
SCENE        *   scene_free;
SPELL              *   spell_free;
SKILL              *   skill_free;
SPAWN              *   spawn_free;
ATTACK             *   attack_free;
PLAYER             *   actor_free;
USER               *   pc_free;
SPELL_BOOK         *   spell_book_free;
PROP               *   prop_free;
NOTE               *   note_free;
ALIAS              *   alias_free;
EVENT              *   event_free;   
SPELL              *   spell_free;
SKILL              *   skill_free;
HELP               *   help_free;

NUMBER_LIST             *   number_list_free; 
STRING_LIST             *   string_list_free;         

BONUS               bonus_zero;
ATTACK              attack_zero;
CONNECTION          connection_zero;
SPAWN               spawn_zero;
INSTANCE            instance_zero;
SCRIPT              script_zero;
VARIABLE            variable_zero;
ZONE                zone_zero;
EXTRA_DESCR         extra_descr_zero;
EXIT                exit_zero;
SCENE         scene_zero;
PROP_TEMPLATE          prop_template_zero;
SHOP                shop_zero;
ACTOR_TEMPLATE         actor_template_zero;
SPAWN               spawn_zero;
ATTACK              attack_zero;
PLAYER              ch_zero;
USER                pc_zero;
PROP                prop_zero;
TERRAIN             terrain_zero;
SPELL               spell_zero;
SKILL               skill_zero;
SPELL_BOOK          spell_book_zero;
ALIAS               alias_zero;
EVENT               event_zero;  
HELP                help_zero;

#if defined(PLANETARY)
PLANETARY           planet_zero;
STELLAR             stellar_zero;
LUNAR               lunar_zero;
#endif

NUMBER_LIST              number_zero;
STRING_LIST              string_zero;

/*
 * Memory management.
 * Increase MAX_STRING if you have too.
 * Tune the others only if you understand what you're doing.
 */
#define                 MAX_STRING      (MAX_STRING_LENGTH*MIL)
#define			MAX_PERM_BLOCK	(MAX_STRING_LENGTH*10)
#define			MAX_MEM_LIST	13

void *			rgFreeList	[MAX_MEM_LIST];
const int		rgSizeList	[MAX_MEM_LIST]	=
{
    16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768-64, 65536-64, 131072-64
};

int         nAllocString;
int         sAllocString;
int         nAllocPerm;
int         sAllocPerm;

int         num_pulse;

char *      string_space;
char *      top_string;
char        str_empty       [1];

int         top_bonus          = 0;
int         top_zone           = 0;
int         top_ed             = 0;
int         top_exit           = 0;
int         top_help           = 0;
int         top_actor_template    = 0;
int         top_prop_template     = 0;
int         top_spawn          = 0;
int         top_scene          = 0;
int         top_spell          = 0;
int         top_skill          = 0;
int         top_shop           = 0;
int         top_variable       = 0;
int         top_event          = 0;
int         top_instance       = 0;
int         top_script         = 0;
int         top_player    = 0;
int         top_userdata       = 0;
int         top_alias     = 0;
int         top_attack         = 0;
int         top_prop           = 0;
int         top_connection     = 0;
int         top_note           = 0;

int         top_dbkey_help      = 0;
int         top_dbkey_spell     = 0;
int         top_dbkey_skill     = 0;
int         top_dbkey_script    = 0;
int         top_dbkey_actor     = 0;
int         top_dbkey_prop      = 0;
int         top_dbkey_scene     = 0;
int         top_dbkey_terrain   = 0;


extern bool            fBootDb;
extern FILE *          fpZone;
extern char            strzone[MAX_INPUT_LENGTH];
extern int             social_count;

VARIABLE           * mud_var_list;


#define         ZONE_NONE       0
#define         ZONE_CHANGED    1
#define         ZONE_ADDED      2

#define         SEX_NONE        4
#define         SCENE_NONE       0
#define         EXIT_NONE         0
#define         ITEM_NONE       0
#define         EXTRA_NONE      0
#define         ITEM_WEAR_NONE  0
#define         ACTOR_NONE        0
#define         AFFECT_NONE     0




HELP               *   help_free;
HELP                help_zero;



NUMBER_LIST *new_number_list( void )
{
    NUMBER_LIST *pNumber;

    if ( number_list_free == NULL )
    {
         pNumber = alloc_perm( sizeof(*pNumber) );
    }
    else {
         pNumber = number_list_free;
         number_list_free = number_list_free->next;
    }

    *pNumber = number_zero;
    pNumber->num = 0;
    pNumber->next = NULL;

    return pNumber;   
}      


STRING_LIST *new_string_list( void )
{
    STRING_LIST *pString;

    if ( string_list_free == NULL )
    {
         pString = alloc_perm( sizeof(*pString) );
    }
    else {
         pString = string_list_free;
         string_list_free = string_list_free->next;
    }

    *pString = string_zero;
    pString->str = str_dup( "" );
    pString->next = NULL;
    return pString;   
}      



void free_number_list ( NUMBER_LIST *pNumber )
{
   pNumber->next = number_list_free;
   number_list_free = pNumber;
   return;
}

void free_string_list( STRING_LIST *pString )
{
    free_string( pString->str );

    pString->next = string_list_free;
    string_list_free = pString->next;
    return;
}



void free_help( HELP *pHelp )
{
    free_string( pHelp->keyword );
    free_string( pHelp->text );
    free_string( pHelp->name );
    free_string( pHelp->immtext );
    free_string( pHelp->seealso );
    free_string( pHelp->syntax );
    free_string( pHelp->example );

    if (help_free == NULL ) {
       pHelp->next         =   NULL;
    } else {
       pHelp->next         =   help_free;
    }

    help_free           =   pHelp;
    return;
}


CONNECTION *new_connection( void )
{
    CONNECTION *dnew;

    if ( connection_free == NULL )
    {
        dnew = alloc_mem( sizeof(*dnew) );
        top_connection++;
    }
    else
    {
        dnew            = connection_free;
        connection_free = connection_free->next;
    }

    *dnew               = connection_zero;
    dnew->host          = str_dup( "" );
    dnew->connection    = -1;
    dnew->connected     = NET_SHOW_TITLE;
    dnew->lingua        = 0;
    dnew->pager_head  = NULL;
    dnew->pager_point = NULL;
    dnew->outsize       = 2000;
    dnew->pEdit         = NULL;
    dnew->pString       = NULL;
    dnew->fpromptok     = TRUE;
    dnew->mccp=0;
    dnew->teltop=0;
    dnew->terminal_type = strdup("");
    dnew->comm_flags=0;
    dnew->cols=0;
    dnew->rows=0;
    dnew->outtop=dnew->msouttop=0;
    dnew->outbuf        = alloc_mem( dnew->outsize );

    return dnew;
}



void free_connection( CONNECTION *d )
{
    free_string( d->host );
    free_mem( d->outbuf, d->outsize );      /* RT socket leak fix */
    d->next    = connection_free;
    connection_free = d;
    return;
}




PLAYER *new_player( void )
{
    PLAYER *ch;

    if ( actor_free == NULL )
    {
        ch          =   alloc_perm( sizeof(*ch) );
        top_player++;
    }
    else
    {
        ch             =   actor_free;
        actor_free      =   actor_free->next;
    }

    *ch                 = ch_zero;
    ch->name            = &str_empty[0];
    ch->fmode           = 3;
    ch->short_descr     = &str_empty[0];
    ch->long_descr      = &str_empty[0];
    ch->description     = &str_empty[0];
    ch->prompt          = &str_empty[0];
    ch->tracking         = &str_empty[0];
    ch->hunting         = &str_empty[0];
    ch->keywords        = &str_empty[0];
    ch->hkeywords       = &str_empty[0];
    ch->armor           = 100;
    ch->position        = POS_STANDING;
    ch->master          = NULL;
    ch->monitor         = NULL;
    ch->leader          = NULL;
    ch->fighting        = NULL;
    ch->riding          = NULL;
    ch->rider           = NULL;
    ch->furniture       = NULL;
    ch->spells          = NULL; 
    ch->exp             = 1;
    ch->exp_level       = 0;    
    ch->pIndexData      = NULL;
    ch->bonus           = NULL;
    ch->pnote           = NULL;
    ch->instances       = NULL;
    ch->timer           = -1;
    ch->wait            = 0;
    ch->bonuses         = 0;
    ch->position        = POS_RESTING;
    ch->hitroll         = 0;
    ch->flag             = PLR_BLANK | PLR_COMBINE | PLR_PROMPT;
    ch->flag2            = 0;
    ch->bounty          = 0;
    ch->owed            = 0;
    ch->damroll         = 0;

    ch->perm_str        = number_range(1,3) + 11;
    ch->perm_int        = number_range(1,3) + 11;
    ch->perm_wis        = number_range(1,3) + 11;
    ch->perm_dex        = number_range(1,3) + 11;
    ch->perm_con        = number_range(1,3) + 11;

    ch->mod_str         = 0;
    ch->mod_int         = 0;
    ch->mod_wis         = 0;
    ch->mod_dex         = 0;
    ch->mod_con         = 0;

    ch->hit             = MAXHIT(ch);
    ch->move            = MAXMOVE(ch);
    ch->pagelen         = 20;
    ch->size            = SIZE_AVERAGE;

    ch->userdata          = new_user( );
                              
    return ch;
}


void free_char( PLAYER *ch )
{
    PROP *prop, *prop_next;
    BONUS *paf, *paf_next;
    INSTANCE *t, *trigger_next;
    VARIABLE *va, *va_next;
    PLAYER *pet;

    if ( ch == NULL ) return;    

    clear_events( ch, TYPE_ACTOR );

    for ( pet = actor_list; pet != NULL; pet = pet->next )
    {
        if ( ( pet->master == ch  ) && IS_SET(pet->flag, ACTOR_PET) )
        break;
    }
    if ( pet != NULL ) extractor_char( pet, TRUE );

    for ( prop = ch->carrying; prop != NULL; prop = prop_next )
    {
        prop_next = prop->next_content;
        extractor_prop( prop );
    }

    for ( t = ch->instances; t != NULL; t = trigger_next )
    {
    trigger_next     = t->next;
    free_instance( t );
    }

    for ( va = ch->globals;  va != NULL; va = va_next )
    {
        va_next = va->next;
        free_variable( va );
    }

    for ( paf = ch->bonus; paf != NULL; paf = paf_next )
    {
        paf_next = paf->next;
        bonus_remove( ch, paf );
    }

    free_string( ch->name           );
    free_string( ch->short_descr    );
    free_string( ch->long_descr     );
    free_string( ch->description    );
    free_string( ch->prompt         );
    free_string( ch->tracking        );
    free_string( ch->keywords       );

    clear_spell_book( ch->spells ); 


#if defined(IMC)
    if ( ch->userdata ) imc_freechardata( ch );
#endif
    
    free_user( ch->userdata );
    ch->userdata = NULL;

    ch->next         = actor_free;
    actor_free        = ch;
    return;
}


#if defined(INTERGALACTIC_PLANETARY)
STELLAR *new_stellar( void ) 
{
    STELLAR *stellar;

    if ( stellar_free == NULL )
    {
        stellar = alloc_perm( sizeof(*stellar) );
        top_stellar++;
    }
    else
    {
        stellar = stellar_free;
        stellar_free = stellar_free->next;
    }

    *stellar = stellar_zero;
     stellar->name = str_dup( "" );
     return stellar;
}

LUNAR *new_lunar( void ) 
{
    LUNAR *lunar;

    if ( lunar_free == NULL )
    {
        lunar = alloc_perm( sizeof(*lunar) );
        top_lunar++;
    }
    else
    {
        lunar = lunar_free;
        lunar_free = lunar_free->next;
    }

    *lunar = lunar_zero;
     lunar->name = str_dup( "" );
     lunar->description = str_dup( "" );
     return lunar;
}

PLANETARY *new_planet( void ) 
{
    PLANETARY *planetary;

    if ( planet_free == NULL )
    {
        planetary = alloc_perm( sizeof(*planetary) );
        top_planet++;
    }
    else
    {
        planetary = planet_free;
        planet_free = planet_free->next;
    }

    *planetary = planet_zero;
     planetary->name = str_dup( "" );
     planetary->description = str_dup( "" );
     return planetary;
}
#endif

ALIAS *new_alias( void )
{
    ALIAS *alias;

    if ( alias_free == NULL )
    {
        alias         =   alloc_perm( sizeof(*alias) );
        top_alias++;
    }
    else
    {
        alias         =   alias_free;
        alias_free    =   alias_free->next;
    }

    *alias            = alias_zero;
    alias->name       = str_dup( "" );
    alias->exp        = str_dup( "" );

    return alias;
}


void free_alias( ALIAS *alias )
{
    if ( alias == NULL ) return;

    alias->next = alias_free;
    alias_free = alias;
    
    free_string( alias->name );
    free_string( alias->exp );
    return;
}


void free_user( USER *pc )
{
    ALIAS         *alias, *alias_next;

    if ( pc == NULL ) return;

    pc->next = pc_free;
    pc_free  = pc;    

    for ( alias = pc->aliases;  alias != NULL;  alias = alias_next ) {
       alias_next = alias->next;
       free_alias( alias );
    }

    free_string( pc->email         );
    free_string( pc->bamfout       );
    free_string( pc->bamfin        );
    free_string( pc->constellation );
    free_string( pc->pwd           );
    free_string( pc->denial        );
    free_string( pc->history       );
    return;
};
    


USER *new_user( void )
{
    USER *pc;

    if ( pc_free == NULL )
    {
        pc           =   alloc_perm( sizeof(*pc) );
        top_userdata++;
    }
    else
    {
        pc           =   pc_free;
        pc_free      =   pc_free->next;
    }

    *pc                         = pc_zero;
    pc->security                = 9;
    pc->wizinvis                = 0;
    pc->home                    = 0;
    pc->death                   = SCENE_VNUM_DEATH;
    pc->flag = &str_empty[0];

    pc->tell_buf[0] ='\0';
    pc->say_buf[0] ='\0';
    pc->ooc_buf[0] ='\0';
    pc->chat_buf[0] ='\0';
    pc->immt_buf[0] ='\0';

    pc->tell_last[0] ='\0';
    pc->say_last[0] ='\0';
    pc->chat_last[0] ='\0';
    pc->immt_last[0] ='\0';

    {
        int x;

        for ( x = 0; x < MAX_COLORS; x++ )
        pc->colors[x]           = 15;
    }

    /*
     * Defaults.
     */
/*
    pc->colors[3]               = 2;
    pc->colors[0]               = 3;
    pc->colors[4]               = 13;
    pc->colors[5]               = 12;
 */

    pc->condition[COND_THIRST]  = 50;
    pc->condition[COND_FULL]    = 50;
    pc->bamfin                  = &str_empty[0];
    pc->bamfout                 = &str_empty[0];
    pc->constellation           = &str_empty[0];
    pc->email                   = &str_empty[0];
    pc->pwd                     = &str_empty[0];
    pc->denial                  = &str_empty[0];
    pc->title                   = &str_empty[0];
    pc->history                 = &str_empty[0]; 
    pc->logon                   = current_time;
    pc->app_time                = 0;
    pc->next                    = NULL;


#if defined(IMC)
    pc->imcchardata            = NULL;
#endif
    return pc;
}



PROP *new_prop( void )
{
    PROP *pProp;

    if ( prop_free == NULL )
    {
        pProp          =   alloc_perm( sizeof(*pProp) );
        top_prop++;
    }
    else
    {
        pProp          =   prop_free;
        prop_free      =   prop_free->next;
    }

    *pProp                     = prop_zero;
    pProp->next                = NULL;
    pProp->next_content        = NULL;
    pProp->contains            = NULL;
    pProp->in_prop             = NULL;
    pProp->carried_by          = NULL;
    pProp->extra_descr         = NULL;
    pProp->bonus               = NULL;
    pProp->pIndexData          = NULL;
    pProp->in_scene            = NULL;
    pProp->instances           = NULL;           /* for scripts */
    pProp->current             = NULL;
    pProp->name                = NULL;
    pProp->short_descr         = NULL;
    pProp->short_descr_plural  = NULL;
    pProp->description         = NULL;
    pProp->description_plural  = NULL;
    pProp->action_descr        = NULL;
    pProp->real_description    = NULL;
    pProp->item_type           = ITEM_TRASH;
    pProp->extra_flags         = ITEM_TAKE;
    pProp->wear_flags          = 0;
    pProp->wear_loc            = WEAR_NONE;
    pProp->weight              = 1;
    pProp->cost                = 0;
    pProp->level               = 0;
    pProp->timer               = -1;
    pProp->size                = 0;
    pProp->material            = 0;
    pProp->value[0]            = 0;
    pProp->value[1]            = 0;
    pProp->value[2]            = 0;
    pProp->value[3]            = 0;

    return pProp;
};




void free_prop( PROP *pProp )
{
    EXTRA_DESCR *ed, *ed_next;
    BONUS *af, *af_next;
    INSTANCE *tr, *tr_next;
    VARIABLE *va, *va_next;


    pProp->next = prop_free;
    prop_free   = pProp;

    clear_events( pProp, TYPE_PROP );

    free_string( pProp->name );
    free_string( pProp->short_descr );
    free_string( pProp->short_descr_plural );
    free_string( pProp->description );
    free_string( pProp->description_plural );
    free_string( pProp->action_descr );
    free_string( pProp->real_description );

    for ( ed = pProp->extra_descr;  ed != NULL; ed = ed_next )
    {
        ed_next = ed->next;
        free_extra_descr( ed );
    }

    for ( af = pProp->bonus;  af != NULL; af = af_next )
    {
        af_next = af->next;
        free_bonus( af );
    }

    for ( tr = pProp->instances;  tr != NULL; tr = tr_next )
    {
        tr_next = tr->next;
        free_instance( tr );
    }

    for ( va = pProp->globals;  va != NULL; va = va_next )
    {
        va_next = va->next;
        free_variable( va );
    }

    return;
};



SPAWN *new_spawn( void )
{
    SPAWN *pSpawn;

    if ( spawn_free == NULL )
    {
        pSpawn          =   alloc_perm( sizeof(*pSpawn) );
        top_spawn++;
    }
    else
    {
        pSpawn          =   spawn_free;
        spawn_free      =   spawn_free->next;
    }

    *pSpawn             =   spawn_zero;
    pSpawn->next        =   NULL;
    pSpawn->command     = 'X';
    pSpawn->loc         =  -1;
    pSpawn->percent     =   0;
    pSpawn->rs_dbkey     =   0;
    pSpawn->dbkey        =   0;
    pSpawn->num         =   1;

    return pSpawn;
}



void free_spawn( SPAWN *pSpawn )
{
    pSpawn->next            = spawn_free;
    spawn_free              = pSpawn;
    return;
}


TERRAIN *new_terrain( void )
{
    TERRAIN *pTerrain;

    pTerrain        =   alloc_perm( sizeof(*pTerrain) );
    top_dbkey_terrain++;

    *pTerrain           =   terrain_zero;
    pTerrain->next        =   NULL;
    pTerrain->name        = str_dup( "unset" );
    pTerrain->winter      = str_dup( "It is winter.\n\r" );
    pTerrain->spring      = str_dup( "It is spring.\n\r" );
    pTerrain->summer      = str_dup( "It is summer.\n\r" );
    pTerrain->fall        = str_dup( "It is fall.\n\r" );
    pTerrain->map_char    = '.';
    pTerrain->dbkey         =  top_dbkey_terrain;

    return pTerrain;
}



SKILL *new_skill( void )
{
    SKILL *pSkill;

    pSkill        =   alloc_perm( sizeof(*pSkill) );
    top_dbkey_skill++;  top_skill++;

    *pSkill             =   skill_zero;
    pSkill->name        =   str_dup( "" );
    pSkill->next        =   NULL;
    pSkill->dbkey        =   top_dbkey_skill;  

    return pSkill;
}

/*
 * Does not free the strings.  Skill index is static.
 */
void free_skill( SKILL *pSkill )
{
    if ( pSkill == NULL ) return;

    pSkill->next = skill_free;
    skill_free  = pSkill;

    return;
};

HELP *new_help( void )
{
    HELP *pHelp;

    pHelp        =   alloc_perm( sizeof(*pHelp) );
    top_dbkey_help++;  top_help++;

    *pHelp             =   help_zero;
    pHelp->next        =   NULL;
    pHelp->dbkey        =   top_dbkey_help;  

    pHelp->name=
    pHelp->immtext=
    pHelp->seealso=
    pHelp->example=
    pHelp->syntax=
    pHelp->text=
    pHelp->keyword=   &str_empty[0];

    return pHelp;
}


SPELL *new_spell( void )
{
    SPELL *pSpell;

    pSpell        =   alloc_perm( sizeof(*pSpell) );
    top_dbkey_spell++;  top_spell++;

    *pSpell             =   spell_zero;
    pSpell->name        =   str_dup( "" );
    pSpell->minimum_position    =   POS_STANDING;
    pSpell->next        =   NULL;
    pSpell->dbkey        =   top_dbkey_spell;  

    return pSpell;
}

void free_spell( SPELL *pSpell )
{
    if ( pSpell == NULL ) return;

    pSpell->next = spell_free;
    spell_free  = pSpell;

    free_string( pSpell->name       );
    return;
};


SPELL_BOOK *new_spell_book( void )
{
    SPELL_BOOK *pSpell;

    pSpell        =   alloc_perm( sizeof(*pSpell) );

    *pSpell             =   spell_book_zero;
    pSpell->next        =   NULL;
    pSpell->name        = &str_empty[0];

    return pSpell;
}

void free_spell_book( SPELL_BOOK *pSpell )
{
    if ( pSpell == NULL ) return;

    pSpell->next = spell_book_free;
    spell_book_free  = pSpell;
    return;
};






ZONE *new_zone( void )
{
    ZONE *pZone;
    char buf[MAX_INPUT_LENGTH];

    if ( zone_free == NULL )
    {
        pZone   =   alloc_perm( sizeof(*pZone) );
        top_zone++;
    }
    else
    {
        pZone       =   zone_free;
        zone_free   =   zone_free->next;
    }


    *pZone                  =   zone_zero;
    pZone->next             =   NULL;
    pZone->name             =   str_dup( "New zone" );
    pZone->repop            =   &str_empty[0];
    pZone->zone_flags       =   ZONE_ADDED;
    pZone->security         =   1;
    pZone->builders         =   str_dup( "None" );
    pZone->ldbkey            =   0;
    pZone->udbkey            =   0;
    pZone->age              =   0;
    pZone->nplayer          =   0;
    pZone->dbkey             =   top_zone-1;
    sprintf( buf, "zone-%s%d.zone", pZone->dbkey-1 < 10 ? "00" :
                                   pZone->dbkey-1 < 100 ? "0" : "",
                                   pZone->dbkey-1 );
    pZone->filename         =   str_dup( buf );

    return pZone;
}



void free_zone( ZONE *pZone )
{
    free_string( pZone->name );
    free_string( pZone->filename );
    free_string( pZone->repop );
    free_string( pZone->builders );

    pZone->next         =   zone_free->next;
    zone_free           =   pZone;
    return;
}



EXTRA_DESCR *new_extra_descr( void )
{
    EXTRA_DESCR *pExtra;

    if ( extra_descr_free == NULL )
    {
        pExtra              =   alloc_perm( sizeof(*pExtra) );
        top_ed++;
    }
    else
    {
        pExtra              =   extra_descr_free;
        extra_descr_free    =   extra_descr_free->next;
    }

    *pExtra                 =   extra_descr_zero;
    pExtra->keyword         =   NULL;
    pExtra->description     =   NULL;
    pExtra->next            =   NULL;

    return pExtra;
}



void free_extra_descr( EXTRA_DESCR *pExtra )
{
    free_string( pExtra->keyword );
    free_string( pExtra->description );

    pExtra->next        =   extra_descr_free;
    extra_descr_free    =   pExtra;
    return;
}



EXIT *new_exit( void )
{
    EXIT *pExit;

    if ( exit_free == NULL )
    {
        pExit           =   alloc_perm( sizeof(*pExit) );
        top_exit++;
    }
    else
    {
        pExit           =   exit_free;
        exit_free       =   exit_free->next;
    }

    *pExit              =   exit_zero;
    pExit->to_scene      =   NULL;
    pExit->next         =   NULL;
    pExit->dbkey         =   0;
    pExit->exit_flags    =   0;
    pExit->key          =   0;
    pExit->keyword      =   NULL;
    pExit->description  =   str_dup("");
    pExit->rs_flags     =   0;

    return pExit;
}



void free_exit( EXIT *pExit )
{
    free_string( pExit->keyword );
    free_string( pExit->description );

    pExit->next         =   exit_free;
    exit_free           =   pExit;
    return;
}


ATTACK *new_attack ( void )
{
    ATTACK *attack;

    if ( attack_free == NULL )
    {
        attack          = alloc_perm( sizeof(*attack) );
        top_attack++;
    }
    else
    {
        attack          =   attack_free;
        attack_free     =   attack_free->next;
    }

    *attack      = attack_zero;
    attack->idx  = 0;
    attack->dam1 = 0;
    attack->dam2 = 0;

    return attack;
}


void free_attack ( ATTACK *attack )
{
    attack->next     = attack_free;
    attack_free      = attack;
    return;
}
 

SPELL *new_spell_index( void )
{
    SPELL *pSpell;

    if ( spell_free == NULL )
    {
        pSpell           =   alloc_perm( sizeof(*pSpell) );
        top_spell++;
    }
    else
    {
        pSpell           =   spell_free;
        spell_free =   spell_free->next;
    }

    *pSpell                  = spell_zero;

    pSpell->name             =   &str_empty[0];
    pSpell->instances        = NULL;

    return pSpell;
}


SCENE *new_scene( void )
{
    SCENE *pScene;
    int door;

    if ( scene_free == NULL )
    {
        pScene           =   alloc_perm( sizeof(*pScene) );
        top_scene++;
    }
    else
    {
        pScene           =   scene_free;
        scene_free =   scene_free->next;
    }

    *pScene                  =   scene_zero;
    pScene->next             =   NULL;
    pScene->spawn_first      =   NULL;
    pScene->spawn_last       =   NULL;
    pScene->people           =   NULL;
    pScene->contents         =   NULL;
    pScene->extra_descr      =   NULL;
    pScene->zone             =   NULL;

    for ( door=0; door < MAX_DIR; door++ )
        pScene->exit[door]   =   NULL;

    pScene->owner            =   
    pScene->name             =   &str_empty[0];
    pScene->description      =   &str_empty[0];
    pScene->client           =   &str_empty[0];
    pScene->dbkey             =   0;
    pScene->scene_flags       =   0;
    pScene->light            =   0;
    pScene->move      =   0;
    pScene->terrain          =   0;

    return pScene;
}

void free_spell_index( SPELL *pSpell )
{
    INSTANCE *t, *trigger_next;

    free_string( pSpell->name );

    for ( t = pSpell->instances; t != NULL; t = trigger_next )
    {
    trigger_next     = t->next;
    free_instance( t );
    }

    pSpell->next     =   spell_free;
    spell_free =   pSpell;
    return;
}




void free_scene( SCENE *pScene )
{
    int door;
    EXTRA_DESCR *pExtra, *pExtra_next;
    SPAWN *pSpawn, *pSpawn_next;
    VARIABLE *va, *va_next;
    INSTANCE *t, *trigger_next;

    free_string( pScene->name );
    free_string( pScene->description );
    clear_events( pScene, TYPE_SCENE );

    for ( door = 0; door < MAX_DIR; door++ )
    {
        if ( pScene->exit[door] != NULL )
            free_exit( pScene->exit[door] );
    }

    for ( pExtra = pScene->extra_descr; pExtra != NULL; pExtra = pExtra_next )
    {
        pExtra_next = pExtra->next;
        free_extra_descr( pExtra );
    }

    for ( pSpawn = pScene->spawn_first; pSpawn != NULL; pSpawn = pSpawn_next )
    {
        pSpawn_next = pSpawn->next;
        free_spawn( pSpawn );
    }

    for ( t = pScene->instances; t != NULL; t = trigger_next )
    {
    trigger_next     = t->next;
    free_instance( t );
    }

    for ( va = pScene->globals;  va != NULL; va = va_next )
    {
        va_next = va->next;
        free_variable( va );
    }

    for ( door = 0;  door < MAX_KEY_HASH;  door++ )
    {
        SCENE *scene;

        for ( scene = scene_hash[door];  scene != NULL;  scene = scene->next )
        {
            if ( scene->next == pScene )
            scene->next = scene->next->next;
        }
    }

    pScene->next     =   scene_free;
    scene_free =   pScene;
    return;
}



BONUS *new_bonus( void )
{
    BONUS *pAf;

    if ( bonus_free == NULL )
    {
        pAf             =   alloc_perm( sizeof(*pAf) );
        top_bonus++;
    }
    else
    {
        pAf             =   bonus_free;
        bonus_free      =   bonus_free->next;
    }

    *pAf            =   bonus_zero;
    pAf->next       =   NULL;
    pAf->location   =   0;
    pAf->modifier   =   0;
    pAf->type       =   0;
    pAf->duration   =   0;
    pAf->bitvector  =   0;
    pAf->msg_off    =   &str_empty[0];

    return pAf;
}



void free_bonus( BONUS* pAf )
{
    pAf->next           = bonus_free;
    if ( pAf->msg_off ) {
      free_string( pAf->msg_off );
      pAf->msg_off = NULL;
    }
    bonus_free         = pAf;
    return;
}



PROP_TEMPLATE *new_prop_template( void )
{
    PROP_TEMPLATE *pProp;
    int value;

    if ( prop_template_free == NULL )
    {
        pProp           =   alloc_perm( sizeof(*pProp) );
        top_prop_template++;
    }
    else
    {
        pProp            =   prop_template_free;
        prop_template_free  =   prop_template_free->next;
    }

    *pProp               =   prop_template_zero;
    pProp->next          =   NULL;
    pProp->extra_descr   =   NULL;
    pProp->bonus      =   NULL;
    pProp->zone          =   NULL;
    pProp->name          =   str_dup( "unset" );
    pProp->short_descr   =   str_dup( "unset" );
    pProp->description   =   str_dup( "unset\n\r" );
    pProp->owner              =
    pProp->action_descr       = &str_empty[0];
    pProp->short_descr_plural = &str_empty[0];
    pProp->description_plural = &str_empty[0];
    pProp->real_description   = &str_empty[0];
    pProp->dbkey          =   0;
    pProp->size          =   SIZE_ANY;
    pProp->item_type     =   ITEM_TRASH;
    pProp->timer         =   0;
    pProp->extra_flags   =   0;
    pProp->wear_flags    =   0;
    pProp->count         =   0;
    pProp->weight        =   0;
    pProp->level         =   0;
    pProp->cost          =   0;
    for ( value=0; value<10; value++ )
        pProp->value[value]  =   0;

    return pProp;
}



void free_prop_template( PROP_TEMPLATE *pProp )
{
    EXTRA_DESCR *pExtra;
    BONUS *pAf;

    free_string( pProp->name );
    free_string( pProp->short_descr );
    free_string( pProp->short_descr_plural );
    free_string( pProp->description );
    free_string( pProp->description_plural );
    free_string( pProp->action_descr );

    for ( pAf = pProp->bonus; pAf != NULL; pAf = pAf->next )
    {
        free_bonus( pAf );
    }

    for ( pExtra = pProp->extra_descr; pExtra != NULL; pExtra = pExtra->next )
    {
        free_extra_descr( pExtra );
    }
    
    pProp->next              = prop_template_free;
    prop_template_free          = pProp;
    return;
}



SHOP *new_shop( void )
{
    SHOP *pShop;
    int buy;

    if ( shop_free == NULL )
    {
        pShop           =   alloc_perm( sizeof(*pShop) );
        top_shop++;
    }
    else
    {
        pShop           =   shop_free;
        shop_free       =   shop_free->next;
    }

    *pShop                = shop_zero;
    pShop->next           = NULL;
    pShop->keeper         = 0;
    pShop->profit_buy     = 100;
    pShop->profit_sell    = 100;
    pShop->open_hour      = 0;
    pShop->close_hour     = 23;

    pShop->comp_index     = 0;
    pShop->buy_index      = 0;
    pShop->sell_index     = 0;
    pShop->no_such_item   = str_dup( "I don't have that to sell you." );
    pShop->cmd_not_buy     = str_dup( "I am not interested in that item." );
    pShop->list_header    = str_dup( "" );
    pShop->hours_excuse   = str_dup( "Sorry, we're open from %s to %s." );

    for ( buy=0; buy<MAX_TRADE; buy++ )
        pShop->buy_type[buy]    =   ITEM_NONE;

    return pShop;
}



void free_shop( SHOP *pShop )
{
    free_string( pShop->no_such_item );
    free_string( pShop->cmd_not_buy );

    pShop->next = shop_free;
    shop_free   = pShop;
    return;
}



ACTOR_TEMPLATE *new_actor_template( void )
{
    ACTOR_TEMPLATE *pActor;
    int iAttack;

    if ( actor_template_free == NULL )
    {
        pActor           =   alloc_perm( sizeof(*pActor) );
        top_actor_template++;
    }
    else
    {
        pActor            =   actor_template_free;
        actor_template_free  =   actor_template_free->next;
    }

    *pActor               =   actor_template_zero;
    pActor->next          =   NULL;
    pActor->pShop         =   NULL;
    pActor->zone          =   NULL;
    pActor->owner         =   &str_empty[0];
    pActor->name          =   str_dup( "unset" );
    pActor->short_descr   =   str_dup( "unset" );
    pActor->long_descr    =   str_dup( "unset\n\r" );
    pActor->description   =   &str_empty[0];
    pActor->dbkey          =   0;
    pActor->exp           =   1;
    pActor->pSpells       =   NULL;
    pActor->count         =   0;
    pActor->sex           =   0;
    pActor->flag           =   ACTOR_NPC;
    pActor->timer         =   0;
    pActor->bonuses   =   0;
    pActor->money         =   0;
    pActor->size          =   SIZE_AVERAGE;
    for ( iAttack = 0;  iAttack < MAX_ATTACK;  iAttack++ )
    pActor->attacks[iAttack] = NULL;

/*
    for ( iAttack = 0;  iAttack < MAX_SKILL;  iAttack++ )
    pActor->learned[iAttack] = 0;
    
    pActor->learned[gsn_zengalli] = 100;
    pActor->learned[gsn_dodge]    = 25;
    pActor->learned[gsn_parry]    = 25;
 */

    return pActor;
}



void free_actor_template( ACTOR_TEMPLATE *pActor )
{
    free_string( pActor->name );
    free_string( pActor->short_descr );
    free_string( pActor->long_descr );
    free_string( pActor->description );
    clear_spell_book( pActor->pSpells );
 
    free_shop( pActor->pShop );

    pActor->next              = actor_template_free;
    actor_template_free          = pActor;
    return;
}



/*
 * Frees a variable for scripts.
 * This is used most often.
 */
void free_variable( VARIABLE *var )
{
    VARIABLE *pVar, *pVarNext;
    
    if ( var == NULL ) return; 

    free_string( var->name );
    if ( var->type == TYPE_STRING )
    free_string( var->value );

    var->name = str_dup( "" );
    var->next        = variable_free;
    variable_free    = var;    

    /*
     * Remove from master variable list
     */
    if ( var == mud_var_list ) mud_var_list = mud_var_list->next;
    else
    for ( pVar = mud_var_list;  pVar != NULL;  pVar = pVarNext ) {
      pVarNext = pVar->next;
      if ( pVarNext == var ) {
         pVar->next = pVarNext->next;
         return;
      }      
    }
    return;
}

VARIABLE *new_var( void )
{
    VARIABLE *var;

    if ( variable_free == NULL )
    {
        var      = alloc_perm( sizeof(*var) );
        top_variable++;
        *var          = variable_zero;
    }
    else
    {
        var           = variable_free;
        variable_free = variable_free->next;
        *var          = variable_zero;
    }

    var->type         = TYPE_STRING;
    var->value        = str_dup( "" );
    var->next         = NULL;
    return var;
}

/*
 * This one might cause a leak.  Not sure why.
 */
VARIABLE *new_variable( int type, void * value )
{
    VARIABLE *var;

    if ( variable_free == NULL )
    {
        var      = alloc_perm( sizeof(*var) );
        top_variable++;
        *var          = variable_zero;
    }
    else
    {
        var           = variable_free;
        variable_free = variable_free->next;
        *var          = variable_zero;
    }

    var->type         = type;
    var->value        = value;
    var->next         = NULL;
    return var;
}


EVENT *new_event( void )
{
    EVENT *pEvent;

    if ( event_free == NULL )
    {
        pEvent      = alloc_perm( sizeof(*pEvent) );
        top_event++;
    }
    else
    {
        pEvent      = event_free;
        event_free  = event_free->next;
    }

    *pEvent         = event_zero;
    pEvent->next    = NULL;
    pEvent->instance= NULL;
    return pEvent;
}


INSTANCE *new_instance( void )
{
    INSTANCE *t;


    if ( instance_free == NULL )
    {
        t      = alloc_perm( sizeof(*t) );
        top_instance++;
    }
    else
    {
        t      = instance_free;
        instance_free = instance_free->next;
    }

    *t         = instance_zero;
    t->next    = NULL;
    return t;
}



/*
 * Frees a trigger, used when a actor dies.
 */
void free_instance( INSTANCE *instance )
{
    VARIABLE *va;
    VARIABLE *va_next;

    if ( instance == NULL ) return;

    instance->next    = instance_free;
    instance_free     = instance;
    
    for ( va = instance->locals;  va != NULL; va = va_next )
    {
        va_next = va->next;
        free_variable( va );
    }
    instance->locals=NULL;

    return;
}

/*
 * Frees an event. 
 */
void free_event( EVENT *pEvent )
{
    if ( pEvent == NULL ) return;

    pEvent->next       = event_free;
    free_instance ( pEvent->instance );
    pEvent->instance   = NULL;
    event_free         = pEvent;

    return;
}

/*
 * Hardly ever used ( will be when you can use OLC to delete indexes )
 */
void free_script( SCRIPT *scr )
{
    if ( scr == NULL ) return;

    free_string( scr->name     );
    free_string( scr->commands );

    scr->next       = script_free;
    script_free     = scr;
    return;
}

SCRIPT *new_script( void )
{
    SCRIPT *script;

script_free = NULL;

    if ( script_free == NULL )
    {
        script    = alloc_perm( sizeof(*script) );
        top_script++;
    }
    else
    {
        script        = script_free;
        script_free   = script_free->next;
    }
    
    *script     = script_zero;
    script->commands = str_dup( "" );
    script->name     = str_dup( "new script" );
    return script;
}
    


/* END MEMORY RECYCLING STUFF */

/*
 * Read a letter from a file.
 */
char fread_letter( FILE *fp )
{
    char c;

    do
    {
    c = getc( fp );
    }
    while ( isspace(c) );

    return c;
}



/*
 * Read a number from a file.
 */
int fread_number( FILE *fp )
{
    int number;
    bool sign;
    char c;

    do
    {
    c = getc( fp );
    }
    while ( isspace(c) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
    {
	c = getc( fp );
    }
    else if ( c == '-' )
    {
	sign = TRUE;
	c = getc( fp );
    }

    if ( !isdigit(c) )
    {
	wtf_logf( "Fread_number: bad format." );
	exit( 1 );
    }

    while ( isdigit(c) )
    {
	number = number * 10 + c - '0';
	c      = getc( fp );
    }

    if ( sign )
	number = 0 - number;

    if ( c == '|' )
	number += fread_number( fp );
    else if ( c != ' ' )
	ungetc( c, fp );

    return number;
}



/*
 * Read and allocate space for a string from a file.
 * These strings are read-only and shared.
 * Strings are hashed:
 *   each string prepended with hash pointer to prev string,
 *   hash code is simply the string length.
 * This function takes 40% to 50% of boot-up time.
 */
char *fread_string( FILE *fp )
{
    char *plast;
    char c;

    plast = top_string + sizeof(char *);
    if ( plast > &string_space[MAX_STRING] )
    {
	wtf_logf( "Fread_string: MAX_STRING %d exceeded (%d read).", MAX_STRING, plast );
	exit( 1 );
    }

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	c = getc( fp );
    }
    while ( isspace(c) );

    if ( ( *plast++ = c ) == '~' )
        return &str_empty[0];

    for ( ;; )
    {
	/*
	 * Back off the char type lookup,
	 *   it was too dirty for portability.
	 *   -- Furey
	 */
	switch ( *plast = getc( fp ) )
	{
	default:
	    plast++;
	    break;

	case EOF:
	    wtf_logf( "Fread_string: EOF" );
	    exit( 1 );
	    break;

	case '\n':
	    plast++;
	    *plast++ = '\r';
	    break;

	case '\r':
	    break;

	case '~':
	    plast++;
	    {
		union
		{
		    char *	pc;
		    char	rgc[sizeof(char *)];
		} u1;
		int ic;
		int iHash;
		char *pHash;
		char *pHashPrev;
		char *pString;

		plast[-1] = '\0';
		iHash     = UMIN( MAX_KEY_HASH - 1, plast - 1 - top_string );
		for ( pHash = string_hash[iHash]; pHash; pHash = pHashPrev )
		{
		    for ( ic = 0; ic < sizeof(char *); ic++ )
			u1.rgc[ic] = pHash[ic];
		    pHashPrev = u1.pc;
		    pHash    += sizeof(char *);

		    if ( top_string[sizeof(char *)] == pHash[0]
		    &&   !strcmp( top_string+sizeof(char *)+1, pHash+1 ) )
			return pHash;
		}

		if ( fBootDb )
		{
		    pString		= top_string;
		    top_string		= plast;
		    u1.pc		= string_hash[iHash];
		    for ( ic = 0; ic < sizeof(char *); ic++ )
			pString[ic] = u1.rgc[ic];
		    string_hash[iHash]	= pString;

		    nAllocString += 1;
		    sAllocString += top_string - pString;
		    return pString + sizeof(char *);
		}
		else
		{
		    return str_dup( top_string + sizeof(char *) );
		}
	    }
	}
    }
}


char *fread_string_eol( FILE *fp )
{
    static bool actor_special[256-EOF];
    char *plast;
    char c;
 
    if ( actor_special[EOF-EOF] != TRUE )
    {
        actor_special[EOF -  EOF] = TRUE;
        actor_special['\n' - EOF] = TRUE;
        actor_special['\r' - EOF] = TRUE;
    }
 
    plast = top_string + sizeof(char *);
    if ( plast > &string_space[MAX_STRING] )
    {
        wtf_logf( "Fread_string_eol: MAX_STRING %d exceeded.", MAX_STRING );
        if ( fBootDb ) exit( 1 );
        else return &str_empty[0];
    }
 
    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
        c = getc( fp );
    }
    while ( isspace(c) );
 
    if ( ( *plast++ = c ) == '\n')
        return &str_empty[0];
 
    for ( ;; )
    {
        if ( !actor_special[ ( *plast++ = getc( fp ) ) - EOF ] )
            continue;
 
        switch ( plast[-1] )
        {
        default:
            break;
 
        case EOF:
        case '\n':  case '\r':
            {
                union
                {
                    char *      pc;
                    char        rgc[sizeof(char *)];
                } u1;
                int ic;
                int iHash;
                char *pHash;
                char *pHashPrev;
                char *pString;
 
                plast[-1] = '\0';
                iHash     = UMIN( MAX_KEY_HASH - 1, plast - 1 - top_string );
                for ( pHash = string_hash[iHash]; pHash; pHash = pHashPrev )
                {
                    for ( ic = 0; ic < sizeof(char *); ic++ )
                        u1.rgc[ic] = pHash[ic];
                    pHashPrev = u1.pc;
                    pHash    += sizeof(char *);
 
                    if ( top_string[sizeof(char *)] == pHash[0]
                    &&   !strcmp( top_string+sizeof(char *)+1, pHash+1 ) )
                        return pHash;
                }
 
                if ( fBootDb )
                {
                    pString             = top_string;
                    top_string          = plast;
                    u1.pc               = string_hash[iHash];
                    for ( ic = 0; ic < sizeof(char *); ic++ )
                        pString[ic] = u1.rgc[ic];
                    string_hash[iHash]  = pString;
 
                    nAllocString += 1;
                    sAllocString += top_string - pString;
                    return pString + sizeof(char *);
                }
                else
                {
                    return str_dup( top_string + sizeof(char *) );
                }
            }
        }
    }
}

/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE *fp )
{
    char c;

	c = getc( fp );
    while ( c != '\n' && c != '\r' ) c = getc( fp );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return;
}



/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp )
{
    static char word[MAX_INPUT_LENGTH];
    char *pword;
    char cEnd;

    do
    {
	cEnd = getc( fp );
    }
    while ( isspace( cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
    {
	pword   = word;
    }
    else
    {
	word[0] = cEnd;
	pword   = word+1;
	cEnd    = ' ';
    }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
	*pword = getc( fp );
	if ( cEnd == ' ' ? isspace(*pword) : *pword == cEnd )
	{
	    if ( cEnd == ' ' )
		ungetc( *pword, fp );
	    *pword = '\0';
	    return word;
	}
    }

    wtf_logf( "Fread_word: word too long." );
//    if ( fBootDb ) exit( 1 );
    return "";
}



/*
 * Allocate some ordinary memory, with the expectation of freeing it someday.
 */
void *alloc_mem( int sMem )
{
    void *pMem;
    int iList;

    pMem = malloc(sMem);
    memset(pMem,0,sMem);
    return pMem;

    for ( iList = 0; iList < MAX_MEM_LIST; iList++ )
    {
	if ( sMem <= rgSizeList[iList] )
	    break;
    }

    if ( iList == MAX_MEM_LIST )
    {
	wtf_logf( "Alloc_mem: size %d too large.", sMem );
	exit( 1 );
    }

    if ( rgFreeList[iList] == NULL )
    {
	pMem		  = alloc_perm( rgSizeList[iList] );
    }
    else
    {
	pMem              = rgFreeList[iList];
	rgFreeList[iList] = * ((void **) rgFreeList[iList]);
    }

    return pMem;
}



/*
 * (OLD) Free some memory.
 * Recycle it back onto the free list for blocks of that size.
 */
void ofree_mem( void *pMem, int sMem )
{
    int iList;

    for ( iList = 0; iList < MAX_MEM_LIST; iList++ )
    {
	if ( sMem <= rgSizeList[iList] )
	    break;
    }

    if ( iList == MAX_MEM_LIST )
    {
	wtf_logf( "Free_mem: size %d too large.", sMem );
	exit( 1 );
    }

    * ((void **) pMem) = rgFreeList[iList];
    rgFreeList[iList]  = pMem;

    return;
}


/*
 * Free some memory.
 * Recycle it back onto the free list for blocks of that size.
 */
void free_mem( void *pMem, int sMem )
{
    free(pMem); return;
}



/*
 * Allocate some permanent memory.
 * Permanent memory is never freed, pointers into it may be copied safely.
 */
void *alloc_perm( int sMem )
{
    static char *pMemPerm;
    static int iMemPerm;
    void *pMem;

    while ( sMem % sizeof(long) != 0 )
	sMem++;
    if ( sMem > MAX_PERM_BLOCK )
    {
	wtf_logf( "Alloc_perm: %d too large.", sMem );
	exit( 1 );
    }

    if ( pMemPerm == NULL || iMemPerm + sMem > MAX_PERM_BLOCK )
    {
	iMemPerm = 0;
	if ( ( pMemPerm = calloc( 1, MAX_PERM_BLOCK ) ) == NULL )
	{
	    perror( "Alloc_perm" );
	    exit( 1 );
	}
    }

    pMem        = pMemPerm + iMemPerm;
    iMemPerm   += sMem;
    nAllocPerm += 1;
    sAllocPerm += sMem;
    return pMem;
}



/*
 * Duplicate a string into dynamic memory.
 * Fread_strings are read-only and shared.
 */
char *str_dup( const char *str )
{
    char *str_new;

    
    if ( !str || str[0] == '\0' )
        return &str_empty[0];

    if ( str >= string_space && str < top_string )
	return (char *) str;

    num_str_dup++;
    str_new = alloc_mem( strlen(str) + 1 );
    strcpy( str_new, str );
    return str_new;
}



/*
 * Free a string.
 * Null is legal here to protect from crashes.
 * Read-only shared strings are not touched.
 */
void free_string( char *pstr )
{
    if ( pstr == NULL
    ||   pstr == &str_empty[0]
    || ( pstr >= string_space && pstr < top_string ) )
	return;

    ofree_mem( pstr, strlen(pstr) + 1 );
    return;
}




/*
 * This is the only thing that actually *NEEDS* to be here as in a CMD_FUN
 * -- Locke
 * Syntax:  memory
 */
void cmd_memory( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    PROP *prop;
    PLAYER *actor;
    int count_actor = 0, count_prop = 0, count_plr = 0;
    int total = 0;

    to_actor( CLRSCR, ch );

    one_argument( argument, buf );
    if ( !str_cmp( buf, "variables" ) ) {
        VARD *v;
        for( v = mud_var_list; v != NULL; v = v->next )
        { 
            sprintf( buf, "%s\n\r", v->name );
            to_actor( buf, ch );
        }
        return;
    }

    for ( prop = prop_list;  prop != NULL; prop = prop->next )
        count_prop++;

    for ( actor = actor_list; actor != NULL; actor = actor->next )
    {
        if ( NPC(actor) ) count_actor++;
                      else count_plr++;
    }

    total += sizeof( BONUS       ) * top_bonus;
    total += sizeof( ZONE        ) * top_zone;
    total += sizeof( EXTRA_DESCR ) * top_ed;
    total += sizeof( EXIT        ) * top_exit;
    total += sizeof( HELP        ) * top_help;
    total += sizeof( SPAWN       ) * top_spawn;
    total += sizeof( SPELL       ) * top_spell;
    total += sizeof( SKILL       ) * top_skill;
    total += sizeof( SHOP        ) * top_shop;
    total += sizeof( ACTOR_TEMPLATE ) * top_actor_template;
    total += sizeof( PROP_TEMPLATE  ) * top_prop_template;
    total += sizeof( SCENE ) * top_scene;
    total += sizeof( SCRIPT      ) * top_script;
    total += sizeof( VARIABLE    ) * top_variable;
    total += sizeof( INSTANCE    ) * top_instance;
    total += sizeof( PLAYER      ) * top_player;
    total += sizeof( PROP        ) * top_prop;
    total += sizeof( PLAYER      ) * top_userdata;
    total += sizeof( ATTACK      ) * top_attack;
    total += sizeof( CONNECTION  ) * top_connection;
    total += sizeof( NOTE        ) * top_note;
    total += sizeof( TERRAIN     ) * top_dbkey_terrain;

    sprintf( buf, "Type    %5s  %7s  [%5s] Top Vnums\n\r", "Num", "Bytes", "Each" );  to_actor( buf, ch );
    sprintf( buf, "Bonuses %5d  %7d  [%5d]\n\r",    (int) top_bonus,        (int) (sizeof( BONUS      ) * top_bonus),    (int)  sizeof( BONUS      ) ); to_actor( buf, ch );
    sprintf( buf, "Attacks %5d  %7d  [%5d]\n\r",    (int) top_attack,       (int) (sizeof( ATTACK      ) * top_attack),     (int) sizeof( ATTACK      ) ); to_actor( buf, ch );
    sprintf( buf, "Descs   %5d  %7d  [%5d]\n\r",    (int) top_connection,   (int) (sizeof( CONNECTION  ) * top_connection), (int) sizeof( CONNECTION  ) ); to_actor( buf, ch );
    sprintf( buf, "ExDes   %5d  %7d  [%5d]\n\r",    (int) top_ed,           (int) (sizeof( EXTRA_DESCR ) * top_ed),         (int) sizeof( EXTRA_DESCR ) ); to_actor( buf, ch );
    sprintf( buf, "Exits   %5d  %7d  [%5d]\n\r",    (int) top_exit,         (int) (sizeof( EXIT        ) * top_exit),       (int) sizeof( EXIT        ) ); to_actor( buf, ch );
    sprintf( buf, "Helps   %5d  %7d  [%5d]\n\r",    (int) top_help,         (int) (sizeof( HELP        ) * top_help),       (int) sizeof( HELP        ) ); to_actor( buf, ch );
    sprintf( buf, "Notes   %5d  %7d  [%5d]\n\r",    (int) top_note,         (int) (sizeof( NOTE        ) * top_note),       (int) sizeof( NOTE        ) ); to_actor( buf, ch );
    sprintf( buf, "Zones   %5d  %7d  [%5d]\n\r",    (int) top_zone,         (int) (sizeof( ZONE        ) * top_zone),       (int) sizeof( ZONE        ) ); to_actor( buf, ch );
    sprintf( buf, "MobIndx %5d  %7d  [%5d] %d\n\r", (int) top_actor_template, (int) ( sizeof( ACTOR_TEMPLATE   ) * top_actor_template), (int)  sizeof( ACTOR_TEMPLATE   ), top_dbkey_actor    ); to_actor( buf, ch );
    sprintf( buf, "ObjIndx %5d  %7d  [%5d] %d\n\r", (int) top_prop_template,  (int) ( sizeof( PROP_TEMPLATE   ) * top_prop_template), (int)  sizeof( PROP_TEMPLATE   ), top_dbkey_prop    ); to_actor( buf, ch );
    sprintf( buf, "Scenes  %5d  %7d  [%5d] %d\n\r", (int) top_scene,        (int) (sizeof( SCENE  ) * top_scene),      (int)  sizeof( SCENE  ), top_dbkey_scene   ); to_actor( buf, ch );
    sprintf( buf, "Scripts %5d  %7d  [%5d] %d\n\r", (int) top_script,       (int) (sizeof( SCRIPT      ) * top_script),   (int)   sizeof( SCRIPT      ), top_dbkey_script ); to_actor( buf, ch );
    sprintf( buf, "Cues    %5d  %7d  [%5d]\n\r",    (int) top_spawn,        (int) (sizeof( SPAWN       ) * top_spawn),     (int)  sizeof( SPAWN       ) ); to_actor( buf, ch );
    sprintf( buf, "Terrain %5d  %7d  [%5d]\n\r",    (int) top_dbkey_terrain, (int)( sizeof(TERRAIN   ) * top_dbkey_terrain), (int) sizeof(TERRAIN    ) ); to_actor( buf, ch );
    sprintf( buf, "Spells  %5d  %7d  [%5d] %d\n\r", (int) top_spell,        (int) (sizeof( SCENE  ) * top_spell),     (int)   sizeof( SPELL  ), top_dbkey_spell   ); to_actor( buf, ch );
    sprintf( buf, "Skills  %5d  %7d  [%5d] %d\n\r", (int) top_skill,        (int) (sizeof( SKILL  ) * top_skill),     (int)   sizeof( SKILL  ), top_dbkey_skill   ); to_actor( buf, ch );
    sprintf( buf, "Shops   %5d  %7d  [%5d]\n\r",    (int) top_shop,         (int) (sizeof( SHOP        ) * top_shop),     (int)   sizeof( SHOP        ) ); to_actor( buf, ch );
    sprintf( buf, "Trigs   %5d  %7d  [%5d]\n\r",    (int) top_instance,     (int) (sizeof( INSTANCE     ) * top_instance),  (int)   sizeof( INSTANCE     ) ); to_actor( buf, ch );
    sprintf( buf, "Vars    %5d  %7d  [%5d]\n\r",    (int) top_variable,     (int) (sizeof( VARIABLE    ) * top_variable), (int)   sizeof( VARIABLE    ) ); to_actor( buf, ch );
    sprintf( buf, "CharD   %5d  %7d  [%5d]\n\r",    (int) top_player,  (int) (sizeof( PLAYER        ) * top_player), (int)  sizeof( PLAYER        ) ); to_actor( buf, ch );
    sprintf( buf, "ObjD    %5d  %7d  [%5d]\n\r",    (int) top_prop,         (int) (sizeof( PROP         ) * top_prop),     (int)    sizeof( PROP         ) ); to_actor( buf, ch );
    sprintf( buf, "PcD     %5d  %7d  [%5d]\n\r",    (int) top_userdata,     (int) (sizeof( PLAYER          ) * top_userdata),   (int)   sizeof( PLAYER          ) ); to_actor( buf, ch );
    sprintf( buf, "Socials %5d  %7d  [%5d]\n\r",    (int) social_count,     (int) (sizeof( struct social_type ) * social_count), (int) sizeof( struct social_type ) ); to_actor( buf, ch );
    sprintf( buf, "Actors  %5d  %7d  [%5d]*\n\r",   (int) count_actor,     (int)  (count_actor * sizeof( PLAYER )),           (int)   sizeof( PLAYER ) );        to_actor( buf, ch );
    sprintf( buf, "Objects %5d  %7d  [%5d]*\n\r",   (int) count_prop,      (int)  (count_prop * sizeof( PROP  )),           (int)   sizeof( PROP  ) );        to_actor( buf, ch );
    sprintf( buf, "Players %5d  %7d  [%5d]*\n\r",   (int) count_plr,        (int) (count_plr * (sizeof( PLAYER ) + sizeof( PLAYER ))),(int) ( sizeof( PLAYER ) + sizeof( PLAYER )) ); to_actor( buf, ch );

    sprintf( buf, "Total type allocation is %d bytes.\n\r", total );
    to_actor( buf, ch );

    {
        VARD *v;
        int i=0;
        for ( v = mud_var_list; v != NULL; v=v->next_master_var ) i++;
        sprintf( buf, "Counted %d variables in the master variable list.\n\r", i );
        to_actor( buf, ch );
    }

    sprintf( buf, "%d calls to str_dup() since last boot,\n\r", num_str_dup );
    to_actor( buf, ch );

    sprintf( buf, "Booted with %d strings of %d/%d bytes.\n\r",
             nAllocString, sAllocString, MAX_STRING );
    to_actor( buf, ch );

    sprintf( buf, "Perms %d blocks of %d bytes.\n\rThere were %d guest characters created today.\n\r",
             nAllocPerm, sAllocPerm, guestnumber );
    to_actor( buf, ch );

    {
    extern int packet[60];
    extern int packet_count;
    extern int byte_count;
    
    int p;
    int tot = 0;
   
    for ( p = 0; p < 60; p++ )
    {
      if ( packet[p] == -1 ) continue;
      tot += packet[p];
    }
      
    sprintf( buf,
     "Average IP packets per minute is %d, total packets sent: %d (%d bytes)\n\r",
     tot / UMAX(p,1), packet_count, byte_count );
    to_actor( buf, ch );
    }

    return;
}






