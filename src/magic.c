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
#include "magic.h"
#include "skills.h"
#include "script.h"


/*
 * The kludgy global is for spells who want more stuff from command line.
 * Used in prop_cast_spell and cast
 */
char *target_name;


/*
 * Clears the spellbook.
 */
void clear_spell_book( SPELL_BOOK *pSpellBook ) {
    SPELL_BOOK *s, *spell_next;

    for( s = pSpellBook;  s != NULL;  s = spell_next ) {
         spell_next = s->next;
         free_spell_book( s );
    }
}

/*
 * Is this spell in this book?
 */
bool has_spell( SPELL_BOOK *book, int dbkey ) {
    SPELL_BOOK *pSpell;

    for ( pSpell = book;  pSpell != NULL;  pSpell = pSpell->next ) 
    { if ( dbkey == pSpell->dbkey ) return TRUE;  }

    return FALSE;
}


/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell( int level, PLAYER *victim )
{
    int save;

    save = 40 + ( victim->saving_throw ) * 5;
    save = URANGE( 5, save, 95 );

    return (number_percent( ) < save);
}



/*
 * Calculate the mana of a certain type on an actor 1 level deep.
 */
int find_gem_mana( PLAYER *ch, int bit )
{
    PROP *prop;
    PROP *cont;
    int mana = 0;

    if ( bit == 0 ) return -1;

    for ( prop = ch->carrying; prop != NULL; prop = prop->next_content )
    {
        if ( prop->item_type == ITEM_GEM && VAL_SET(prop, 0, bit) )
        {  mana += prop->value[1];    }
        else
        if ( prop->item_type == ITEM_CONTAINER && !IS_SET(prop->value[1], CONT_CLOSED) )
        {
            for ( cont = prop->contains; cont != NULL; cont = cont->next_content )
            {
                 if ( cont->item_type == ITEM_GEM && VAL_SET(cont, 0, bit) )
                 mana += cont->value[1];
            }
        }
    }

    return mana;
}



/*
 * Takes mana from appropriate gems 1 level deep.
 */
void take_mana_gem( PLAYER *ch, int mana, int bit )
{
    PROP *prop, *prop_next;
    PROP *cont, *cont_next;

    if ( mana == 0 ) return;

    for ( prop = ch->carrying; prop != NULL; prop = prop_next )
    {
        prop_next = prop->next_content;
        if ( prop->item_type == ITEM_GEM && VAL_SET(prop, 0, bit) )
        {
            if ( prop->value[1] >= mana )
            {
                prop->value[1] -= mana;
                if ( prop->value[1] == 0 )
                {
                act( "$p is drained of all its power, and shatters into thousands of shards!",
                     ch, prop, NULL, TO_ACTOR );
                prop_from_actor( prop );
                extractor_prop( prop );
                }
                return;
            }
            else
            {
                mana -= prop->value[1];
                act( "$p is drained of all its power, and shatters into thousands of shards!",
                     ch, prop, NULL, TO_ACTOR );
                prop_from_actor( prop );
                extractor_prop( prop );
            }
        }
        else
        if ( prop->item_type == ITEM_CONTAINER && !IS_SET(prop->value[1], CONT_CLOSED) )
        {
            for ( cont = prop->contains; cont != NULL; cont = cont_next )
            {
            cont_next = cont->next_content;
            if ( cont->item_type == ITEM_GEM && VAL_SET(cont, 0, bit) )
            {
               if ( cont->value[1] >= mana )
               {
                   cont->value[1] -= mana;
                   if ( cont->value[1] == 0 )
                   {
                   act( "$p is drained of all its power, and shatters into thousands of shards!",
                        ch, cont, NULL, TO_ACTOR );
                   prop_from_actor( cont );
                   extractor_prop( cont );
                   }
                   return;
               }
               else
               {
                   mana -= cont->value[1];
                   act( "$p is drained of all its power, and shatters into thousands of shards!",
                        ch, cont, NULL, TO_ACTOR );
                   prop_from_actor( cont );
                   extractor_prop( cont );
               }
            }
            }
        }
    }

    return;
}



/*
 * view spellbook command
 */   

