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
#include "skills.h"

#define IS_ADEPT(ch,pSkill)     ( pSkill->skill_level >= pSkill->max_learn )
#define IS_GROUP_INDEX(pSkill)  ( pSkill && pSkill->group_code )
#define FIND_GROUP(pSkill)  ( pSkill->group_code && get_skill_index( pSkill->group_code) )

#define IS_FILTERED(pGroup) (\
             pGroup == GRO_MALADICTIONS\
          || pGroup == GRO_CONJURATIONS\
          || pGroup == GRO_ILLUSIONS\
          || pGroup == GRO_DIVINATIONS\
          || pGroup == GRO_ENCHANTMENTS\
          || pGroup == GRO_SUMMONINGS\
          || pGroup == GRO_NECROMANTICS\
          || pGroup == GRO_FIRE_BASIC\
          || pGroup == GRO_WATER_BASIC\
          || pGroup == GRO_AIR_BASIC\
          || pGroup == GRO_EARTH_BASIC ) 

/*---------------------------------------------------------------*/
/* Skill Index functions.
 */

/*
 * Kludgy workaround.
 */
int skill_dbkey( SKILL *pSkill ) {
    if ( !pSkill ) return 0;
    return pSkill->dbkey;
}

char *skill_name( int dbkey ) {
    SKILL *pSkill = get_skill_index( dbkey );

    if ( !pSkill ) return "unknown";
    return pSkill->name;
}

/*
 * Locates a skill in the Skill Index by skill name.
 */
SKILL *skill_lookup( const char *name )
{
    SKILL *pSkill=NULL;

    if ( MTD(name) ) return NULL;

    HASHSEARCH(skill__hash, !str_prefix( name, pSkill->name ), pSkill );

    return pSkill;
}


bool  has_skill( PLAYER *ch, SKILL *pSkill ) {
      SKILL *pSk;

      if ( !pSkill ) return FALSE;

      for ( pSk = ch->learned; pSk != NULL; pSk = pSk->next ) 
         if ( pSk->dbkey == pSkill->dbkey ) return TRUE;

      return FALSE;
}

/*----------------------------------------------------------------*/
/* Copy to player's Skill List.
 */

SKILL *skill_copy( SKILL *pSource ) 
{
    SKILL *pTarget;

    if ( !pSource ) return NULL;

 pTarget = new_skill( );

 pTarget->dbkey             = pSource->dbkey;
 pTarget->name             = pSource->name;
 pTarget->level            = pSource->level;
 pTarget->slot             = pSource->slot;

 pTarget->skill_level      = 0;
 pTarget->skill_time       = 0;
 pTarget->last_used        = 0;

 pTarget->target           = pSource->target;
 pTarget->msg_off          = pSource->msg_off;
 pTarget->minimum_position = pSource->minimum_position;
 pTarget->group_code       = pSource->group_code;
 pTarget->required_percent = pSource->required_percent;
 pTarget->req_str          = pSource->req_str;
 pTarget->req_int          = pSource->req_int;
 pTarget->req_wis          = pSource->req_wis;
 pTarget->req_dex          = pSource->req_dex;
 pTarget->req_con          = pSource->req_con;
 pTarget->slot             = pSource->slot;
 pTarget->delay            = pSource->delay;
 pTarget->max_prac         = pSource->max_prac;
 pTarget->max_learn        = pSource->max_learn;
 pTarget->cost             = pSource->cost;

    return pTarget;
}

/*----------------------------------------------------------------*/
/* Player Skill List functions
 */

int learned( PLAYER *ch, int dbkey ) {
    SKILL *pSkill; 
    int percent=0;

    for ( pSkill = ch->learned;  pSkill != NULL;  pSkill = pSkill->next )
    percent = pSkill->dbkey == dbkey ? pSkill->skill_level : percent;

    return percent;
}

/*
 * Nim's only capitalized function.
 */
int LEARNED( PLAYER *ch, char *name ) {
    SKILL *pSkill; 
    int percent=0;

    for ( pSkill = ch->learned;  pSkill != NULL;  pSkill = pSkill->next )
    percent = !str_cmp(name,pSkill->name) ? pSkill->skill_level : percent;

    return percent;
}

/*
 * Updates the value of a skill, creating a new Skill List entry
 * if needed.
 */
