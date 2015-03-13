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
#include <unistd.h>

#include <zlib.h>

#include "nimud.h"
#include "skills.h"
#include "script.h"
#include "defaults.h"
#include "net.h"

#if !defined(BSD)
extern	int	_filbuf		args( (FILE *) );
#endif



/*
 * Array of containers read for proper re-nesting of props.
 */
PROP_DATA *  rgObjNest   [MAX_NEST];



void       fwrite_char ( PLAYER_DATA *ch, FILE *fp );
void       fwrite_actor  ( PLAYER_DATA *ch, FILE *fp );
void       fwrite_prop  ( PROP_DATA *prop, FILE *fp, int iNest );
void       fread_prop   ( void *owner, int owner_type, FILE *fp );
PLAYER_DATA *fread_char  ( PLAYER_DATA *ch, FILE *fp );
PLAYER_DATA *fread_actor   ( PLAYER_DATA *ch, FILE *fp );

/*
 * Save a character and inventory.   ACTOR infrastructure removed.
 */
void save_actor_prop( PLAYER_DATA *ch )
{
    PLAYER_DATA *pet;
    char strsave[MAX_INPUT_LENGTH];
    FILE *fp;

    if (IS_NPC(ch))
    return;

    if ( PC(ch,level) == 0 )
	return;

    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

    PC(ch,save_time) = current_time;
    fclose( fpReserve );
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
    if ( ( fp = fopen( strsave, "w" ) ) == NULL )
    {
	bug( "Save_actor_prop: fopen", 0 );
	perror( strsave );
    return;
    }

    fwrite_char( ch, fp );

    if ( ch->carrying != NULL )
    fwrite_prop( ch->carrying, fp, 0 );

    for ( pet = actor_list; pet != NULL; pet = pet->next )
    {
        if ( ( pet->master == ch ) && IS_SET(pet->act, ACT_PET) )
        fwrite_actor( pet, fp );
    }

#if defined(IMC)
    imc_savechar( ch, fp );
#endif

    fprintf( fp, "#END\n\n" );
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}


/*
 * Write the char.
 */
