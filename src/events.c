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
/******************************************************************************
 * Locke's   __ -based on merc v2.2-____        NIM Server Software           *
 * ___ ___  (__)__    __ __   __ ___|  | v5.1a  Version 5.1a (ALPHA)          *
 * |  /   \  __|  \__/  |  | |  |      |        unreleased+revamped 2003      *
 * |       ||  |        |  \_|  | ()   |                                      *
 * |    |  ||  |  |__|  |       |      |                                      *
 * |____|__|___|__|  |__|\___/__|______|        http://www.nimud.org/nimud    *
 *   n a m e l e s s i n c a r n a t e          dedicated to chris cool       *
 ******************************************************************************/

/*
 * Locke's event queue for scripts.
 * Part of NiMUD Version 5.1a. 
 * See readme.txt and license.txt
 */


#if defined(BSD)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <zlib.h>

#include "nimud.h"
#include "script.h"
#include "defaults.h"

EVENT *event_queue;

void assign_var_trig( void * owner, int type, INSTANCE *instance, VARIABLE *var, char *name );

void assign_var_trig( void * owner, int type, INSTANCE *instance, 
                      VARIABLE *var, char *name )
{
    VARIABLE *v;

    if (instance == NULL ) return;
    if (var == NULL) return;

    v = new_variable( TYPE_STRING, NULL );
    v->next = instance->locals;
    instance->locals = v;

    v->name = str_dup( name );
    v->type = var->type;
    v->value = var->type == TYPE_STRING ? str_dup( var->value ) : 
               var->value;
    return;
}


/*
 * Adds a scripted event to the queue.
 */ 
void add_event( void * owner, int type, int dbkey, int delay, 
                char *special, PLAYER *actor, PLAYER *target,
                PROP *catalyst, char *astr, char *bstr   ) 
{
    EVENT *pEvent;
    SCRIPT *script;
    INSTANCE *pTrig;
    VARIABLE *var;

    script = get_script( dbkey );
    if ( script == NULL ) return;
    if ( owner == NULL ) return; 

    pTrig = new_instance( );    
    pEvent = new_event( ); 

    pEvent->time = delay;
    pEvent->instance = pTrig;
    pTrig->script = script;
    pTrig->location = pTrig->script->commands;  /* Start! */

    var = new_variable( TYPE_STRING, NULL );

    if ( actor )
            {
                var->type  = TYPE_ACTOR;
                var->value = (PLAYER *)actor;
                assign_var_trig( owner, type, pTrig, var, "%actor%" );

                var->type = TYPE_STRING;
                var->value = (char *)str_dup( STR((PLAYER *)actor,name));
                assign_var_trig( owner, type, pTrig, var, "%aname%" );
                free_string((char *)var->value);
            }

    if ( target )
            {
                var->type  = TYPE_ACTOR;
                var->value = (PLAYER *)target;
                assign_var_trig( owner, type, pTrig, var, "%target%" );

                var->type = TYPE_STRING;
                var->value = (char *)str_dup( STR((PLAYER *)target,name));
                assign_var_trig( owner, type, pTrig, var, "%tname%" );
                free_string((char *)var->value);
            }



     if ( catalyst )
            {
                var->type = TYPE_PROP;
                var->value = (PROP *)catalyst;
                assign_var_trig( owner, type, pTrig, var, "%catalyst%" );
            }

     if ( astr )
            {
                var->type = TYPE_STRING;
                var->value = str_dup( astr );
                assign_var_trig( owner, type, pTrig, var, "%astr%" );
            }

     if ( bstr )
            {
                var->type = TYPE_STRING;
                var->value = str_dup( bstr );
                assign_var_trig( owner, type, pTrig, var, "%bstr%" );
            }

    free_variable( var );

    sprintf( log_buf, "Notify> Event added [%d] %s %s %s", delay, type == TYPE_ACTOR ? "actor" : "prop/scene",
                                                           actor ? NAME(actor) : "",
                                                           target ? NAME(target) : ""  );
    NOTIFY( log_buf, LEVEL_IMMORTAL, WIZ_NOTIFY_EVENT );

    pEvent->next = event_queue;  
    event_queue = pEvent;
    return; 
}


void rem_event( EVENT *event ) 
{
    EVENT *pEvent;
    EVENT *event_next;
    EVENT *event_prev;    

    if ( event == NULL || event_queue == NULL ) return;

    /*
     * Remove the head of the chain.
     */
    if ( event == event_queue ) {
         event_queue = event->next; 
         free_event( event );  
         return; 
    }

    /*
     * Remove the tail of the chain.
     */
    if ( event->next == NULL )
    {

    for ( pEvent = event_queue;  pEvent != NULL;  pEvent = pEvent->next )
    if ( pEvent->next == event ) break;  
 
    if ( pEvent == NULL ) return;

    pEvent->next = NULL;
    free_event( event );
    return;
    }

    /*
     * Remove something in the middle. 
     */
    event_prev = event_queue;
    for ( pEvent = event_queue;  pEvent != NULL;  pEvent = event_next )
    {
        event_next = pEvent->next;
        if ( pEvent == event ) {
             for ( event_prev = event_queue;  event_prev != NULL; 
                   event_prev = event_prev->next ) {
                 if ( event_prev->next == event ) {
                      event_prev->next = event_prev->next->next;
                      free_event( event );
                    }
               }
        }
    }       

    return;
}

/*
 * Called from Update Handler
 * Updates future events, removes past events, runs all current events 
 */
void update_event( void ) {
    EVENT *pEvent;
    EVENT *pEvent_next;

    for ( pEvent = event_queue;  pEvent != NULL;  pEvent = pEvent_next ) {
         pEvent_next = pEvent->next;
         if ( pEvent->time-- <= 0 )  {
            pEvent->time = 0;            
            if (pEvent->instance->location == NULL) rem_event( pEvent );  
            else 
            parse_script( pEvent->instance, pEvent->owner, pEvent->type );
         }
    }
}  

void clear_events( void * owner, int type ) {
    EVENT *pEvent, *pEvent_next;

    for ( pEvent = event_queue; pEvent != NULL; pEvent = pEvent_next )  {
          pEvent_next = pEvent->next; 
          if ( pEvent->type == type
            && pEvent->owner == NULL ) rem_event( pEvent );
    } 
} 


void cmd_events( PLAYER *ch, char *argument ) {
    PROP *pProp = NULL;
    PLAYER *pActor = NULL;
    SCENE *pScene = NULL;
    char buf[MAX_STRING_LENGTH];
    EVENT *pEvent;

    to_actor( "Current events:\n\r", ch );
    for ( pEvent = event_queue;   pEvent != NULL;   pEvent = pEvent->next ) {

          pActor = NULL;
          pProp = NULL;
          pScene = NULL;

          if ( pEvent->type == TYPE_ACTOR ) 
          pActor = (PLAYER *)(pEvent->owner); 
          if ( pEvent->type == TYPE_PROP ) 
          pProp = (PROP *)(pEvent->owner); 
          if ( pEvent->type == TYPE_SCENE ) 
          pScene = (SCENE *)(pEvent->owner);
          if ( pEvent->type == TYPE_STRING ) continue;  

          sprintf( buf, "Delay: [%5d]   Caller: [%11s]   Name: [%11s]\n\r",
                pEvent->time, 
                pActor ? STR(pActor,short_descr) :
               (pProp ? STR(pProp,short_descr) : 
               (pScene ? pScene->name : "" )), pEvent->instance->script->name );
 
          to_actor( buf, ch );
    } 

    return;
}
