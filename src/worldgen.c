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

// historic:
/****************************************************************************
******************************************************************************
* Locke's   __ -based on merc v5.0-____        NIM Server Software           *
* ___ ___  (__)__    __ __   __ ___| G| v4.0   Version 4.0 GOLD EDITION      *
* |  /   \  __|  \__/  |  | |  |     O|        documentation release         *
* |       ||  |        |  \_|  | ()  L|        Hallow's Eve 1999             *
* |    |  ||  |  |__|  |       |     D|                                      *
* |____|__|___|__|  |__|\___/__|______|        http://www.nimud.org/nimud    *
*   n a m e l e s s i n c a r n a t e          dedicated to chris cool       *
******************************************************************************
 *   ___  ___  ___  __    __                                                *
 *  |   ||   ||   ||  |\/|  | 2-x    NiMUD is a software currently under    *
 *   |  \ | |  | |  | |\/| |         development.  It is based primarily on *
 *   | |\\| |  | |  | |  | |         the discontinued package, NiMUD 2.2.    *
 *   | | \  |  | |  | |  | |         NiMUD is being written and developed   *
 *  |___||___||___||__|  |__|        By Locke and Surreality as a new,      *
 *   NAMELESS INCARNATE *MUD*        frequently updated package similar to  *
 *        S O F T W A R E            the original NiMUD 2.x.                 *
 *                                                                          *
 *  Just look for the Iron Maiden skull wherever NiMUD products are found.  *
 *                                                                          *
 *  Much time and thought has gone into this software and you are           *
 *  benefitting.  We hope that you share your changes too.  What goes       *
 *  around, comes around.                                                   *
 ****************************************************************************/

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
 * The following code is for the creation of a generated world file.
 * Each array is used internally by the processor in order to create
 * a larger world.  Populate this world as you wish.
 *
 * Terrain data is referenced here, but is not actually copied to the
 * scene.  This allows a dynamic terrain space, referenced as needed.
 * Set constraints based on maximum map size.
 *
 * Terrain types are stored in the mud.nfo zone file.
 */

/*
 * Size constraints.
 */
#define MAP_X       400
#define MAP_Y       400


/*
 * Command line terrain editing.
 * Syntax: terrain <dbkey> <command> <value>
 */
