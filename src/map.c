
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


struct MapData {
 SCENE *s;
};
                            /*N  E  S   W  U  D  NW NE  SW  SE */
static int dirx[MAX_DIR] =  { 0, 1, 0, -1, 0, 0, -1, 1, -1, 1 };  
static int diry[MAX_DIR] =  { 1, 0, -1, 0, 0, 0, 1, 1, -1, -1 };

char EXC( char c, EXIT *e );

char EXC( char c, EXIT *e ) {

 if ( !e ) return '+';

 if ( IS_SET(e->exit_flags,EXIT_TRANSPARENT) ) return '8';
 if ( IS_SET(e->exit_flags,EXIT_WINDOW) ) return 'O';
 if ( IS_SET(e->exit_flags,EXIT_NOMOVE) ) return 'X';
 if ( IS_SET(e->exit_flags,EXIT_SECRET) ) return 'S';
 if ( IS_SET(e->exit_flags,EXIT_CONCEALED) ) return 'C';
 if ( IS_SET(e->exit_flags,EXIT_JAMMED) ) return 'J';
 if ( IS_SET(e->exit_flags,EXIT_BASHPROOF) ) return '#';
 if ( IS_SET(e->exit_flags,EXIT_EAT_KEY) ) return 'U';
 if ( IS_SET(e->exit_flags,EXIT_ISDOOR) ) return 'D';

 return c;
}

