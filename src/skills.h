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
 * Skill group defines.
 */

#define GRO_CRAFT                         10
#define GRO_LANGUAGE                          11
#define GRO_LEGERDEMAIN                       12
#define GRO_OFFENSE                           13
#define GRO_DEFENSE                           14
#define GRO_SURVIVAL                          15
#define GRO_WP                                16
#define GRO_FIRE_BASIC                       501
#define GRO_WATER_BASIC                      502
#define GRO_AIR_BASIC                        503
#define GRO_EARTH_BASIC                      504
#define GRO_FIRE_ADV                         601
#define GRO_WATER_ADV                        602
#define GRO_AIR_ADV                          603
#define GRO_EARTH_ADV                        604
#define GRO_FIRE_EXP                         701
#define GRO_WATER_EXP                        702
#define GRO_AIR_EXP                          703
#define GRO_EARTH_EXP                        704
#define GRO_SPC_GENERAL                      901
#define GRO_NECROMANTICS                     902
#define GRO_CONJURATIONS                     903
#define GRO_MALADICTIONS                     904
#define GRO_ILLUSIONS                        905
#define GRO_DIVINATIONS                      906
#define GRO_ENCHANTMENTS                     907
#define GRO_SUMMONINGS                       908
#define GRO_NONE                               0

/*
 * never used
 */
#define ADD_SKILL(ch,skill)  ( skill->next= ch->learned; ch->learned=skill )