void fwrite_char( PLAYER_DATA *ch, FILE *fp )
{
    BONUS_DATA *paf;

    /*
     * Guest character support.
     */
    if ( !str_cmp(ch->name, "guest") || !str_cmp(ch->name, "Guest") ) return;

    if ( !str_cmp( ch->short_descr, "(null)" ) ) 
    { free_string( ch->short_descr ); ch->short_descr = str_dup(""); }
    if ( !str_cmp( ch->long_descr, "(null)" ) ) 
    { free_string( ch->long_descr ); ch->long_descr = str_dup(""); }
    
    fprintf( fp, "#PLAYER\n"     );
    fprintf( fp, "N %s~\n",   ch->name        );
    fprintf( fp, "S %s~\n",   ch->short_descr );
    fprintf( fp, "L %s~\n"  , fix_string( ch->long_descr  ) );
    fprintf( fp, "D\n%s~\n",  fix_string( ch->description ) );
    fprintf( fp, "Sx %d\n",   ch->sex                 );
    fprintf( fp, "Sz %d\n",   ch->size        );
    fprintf( fp, "Race %d\n", ch->race        );
    fprintf( fp, "Fi %d\n",   ch->fmode       );
    fprintf( fp, "Credits %d\n", ch->credits       );
    fprintf( fp, "Bucks %d\n",   ch->bucks         );

#if defined(NEVER)
    { INSTANCE_DATA *pInstance;
      for ( pInstance = ch->instances;  pInstance != NULL;
            pInstance = pInstance->next )
/* only for always running scripts, dont save completed ones */
      if ( !MTD(pInstance->location) ) 
      fprintf( fp, "Script %d", pInstance->script->vnum );
    }
#endif

    { ALIAS_DATA *alias;  for ( alias = PC(ch,aliases); alias!=NULL; alias=alias->next ) fprintf( fp, "alias %s~ %s~\n", alias->name, alias->exp ); }

    fprintf( fp, "Logins %d\n", PC(ch,logins) );
    fprintf( fp, "Lv %d\n",   PC(ch,level)    );
    fprintf( fp, "App %d\n",  PC(ch,app_time) );
    fprintf( fp, "HeroP %d\n",  PC(ch,stat_points) );
    fprintf( fp, "Bou %d\n",  ch->bounty      );
    fprintf( fp, "O %d\n",    ch->owed        );
    fprintf( fp, "Flag %s~\n",  PC(ch,flag)        );
    fprintf( fp, "Exp %d\n",    ch->exp       );
    fprintf( fp, "ExpLev %d\n", ch->exp_level );
    fprintf( fp, "Sc %d\n",   PC(ch,security) );
    fprintf( fp, "Wi %d\n",   PC(ch,wizinvis) );
    fprintf( fp, "Death %d\n", PC(ch,death) );
    fprintf( fp, "Home %d\n",  PC(ch,home) );
    fprintf( fp, "Map %d\n", PC(ch,mapsize) );
    fprintf( fp, "History\n%s~\n", PC(ch,history) ); 
    fprintf( fp, "Pl %d\n",
                 PC(ch,played) + (int) (current_time - PC(ch,logon) ) );
    fprintf( fp, "R %d\n", ch->in_scene->vnum );

        {
                   int count;

                      for ( count = 0;  count < MAX_COLORS; count++ )
	    fprintf( fp, "Color %d %d\n", count, PC(ch, colors[count]) );
        }

    fprintf( fp, "BD %d %d %d\n",   PC(ch,birth_day),
                 PC(ch,birth_month), PC(ch,birth_year) );
    fprintf( fp, "H %d %d\n", ch->hit, ch->move );
    fprintf( fp, "MK %d %d\n", ch->mana, ch->karma );

        {  SPELL_BOOK_DATA *pSpell;
           for ( pSpell = ch->spells; pSpell != NULL;  pSpell = pSpell->next )
                fprintf( fp, "Spell %d %s~\n", pSpell->vnum, pSpell->name);
        }

    fprintf( fp, "A %d A2 %d\n", ch->act, ch->act2 );
    fprintf( fp, "AB %d\n",   ch->bonuses         );
    if ( !MTD(PC(ch,denial)) )
      fprintf( fp, "Denial %s~\n",  fix_string( PC(ch,denial) ) );
    fprintf( fp, "Po %d\n",    /* Bug fix from Alander */
        ch->position == POS_FIGHTING ? POS_STANDING : ch->position );
    fprintf( fp, "ST %d\n",   ch->saving_throw        );
    fprintf( fp, "Hi %d\n",   ch->hitroll             );
    fprintf( fp, "Da %d\n",   ch->damroll             );
    fprintf( fp, "AC %d\n",   ch->armor               );
    fprintf( fp, "De %d\n",   PC(ch,deaf)                );

    if ( IS_NPC(ch) )
    {
    fprintf( fp, "V %d\n",   ch->pIndexData->vnum    );
    }
    else
    {
    fprintf( fp, "P %s~\n",      PC(ch,pwd)         );
    fprintf( fp, "Pr %s~\n",     ch->prompt         );
    fprintf( fp, "Pg %d\n",      ch->pagelen        );
    fprintf( fp, "Title %s~\n",   PC(ch,title)       );

    if ( IS_IMMORTAL(ch) )
    {
    fprintf( fp, "Bi  %s~\n",    PC(ch,bamfin)      );
    fprintf( fp, "Bo %s~\n",     PC(ch,bamfout)     );
    fprintf( fp, "Co %s~\n",     PC(ch,constellation) );
    }
    fprintf( fp, "Em %s~\n",     PC(ch,email)      );
    fprintf( fp, "K %s~\n",      ch->keywords   );
    fprintf( fp, "AP %d %d %d %d %d\n",
        ch->perm_str,
        ch->perm_int,
        ch->perm_wis,
        ch->perm_dex,
        ch->perm_con );

    fprintf( fp, "AM %d %d %d %d %d\n",
        ch->mod_str,
        ch->mod_int,
        ch->mod_wis,
        ch->mod_dex,
        ch->mod_con );

    fprintf( fp, "C %d %d %d\n",
        PC(ch,condition[0]),
        PC(ch,condition[1]),
        PC(ch,condition[2]) );
 
        {
        SKILL_DATA *pSkill;
	for ( pSkill = ch->learned; pSkill != NULL; pSkill = pSkill->next )
	{
            if ( pSkill->skill_level > 0 )
	    {
                fprintf( fp, "Sk %d '%s' %d %d\n",
                    pSkill->skill_level, pSkill->name,
                    pSkill->teacher, pSkill->skill_time );
	    }
	}
        }
    }

    for ( paf = ch->bonus; paf != NULL; paf = paf->next )
    {
	/* Thx Alander */
	if ( paf->type < 0 )
	    continue;

    fprintf( fp, "Af '%s' %d %d %d %d\n",
	    skill_name(paf->type),
	    paf->duration,
	    paf->modifier,
	    paf->location,
        paf->bitvector );
    }

    fprintf( fp, "END\n\n" );
    return;
}

/*
 * Write the actor.
 */
