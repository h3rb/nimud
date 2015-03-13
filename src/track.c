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
#include "skills.h"


/***************************************************************************
 *   File: graph.c                                       Part of CircleMUD *
 *  Usage: various graph algorithms                                        *
 *                                                                         *
 *  All rights reserved.  See license.doc for complete information.        *
 *                                                                         *
 *  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
 *  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
 ***************************************************************************/


#define TRACK_THROUGH_DOORS

/*
 * You can define or not define TRACK_THROUGH_DOORS, above, depending on
 * whether or not you want track to find paths which lead through closed
 * or hidden doors.
 */

/*
 * BFS return values.
 */
#define BFS_ERROR           -1 
#define BFS_ALREADY_THERE   -2  
#define BFS_NO_PATH         -3


struct bfs_queue_struct 
{
   SCENE *scene;
   int   dir;
   int   depth;
   struct bfs_queue_struct *next;
};

struct scene_list_struct 
{
   SCENE *scene;
   struct scene_list_struct *next;
};

static struct bfs_queue_struct *queue_head = NULL, *queue_tail = NULL;

static struct scene_list_struct *list_head = NULL, *list_tail = NULL;

/* Utility macros */
#define MARK( scene )          ( SET_BIT( ( scene )->scene_flags, SCENE_MARK))
#define UNMARK( scene )        ( REMOVE_BIT( ( scene )->scene_flags, SCENE_MARK))
#define IS_MARKED( scene )     ( IS_SET( ( scene )->scene_flags, SCENE_MARK))
#define TOSCENE( scene, y )     ( ( scene )->exit[ ( y ) ]->to_scene )
#define IS_CLOSED( scene, y )  ( IS_SET( ( scene )->exit[( y )]->exit_flags, \
                                EXIT_CLOSED))

#ifdef TRACK_THROUGH_DOORS
#define VALID_EDGE( scene, y )                                              \
                              ( ( scene )->exit[( y )] &&                   \
                              ( TOSCENE(( scene ), ( y ) ) != NULL) &&       \
                              ( !IS_MARKED( TOSCENE( ( scene ), ( y ) ) ) ) )
#else
#define VALID_EDGE( scene , y )                                             \
                              ( ( scene )->exit[ ( y ) ] &&                 \
                              ( TOSCENE( ( scene ), ( y ) ) != NULL ) &&     \
                              ( !IS_CLOSED( (scene), ( y ) ) ) &&           \
                              ( !IS_MARKED( TOSCENE( ( scene ), ( y ) ) ) ) )
#endif

void list_enqueue( SCENE *scene )
{
   static struct scene_list_struct *curr;

   curr = alloc_mem( sizeof( struct scene_list_struct ) );
   curr->scene = scene;
   curr->next = NULL;

   if( list_tail != NULL )
   {
      list_tail->next = curr;
      list_tail = curr;
   }
   else
      list_head = list_tail = curr;

   return;
}

void bfs_enqueue( SCENE *scene, int dir, int depth )
{
   struct bfs_queue_struct *curr;

   curr = alloc_mem( sizeof( struct bfs_queue_struct ) );
   curr->scene = scene;
   curr->dir = dir;
   curr->depth = depth + 1;
   curr->next = NULL;

   if( queue_tail != NULL )
   {
      queue_tail->next = curr;
      queue_tail = curr;
   }
   else
      queue_head = queue_tail = curr;

   list_enqueue( scene );

   return;
}

void bfs_dequeue(void)
{
   struct bfs_queue_struct *curr;

   curr = queue_head;

   if( (queue_head = queue_head->next) == NULL )
      queue_tail = NULL;

   free_mem( curr, sizeof( struct bfs_queue_struct ) );
   return;
}

void list_dequeue( void )
{
   struct scene_list_struct *curr;

   curr = list_head;

   if( (list_head = list_head->next) == NULL )
      list_tail = NULL;

   UNMARK( curr->scene );
   free_mem( curr, sizeof( struct scene_list_struct ) );
   return;
}

void bfs_clear_queue(void) 
{
   while( queue_head != NULL )
      bfs_dequeue();
   return;
}

void list_clear_queue( void )
{
   while( list_head != NULL )
      list_dequeue();
}

/* find_first_step: given a source scene and a target scene, find the first
   step on the shortest path from the source to the target.

   Intended usage: in actor_activity, give a actor a dir to go if they're
   tracking another actor or a PC.  Or, a 'track' skill for PCs.
*/


