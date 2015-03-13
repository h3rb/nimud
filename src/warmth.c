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


/*
 * Returns the temperature of the
 * scene the character is in.
 */
int scene_temp( PLAYER *ch, SCENE *pScene ) {
    EXIT *pExit;
        int direction = 0;
        int scene_temp = 0;
        int temp_adjust = 0;
        int temp = weather.temperature;

        if ( NPC( ch ) || !ch->in_scene )
                return 0;

        /*
         * If I am not mistaken this doesn't work as of yet.
         * I am rather lazy so I dont know if I will get this
         * part working ever.  What it is SUPPOSE to do is if
         * the current scene your in has exits that lead to the
         * outside and they don't have doors or the doors are
         * open then the scene temperature drops down.
         * Also I should add a check to see if the exit is a
         * window and if the window is open I just keep forgetting
         * about that.  
         *                                               -- Daurven
         *
         * I think I added this functionality to the main editor. -L
         */
    for( direction = 0;  direction < MAX_DIR; direction++ ) {
        pExit = ch->in_scene->exit[direction];

                if ( IS_SET( ch->in_scene->scene_flags, SCENE_INDOORS )
                && ch->in_scene->scene_temp != 0 ) {
                        if ( pExit != NULL && !IS_SET( pExit->exit_flags, EXIT_CLOSED )
                        && !IS_SET( pExit->to_scene->scene_flags, SCENE_INDOORS ) )
                                scene_temp = temp / 2 - ch->in_scene->scene_temp;
                        else
                                scene_temp = ch->in_scene->scene_temp;
                }
        }

        switch (ch->in_scene->move) {
        default:
                temp_adjust = 0;
                break;
        case MOVE_INSIDE:
                case MOVE_CITY:
                temp_adjust = 0;
                break;
        case MOVE_DESERT:
                temp_adjust = 10;
                break;
        case MOVE_ICELAND:
                temp_adjust = -30;
                break;
        case MOVE_MOUNTAIN:
                temp_adjust = -15;
                break;
        case MOVE_HILLS:
                case MOVE_FIELD:
                temp_adjust = -5;
                break;
                case MOVE_FOREST:
                        temp_adjust = -10;
                        break;
        case MOVE_AIR:
                temp_adjust = -3;
                break;
    }

        scene_temp += temp_adjust;
        temp += scene_temp;
        return temp;
}