SKILL *update_skill( PLAYER *ch, int dbkey, int value ) {
   SKILL *pSkill;

   for ( pSkill = ch->learned; pSkill != NULL; pSkill = pSkill->next ) {
       if ( pSkill->dbkey == dbkey ) {
          pSkill->skill_level = value;
          return pSkill;
       }
   }    

   pSkill = get_skill_index( dbkey );
   if ( !pSkill ) return NULL;

   pSkill = skill_copy( pSkill );
   pSkill->next = ch->learned;
   ch->learned = pSkill;
   pSkill->skill_level = value;

   return pSkill;
}


/*
 * Adds a skill by name, creating a new Skill List entry if needed.
 */
void add_skill( PLAYER *ch, char *name, int value ) {
   SKILL *pSkill;

   pSkill = skill_lookup( name );
   if ( pSkill ) update_skill( ch, pSkill->dbkey, value );
   return;
}

/*
 * Return a percentage based on skill value.
 */
char *skill_level( SKILL *pSkill )
{
    int percent = PERCENTAGE(pSkill->skill_level, pSkill->max_learn);

    if ( percent <= 0 )   return "unknown";
    if ( percent == 1 )   return "untrained";
    if ( percent < 10 )   return "dabbling";
    if ( percent < 20 )   return "studying";
    if ( percent < 30 )   return "novice";
    if ( percent < 40 )   return "mediocre";
    if ( percent < 50 )   return "adequate";
    if ( percent < 60 )   return "advancing";
    if ( percent < 70 )   return "skilled";
    if ( percent < 80 )   return "trained";
    if ( percent < 100 )  return "mastering";
    if ( percent >= 100 ) return "adept";
    return "unknown";
};


SKILL *find_skill_pc( PLAYER *ch, int dbkey ) {
   SKILL *pSkill;

   for ( pSkill = ch->learned; pSkill != NULL; pSkill = pSkill->next ) {
      if ( pSkill->dbkey == dbkey ) return pSkill;
   }

   return NULL;
}

SKILL *find_group_pc( PLAYER *ch, SKILL *pSkill ) {
   SKILL *pGroup;

   if ( !ch || !pSkill ) return NULL;

   for ( pGroup = ch->learned; 
         pGroup != NULL;
         pGroup = pGroup->next ) {
        if ( pGroup->dbkey == pSkill->group_code )
        return pGroup;
   }

   return NULL;
}


/*--------------------------------------------------------------------
 *Skill advancement.
 */

/*
 * Newt: General purpose func to advance a skill. 
 * Locke: Send this func a copy of the PLAYER's skill, not the index.
 * Returns: TRUE if (ch)'s skill was advanced
 *          FALSE if not
 */
bool advance_skill( PLAYER *ch, SKILL *pSkill, 
                    int advance, int time_mod )
{
    SKILL *pGroup;

    if ( !NPC(ch)			/* PCs only		*/
/*      && pSkill->skill_time == 0 */
      && has_prereq( ch, pSkill, FALSE )/* Has the prerequisite */
      && !IS_ADEPT(ch, pSkill) 		/* Not yet an adept	*/
      && advance > 0 )			/* Actually improved    */  
    {
    	pSkill->skill_level += advance;
    	pSkill->skill_time = ((int_app[URANGE(0,get_curr_int(ch),25)].learn +
                            wis_app[URANGE(0,get_curr_wis(ch),25)].practice)/2) +
                            pSkill->learn_rate +
			    time_mod;

	if ( pSkill->skill_level == 1 )
        act( "You discover $t.", ch, pSkill->name, NULL, TO_ACTOR);
	else
	if ( pSkill->skill_level > 1 )
        act( "You improve in $t.", ch, pSkill->name,NULL,TO_ACTOR);

        if ( number_range( 0, 5 ) == 2 ) { exp_gain( ch, number_range( (ch->exp_level*2)/2, 100+ch->exp_level )/3, FALSE  ); }

        if ( number_range( 0, 30 ) == 2 ) { 
                   to_actor( "Super skill bonus: ", ch );
                   exp_gain( ch, number_range( (ch->exp_level*2) / 2, 100+ch->exp_level)/2, TRUE ); }

        /* Update group listing - check for slot lookup problems */
        
        pGroup = find_skill_pc( ch, pSkill->group_code );
        if ( pGroup ) {
        pGroup->skill_level = 1; 
        } else {
           SKILL *pNewGroup = skill_copy( get_skill_index( pSkill->group_code)  );
           if ( pNewGroup ) {
           pNewGroup->next = ch->learned;
           ch->learned = pNewGroup;
           }
        }
    	return TRUE;
    }
    return FALSE;
}


