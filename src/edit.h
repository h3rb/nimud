/*
******************************************************************************
* Locke's   __ -based on merc v2.2-____        NIM Server Software           *
* ___ ___  (__)__    __ __   __ ___| G| v4.0   Version 4.0 GOLD EDITION      *
* |  /   \  __|  \__/  |  | |  |     O|        documentation release         *
* |       ||  |        |  \_|  | ()  L|        Hallow's Eve 1999             *
* |    |  ||  |  |__|  |       |     D|                                      *
* |____|__|___|__|  |__|\___/__|______|        http://www.nimud.org/nimud    *
*   n a m e l e s s i n c a r n a t e          dedicated to chris cool       *
******************************************************************************
 */

COMMAND( cmd_zedit         );
COMMAND( cmd_redit         );
COMMAND( cmd_oedit         );
COMMAND( cmd_aedit         );
COMMAND( cmd_sedit         );
COMMAND( cmd_hedit         );

extern  ACTOR_INDEX_DATA *        actor_index_hash  [MAX_KEY_HASH];
extern  PROP_INDEX_DATA *        prop_index_hash  [MAX_KEY_HASH];
extern  SCENE_INDEX_DATA *       scene_index_hash [MAX_KEY_HASH];


extern  SHOP_DATA *             shop_first;
extern  SHOP_DATA *             shop_last;

#define         ZONE_NONE       0
#define         ZONE_CHANGED    1
#define         ZONE_ADDED      2
#define         ZONE_STATIC     4

#define         SEX_NONE        4

#define         SCENE_NONE       0

#define         EX_NONE         0

#define         ITEM_NONE      -1

#define         EXTRA_NONE      0

#define         ITEM_WEAR_NONE  0

#define         ACT_NONE        0

#define         AFFECT_NONE     0


#define SKILLSN(sn) (sn != -1 ? skill_table[sn].slot : 0)

/* optional redefine for builder allowing npcs to edit areas 
   not recommended */
/*
#define IS_BUILDER(ch, zone)  ( GET_PC(ch,security,9) <= zone->security     \
                             || strstr( zone->builders, ch->name ) != NULL  \
                             || strstr( zone->builders, "All" ) != NULL    \
                             || (IS_NPC(ch) && ch->pIndexData->zone == zone) )
*/

#define IS_BUILDER(ch, zone)  ( GET_PC(ch,security,9) <= zone->security     \
                             || strstr( zone->builders, ch->name ) != NULL  \
                             || strstr( zone->builders, "All" ) != NULL    )