void fwrite_actor( PLAYER_DATA *ch, FILE *fp )
{
    BONUS_DATA *paf;
    
    fprintf( fp, "#ACTOR\n"                          );
    fprintf( fp, "V %d\n",    ch->pIndexData->vnum    );
    if ( ch->name != NULL )
    fprintf( fp, "N %s~\n", ch->name                  );
    if ( ch->short_descr != NULL )
    fprintf( fp, "S %s~\n", ch->short_descr           );
    if ( ch->long_descr != NULL )
    fprintf( fp, "L %s~\n",   fix_string( ch->long_descr  ) );
    if ( ch->description != NULL )
    fprintf( fp, "D\n%s~\n",  fix_string( ch->description ) );
    fprintf( fp, "Sx %d\n",   ch->sex                 );
    fprintf( fp, "Sz %d\n",   ch->size                );
    fprintf( fp, "R %d\n",    ch->in_scene->vnum       );
    fprintf( fp, "Owner %s~\n",   ch->owner               );
    fprintf( fp, "T %d\n",    ch->timer               );

/*
    { INSTANCE_DATA *pInstance;
      for ( pInstance = ch->instances;  pInstance != NULL;
            pInstance = pInstance->next )
      fprintf( fp, "Script %d", pInstance->script->vnum );
    }
 */

    fprintf( fp, "H %d %d\n", ch->hit, ch->move       );
    fprintf( fp, "A %d A2 %d\n", ch->act, ch->act2    );
    fprintf( fp, "AB %d\n",   ch->bonuses         );
    fprintf( fp, "P %d\n",    /* Bug fix from Alander */
        ch->position == POS_FIGHTING ? POS_STANDING : ch->position );
    fprintf( fp, "ST %d\n",   ch->saving_throw        );
    fprintf( fp, "Hi %d\n",   ch->hitroll             );
    fprintf( fp, "Da %d\n",   ch->damroll             );
    fprintf( fp, "AC %d\n",   ch->armor               );
    fprintf( fp, "AP %d %d %d %d %d\n",
        ch->perm_str,
        ch->perm_int,
        ch->perm_wis,
        ch->perm_dex,
        ch->perm_con );

    fprintf( fp, "AM %d %d %d %d %d\n",
        ch->mod_str,
        ch->mod_int,
        ch->mod_wis,
        ch->mod_dex,
        ch->mod_con );

    for ( paf = ch->bonus; paf != NULL; paf = paf->next )
    {
        /* Thx Alander */
        if ( paf->type < 0 )
            continue;

        fprintf( fp, "Af '%s' %d %d %d %d\n",
            skill_name(paf->type),
            paf->duration,
            paf->modifier,
            paf->location,
            paf->bitvector );
    }

    fprintf( fp, "END\n\n" );

    if ( ch->carrying != NULL )
    fwrite_prop ( ch->carrying, fp, 0 );

    return;
}


/*
 * Write a prop list and each prop's contents.
 */