/*---------------------------------------
 * Skill tree / prerequisite tree.
 */
bool has_prereq( PLAYER *ch, SKILL *pSkill, bool fTell )
{
    bool ok = TRUE;
    SKILL *pGroup;

    if ( NPC(ch) ) return TRUE;

    if ( (pGroup = find_group_pc(ch,pSkill)) )
    {
        if ( pGroup->skill_level < pSkill->required_percent )
        update_skill(ch,pGroup->dbkey,pGroup->required_percent);
/*
        {
            if(fTell)
            {
                char buf[MAX_STRING_LENGTH];

                sprintf( buf,
                      "You have not yet achieved enough knowledge of %s.\n\r",
                      pSkill->name );
                to_actor( buf, ch );
            }
            ok = FALSE;
        }
*/
    }

    if ( get_curr_str( ch ) < pSkill->req_str )
    {
        if (fTell)
        to_actor( "You are too feeble to train this skill.\n\r", ch );
        ok = FALSE;
    }

    if ( get_curr_int( ch ) < pSkill->req_int )
    {
        if (fTell)
        to_actor( "You fail to decipher the secrets of this skill.\n\r", ch );
        ok = FALSE;
    }

    if ( get_curr_wis( ch ) < pSkill->req_wis )
    {
        if (fTell)
        to_actor( "You lack the basic, logical understandings to study this skill.\n\r", ch );
        ok = FALSE;
    }

    if ( get_curr_dex( ch ) < pSkill->req_dex )
    {
        if (fTell)
        to_actor( "You are not quite agile enough to train this skill.\n\r", ch );
        ok = FALSE;
    }

    if ( get_curr_con( ch ) < pSkill->req_con )
    {
        if (fTell)
        to_actor( "The rigorous training and practice of this skill would overcome your frailty.\n\r", ch );
        ok = FALSE;
    }

    return (ok);
}



/*---------------------------------
 * Display group.
 */
char *disp_group( PLAYER *ch, SKILL *pGroup )
{
    static char buf[MAX_STRING_LENGTH];
    char fub[MAX_STRING_LENGTH];

    /*
     * Filter 
     */ 

    if ( IS_FILTERED(pGroup->dbkey) )
    return ""; 

    if ( pGroup->group_code == 0 
      || pGroup->group_code == GRO_WP
      || IS_ADEPT(ch,pGroup) )
    snprintf( buf, MAX_STRING_LENGTH, "%s", pGroup->name );
    else
    snprintf( buf, MAX_STRING_LENGTH, "%s - %s", pGroup->name,
                  skill_level(pGroup) );

    sprintf( fub, "%s[ %s%s%s ]%s",
             trunc_fit( "---------------------------------------",
               30-(strlen(buf)/2) ),
             HAS_ANSI(ch) ? BOLD : "",
             string_proper( buf ),
             HAS_ANSI(ch) ? ANSI_NORMAL : "",
             trunc_fit( "---------------------------------------",
               30-(strlen(buf)/2) ));

    snprintf( buf, MAX_STRING_LENGTH, "%s\n\r", trunc_fit( fub, HAS_ANSI(ch) ? 60+8 : 60 ) );
    return buf;
}


/*
 * Display info for a single skill.
 */
char *disp_skill( PLAYER *ch, SKILL *pSkill )
{
    static char buf[MAX_STRING_LENGTH];

    snprintf( buf, MAX_STRING_LENGTH, "%s - %s",
             trunc_fit(pSkill->name, 18),
             skill_level(pSkill) );

    return buf;
}

/*
 * Display a skill group.
 */
