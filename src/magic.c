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
void clear_spell_book( SPELL_BOOK_DATA *pSpellBook ) {
    SPELL_BOOK_DATA *spell, *spell_next;

    for( spell = pSpellBook;  spell != NULL;  spell = spell_next ) {
         spell_next = spell->next;
         free_spell_book_data( spell );
    }
}

/*
 * Is this spell in this book?
 */
bool has_spell( SPELL_BOOK_DATA *book, int vnum ) {
    SPELL_BOOK_DATA *pSpell;

    for ( pSpell = book;  pSpell != NULL;  pSpell = pSpell->next ) 
    { if ( vnum == pSpell->vnum ) return TRUE;  }

    return FALSE;
}


/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell( int level, PLAYER_DATA *victim )
{
    int save;

    save = 40 + ( victim->saving_throw ) * 5;
    save = URANGE( 5, save, 95 );

    return (number_percent( ) < save);
}



/*
 * Calculate the mana of a certain type on an actor 1 level deep.
 */
int find_gem_mana( PLAYER_DATA *ch, int bit )
{
    PROP_DATA *prop;
    PROP_DATA *cont;
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
void take_mana_gem( PLAYER_DATA *ch, int mana, int bit )
{
    PROP_DATA *prop, *prop_next;
    PROP_DATA *cont, *cont_next;

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
                extract_prop( prop );
                }
                return;
            }
            else
            {
                mana -= prop->value[1];
                act( "$p is drained of all its power, and shatters into thousands of shards!",
                     ch, prop, NULL, TO_ACTOR );
                prop_from_actor( prop );
                extract_prop( prop );
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
                   extract_prop( cont );
                   }
                   return;
               }
               else
               {
                   mana -= cont->value[1];
                   act( "$p is drained of all its power, and shatters into thousands of shards!",
                        ch, cont, NULL, TO_ACTOR );
                   prop_from_actor( cont );
                   extract_prop( cont );
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

void cmd_spellbook( PLAYER_DATA *ch, char *argument )
{ 
   SPELL_DATA *pSpell;
   SPELL_BOOK_DATA *pSpellBook;
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
       if ( (pSpell = get_spell_index( pSpellBook->vnum )) != NULL )
       {
           SKILL_DATA *pGroup = get_skill_index( pSpell->group_code );

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
int prop_cast_spell( PROP_DATA *item, int vnum, PLAYER_DATA *ch, 
                     PLAYER_DATA *victim, PROP_DATA *prop )
{
    SPELL_DATA *pSpell= get_spell_index(vnum);
    INSTANCE_DATA *pInstance;
    SCRIPT_DATA *pScript;
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
        INSTANCE_DATA *pCopy = new_instance( );
        VARIABLE_DATA *var;

        /*
         * Copy and activate the instance.
         */
        pCopy->script   = pScript;
        pCopy->location = pInstance->script->commands;
        pCopy->next     = ch->instances;
        ch->instances   = pCopy;

        var = new_variable_data( );  /* temporary variable */

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
	PLAYER_DATA *vch;
	PLAYER_DATA *vch_next;

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
void cmd_scribe( PLAYER_DATA *ch, char *argument ) {
   SPELL_DATA *pSpell;
   PLAYER_DATA *rch = NULL;
   ACTOR_INDEX_DATA *pActorIndex = NULL;
   char buf[MAX_STRING_LENGTH]; 
   bool fMatch;
   int count=1;
   SPELL_BOOK_DATA *pSpellBook;

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
        if ( IS_NPC(rch)
          && IS_SET(rch->act, ACT_PRACTICE) )
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
      if ( (pSpell = get_spell_index( pSpellBook->vnum )) != NULL )
      {
           SKILL_DATA *pGroup = find_skill_pc( ch, pSpell->group_code );

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
   if ( has_spell(ch->spells, pSpell->vnum) ) {
       send_to_actor( "You have already scribed that spell.\n\r", ch );
       return;
   }

   /*
    * Check to see if there is a practitioner nearby.
    */
   pActorIndex = NULL;
   for ( rch = ch->in_scene->people;  rch != NULL; rch = rch->next_in_scene )
   {
       if ( IS_NPC(rch)
         && IS_SET(rch->act, ACT_PRACTICE) )
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
   
   if ( has_spell(pActorIndex->pSpells, pSpell->vnum) ) {

        SPELL_BOOK_DATA *pNewPage;
        SPELL_BOOK_DATA *pEndPage;
        SKILL_DATA *pGroup = find_skill_pc( ch, pSpell->group_code );

        if ( pSpell->level > ch->exp_level ) {
             send_to_actor( "You are not yet ready for that spell.\n\r", ch );
             return;
        } 

         pNewPage = new_spell_book_data( );
         act( "$n scribe$v a new spell with $N's instruction.", ch, NULL, rch, TO_SCENE );

         pNewPage->next = NULL;
         pNewPage->vnum = pSpell->vnum;  
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

         if ( pGroup ) update_skill(ch,pGroup->vnum,pGroup->required_percent);
         return;                   
   }
   else act( "$E does not know of that spell.", ch, NULL, rch, TO_ACTOR );

   return;
}





void cmd_cast( PLAYER_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    PLAYER_DATA *victim=NULL;
    SPELL_DATA *pSpell;
    INSTANCE_DATA *pInstance;
    PROP_DATA *prop;
    int target_type=TYPE_ACTOR;
    void *vo;
    int mana;
    int actor_mana;

    /*
     * Switched NPC's can cast spells, but others can, too, if they are alone.
     */
    if ( IS_NPC(ch) && (ch->master != NULL) )
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
    if ( !pSpell || !has_spell( ch->spells, pSpell->vnum ) ) { 
     send_to_actor( "What spell is that?\n\r", ch ); return; }

    for ( pInstance = pSpell->instances;  pInstance != NULL;  pInstance = pInstance->next )
     { /* start */

     SCRIPT_DATA *pScript;

    pScript = pInstance->script;
    if ( !pScript ) { send_to_actor( "No such spell.\n\r", ch ); return; }

    /*
     * Check to see if spell is known.
     */
    {
          SPELL_BOOK_DATA *pSpellBook;

          for ( pSpellBook = ch->spells;  pSpellBook != NULL; pSpellBook = pSpellBook->next )
          if ( pSpellBook->vnum == pSpell->vnum ) break;

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
    mana = IS_NPC(ch) ? 0 : ( get_curr_int(ch)/25 ) * pSpell->mana_cost;

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

            if ( IS_SET(victim->act, ACT_GOOD) ) {
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

    if ( !IS_NPC(ch) && mana > actor_mana )
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

//    SKILL_DATA *pGroup = find_skill_pc( ch, pSpell->group_code );

    if ( !IS_NPC(ch) && number_percent( ) > 50 )
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
        INSTANCE_DATA *pCopy;
        VARIABLE_DATA *var;

        pCopy           = new_instance( );
        pCopy->script   = pScript;
        pCopy->location = pCopy->script->commands;
        pCopy->locals   = NULL;
        pCopy->next     = ch->instances;
        ch->instances   = pCopy;

        var = new_variable_data( );  /* temporary variable */

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
	PLAYER_DATA *vch;
	PLAYER_DATA *vch_next;

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






void cmd_mana( PLAYER_DATA *ch, char *argument ) {
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




void cmd_reagents( PLAYER_DATA *ch, char *argument ) {

        PROP_DATA *pProp, *pProp2;
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
