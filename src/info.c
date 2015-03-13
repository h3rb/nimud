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


char *  const   where_name  [] =
{
    "<as earring>",
    "<as earring>",
    "<placed on head>",
    "<fastened to helm>",
    "<tied about forehead>",
    "<over face>",
    "<piercing nose>",
    "<around neck>",
    "<around neck>",
    "<as shirt>",
    "<on body>",
    "<strapped to back>",
    "<on legs>",
    "<as pants>",
    "<as stockings>",
    "<on feet>",
    "<around ankle>",
    "<around ankle>",
    "<on arms>",
    "<around wrist>",
    "<around wrist>",
    "<on hands>",
    "<on finger>",
    "<on finger>",
    "<as shield>",
    "<about body>",
    "<floating nearby>",
    "<worn about waist>",
    "<over loins>",
    "<pinned on garment>",
    "<thrust through belt>",
    "<thrust through belt>",
    "<thrust through belt>",
    "<thrust through belt>",
    "<thrust through belt>",
    "<held in hand>",
    "<held in hand>",
    "<slung over shoulder>",
    "<slung over shoulder>",
    "<wield primary>",
    "<wield secondary>",
    "<fastened to waist>",
    "<fastened to waist>",
    "<fastened to waist>",
    "<on shoulders>",
};




#define MAX_ITERATION       (IS_IMMORTAL(ch) ? 16 : 4)      /* for old ver */


#define DEFCOLOR(ch)  default_color_variable = !NPC(ch) && \
                      IS_SET(ch->flag2,PLR_TIMECOLOR) ? time_color(ch) : "^N"; \
                      default_color_variable_di = !NPC(ch) && IS_SET(ch->flag2,PLR_TIMECOLOR) ? time_color(ch) : NTEXT;

bool isGrey = FALSE;
/*
 * 
 */
char *time_color( PLAYER *ch ) {
    isGrey = FALSE;
    if ( !IS_OUTSIDE(ch) ) return "^N";
    if ( IS_SET(ch->flag2, PLR_TIMECOLOR) ) {
        switch( weather.hour ) {

             case 0: 
             case 1: 
             case 2: 
             case 3: 
             case 4: 
             case 5: isGrey=TRUE; return weather.moon_phase==MOON_FULL 
                                                  ? "^N" : "^N^=^B";  
             case 6: return "^N^1";
             case 7: return "^N^3";
             case 8: 
             case 9: 
             case 10:  
             case 11:  
             case 12:
             case 13:  return "^N";
             case 14:  return "^3";
             case 15:    
             case 16:  
             case 17:  return "^6";
             case 18:  return "^4";
             case 19:    
             case 20:  return "^5";
             case 21: 
             case 22: 
             case 23: return weather.moon_phase==MOON_FULL ? "^N^0^B^=" : "^1^B"; 
             default:  return "^N";
             
        }
    }
    return "^N"; 
}



void show_scene_to_actor( PLAYER *ch, SCENE *pScene, 
                        int dist, int dir )
{
    PLAYER *rch;
    char buf[MAX_STRING_LENGTH];
    char *p;
    int count;

    /*
     * Count the number of people.
     */
    count = 0;
    for ( rch = pScene->people; rch != NULL;  rch = rch->next_in_scene )
    {
       if ( rch != ch && can_see( ch, rch ) )
       {
           count++;
           if ( NPC(ch) && IS_SET(ch->flag, ACTOR_NOSCAN) )  count--;
       }
    }
    
    if ( scene_is_dark( pScene )
      || !check_blind( ch )
      || count <= 0 )
    return;
    
    switch ( dist )
    {
       case 1: snprintf( buf, MAX_STRING_LENGTH, "Nearby %s you can see", dir_name[dir] ); break;
       case 2: snprintf( buf, MAX_STRING_LENGTH, "%s of here, you see", capitalize(dir_name[dir]) ); break;
       case 3: snprintf( buf, MAX_STRING_LENGTH, "In the distance to the %s is", dir_name[dir] ); break;
       case 4: snprintf( buf, MAX_STRING_LENGTH, "Far away %s from you is",      dir_name[dir] ); break;
      default: break;
    }       
    
    for( rch = pScene->people;  rch != NULL;  rch = rch->next_in_scene )
    {        
        if ( !can_see( ch, rch )
          || (NPC(ch) && IS_SET(ch->flag, ACTOR_NOSCAN))
          || ch == rch )
        continue;
        
        if ( --count < 0 ) break;

        strcat( buf, " " );
        strcat( buf, NAME(rch) );

        if ( count > 1 )
        strcat( buf, "," );
        
        if ( count == 1 )
        strcat( buf, " and" );        
        
        if ( count == 0 )
        strcat( buf, ".\n\r" );
    }
        
    p = format_string( str_dup(buf) );    
    page_to_actor( buf, ch );
    free_string( p );
    return;
}        



/*
 * Brand new version.
 */
void scan_direction( PLAYER *ch, int dir )
{ 
    SCENE *pScene;
    EXIT *pExit;
    char buf[MAX_STRING_LENGTH];
    int dist;
    
    if ( (pExit = ch->in_scene->exit[dir]) == NULL
      || IS_SET(pExit->exit_flags, EXIT_CLOSED) )
    return;    

    for( dist = 1; dist < 5; dist++ )
    {
        if ( (pScene = pExit->to_scene) == NULL )
        break;
        
        show_scene_to_actor( ch, pScene, dist, dir );

        if ( (pExit = pScene->exit[dir]) == NULL )
        break;

        if ( IS_SET(pExit->exit_flags, EXIT_CLOSED)
          || IS_SET(pExit->exit_flags, EXIT_CONCEALED) )
        {
            if ( !IS_SET(pExit->exit_flags, EXIT_CONCEALED) )
            {
                snprintf( buf, MAX_STRING_LENGTH, "A closed %s%s blocks your view.\n\r",
                         pExit->keyword,
                         dist == 1 ? " nearby"          :
                         dist == 2 ? " close by"        :
                         dist == 3 ? " in the distance" : "" );
                page_to_actor( buf, ch );
            }

            break;
        }
        else
        if ( IS_SET(pExit->exit_flags, EXIT_ISDOOR)
          && !IS_SET(pExit->exit_flags, EXIT_CONCEALED) )
        {
            snprintf( buf, MAX_STRING_LENGTH, "You can see an opened %s%s.\n\r",
                     pExit->keyword,
                     dist == 1 ? " nearby"          :
                     dist == 2 ? " close by"        :
                     dist == 3 ? " in the distance" : "" );
            page_to_actor( buf, ch );
        }
        else
        if ( !IS_SET(pExit->exit_flags, EXIT_ISDOOR)
          && !IS_SET(pExit->exit_flags, EXIT_CONCEALED)
          && !MTD(pExit->keyword) )
        {
            snprintf( buf, MAX_STRING_LENGTH, "You are able to peer through a%s %s.\n\r",
                     IS_VOWEL(pExit->keyword[0]) ? "n" : "",
                     pExit->keyword );
            page_to_actor( buf, ch );
        }
    }
    
    return;
}