void display_skills( PLAYER *ch, int group )
{
    char final[4*MAX_STRING_LENGTH];
    SKILL *pGroup;
    SKILL *pSkill;
    int col;

    if ( group > 0 )
    {
        pGroup = find_skill_pc( ch, group );
        if (pGroup && pGroup->group_code != 0)
        {
           
            int c = 0;

            page_to_actor( disp_group(ch, pGroup), ch );

            for ( pSkill=ch->learned;  pSkill != NULL;  pSkill=pSkill->next )
            {
                if ( pSkill->name == NULL )
                break;

                if ( pSkill->skill_level > 0
                  && pSkill->group_code==pGroup->dbkey
                  && !IS_FILTERED(pSkill->group_code) )
                {
                    page_to_actor( disp_skill(ch,pSkill), ch );

                    c++;
                    if ( c % 2 == 0 )
                    {
                        page_to_actor( "\n\r", ch );
                        c = 0;
                    }
                    if ( c % 2 != 0 )  page_to_actor( "\n\r", ch );
                }
            }

        }
        else
        to_actor( "You do not know of that skill group.\n\r", ch );
        return;
    }

    final[0] = '\0';

    for (pGroup = ch->learned; pGroup != NULL;  pGroup = pGroup->next )  
    {
        if ( pGroup->group_code > 0 ) continue;

        if (pGroup->skill_level > 0)
        {
            col = 0;
            strcat( final, disp_group(ch, pGroup) );

            for ( pSkill=ch->learned;
                  pSkill != NULL; 
                  pSkill = pSkill->next )
            {
                if ( MTD(pSkill->name) )
                break;

                if ( pSkill->skill_level > 0
                  && pSkill->group_code == pGroup->dbkey )
                {
                    char buf[MAX_STRING_LENGTH];

                    col++;
                    if ( col % 2 == 0 )
                    {
                        snprintf( buf, MAX_STRING_LENGTH, "%-27s", disp_skill(ch,pSkill) );
                        strcat( final, "  |  " );
                        strcat( final, buf );
                        strcat( final, "\n\r" );
                        col = 0;
                    }
                    else
                    {
                        snprintf( buf, MAX_STRING_LENGTH, "%28s", disp_skill(ch,pSkill) );
/*                        strcat( final, "      " );*/
                        strcat( final, buf );
                    }
                }
            }
            if ( col % 2 != 0 )  strcat( final, "\n\r" );
        }
    }
    page_to_actor( final, ch );

    return;
}


void show_skills_list( PLAYER *ch, SKILL *pSkill ) {
    char  buf[MSL];

        buf[0] = '\0';
        for ( ; pSkill != NULL;  pSkill = pSkill->next )
        {
            if ( pSkill->skill_level > 0 )
            {
                strcat( buf, capitalize( pSkill->name ) );
		strcat( buf, " for " );
		strcat( buf, name_amount( pSkill->cost ) );
                strcat( buf, ".\n\r" );
            }
        }

    to_actor( buf, ch );
}

void show_skills( PLAYER *ch, SKILL *pSkill ) {
   int col=0;
   char arg[MSL];

        for( ;
             pSkill != NULL;  pSkill = pSkill->next )
        {
            SKILL *pIndex = get_skill_index( pSkill->dbkey );
            
            if ( !pIndex ) continue;

            col++;
            sprintf( arg, "%20s/%3d ",
                     trunc_fit( pSkill->name, 20 ),
                     pSkill->skill_level );
            to_actor( arg, ch );
            if ( col % 3 == 0 )
            {
                to_actor( "\n\r", ch );
                col = 0;
            }
        }

        if ( col % 3 != 0 )  to_actor( "\n\r", ch );
        return;
}


/*
 * Syntax:  skills
 *          skills [skill]
 *          skills [group]
 */