void cmd_spellbook( PLAYER *ch, char *argument )
{ 
   SPELL *pSpell;
   SPELL_BOOK *pSpellBook;
   char buf[MAX_STRING_LENGTH];
   bool fMatch;
   int count;

   if ( !get_item_char( ch, ITEM_SPELLBOOK ) ) {
       send_to_actor( "You cannot find your spellbook.\n\r", ch );
       return;
   }

   count = 1;
   buf[0] = '\0';
   fMatch = FALSE;
   for ( pSpellBook = ch->spells;  pSpellBook != NULL;  pSpellBook = pSpellBook->next )
   {
       if ( (pSpell = get_spell_index( pSpellBook->dbkey )) != NULL )
       {
           SKILL *pGroup = get_skill_index( pSpell->group_code );

           strcat( buf, "Page " );
           strcat( buf, numberize( count++ ) );
           strcat( buf, " reads " );
           strcat( buf, pSpell->name );
           strcat( buf, " of the arcana " );
           strcat( buf, pGroup ? capitalize(pGroup->name) : "unknown" );
           strcat( buf, ".\n\r" );
           fMatch = TRUE;
       }
   }
   

   if ( !fMatch ) {
      send_to_actor( "Your spellbook is empty.\n\r", ch );
      return;
   }

   display_interp( ch, "^3" );
   send_to_actor( "You thumb through the vellum pages of your spellbook:\n\r", ch );
   send_to_actor( buf, ch );
   display_interp( ch, "^N" );
   return;
}



/*
 * Cast spells at targets using a magical prop.
 */
int prop_cast_spell( PROP *item, int dbkey, PLAYER *ch, 
                     PLAYER *victim, PROP *prop )
{
    SPELL *pSpell= get_spell_index(dbkey);
    INSTANCE *pInstance;
    SCRIPT *pScript;
    int target_type=TYPE_ACTOR;
    void *vo;

    if ( !pSpell ) return FALSE;

    for ( pInstance = pSpell->instances;  pInstance != NULL; pInstance = pInstance->next )
    {
    pScript = pInstance->script;

    if ( !pScript ) return FALSE;

    switch ( pSpell->target )
    {
    default: return FALSE; break;

    case TAR_IGNORE: 	vo = NULL;	break;

    case TAR_CHAR_OFFENSIVE:  
        target_type = TYPE_ACTOR;
	if ( victim == NULL ) victim = ch->fighting;
        if ( victim == NULL )
	{
            send_to_actor( "The spell fizzles without a valid target.\n\r", ch );
	    return FALSE;
	}
	vo = (void *) victim;
	break;

    case TAR_CHAR_DEFENSIVE:  
        target_type = TYPE_ACTOR;
	if ( victim == NULL ) victim = ch;
	vo = (void *) victim;
	break;

    case TAR_CHAR_SELF:       
        target_type = TYPE_ACTOR;
	vo = (void *) ch;
	break;

    case TAR_PROP_INV:        
        target_type = TYPE_PROP;
	if ( prop == NULL )
	{
            send_to_actor( "The spell fizzles without a valid target.\n\r", ch );
	    return FALSE;
	}
	vo = (void *) prop;
	break;
    }

    target_name = "";

    {
        INSTANCE *pCopy = new_instance( );
        VARIABLE *var;

        /*
         * Copy and activate the instance.
         */
        pCopy->script   = pScript;
        pCopy->location = pInstance->script->commands;
        pCopy->next     = ch->instances;
        ch->instances   = pCopy;

        var = new_var( );  /* temporary variable */

        if ( vo ) {
        var->type  = target_type;
        var->value = vo;
        assign_local( pCopy, var, "%target%" );
        }

        var->type = TYPE_PROP;
        var->value = item;
        assign_local( pCopy, var, "%prop%" );

        free_variable( var );      /* free temporary variable */
       /* trigger( ch, TYPE_ACTOR, pCopy ); */
/*parse_script( pCopy, ch, TYPE_ACTOR );*/
    }

    }  /* end for() */

    if ( pSpell->target == TAR_CHAR_OFFENSIVE
    &&   victim != ch
    &&   victim->master != ch )
    {
	PLAYER *vch;
	PLAYER *vch_next;

        /*
         * Start fights.
         */
	for ( vch = ch->in_scene->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_scene;
	    if ( victim == vch && victim->fighting == NULL )
	    {
                oroc( victim, ch );
		break;
	    }
	}
    }
    return TRUE;
}






/*
 * The act of inscription; a way to copy a spell from a master.
 */
