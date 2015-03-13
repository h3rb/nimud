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


/*
 * Thanks again, Morg.
 * Modifications by Locke...
 */
char * garble_text( char *t, int prcnt_comp )
{
   static char buf[MAX_STRING_LENGTH];
   int nChars = 0;
   int rand1, rand2;
   
   static char actor_table[] =
   {
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
      'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
   };

   if( t == NULL || t[0] == '\0' )
   {
      bug( "Garble_text: t NULL.", 0 );
      return t;
   }

   if( strlen( t ) > MAX_STRING_LENGTH )
   {
      bug( "Garble_text: Argument to long.", 0 );
      return t;
   } 

   if( prcnt_comp <= 0   )  prcnt_comp = 1;
   if( prcnt_comp >= 90 )  return t;

   for( rand1 = 0; rand1 < strlen( t ); rand1++ )
      if( isalpha( t[rand1] ) )
         nChars++;

   nChars -= nChars * prcnt_comp / 100;

   if( nChars <= 0 )
      return t;

   buf[0] = '\0';
   strcpy( buf, t );
   for( ; nChars > 0; nChars-- )
   {
      rand1 = number_range( 0, strlen( buf ) - 1 );
      if( !isalpha( buf[rand1] ) )
      {
         nChars++;
         continue;
      }
      rand2 = number_range( 0, 24 );
      buf[rand1] = buf[rand1] >= 'A' && buf[rand1] <= 'Z' ?
       UPPER( actor_table[rand2] ) : actor_table[rand2];
   }   

   return buf;
}



/*
 * Fun fun fun.
 */
int check_speech( PLAYER *ch, PLAYER *speaker, int gsn )
{
   SKILL *pSkill = find_group_pc( ch, get_skill_index( gsn ) );
   SKILL *pIndex = get_skill_index( gsn );
   int percent;

   if ( !pSkill || !pIndex ) return 0;

   percent = pSkill->skill_level;

   if( !NPC(ch) && IS_SET(ch->flag2, WIZ_TRANSLATE) )
      return 100;

   if( !NPC(speaker) && IS_SET(speaker->flag2, WIZ_TRANSLATE) )
     return 100;

   if ( percent >= pIndex->max_learn )
     return 100;

   if ( percent < 90 )
   percent += number_range( 0, 15 ) - 8;  /* Language incentive*/

   
   if( ch != speaker && pSkill->skill_level < percent )
     skill_check( ch, pSkill, percent );
    

   return percent;
}