void fwrite_prop( PROP_DATA *prop, FILE *fp, int iNest )
{
    EXTRA_DESCR_DATA *ed;
    BONUS_DATA *paf;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( prop->next_content != NULL )
    fwrite_prop( prop->next_content, fp, iNest );

    /*
     * Castrate storage zones.
     */
    if ( IS_SET( prop->extra_flags, ITEM_NOSAVE ) )
	return;

    fprintf( fp, "\n#PROP\n" );
    fprintf( fp, "Nest %d\n",   iNest                );
    fprintf( fp, "V %d\n",   prop->pIndexData->vnum   );
    if ( prop->name )
    fprintf( fp, "N %s~\n",  prop->name               );
    if ( prop->short_descr )     
    fprintf( fp, "S %s~\n",  prop->short_descr        );
    if ( prop->short_descr_plural )
    fprintf( fp, "SP %s~\n", prop->short_descr_plural );
    if ( prop->description )
    fprintf( fp, "D\n%s~\n", prop->description        );
    if ( prop->action_descr )     
    fprintf( fp, "A %s~\n",  prop->action_descr       );
    fprintf( fp, "E %d\n",   prop->extra_flags        );
    fprintf( fp, "W %d\n",   prop->wear_flags         );
    fprintf( fp, "WL %d\n",  prop->wear_loc           );
    fprintf( fp, "I %d\n",   prop->item_type          );
    fprintf( fp, "Wt %d\n",  prop->weight             );
    fprintf( fp, "L %d\n",   prop->level              );
    fprintf( fp, "Sz %d\n",  prop->size               );

/*
    { INSTANCE_DATA *pInstance;
      for ( pInstance = prop->instances;  pInstance != NULL;
            pInstance = pInstance->next )
      fprintf( fp, "Script %d", pInstance->script->vnum );
    }
 */

    if ( prop->timer != 0 )
    fprintf( fp, "T %d\n",   prop->timer              );
    fprintf( fp, "C %d\n",   prop->cost               );
    fprintf( fp, "Va %d %d %d %d\n",
             prop->value[0],
             prop->value[1],
             prop->value[2],
             prop->value[3] );

    switch ( prop->item_type )
    {
        case ITEM_POTION:
        case ITEM_SCROLL:
        if ( prop->value[1] > 0 )
        {  fprintf( fp, "Sp 1 '%s'\n", skill_name(prop->value[1]) ); }
        if ( prop->value[2] > 0 )
        {  fprintf( fp, "Sp 2 '%s'\n", skill_name(prop->value[2]) ); }
        if ( prop->value[3] > 0 )
        {  fprintf( fp, "Sp 3 '%s'\n", skill_name(prop->value[3]) ); }
        break;

        case ITEM_PILL:
        if ( prop->value[3] > 0 )
        {  fprintf( fp, "Sp 3 '%s'\n", skill_name(prop->value[3]) ); }
        break;
    }

    for ( paf = prop->bonus; paf != NULL; paf = paf->next )
    {
    fprintf( fp, "Af %d %d %d %d\n",
	    paf->duration,
	    paf->modifier,
	    paf->location,
        paf->bitvector );
    }

    for ( ed = prop->extra_descr; ed != NULL; ed = ed->next )
    { fprintf( fp, "Ex %s~\n%s~\n", ed->keyword, ed->description ); }

    fprintf( fp, "END\n\n" );

    if ( prop->contains != NULL )
    fwrite_prop( prop->contains, fp, iNest + 1 );

    return;
}



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_actor_prop( CONNECTION_DATA *d, char *name )
{
    char strsave[MAX_INPUT_LENGTH];
    PLAYER_DATA *ch, *actor;
    FILE *fp;
    bool found;

    ch = new_player_data( );

    d->character			= ch;
    ch->desc				= d;
    free_string( ch->name );
    ch->name				= str_dup( name );

    actor = ch;

#if defined(IMC)
    imc_initchar( ch );
#endif

    found = FALSE;
    fclose( fpReserve );
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( name ) );
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

        if ( letter != '#' )
	    {
		bug( "Load_actor_prop: # not found.", 0 );
		break;
	    }

        word = fread_word( fp );
             if ( !str_cmp( word, "PLAYER" ) )  actor = fread_char ( ch, fp );
        else if ( !str_cmp( word, "ACTOR" ) )
        {
            actor = fread_actor  ( ch, fp );
            if ( actor != NULL && actor != ch )
            {
                add_follower( actor, ch );
                actor_to_scene( actor, ch->in_scene );
                actor->next = actor_list;
                actor_list = actor;

        script_update( actor, TYPE_ACTOR, TRIG_BORN, NULL, NULL, NULL, NULL );

            }
            else actor = ch;
        }
        else if ( !str_cmp( word, "OBJECT" ) 
               || !str_cmp( word, "PROP"   ) ) fread_prop  ( actor, TYPE_ACTOR, fp );
        else if ( !str_cmp( word, "END"    ) ) break;
	    else
	    {
		bug( "Load_actor_prop: bad section.", 0 );
		break;
	    }
	}
	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return found;
}




/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                \
				if ( !str_cmp( word, literal ) )	\
                {                                   \
                    field  = value;                 \
                    fMatch = TRUE;                  \
                    break;                          \
                }

