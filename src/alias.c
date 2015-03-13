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
#include <crypt.h>
#include <unistd.h>
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


ALIAS *aliases;

/*
 * Adds an alias to a char.
 */
void add_alias( PLAYER *ch, char *name, char *exp )
{
     ALIAS *alias;
     ALIAS *alias_next;

     if ( NPC(ch) ) return;

     alias_next = PC(ch,aliases);
     alias = new_alias( );
     alias->name = str_dup( name );
     alias->exp = str_dup( exp );
     alias->next = alias_next;
     PC(ch,aliases) = alias;
     return;
}

/*
 * Finds an alias from the command.
 */
ALIAS *find_alias( PLAYER *ch, char *exp )
{
     ALIAS *alias;

     if ( NPC(ch) ) return NULL;

     alias = PC(ch,aliases);
     if ( alias == NULL ) return NULL;

     for ( alias = PC(ch,aliases);  alias != NULL;  alias = alias->next )
      if ( !str_cmp( alias->name, exp ) ) return alias;

     return NULL;
}

/*
 * Syntax: alias 
 *         alias [name] edit
 *         alias [name] [command-list]
 *         alias [name] remove
 *         alias [name] delete
 *         alias off
 *         alias list
 *         alias clear
 *
 * Related feature: 
 *         set alias
 */
void cmd_alias( PLAYER *ch, char *argument )
{
     char arg[MAX_STRING_LENGTH];
     ALIAS *alias;

     if ( NPC(ch) ) return;

     argument = one_argument( argument, arg );
     
     if ( !IS_SET(ch->flag2, PLR_ALIASES) )
     {
     SET_BIT(ch->flag2,PLR_ALIASES);
     to_actor( "Aliases enabled.\n\r", ch );
     }

     if ( arg[0] == '\0' || !str_cmp( arg, "list" ) ) {
     int count=0;
     to_actor("Aliased commands:\n\r", ch );
     for ( alias=PC(ch,aliases); alias!=NULL; alias=alias->next ) {
          to_actor( alias->name, ch );
          to_actor( " -> ", ch );
          to_actor( alias->exp, ch );
          to_actor( "\n\r", ch ); count++;
     }
     sprintf( arg, "%d aliases\n\r", count );
     if ( count == 0 ) to_actor( "none\n\r", ch );
       else to_actor( arg, ch );
     return;
     }
  
     if ( !str_cmp( arg, "clear" ) ) {
       ALIAS *alias_next;
       for ( alias=PC(ch,aliases); alias!=NULL; alias=alias_next ) { alias_next = alias->next; free_alias(alias); }
       PC(ch,aliases)=NULL;
       to_actor( "Cleared.\n\r", ch );
       return;
     }

     if ( !str_cmp( arg, "off" ) ) {
     to_actor( "Aliases disabled.\n\r", ch );
     REMOVE_BIT(ch->flag2,PLR_ALIASES);
     return;
     }

     if ( (alias = find_alias( ch, arg ) ) == NULL ) {
     add_alias( ch, arg, argument );     
     to_actor( "Alias added.\n\r", ch );
     return;
     }

     if ( !str_prefix( argument, "delete" )
       || !str_prefix( argument, "remove" ) )
     {
        ALIAS *prev;
        alias = find_alias(ch,arg);
        if ( !alias ) to_actor( "Couldn't find that alias.\n\r", ch );
        else {
            ALIAS *a;
            if ( alias == PC(ch,aliases) ) { PC(ch,aliases)=PC(ch,aliases)->next;  free_alias(alias); return; }
            prev = PC(ch,aliases);
            a=prev->next;
            while ( alias != a ) { a=a->next; prev=prev->next; }
            prev->next=prev->next->next; free_alias(a);
            to_actor("Removed.\n\r", ch);
        }
        return;
     }

     if ( (alias = find_alias( ch, arg ) ) != NULL ) {
        ALIAS *prev;
        alias = find_alias(ch,arg);
        if ( !alias ) to_actor( "No such alias, creating new.\n\r", ch );
        else {
            ALIAS *a;
            if ( alias == PC(ch,aliases) ) { PC(ch,aliases)=PC(ch,aliases)->next;  free_alias(alias); return; }
            prev = PC(ch,aliases);
            a=prev->next;
            while ( alias != a ) { a=a->next; prev=prev->next; }
            prev->next=prev->next->next; free_alias(a);
           to_actor( "Alias revised.\n\r", ch );
        }
     add_alias( ch, arg, argument );     
     }
}

