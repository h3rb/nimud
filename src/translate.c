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


#if defined(TRANSLATE)

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libintl.h>

#include <zlib.h>

#include "nimud.h"

/*
 * Translation routines for multi-language translation.
 * Uses the gettext standard libintl.h
 * READ /docs/multilanguage.faq
 */

void cmd_translate( PLAYER *ch, char *argument ) {
     int t;

     if ( ch->desc == NULL ) return;

     if ( *argument != '\0' ) {
     t = find_language( argument );
     if ( t == -1 ) { 
     send_to_actor( "Language not found or in list of supported languages.\n\r", ch );
     return;
     }
     if ( t != ch->desc->lingua ) {
          send_to_actor( "Language translation set to ", ch );
          send_to_actor( lingua_table[t].name, ch );
          send_to_actor( ".\n\r", ch );
          return;
     }
     }

     t = ch->desc->lingua;
     send_to_actor( "Language: ", ch );
     send_to_actor( lingua_table[t].name, ch );
     send_to_actor( ".\n\r", ch );
     return;
}


int find_language( char *argument ) {
     int i;

     for ( i = 0; i < MAX_LINGUA; i++ ) {
          if ( !str_prefix( argument, lingua_table[i].name ) )
               return i;
     }
     return -1;
}

char *translate( CONNECTION *d, char *outbuf ) {
     char buf[MAX_STRING_LENGTH];

     if ( d->lingua ==  0 ) 
         return str_dup( outbuf );

     return str_dup( gettext( "../lingua/translated" ) );
};

#endif

/*
 * Please note, while it is not supported, a multi-lingual
 * translator for the purpose of modifying code does exist
 * in OpenGPL/open source on sourceforge.net-- I didn't
 * get into it too much because it seemed to be not worth
 * the trouble (I speak English very well, in fact, and
 * don't need it to code) -- but it is possible to translate
 * just the comments.
 *
 * Also, linguaphile can translate the docs, but in the
 * interest of saving disc space, I did not include translated
 * /docs/ dir versions, nor pre-translated help files.
 * 
 * Instead we've opted to "do it on the fly" --
 * good luck!
 * 
 * -locke
 */