void cmd_skills( PLAYER *ch, char * argument )
{
    SKILL *pSkill;
    bool fShow = FALSE;

    if (NPC(ch))
    {
        show_skills( ch, ch->learned );
        return;
    }

    if ( *argument == '\0' ) {
        display_skills( ch, -1 );
        return;
    }

    if ( (pSkill = skill_lookup( argument )) )
    {
        display_skills( ch, pSkill->dbkey );
        fShow = TRUE;
    }

    if ( (pSkill = find_skill_pc( ch, skill_dbkey(skill_lookup( argument )))) )
    {
        char buf[MAX_STRING_LENGTH];
        PLAYER *rch;

        for ( rch = ch->in_scene->people;  rch != NULL; rch=rch->next_in_scene )
        {
            if ( NPC(rch) 
              && IS_SET(rch->flag, ACTOR_PRACTICE)
              && find_skill_pc( rch, skill_dbkey(pSkill) ) )
            break;
        }

        if ( !rch ) {
            to_actor( 

"You do not know anyone to teach you that here.\n\r", ch );

            return;
        }

        if ( pSkill->skill_level >= pSkill->max_learn )
        {
            snprintf( buf, MAX_STRING_LENGTH, 

"You are an adept of %s.\n\r", pSkill->name );

            to_actor( buf, ch );
            return;
        }

        snprintf( buf, MAX_STRING_LENGTH, "You are %s in %s.\n\r",
                       skill_level(pSkill),
                       pSkill->name );
        to_actor( buf, ch );

        snprintf( buf, MAX_STRING_LENGTH, 

"You are apprenticed to %s.\n\r", NAME(rch) );

        to_actor( buf, ch );

        if ( pSkill->skill_time > 0 )
        {
        snprintf( buf, MAX_STRING_LENGTH, 
        "You still have %s game hours left until your next session.\n\r",

                 pSkill->skill_time > 10 ? "many" :
                 pSkill->skill_time > 5  ? "several" :
                 pSkill->skill_time > 2  ? "a few" : "a couple of" );
        to_actor( buf, ch );
        }
        else
        to_actor( 

"You may now learn more about that skill.\n\r", ch );

        return;
    }

    if ( !fShow )
    {
        page_to_actor( 
"------------------------------------------------------------\n\r", ch );
        display_skills( ch, -1 );
        page_to_actor( 
"============================================================\n\r", ch );
    }

#if defined(NEVER)
    {
        int race = race_lookup( ch->race );
        SKILL *x; int total=0;

        for ( x = ch->learned; x!=NULL; x=x->next ) total++;

    if ( total >= RACE(race,max_skills) )
    page_to_actor( "You have achieved the maximum number of skills.\n\r", ch );

    }
#endif
    return;
}

/*
 * Syntax:  learn
 *          learn [skill/group]
 */