void cmd_scribe( PLAYER *ch, char *argument ) {
   SPELL *pSpell;
   PLAYER *rch = NULL;
   ACTOR_TEMPLATE *pActorIndex = NULL;
   char buf[MAX_STRING_LENGTH]; 
   bool fMatch;
   int count=1;
   SPELL_BOOK *pSpellBook;

   while ( *argument == ' ') argument++;   

   /*
    * Check for the presence of a spellbook.
    */
   if ( !get_item_char(ch, ITEM_SPELLBOOK) ) {
       send_to_actor( "You cannot find your spellbook.\n\r", ch );
       return;
   }

   /*
    * See if they've just typed 'scribe'
    */
   if ( *argument == '\0' 
     || !str_prefix(argument, "list") ) {

   buf[0] = '\0';
   fMatch = FALSE;


    /*
     * Find the teacher.
     */
      pActorIndex = NULL;
      for ( rch = ch->in_scene->people;  rch != NULL; rch = rch->next_in_scene )
      {
        if ( NPC(rch)
          && IS_SET(rch->flag, ACTOR_PRACTICE) )
        {
            pActorIndex = rch->pIndexData;
            break;
        }
      }

      if ( !pActorIndex || !rch ) {
        send_to_actor( "There is no one here to teach you.\n\r", ch ); 
        return;
      }

    /*
     * List the spells.
     */
   for ( pSpellBook = rch->spells;  pSpellBook != NULL;  pSpellBook = pSpellBook->next )
   {
      if ( (pSpell = get_spell_index( pSpellBook->dbkey )) != NULL )
      {
           SKILL *pGroup = find_skill_pc( ch, pSpell->group_code );

           strcat( buf, "Page " );
           strcat( buf, numberize( count++ ) );
           strcat( buf, " reads '" );
           strcat( buf, pSpell->name );
           strcat( buf, "' of arcana " );

           if ( pGroup ) { strcat( buf, capitalize( pGroup->name ) ); 
                           strcat( buf, "." ); }
           strcat( buf, "\n\r" );
           fMatch = TRUE;
      }
   }

      if ( fMatch ) {
         send_to_actor( "Available spells:\n\r", ch ); 
         send_to_actor( buf, ch ); 
      }
      else act( "$N knows nothing of magick.", 
                ch, NULL, rch, TO_ACTOR ); 
      return;
   }

   /* scribe 'spell name'  
    * Otherwise, check to see if the spell exists, anywhere.
    */
   pSpell = find_spell( argument );
   if ( !pSpell ) {
      send_to_actor( "There is no such spell.\n\r", ch );
      return;
   }

   /*
    * Check to see if they already know it.
    */
   if ( has_spell(ch->spells, pSpell->dbkey) ) {
       send_to_actor( "You have already scribed that spell.\n\r", ch );
       return;
   }

   /*
    * Check to see if there is a practitioner nearby.
    */
   pActorIndex = NULL;
   for ( rch = ch->in_scene->people;  rch != NULL; rch = rch->next_in_scene )
   {
       if ( NPC(rch)
         && IS_SET(rch->flag, ACTOR_PRACTICE) )
       {
           pActorIndex = rch->pIndexData;
           break;
       }
   }

   /*
    * Scribe the spell.
    */
   if ( !pActorIndex ) {
        send_to_actor( "There is no one here to teach you.\n\r", ch ); 
        return;
   }
   
   if ( has_spell(pActorIndex->pSpells, pSpell->dbkey) ) {

        SPELL_BOOK *pNewPage;
        SPELL_BOOK *pEndPage;
        SKILL *pGroup = find_skill_pc( ch, pSpell->group_code );

        if ( pSpell->level > ch->exp_level ) {
             send_to_actor( "You are not yet ready for that spell.\n\r", ch );
             return;
        } 

         pNewPage = new_spell_book( );
         act( "$n scribe$v a new spell with $N's instruction.", ch, NULL, rch, TO_SCENE );

         pNewPage->next = NULL;
         pNewPage->dbkey = pSpell->dbkey;  
         pNewPage->name = str_dup( pActorIndex->short_descr );

         if ( ch->spells == NULL ) ch->spells = pNewPage;
         else {
                pEndPage = ch->spells; 
                while( pEndPage->next != NULL) pEndPage = pEndPage->next;
                pEndPage->next = pNewPage;
              }

         snprintf( buf, MAX_STRING_LENGTH, "You scribed %s from the spellbook of %s.\n\r", 
                  pSpell->name, pActorIndex->short_descr );
         send_to_actor( buf, ch );

         if ( pGroup ) update_skill(ch,pGroup->dbkey,pGroup->required_percent);
         return;                   
   }
   else act( "$E does not know of that spell.", ch, NULL, rch, TO_ACTOR );

   return;
}