void cmd_map( PLAYER *ch, char *argument ) {
 SCENE *start;
 static struct MapData map[5*5];
 char output[MSL];
 int x,y,E,E2;

 for ( x=0; x<5; x++ ) for ( y=0; y<5; y++ ) map[x+(y*5)].s=NULL;

 start=ch->in_scene;

 for ( E=0; E<MAX_DIR; E++ ) {
  SCENE *to;
  if ( E == DIR_UP || E == DIR_DOWN ) continue;
  if ( start->exit[E] && (to=start->exit[E]->to_scene) ) {
   map[(2+dirx[E])+(2+diry[E])*5].s=to;
   for ( E2=0; E2<MAX_DIR; E2++ ) {
    SCENE *to2;
    if ( E2 == DIR_UP || E2 == DIR_DOWN ) continue;
    if ( to->exit[E2] && (to2=to->exit[E2]->to_scene) ) {
     map[(2+dirx[E]+dirx[E2])+(2+diry[E]+diry[E2])*5].s=to2;
    }
   }
  }
 }

 for ( y=4; y>=0; y-- ) {
 for ( x=0; x<5; x++ ) if ( map[x+y*5].s ) {
  SCENE *here=map[x+y*5].s;
  char *c, b=' ';
  switch ( here->move ) {
   case MOVE_INSIDE:       c="^N";       b='-';  break;
   case MOVE_CITY:         c="^N^B";     b='-';  break;
   case MOVE_FIELD:        c="^#^2^B";   b='.';  break;
   case MOVE_FOREST:       c="^N^2^I";   b='T';  break;
   case MOVE_HILLS:        c="^N^7^I";   b='h';  break;
   case MOVE_MOUNTAIN:     c="^N^7^I^B"; b='M';  break;
   case MOVE_WATER_SWIM:   c="^N^:^3";   b='~';  break;
   case MOVE_WATER_NOSWIM: c="^N^:^7";   b='~';  break;
   case MOVE_UNDERWATER:   c="^N^1^I";   b='$';  break;
   case MOVE_AIR:          c="^N^=";     b='*';  break;
   case MOVE_DESERT:       c="^N^6";     b='&';  break;
   case MOVE_ICELAND:      c="^N^<";     b='=';  break;
   case MOVE_CLIMB:        c="^N^I";     b='-';  break;
    default: c="^N"; break;
  }
  sprintf( output, "%s%c%c%c%c%c^N  ", c, here->exit[DIR_NW] ? EXC('\\',here->exit[DIR_NW]) : '+', b, here->exit[DIR_NORTH] 
? EXC('|',here->exit[DIR_NORTH]) 
: b, b, here->exit[DIR_NE] ? EXC('/',here->exit[DIR_NE]) : '+' );
  to_actor( output, ch );
 }  else  to_actor( "       ", ch );
 to_actor( "\n\r", ch );

 for ( x=0; x<5; x++ ) if ( map[x+y*5].s ) {
  SCENE *here=map[x+y*5].s;
  char *c, b=' ';
  switch ( here->move ) {
   case MOVE_INSIDE:       c="^N";       b='|';  break;
   case MOVE_CITY:         c="^N^B";     b='|';  break;
   case MOVE_FIELD:        c="^#^2^B";   b='\'';  break;
   case MOVE_FOREST:       c="^N^2^I";   b='P';  break;
   case MOVE_HILLS:        c="^N^7^I";   b='n';  break;
   case MOVE_MOUNTAIN:     c="^N^7^I^B"; b='M';  break;
   case MOVE_WATER_SWIM:   c="^N^:^3";   b='~';  break;
   case MOVE_WATER_NOSWIM: c="^N^:^7";   b='~';  break;
   case MOVE_UNDERWATER:   c="^N^1^I";   b='$';  break;
   case MOVE_AIR:          c="^N^=";     b='*';  break;
   case MOVE_DESERT:       c="^N^6";     b='&';  break;
   case MOVE_ICELAND:      c="^N^<";     b='|';  break;
   case MOVE_CLIMB:        c="^N^I";     b='|';  break;
    default: c="^N"; break;
  }
  sprintf( output, "%s%c%s%s%s%c%c^N  ", c, here->exit[DIR_WEST] ? EXC('<',here->exit[DIR_WEST]) : '|', here->exit[DIR_UP] 
? "^^" : " ",
                                            (x==2&&y==2)?"^B@^N":" ", (x==2&&y==2) ? c : "",
                                            here->exit[DIR_DOWN] ? 'v' : ' ', here->exit[DIR_EAST] ? 
EXC('>',here->exit[DIR_EAST]) : '|' );
  to_actor( output, ch );
 }  else  to_actor( "       ", ch );
 to_actor( "\n\r", ch );

 for ( x=0; x<5; x++ ) if ( map[x+y*5].s ) {
  SCENE *here=map[x+y*5].s;
  char *c, b=' ';
  switch ( here->move ) {
   case MOVE_INSIDE:       c="^N";       b='-';  break;
   case MOVE_CITY:         c="^N^B";     b='-';  break;
   case MOVE_FIELD:        c="^#^2^B";   b='.';  break;
   case MOVE_FOREST:       c="^N^2^I";   b='T';  break;
   case MOVE_HILLS:        c="^N^7^I";   b='h';  break;
   case MOVE_MOUNTAIN:     c="^N^7^I^B"; b='M';  break;
   case MOVE_WATER_SWIM:   c="^N^:^3";   b='~';  break;
   case MOVE_WATER_NOSWIM: c="^N^:^7";   b='~';  break;
   case MOVE_UNDERWATER:   c="^N^1^I";   b='$';  break;
   case MOVE_AIR:          c="^N^=";     b='*';  break;
   case MOVE_DESERT:       c="^N^6";     b='&';  break;
   case MOVE_ICELAND:      c="^N^<";     b='=';  break;
   case MOVE_CLIMB:        c="^N^I";     b='-';  break;
    default: c="^N"; break;
  }
  sprintf( output, "%s%c%c%c%c%c^N  ", c, here->exit[DIR_SW] ? EXC('/',here->exit[DIR_SW]) : '+',
   b, here->exit[DIR_SOUTH] ? EXC('|',here->exit[DIR_SOUTH]) : b, b, here->exit[DIR_SE] ? EXC('\\',here->exit[DIR_SE]) : '+' );
  to_actor( output, ch );
 }  else  to_actor( "       ", ch );
 to_actor( "\n\r", ch );
 to_actor( "\n\r", ch );

 }

 return;

}