void cmd_learn( PLAYER *ch,  char *argument )
{
    SKILL *pSkill=NULL;
    SKILL *pSSkill=NULL;
    SKILL *pTSkill=NULL;
    PLAYER *rch;
    char buf[MAX_STRING_LENGTH];
    int amount;
    bool fFound = FALSE;

    if ( MTD(argument) )
    {
        char buf2[MAX_STRING_LENGTH];

        /* Scan for teacher. */
       
        for ( rch = ch->in_scene->people;
              rch != NULL;
              rch = rch->next_in_scene )
        {
            if ( NPC(rch) && IS_SET(rch->flag, ACTOR_PRACTICE) )
                break;
        }

        if ( !rch )
        {
        to_actor( "There is no one here to teach you.\n\r", ch );
        return;
        }

        /* Show teacher's repertoire */

        sprintf( buf2, "%s is offering the following apprenticeships:\n\r\n\r", 
                 capitalize(NAME(rch)) );
        to_actor( buf2, ch );


        /* crossreference - dont display where student is present */
        for ( pSkill = rch->learned; pSkill != NULL; pSkill=pSkill->next ) {

        fFound = FALSE;
        for ( pSSkill = ch->learned; pSSkill != NULL; pSSkill=pSSkill->next ) 
        if ( pSSkill->teacher == rch->pIndexData->dbkey
          && pSSkill->dbkey == pSkill->dbkey ) fFound = TRUE;
        
        if ( !fFound && pSkill->cost != 0 ) {
        snprintf( buf, MAX_STRING_LENGTH, "     %s at %s per lesson.\n\r", 
                 capitalize(pSkill->name), name_amount(pSkill->cost) ); 
        to_actor( buf, ch );
        }

        }

        to_actor( "\n\r", ch );

        /* Show active apprenticeships. */

        sprintf( buf2, "  %s is teaching you:\n\r",
                 capitalize(HE_SHE(rch)) );
        to_actor( buf2, ch );

        /* Scan through list on player determining active apprenticeships */ 

        fFound = FALSE;
        for ( pSkill = ch->learned; pSkill != NULL; pSkill=pSkill->next ) {
        if ( pSkill->teacher != rch->pIndexData->dbkey 
          || pSkill->group_code == 100 /* language */ ) continue;
        if ( pSkill->skill_level >= pSkill->max_learn ) continue;
        snprintf( buf, MAX_STRING_LENGTH, "     %s at %s per lesson.\n\r", 
                 capitalize(pSkill->name), name_amount(pSkill->cost) ); 
        to_actor( buf, ch );
        fFound = TRUE;
        }

	if ( !fFound )
	{
        snprintf( buf, MAX_STRING_LENGTH, "  You are not yet apprenticed to %s.\n\r",
                 NAME(rch) );
        to_actor( buf, ch );
		return;
	}
        return;
    }

/*
 * Otherwise it was apprentice <blah>
 */
    
    if ( !( pSkill = skill_lookup( argument ) ) )
    {
        to_actor( "There is no such skill.\n\r", ch );
        return;
    }

    /* Scan for teacher. */

    for ( rch = ch->in_scene->people;  rch != NULL; rch = rch->next_in_scene )
    {
        if ( NPC(rch)
          && IS_SET(rch->flag, ACTOR_PRACTICE)
          && has_skill( rch, pSkill ) ) {
            pTSkill = find_skill_pc( rch, skill_dbkey(pSkill) );
            break;
          }
    }

    if ( !rch || !pTSkill )
    {
    	to_actor( "There is no one here to teach you that here.\n\r", ch );
    	return;
    }

    /*
     * Tuition costs check.
     */
    amount = pSkill->cost;

    if ( tally_coins( ch ) < amount )
    {
	to_actor( "You don't have enough money to learn that skill.\n\r", ch );
	return;
    }

    /* Tally racial skill limits */ 
#if defined(NEVER)
    {
        int race = race_lookup( ch->race );
        int total = 0;  SKILL *x;

        for ( x = ch->learned;  x != NULL;  x = x->next ) total++;

        if ( total >= RACE(race,max_skills) )
        {
            to_actor( "You have achieved the maximum number of skills for your race.\n\r", ch );
            return;
        }
    }
#endif

    if ( !has_prereq( ch, pSkill, TRUE ) ) return;



    /*
     * Locate the skill on the Student.
     */
    pSSkill = find_skill_pc( ch, skill_dbkey(pSkill) );

    if ( !pSSkill ) {
     pSSkill = update_skill( ch, pSkill->dbkey, 1 ); /* create new */
     update_skill( ch, pSSkill->group_code, 100 );    /* add the group */
    }

   /*
    * Can the student learn more from this instructor?
    */
    if ( pSSkill->skill_level > pTSkill->skill_level ) {
         to_actor( "You can learn no more from this instructor.\n\r", ch );
         return;
    }


     /* check the skill time */
        if ( pSSkill->skill_time > 0 )
        {
        snprintf( buf, MAX_STRING_LENGTH, 

"You still have %s game hours left until your next session.\n\r",

                 pSkill->skill_time > 10 ? "many" :
                 pSkill->skill_time > 5  ? "several" :
                 pSkill->skill_time > 2  ? "a few" : "a couple of" );
        to_actor( buf, ch );
        return;
        }


    /*
     * Take the tuition. 
     */
    snprintf( buf, MAX_STRING_LENGTH, "You receive %s in change.\n\r",
                 sub_coins( amount, ch ) );
    to_actor( buf, ch );



    /*
     * New skill bonus.
     */
	/* If a new skill... */
    if ( pSSkill->skill_level <= 1 )
    { 
        pSSkill = update_skill(ch, pSkill->dbkey, 3);
        pSSkill->teacher = rch->pIndexData->dbkey;
        display_interp( ch, "^B" );
        to_actor( "New skill bonus.\n\r", ch );
        display_interp( ch, "^N" );
    snprintf( buf, MAX_STRING_LENGTH, "You are now apprenticed in %s with %s.\n\r",
             pSSkill->name, NAME(rch) );
    to_actor( buf, ch );
    }
    else       /* If changing tutor... */
    if ( pSSkill->teacher != rch->pIndexData->dbkey )
    {
        snprintf( buf, MAX_STRING_LENGTH, "You have left the apprenticeship of your old mentor.\n\r" );
        to_actor( buf, ch );
        pSSkill->teacher = rch->pIndexData->dbkey;
    }

    snprintf( buf, MAX_STRING_LENGTH, "You learn %s from %s.\n\r",
             pSSkill->name, NAME(rch) );
    to_actor( buf, ch );
    /* improve the skill */
    update_skill( ch, pSkill->dbkey, 
       pSSkill->skill_level + number_range( 2, get_curr_wis(ch)/2 ) );
    pSSkill->skill_time = pSSkill->level > 35 ? pSkill->learn_rate : 1;
    return;
}


/*
 * Newt: Progressive cost for practicing depending on skill level.
 */