char *format_prop_to_actor( PROP *prop, PLAYER *ch, bool fShort )
{
    static char buf[MAX_STRING_LENGTH];
    char pre[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if ( fShort )
    {
        pre[0] = '\0';
        if ( IS_PROP_STAT(prop, ITEM_INVIS)     ) strcat( pre, "invisible "  );
        if ( IS_AFFECTED(ch, BONUS_DETECT_MAGIC)
          && IS_PROP_STAT(prop, ITEM_MAGIC)     ) strcat( pre, "shimmering " );
        if ( IS_PROP_STAT(prop, ITEM_GLOW)      ) strcat( pre, "glowing "    );
        if ( IS_PROP_STAT(prop, ITEM_BURNT)   ) strcat( pre, "charred "    );
	if ( IS_PROP_STAT(prop, ITEM_BURNING) ) strcat( pre, "burning " );
        if ( IS_PROP_STAT(prop, ITEM_HUM)       ) strcat( pre, "humming "    );

        if ( TOO_BIG(ch,prop)   && (prop->carried_by == ch
          || IS_SET(prop->extra_flags, ITEM_INVENTORY)) ) strcat(pre, "loose " );
        else
        if ( TOO_SMALL(ch,prop) && (prop->carried_by == ch
          || IS_SET(prop->extra_flags, ITEM_INVENTORY)) ) strcat(pre, "tight " );

        switch ( prop->item_type )
        {
            default: break;
    case ITEM_LIGHT:
/*         if( prop->value[1] )*/
         {
            int percent = PERCENTAGE( prop->value[0], prop->value[1] );
            if ( prop->value[1] < prop->value[0] ) percent = 100;

            if ( !prop->value[0] && VAL_SET(prop,3,LIGHT_FILLABLE) )
            strcat( pre, "empty " );
            else
            strcat( pre,
             !IS_LIT(prop)  ? "unlit "    :
             percent <  10 ? "very dim " :
             percent <  20 ? "dim "      :
                             "lit " );
         }
        break;
    case ITEM_DRINK_CON: if ( !prop->value[0] ) strcat( pre, "empty " ); break;
    case ITEM_ARMOR:
         if( prop->value[0] )
         {
            int percent;

            percent = PERCENTAGE( prop->value[0], prop->pIndexData->value[0] );
            strcat( pre,
             percent <  10 ? "destroyed "     :
             percent <  20 ? "poor "          :
             percent <  30 ? "badly damaged " :
             percent <  40 ? "damaged "       :
             percent <  50 ? "worn "          :
             percent <  80 ? "somewhat used " :
                             "new " );
         }
        break;
    case ITEM_GEM:
         if( prop->value[1] )
         {
            int percent;
            percent = PERCENTAGE( prop->value[1], prop->value[2] );

            strcat( pre,
             percent <  10 ? "darkened " :
             percent <  20 ? "pulsing "  :
                             "" );
         }
        break;
        }

        if ( pre[0] != '\0' )
        {
            snprintf( buf, MAX_STRING_LENGTH, "%s %s%s", IS_VOWEL(pre[0]) ? "an" : "a", pre,
                     smash_article( STR(prop, short_descr) ) );
        }
        else
        strcat( buf, STR(prop,short_descr) );

        if ( prop->item_type == ITEM_DRINK_CON
          && IS_SET(prop->extra_flags, ITEM_INVENTORY)
          && prop->value[0] != 0 )
        {
            if ( prop->value[2] < 0 || prop->value[2] >= LIQ_MAX )
            {
                prop->value[2] = 0;
                bug( "Invalid liquid on prop %d.", prop->pIndexData->dbkey );
            }

            strcat( buf, " of " );
            strcat( buf, liq_table[prop->value[2]].liq_name );
        }
    }
    else
    {
        PLAYER *puller;

        if ( IS_PROP_STAT(prop, ITEM_INVIS)     )   strcat( buf, "(Invisible) ");
        if ( IS_AFFECTED(ch, BONUS_DETECT_EVIL)
          && IS_PROP_STAT(prop, ITEM_EVIL)   )   strcat( buf, "(Red Aura) "    );
        if ( IS_AFFECTED(ch, BONUS_DETECT_MAGIC)
          && IS_PROP_STAT(prop, ITEM_MAGIC)  )   strcat( buf, "(Yellow Glow) " );
        if ( IS_PROP_STAT(prop, ITEM_GLOW)      )   strcat( buf, "(Glowing) "  );
        if ( IS_PROP_STAT(prop, ITEM_HUM)       )   strcat( buf, "(Humming) "  );

        if ( (puller = hitched( prop )) == NULL )
        {
            if ( ch->furniture == prop )
            {
                strcat( buf, capitalize(STR(prop,short_descr)) );
                strcat( buf, " upon which you are " );
                strcat( buf, position_name( ch->position ) );
                strcat( buf, " is here.\n\r" );
            }
            else
            strcat( buf, STR(prop, description) );
        }
        else
        {
            strcat( buf, "   " );
            strcat( buf, capitalize(STR(prop, short_descr)) );
            strcat( buf, " is hitched to " );
            strcat( buf, PERS(puller, ch) );
            strcat( buf, ".\n\r" );
        }       
        
        if ( prop->item_type == ITEM_FURNITURE 
          && prop->contains != NULL )
        {
            char xbuf[MAX_STRING_LENGTH];

            if ( prop->contains->next_content != NULL
              && can_see_prop(ch, prop->contains)
              && can_see_prop(ch, prop->contains->next_content) )
             sprintf( xbuf, "      A few items are on %s.\n\r",
                     STR(prop,short_descr) );
            else
            if ( can_see_prop(ch, prop->contains) )
             sprintf( xbuf, "      %s is on %s.\n\r",
                     capitalize(STR(prop->contains,short_descr)),
                     STR(prop,short_descr) );

            strcat( buf, xbuf );
        }
    }

    return buf;
}




/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_actor( PROP *list, PLAYER *ch, bool fShort, bool fShowNothing )
{
    char buf[MAX_STRING_LENGTH];
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    PROP **lastprop;
    PROP *prop;
    int nShow;
    int iShow;
    int count;
    bool fCombine;

    if ( ch->desc == NULL )
        return;
    
    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( prop = list; prop != NULL; prop = prop->next_content ) count++;
    prgpstrShow	= alloc_mem( count * sizeof(char *) );
    prgnShow    = alloc_mem( count * sizeof(int)    );
    lastprop     = alloc_mem( count * sizeof(PROP *) );
    nShow       = 0;

    /*
     * Format the list of props.
     */
    for ( prop = list; prop != NULL; prop = prop->next_content )
    {
        if ( prop->wear_loc == WEAR_NONE
          && can_see_prop( ch, prop )
          && hitched(prop) == NULL )
        {
	    if ( prop->item_type == ITEM_FURNITURE
	    &&   prop->value[0]/100 > 0
            &&  (OCCUPADO(prop) || ch->furniture == prop) )
		continue;
            pstrShow = str_dup( format_prop_to_actor( prop, ch, fShort ) );
            fCombine = FALSE;

            if ( NPC(ch) || IS_SET(ch->flag2, PLR_COMBINE) )
            {
                /*
                 * Look for duplicates, case sensitive.
                 * Matches tend to be near end so run loop backwards.
                 */
                for ( iShow = nShow - 1; iShow >= 0 && iShow < count; iShow-- )
                {
                    if ( !strcmp( prgpstrShow[iShow], pstrShow )
                      && prop->item_type != ITEM_MONEY )
                    {
                    ++prgnShow[iShow];
                    lastprop[iShow] = prop;
                    fCombine = TRUE;
                    break;
                    }
                }
            }

            /*
             * Couldn't combine, or didn't want to.
             */
            if ( !fCombine )
            {
                 prgpstrShow [nShow] = str_dup( pstrShow );
                 prgnShow    [nShow] = 1;
                 lastprop     [nShow] = prop;
                 nShow++;
            }

            free_string( pstrShow );
        }
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
        if ( fShort )
        send_to_actor( " ", ch );

        if ( NPC(ch) || IS_SET(ch->flag2, PLR_COMBINE) )
        {
            if ( prgnShow[iShow] > 1 )
            {
                if (fShort) snprintf( buf, MAX_STRING_LENGTH, "%s ", numberize(prgnShow[iShow]) );
                else 
                {
                    char *pt;

                    pt = STR(lastprop[iShow], description_plural);
                    if ( MTD(pt) )
                    {
                         snprintf( buf, MAX_STRING_LENGTH, "%s %s are here.\n\r",
                                       numberize(prgnShow[iShow]),
                                       pluralize(STR(lastprop[iShow],short_descr))  );
                         buf[0] = UPPER(buf[0]);
                    }
                    else buf[0] = '\0';
                }
                page_to_actor( buf, ch );
            }
        }

        if (fShort)
        {
            page_to_actor( prgnShow[iShow] > 1 ? pluralize( STR(lastprop[iShow],short_descr) ) :
                                                prgpstrShow[iShow],
                          ch );
            page_to_actor( "\n\r", ch );
        }
        else
        {
            if ( buf[0] == '\0' && prgnShow[iShow] > 1 )
            {
                snprintf( buf, MAX_STRING_LENGTH, STR(lastprop[iShow], description_plural),
                              numberize(prgnShow[iShow]) );
                page_to_actor( capitalize(buf), ch );
            }
            else
            if ( prgnShow[iShow] <= 1 )
            page_to_actor( capitalize(prgpstrShow[iShow]), ch );
        }

        free_string( prgpstrShow[iShow] );
        lastprop[iShow] = NULL;
    }

    if ( fShowNothing && nShow == 0 )
    {
        if ( NPC(ch) || IS_SET(ch->flag2, PLR_COMBINE) )
            page_to_actor( "     ", ch );
        page_to_actor( "Nothing.\n\r", ch );
    }

    /*
     * Clean up.
     */
    free_mem( prgpstrShow, count * sizeof(char *)      );
    free_mem( prgnShow,    count * sizeof(int)         );
    free_mem( lastprop,     count * sizeof(PROP *)  );

    return;
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
char *show_list_to_actor2( PROP *list, PLAYER *ch, char *prefix )
{
    static char final[MAX_STRING_LENGTH*4];
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    PROP **lastprop;
    PROP *prop;
    int nShow;
    int iShow;
    int count;
    bool fCombine;

    if ( ch->desc == NULL )
        return "";
    
    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( prop = list; prop != NULL; prop = prop->next_content ) count++;
    prgpstrShow = alloc_mem( count * sizeof(char *) );
    prgnShow    = alloc_mem( count * sizeof(int)    );
    lastprop     = alloc_mem( count * sizeof(PROP *) );
    nShow       = 0;

    /*
     * Format the list of props.
     */
    for ( prop = list; prop != NULL; prop = prop->next_content )
    {
        if ( prop->wear_loc == WEAR_NONE
          && can_see_prop( ch, prop )
          && hitched(prop) == NULL )
        {
	    if ( prop->item_type == ITEM_FURNITURE
	    &&   prop->value[0]/100 > 0
            &&  (OCCUPADO(prop) || ch->furniture == prop) )
		continue;
            pstrShow = str_dup( format_prop_to_actor( prop, ch, TRUE ) );
            fCombine = FALSE;

            if ( NPC(ch) || IS_SET(ch->flag2, PLR_COMBINE) )
            {
                /*
                 * Look for duplicates, case sensitive.
                 * Matches tend to be near end so run loop backwards.
                 */
                for ( iShow = nShow - 1; iShow >= 0 && iShow < count; iShow-- )
                {
                    if ( !strcmp( prgpstrShow[iShow], pstrShow )
                      && prop->item_type != ITEM_MONEY )
                    {
                    ++prgnShow[iShow];
                    lastprop[iShow] = prop;
                    fCombine = TRUE;
                    break;
                    }
                }
            }

            /*
             * Couldn't combine, or didn't want to.
             */
            if ( !fCombine )
            {
                 prgpstrShow [nShow] = str_dup( pstrShow );
                 prgnShow    [nShow] = 1;
                 lastprop     [nShow] = prop;
                 nShow++;
            }

            free_string( pstrShow );
        }
    }

    /*
     * Output the formatted list.
     */

    sprintf( final, nShow == 1 ? prefix
                              : replace_string( prefix, " is ", " are " ) );
    
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
        if ( iShow == nShow-1 && nShow != 1 )
        strcat( final, "and " );

        if ( prgnShow[iShow] > 1 && lastprop[iShow] )
        {         
            strcat( final, numberize(prgnShow[iShow]) );
            strcat( final, " " );
            strcat( final, pluralize( STR(lastprop[iShow],short_descr) ) );
        }
        else
        strcat( final, prgpstrShow[iShow] );

        if ( iShow != nShow-1 )
        strcat( final, ", " );


        free_string( prgpstrShow[iShow] );
        lastprop[iShow] = NULL;
    }

    if ( nShow == 0 ) strcat( final, "nothing" );
    
    strcat( final, ". " );

    {
        char *p;
        p = format_string( str_dup( final ) );
        sprintf( final, "%s", p );
        free_string( p );
    }

    /*
     * Clean up.
     */
    free_mem( prgpstrShow, count * sizeof(char *)      );
    free_mem( prgnShow,    count * sizeof(int)         );
    free_mem( lastprop,     count * sizeof(PROP *)  );
    
    return final;
}



void show_peek_to_actor( PROP *list, PLAYER *ch, bool fShort,
                        bool fShowNothing, int percent )
{
    char buf[MAX_STRING_LENGTH];
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    PROP **lastprop;
    PROP *prop;
    int nShow;
    int iShow;
    int count;
    bool fCombine;

    if ( ch->desc == NULL )
	return;
    
    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( prop = list; prop != NULL; prop = prop->next_content )
	count++;
    prgpstrShow	= alloc_mem( count * sizeof(char *) );
    prgnShow    = alloc_mem( count * sizeof(int)    );
    lastprop     = alloc_mem( count * sizeof(PROP *) );
    nShow	= 0;

    /*
     * Format the list of props.
     */
    for ( prop = list; prop != NULL; prop = prop->next_content )
    {
        bool fHitched = (hitched(prop) != NULL ? TRUE : FALSE);

    if ( prop->wear_loc == WEAR_NONE
      && can_see_prop( ch, prop )
      && percent > number_percent( )
      && occupant(prop)
      && !fHitched )
	{
        pstrShow = percent > number_percent( ) ?
                   format_prop_to_actor( prop, ch, fShort ) : "something";
	    fCombine = FALSE;

        if ( NPC(ch) || IS_SET(ch->flag2, PLR_COMBINE) )
        {
		/*
		 * Look for duplicates, case sensitive.
         * Matches tend to be near end so run loop backwards.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
            if ( ( !strcmp( prgpstrShow[iShow], pstrShow )
                 || lastprop[iShow] == prop )
               && prop->item_type != ITEM_MONEY )
		    {
            ++prgnShow[iShow];
            lastprop[iShow] = prop;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = 1;
        lastprop     [nShow] = prop;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
    if ( NPC(ch) || IS_SET(ch->flag2, PLR_COMBINE) )
	{
        if ( prgnShow[iShow] > 1 )
	    {
        if (fShort) snprintf( buf, MAX_STRING_LENGTH, "%s ", numberize( prgnShow[iShow] ) );
        else
        {
            char *pt;
            pt = STR(lastprop[iShow], description_plural);

            if ( pt == NULL || pt[0] == '\0' )
            {
                snprintf( buf, MAX_STRING_LENGTH, "%s %s are here.\n\r", 
                numberize( prgnShow[iShow] ), pluralize( STR(lastprop[iShow],short_descr) ) );
                buf[0] = UPPER(buf[0]);
            }
            else
            {
                buf[0] = '\0';
            }
        }
        page_to_actor( buf, ch );
	    }
	}

    if (fShort)
    {
      page_to_actor( prgnShow[iShow] > 1 ? pluralize( STR(lastprop[iShow],short_descr) ) :
                    prgpstrShow[iShow], ch );
      page_to_actor( "\n\r", ch );
    }
    else
    {
        if ( buf[0] == '\0' && prgnShow[iShow] > 1 )
        {
            snprintf( buf, MAX_STRING_LENGTH, STR(lastprop[iShow], description_plural), numberize( prgnShow[iShow] ) );
            page_to_actor( capitalize(buf), ch );
        }
        else
        if ( prgnShow[iShow] <= 1 )
        page_to_actor( capitalize(prgpstrShow[iShow]), ch );
    }
        free_string( prgpstrShow[iShow] );
        lastprop[iShow] = NULL;
    }

    if ( fShowNothing && nShow == 0 )
    {
    if ( NPC(ch) || IS_SET(ch->flag2, PLR_COMBINE) )
        page_to_actor( "     ", ch );
    page_to_actor( "Nothing.\n\r", ch );
    }

    /*
     * Clean up.
     */
    free_mem( prgpstrShow, count * sizeof(char *)      );
    free_mem( prgnShow,    count * sizeof(int)         );
    free_mem( lastprop,     count * sizeof(PROP *)  );

    return;
}



/*
 *  Shows a actor/player to character (look in scene)
 */
void show_actor_to_actor_0( PLAYER *victim, PLAYER *ch )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char *p;

    buf[0] = '\0';
    if ( victim == NULL || ch == NULL ) return;

    if ( victim->rider != NULL )
        return;

    if ( victim->riding == ch )
        return;



    /*
     * FOR HOODS by Morgenes, Ported by Daurven, Mods by Locke
     */
    if ( !is_hooded( victim ) ) {
        if ( victim->position == POS_STANDING
        && victim->riding   == NULL
        && victim->furniture == NULL
        && STR(victim, long_descr)[0] != '\0' ) {
            strcat( buf, "   " );
            strcat( buf, STR(victim, long_descr) );

            p = format_string( str_dup( capitalize(buf) ) );
            STC( p, ch );
            free_string( p );
         return;
  }
    } else {
        if ( victim->position == POS_STANDING
        && victim->riding   == NULL
        && victim->furniture == NULL ) {
            strcat( buf, "   " );
            strcat( buf, "There is a hooded figure nearby." );

            p = format_string( str_dup( capitalize(buf) ) );
            STC( p, ch );
            free_string( p );
         return;
  }
 }

/*    send_to_actor( "ACTOR/PLR ", ch );       */

    if ( IS_AFFECTED(victim, BONUS_INVISIBLE)   ) strcat( buf, "(Invisible) "   );
    if ( IS_AFFECTED(victim, BONUS_HIDE)        ) strcat( buf, "(Hiding) "      );
    if ( IS_AFFECTED(victim, BONUS_CHARM)       ) strcat( buf, "(Charmed) "     );
    if ( IS_AFFECTED(victim, BONUS_PASS_DOOR)   ) strcat( buf, "(Translucent) " );
    if ( IS_AFFECTED(victim, BONUS_FAERIE_FIRE) ) strcat( buf, "(Pink Aura) "   );
    if ( IS_AFFECTED(victim, BONUS_SANCTUARY)   ) strcat( buf, "(White Aura) "  );
    if ( !NPC(victim)
      && IS_AFFECTED(victim, BONUS_FLYING)      ) strcat( buf, "(Floating) "    );
    if ( !NPC(victim)
      && victim->desc == NULL                 ) strcat( buf, "(Linkless) "    );

    if ( victim->position == POS_STANDING
      && victim->riding   == NULL
      && victim->furniture == NULL
      && STR(victim, long_descr)[0] != '\0' )
    {
    strcat( buf, STR(victim, long_descr) );

    p = format_string( str_dup( capitalize(buf) ) );
    page_to_actor( p, ch );
    free_string( p );

	return;
    }

    strcat( buf, capitalize( PERS( victim, ch ) ) );

    switch ( victim->position )
    {
    case POS_DEAD:     strcat( buf, " is dead" );               break;
    case POS_MORTAL:   strcat( buf, " is mortally wounded" );   break;
    case POS_INCAP:    strcat( buf, " is incapacitated" );      break;
    case POS_STUNNED:  strcat( buf, " is lying here stunned" ); break;
    case POS_SLEEPING: strcat( buf, " is sleeping" );           break;
    case POS_SITTING:  strcat( buf, " is sitting" );            break;
    case POS_RESTING:  strcat( buf, " is resting" );            break;
    case POS_STANDING:
     if ( victim->rider == ch ) strcat( buf, " that you are riding" );
     strcat( buf, " is here" );
     break;
    case POS_FIGHTING:
      strcat( buf, " is here, fighting " );
      if ( victim->fighting == NULL )
          strcat( buf, "thin air" );
      else if ( victim->fighting == ch )
          strcat( buf, "YOU" );
      else if ( victim->in_scene == victim->fighting->in_scene )
      {
          strcat( buf, PERS( victim->fighting, ch ) );
      }
      else
          strcat( buf, "someone who left" );
     break;
    }

    if ( victim->riding != NULL && victim->riding->in_scene == victim->in_scene )
    {
        sprintf( buf2, ", mounted on %s.\n\r",
                          PERS(victim->riding, ch) );
        strcat( buf, buf2 );
    }
    else
    {
    if ( victim->furniture != NULL )
    {
        sprintf( buf2, " on %s.\n\r", can_see_prop( ch, victim->furniture ) ?
                                      STR(victim->furniture, short_descr)  :
                                      "something" );
        strcat( buf, buf2 );
    }
    else
    strcat( buf, ".\n\r" );
    }

    p = format_string( str_dup(capitalize(buf)) );
    page_to_actor( p, ch );
    free_string( p );

    if ( victim->hitched_to != NULL && can_see_prop( ch, victim->hitched_to ) )
    {
        snprintf( buf, MAX_STRING_LENGTH, "  %s has been hitched to %s.\n\r", NAME(victim),
                 STR(victim->hitched_to,short_descr) );
        page_to_actor( buf, ch );
    }
    return;
}



/*
 * Shows the generated prop description to a character.
 * (Once again I must say, Thanks to Kalgen for this AWESOME idea)
 * Thanks to Morgenes for the bug fixes/improvements.
 *                                    - Locke
 * Locations not displayed:
 * on belt (sheathed)
 * stockings (if wearing pants/leggings)
 * nose (if wearing something on face)
 * ankles (if wearing stockings)
 * shirt (if body wear)
 * loins (if wearing pants/leggings)
 */
void show_equipment( PLAYER *ch, PLAYER *tch )
{
   PROP *prop;
   PROP *prop2;
   char buf[MAX_STRING_LENGTH];
   char descr[MAX_STRING_LENGTH];
   char *finalstr;
   bool fBool;
   int oType;
   char *p1;
   char *p2;

   buf[0] = '\0';
   descr[0] = '\0';

   fBool = FALSE;

   p1 = NULL;
   p2 = NULL;

   if( ( ( prop = get_eq_char( ch, WEAR_FINGER_L ) ) != NULL
    && can_see_prop( tch, prop ) )
    && ( ( prop2 = get_eq_char( ch, WEAR_FINGER_R ) ) != NULL
    && can_see_prop( tch, prop2 ) )
    && !strcmp( (p1 = str_dup(format_prop_to_actor(prop,  ch, TRUE))),
                (p2 = str_dup(format_prop_to_actor(prop2, ch, TRUE))) ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s is wearing two %s on %s fingers",
                    HE_SHE( ch ),  pluralize( STR(prop,short_descr) ),   HIS_HER( ch ) );
      strcat( descr, buf );
   }
   else
   {
      if( ( prop = get_eq_char( ch, WEAR_FINGER_L ) ) != NULL
       && can_see_prop( tch, prop ) )
      {
         snprintf( buf, MAX_STRING_LENGTH, "On %s left hand, %s has placed %s",
          HIS_HER( ch ), HE_SHE( ch ), PERSO(prop, ch) );
         strcat( descr, buf );
         fBool = TRUE;
      }
     
      if( ( prop = get_eq_char( ch, WEAR_FINGER_R ) ) != NULL
       && can_see_prop( tch, prop ) )
      { 
         if( fBool )
            snprintf( buf, MAX_STRING_LENGTH, ", while %s encircles a finger on %s right",
             PERSO(prop, ch), HIS_HER( ch ) );
         else
            snprintf( buf, MAX_STRING_LENGTH, "On %s right hand %s %s",
             HIS_HER( ch ), is_are( PERSO(prop, ch) ), PERSO(prop, ch) );
         strcat( descr, buf );
      }
   }

   free_string( p1 );
   free_string( p2 );
   p1 = NULL;
   p2 = NULL;

   if( ( prop = get_eq_char( ch, WEAR_HANDS ) ) != NULL
    && can_see_prop( tch, prop ) )
   {
      if( descr[0] != '\0' )
         snprintf( buf, MAX_STRING_LENGTH, ", with %s covering %s hands.",
          format_prop_to_actor(prop, ch, TRUE), HIS_HER(ch) );
      else
         snprintf( buf, MAX_STRING_LENGTH, "%s %s worn on %s hands.",
          format_prop_to_actor(prop, ch, TRUE), is_are( format_prop_to_actor(prop, ch, TRUE) ), HIS_HER( ch ) );
      strcat( descr, buf );
   }
   else if( descr[0] != '\0' )
      strcat( descr, ".  " );

   fBool = FALSE;

   if( ( prop = get_eq_char( ch, WEAR_HEAD ) ) != NULL
    && can_see_prop( tch, prop ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s is wearing %s on %s head.  ",
       HE_SHE( ch ), format_prop_to_actor(prop, ch, TRUE), HIS_HER( ch ) );
      strcat( descr, buf );
   }

   if( ( prop = get_eq_char( ch, WEAR_FOREHEAD ) ) != NULL
    && can_see_prop( tch, prop ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "Around %s forehead, %s has wrapped %s.",
       HIS_HER(ch), HE_SHE(ch), PERSO(prop, ch) );
      strcat( descr, buf );
   }

   if( ( ( prop = get_eq_char( ch, WEAR_NECK_1 ) ) != NULL
    && can_see_prop( tch, prop ) )
    && ( ( prop2 = get_eq_char( ch, WEAR_NECK_2 ) ) != NULL
    && can_see_prop( tch, prop2 ) )
    && !str_cmp( (p1 = str_dup(format_prop_to_actor(prop,  ch, TRUE))),
                 (p2 = str_dup(format_prop_to_actor(prop2, ch, TRUE))) ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "Two %s are clasped around %s neck.  ",
       pluralize( STR(prop, short_descr) ), HIS_HER( ch ) );
      strcat( descr, buf );
   }
   else
   {
      if( ( prop = get_eq_char( ch, WEAR_NECK_1 ) ) != NULL
       && can_see_prop( tch, prop ) )
      {
         if( get_eq_char( ch, WEAR_NECK_2 ) == NULL )
            snprintf( buf, MAX_STRING_LENGTH, "%s %s worn about %s neck.  ",
             PERSO(prop,ch), is_are( PERSO(prop,ch) ),
             HIS_HER( ch ) );
         else
            snprintf( buf, MAX_STRING_LENGTH, "%s and ", PERSO(prop,ch) );
         strcat( descr, buf );
         fBool = TRUE;
      }
   
      if( ( prop = get_eq_char( ch, WEAR_NECK_2 ) ) != NULL
       && can_see_prop( tch, prop ) )
      {
         if( fBool )
            snprintf( buf, MAX_STRING_LENGTH, "%s are worn about %s neck.  ",
             PERSO(prop,ch), HIS_HER( ch ) );
         else
            snprintf( buf, MAX_STRING_LENGTH, "%s %s worn about %s neck.  ",
             PERSO(prop,ch), is_are( PERSO(prop,ch) ), HIS_HER( ch ) );
         strcat( descr, buf );
      }
   }

   free_string( p1 );
   free_string( p2 );
   p1 = NULL;
   p2 = NULL;

   fBool = FALSE;

   if( ( ( prop = get_eq_char( ch, WEAR_EAR_L ) ) != NULL
    && can_see_prop( tch, prop ) )
    && ( ( prop2 = get_eq_char( ch, WEAR_EAR_R ) ) != NULL
    && can_see_prop( tch, prop2 ) )
    && !str_cmp( (p1 = str_dup(format_prop_to_actor(prop,  ch, TRUE))),
                 (p2 = str_dup(format_prop_to_actor(prop2, ch, TRUE))) ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s has a pair of %s on %s ears.",
       HE_SHE( ch ), pluralize( STR(prop, short_descr) ), HIS_HER( ch ) );
      strcat( descr, buf );
   } 
   else
   {
      if( ( prop = get_eq_char( ch, WEAR_EAR_L ) ) != NULL
       && can_see_prop( tch, prop ) )
      {
         snprintf( buf, MAX_STRING_LENGTH, "%s pierces %s left ear",
          PERSO(prop,ch), HIS_HER( ch ) );
         strcat( descr, buf );
         fBool = TRUE;
      }
      if( ( prop = get_eq_char( ch, WEAR_EAR_R ) ) != NULL
       && can_see_prop( tch, prop ) )
      {
         if( fBool )
            snprintf( buf, MAX_STRING_LENGTH, " and %s %s worn on %s right",
             PERSO(prop,ch), is_are( PERSO(prop,ch) ), HIS_HER( ch ) );
         else
            snprintf( buf, MAX_STRING_LENGTH, "%s right ear is pierced with %s",
             HIS_HER( ch ), PERSO(prop,ch) );
         strcat( descr, buf );
      }
      if( fBool || get_eq_char( ch, WEAR_EAR_R ) )
         strcat( descr, ".  " );
   }

   free_string( p1 );
   free_string( p2 );
   p1 = NULL;
   p2 = NULL;

   if( ( prop = get_eq_char( ch, WEAR_NOSE ) ) != NULL
    && can_see_prop( tch, prop )
    && get_eq_char( ch, WEAR_FACE ) == NULL )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s %s piercing %s nose.",
       format_prop_to_actor(prop, ch, TRUE), is_are( format_prop_to_actor(prop, ch, TRUE) ), HIS_HER( ch ) );
      strcat( descr, buf );
   }

   if( ( prop = get_eq_char( ch, WEAR_FACE ) ) != NULL
    && can_see_prop( tch, prop ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s covers %s face.", format_prop_to_actor(prop, ch, TRUE), HIS_HER( ch ) );
      strcat( descr, buf );
   }

   fBool = FALSE;
   
   if( ( prop = get_eq_char( ch, WEAR_ABOUT ) ) != NULL
    && can_see_prop( tch, prop ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s %s draped over %s ",
       format_prop_to_actor(prop, ch, TRUE), is_are( format_prop_to_actor(prop, ch, TRUE) ), HIS_HER( ch ) );
      strcat( descr, buf );
      if( ( prop = get_eq_char( ch, WEAR_BODY ) ) != NULL
       && can_see_prop( tch, prop ) )
      {
         snprintf( buf, MAX_STRING_LENGTH, "%s.  ",
          smash_article( format_prop_to_actor(prop, ch, TRUE) ) );
      }
      else
      if( ( prop = get_eq_char( ch, WEAR_SHIRT ) ) != NULL
       && can_see_prop( tch, prop ) )
      {
         snprintf( buf, MAX_STRING_LENGTH, "%s.  ",
          smash_article( format_prop_to_actor(prop, ch, TRUE) ) );
      }
      else
         snprintf( buf, MAX_STRING_LENGTH, "naked chest.  " );
      strcat( descr, buf );
   }
   else if( ( prop = get_eq_char( ch, WEAR_BODY ) ) != NULL 
    && (get_eq_char( tch, WEAR_SHIRT ) == NULL 
     || get_eq_char( tch, WEAR_PANTS ) == NULL)
    && can_see_prop( tch, prop ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s %s being worn on %s body.",
       format_prop_to_actor(prop, ch, TRUE), is_are( format_prop_to_actor(prop, ch, TRUE) ), HIS_HER( ch ) );
      strcat( descr, buf );
   }
   
   if( ( prop = get_eq_char( ch, WEAR_SHIRT ) ) != NULL
    && !get_eq_char( ch, WEAR_ABOUT ) 
    && can_see_prop( tch, prop ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s fits over %s torso.",
       format_prop_to_actor(prop, ch, TRUE), HIS_HER( ch ) );
      strcat( descr, buf );
   }
  
   if( ( prop = get_eq_char( ch, WEAR_ARMS ) ) != NULL
    && can_see_prop( tch, prop ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s %s protecting %s arms.  ",
       format_prop_to_actor(prop, ch, TRUE), is_are( format_prop_to_actor(prop, ch, TRUE) ), HIS_HER( ch ) );
      strcat( descr, buf );
   }

   if( ( prop = get_eq_char( ch, WEAR_PANTS ) ) != NULL
    && can_see_prop( tch, prop )
    && get_eq_char( ch, WEAR_LEGS ) == NULL )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s is wearing %s on %s legs.  ",
       HE_SHE( ch ), format_prop_to_actor(prop, ch, TRUE), HIS_HER( ch ) );
      strcat( descr, buf );
   }

   if( ( prop2 = get_eq_char( ch, WEAR_LEGS ) ) != NULL
    && can_see_prop( tch, prop2 ) )
   {
       prop = get_eq_char( ch, WEAR_PANTS );
      if ( prop == NULL )
      snprintf( buf, MAX_STRING_LENGTH, "%s is wearing %s on %s legs.  ",
       HE_SHE( ch ), format_prop_to_actor(prop2, ch, TRUE), HIS_HER( ch ) );
      else
      snprintf( buf, MAX_STRING_LENGTH, "%s %s worn over %s on %s legs.  ",
       format_prop_to_actor(prop2, ch, TRUE), is_are( format_prop_to_actor(prop2, ch, TRUE) ),
       format_prop_to_actor(prop, ch, TRUE), HIS_HER( ch ) );

      strcat( descr, buf );
   }

   if( ( prop = get_eq_char( ch, WEAR_STOCKING ) ) != NULL
    && can_see_prop( tch, prop )
    && get_eq_char( ch, WEAR_PANTS ) == NULL )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s has pulled %s onto %s legs.  ",
       HE_SHE( ch ), format_prop_to_actor(prop, ch, TRUE), HIS_HER( ch ) );
      strcat( descr, buf );
   }

   if( ( prop = get_eq_char( ch, WEAR_FEET ) ) != NULL
    && can_see_prop( tch, prop ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s wears %s as footwear.  ",
       HE_SHE( ch ), format_prop_to_actor(prop, ch, TRUE) );
      strcat( descr, buf );
   }

   if( descr[0] != '\0' )
   {
      send_to_actor( "\n\r", tch );
      send_to_actor( "   ", tch );

      finalstr = format_string( str_dup( descr ) );
      send_to_actor( finalstr, tch );
      free_string( finalstr );

      descr[0] = '\0';
   }

   if( ( ( prop = get_eq_char( ch, WEAR_WRIST_L ) ) != NULL
    && can_see_prop( tch, prop ) )
    && ( ( prop2 = get_eq_char( ch, WEAR_WRIST_R ) ) != NULL
    && can_see_prop( tch, prop2 ) )
    && !str_cmp( (p1 = str_dup(format_prop_to_actor(prop,  ch, TRUE))),
                 (p2 = str_dup(format_prop_to_actor(prop2, ch, TRUE))) ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "Two %s are on each wrist.  ", pluralize( STR(prop, short_descr) ) );
      strcat( descr, buf );
   }
   else
   {
      if( ( prop = get_eq_char( ch, WEAR_WRIST_L ) ) != NULL
       && can_see_prop( tch, prop ) )
      {
         snprintf( buf, MAX_STRING_LENGTH, "Encircling %s left wrist, %s %s",
          HIS_HER( ch ), is_are( format_prop_to_actor(prop, ch, TRUE) ), format_prop_to_actor(prop, ch, TRUE) );
         strcat( descr, buf );
         fBool = TRUE;
      }
    
      if( ( prop = get_eq_char( ch, WEAR_WRIST_R ) ) != NULL
       && can_see_prop( tch, prop ) )
      {
         if( fBool )
            snprintf( buf, MAX_STRING_LENGTH, " and %s on %s right.  ",
             format_prop_to_actor(prop, ch, TRUE), HIS_HER( ch ) );
         else
            sprintf( buf,
             "Encircling %s right wrist %s %s.  ",
             HIS_HER( ch ), is_are( format_prop_to_actor(prop, ch, TRUE) ), format_prop_to_actor(prop, ch, TRUE) );
         strcat( descr, buf );
      }
      else if( fBool )
         strcat( descr, ".  " );
   }

   free_string( p1 );
   free_string( p2 );
   p1 = NULL;
   p2 = NULL;

   if ( get_eq_char( ch, WEAR_STOCKING ) == NULL )
   {

   fBool = FALSE;

   if( ( ( prop = get_eq_char( ch, WEAR_ANKLE_L ) ) != NULL
    && can_see_prop( tch, prop ) )
    && ( ( prop2 = get_eq_char( ch, WEAR_ANKLE_R ) ) != NULL
    && can_see_prop( tch, prop2 ) )
    && !str_cmp( (p1 = str_dup(format_prop_to_actor(prop,  ch, TRUE))),
                 (p2 = str_dup(format_prop_to_actor(prop2, ch, TRUE))) ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s has two %s fastened around %s ankles.",
       HE_SHE( ch ), pluralize( STR(prop, short_descr) ), HIS_HER( ch ) );
      strcat( descr, buf );
   } 
   else
   {
      if( ( prop = get_eq_char( ch, WEAR_ANKLE_L ) ) != NULL
       && can_see_prop( tch, prop ) )
      {
         snprintf( buf, MAX_STRING_LENGTH, "%s %s fastened around %s left ankle",
          format_prop_to_actor(prop, ch, TRUE), is_are( format_prop_to_actor(prop, ch, TRUE) ), HIS_HER( ch ) );
         strcat( descr, buf );
         fBool = TRUE;
      }
      if( ( prop = get_eq_char( ch, WEAR_ANKLE_R ) ) != NULL
       && can_see_prop( tch, prop ) )
      {
         if( fBool )
            snprintf( buf, MAX_STRING_LENGTH, " and %s %s around %s right",
             format_prop_to_actor(prop, ch, TRUE), is_are( format_prop_to_actor(prop, ch, TRUE) ), HIS_HER( ch ) );
         else
            snprintf( buf, MAX_STRING_LENGTH, "Around %s right ankle %s %s",
            HIS_HER( ch ), is_are( format_prop_to_actor(prop, ch, TRUE) ), format_prop_to_actor(prop, ch, TRUE) );
         strcat( descr, buf );
      }
      if( fBool || get_eq_char( ch, WEAR_ANKLE_R ) )
         strcat( descr, ".  " );
   }

   }

   fBool = FALSE;

   if( ( ( prop = get_eq_char( ch, WEAR_SHOULDER_L ) ) != NULL
    && can_see_prop( tch, prop ) )
    && ( ( prop2 = get_eq_char( ch, WEAR_SHOULDER_R ) ) != NULL
    && can_see_prop( tch, prop2 ) )
    && !str_cmp( (p1 = str_dup(format_prop_to_actor(prop,  ch, TRUE))),
                 (p2 = str_dup(format_prop_to_actor(prop2, ch, TRUE))) ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s has two %s slung over %s shoulders.",
       HE_SHE( ch ), pluralize( STR(prop,short_descr) ), HIS_HER( ch ) );
      strcat( descr, buf );
   } 
   else
   {
      if( ( prop = get_eq_char( ch, WEAR_SHOULDER_L ) ) != NULL
       && can_see_prop( tch, prop ) )
      {
         snprintf( buf, MAX_STRING_LENGTH, "%s %s slung over %s left shoulder",
          format_prop_to_actor(prop, ch, TRUE), is_are( format_prop_to_actor(prop, ch, TRUE) ), HIS_HER( ch ) );
         strcat( descr, buf );
         fBool = TRUE;
      }
      if( ( prop = get_eq_char( ch, WEAR_SHOULDER_R ) ) != NULL
       && can_see_prop( tch, prop ) )
      {
         if( fBool )
            snprintf( buf, MAX_STRING_LENGTH, " and %s %s slung over %s right",
             format_prop_to_actor(prop, ch, TRUE), is_are( format_prop_to_actor(prop, ch, TRUE) ), HIS_HER( ch ) );
         else
            snprintf( buf, MAX_STRING_LENGTH, "Over %s right shoulder %s %s",
            HIS_HER( ch ), is_are( format_prop_to_actor(prop, ch, TRUE) ), format_prop_to_actor(prop, ch, TRUE) );
         strcat( descr, buf );
      }
      if( fBool || get_eq_char( ch, WEAR_SHOULDER_R ) )
         strcat( descr, ".  " );
   }

   free_string( p1 );
   free_string( p2 );
   p1 = NULL;
   p2 = NULL;

   fBool = FALSE;

   prop  = get_eq_char( ch, WEAR_HOLD_1 );
   if ( !prop )  prop  = get_eq_char( ch, WEAR_WIELD_1 );

   prop2 = get_eq_char( ch, WEAR_HOLD_2 );
   if ( !prop2 ) prop2 = get_eq_char( ch, WEAR_WIELD_2 );

   if( prop != NULL     && can_see_prop( tch, prop )
    && prop2 != NULL    && can_see_prop( tch, prop2 )
    && (prop->wear_loc == prop2->wear_loc-1)
    && !str_cmp( (p1 = str_dup(format_prop_to_actor(prop,  ch, TRUE))),
                 (p2 = str_dup(format_prop_to_actor(prop2, ch, TRUE))) ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "Two %s are %s in %s hands.",
                       pluralize( STR(prop, short_descr) ),
                       prop->wear_loc == WEAR_WIELD_1 ? "wielded" : "held",
                       HIS_HER( ch ) );
      strcat( descr, buf );
   }
   else
   if ( prop != NULL 
        && can_see_prop( tch, prop ) 
        && prop->wear_loc == WEAR_WIELD_1
        && IS_SET(prop->wear_flags, ITEM_TWO_HANDED) )
   {
        snprintf( buf, MAX_STRING_LENGTH, "%s %s gripped in both %s hands.\n\r",
                      format_prop_to_actor( prop, ch, TRUE ),
                      is_are( format_prop_to_actor( prop, ch, TRUE ) ),
                      HIS_HER( ch ) );
        strcat( descr, buf );
   }
   else  
   {
      oType = ITEM_TRASH;
      if( prop != NULL && can_see_prop( tch, prop ) )
      {
         snprintf( buf, MAX_STRING_LENGTH, "%s %s %s in one hand",
             format_prop_to_actor(prop, ch, TRUE),
             is_are( format_prop_to_actor(prop, ch, TRUE) ),
             prop->wear_loc == WEAR_WIELD_1 ? "being wielded" : "held" );
         strcat( descr, buf );
         fBool = TRUE;
      }

      if( prop2 != NULL && can_see_prop( tch, prop2 ) )
      {
         if( fBool )
         {
            if ( prop->wear_loc == (prop2->wear_loc-1) )
            snprintf( buf, MAX_STRING_LENGTH, ", with %s in the other.",
                     format_prop_to_actor(prop2, ch, TRUE) );
            else
            snprintf( buf, MAX_STRING_LENGTH, ", and %s %s %s in the other.  ",
                format_prop_to_actor(prop2, ch, TRUE),
                is_are( format_prop_to_actor(prop2, ch, TRUE) ),
                prop2->wear_loc == WEAR_WIELD_2 ? "wielded" : "held" );
         }
         else
         {
            snprintf( buf, MAX_STRING_LENGTH, "%s %s being %s.  ",
                format_prop_to_actor(prop2, ch, TRUE),
                is_are( format_prop_to_actor(prop2, ch, TRUE) ),
                prop2->wear_loc == WEAR_WIELD_2 ? "wielded" : "held" );
         } 
         strcat( descr, buf );
      }
      else
      if( fBool ) strcat( descr, ".  " );
   }

   free_string( p1 );
   free_string( p2 );
   p1 = NULL;
   p2 = NULL;

   if( ( prop = get_eq_char( ch, WEAR_BACK ) ) != NULL
    && can_see_prop( tch, prop ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s is strapped to %s back.  ",
        format_prop_to_actor(prop, ch, TRUE), HIS_HER( ch ) );
      strcat( descr, buf );
   }
   
   fBool = FALSE;
   
   if( ( prop = get_eq_char( ch, WEAR_FLOATING ) ) != NULL
    && can_see_prop( tch, prop ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s is floating near %s head.  ",
       format_prop_to_actor(prop, ch, TRUE), HIS_HER( ch ) );
      strcat( descr, buf );
   }

   if( ( prop = get_eq_char( ch, WEAR_WAIST ) ) != NULL
    && can_see_prop( tch, prop )
    && (get_eq_char( ch, WEAR_LOIN ) == NULL 
     && get_eq_char( ch, WEAR_PANTS ) == NULL) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s fits snugly around %s waist.  ",
       format_prop_to_actor(prop, ch, TRUE), HIS_HER( ch ) );
      strcat( descr, buf );
   }

   if( ( prop = get_eq_char( ch, WEAR_SHOULDERS ) ) != NULL
    && can_see_prop( tch, prop ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s is wearing %s.  ",
       format_prop_to_actor(prop, ch, TRUE), HIS_HER( ch ) );
      strcat( descr, buf );
   }

   if( ( prop2 = get_eq_char( ch, WEAR_LOIN ) ) != NULL
    && can_see_prop( tch, prop2 )
    && get_eq_char( ch, WEAR_PANTS ) == NULL )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s %s wrapped loosely below %s %s.  ",
       format_prop_to_actor(prop2, ch, TRUE), is_are( format_prop_to_actor(prop2, ch, TRUE) ),
       HIS_HER( ch ), prop ? smash_article(format_prop_to_actor(prop, ch, TRUE)) : "waist" );
      strcat( descr, buf );
   }

   if( ( prop = get_eq_char( ch, WEAR_SHIELD ) ) != NULL
    && can_see_prop( tch, prop ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s serves to protect %s from attacks.  ",
        format_prop_to_actor(prop, ch, TRUE), HIM_HER( ch ) );
      strcat( descr, buf );
   }
   
   if( descr[0] != '\0' )
   {
      send_to_actor( "\n\r", tch );
      send_to_actor( "   ", tch );

      finalstr = format_string( str_dup( descr ) );
      send_to_actor( finalstr, tch );
      free_string( finalstr );
   }

   return;
}