PLAYER_DATA *fread_char( PLAYER_DATA *ch, FILE *fp )
{
    char *word;
    bool fMatch;


    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
        fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
        KEY( "A",  ch->act,            fread_number( fp ) );
        KEY( "A2",  ch->act2,           fread_number( fp ) );
        KEY( "AB",  ch->bonuses,    fread_number( fp ) );
        KEY( "AC",  ch->armor,          fread_number( fp ) );

        if ( !str_cmp( word, "Alias" ) ) {
          ALIAS_DATA *alias=new_alias_data();  alias->next=PC(ch,aliases); PC(ch,aliases)=alias; 
          alias->name = fread_string(fp); alias->exp=fread_string(fp);
          fMatch=TRUE; break;
        }

        if ( !str_cmp( word, "Af" ) )
	    {
		BONUS_DATA *paf;
        SKILL_DATA *pSkill;

        paf = new_bonus( );
        pSkill = skill_lookup( fread_word( fp ) );
        if ( !pSkill )
        bug( "Fread_char: unknown skill.", 0 );
        else
        paf->type = pSkill->vnum;
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->next	= ch->bonus;
		ch->bonus	= paf;
		fMatch = TRUE;
		break;
	    }

        if ( !str_cmp( word, "AM"  ) )
	    {
            ch->mod_str  = fread_number( fp );
            ch->mod_int  = fread_number( fp );
            ch->mod_wis  = fread_number( fp );
            ch->mod_dex  = fread_number( fp );
            ch->mod_con  = fread_number( fp );
            fMatch = TRUE;
            break;
	    }

        if ( !str_cmp( word, "AP" ) )
	    {
            ch->perm_str = fread_number( fp );
            ch->perm_int = fread_number( fp );
            ch->perm_wis = fread_number( fp );
            ch->perm_dex = fread_number( fp );
            ch->perm_con = fread_number( fp );
            fMatch = TRUE;
            break;
	    }

        if ( !str_cmp( word, "App" ) )
            {
            PC(ch,app_time)    = fread_number( fp );
            fMatch = TRUE;
            break;
            }

	    break;

	case 'B':
        if ( !str_cmp( word, "BD" ) )
        {
            PC(ch,birth_day)   = fread_number( fp );
            PC(ch,birth_month) = fread_number( fp );
            PC(ch,birth_year)  = fread_number( fp );
            fMatch = TRUE;
        }
        KEY( "Bi",  PC(ch,bamfin),     fread_string( fp ) );
        KEY( "Bo", PC(ch,bamfout),    fread_string( fp ) );
        KEY( "Bou",  ch->bounty,        fread_number( fp ) );
        KEY( "Bucks",   ch->bucks,           fread_number( fp ) );
	    break;

        case 'T':
        if ( !str_cmp( word, "Title" ) ) {
                free_string( PC(ch,title) );
                PC(ch,title) = fread_string( fp );
                fMatch = TRUE;
         }
        break;   

	case 'C':        
        if ( !str_cmp( word, "C" ) )
	    {
                PC(ch,condition[0]) = fread_number( fp );
                PC(ch,condition[1]) = fread_number( fp );
                PC(ch,condition[2]) = fread_number( fp );
        fMatch = TRUE;
		break;
	    }

        if ( !str_cmp( word, "Color" ) )
            {  int count = fread_number( fp );   PC(ch,colors[count]) = 
fread_number(fp); fMatch = TRUE; break; }

        if ( !str_cmp( word, "Co" ) )
	    {
        free_string( PC(ch,constellation) );
        PC(ch,constellation) = fread_string( fp );
		fMatch = TRUE;
		break;
	    }
        KEY( "Credits", ch->credits,         fread_number( fp ) );

	case 'D':
        KEY( "Da",     ch->damroll,          fread_number( fp ) );
        KEY( "De",    PC(ch,deaf),           fread_number( fp ) );
	KEY( "Death",    PC(ch,death),       fread_number( fp ) );
        KEY( "D",    ch->description,        fread_string( fp ) );
        KEY( "Denial",  PC(ch,denial),          fread_string( fp ) );
	    break;

	case 'E':
        KEY( "Exp",  ch->exp,           fread_number( fp ) );    
        KEY( "ExpLev", ch->exp_level,   fread_number( fp ) );
        KEY( "Em",   PC(ch,email),      fread_string( fp ) );
        if ( !str_cmp( word, "END" ) )
        return ch;
	    break;

    case 'F':
         KEY( "Flag", PC(ch,flag),  fread_string( fp ) );
         KEY( "Fi", ch->fmode,            fread_number( fp ) );
        break;

    case 'O':
        KEY( "O",  ch->owed,        fread_number( fp ) );
        break;

    case 'M':
        KEY( "Map", PC(ch,mapsize), fread_number( fp ) );
        if ( !str_cmp( word, "MK" ) )
           {
               ch->mana  = fread_number( fp );
               ch->karma = fread_number( fp );
               fMatch = TRUE;
               break;
           }
           break;

    case 'H':
        KEY( "HeroP",   PC(ch,stat_points),    fread_number(fp) );
	KEY( "Home",    PC(ch,home),            fread_number( fp ) );
        KEY( "Hi",   ch->hitroll,              fread_number( fp ) );
        KEY( "History", PC(ch,history),        fread_string( fp ) );                                      
        if ( !str_cmp( word, "H" ) )
	    {
		ch->hit		= fread_number( fp );
		ch->move	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    break;

    case 'K':
        KEY( "K",   ch->keywords,      fread_string( fp ) );
       break;

	case 'L':
        KEY( "Logins",   PC(ch,logins),            fread_number( fp ) );
        KEY( "Lv",       PC(ch,level),              fread_number( fp ) );
        KEY( "L",       ch->long_descr,            fread_string( fp ) );
	    break;

	case 'N':
        if ( !str_cmp( word, "N" ) )
	    {
		/*
		 * Name already set externally.
		 */
		fread_to_eol( fp );
		fMatch = TRUE;
		break;
	    }
	    break;

        case 'P':
        KEY( "Pr",      ch->prompt,          fread_string( fp ) );
        KEY( "P",       PC(ch,pwd),          fread_string( fp ) );
        KEY( "Pl",      PC(ch,played),       fread_number( fp ) );
        KEY( "Pg",      ch->pagelen,         fread_number( fp ) );
        KEY( "Po",      ch->position,        fread_number( fp ) );
	    break;

	case 'R':
        KEY( "Ra",        ch->race,               fread_number(fp)+1);
        /* SPAWNS RACE */
        KEY( "Race",      ch->race,               fread_number( fp ) );

        if ( !str_cmp( word, "R" ) )
	    {
		ch->in_scene = get_scene_index( fread_number( fp ) );
		if ( ch->in_scene == NULL )
             ch->in_scene = get_scene_index( SCENE_VNUM_TEMPLATE );
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'S':
        KEY( "ST", ch->saving_throw,      fread_number( fp ) );
        KEY( "Sx", ch->sex,               fread_number( fp ) );
        KEY( "Sz", ch->size,              fread_number( fp ) );
        KEY( "S",  ch->short_descr,       fread_string( fp ) );
        KEY( "Sc", PC(ch,security),       fread_number( fp ) );

/*
 * This was removed because it doubles the scripts on a given object.
 * This was originally added to provide a way for an object's
 * scripts to be dynamic.  This may want to be rethought.
 */
#if defined(NEVER)
        if ( !str_cmp( word, "Script" ) ) {
           INSTANCE_DATA *pInstance = new_instance();
           pInstance->script = get_script_index( fread_number( fp ) );
           if ( !pInstance->script ) { free_instance(pInstance); break; }
           pInstance->next = ch->instances;
           ch->instances = pInstance;  
           /*
            * Note: scripts must be 'triggered' here
            */      
        }
#endif
 
        if ( !str_cmp( word, "Spell" ) )
            {
                 SPELL_BOOK_DATA *pSpell;

                 pSpell = new_spell_book_data( );
                 pSpell->vnum = fread_number( fp );
                 pSpell->name = fread_string( fp );
                 pSpell->next = ch->spells;
                 ch->spells = pSpell;
                 fMatch = TRUE;
                 break;
            }

        if ( !str_cmp( word, "Sk" ) )
	    {
		SKILL_DATA *pSkill;
		int value;
                int teacher;
                int skill_time;
                char *w;

                value   = fread_number( fp );
                w= fread_word( fp );
                pSkill = skill_lookup( w );
                teacher = fread_number( fp );
                skill_time = fread_number( fp );

                if ( !pSkill )
                bugs( "Fread_char: unknown skill '%s'", w );
                else
                {
                    pSkill=update_skill(ch,pSkill->vnum,value);
                    pSkill->skill_time = skill_time;
                    pSkill->teacher = teacher;
                }
		fMatch = TRUE;
	    }

	    break;

	case 'V':
        if ( !str_cmp( word, "V" ) )
	    {
		ch->pIndexData = get_actor_index( fread_number( fp ) );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	KEY( "Wi",    PC(ch,wizinvis),            fread_number( fp ) );
	    break;
	}

	if ( !fMatch )
	{
        sprintf( log_buf, "Fread_char: no match (%s).", word );
        bug( log_buf, 0 );
        if ( !feof( fp ) ) fread_to_eol( fp );
	}
    }
}



PLAYER_DATA *fread_actor ( PLAYER_DATA *ch, FILE *fp )
{
    char *word;
    bool fMatch;
    bool fVnum = FALSE;

    ch = new_player_data( );
    free_user_data( ch->userdata );
    ch->userdata = NULL;

    ch->pIndexData     = NULL;

    ch->name           = NULL;
    ch->short_descr    = NULL;
    ch->long_descr     = NULL;
    ch->description    = NULL;
    ch->position       = POS_STANDING;

    ch->instances      = NULL;
    ch->current        = NULL;

    ch->prompt     = str_dup( "Mob(%h %m): " );
    ch->armor      = 100;
    ch->hit        = MAXHIT(ch);
    ch->move       = MAXMOVE(ch);

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
        fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
        KEY( "A",   ch->act,            fread_number( fp ) );
        KEY( "A2",  ch->act2,           fread_number( fp ) );
        KEY( "AB",  ch->bonuses,    fread_number( fp ) );
        KEY( "AC",  ch->armor,          fread_number( fp ) );

        if ( !str_cmp( word, "Af" ) || !str_cmp( word, "Af" ) )
	    {
		BONUS_DATA *paf;
        SKILL_DATA *pSkill;

        paf = new_bonus( );
        pSkill = skill_lookup( fread_word( fp ) );
        if ( !pSkill )
        bug( "Fread_actor: unknown skill.", 0 );
        else
        paf->type = pSkill->vnum;
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->next	= ch->bonus;
		ch->bonus	= paf;
		fMatch = TRUE;
		break;
	    }

        if ( !str_cmp( word, "AM"  ) )
	    {
                ch->mod_str  = fread_number( fp );
                ch->mod_int  = fread_number( fp );
                ch->mod_wis  = fread_number( fp );
                ch->mod_dex  = fread_number( fp );
                ch->mod_con  = fread_number( fp );
		fMatch = TRUE;
		break;
	    }

        if ( !str_cmp( word, "AP" ) )
	    {
                ch->perm_str = fread_number( fp );
                ch->perm_int = fread_number( fp );
                ch->perm_wis = fread_number( fp );
                ch->perm_dex = fread_number( fp );
                ch->perm_con = fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'D':
        KEY( "Da",     ch->damroll,          fread_number( fp ) );
        KEY( "D",    ch->description,        fread_string( fp ) );
	    break;

	case 'E':
        if ( !str_cmp( word, "END" ) )
        {
            if ( !fVnum )
            {
                bug( "Fread_actor: Incomplete actor.", 0 );
                free_char( ch );
                return NULL;
            }


    if ( ch->pIndexData->instances != NULL )
    {
    INSTANCE_DATA *trig, *pTrig;

    for ( pTrig = ch->pIndexData->instances;  pTrig != NULL;  pTrig = pTrig->next )
    {
        trig               = new_instance( );

        trig->script       = pTrig->script;
        trig->location     = NULL;

        trig->next       = ch->instances;
        ch->instances    = trig;
    }
    }


            return ch;
        }
	    break;

	case 'H':
        KEY( "Hi",   ch->hitroll,              fread_number( fp ) );
                                              
        if ( !str_cmp( word, "H" ) )
	    {
		ch->hit		= fread_number( fp );
		ch->move	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    break;

    case 'K':
        KEY( "K",       ch->keywords,      fread_string( fp ) );
        break;

	case 'L':
        KEY( "L",       ch->long_descr,            fread_string( fp ) );
	    break;

	case 'N':
        KEY( "N",       ch->name,            fread_string( fp ) );
	    break;

    case 'P':
        KEY( "P",       ch->position,        fread_number( fp ) );
	    break;

	case 'R':
        KEY( "Ra",        ch->race,               fread_number( fp )+1 );
        KEY( "Race",      ch->race,               fread_number( fp ) );

        if ( !str_cmp( word, "R" ) )
	    {
		ch->in_scene = get_scene_index( fread_number( fp ) );
		if ( ch->in_scene == NULL )
        ch->in_scene = get_scene_index( SCENE_VNUM_TEMPLATE );

		fMatch = TRUE;
		break;
	    }

	    break;
     case 'O':
        KEY( "Owner",  ch->owner,      fread_string( fp ) );
      break;
    case 'T':
        KEY( "T",  ch->timer,             fread_number( fp ) );
        break;

	case 'S':
        KEY( "ST", ch->saving_throw,      fread_number( fp ) );
        KEY( "Sx", ch->sex,               fread_number( fp ) );
        KEY( "Sz", ch->size,              fread_number( fp ) );
        KEY( "S",  ch->short_descr,       fread_string( fp ) );

#if defined(NEVER)
        if ( !str_cmp( word, "Script" ) ) {
           INSTANCE_DATA *pInstance = new_instance();
           pInstance->script = get_script_index( fread_number( fp ) );
           if ( !pInstance->script ) { free_instance(pInstance); break; }
           pInstance->next = ch->instances;
           ch->instances = pInstance;  
           fMatch=TRUE;
           /*
            * Note: scripts must be 'triggered' after here
            */      
        }
#endif

	    break;
	case 'W':
	    break;

	case 'V':
        if ( !str_cmp( word, "V" ) )
	    {
            ch->pIndexData = get_actor_index( fread_number( fp ) );
            if ( ch->pIndexData == NULL )
            ch->pIndexData = get_actor_index( ACTOR_VNUM_TEMPLATE );

            if ( ch->pIndexData->instances != NULL )
            {
                INSTANCE_DATA *trig, *pTrig;

                for ( pTrig = ch->pIndexData->instances;
                      pTrig != NULL;  pTrig = pTrig->next )
                {
                    trig               = new_instance( );

                    trig->script       = pTrig->script;
                    trig->location     = NULL;

                    trig->next       = ch->instances;
                    ch->instances    = trig;
                }
            }

            fMatch = TRUE;
            fVnum  = TRUE;
            break;
	    }
	    break;

	}

	if ( !fMatch )
	{
        sprintf( log_buf, "Fread_actor: no match (%s).", word );
        bug( log_buf, 0 );
        if ( !feof( fp ) ) fread_to_eol( fp );
	}
    }
}


void fread_prop( void *owner, int owner_type, FILE *fp )
{
    PROP_DATA *prop;
    char *word;
    int iNest;
    bool fMatch;
    bool fNest;
    bool fVnum;
    extern PROP_DATA *prop_free;     /* From mem.c */
    
    prop     = new_prop( );
    fNest   = FALSE;
    fVnum   = FALSE;
    iNest   = 0;

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
        fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
        KEY( "A",        prop->action_descr,   fread_string( fp ) );
        if ( !str_cmp( word, "Af" ) )
	    {
        BONUS_DATA *paf = new_bonus( );

		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
        paf->next       = prop->bonus;
		prop->bonus	= paf;
        fMatch          = TRUE;
		break;
	    }
	    break;

	case 'C':
        KEY( "C",    prop->cost,      fread_number( fp ) );
        break;

	case 'D':
        KEY( "D", prop->description,       fread_string( fp ) );
	    break;

	case 'E':
        KEY( "E",  prop->extra_flags,       fread_number( fp ) );

        if ( !str_cmp( word, "Ex" ) )
	    {
        EXTRA_DESCR_DATA *ed = new_extra_descr( );
        ed->keyword          = fread_string( fp );
        ed->description      = fread_string( fp );
        ed->next             = prop->extra_descr;
        prop->extra_descr     = ed;
		fMatch = TRUE;
	    }

        if ( !str_cmp( word, "END" ) )
	    {
		if ( !fNest || !fVnum )
		{
		    bug( "Fread_prop: incomplete prop.", 0 );
		    free_string( prop->name        );
		    free_string( prop->description );
            free_string( prop->action_descr );
		    free_string( prop->short_descr );
		    prop->next = prop_free;
		    prop_free  = prop;
		    return;
		}
		else
		{ // finish off the prop

    if ( prop->pIndexData->instances != NULL )
    {
    INSTANCE_DATA *trig, *pTrig;

    for ( pTrig = prop->pIndexData->instances;  pTrig != NULL;  pTrig = pTrig->next )
    {
        trig               = new_instance( );

        trig->script       = pTrig->script;
        trig->location     = NULL;

        trig->next       = prop->instances;
        prop->instances    = trig;
    }
    }

            prop->next   = prop_list;
		    prop_list	= prop;
		    prop->pIndexData->count++;
		    if ( iNest == 0 || rgObjNest[iNest] == NULL )
            {
                if ( owner == NULL )
                extract_prop( prop );
                else
                {
                   if ( owner_type == TYPE_ACTOR )
                   {
                       int flags = prop->extra_flags;
                       prop_to_actor( prop, (PLAYER_DATA *)owner );
                       if ( IS_SET(flags,ITEM_USED) )
                       SET_BIT(prop->extra_flags,ITEM_USED);
                   }
                   else
                   prop_to_scene( prop, (SCENE_INDEX_DATA *)owner );
                }
            }
		    else
			prop_to_prop( prop, rgObjNest[iNest-1] );

            script_update( prop, TYPE_PROP, TRIG_BORN, NULL, NULL, NULL, NULL );

		    return;
		}
	    }
	    break;

	case 'I':
        KEY( "I",    prop->item_type,         fread_number( fp ) );
	    break;

	case 'L':
        KEY( "L",       prop->level,             fread_number( fp ) );
	    break;

	case 'N':
        KEY( "N",        prop->name,              fread_string( fp ) );

	    if ( !str_cmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		{
		    bug( "Fread_prop: bad nest %d.", iNest );
		}
		else
		{
		    rgObjNest[iNest] = prop;
		    fNest = TRUE;
		}
		fMatch = TRUE;
	    }
	    break;

	case 'S':
        KEY( "S",  prop->short_descr,         fread_string( fp ) );
        KEY( "SP", prop->short_descr_plural,  fread_string( fp ) );
        KEY( "Sz", prop->size,                fread_number( fp ) );

#if defined(NEVER)
        if ( !str_cmp( word, "Script" ) ) {
           INSTANCE_DATA *pInstance = new_instance();
           pInstance->script = get_script_index( fread_number( fp ) );
           if ( !pInstance->script ) { free_instance(pInstance); break; }
           pInstance->next = prop->instances;
           prop->instances = pInstance;  
           /*
            * Note: scripts must be 'triggered' here
            */      
        }
#endif

        if ( !str_cmp( word, "Sk" ) )
	    {
		int iValue;
		SKILL_DATA *pSkill;

		iValue = fread_number( fp );
		pSkill     = skill_lookup( fread_word( fp ) );
		if ( iValue < 0 || iValue > 3 )
		{
		    bug( "Fread_prop: bad iValue %d.", iValue );
		}
		else if ( !pSkill )
		{
		    bug( "Fread_prop: unknown skill.", 0 );
		}
		else
		{
		    prop->value[iValue] = pSkill->vnum;
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
        KEY( "T",       prop->timer,             fread_number( fp ) );
	    break;

	case 'V':
        if ( !str_cmp( word, "Va" ) )
	    {
		prop->value[0]	= fread_number( fp );
		prop->value[1]	= fread_number( fp );
		prop->value[2]	= fread_number( fp );
        prop->value[3]   = fread_number( fp );
		fMatch		= TRUE;
		break;
	    }

        if ( !str_cmp( word, "V" ) )
	    {
        int vnum;
                
        vnum = fread_number( fp );
        if ( ( prop->pIndexData = get_prop_index( vnum ) ) == NULL )
               prop->pIndexData = get_prop_index( PROP_VNUM_TEMPLATE );
        fVnum = TRUE;
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
        KEY( "W",      prop->wear_flags,        fread_number( fp ) );
        KEY( "WL",     prop->wear_loc,          fread_number( fp ) );
        KEY( "Wt",      prop->weight,            fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
        sprintf( log_buf, "Fread_prop: %s no match.", word );
        bug( log_buf, 0 );
	    fread_to_eol( fp );
	}
    }
}