int practice_cost( int amount, int level )
{
    int cost;
    cost =   level < 10 ? amount / 2     /* Basics are cheap */
	   : level < 30 ? amount 
	   : level < 40 ? amount * 6 / 5 /* But the better you */
	   : level < 50 ? amount * 5 / 4 /* want to become...  */
	   : level < 60 ? amount * 4 / 3
	   : level < 70 ? amount * 3 / 2 /* The more it'll     */
	   :              amount * 2;    /* cost you!          */
    return ( cost );
}

/*
 * Syntax: train [pc name] [skill]
 */

void cmd_train( PLAYER *ch, char *argument )
{
    PLAYER *who;
    SKILL *pSkill;

    char arg[ MAX_INPUT_LENGTH ];

#define PC_TEACH_LIMIT (75) /* Minimum percentage in skill to teach */

    if ( MTD( argument ) )
    {
	to_actor( "Train who in which skill?\n\r", ch );
	return;
    }
    /* Huge amount of sanity checks... */
    argument = one_argument( argument, arg );
    if ( ( who = get_actor_scene( ch, arg ) ) == NULL )
    {
	to_actor( "You can see no such creature here.\n\r", ch );
	return;
    }
    if ( ( who == ch ) )
    {
        to_actor( "Teaching yourself?\n\r", ch );
        return;
    }
    /* Valid skill? */
    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	to_actor( "Train which skill?\n\r", ch );
	return;
    }
    if ( !(pSkill = skill_lookup( arg )) )
    {
	to_actor( "There is no such skill.\n\r", ch );
	return;
    }

     /* Can teach? Insert "teaching" skill check here... */

    if ( (pSkill=find_skill_pc(ch,pSkill->dbkey)) 
       && pSkill->skill_level < PC_TEACH_LIMIT )
    {
	sprintf( arg, "You are not skilled enough in %s!\n\r",
		pSkill->name );
	to_actor( arg, ch );
	return;
    }

    /* Above 1/3 limit & and trainee ready to advance? */
    if ( pSkill->skill_level/3 > learned(who,pSkill->dbkey)
      && advance_skill( who, pSkill, 1, 0 ) )
    {
    	sprintf( arg, "You train $N in %s.", pSkill->name );
    	act( arg, ch, NULL, who, TO_ACTOR );
    	sprintf( arg, "$n trains you in %s.", pSkill->name );
    	act( arg, ch, NULL, who, TO_VICT );
    }
    else
    act( "$N cannot be trained by you.", ch, NULL, who, TO_ACTOR );
    return;
}


/*---------------------------------------------------------------*/
/* Used throughout the source.
 */

/*
 * Check for a skill.
 * Send player skill list entry as pSkill; or use the Teacher's or
 * call with find_skill()
 */
bool skill_check( PLAYER *ch, SKILL *pSkill, int modifier )
{
    int level,l,roll=number_percent( );
    float ratio=0;
    bool success=FALSE;

    if ( !pSkill ) return TRUE;

    /* Spawn last used time */
    if ( !NPC(ch) )
        pSkill->last_used = 0;

    if ( NPC(ch) && learned(ch,pSkill->dbkey) == 0 ) return number_percent() > 40;

    ratio = 1.0 + ( (float) modifier ) / 100.0;

    level = (l=learned(ch,pSkill->dbkey)) * ratio;

    roll = roll + (roll/2);

    success = URANGE(0,level,130) > roll;

    { char buf[MAX_STRING_LENGTH];
    snprintf( buf, MAX_STRING_LENGTH, "Skill> %s using %s %d+%d%% yielded %d%% r:%1.3f against %d %s",
              NAME(ch), pSkill->name, l, modifier, level, ratio, roll, success ? "success" : "fail" );
    NOTIFY( buf, LEVEL_IMMORTAL, WIZ_NOTIFY_DAMAGE );
    }

    if( success )
    {
        /* Improvement by use chance 0% ... 4% */
    if ( ((4 - int_app[URANGE(0,get_curr_int(ch),25)].learn)/4)+1 >=number_range(1,100))
        advance_skill( ch, pSkill, number_range(1,pSkill->max_prac),0);
        return TRUE;
    }
    else
    {
        /* Learn from mistakes 0% ... 3% */
    if ( ((2 - int_app[URANGE(0,get_curr_int(ch),25)].learn)/4)+1 >= number_range(1,100) )
        advance_skill( ch, pSkill, number_range(1,pSkill->max_prac),0);
        return FALSE;
    }
}


