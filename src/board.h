/****************************************************************************
******************************************************************************
* Locke's   __ -based on merc v2.2-____        NIM Server Software           *
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
 *   | |\\| |  | |  | |  | |         the discontinued package, Merc 2.2.    *
 *   | | \  |  | |  | |  | |         NiMUD is being written and developed   *
 *  |___||___||___||__|  |__|        By Locke and Surreality as a new,      *
 *   NAMELESS INCARNATE *MUD*        frequently updated package similar to  *
 *        S O F T W A R E            the original Merc 2.x.                 *
 *                                                                          *
 *  Just look for the Iron Maiden skull wherever NiMUD products are found.  *
 *                                                                          *
 *  Much time and thought has gone into this software and you are           *
 *  benefitting.  We hope that you share your changes too.  What goes       *
 *  around, comes around.                                                   *
 ****************************************************************************/
 
/*
 * I could have wrote this as a dynamic entity, but I'm lazy.
 */
 
#define MAX_BOARD       42

/* currently unused */ 
#if defined(unix)
#define BOARD_PATH      "boards/"
#else
#define BOARD_PATH      ""
#endif
 