void show_equipment_table( PLAYER *ch, PLAYER *victim )
{
    PROP *prop;
    char buf[MAX_STRING_LENGTH];
    int iWear;
    bool found = FALSE;

    if ( ch != victim )
       act( "$N is using:", ch, NULL, victim, TO_ACTOR );
  else send_to_actor( "You are using:\n\r", ch );

    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {

    if ( ( prop = get_eq_char( victim, iWear ) ) == NULL )
        continue;

    snprintf( buf, MAX_STRING_LENGTH, "%-24s %s\n\r", iWear == WEAR_WIELD_1
                               && IS_SET(prop->wear_flags, ITEM_TWO_HANDED)
                                 ? "<wielded both hands>" : where_name[iWear],
                                  can_see_prop( ch, prop ) ?
                                      format_prop_to_actor(prop, ch, TRUE)
                                      : "something" );
    page_to_actor( buf, ch );
	found = TRUE;
    }

    if ( !found )  page_to_actor( "     Nothing.\n\r", ch );
    return;
}



/*
 * Shows the actually character (look at someone)
 */
void show_actor_to_actor_1( PLAYER *victim, PLAYER *ch )
{
    char buf[MAX_STRING_LENGTH];
    char final[MAX_STRING_LENGTH];

    if ( can_see( victim, ch ) )
    {
         act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
         if ( ch != victim )
         act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
         else act( "$n looks $mself over.", ch, NULL, NULL, TO_NOTVICT );
    }

    if ( !MTD(STR(victim,description)) )
    send_to_actor( STR(victim, description), ch );

    show_equipment( victim, ch );

    send_to_actor( "\n\r",   ch );

    final[0] = '\0';

    snprintf( buf, MAX_STRING_LENGTH, "     %s looks %s.\n\r",
                        capitalize(PERS( victim, ch )),
                                  STRING_HITS(victim) );
    strcat( final, buf );

    switch ( victim->position )
    {
        case POS_DEAD:
          snprintf( buf, MAX_STRING_LENGTH, "%s is dead. ", capitalize(PERS(victim,ch)) );
          strcat( final, buf );
         break;
        case POS_MORTAL:
          snprintf( buf, MAX_STRING_LENGTH, "%s has wounds that may bring death. ",
                   capitalize(PERS(victim,ch)) );
          strcat( final, buf );
         break;
        case POS_INCAP:
          snprintf( buf, MAX_STRING_LENGTH, "%s has been rendered incapacitated. ",
                   capitalize(PERS(victim,ch)) );
          strcat( final, buf );
         break;
        case POS_STUNNED:
          snprintf( buf, MAX_STRING_LENGTH, "%s is stunned. ",
                   capitalize(PERS(victim,ch)) );
          strcat( final, buf );
         break;
        case POS_SLEEPING:
          if ( victim->furniture )
          snprintf( buf, MAX_STRING_LENGTH, "%s is fast asleep upon %s. ",
                        capitalize(HE_SHE(victim)), PERSO(victim->furniture,ch) );
          else
          snprintf( buf, MAX_STRING_LENGTH, "%s is fast asleep. ",
                        capitalize(HE_SHE(victim)) );
          strcat( final, buf );
         break;
        case POS_SITTING:
          if ( victim->furniture )
          snprintf( buf, MAX_STRING_LENGTH, "%s is sitting on %s. ",
                        capitalize(HE_SHE(victim)), PERSO(victim->furniture,ch) );
          else
          snprintf( buf, MAX_STRING_LENGTH, "%s is sitting. ",
                        capitalize(HE_SHE(victim)) );
          strcat( final, buf );
         break;
        case POS_RESTING:
          if ( victim->furniture )
          snprintf( buf, MAX_STRING_LENGTH, "%s rests on %s. ",
                        capitalize(HE_SHE(victim)), PERSO(victim->furniture,ch) );
          else
          snprintf( buf, MAX_STRING_LENGTH, "%s is resting. ",
                        capitalize(HE_SHE(victim)) );
          strcat( final, buf );
         break;
        case POS_FIGHTING:
          if ( victim->fighting != NULL )
          {
          snprintf( buf, MAX_STRING_LENGTH, "     %s is fighting with %s.\n\r",
                        PERS( victim, ch ),  PERS( victim->fighting, ch ) );
          strcat( final, buf );
          }
         break;
          default:  break;
    }

if ( victim->size )
{
    snprintf( buf, MAX_STRING_LENGTH, "%s stands the height of %s %s",
             capitalize(HE_SHE(victim)),
               victim->size >= 4 ? numberize((victim->size*4)/12) : "less than one", 
               victim->size >= 4 ? "feet" : "foot" );

    strcat( final, buf );

    if ( ch->size != victim->size )
    {
        char *temp = str_dup(numberize( abs(ch->size - victim->size ) ));
        snprintf( buf, MAX_STRING_LENGTH, ", which is roughly %s handspans %s than you.\n\r",
             temp,
             ch->size - victim->size > 0 ? "shorter" : "taller" );
        strcat( final, buf );
        free_string(temp);
    }
    else
    strcat( final, ".\n\r" );
}
    if ( !NPC(victim) )
    {
        char *race_name;
        int age;
        int race;

        race = race_lookup( victim->race );

        if ( race == 0 )
        {
            age = 0;
            race_name = "human";
        }
        else
        {
            age = PERCENTAGE(GET_AGE(victim),RACE(race,base_age) );
            race_name = RACE(race,race_name);
        }

        snprintf( buf, MAX_STRING_LENGTH, "%s is a%s %s.\n\r",
                 capitalize(NAME(victim)),
                 age < 30 ? " young" :
                 age < 40 ? " matured" :
                 age < 60 ? " middle-aged" :
                 age < 80 ? "n aging" :
                 age < 90 ? "n elderly" : "n ancient",
                 race_name );
        strcat( final, buf );
    }
    
    if ( victim->riding != NULL )
    {
        snprintf( buf, MAX_STRING_LENGTH, "     %s rides atop %s.\n\r", HE_SHE(victim), NAME(victim->riding) );
	strcat( final, buf );
    }
    else
    if ( victim->rider != NULL )
    {
        snprintf( buf, MAX_STRING_LENGTH, "     %s is being ridden by %s.\n\r", HE_SHE(victim), capitalize(NAME(victim->rider)) );
        strcat( final, buf );
    }

    if ( GET_PC(victim,condition[COND_DRUNK],0) > 50 )
    {
        snprintf( buf, MAX_STRING_LENGTH, " %s looks a bit tipsy and smells of drink.\n\r",
                 HE_SHE(victim) );
        strcat( final, buf );
    }

	{
		char *p;
                p = format_string( str_dup(final) );
		send_to_actor( "   ", ch );
		send_to_actor( p, ch );
		free_string( p );
	}

    send_to_actor( "\n\r", ch );

    if ( (IS_IMMORTAL(ch) && IS_SET(ch->flag2, WIZ_EQUIPMENT)) || 
         (victim->master == ch && NPC(victim)) )
    {
    display_interp( ch, color_table[PC(ch,colors[COLOR_PROP_LIST])].di );
       show_equipment_table( ch, victim );
       page_to_actor( "\n\r", ch );
       show_inventory( victim, ch, FALSE );
    display_interp( ch, "^N" );
    }

    return;
}


 /*
  *  Show whose in the scene (all actors/players)
  */