void cmd_cast( PLAYER *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    PLAYER *victim=NULL;
    SPELL *pSpell;
    INSTANCE *pInstance;
    PROP *prop;
    int target_type=TYPE_ACTOR;
    void *vo;
    int mana;
    int actor_mana;

    /*
     * Switched NPC's can cast spells, but others can, too, if they are alone.
     */
    if ( NPC(ch) && (ch->master != NULL) )
	return;

    target_name = one_argument( argument, arg1 );
    one_argument( target_name, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_actor( "Cast which what where?\n\r", ch );
	return;
    }

    /*
     * Unfinished redundancy check.
     */
    pSpell = find_spell( arg1 );
    if ( !pSpell || !has_spell( ch->spells, pSpell->dbkey ) ) { 
     send_to_actor( "What spell is that?\n\r", ch ); return; }

    for ( pInstance = pSpell->instances;  pInstance != NULL;  pInstance = pInstance->next )
     { /* start */

     SCRIPT *pScript;

    pScript = pInstance->script;
    if ( !pScript ) { send_to_actor( "No such spell.\n\r", ch ); return; }

    /*
     * Check to see if spell is known.
     */
    {
          SPELL_BOOK *pSpellBook;

          for ( pSpellBook = ch->spells;  pSpellBook != NULL; pSpellBook = pSpellBook->next )
          if ( pSpellBook->dbkey == pSpell->dbkey ) break;

          if ( pSpellBook == NULL ) {
              send_to_actor( "You don't know of that spell.\n\r", ch );
          }
    }

    if ( ch->position < pSpell->minimum_position )
    {
        send_to_actor( "It is impossible to concentrate enough in this position.\n\r", ch );
	return;
    }

    if ( pSpell->mana_cost == 0 ) { mana = 0; send_to_actor( "You prepare the spell.\n\r", ch ); 
     } else  
    mana = NPC(ch) ? 0 : ( get_curr_int(ch)/25 ) * pSpell->mana_cost;

    /*
     * Locate targets.
     */
    victim	= NULL;
    prop	= NULL;
    vo		= NULL;
      
    switch ( pSpell->target )
    {
    default: return;

    case TAR_IGNORE:
	break;

    case TAR_CHAR_OFFENSIVE: 
	if ( arg2[0] == '\0' )
	{
	    if ( ( victim = ch->fighting ) == NULL )
	    {
		send_to_actor( "Cast the spell on whom?\n\r", ch );
		return;
	    }
	}
	else
	{
	    if ( ( victim = get_actor_scene( ch, arg2 ) ) == NULL )
	    {
		send_to_actor( "They aren't here.\n\r", ch );
		return;
	    }

            if ( IS_SET(victim->flag, ACTOR_GOOD) ) {
                send_to_actor( "You don't want to fight them.\n\r", ch );
                return;
            }
	}

        target_type = TYPE_ACTOR;
	vo = (void *) victim;
	break;

    case TAR_CHAR_DEFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    victim = ch;
	}
	else
	{
	    if ( ( victim = get_actor_scene( ch, arg2 ) ) == NULL )
	    {
		send_to_actor( "They aren't here.\n\r", ch );
		return;
	    }
	}

        target_type = TYPE_ACTOR;
	vo = (void *) victim;
	break;

    case TAR_CHAR_SELF:
    if ( arg2[0] != '\0' && !is_name( arg2, STR(ch, name) ) )
	{
	    send_to_actor( "You cannot cast this spell on another.\n\r", ch );
	    return;
	}

        target_type = TYPE_ACTOR;
	vo = (void *) ch;
	break;

    case TAR_PROP_INV:
	if ( arg2[0] == '\0' )
	{
	    send_to_actor( "What should the spell be cast upon?\n\r", ch );
	    return;
	}

    if ( ( prop = get_prop_carry( ch, arg2 ) ) == NULL )
	{
        send_to_actor( "You are not carrying that in your hand.\n\r", ch );
	    return;
	}

        target_type = TYPE_PROP;
	vo = (void *) prop;
	break;
    }

    /*
     * Tally the gem mana or the personal mana depending on the spell.
     */
    actor_mana = pSpell->mana_type == MANA_NONE ? ch->mana :
                 find_gem_mana( ch, pSpell->mana_type );

    if ( !NPC(ch) && mana > actor_mana )
    {
        switch ( pSpell->mana_type )
        {
            default: send_to_actor( "You lack the mana.\n\r", ch ); break;
     case MANA_FIRE: send_to_actor( "You don't have enough Fire gems.\n\r", ch ); break;
      case MANA_AIR: send_to_actor( "You don't have enough Air gems.\n\r", ch ); break;
    case MANA_WATER: send_to_actor( "You don't have enough Water gems.\n\r", ch ); break;
    case MANA_EARTH: send_to_actor( "You don't have enough Earth gems.\n\r", ch ); break;
        }
        return;
    }
      
    WAIT_STATE( ch, pSpell->delay );

    /*
     * Mana check and attempt cast.
     */
    if ( IS_IMMORTAL(ch) || actor_mana >= mana )
    {

//    SKILL *pGroup = find_skill_pc( ch, pSpell->group_code );

    if ( !NPC(ch) && number_percent( ) > 50 )
    {
	send_to_actor( "You lost your concentration.\n\r", ch );
        if ( mana > ch->mana ) {
           ch->mana = 0;
           mana -= ch->mana*2;
        } else ch->mana -= mana/2;

        take_mana_gem( ch, mana/2, pSpell->mana_type );
        return;
    }
    else
    {
    if ( mana > ch->mana ) 
      mana -= ch->mana;
      ch->mana = 0;
    }

    ch->mana -= mana;
        
        if ( pSpell->mana_type != MANA_NONE )
        take_mana_gem( ch, mana, pSpell->mana_type );

    /*
     * Create a new instance of the spell and
     * assign it to the caster.
     */
    {
        INSTANCE *pCopy;
        VARIABLE *var;

        pCopy           = new_instance( );
        pCopy->script   = pScript;
        pCopy->location = pCopy->script->commands;
        pCopy->locals   = NULL;
        pCopy->next     = ch->instances;
        ch->instances   = pCopy;

        var = new_var( );  /* temporary variable */

        if ( vo ) {
        var->type  = target_type;
        var->value = vo;
        assign_local( pCopy, var, "%target%" );

        var->type  = TYPE_STRING;
        var->value = str_dup( target_name );
        assign_local( pCopy, var, "%name%" );
        }

        free_variable( var );      /* free temporary variable */

        /* start the parse */
        ch->current  = ch->instances;
        parse_script( pCopy, ch, TYPE_ACTOR );
    }
    }
    }

    if ( pSpell->target == TAR_CHAR_OFFENSIVE
    &&   victim 
    &&   victim != ch
    &&   victim->master != ch )
    {
	PLAYER *vch;
	PLAYER *vch_next;

	for ( vch = ch->in_scene->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_scene;
	    if ( victim == vch && victim->fighting == NULL )
	    {
                oroc( victim, ch );
		break;
	    }
	}
    }

    return;
}