void cmd_terrain( PLAYER *ch, char *argument )
{
    TERRAIN *pTerrain;
    char arg[MAX_INPUT_LENGTH];
    int dbkey, value;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_actor( "Terrain syntax:\n\r", ch );
        send_to_actor( "   terrain new\n\r", ch );
        send_to_actor( "   terrain list\n\r", ch );
        send_to_actor( "   terrain <dbkey> <command|field|movetype> <value>\n\r\n\r", ch );
        send_to_actor( "Commands:  show\n\r", ch );
        send_to_actor( "Fields:    winter summer fall spring name map\n\r", ch );
        return;
    };

    if ( !str_cmp( arg, "list" ) )
    {
        for ( pTerrain = terrain_list;  pTerrain != NULL;  pTerrain = pTerrain->next )
        {
            sprintf( arg, "[%3d] %c %s\n\r", pTerrain->dbkey, pTerrain->map_char, pTerrain->name );
            send_to_actor( arg, ch );
        }

        return;
    }

    if ( !str_cmp( arg, "new" ) )
    {
        pTerrain = new_terrain( );
        pTerrain->next = terrain_list;
        terrain_list = pTerrain;
        send_to_actor( "Terrain created.\n\r", ch );
        return;
    }

    dbkey = atoi(arg);
    argument = one_argument( argument, arg );
    value = atoi(arg);

    for ( pTerrain = terrain_list;  pTerrain != NULL;  pTerrain = pTerrain->next )
    {
        if ( dbkey == pTerrain->dbkey )
        break;
    }

    if ( pTerrain == NULL )
    {
        send_to_actor( "Terrain not found.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "name" ) )
    {
        free_string( pTerrain->name );
        pTerrain->name = str_dup( argument );
        send_to_actor( "Terrain name set.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "spring" ) )
    {
        string_append( ch, &pTerrain->spring );
        return;
    }

    if ( !str_cmp( arg, "summer" ) )
    {
        string_append( ch, &pTerrain->summer );
        return;
    }

    if ( !str_cmp( arg, "fall" ) )
    {
        string_append( ch, &pTerrain->fall   );
        return;
    }

    if ( !str_cmp( arg, "winter" ) )
    {
        string_append( ch, &pTerrain->winter );
        return;
    }

    if ( !str_cmp( arg, "map" ) )
    {
        pTerrain->map_char = *argument;
        send_to_actor( "Map character set.\n\r", ch );
        return;
    }

    if ( move_number( arg ) != MOVE_MAX )
    {
         pTerrain->move  = move_number( arg );
         send_to_actor( "Sector type set.\n\r", ch );
         return;
    }

    /* Otherwise, show... */

    sprintf( arg, "Name:  [%s]\n\r", pTerrain->name );
    send_to_actor( arg, ch );

    sprintf( arg, "Vnum:  [%3d]   Sector:  [%s]    Map char: [%c]\n\r",
             pTerrain->dbkey,
             move_name( pTerrain->move ),
             pTerrain->map_char );
    send_to_actor( arg, ch );

    sprintf( arg, "Spring:\n\r%sSummer:\n\r%sFall:\n\r%sWinter:\n\r%s",
             pTerrain->spring,
             pTerrain->summer,
             pTerrain->fall,
             pTerrain->winter );
    send_to_actor( arg, ch );

    return;
};




void show_terrain( PLAYER *ch, SCENE *pScene )
{
    char *description;
    TERRAIN *pTerrain;

    if ( pScene->terrain == 0 )
    return;

    for ( pTerrain = terrain_list;  pTerrain != NULL;  pTerrain = pTerrain->next )
    {
        if ( pScene->terrain == pTerrain->dbkey )
        break;
    };


    if ( pTerrain == NULL )
    return;

    description = NULL;

    switch ( weather.month )
    {
        case 0:  /* winter */
        case 1:
        case 11:
             description = pTerrain->winter;
         break;

        case 2:  /* spring */
        case 3:
        case 4:
             description = pTerrain->spring;
         break;

        case 5:  /* summer */
        case 6:
        case 7:
             description = pTerrain->summer;
          break;

        case 8:  /* fall   */
        case 9:
        case 10:
             description = pTerrain->fall;
         break;
    };

    if ( description == NULL ) return;

    send_to_actor( description, ch );
    return;
}


const     int    tdir[MAX_DIR][2] =
{ /*   x  y    */
    {  0, -1  },  /* DIR_NORTH */
    {  1,  0  },  /* DIR_EAST  */
    {  0,  1  },  /* DIR_SOUTH */
    { -1,  0  },  /* DIR_WEST  */
    {  0,  0  },  /* DIR_UP not supported */
    {  0,  0  },  /* DIR_DOWN not supported */
    { -1, -1  },  /* DIR_NW    */
    {  1, -1  },  /* DIR_NE    */
    { -1,  1  },  /* DIR_SW    */
    {  1,  1  }   /* DIR_SE    */
};


void generate( PLAYER *ch, int ldbkey, int hdbkey, bool fOverwrite )
{
    SCENE *pScene;
    TERRAIN    *pTerrain;
    char buf[MAX_STRING_LENGTH];
    char *map;
    int iScene[MAX_X][MAX_Y];
    int x, y, door;
    int maxX = 0, maxY = 0;
    int dbkey;

    /*
     * Make all dbkeys -1
     */
    for ( y = 0;  y < MAX_Y;  y++ )
    {
        for ( x = 0;  x < MAX_X;  x++ )
        {
            iScene[x][y] = -1;
        }
    }

    /*
     * Find map scene.
     */
    pScene = get_scene( ldbkey );

    if ( pScene == NULL )
    {
        send_to_actor( "No map scene.\n\r", ch );
        return;
    }

    send_to_actor( "[Generating Landscape]\n\r", ch );

    y = 0;
    x = 0;
    dbkey = ldbkey+1;
    for ( map = pScene->description; *map && dbkey <= hdbkey;  map++ )
    {
        /*
         * New line.
         */
        if ( *map == '\n' )
        {
            y++;
            send_to_actor( "\n\r", ch );
            map++;

            if ( *map == '\r' ) map++;

            if ( x > maxX && x < MAX_X )
            maxX = x;
            x = 0;

            /*
             * Map is too long.  (Map is too high)
             */
            if ( y >= MAX_Y )
            {
                maxY = MAX_Y;
                send_to_actor( "\n\r << clipped\n\r", ch );
                break;
            }
        }

        /*
         * End of string.
         */
        if ( *map == '\0' )
        {
            maxY = y;
            break;
        }

        /*
         * Too long of a line.  (Map is too wide)
         */
        if ( x >= MAX_X )
        {
            send_to_actor( "\n\r << wrapped\n\r", ch );
            x = 0;
            y++;
            if ( y >= MAX_Y )
            {
                maxY = MAX_Y;
                send_to_actor( "\n\r << clipped\n\r", ch );
                break;
            }
        }

        /*
         * Find the appropriate terrain data.
         */
        for ( pTerrain = terrain_list;  pTerrain != NULL;  pTerrain = pTerrain->next )
        {
            if ( pTerrain->map_char == *map )
            break;
        }

        if ( pTerrain != NULL )
        {
            snprintf( buf, MAX_STRING_LENGTH, "%c", *map );
            send_to_actor( buf, ch );
        }
        else
        {
            send_to_actor( "?", ch );
            x++;  /* continue moving */
            continue;
        }

        iScene[x][y] = dbkey;

        pScene = get_scene( iScene[x][y] );

        /*
         * Generate a new scene.
         */
        if ( pScene == NULL )
        {
            int iHash;

            pScene                   = new_scene();
            pScene->zone             = get_dbkey_zone( dbkey );
            pScene->dbkey             = dbkey;

            if ( dbkey > top_dbkey_scene ) top_dbkey_scene = dbkey;

            iHash                   = dbkey % MAX_KEY_HASH;
            pScene->next             = scene_hash[iHash];
            scene_hash[iHash]  = pScene;
            pScene->name             = str_dup( pTerrain->name );
        }

        /*
         * Set terrain information.
         * Overwrite if desired.
         */
        if ( fOverwrite || pScene->name == NULL )
        {
            free_string( pScene->name );
            pScene->name             = str_dup( pTerrain->name );
        }

            pScene->terrain = pTerrain->dbkey;
            pScene->move = pTerrain->move;

        dbkey++;
        x++;                   /* <- Yo, MudMCs, this is important */
    }

    /*
     * Connect scenes.
     */
    snprintf( buf, MAX_STRING_LENGTH, "(end)\n\r[Map Size %d, %d]\n\r[Connecting Scenes]\n\r", maxX, maxY );
    send_to_actor( buf, ch );

    for ( y = 0; y < maxY; y++ )
    {
        for ( x = 0; x < maxX; x++ )
        {
            if (iScene[x][y] == -1)
                continue;

            for ( door = 0;  door < MAX_DIR;  door++ )
            {
                int to_dbkey;

                if ( door == DIR_UP
                  || door == DIR_DOWN )
                continue;

                /*
                 * Range Check
                 */
                if ( x + tdir[door][0] < 0
                  || x + tdir[door][0] >= MAX_X
                  || y + tdir[door][1] < 0
                  || y + tdir[door][1] >= MAX_Y )
                continue;

                to_dbkey = iScene[(x + tdir[door][0])][(y + tdir[door][1])];

                if ( to_dbkey == -1 )
                continue;

                pScene = get_scene( iScene[x][y] );

                if ( pScene->exit[door] != NULL )
                {
                    if ( !fOverwrite )
                    continue;

                    free_exit( pScene->exit[door] );
                    pScene->exit[door] = NULL;
                }


               pScene->exit[door] = new_exit();

               pScene->exit[door]->to_scene = get_scene( to_dbkey );
               pScene->exit[door]->dbkey = to_dbkey;
            }
            send_to_actor( "+", ch );
        }
        send_to_actor( "\n\r", ch );
    }

    send_to_actor( "[Generation complete]\n\r", ch );
    return;
};