void show_actor_to_actor( PLAYER *list, PLAYER *ch )
{
    PLAYER *rch;
    bool fDark = 0;

    for ( rch = list; rch != NULL; rch = rch->next_in_scene )
    {
        if ( rch == ch )
            continue;

        if ( !NPC(rch)
        &&   GET_PC(rch,wizinvis,0) >= GET_PC(ch,level,0)
        &&   get_trust( ch ) < get_trust( rch ) )
             continue;

        if ( can_see( ch, rch ) )
        {
             show_actor_to_actor_0( rch, ch );
        }
        else
        if ( scene_is_dark( ch->in_scene ) && IS_AFFECTED(rch, BONUS_INFRARED) )
        fDark++;
    }

    if ( fDark > 0 )
    {
        char buf[MAX_STRING_LENGTH];

        snprintf( buf, MAX_STRING_LENGTH, "%s red eyes stare at you from the darkness.\n\r",
                     capitalize( numberize( fDark * 2 ) ) );
        send_to_actor( buf, ch );
    }
    return;
}

 
bool check_blind( PLAYER *ch )
{
    if ( !NPC(ch) && IS_SET(ch->flag2, WIZ_HOLYLIGHT) )
    return TRUE;

    if ( IS_AFFECTED(ch, BONUS_BLIND) )
    {
    send_to_actor( "It is impossible to see a thing!\n\r", ch );
    return FALSE;
    }

    return TRUE;
}

bool actor_look_list( PLAYER *ch, PROP *list, char *arg )
{
    PROP *prop;
    char *pdesc;
    char buf[MAX_STRING_LENGTH];
    char argnew[MAX_STRING_LENGTH];
    int count;
    int number;
    
    number = number_argument( arg, argnew );
    count  = 0;

    for ( prop = list; prop != NULL; prop = prop->next_content )
    {
        if ( can_see_prop( ch, prop ) )
        {
            pdesc = get_extra_descr( arg, prop->extra_descr );
            if ( pdesc == NULL )
            pdesc = get_extra_descr( arg, prop->pIndexData->extra_descr );

            if ( pdesc != NULL )
            {
                    page_to_actor( pdesc, ch );
                    return TRUE;
            }
        }

        if ( is_name( argnew, STR(prop, name) ) && ++count == number )
        {
            char fub[MAX_STRING_LENGTH];
            
            page_to_actor( STR(prop, real_description), ch );
            switch ( prop->item_type )
            {
              default:
                break;

              case ITEM_LIGHT:
                 if( prop->value[1] )
                 {
                     int percent = PERCENTAGE( prop->value[0], prop->value[1] );

                     snprintf( buf, MAX_STRING_LENGTH, "\n\r%s %s %s\n\r",
                                  capitalize( STR(prop, short_descr) ),
                                  is_are( STR(prop, short_descr) ),
                       !IS_LIT(prop)  ? "extinguished."              :
                       percent <  10 ? "flickering and sputtering." :
                       percent <  20 ? "flickering."                :
                       percent <  30 ? "flickering slightly."       :
                       percent <  40 ? "providing ample light."     :
                                       "glowing brightly." );
                     send_to_actor( buf, ch );
                 }
              break;

              case ITEM_BOARD:
                  cmd_note( ch, "list" );
                break;

              case ITEM_DRINK_CON:
                 if ( prop->value[1] <= 0 )
                     {
                         send_to_actor( "It is empty.\n\r", ch );
                         break;
                     }
              break;

              case ITEM_RANGED_WEAPON:
               if ( prop->value[3] > 0 )
                  {
                      snprintf( buf, MAX_STRING_LENGTH, "It has %d round%s ready to fire.\n\r",
                               prop->value[3],
                               prop->value[3] > 1 ? "s" : "" );
                      send_to_actor( buf, ch );
                  }
                  else send_to_actor( "It is not readied.\n\r", ch );
              break;

              case ITEM_SPELLBOOK:
                if ( prop->carried_by == ch ) 
                    cmd_spellbook( ch, "" );                 
              break;

              case ITEM_ARMOR:
                    if( prop->value[1] )
                    {
                       int percent;

                       percent = PERCENTAGE( prop->value[1], prop->value[2] );
                       snprintf( buf, MAX_STRING_LENGTH, "\n\r%s %s\n\r",
                                     capitalize( STR(prop, short_descr) ),
                        percent <  10 ? "is in pieces." :
                        percent <  20 ? "is nearly destroyed." :
                        percent <  30 ? "is heavily damaged." :
                        percent <  40 ? "is in poor shape." :
                        percent <  50 ? "looks worn." :
                        percent <  60 ? "is starting to look rather worn." :
                        percent <  70 ? "looks well used." :
                        percent <  80 ? "is slightly damaged." :
                        percent <  90 ? "is in fair shape." :
                        percent < 100 ? "is in good shape." :
                                        "looks excellent." );
                       send_to_actor( buf, ch );
                    }
              case ITEM_CLOTHING:
              		if ( prop->carried_by != ch
              		  || prop->wear_loc <= WEAR_BELT_1 )
              		{
                    if ( TOO_BIG(ch, prop) )
                    send_to_actor( "It looks too big for you to wear.\n\r", ch );
               else if ( TOO_SMALL(ch, prop) )
                    send_to_actor( "It looks too small for you to wear.\n\r", ch );
               else send_to_actor( "You could probably wear it.\n\r", ch );
                    }
              break;

            case ITEM_GOODS:     send_to_actor( "A standard trade good.\n\r", ch ); break;
            case ITEM_COMPONENT: send_to_actor( "A common spell component.\n\r", ch ); break;

            case ITEM_GEM:
             if( prop->value[1] )
             {
                int percent = PERCENTAGE( prop->value[1], prop->value[2] );

                snprintf( buf, MAX_STRING_LENGTH, "\n\r%s %s\n\r",
                         capitalize( STR(prop, short_descr) ),
                         percent <  10 ? "is dimly flickering."   :
                         percent <  20 ? "is slowly pulsating."   :
                         percent <  30 ? "is quickly pulstating." :
                         percent <  40 ? "is glowing."            :
                         percent <  50 ? "glows brightly."
                                       : "radiates with energy."    );
                send_to_actor( buf, ch );
             }
            break;
            }

            if ( prop->item_type == ITEM_FURNITURE && prop->contains != NULL )
            {
                sprintf( fub, "On %s is ", format_prop_to_actor( prop, ch, TRUE ) );
                page_to_actor( show_list_to_actor2( prop->contains, ch, fub ), ch );
            }

            if ( has_occupant( prop ) )
            show_occupants_to_actor( prop, ch );
            return TRUE;
        }
    }

    return FALSE;
}



/*
 * The scan command.
 */
void cmd_scan( PLAYER *ch, char *argument )
{
       SCENE *pScene;
       bool fMatch = FALSE; 

       display_interp( ch, "^:^B" );

       pScene = ch->in_scene;
       if ( ch->in_scene == NULL ) return;

       if ( pScene->exit[DIR_NORTH] != NULL
        && pScene->exit[DIR_NORTH]->to_scene != NULL
        && pScene->exit[DIR_NORTH]->to_scene->people != NULL  ) {
       send_to_actor( "You glance north:\n\r", ch );
       cmd_look( ch, "north" );
       fMatch = TRUE;
       }

       if ( pScene->exit[DIR_SOUTH] != NULL
        && pScene->exit[DIR_SOUTH]->to_scene != NULL
        && pScene->exit[DIR_SOUTH]->to_scene->people != NULL  ) {
       send_to_actor( "You glance south:\n\r", ch );
       cmd_look( ch, "south" );
       fMatch = TRUE;
       }

       if ( pScene->exit[DIR_EAST] != NULL
        && pScene->exit[DIR_EAST]->to_scene != NULL
        && pScene->exit[DIR_EAST]->to_scene->people != NULL  ) {
       send_to_actor( "You glance east:\n\r", ch );
       cmd_look( ch, "east" );
       fMatch = TRUE;
       }

       if ( pScene->exit[DIR_WEST] != NULL
        && pScene->exit[DIR_WEST]->to_scene != NULL
        && pScene->exit[DIR_WEST]->to_scene->people != NULL  ) {
       send_to_actor( "You glance west:\n\r", ch );
       cmd_look( ch, "west" );
       fMatch = TRUE;
       }

       if ( pScene->exit[DIR_UP] != NULL
        && pScene->exit[DIR_UP]->to_scene != NULL
        && pScene->exit[DIR_UP]->to_scene->people != NULL  ) {
       send_to_actor( "You glance up:\n\r", ch );
       cmd_look( ch, "up" );
       fMatch = TRUE;
       }

       if ( pScene->exit[DIR_DOWN] != NULL
        && pScene->exit[DIR_DOWN]->to_scene != NULL
        && pScene->exit[DIR_DOWN]->to_scene->people != NULL  ) {
       send_to_actor( "You glance down:\n\r", ch );
       cmd_look( ch, "down" );
       fMatch = TRUE;
       }

       if ( pScene->exit[DIR_NE] != NULL
        && pScene->exit[DIR_NE]->to_scene != NULL
        && pScene->exit[DIR_NE]->to_scene->people != NULL  ) {
       send_to_actor( "You glance northeast:\n\r", ch );
       cmd_look( ch, "northeast" );
       fMatch = TRUE;
       }

       if ( pScene->exit[DIR_NW] != NULL
        && pScene->exit[DIR_NW]->to_scene != NULL
        && pScene->exit[DIR_NW]->to_scene->people != NULL  ) {
       send_to_actor( "You glance northwest:\n\r", ch );
       cmd_look( ch, "northwest" );
       fMatch = TRUE;
       }

       if ( pScene->exit[DIR_SE] != NULL
        && pScene->exit[DIR_SE]->to_scene != NULL
        && pScene->exit[DIR_SE]->to_scene->people != NULL  ) {
       send_to_actor( "You glance southeast:\n\r", ch );
       cmd_look( ch, "southeast" );
       fMatch = TRUE;
       }

       if ( pScene->exit[DIR_SW] != NULL
        && pScene->exit[DIR_SW]->to_scene != NULL
        && pScene->exit[DIR_SW]->to_scene->people != NULL  ) {
       send_to_actor( "You glance southwest:\n\r", ch );
       cmd_look( ch, "southwest" );
       fMatch = TRUE;
       }

       if ( !fMatch ) send_to_actor( "There is no one nearby.\n\r", ch );
       display_interp( ch, "^N" );
};

/*
 * Syntax:  look
 *          look auto
 *          look [direction]
 *          look [prop]
 *          look [person]
 *          look [description]
 */