int find_first_step( SCENE *src, SCENE *target, int depth )
{
   int curr_dir;

   if ( src == NULL || target == NULL ) {
      bug("Illegal value passed to find_first_step (graph.c)", 0 );
      return BFS_ERROR;
   }

   if (src == target)
      return BFS_ALREADY_THERE;

   queue_head = queue_tail = NULL;
   list_head = list_tail = NULL;

   MARK( src );
   list_enqueue( src );

   /* first, enqueue the first steps, saving which direction we're going. */
   for (curr_dir = 0; curr_dir < MAX_DIR; curr_dir++)
      if ( VALID_EDGE( src, curr_dir ) ) 
      {
         MARK( TOSCENE( src, curr_dir ) );
         bfs_enqueue( TOSCENE( src, curr_dir ), curr_dir, 0 );
      }

   /* now, do the classic BFS. */
   while( queue_head )
   {
      if( queue_head->depth >= depth )
      {
         bfs_clear_queue();
         list_clear_queue();
         return BFS_NO_PATH;
      }
      if( queue_head->scene == target ) 
      {
	 curr_dir = queue_head->dir;
	 bfs_clear_queue();
         list_clear_queue();
	 return curr_dir;
      }
      else
      {
         for( curr_dir = 0; curr_dir < MAX_DIR; curr_dir++ )
         {
            if (VALID_EDGE( queue_head->scene, curr_dir ) ) 
            {
               MARK( TOSCENE( queue_head->scene, curr_dir ) );
	       bfs_enqueue( TOSCENE( queue_head->scene, curr_dir  ),
                queue_head->dir, queue_head->depth );
            }
         }
         bfs_dequeue();
      }
   }
   list_clear_queue();
   return BFS_NO_PATH;
}


/************************************************************************
 *  Functions and Commands which use the above fns                      *
 ************************************************************************/


/*
 * Caution: called each combat pulse.
 */

void cmd_hunt( PLAYER *ch, char *argument )
{
 
/*     char buf[MAX_STRING_LENGTH];*/
     char arg[MAX_INPUT_LENGTH];

     one_argument( argument, arg );


     if ( !str_cmp( arg, "continue" ) ) {
     if ( ch->position == POS_FIGHTING 
      || MTD(ch->hunting) ) 
         return;

         cmd_track( ch, ch->hunting );
         return;
     }

     if ( arg[0] == '\0' ) {
        send_to_actor( "You have stopped hunting", ch );

        if ( MTD(ch->hunting) ) {
        send_to_actor( " ", ch );
        send_to_actor( ch->hunting, ch );
        send_to_actor( ".\n\r", ch );
        }
        else {
        send_to_actor( ".\n\r", ch );
        }

        free_string( ch->hunting );
        ch->hunting = str_dup( "" );
        free_string( ch->tracking );
        ch->tracking = str_dup( arg );
     }
     else
     {
         free_string( ch->tracking );
         ch->tracking = str_dup( "" );
         ch->hunting = str_dup( arg );
         cmd_track( ch, arg );
     }

     return;
}

/*
 * Syntax:  track [person]
 */
void cmd_track ( PLAYER *ch, char *argument ) 
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   PLAYER *vict;
   int dir;
   one_argument(argument, arg);

   if ( arg[0] == '\0' && MTD(ch->tracking) )
   {
      send_to_actor( "Track whom?\n\r", ch);
      return;
   }
     
      if ( !str_cmp( arg, "stop" ) ) {
         free_string( ch->tracking );
         ch->tracking = str_dup( arg );
         send_to_actor( "You stop tracking your prey.\n\r", ch );
      }

   if ( ( arg[0] == '\0' && !MTD(ch->tracking) )
     && !str_cmp( arg, "continue" ) )
   {
      if ( ( vict = get_actor_world( ch, arg ) ) == NULL )
      {
         send_to_actor( "There is no path that leads there.\n\r", ch );
         free_string( ch->tracking );
         ch->tracking = str_dup( "" );
         return;
      }

      if ( ch == vict && !MTD(ch->tracking) )
      {
         send_to_actor( "You stop tracking your prey.\n\r", ch );
         free_string( ch->tracking );
         ch->tracking = str_dup( "" );
         return;
      }

   }

   if ( MTD(ch->tracking) && arg[0]== '\0' )
   {
      send_to_actor( "You are not tracking anyone.\n\r", ch );
      free_string( ch->tracking );
      ch->tracking = str_dup( "" );
      return;
   }

   if( ( vict = get_actor_world( ch, ch->tracking ) ) == NULL )
   {
      send_to_actor("You cannot find your prey.\n\r", ch);
      free_string( ch->tracking );
      ch->tracking = str_dup( "" );
      return;
   }

   dir = find_first_step( ch->in_scene, vict->in_scene, TRACK_DEPTH(ch) );

   switch( dir )
   {
      case BFS_ERROR:
         bug( "cmd_track: BFS Error", 0 );
         ch->tracking = NULL;
         break;
      case BFS_ALREADY_THERE:
         free_string( ch->tracking );
         ch->tracking = str_dup( "" );
         break;
      case BFS_NO_PATH:
         snprintf( buf, MAX_STRING_LENGTH, "The trail to %s leads nowhere.\n\r", HIM_HER(vict) );
         send_to_actor(buf, ch);
         free_string( ch->tracking );
         ch->tracking = str_dup( "" );
         break;
      default:
         /*
          * The next few lines make it give you a random direction if you
          * fail the random skill roll.
          */
         if( !skill_check( ch, skill_lookup( "track" ), 0 ) )
           dir = number_percent( ) % MAX_DIR;

         sprintf(buf, "The trail leads off to %s.\n\r",
                      dir_rev[rev_dir[dir]] );
         send_to_actor(buf, ch);
         if ( !MTD(ch->hunting) ) {
             interpret( ch, dir_name[dir] );   /* WATCH FOR RECURSION */
         }
         if( ch->tracking[0] == '\0' )
            ch->tracking = str_dup( arg );
         break;
   }
   return;
}