void cmd_mana( PLAYER *ch, char *argument ) {
   char buf[MAX_STRING_LENGTH];

   snprintf( buf, MAX_STRING_LENGTH, "Fire: %d   Water: %d   Earth: %d   Air: %d\n\r\n\r",
            find_gem_mana( ch, MANA_FIRE  ),
            find_gem_mana( ch, MANA_WATER ),
            find_gem_mana( ch, MANA_EARTH ),
            find_gem_mana( ch, MANA_AIR   )  );

   send_to_actor( buf, ch );

   snprintf( buf, MAX_STRING_LENGTH, "Personal mana: %d\n\r\n\r", ch->mana );
   send_to_actor( buf, ch );

}




void cmd_reagents( PLAYER *ch, char *argument ) {

        PROP *pProp, *pProp2;
        int quantities[MAX_COMPONENTS];
        int displayed=0;
        int i;

        for ( i=0; i<MAX_COMPONENTS; i++ ) quantities[i]=0;

        for( pProp = ch->carrying;  pProp != NULL;  pProp = pProp->next_content )
        {
                 for ( pProp2 = pProp->contains;  pProp2 != NULL;  pProp2 = pProp2->next_content )
                 {
                     if ( pProp2->item_type == ITEM_COMPONENT ) 
                       quantities[URANGE(0,pProp2->value[0],MAX_COMPONENTS-1)]++;
                 }

            if ( pProp->item_type == ITEM_COMPONENT )
                       quantities[URANGE(0,pProp->value[0],MAX_COMPONENTS-1)]++;
        }

        page_to_actor( "Your reagents supply:\n\r", ch );
        for ( i=0; i<MAX_COMPONENTS; i++ )
          if ( quantities[i] > 0 ) {
             char buf[MSL];
             snprintf( buf, MSL, "%20s: %d\n\r", comp_table[i].name, quantities[i] );
             page_to_actor( buf, ch );  displayed++;
          }
        if ( !displayed ) page_to_actor( "None.\n\r", ch );
 
}