void cmd_look( PLAYER *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    EXIT *pexit;
    SCENE *in_scene;
    PLAYER *victim;
    PROP *prop;
    int door;

    in_scene = ch->in_scene;

    if ( is_number( argument ) ) {cmd_note( ch, argument ); return;}

/*    if ( ch->desc == NULL )
    return;   */

    if ( ch->position < POS_SLEEPING )
    {
	send_to_actor( "It is impossible to see anything but stars!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_actor( "It is impossible to see anything, you're sleeping!\n\r", ch );
	return;
    }

    if ( !check_blind( ch ) )
	return;

    if ( !NPC(ch)
      && !IS_SET(ch->flag, WIZ_HOLYLIGHT)
      && scene_is_dark( in_scene ) )
    {
        bool fLight = FALSE;
        int direction = 0;
        int count = 0;
        EXIT *pExit;
        char b[MAX_INPUT_LENGTH];

        b[0] = '\0';
        for( direction = 0; direction < MAX_DIR; direction++ )
        {
             pExit = in_scene->exit[direction];
             if ( pExit != NULL
               && !IS_SET(pExit->exit_flags, EXIT_CLOSED)
               && pExit->to_scene != NULL
               && !scene_is_dark( pExit->to_scene ) ) 
             count++;
        }

        for( direction = 0;  direction < MAX_DIR; direction++ )
        {
             pExit = in_scene->exit[direction];
             if ( pExit != NULL 
               && !IS_SET(pExit->exit_flags, EXIT_CLOSED)
               && pExit->to_scene != NULL
               && !scene_is_dark( pExit->to_scene ) )
             {
                  strcat( b, dir_name[direction] );
                  fLight = TRUE;
                  switch( --count ) 
                  {
                        case 1: strcat( b, " and " ); break;
                        case 0: break;
                       default: strcat( b, ", " ); break;
                  }
             }
        }

        if (!fLight)
        {
             send_to_actor( "   You are surrounded by darkness.\n\r", ch );
        }
        else
        {
             act( "You can see light to the $t.", ch, b, NULL, TO_ACTOR);
        }

        show_actor_to_actor( in_scene->people, ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( !str_cmp( arg2, "at" ) )
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
    {
	/* 'look' or 'look auto' */
    if ( !NPC(ch) &&  HAS_ANSI(ch) )
    {
      /*
       * Change room description color based on time.
       */

    } else {
          if ( !NPC(ch) ) {
    display_interp( ch, "^N" );
    display_interp( ch, color_table[PC(ch,colors[COLOR_SCENE_TITLE])].di );
          }
//          } else {
//             display_interp( ch, "^N" );
//             display_interp( ch, time_color(ch) );
//             display_interp( ch, "^B" );
//          }
    }


    if ( ch->desc && ch->desc->client ) {
      char temp[MAX_STRING_LENGTH];

      sprintf( temp, "TITLE %s", in_scene->name );
      sendcli( ch->desc, temp );

/*      cmd_exits( ch, "clientauto" );*/
    }
    else {

    if ( !NPC(ch) && IS_SET(ch->flag2, PLR_AUTOMAP) && !ch->fighting ) {
        char temp[MIL];
        sprintf( temp, "border world tight terrain %d", PC(ch,mapsize) != 0 
                                                ? PC(ch,mapsize) : 4 );
        cmd_map( ch, temp );
    }

    //DEFCOLOR(ch); 
    //display_interp( ch, default_color_variable );
    if ( isGrey && weather.sky == MOON_RISE && weather.moon_phase != MOON_NEW ) display_interp( ch, "^N" ); 
    display_interp( ch, "^B" );
    snprintf( buf, MSL, "%s", in_scene->name );
    send_to_actor( string_proper(buf), ch ); /* SEND the TITLE */

    if ( !NPC(ch) && IS_SET(ch->flag2, PLR_AUTOEXIT) )
        display_interp( ch, "^N" ); 

    if ( !NPC(ch) ) {
        display_interp( ch, color_table[PC(ch,colors[COLOR_SCENE_TITLE])].di );
        cmd_exits( ch, "auto" );
       }
    }

    if ( !NPC(ch) && HAS_ANSI(ch) )
    {
    display_interp( ch, "^N" );
    }

    if ( IS_SET(in_scene->scene_flags, SCENE_SAVE) )
    send_to_actor( "*", ch );
    
    send_to_actor( "\n\r", ch );

            DEFCOLOR(ch);

    if ( arg1[0] == '\0'
    || ( !NPC(ch) && !IS_SET(ch->flag2, PLR_BRIEF) ) 
    || MTP((in_scene->description)) )
    {
        SCENE *template;
        
        template = get_scene( in_scene->template );
        if ( template != NULL && !MTP( template->description ) )
        {
            /*if ( weather.hour > 5 )*/
            send_to_actor( "   ", ch );  /*indent*/
            display_interp( ch, template->description );
        }
    }

    /*
     * Worldgen
     */
    if ( in_scene->terrain != 0 ) show_terrain( ch, in_scene );

    if ( !MTP((in_scene->description)) && !IS_SET(ch->flag2,PLR_BRIEF) )
        {
        send_to_actor( "   ", ch ); /* indent */
        DEFCOLOR(ch);
        display_interp( ch, in_scene->description );
        }
            if ( !NPC(ch) && IS_SET(ch->flag2,PLR_VERBOSE) ) {
            display_interp( ch, default_color_variable );
            send_to_actor( "\n\r", ch );
            cmd_exits( ch, "" );
            }
            default_color_variable = "^N";
            default_color_variable_di = NTEXT;


    display_interp( ch, "^N" );
    send_to_actor( "\n\r", ch );
    if ( !NPC(ch) ) {
    display_interp( ch, color_table[PC(ch,colors[COLOR_PROP_LIST])].di );
    show_list_to_actor( in_scene->contents, ch, FALSE, FALSE );
    show_actor_to_actor( in_scene->people,   ch );

    display_interp( ch, "^N" );

    if ( ch->desc) sendcli( ch->desc, in_scene->client );
    }
	return;
    }

    if ( !str_cmp( arg1, "i" ) || !str_cmp( arg1, "in" ) )
    {
        /* 'look in' */
        if ( arg2[0] == '\0' )
        {
            send_to_actor( "Look in what?\n\r", ch );
            return;
        }

        if ( ( prop = get_prop_here( ch, arg2 ) ) == NULL )
        {
            send_to_actor( "You do not see that here.\n\r", ch );
            return;
        }

        switch ( prop->item_type )
        {
        default: send_to_actor( "That is not a container.\n\r", ch );       
            break;

        case ITEM_LIGHT:
             if ( prop->value[1] != 0 )
             {
             int percent = PERCENTAGE(prop->value[0], prop->value[1]);

             if ( prop->value[0] <= 0 )
             {
                 send_to_actor( "It is empty.\n\r", ch );
                 break;
             }
             else
             {
             snprintf( buf, MAX_STRING_LENGTH, "It %s fuel.\n\r",
             percent < 10 ? "contains a low amount of"      :
             percent < 40 ? "contains less than half its capacity of" :
             percent < 70 ? "contains an ample amount of"   :
             percent < 90 ? "is mostly filled with"         : "is full of" );
             send_to_actor( buf, ch );
             }
             }
             else send_to_actor( "It contains nothing.\n\r", ch );
        break;
  
        case ITEM_BOOK: if ( STR(prop,action_descr) ) send_to_actor( "Written herein:\n\r", ch ); send_to_actor( STR(prop,action_descr), ch ); break;

        case ITEM_DRINK_CON:
             if ( prop->value[1] <= 0 )
                 {
                     send_to_actor( "It is empty.\n\r", ch );
                     break;
                 }
             else
             {
             int percent = PERCENTAGE(prop->value[0], prop->value[1]);

             snprintf( buf, MAX_STRING_LENGTH, "It %s of a%s %s liquid.\n\r",
                      percent < 10 ? "has but a drop"         :
                      percent < 40 ? "is less than half full" :
                      percent < 60 ? "is half full"           :
                      percent < 70 ? "is near full"           : "full",
                      IS_VOWEL(liq_table[prop->value[2]].liq_color[0]) ? "n" : "",
                      liq_table[prop->value[2]].liq_color );
             send_to_actor( buf, ch );
             }
        break;

        case ITEM_CONTAINER:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
            if ( IS_SET(prop->value[1], CONT_CLOSED) )
            {
                send_to_actor( "It is closed.\n\r", ch );
                break;
            }

            if ( prop->item_type == ITEM_CONTAINER )
            {
                 snprintf( buf, MAX_STRING_LENGTH, "%s contains ",
                     format_prop_to_actor( prop, ch, TRUE ) );
                 buf[0] = UPPER(buf[0]);
            }
            else
            {
            	act( "$n searches the remains of $p.", ch, prop, NULL, TO_SCENE );
            	snprintf( buf, MAX_STRING_LENGTH, "Searching the remains of %s reveals ",
                     format_prop_to_actor( prop, ch, TRUE ) );
            }

            page_to_actor( show_list_to_actor2( prop->contains, ch, buf ), ch );
        break;
	}
        return;
    }

    if ( !str_cmp( arg1, "o" ) || !str_cmp( arg1, "on" ) )
    {
        /* 'look on' */
        if ( arg2[0] == '\0' )
        {
            send_to_actor( "Look on what?\n\r", ch );
            return;
        }

        if ( (prop = get_prop_here( ch, arg2 )) == NULL )
        {
            send_to_actor( "You do not see that here.\n\r", ch );
            return;
        }
        
        if ( prop->item_type != ITEM_FURNITURE || prop->contains  == NULL )
        act( "There is nothing on $p.", ch, prop, NULL, TO_ACTOR );
        else
        {
        if ( prop->contains->next_content != NULL )
        snprintf( buf, MAX_STRING_LENGTH, "On %s are ", format_prop_to_actor( prop, ch, TRUE ) );
        else
        snprintf( buf, MAX_STRING_LENGTH, "On %s is ", format_prop_to_actor( prop, ch, TRUE ) );

        send_to_actor( show_list_to_actor2( prop->contains, ch, buf ), ch );
        }
        
        if ( has_occupant( prop ) )
        show_occupants_to_actor( prop, ch );
        return;
    }

    if ( !str_cmp( arg1, "a" ) || !str_cmp( arg1, "at" ) )
    {
	strcpy( arg1, arg2 );
        arg2[0] = '\0';
    }

    if ( (victim = get_actor_scene( ch, arg1 )) != NULL )
    {
	show_actor_to_actor_1( victim, ch );
	return;
    }

    if ( actor_look_list( ch, ch->carrying, arg1 ) )
    return;

    if ( actor_look_list( ch, in_scene->contents, arg1 ) )
    return;

    {
        char *pdesc;
        
        pdesc = get_extra_descr( arg1, in_scene->extra_descr );        
        if ( pdesc != NULL )
        {
            page_to_actor( pdesc, ch );
            return;
        }
    }

    if ( ( door = get_dir( arg1 ) ) == MAX_DIR )
    {
	send_to_actor( "You do not see that here.\n\r", ch );
	return;
    }

    /* 'look direction' */
    if ( ( pexit = in_scene->exit[door] ) == NULL )
    {
        cmd_time( ch, "internal" );
        snprintf( buf, MAX_STRING_LENGTH, "There is nothing of note %sward from here.\n\r",
                      dir_name[door] );
        send_to_actor( buf, ch );
        return;
    }

    if ( pexit->to_scene != NULL && scene_is_dark( pexit->to_scene ) )
    {
        if (door!=DIR_DOWN && door!=DIR_UP)
        snprintf( buf, MAX_STRING_LENGTH, "To the %s is darkness.\n\r", dir_name[door] );
        else snprintf( buf, MAX_STRING_LENGTH, "%s is darkness.\n\r", dir_name[door] );
        send_to_actor( buf, ch );
        return;
    }

    if ( !MTD(pexit->description) )  send_to_actor( pexit->description, ch );
    else
    if ( MTD(pexit->keyword) || !IS_SET( pexit->exit_flags, EXIT_ISDOOR ) )
    {
        snprintf( buf, MAX_STRING_LENGTH, "There is nothing of note %sward from here.\n\r",
                      dir_name[door] );
        send_to_actor( buf, ch );
    }

    if ( !MTD(pexit->keyword)
      && !IS_SET(pexit->exit_flags, EXIT_SECRET)
      && !IS_SET(pexit->exit_flags, EXIT_CONCEALED) )
    {
        if ( IS_SET(pexit->exit_flags, EXIT_CLOSED) )
        act( "The $T is closed.", ch, NULL, pexit->keyword, TO_ACTOR );
        else if ( IS_SET(pexit->exit_flags, EXIT_ISDOOR) )
        act( "The $T is open.",   ch, NULL, pexit->keyword, TO_ACTOR );
    }

    if ( (( IS_SET(pexit->exit_flags, EXIT_WINDOW)
      && !IS_SET(pexit->exit_flags, EXIT_CLOSED) )
      || IS_SET(pexit->exit_flags, EXIT_TRANSPARENT) )
      && pexit->to_scene != NULL )
    {
        act( "Through the $t you see $T:", ch, pexit->keyword, pexit->to_scene->name, TO_ACTOR );

        if ( !MTD(pexit->to_scene->description) )
        {
        send_to_actor( "   ", ch ); /* indent */
        display_interp( ch, pexit->to_scene->description );
        }

        cmd_time( ch, "internal" );
        show_list_to_actor( pexit->to_scene->contents, ch, FALSE, FALSE );
/*        show_actor_to_actor( pexit->to_scene->people,   ch ); */
    }

    if ( !IS_SET(pexit->exit_flags, EXIT_CLOSED)
      && !IS_SET(pexit->exit_flags, EXIT_CONCEALED) )
    {
         if ( !IS_SET(pexit->exit_flags, EXIT_WINDOW) )
         act( "$n glances $t.", ch, dir_name[door], NULL, TO_SCENE );
         else
         act( "$n peers through the $t.", ch, pexit->keyword, NULL, TO_SCENE ); 
    }
   
    scan_direction( ch, door );

    return;
}




/*
 * Thanks to Zrin for auto-exit part.
 * Syntax:  exits
 *          exits auto
 */
void cmd_exits( PLAYER *ch, char *argument )
{
    extern char * const dir_name[];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    EXIT *pexit;
    PROP *prop;
    bool found;
    bool fAuto;
    int door;

    buf[0] = '\0';
    fAuto  = !str_cmp( argument, "auto" );

    if ( !IS_SET(ch->flag2,PLR_AUTOEXIT) && fAuto ) return;

    if ( !str_cmp( argument, "clientauto" ) ) {

          send_to_actor( "SHOWEXITS", ch );
          if ( ch->desc ) {

          for( door = 0;  door < MAX_DIR; door++ )
          {
	      	if ( ( pexit = ch->in_scene->exit[door] ) != NULL
		&&   pexit->to_scene != NULL
	        &&   !IS_SET(pexit->exit_flags, EXIT_CONCEALED) )
                {
                        send_to_actor( " ", ch );
                        send_to_actor( dir_letter[door], ch );
                }
          }
          }

          return;
    }

    if ( scene_is_dark( ch->in_scene ) && !IS_SET(ch->flag2, WIZ_HOLYLIGHT) )
      {
          if ( !fAuto ) 
          send_to_actor( "It's too dark to see anything!\n\r", ch );
          return;
	}


    if ( !check_blind( ch ) )
	return;

    strcpy( buf, fAuto ? " [" : "" );

    found = FALSE;
    for ( door = 0; door < MAX_DIR; door++ )
    {
	if ( ( pexit = ch->in_scene->exit[door] ) != NULL
	&&   pexit->to_scene != NULL
        &&   !IS_SET(pexit->exit_flags, EXIT_CONCEALED) )
	{
            if ( fAuto )
            {
                if ( (IS_SET(pexit->exit_flags, EXIT_CLOSED)
                   && IS_SET(pexit->exit_flags, EXIT_SECRET))
                  || IS_SET(pexit->exit_flags, EXIT_CONCEALED) )
                continue;
                
                strcat( buf, dir_letter[door] );
                found = TRUE;
            }
	    else
	    {
            if ( !IS_SET(pexit->exit_flags, EXIT_CLOSED)
              && !MTD(pexit->keyword) )
            {
                sprintf( buf + strlen(buf), "A%s %s leads %s to %s.\n\r",
                         IS_VOWEL(pexit->keyword[0]) ? "n" : "",
                         pexit->keyword, dir_name[door],
                         scene_is_dark( pexit->to_scene )  ?  "darkness"
                                            : pexit->to_scene->name );
                found = TRUE;
            }
            else
            if ( !IS_SET(pexit->exit_flags, EXIT_SECRET) )
            {
                sprintf( buf + strlen(buf), "%s%s%s is %s%s.\n\r",
                  door != DIR_UP && door != DIR_DOWN ? "To the " : "",
                  door != DIR_UP && door != DIR_DOWN ? dir_name[door]
                                               : capitalize(dir_name[door]),
                  door == DIR_UP || door == DIR_DOWN ? "ward from here" : "",
                  IS_SET(pexit->exit_flags, EXIT_CLOSED) ? "a closed " : "",
                  IS_SET(pexit->exit_flags, EXIT_CLOSED) ? pexit->keyword :
                  scene_is_dark( pexit->to_scene )  ?  "darkness"
                                            : pexit->to_scene->name );
                found = TRUE;
            }
	    }
	}
    }
    
    for ( prop = ch->in_scene->contents;  prop != NULL;  prop =prop->next_content )
    {
    	if ( prop->item_type == ITEM_FURNITURE
    	  && IS_SET(prop->value[1], FURN_EXIT)
         && !IS_SET(prop->value[1], FURN_NOSHOW)
          && can_see_prop( ch, prop ) )
    	{
		if ( !fAuto )
		sprintf( buf2, "%s", STR(prop, action_descr) );
		else
		sprintf( buf2, " (%s)", STR(prop, short_descr) );
 
    		send_to_actor( buf2, ch );
    		found = TRUE;
    	}
    }

    if ( !found )
        strcat( buf, fAuto ? "" : "There are no obvious ways out.\n\r" );

    if ( fAuto )
        strcat( buf, "]" );

    if ( buf[1] != ']' ) send_to_actor( buf, ch );
    return;
}


#define STAT(func, str1, str2, str3, str4, str5 ) \
                if (func(ch) <  8) sprintf(buf, str1 );       \
           else if (func(ch) < 12) sprintf(buf, str2 );       \
           else if (func(ch) < 16) sprintf(buf, str3 );       \
           else if (func(ch) < 22) sprintf(buf, str4 );       \
           else                    sprintf(buf, str5 );


/*
 * Syntax:  score
 */
void cmd_score( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int i, race;

    clrscr(ch);

    if ( NPC(ch) )
    {
        cmd_stat( ch, "self" );
        return;
    }

    display_interp( ch, "^N" );
    
    buf2[0] = '\0';

    race = race_lookup( ch->race );
    if ( race != 0 )
    {
         i = PERCENTAGE(GET_AGE(ch),RACE(race,base_age));
         snprintf( buf, MAX_STRING_LENGTH, "You are a%s %s of %s named %s%s%s,\n\r",
                   i < 30 ? " young" :
                   i < 60 ? " middle-aged" :
                   i < 80 ? "n aging" :
                   i < 90 ? "n elderly" : "n ancient ",
                   RACE(race,race_name), 
                   RACE(race,race_name_type), 
                   ch->name, 
                   PC(ch,title) != NULL && PC(ch,title) != &str_empty[0] ? " " : "",
                   PC(ch,title) != NULL && PC(ch,title) != &str_empty[0] ? PC(ch,title) : ""   );
    }
    else
    {
         i = PERCENTAGE(GET_AGE(ch),100);
         snprintf( buf, MAX_STRING_LENGTH, "You are a%s %s named %s%s%s,\n\r",
                   i < 30 ? " young" :
                   i < 60 ? " middle-aged" :
                   i < 80 ? "n aging" :
                   i < 90 ? "n elderly" : "n ancient", 
         ch->sex == SEX_MALE ? "man" : "woman", ch->name, 
                   PC(ch,title) != NULL && PC(ch,title) != &str_empty[0] ? " " : "",
                   PC(ch,title) != NULL && PC(ch,title) != &str_empty[0] ? PC(ch,title) : ""   );
    }

    strcat( buf2, buf );

    {
        int p = PC(ch,played);
        int ct = (int) (current_time - PC(ch,logon));

        i = (p + ct)/7200;
    }

    if ( i > 0 )
    {
    if ( i > 24 )
    {
        int days;
        int hours;
        char smuf[MAX_STRING_LENGTH];
        char *p;

        days  = i / 24;
        hours = i % 24;

        p = str_dup( numberize(days) );
        snprintf( smuf, MSL, "%s", p );
        snprintf( buf, MAX_STRING_LENGTH, " with a total of %s day%s and %s hour%s of play,\n\r",
                      smuf, days == 1 ? "" : "s",
                      numberize( hours ), hours == 1 ? "" : "s" );
        free_string(p);
    }
    else
    {
        snprintf( buf, MAX_STRING_LENGTH, " with a total of %s hour%s of play,\n\r",
                      numberize( i ), i == 1 ? "" : "s" );
    }
    strcat( buf2, buf );
    }

    snprintf( buf, MAX_STRING_LENGTH, " %d inches tall,\n\r",  ch->size*4  );
    strcat( buf2, buf );
    
    snprintf( buf, MAX_STRING_LENGTH, " you are %s years old,\n\r ", numberize( GET_AGE(ch) ) );
    strcat( buf2, buf );

    if ( PC(ch,birth_month) == weather.month )
    {

        int daydist = PC(ch,birth_day) - weather.day;

        if ( daydist == -1 )
        strcat( buf2, " yesterday was your birthday, " );
        else
        if ( daydist == 0 )
        strcat( buf2, " today is your birthday, " );
        else
        if ( daydist == 1 )
        strcat( buf2, " tomorrow is your birthday, " );
        else
        if ( daydist > 1 && daydist < 30 )
        {
            snprintf( buf, MAX_STRING_LENGTH, " your birthday is in %s days,", numberize(daydist) );
            strcat( buf2, buf );
        }
    }
    strcat( buf2, "\n\r" );
    
    snprintf( buf, MAX_STRING_LENGTH, " and you are currently %s and %s.\n\r\n\r",
                  STRING_HITS(ch), STRING_MOVES(ch) );
    strcat( buf2, buf );

    {
	char *p;

    p = format_indent( str_dup(buf2), " ", 76, TRUE );
    display_interp( ch, "^N" );
    send_to_actor( "\n\r", ch );
    send_to_actor( p, ch );
    display_interp( ch, "^N" );
    free_string( p );
    }

    display_interp( ch, "^3^:^B" );
    snprintf( buf, MAX_STRING_LENGTH, "\n\rYou are level %d with %d experience points, needing %d for the next.", 
                  ch->exp_level, ch->exp,
                  ((ch->exp_level+1)*EXP_PER_LEVEL)-(ch->exp) );
    send_to_actor( buf, ch );
    display_interp( ch, "^+^N" ); 
    send_to_actor( "\n\r", ch );
  
    if ( ch->carry_number != 0 )
    {
        char *p=str_dup( numberize( ch->carry_number ) );
        char *q=str_dup( numberize( ch->carry_weight/2+1 ) );
        snprintf( buf, MAX_STRING_LENGTH, "\n\rYou are carrying %s item%sweighing roughly %s stone%s\n\r",
                      p,
                      ch->carry_number == 1 ? " " : "s ",
                      q,
                      ch->carry_weight == 1 ? "." : "s." );
        send_to_actor( format_string(buf), ch );
        free_string(p);
        free_string(q);
    }

    
    {
    char *p;
    char fbuf[MAX_STRING_LENGTH];
    
    fbuf[0] = '\0';
    
    sprintf( fbuf, "You possess a " );

    STAT(get_curr_int, "moronic",
                       "simpleton's",
                       ch->sex == SEX_MALE   ? "man's" :
                       ch->sex == SEX_FEMALE ? "woman's" : "eunich's",
                       "scholarly",
                       "genius" );
    strcat( fbuf, buf );

    STAT(get_curr_str, " frail ",
                       " ",
                       " hardened ",
                       " muscular ",
                       " mighty " );
    strcat( fbuf, buf );

    STAT(get_curr_wis, " physique, with a head like a hole,  ",
                       " body, ",
                       " build, with good common sense,  ",
                       " frame, with a wise air about you,  ",
                       " physique, with a perceptive mind,  " );
    strcat( fbuf, buf );

    STAT(get_curr_dex, " and you are lithargic and" ,
                       " and you are ",
                       " and you are nimble and",
                       " and you are quick and",
                       " and you are like lightning and" );
    strcat( fbuf, buf );
     
    STAT(get_curr_con, " quite faint-hearted.  ",
                       " of average health.  ",
                       " healthy.  ",
                       " brazen.  ",
                       " very healthy.  " );
    strcat( fbuf, buf );
    
    p = format_indent( str_dup(fbuf), " ", 76, TRUE );
    send_to_actor( "\n\r^4", ch );
    cmd_fight( ch, "" );
    display_interp( ch, "^B" );
    send_to_actor( p, ch );
    free_string( p );
    }

    if ( ch->userdata )
    {
    display_interp( ch, "^6" );
    snprintf( buf, MAX_STRING_LENGTH, "You are %s, %s, %s",
         (PC(ch,condition[COND_DRUNK] ) > 50) ? "drunk" : 
         (PC(ch,condition[COND_DRUNK] ) > 20) ? "tipsy" : 
                                                "sober",
         
         (PC(ch,condition[COND_THIRST]) >   50) ? "quenched"    :
         (PC(ch,condition[COND_THIRST]) >   10) ? "not thirsty" :
         (PC(ch,condition[COND_THIRST]) >= -10) ? "thirsty"     : 
                                                  "dehydrated",
         
         (PC(ch,condition[COND_FULL]  ) >  50) ? "full"            :
         (PC(ch,condition[COND_FULL]  ) >  20) ? "satisfied"       :
         (PC(ch,condition[COND_FULL]  ) >   0) ? "slightly hungry" :
         (PC(ch,condition[COND_FULL]  ) > -30) ? "hungry"          :
         (PC(ch,condition[COND_FULL]  ) > -50) ? "famished"        :
                                                 "starving" );
    send_to_actor( buf,  ch );
    }

    switch ( ch->position )
    {
    case POS_DEAD:     send_to_actor( ", and have died.\n\r",              ch ); break;
    case POS_MORTAL:   send_to_actor( ", and have been mortally wounded.\n\r",  ch ); break;
    case POS_INCAP:    send_to_actor( ", and have been incapacitated.\n\r",     ch ); break;
    case POS_STUNNED:  send_to_actor( ", and, unfortunately, stunned.\n\r",     ch ); break;
    case POS_SLEEPING:
    case POS_SITTING:
    case POS_RESTING:
    if ( ch->furniture )
    {
        snprintf( buf, MAX_STRING_LENGTH, ", and %s on %s.\n\r", position_name(ch->position),
                 STR(ch->furniture,short_descr) );
    } 
    else
    snprintf( buf, MAX_STRING_LENGTH, ", and %s.\n\r", position_name(ch->position) );
    
    send_to_actor( buf, ch );
    break;
    case POS_STANDING:
     if ( ch->riding != NULL )
     {
        snprintf( buf, MAX_STRING_LENGTH, ", and you are riding %s.\n\r", NAME(ch->riding) );
        send_to_actor( buf, ch );
     }
     else
     send_to_actor( ".\n\r", ch );
    break;
    case POS_FIGHTING:
     if ( ch->riding != NULL )
     {
        snprintf( buf, MAX_STRING_LENGTH, ", and engaged in combat with %s.\n\r", NAME(ch->fighting) );
        send_to_actor( buf, ch );
     }
     else
     send_to_actor( ", and fighting.\n\r",          ch );
    break;
    }

    display_interp( ch, "^N" );
    display_interp( ch, "^B" );
     if ( ch->bounty <=   0  ) send_to_actor( "You are a protected citizen", ch );
else if ( ch->bounty <   500  ) send_to_actor( "You are noted as a troublemaker", ch );
else if ( ch->bounty <  10000 ) send_to_actor( "You are considered a criminal", ch );
else if ( ch->bounty < 25000 ) send_to_actor( "You are a fugitive from justice", ch );
                           else send_to_actor( "You are a feared outlaw", ch );

    if ( ch->owed > 0 ) {
      if ( ch->owed > 500 )
    send_to_actor( ", and owed a sizeable reward.\n\r", ch ); 
     else
    send_to_actor( ", and owed a reward.\n\r", ch );
    }
    else
    send_to_actor( ".\n\r", ch );

    display_interp( ch, "^N" );  

    if ( ch->karma > 0 ) { display_interp( ch, "^2^B" );  }
    if ( ch->karma > 100 ) { display_interp( ch, "^:^3^B" ); } 
    if ( ch->karma < -100 ) { display_interp( ch, "^?" ); }
    else display_interp( ch, "^6" );

     if ( ch->karma  == 0 ) send_to_actor( "Your karma is neutral.", ch );
else if ( ch->karma < -50 ) send_to_actor( "You've got bad karma written all over you.", ch);
else if ( ch->karma >  50 ) send_to_actor( "Your karma is ultrapositive.", ch );
else if ( ch->karma  <  0 ) send_to_actor( "Your karma is negative.", ch );
                       else send_to_actor( "Your karma is positive.", ch );
    display_interp( ch, "^+^1^N" );
    send_to_actor( "\n\r", ch );

     if ( ch->mana > 100 ) send_to_actor( "You feel energized.\n\r", ch );
else if ( ch->mana > 75  ) send_to_actor( "Your mind is clear.\n\r", ch );
else if ( ch->mana > 50  ) send_to_actor( "You feel focused.\n\r", ch );
else if ( ch->mana > 25  ) send_to_actor( "Your thoughts are blurred.\n\r", ch );
else if ( ch->mana >  0  ) send_to_actor( "Your mind is weak.\n\r", ch );
else send_to_actor( "Your mind is unfocused; your thoughts are a cacophony of voices.\n\r", ch );

    display_interp( ch, "^N" );

    snprintf( buf, MAX_STRING_LENGTH, "You are currently speaking %s.\n\r", lang_table[ch->speaking].name );
    send_to_actor( buf, ch );

    if ( IS_AFFECTED(ch,BONUS_HIDE) && IS_AFFECTED(ch,BONUS_SNEAK) )
    send_to_actor( "You are attempting camoflage and stealth.\n\r", ch );
    else
    if ( IS_AFFECTED(ch,BONUS_HIDE) )
    send_to_actor( "You are trying to camoflage yourself.\n\r", ch );
    else
    if ( IS_AFFECTED(ch,BONUS_SNEAK) )
    send_to_actor( "You are trying to move with stealth.\n\r", ch );

    if ( IS_IMMORTAL(ch) )
    {
    display_interp( ch, "^B" );
    send_to_actor( "\n\rYou are", ch );

    switch ( get_trust( ch ) )
    {
           default: if ( IS_IMMORTAL(ch) )
                           snprintf( buf, MAX_STRING_LENGTH, " an immortal,"   );
                      else buf[0] = '\0';
                    break;
   case LEVEL_HERO: snprintf( buf, MAX_STRING_LENGTH, " an avatar." ); break;
case LEVEL_BUILDER: snprintf( buf, MAX_STRING_LENGTH, " a builder," ); break;
  case LEVEL_ANGEL: snprintf( buf, MAX_STRING_LENGTH, " an angel."  ); break;
case LEVEL_SUPREME: snprintf( buf, MAX_STRING_LENGTH, " supreme."   ); break;
case LEVEL_DEMIGOD: snprintf( buf, MAX_STRING_LENGTH, " a demigod." ); break;
    case MAX_LEVEL: snprintf( buf, MAX_STRING_LENGTH, ch->sex == SEX_MALE ? " God." : " Goddess." ); break;
    }
    
    send_to_actor( buf, ch );

    if ( get_trust( ch ) == LEVEL_BUILDER ) {
    snprintf( buf, MAX_STRING_LENGTH, " with a building security of %d.\n\r", PC(ch,security) );
    }
    else snprintf( buf, MAX_STRING_LENGTH, " [%d]\n\r", PC(ch,security) );
    send_to_actor( buf, ch );
    }
    else if ( ch->exp_level < 5 ) {
       display_interp( ch, "^N^6" );
       send_to_actor( "You are a lowly newbie.  Behave.\n\r", ch );
       return;
    }

    display_interp( ch, "^N" );
    if ( PC(ch,flag) != NULL && strlen(PC(ch,flag)) > 0 ) {
       send_to_actor( "Your kingdom's banner is: ", ch );
       send_to_actor( PC(ch,flag), ch );
       send_to_actor( "^+^N\n\r", ch );
    }

    cmd_money( ch, NULL );
    return;
}



char *	const	day_name	[] =
{
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"

/* Original Nim5:
    "Lexo, the Great Hawk",
    "Franto, the Falcon",
    "Halag, the Owl",
    "Ruz, the Black Crow",
    "Kalamak, the Phoenix",
    "Aedo, the Golden Eagle",
    "Franwag, the Grey Gull" */                       /*  7 */
};

char *  const   month_name  [] =
{

    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"

/* Original Nim5:
    "Iak, Season of the Ice Cougar, in winter",
    "Vod, Season of the White Fox, in winter",
    "Lin, Season of the Robin, in spring",
    "Nemi, Season of the Crystalfish, in spring",
    "Grez, the Crop Wind, planting season",
    "Po, Season of the Squirrel, in summer",
    "Rani, the Dry Winds, in summer",
    "Falak, the Dragon's Breath, late summer",
    "Kae, Harvest Moon",
    "Ret, Season of the Changing Oak, in autumn",
    "Jal, the New Frost, in autumn",
    "Yun, the Ice Weaver, in winter"
 */

};

static char * const sky_look[8] =
{
    "cloudless",
    "cloudy",
    "obscured by storm clouds",
    "lit by flashes of lightning",
    "slightly overcast",
    "scattered with a few flakes",
    "filled with flakes",
    "a blizzard of white"
};


/*
 * Syntax:  time
 *          time full
 */
void cmd_time( PLAYER *ch, char *argument )
{
    extern char str_boot_time[];
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char descr[MAX_STRING_LENGTH];
    int wind;
    int day;
    bool outside=IS_OUTSIDE(ch);

    if ( !check_blind( ch ) )
    {
        send_to_actor( "You have no clue what is going on, you are blind!\n\r",
                      ch );
        return;
    }

    one_argument( argument, arg );

    if ( str_cmp( arg, "internal" ) ) {
    day     = weather.day + 1;

    switch ( weather.hour )
    {
        case  1:
        case  2: sprintf( descr, "late night"       ); break;
        case  3:
        case  4: sprintf( descr, "early morning"    ); break;
        case  5: sprintf( descr, "just before dawn" ); break;
        case  6: sprintf( descr, "dawn"             ); break;
        case  7: 
        case  8: sprintf( descr, "early morning"    ); break;
        case  9:
        case 10:
        case 11: sprintf( descr, "morning"          ); break;
        case 12: sprintf( descr, "midday"           ); break;
        case 13:
        case 15: sprintf( descr, "afternoon"        ); break;
        case 14: sprintf( descr, "mid-afternoon"    ); break;
        case 16:
        case 17: sprintf( descr, "evening"          ); break;
        case 18: sprintf( descr, "twilight"         ); break;
        case 19: sprintf( descr, "dusk"             ); break;
        case 20:
        case 21:
        case 22: sprintf( descr, "night"            ); break;
        case 23: sprintf( descr, "late at night"    ); break;
        case  0: sprintf( descr, "midnight"         ); break;
    }

    snprintf( buf, MAX_STRING_LENGTH, "It is %s on the day of %s.  The %s of the month of %s, in the year %s. ",
             descr,
             day_name[day % 7],
             day ==  1 ? "first"          :
             day ==  2 ? "second"         :
             day ==  3 ? "third"          :
             day ==  4 ? "fourth"         :
             day ==  5 ? "fifth"          :
             day ==  6 ? "sixth"          :
             day ==  7 ? "seventh"        :
             day ==  8 ? "eighth"         :
             day ==  9 ? "ninth"          :
             day == 10 ? "tenth"          :
             day == 11 ? "eleventh"       :
             day == 12 ? "twelfth"        :
             day == 13 ? "thirteenth"     :
             day == 14 ? "fourteenth"     :
             day == 15 ? "fifteenth"      :
             day == 16 ? "sixteenth"      :
             day == 17 ? "seventeenth"    :
             day == 18 ? "eighteenth"     :
             day == 19 ? "nineteenth"     :
             day == 20 ? "twentieth"      :
             day == 21 ? "twenty-first"   :
             day == 22 ? "twenty-second"  :
             day == 23 ? "twenty-third"   :
             day == 24 ? "twenty-fourth"  :
             day == 25 ? "twenty-fifth"   :
             day == 26 ? "twenty-sixth"   :
             day == 27 ? "twenty-seventh" :
             day == 28 ? "twenty-eighth"  :
             day == 29 ? "twenty-ninth"   :
                         "thirtieth",
             month_name[weather.month],
             numberize( weather.year ) );
     } else buf[0] = '\0';

    { int dir;
      for ( dir=0; dir<MAX_DIR; dir++ ) if ( ch->in_scene->exit[dir] && 
         (!IS_SET(ch->in_scene->exit[dir]->exit_flags,EXIT_CLOSED) || IS_SET(ch->in_scene->exit[dir]->exit_flags,EXIT_TRANSPARENT) ) && 
          !IS_SET(ch->in_scene->exit[dir]->to_scene->scene_flags,SCENE_INDOORS) ) outside=TRUE;
    }

    if ( !outside )
    {  if ( str_cmp( arg, "internal" ) )
        strcat( buf, "You cannot tell what the weather is like from here." );
    }
    else
    {
    wind = weather.windspeed;
    descr[0] = '\0';
    sprintf( descr, "The sky is %s ",
                  (weather.month < 2 || weather.month == 11) ?
                  sky_look[weather.sky+4] : sky_look[weather.sky] );
    strcat( buf, descr );
    sprintf( descr, "and a %s %sward %s blows.\n\r",
             (weather.temperature < 35) ? "cold" :
             (weather.temperature < 60) ? "cool" :
             (weather.temperature < 90) ? "warm" : "hot",
             dir_name[abs(weather.winddir) % 4],
             wind <= 20 ? "breeze"   :
             wind <= 50 ? "wind"     :
             wind <= 80 ? "gust"     :
                          "torrent"    );
    strcat( buf, descr );


    }

    if ( outside && weather.hour < 4 )
    {
        if ( weather.sky != SKY_CLOUDLESS
          && weather.moon_phase != MOON_NEW )
        strcat( buf, "The moon is behind a cloud." );
        else
        switch ( weather.moon_phase )
        {
        case MOON_NEW:
            strcat( buf, "No moon graces the heavens tonight." ); break;
        case MOON_WAXING_CRESCENT:
            strcat( buf, "The waxing moon is a mere crescent in the night sky." ); break;
        case MOON_WAXING_HALF:
            strcat( buf, "The waxing half-moon can be seen on this night." ); break;
        case MOON_WAXING_THREE_QUARTERS:
            strcat( buf, "Tonight, the moon is near full, a waxing gibbous of celestial harmony." ); break;
        case MOON_FULL:
            strcat( buf, "The moon is full." ); break;
        case MOON_WANING_THREE_QUARTERS:
            strcat( buf, "The moon is in the phase of a waning gibbous." ); break;
        case MOON_WANING_HALF:
            strcat( buf, "A waning half-moon treks across the night sky." ); break;
        case MOON_WANING_CRESCENT:
            strcat( buf, "Only a sliver of moon remains in the heavens." ); break;
          default:
            if ( IS_IMMORTAL(ch) )
            strcat( buf, "Something is drastically wrong with the moon!  E-gads!" );
           break;
        }
    }

    {
        char *p;
        p = format_string( str_dup( buf ) );
        send_to_actor( p, ch );
        free_string( p );
    }

    if ( IS_IMMORTAL( ch ) && !str_cmp( arg, "full" ) )
    {
        extern int num_hour;
        extern int pulse_zone;
        extern int pulse_actor;
        extern int pulse_violence;
        extern int autosave_counter;

        snprintf( buf, MAX_STRING_LENGTH, "%s\n\rBooted at:   %s\rSystem time: %s\r",
         VERSION_STR,  str_boot_time, (char *) ctime( &current_time )  );
        send_to_actor( buf, ch );

        sprintf( buf,
 "Temp: %4d  Windspd: %4d  Dir:    %4d  Month: %4d  Hour: %4d  Phase: %4d\n\r"
 "Sky:  %4d  MMHG:    %4d  Change: %4d  Day:   %4d  Year: %4d  Next:  %4d\n\r"
 "Sun:  %4d  NumHour: %4d  zone:   %4d  Mob:   %4d  Fgt:  %4d  Auto:  %4d\n\r",
               weather.temperature,   weather.windspeed,
               weather.winddir %4,    weather.month,
               weather.hour,             weather.moon_phase,
               weather.sky,           weather.mmhg,
               weather.change,        weather.day,
               weather.year,             weather.next_phase,
               weather.sunlight,      num_hour/60,
               pulse_zone,                 pulse_actor,
               pulse_violence,             autosave_counter );
        send_to_actor( buf, ch );
    }

    return;
}



void help_to_actor( char *t, PLAYER *ch, bool fPage )
{/*
   char buf[MAX_STRING_LENGTH*2];
   const char *str;
   const char *i;
   char *point;
  */
   if( t == NULL || *t == '\0' )
   {
      send_to_actor( "\n\r\n\r", ch );
      return;
   }

   display_interp( ch, t );
   return;
}





/*
 * Syntax:  help [keyword]
 */
void cmd_help( PLAYER *ch, char *argument )
{
   char argall[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   HELP *pHelp;
   int dbkey;
   bool fShowed=FALSE;
    
   if ( ch->desc == NULL ) return;

   strcpy( buf, "" );
   strcpy( buf2, "" );

   argument = one_argument( argument, arg1 );

   if ( arg1[0] == '\0' )
      strcpy( arg1, "summary");

   if ( !str_cmp( arg1, "show" ) || !str_cmp( arg1, "index" ) ) {
      int col = 0;
      int i,v;
      int hclass[MAX_HELP_CLASS];


      argument = one_argument( argument, arg1 );

      if ( arg1[0] == '\0' ) {

         for ( i=0; i < MAX_HELP_CLASS; i++ )
            hclass[i] = 0;

         snprintf( buf, MAX_STRING_LENGTH, "\nThe following classes of help files are available:\n\n#     Class           Description\n\r" );
         send_to_actor( buf, ch );
         snprintf( buf, MAX_STRING_LENGTH, "-     -----           -----------\n\r" );
         send_to_actor( buf, ch );

         for ( v=0; v <= top_dbkey_help; v++ ) {
                pHelp = get_help_index(v); 

            if ( !pHelp || pHelp->level > get_trust( ch ) )
               continue;
            hclass[pHelp->class]++;
         }

         for ( i=0; i < MAX_HELP_CLASS; i++ ) {
            sprintf(buf, "%-5d %-15s %s\n", hclass[i], help_class_table[i].name, help_class_table[i].desc);
            send_to_actor( buf, ch );
         }
         snprintf( buf, MAX_STRING_LENGTH, "\nType 'help show [class]' to see available helps for a specific class.\n\r" );
         send_to_actor( buf, ch );

         return;

      } else {
         for ( i=0; i < MAX_HELP_CLASS; i++ ) {
            if ( !str_prefix( arg1, help_class_table[i].name )) {
         for ( dbkey=0; dbkey <= top_dbkey_help; dbkey++ ) {

                pHelp = get_help_index(dbkey); 

                  if ( !pHelp || pHelp->level > get_trust( ch ) )
                  continue;

                  if ( pHelp->class == i) {
                     col++;
                     sprintf( buf2, "%-26s", trunc_fit( string_proper ( pHelp->name ), 25 ) );
                     strcat( buf, buf2 );
                     if ( col % 3 == 0 ) {
                       strcat( buf, "\n\r" );
                       send_to_actor( buf, ch );
                       strcpy( buf, "" );
                       strcpy( buf2, "" );
                     }
                  }
               }
               send_to_actor( buf, ch );
               return;
            }
         }
      }
   }

   if ( arg1[0] == '#' ) {
      char b[MAX_STRING_LENGTH];
      bool fFound = FALSE;        

      argument = arg1;

      argument++;
      if ( *argument == '\0' ) { 
send_to_actor( "No search string entered, try HELP #keyword\n\r", ch ); 
send_to_actor( "or HELP #'phrase'\n\r", ch ); 
return; }

      snprintf( b, strlen(b), "Related Topics:\n\r" );
         for ( dbkey=0; dbkey <= top_dbkey_help; dbkey++ ) {
            pHelp = get_help_index(dbkey); 

         if ( !pHelp || pHelp->level > get_trust( ch ) )
            continue;

         if ( !str_infix( pHelp->keyword, argument )
           || !str_infix( pHelp->name, argument )
           || strstr( pHelp->text, argument )
           || (IS_IMMORTAL(ch) && strstr( pHelp->immtext, argument )) ) {
            if ( !ch->desc || CONNECTED(ch->desc)
               || ch->desc->connected == NET_DOC_MENU ) {
                  fFound = TRUE;
                  strcat( b, pHelp->name );
                  if ( !MTD(pHelp->keyword) ) {
                  strcat( b, " [" );
                  strcat( b, pHelp->keyword );
                  strcat( b, "]"  );
                  }
                  strcat( b, "\n\r" );
               }
         }
      }
      page_to_actor( b, ch );
      if ( !fFound ) page_to_actor( "None.", ch );
      return;
   }

   /* The for() loop appears twice to force direct matching when a=a
    * Second loop is for tricky argument handling so 'help a b' doesn't match a.
    */
   strcpy ( argall, arg1 );
   while ( argument[0] != '\0' ) {
      argument = one_argument( argument, arg1 );
      if ( arg1[0] != '\0' )
         strcat( argall, " " );
      strcat( argall, arg1 );
   }

     for( dbkey=0; dbkey <= top_dbkey_help; dbkey++ ) {
        pHelp = get_help_index(dbkey);
	if ( !pHelp || pHelp->level > get_trust( ch ) )
	    continue;

        if ( !str_cmp( argall, pHelp->name ) )
        {  fShowed=TRUE;
            if ( !ch->desc || CONNECTED(ch->desc)
              || ch->desc->connected == NET_DOC_MENU )
            {
                char b[MAX_STRING_LENGTH];

                b[0] = '\0';
                
/*
                first_arg( pHelp->keyword, b, FALSE ); 
*/
		if ( pHelp->class != HELP_CLASS_TIPS
                  && pHelp->class != HELP_CLASS_NEWBIE ) {
                sprintf(b, "\n%s (%s)\n",
                    pHelp->name, help_class_table[pHelp->class].name );

                if ( HAS_ANSI(ch) ) display_interp( ch, "^B" );
                send_to_actor( b, ch );
                if ( HAS_ANSI(ch) ) display_interp( ch, "^N" );
		}
                send_to_actor( "\n\r", ch );

                if ( str_cmp( pHelp->syntax, "" ) ) {
                   if ( HAS_ANSI(ch) ) display_interp( ch, "^B" );
                   send_to_actor( "Syntax:  ", ch );
                   if ( HAS_ANSI(ch) ) display_interp( ch, "^N" );
                   sprintf(b, "%s\n", pHelp->syntax);
                   send_to_actor( b, ch );
                }

/*
                send_to_actor( "Description:\n\r", ch );
 */

                if ( pHelp->text[0] == '.' )
                help_to_actor( pHelp->text+1, ch, TRUE );
                else
                help_to_actor( pHelp->text  , ch, TRUE );

                if ( IS_IMMORTAL(ch) && str_cmp( pHelp->immtext, "") )  {
                   if ( HAS_ANSI(ch) ) display_interp( ch, "^B" );
                   page_to_actor( "\nImmtext:\n\r", ch );
                   if ( HAS_ANSI(ch) ) display_interp( ch, "^N" );
                   if ( pHelp->immtext[0] == '.' )
                      help_to_actor( pHelp->immtext+1, ch, TRUE );
                   else
                      help_to_actor( pHelp->immtext  , ch, TRUE );
                }

                if ( str_cmp( pHelp->example, "") )  {
                   if ( HAS_ANSI(ch) ) display_interp( ch, "^B" );
                   page_to_actor( "\nExample:\n\r", ch );
                   if ( HAS_ANSI(ch) ) display_interp( ch, "^N" );
                   if ( pHelp->example[0] == '.' )
                      help_to_actor( pHelp->example+1, ch, TRUE );
                   else
                      help_to_actor( pHelp->example  , ch, TRUE );
                }

                if ( str_cmp( pHelp->seealso, "") )  {
                   if ( HAS_ANSI(ch) ) display_interp( ch, "^B" );
                   page_to_actor( "\nSee also: ", ch );
                   if ( HAS_ANSI(ch) ) display_interp( ch, "^N" );
                   sprintf(b, "%s\n", pHelp->seealso);
                   page_to_actor( b, ch );
                } else {
                   page_to_actor( "\n\r", ch );
                }
                return;
            }

            /*
             * Strip leading '.' to allow initial blanks.
             */
            if ( pHelp->text[0] == '.' )
            help_to_actor( pHelp->text+1, ch, FALSE );
            else
            help_to_actor( pHelp->text  , ch, FALSE );

            if ( str_cmp( pHelp->immtext, "") )  {
               if ( pHelp->immtext[0] == '.' )
                  help_to_actor( pHelp->immtext+1, ch, TRUE );
               else
                  help_to_actor( pHelp->immtext  , ch, TRUE );
            }
            return;
        }
    }

   if ( !fShowed )
     for( dbkey=0; dbkey <= top_dbkey_help; dbkey++ ) {
        pHelp = get_help_index(dbkey);
	if ( !pHelp || pHelp->level > get_trust( ch ) )
	    continue;

        if ( is_prename( argall, pHelp->name ) || is_prename( argall, pHelp->keyword )  )
        {
            if ( !ch->desc || CONNECTED(ch->desc)
              || ch->desc->connected == NET_DOC_MENU )
            {
                char b[MAX_STRING_LENGTH];

                b[0] = '\0';
                
/*
                first_arg( pHelp->keyword, b, FALSE ); 
*/
		if ( pHelp->class != HELP_CLASS_TIPS
                  && pHelp->class != HELP_CLASS_NEWBIE ) {
                sprintf(b, "\n%s (%s)\n",
                    pHelp->name, help_class_table[pHelp->class].name );

                if ( HAS_ANSI(ch) ) display_interp( ch, "^B" );
                send_to_actor( b, ch );
                if ( HAS_ANSI(ch) ) display_interp( ch, "^N" );
		}
                send_to_actor( "\n\r", ch );

                if ( str_cmp( pHelp->syntax, "" ) ) {
                   if ( HAS_ANSI(ch) ) display_interp( ch, "^B" );
                   send_to_actor( "Syntax:  ", ch );
                   if ( HAS_ANSI(ch) ) display_interp( ch, "^N" );
                   sprintf(b, "%s\n", pHelp->syntax);
                   send_to_actor( b, ch );
                }

/*
                if ( HAS_ANSI(ch) ) display_interp( ch, "^B" );
                send_to_actor( "Description:\n\r", ch );
                if ( HAS_ANSI(ch) ) display_interp( ch, "^N" );
 */

                if ( pHelp->text[0] == '.' )
                help_to_actor( pHelp->text+1, ch, TRUE );
                else
                help_to_actor( pHelp->text  , ch, TRUE );

                if ( IS_IMMORTAL(ch) && str_cmp( pHelp->immtext, "") )  {
                   if ( HAS_ANSI(ch) ) display_interp( ch, "^B" );
                   page_to_actor( "\nImmtext:\n\r", ch );
                   if ( HAS_ANSI(ch) ) display_interp( ch, "^N" );
                   if ( pHelp->immtext[0] == '.' )
                      help_to_actor( pHelp->immtext+1, ch, TRUE );
                   else
                      help_to_actor( pHelp->immtext  , ch, TRUE );
                }

                if ( str_cmp( pHelp->example, "") )  {
                   if ( HAS_ANSI(ch) ) display_interp( ch, "^B" );
                   page_to_actor( "\nExample:\n\r", ch );
                   if ( HAS_ANSI(ch) ) display_interp( ch, "^N" );
                   if ( pHelp->example[0] == '.' )
                      help_to_actor( pHelp->example+1, ch, TRUE );
                   else
                      help_to_actor( pHelp->example  , ch, TRUE );
                }

                if ( str_cmp( pHelp->seealso, "") )  {
                   if ( HAS_ANSI(ch) ) display_interp( ch, "^B" );
                   page_to_actor( "\nSee also: ", ch );
                   if ( HAS_ANSI(ch) ) display_interp( ch, "^N" );
                   sprintf(b, "%s\n", pHelp->seealso);
                   page_to_actor( b, ch );
                } else {
                   page_to_actor( "\n\r", ch );
                }
                return;
            }

            /*
             * Strip leading '.' to allow initial blanks.
             */
            if ( pHelp->text[0] == '.' )
            help_to_actor( pHelp->text+1, ch, FALSE );
            else
            help_to_actor( pHelp->text  , ch, FALSE );

            if ( str_cmp( pHelp->immtext, "") )  {
               if ( pHelp->immtext[0] == '.' )
                  help_to_actor( pHelp->immtext+1, ch, TRUE );
               else
                  help_to_actor( pHelp->immtext  , ch, TRUE );
            }
            return;
        }
    }

    { char nf[MSL]; snprintf( nf, MSL, "#%s", argall ); cmd_help( ch, nf ); }
//    send_to_actor( "No help on that word.\n\r", ch );
    return;
}



char *get_name_level( int level ) 
{
     switch( level ) {
          case 0: return "Newbie"; break;
          case 1:
          case 2:
          case 3:
          case 4: 
          case 5: return "Freshling"; break;
          case 6:
          case 7:
          case 8:
          case 9: 
          case 10: return "Adventurer"; break;
          case 11:
          case 12:
          case 13:
          case 14:
          case 15:
          case 16:
          case 17:
          case 18:
          case 19: return "Veteran";  break;    
          case 20:     
          case 21: 
          case 22:
          case 23:
          case 24:
          case 25:
          case 26: 
          case 27:
          case 28:
          case 29: return "Dungeoneer"; break;
          case 30:
          case 31:
          case 32:
          case 33:
          case 34:
          case 35:
          case 36:
          case 37:
          case 38:   
          case 39: return "Heroic"; break;
          case 40:
          case 41:
          case 42:
          case 43:
          case 44:
          case 45:
          case 46:
          case 47:
          case 48:
          case 49:
          case 50: return "Avatar"; break;  
          case 99: return "Transcendant"; break;
         case 100:
         case 101: return "Godlike"; break;
          default: return "Legendary"; break; 
        
       }
       return "Mortal"; 
} 
  


/*
 * New 'who' command originally by Alander of Rivers of Mud.
 * NEW 'who' command by Locke of The Isles.
 * Syntax:  who
 *          who [number] [number]
 *          who immortal
 */
void cmd_who( PLAYER *ch, char *argument )
{
    static int max = 0;
    char buf[MAX_STRING_LENGTH];
    CONNECTION *d;
    extern int num_hour;
#if defined(IMC)
    extern int imc_active;  /* from imc */
#endif
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    int gMatch;
    bool fImmortalOnly;

    /*
     * Set default arguments.
     */
    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL;
    fImmortalOnly  = FALSE;

    /*
     * Parse arguments.
     */
    nNumber = 0;
    for ( ;; )
    {
	char arg[MAX_STRING_LENGTH];

	argument = one_argument( argument, arg );
	if ( arg[0] == '\0' )
	    break;

	if ( is_number( arg ) )
	{
	    switch ( ++nNumber )
	    {
	    case 1: iLevelLower = atoi( arg ); break;
	    case 2: iLevelUpper = atoi( arg ); break;
	    default:
		send_to_actor( "Only two level numbers allowed.\n\r", ch );
		return;
	    }
	}
	else
	{
	    arg[3]    = '\0';
        if ( !str_prefix( arg, "immortal" ) )
	    {
		fImmortalOnly = TRUE;
	    }
        }
    }

    /*
     * Now show matching chars.
     */
    nMatch = 0;
    gMatch = 0;
    buf[0] = '\0';
    for ( d = connection_list; d != NULL; d = d->next )
    {
	PLAYER *wch;

	/*
	 * Check for match against restrictions.
	 * Don't use trust as that exposes trusted mortals.
	 */
    if ( !CONNECTED(d) || !can_see( ch, d->character ) )
        continue;

#ifdef NOWHO_BUILDERS
    if ( d->connected != NET_PLAYING && !IS_IMMORTAL(ch) ) continue;
#endif

	wch   = ( d->original != NULL ) ? d->original : d->character;

    if ( NPC(wch) ) continue;

/* editing imms are invis to morts */
    if ( d->connected != NET_PLAYING && !IS_IMMORTAL(ch) ) continue;

    if ( PC(wch,level) < iLevelLower
        ||   PC(wch,level) > iLevelUpper
        || ( fImmortalOnly  && PC(wch,level) <= LEVEL_HERO ) )
	    continue;

        /*
         * Format it up.
         */
         if (IS_HERO(wch) && PC(wch,level) == get_trust( wch ) )
         {
         char doe[MAX_STRING_LENGTH];
         gMatch++;

         if ( d->connected != NET_PLAYING )
         sprintf( doe, " -Creating- " );
         else
         {
         switch ( PC(wch,level) )
         {
                         default: sprintf( doe, " Immortal    " ); break;
                 case LEVEL_HERO: sprintf( doe, " Hero        " ); break;
              case LEVEL_SUPREME: sprintf( doe, " Agent       " ); break;
              case LEVEL_BUILDER: sprintf( doe, " Builder     " ); break;
              case LEVEL_ANGEL: sprintf( doe, " Writer      " ); break;
              case LEVEL_DEMIGOD: sprintf( doe, " Director    " ); break;
                  case MAX_LEVEL: sprintf( doe, " Producer    " ); break;
         }
         }
               if ( HAS_ANSI(ch) ) {
    strcat( buf, NTEXT ); strcat( buf, BOLD ); }
               sprintf( buf + strlen(buf), "[%-13s] ", doe );
               if (!IS_HERO(wch)) sprintf( doe, PC(wch,constellation), wch->name );
               else sprintf( doe, "%s%s%s", wch->name, 
  PC(wch,title) != NULL && PC(wch,title) != &str_empty[0] ? " " : "",
  PC(wch,title) != NULL && PC(wch,title) != &str_empty[0] ? 
  PC(wch,title) : "" );
               strcat( buf, doe );
               strcat( buf, "\n\r" );
        }
        else
        {
        nMatch++;
        if ( IS_IMMORTAL(ch) )
        sprintf( buf + strlen(buf), "[ Mortal   %2d ] %s %s   %s^+^N\n\r", 
wch->exp_level, NAME(wch), PC(wch,title),
!NPC(wch) && !MTD(PC(wch,flag)) ? PC(wch,flag) : ""
);
        else
        {
/* use for no visible morts in game: */
/*        if ( !CONNECTED(ch->desc) )  */ {
           char buf2[MAX_STRING_LENGTH];

          sprintf( buf2, "%s%s%s", wch->name, 
PC(wch,title) != NULL && PC(wch,title) != &str_empty[0] ? " " : "",
PC(wch,title) != NULL && PC(wch,title) != &str_empty[0] ? PC(wch,title) : "" );
         if ( HAS_ANSI(ch) ) {
    if ( wch->karma > 0 ) { strcat( buf, GREEN ); strcat( buf, BOLD );  }
         else
    if ( wch->karma > 100 ) { strcat( buf, B_BLUE ); strcat( buf, CYAN ); strcat( buf, BOLD ); } 
         else
    if ( wch->karma < -100 ) { strcat( buf, RED ); }
    else strcat( buf, YELLOW );
        } 
        sprintf( buf + strlen(buf), "[ %-11s ] %s%-28s Level %-15s %s^+^N\n\r", 
get_name_level(wch->exp_level),  
HAS_ANSI(ch) ? NTEXT : "", 
trunc_fit( buf2, 28 ),
capitalize(numberize(wch->exp_level)), 
!NPC(wch) && !MTD(PC(wch,flag)) ? PC(wch,flag) : ""
); 
        } 

        }
        }
    }

    display_interp( ch, "^B" );
    send_to_actor( buf, ch );
    display_interp( ch, "^N" );

    if (nMatch > 0)
    {
        snprintf( buf, MAX_STRING_LENGTH, "There %s %d visible player%s",
             nMatch > 1 ? "are" : "is",  nMatch,
             nMatch > 1 ? "s" : "" );
        send_to_actor( buf, ch );
    }

    if (gMatch > 0 || fImmortalOnly)
    {
        if (nMatch > 0)
        snprintf( buf, MAX_STRING_LENGTH, " and %d visible god%s", gMatch,
                      gMatch == 1 ? "" : "s" );
        else
        snprintf( buf, MAX_STRING_LENGTH, "There %s %d visible god%s",
             gMatch > 1 ? "are" : "is",  gMatch,
             gMatch > 1 ? "s" : "" );

        send_to_actor( buf, ch );
    }

    max = nMatch + gMatch > max ? nMatch + gMatch : max;
    snprintf( buf, MAX_STRING_LENGTH, ", with a high of %d.\n\r", max );

    if ( IS_IMMORTAL(ch) ) {
    if ( max > 0 ) send_to_actor( buf, ch );

    if ( num_hour / (60 * PULSE_PER_SECOND) < 60 )
    {
    snprintf( buf, MAX_STRING_LENGTH, "Next reboot will occur in %d minute%s.\n\r",
         num_hour / (60 * PULSE_PER_SECOND),
         num_hour / (60 * PULSE_PER_SECOND) != 1 ? "s" : "" );
    }
    else
    {
    snprintf( buf, MAX_STRING_LENGTH, "Next reboot will occur in %d hour%s.\n\r",
        num_hour / (60 * PULSE_PER_SECOND * 60)+1,
        num_hour / (60 * PULSE_PER_SECOND * 60)+1 != 1 ? "s" : "" );
    }
    send_to_actor( buf, ch );
    }

#if defined(IMC)
    if ( imc_active )
    send_to_actor( "Feeling lonely? Try typing 'imc' for relayed chat.\n\r", ch );
#endif
    return;
}



void show_inventory( PLAYER *ch, PLAYER *tch, bool fPeek )
{
    PROP *prop;
    bool ShowPockets = FALSE;
    bool ShowNothing = TRUE;
    
    for ( prop = ch->carrying; prop != NULL; prop = prop->next_content )
    {
        if ( fPeek
          && ch != tch
          && !IS_IMMORTAL(tch) 
          && !skill_check( tch, skill_lookup("peek"), 50 ) )
        continue;

        if ( !can_see_prop( tch, prop ) )
        continue;
        
        if ( prop->wear_loc == WEAR_NONE )
        ShowPockets = TRUE;
            
        if ( prop->item_type != ITEM_CONTAINER )
        continue;
        
        if ( IS_SET(prop->value[1], CONT_CLOSED) )
        {
            page_to_actor( capitalize(STR(prop,short_descr)), ch );
            page_to_actor( " is closed.\n\r", ch );
            ShowNothing = FALSE;
            continue;
        }
        
        if ( prop->contains == NULL )
        continue;
        
        ShowNothing = FALSE;
        act( "$p contains:", tch, prop, NULL, TO_ACTOR );
        if ( fPeek )
        show_peek_to_actor( prop->contains, tch, TRUE, TRUE, 
             LEARNED(tch,"peek") );
        else
        show_list_to_actor( prop->contains, tch, TRUE, TRUE );
    }
    
    if ( !ShowPockets )
    {
    if ( ShowNothing )
    act( "$E $t carrying nothing of note.", 
         tch, tch == ch ? "are" : "is", ch, TO_ACTOR );
    return;
    }

    act( "Tucked away in the folds of $S clothing is:", 
         tch, NULL, ch, TO_ACTOR );

    if ( fPeek )
    show_peek_to_actor( ch->carrying, tch, TRUE, TRUE, LEARNED(tch,"peek") );
    else
    show_list_to_actor( ch->carrying, tch, TRUE, TRUE );
    return;
}



void cmd_title( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    if ( NPC( ch ) ) return;

    free_string( PC(ch,title ));
    PC(ch,title) = str_dup( "" );
    if ( *argument == '\0' ) send_to_actor( "Title cleared.\n\r", ch );
    else { PC(ch,title) = str_dup( argument );  
           send_to_actor( "You are now known as ", ch );
           send_to_actor( NAME(ch), ch );
           send_to_actor( " ", ch );
           send_to_actor( PC(ch,title), ch );
           send_to_actor( ".\n\r", ch );
           snprintf( buf, MAX_STRING_LENGTH, "%s changes %s title to %s %s.\n\r", 
                    NAME(ch), HIS_HER(ch),
                    NAME(ch), PC(ch,title) );
           add_history( ch, buf );
    }  
    return;
}   

/*
 * Syntax:  inventory
 */
void cmd_inventory( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    PROP *h;

    if ( !NPC(ch) )
    display_interp( ch, color_table[PC(ch,colors[COLOR_PROP_LIST])].di );

    show_inventory( ch, ch, FALSE );
    if ( ch->riding )
    {
        page_to_actor( "In the packs of your mount:\n\r", ch );
        show_inventory( ch->riding, ch, FALSE );
    }

    h = get_eq_char( ch, WEAR_HOLD_1 );
    if ( h == NULL ) h = get_eq_char( ch, WEAR_WIELD_1 );
    if ( h != NULL )
    {
        snprintf( buf, MAX_STRING_LENGTH, (get_eq_char( ch, WEAR_WIELD_2 ) == NULL) 
                   && (get_eq_char( ch, WEAR_HOLD_2 )  == NULL) ?
                     "You %s %s in your hands.\n\r"             :
                     "You %s %s in your primary hand.\n\r",
                 h->wear_loc ==  WEAR_HOLD_1 ? "hold" : "wield",
                 PERSO(h, ch) );
        page_to_actor( buf, ch );
    }

    h = get_eq_char( ch, WEAR_HOLD_2 );
    if ( h == NULL ) h = get_eq_char( ch, WEAR_WIELD_2 );
    if ( h != NULL )
    {
        snprintf( buf, MAX_STRING_LENGTH, "In your secondary hand, you %s %s.\n\r",
                 h->wear_loc ==  WEAR_HOLD_2 ? "hold" : "wield",
                 PERSO(h, ch) );
        page_to_actor( buf, ch );
    }

    show_belt( ch, ch ); 

    page_to_actor( "\n\r", ch );

    if ( FALSE )
    { PLAYER *m;
      for ( m=ch->in_scene->people; m!=NULL; m=m->next_in_scene )
       if ( NPC(m) && m->master == ch && ch->riding!=m && m->carrying != NULL ) {
        page_to_actor( "\n\r", ch );
        act( "$N is carrying:", ch, NULL, m, TO_ACTOR );
        show_inventory( m, ch, FALSE );
      }
    }

    display_interp( ch, "^N" );
    return;
}


void show_belt( PLAYER *victim, PLAYER *ch )
{
    char buf[MAX_STRING_LENGTH];
    PROP *prop;
    int belt;

    buf[0] = '\0';

    for ( belt = WEAR_BELT_1;  belt < WEAR_BELT_5; belt++ )
    {
        if ( (prop = get_eq_char( victim, belt )) == NULL )
        continue;

        strcat( buf, "   " );
        strcat( buf, PERSO(prop, ch) );
        strcat( buf, "\n\r" );
    }

    if ( buf[0] != '\0' )
    {
        act( "From $N$y belt hangs:", ch, NULL, victim, TO_ACTOR );
        send_to_actor( buf, ch );
    }
    return;
}



/*
 * Syntax:  peek [person]
 */
void cmd_peek( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PLAYER *victim;
    
    one_argument( argument, arg );
    
    if ( arg[0] == '\0' || (victim = get_actor_scene( ch, arg )) == NULL )
    {
        send_to_actor( "Peek into whose inventory?\n\r", ch );
        return;
    }

    if ( !skill_check( ch, skill_lookup("peek"), 0 ) )
    {
        act( "$n is caught peeking at $N's inventory!", ch, NULL, victim, TO_NOTVICT );
        act( "You catch $n peeking at your inventory!", ch, NULL, victim, TO_VICT );
        send_to_actor( "You were caught!\n\r", ch );
    }

    show_inventory( victim, ch, TRUE );
    show_belt( victim, ch );
    return;
}



/*
 * Syntax:  equipment
 */
void cmd_equipment( PLAYER *ch, char *argument )
{ 
    clrscr(ch);

      if ( !NPC(ch) ) 
    display_interp( ch, color_table[PC(ch,colors[COLOR_PROP_LIST])].di );

  show_equipment_table( ch, ch );
 
   if ( FALSE )
    { PLAYER *pch;
    for ( pch=ch->in_scene->people; pch!=NULL; pch=pch->next_in_scene ) 
     if ( pch->master == ch && NPC(pch) ) {
      page_to_actor( "\n\r", ch );
      show_equipment_table( ch, pch );
     } 
    }

      if ( !NPC(ch) ) 
    display_interp( ch, "^N" );
    return;
}




/*
 * Syntax:  compare [prop] [prop]
 */
void cmd_compare( PLAYER *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    PROP *prop1;
    PROP *prop2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
	send_to_actor( "Compare what to what?\n\r", ch );
	return;
    }

    if ( ( prop1 = get_prop_inv( ch, arg1 ) ) == NULL )
    {
	send_to_actor( "You do not have that item.\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	for ( prop2 = ch->carrying; prop2 != NULL; prop2 = prop2->next_content )
	{
	    if ( prop2->wear_loc != WEAR_NONE
	    &&   can_see_prop( ch, prop2 )
	    &&   prop1->item_type == prop2->item_type
	    && ( prop1->wear_flags & prop2->wear_flags & ~ITEM_TAKE) != 0 )
		break;
	}

	if ( prop2 == NULL )
	{
	    send_to_actor( "You aren't wearing anything comparable.\n\r", ch );
	    return;
	}
    }
    else
    {
    if ( ( prop2 = get_prop_inv( ch, arg2 ) ) == NULL )
	{
	    send_to_actor( "You do not have that item.\n\r", ch );
	    return;
	}
    }
	    
    msg		= NULL;
    value1	= 0;
    value2	= 0;

    if ( prop1 == prop2 )
    msg = "You compare $p to itself.  It looks about the same.";
    else
    if ( prop1->item_type == prop2->item_type )
    {
       switch ( prop1->item_type )
       {
       default:
           msg = "It is impossible to to compare $p and $P.";
           break;

       case ITEM_GEM:
           value1 = prop1->value[1];
           value2 = prop2->value[1];
           if ( prop1->value[0] != prop2->value[0] )
           msg = "You shouldn't compare those gems; they are of a different element.";
           break;

       case ITEM_ARMOR:
           value1 = prop1->value[0] / 2;
           value2 = prop2->value[0] / 2;
           break;

       case ITEM_WEAPON:
       case ITEM_RANGED_WEAPON:
           value1 = prop1->value[1] + prop1->value[2] / 2;
           value2 = prop2->value[1] + prop2->value[2] / 2;
           break;

       case ITEM_AMMO:
           value1 = prop1->value[1];
           value2 = prop2->value[1];
           break;
       }
    }

    if ( msg == NULL )
    {
	     if ( value1 == value2 ) msg = "$p and $P look about the same.";
	else if ( value1  > value2 ) msg = "$p looks better than $P.";
	else                         msg = "$p looks worse than $P.";
    }

    act( msg, ch, prop1, prop2, TO_ACTOR );

    value1 = prop1->weight - prop2->weight;

         if ( value1 == 0 ) msg = NULL;
    else if ( value1 >  3 ) msg = "$p is heavier than $P.";
    else if ( value1 >  0 ) msg = "$p is slightly heavier than $P.";
    else if ( value1 > -3 ) msg = "$P is slightly heavier than $p.";
    else                    msg = "$P is heavier than $p.";

    if ( msg != NULL )     act( msg, ch, prop1, prop2, TO_ACTOR );
    return;
}




/*
 * Syntax:  credits
 */
void cmd_credits( PLAYER *ch, char *argument )
{
    cmd_help( ch, "nimud" );
    return;
}




/*
 * Syntax:  consider [person]
 */
void cmd_consider( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PLAYER *victim;
    char *buf = '\0';
    int hp;
    int curr_wis, curr_con, curr_str, curr_dex, curr_int;
    int plr_wis, plr_con, plr_str, plr_dex, plr_int;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_actor( "Consider killing whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_actor_scene( ch, arg ) ) == NULL )
    {
	send_to_actor( "They're not here.\n\r", ch );
	return;
    }

    cmd_look( ch, arg );
    if ( ch == victim ) { cmd_score( ch, "" ); return; }

    curr_wis = get_curr_wis(victim);
    curr_dex = get_curr_dex(victim);
    curr_con = get_curr_con(victim);
    curr_int = get_curr_int(victim);
    curr_str = get_curr_str(victim);

    plr_wis  = get_curr_wis( ch );
    plr_con  = get_curr_con( ch );
    plr_str  = get_curr_str( ch );
    plr_dex  = get_curr_dex( ch );
    plr_int  = get_curr_int( ch );

    {
      char buf2[MAX_STRING_LENGTH];
/*      int count = 0;*/
      buf2[0] = '\0';

    if ( curr_dex > plr_dex ) { strcat( buf2, "$E is more agile than you.\n\r" );    }
    if ( curr_str > plr_str ) { strcat( buf2, "$E is stronger than you.\n\r"  );  }
    if ( curr_int < 5 ) strcat( buf2, "$N has no mind to speak of.\n\r" );  
    if ( curr_wis < plr_wis ) strcat( buf2, "$N is unwise.\n\r" );
    act( buf2, ch, NULL, victim, TO_ACTOR );

    if ( curr_con > plr_con ) { act( "You are vital and hardened compared to $N." , ch, NULL, victim, TO_ACTOR ); }
                         else { act( "$N is more vital and hardened than you.", ch, NULL, victim, TO_ACTOR ); } 

     }

    sprintf( arg, "$E is " );
    if ( ( NPC(ch) && (IS_SET(victim->flag, ACTOR_GOOD) || 
IS_SET(victim->flag, ACTOR_BOUNTY)) ) || !NPC(ch) || victim->bounty > 0 ) {
     if ( victim->bounty <    70 ) strcat( arg, "a noted troublemaker.\n\r" );
else if ( victim->bounty <   500 ) strcat( arg, "an untrustworthy thief.\n\r" );
else if ( victim->bounty <  1000 ) strcat( arg, "considered a criminal.\n\r" );
else if ( victim->bounty < 10000 ) strcat( arg, "a fugitive from justice.\n\r" );
else                               strcat( arg, "a feared outlaw.\n\r" );
    act( arg, ch, NULL, victim, TO_ACTOR );
    }

    hp     = ch->hit - victim->hit;

   if ( GET_AC(ch) > GET_AC(victim) ) act( "You are well armored compared to $N.", ch, NULL, victim, TO_ACTOR ); 

 if ( hp >=  80 ) buf = "You are much healthier than $M.";
 if ( hp <=  60 ) buf = "You are healthier than $M.";
 if ( hp <=  30 ) buf = "You are a bit healthier than $M.";
 if ( hp <=   5 ) buf = "You are a slight bit healthier than $M.";
 if ( hp <=   0 ) buf = "$E is a slight bit healthier than you.";
 if ( hp <= -10 ) buf = "$E is a bit healthier than you.";
 if ( hp <= -30 ) buf = "$E is healthier than you.";
 if ( hp <= -80 ) buf = "$E is much healthier than you.";

    act( buf, ch, NULL, victim, TO_ACTOR );
    return;
}




/*
 * Syntax:  report
 */
void cmd_report( PLAYER *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    snprintf( buf, MAX_STRING_LENGTH, "You report that you are %s and %s.\n\r",
                  STRING_HITS(ch), STRING_MOVES(ch)  );
    send_to_actor( buf, ch );

    snprintf( buf, MAX_STRING_LENGTH, "$n reports that $e is %s and %s.",
                  STRING_HITS(ch), STRING_MOVES(ch) );
    act( buf, ch, NULL, NULL, TO_SCENE );

    return;
}




/*
 * Syntax:  socials
 */
void cmd_socials( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int iSocial;
    int col;

    clrscr(ch);
 
    col = 0;
    for ( iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++ )
    {
	snprintf( buf, MAX_STRING_LENGTH, "%-12s", social_table[iSocial].name );
	send_to_actor( buf, ch );
	if ( ++col % 6 == 0 )
	    send_to_actor( "\n\r", ch );
    }
 
    if ( col % 6 != 0 )
	send_to_actor( "\n\r", ch );
    return;
}



/*
 * Contributed by Alander.  Updates by Krayy.
 * Syntax:  commands
 */
void cmd_commands( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    int cmd;
    int col;

    clrscr(ch);
 
    col = 0;
    snprintf( buf, MAX_STRING_LENGTH, "\r\n" );

    for ( cmd = 0; command_table[cmd].name[0] != '\0'; cmd++ )
    {
            if ( command_table[cmd].level == -1 ) continue;

        if ( command_table[cmd].level <  LEVEL_HERO
        &&   command_table[cmd].level <= get_trust( ch )
        &&   command_table[cmd].position >= POS_DEAD )
	{
            if ( command_table[cmd].level == -2 )
            {
                strcat( buf, "\n\r" );
                strcat( buf, "\n\r" );

                if ( HAS_ANSI(ch) )
                strcat( buf, BOLD );

                strcat( buf, command_table[cmd].name );

                if ( HAS_ANSI(ch) )
                strcat( buf, NTEXT );

                strcat( buf, "\n\r" );
                col = 0;

            } else {

         sprintf( buf1, "%-12s", command_table[cmd].name );
         strcat( buf, buf1 );

         if ( ++col % 6 == 0 )
         strcat( buf, "\n\r" );
        }

	}
    }
 
    if ( col % 6 != 0 )
    strcat( buf, "\n\r" );

    page_to_actor( buf, ch );
    return;
}




void cmd_wizlist ( PLAYER *ch, char *argument )
{

    cmd_help ( ch, "wizlist" );
    return;

}



void cmd_history( PLAYER *ch, char *argument ) {

/*
 * History
 */
    char arg[MAX_STRING_LENGTH];
    PLAYER *pActor;

    clrscr(ch);

    if ( NPC(ch) ) return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' ) pActor = ch;
    else {

        pActor = get_actor_world( ch, arg );  

        if ( !pActor ) pActor = ch; 
        else if ( NPC(pActor) ) {
              sprintf( arg, "%s will most likely die in obscurity.\n\r", 
                       NAME(pActor) );
              send_to_actor( arg, ch );
              return;
        }
    }

    if ( NPC(pActor) ) return;

    send_to_actor( "\n\r", ch ); 
    sprintf( arg, "The History of %s\n\r\n\rAs recorded by Edigzorr the Great\n\r\n\r", 
             NAME(pActor) );

    send_to_actor( arg, ch );

    if ( PC(pActor,history) == NULL ) send_to_actor( "No history.\n\r", ch );
    else {
       page_to_actor( PC(pActor,history), ch );
       page_to_actor( "\n\r", ch );
    }
    return;
}          


void add_history( PLAYER *ch, char *_out )
{
    static char buf[MAX_STRING_LENGTH];

    if ( NPC(ch) ) return;

    if ( PC(ch,history) != NULL && strlen(PC(ch,history)) > MSL/2 ) 
    { free_string( PC(ch,history) );    
      PC(ch,history) = str_dup( "The story begins anew...\n\r" );
    }

    if ( PC(ch,history) != NULL )
    snprintf( buf, MAX_STRING_LENGTH, "%s%s\n\r", PC(ch,history), _out );
    else
    snprintf( buf, MAX_STRING_LENGTH, "%s\n\r", _out );

    if ( PC(ch,history) != NULL ) free_string( PC(ch,history) );
    PC(ch,history) = str_dup( buf );

    return;
}

