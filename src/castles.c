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

/*
 * Display information about a kingdom to an actor.
 */
void show_kingdom_to_actor( CASTLE_DATA *keep, PLAYER_DATA *ch, int accuracy ) {

    if ( accuracy == 100 ) {

    } else {

    }
}

/*
 * Spy <target>
 *
 * Expends 1 assassin, but reports back on foreign kingdoms
 * that are unaligned.
 */
void cmd_spy( ) {

}

/*
 * Kingdom command; shows information
 *
 * Kingdom            (shows to self about hero's kingdom)
 * Kingdom <target>   <- information is gained about the ally target
 */
void cmd_kingdom( ) {

}

/*
 * Parley <target> war          <- declare war on enemy
 * Parley <target> peace        <- attempt to declare peace
 * Parley <target> alliance     <- request an alliance
 * Parley <target> message      <- send a message
 */
void cmd_parley( ) {
}

/*
 * Ways to manipulate favor and standing among the people.
 * Good standing costs money but increases population growth.
 * Lower taxes also increase immigration.
 * Holidays immediately improve public standing.
 * Festivals cost money but increase population.
 * Decreeing no taxes vastly decreases internal dissidence.
 *
 * Decree taxes <percentage>
 * Decree no taxes
 * Decree festival <days>
 * Decree holiday
 */
void cmd_decree( ) {
}


/*
 * Attempts to build a new castle.
 */
void cmd_commission( ) {
}
