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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <zlib.h>

#include "nimud.h"
#include "skills.h"
#include "shop.h"
#include "defaults.h"


/*
 * Goods list.
 */

const     struct   goods_type    goods_table    [] =
{  /* Name                      Unit           Code      Cost  Wght  Life */
    /* Raw Grain */
    { "barley",                 "sack",      T_CORN,   100,    2,   -1  },
    { "buckwheat",              "sack",      T_CORN,    50,    2,   -1  },
    { "chick peas",             "sack",      T_CORN,   300,    2,   -1  },
    { "lentils",                "sack",      T_CORN,   200,    2,   -1  },
    { "millet",                 "sack",      T_CORN,    30,    2,   -1  },
    { "oats",                   "bushel",     T_CORN,    70,    2,   -1  },
    { "rice",                   "bushel",     T_CORN,   500,    2,   -1  },
    { "rye",                    "bushel",     T_CORN,    70,    2,   -1  },
    { "wheat berries",          "bushel",     T_CORN,   200,    2,   -1  },

    /* Flour */
    { "floured barley",         "sack",        T_FLOUR,  200,    5,   -1  },
    { "buckwheat flour",        "sack",        T_FLOUR,  100,    5,   -1  },
    { "floured rye",            "sack",        T_FLOUR,  150,    5,   -1  },
    { "wheat flour",            "sack",        T_FLOUR,  300,    5,   -1  },

    /* Fabrics */
    { "canvas",                 "bolt",        T_FABRIC,  30,    4,   -1  },
    { "cotton",                 "bolt",        T_FABRIC,  80,    4,   -1  },
    { "wool flannel",           "bolt",        T_FABRIC,  24,    4,   -1  },
    { "homespun",               "bolt",        T_FABRIC,  15,    4,   -1  },
    { "lace",                   "tenspan",     T_FABRIC, 500,    1,   -1  },
    { "linen",                  "bolt",        T_FABRIC,  80,    4,   -1  },
    { "raw wool",               "stone",       T_FABRIC,   2,    1,   -1  },
    { "sailcloth",              "bolt",        T_FABRIC,  10,    1,   -1  },
    { "silk",                   "bolt",        T_FABRIC, 700,    1,   -1  },
    { "mattress ticking",       "tenspan",     T_FABRIC, 100,    1,   -1  },
    { "pillow ticking",         "tenspan",     T_FABRIC,  80,    1,   -1  },
    { "velvet",                 "bolt",        T_FABRIC, 300,    1,   -1  },
    { "wool",                   "bolt",        T_FABRIC, 100,    4,   -1  },

    /* Furs */
    { "bear",                   "fur",         T_HIDE,   500,   12,   -1  },
    { "deer",                   "hide",        T_HIDE,   400,   10,   -1  },
    { "red fox",                "hide",        T_HIDE,  1400,    2,   -1  },
    { "white fox",              "hide",        T_HIDE,  1700,    2,   -1  },
    { "horse",                  "hide",        T_HIDE,   300,   13,   -1  },
    { "jaguar",                 "hide",        T_HIDE,  2500,    6,   -1  },
    { "leather",                "hide",        T_HIDE,   200,    6,   -1  },
    { "leopard",                "hide",        T_HIDE,  2200,    6,   -1  },
    { "lion",                   "hide",        T_HIDE,  2400,    7,   -1  },
    { "mink",                   "fur",         T_HIDE,  1200,    3,   -1  },
    { "panther",                "hide",        T_HIDE,  1500,    6,   -1  },
    { "raccoon",                "fur",         T_HIDE,   200,    2,   -1  },
    { "shark",                  "skin",        T_HIDE,  1500,    7,   -1  },
    { "sheep",                  "hide",        T_HIDE,   600,    4,   -1  },
    { "tiger",                  "hide",        T_HIDE,  2500,    7,   -1  },
    { "wolf",                   "hide",        T_HIDE,   800,    4,   -1  },

    /* Dyes */
    { "copper blue",            "vial",        T_DYE,    330,    1,   -1  },
    { "ultramarine blue",       "vial",        T_DYE,    100,    1,   -1  },
    { "mollusk green",          "vial",        T_DYE,     90,    1,   -1  },
    { "indigo",                 "vial",        T_DYE,    300,    1,   -1  },
    { "ocher",                  "vial",        T_DYE,    120,    1,   -1  },
    { "purple",                 "vial",        T_DYE,    330,    1,   -1  },
    { "cochineal red",          "vial",        T_DYE,     40,    1,   -1  },
    { "iron red",               "vial",        T_DYE,     50,    1,   -1  },
    { "mollusk red",            "vial",        T_DYE,     50,    1,   -1  },
    { "safflower dye",          "vial",        T_DYE,     40,    1,   -1  },
    { "sulfer yellow",          "vial",        T_DYE,     30,    1,   -1  },
    { "vermilion dye",          "vial",        T_DYE,     44,    1,   -1  },

    /* Refined metals */
    { "refined brass",          "bar",         T_METAL,   10,    2,   -1  },
    { "refined bronze",         "bar",         T_METAL,    7,    2,   -1  },
    { "refined copper",         "bar",         T_METAL,    6,    2,   -1  },
    { "refined gold",           "bar",         T_METAL, 1100,    2,   -1  },
    { "refined iron",           "bar",         T_METAL,    5,    2,   -1  },
    { "refined lead",           "bar",         T_METAL,    7,    2,   -1  },
    { "refined platinum",       "half bar",    T_METAL, 2000,    1,   -1  },
    { "refined silver",         "bar",         T_METAL,  300,    2,   -1  },
    { "refined tin",            "bar",         T_METAL,   20,    2,   -1  },

    /* Raw metals */
    { "brass ore",              "nugget",      T_ORE,      2,    1,   -1  },
    { "bronze ore",             "nugget",      T_ORE,      3,    1,   -1  },
    { "copper ore",             "nugget",      T_ORE,      2,    1,   -1  },
    { "gold ore",               "nugget",      T_ORE,    350,    1,   -1  },
    { "iron ore",               "nugget",      T_ORE,      1,    1,   -1  },
    { "lead ore",               "nugget",      T_ORE,      3,    1,   -1  },
    { "platinum ore",           "nugget",      T_ORE,    700,    1,   -1  },
    { "silver ore",             "nugget",      T_ORE,    100,    1,   -1  },
    { "tin ore",                "nugget",      T_ORE,      7,    1,   -1  },

    /* Seasonings */
    {  "angelica",              "jar",         T_SPICE,    3,    1,   -1  },
    {  "anise",                 "jar",         T_SPICE,    8,    1,   -1  },
    {  "basil",                 "jar",         T_SPICE,    2,    1,   -1  },
    {  "bergamot",              "jar",         T_SPICE,    5,    1,   -1  },
    {  "borage",                "jar",         T_SPICE,    5,    1,   -1  },
    {  "calendula",             "jar",         T_SPICE,    4,    1,   -1  },
    {  "caraway",               "jar",         T_SPICE,    2,    1,   -1  },
    {  "chervil",               "jar",         T_SPICE,    4,    1,   -1  },
    {  "chives",                "jar",         T_SPICE,    2,    1,   -1  },
    {  "clary",                 "jar",         T_SPICE,    6,    1,   -1  },
    {  "coriander",             "jar",         T_SPICE,    6,    1,   -1  },
    {  "costmary",              "jar",         T_SPICE,    5,    1,   -1  },
    {  "cumin",                 "jar",         T_SPICE,    7,    1,   -1  },
    {  "dillweed",              "jar",         T_SPICE,    3,    1,   -1  },
    {  "sweet herbs",           "fagot",       T_SPICE,   10,    1,   -1  },
    {  "fennel seed",           "jar",         T_SPICE,    2,    1,   -1  },
    {  "fenugreek",             "jar",         T_SPICE,    3,    1,   -1  },
    {  "garlic",                "jar",         T_SPICE,    2,    1,   -1  },
    {  "horehound",             "jar",         T_SPICE,    5,    1,   -1  },
    {  "horseradish",           "jar",         T_SPICE,    6,    1,   -1  },
    {  "hyssop",                "jar",         T_SPICE,    9,    1,   -1  },
    {  "juniper",               "jar",         T_SPICE,    9,    1,   -1  },
    {  "laurel",                "jar",         T_SPICE,    8,    1,   -1  },
    {  "lemon balm",            "jar",         T_SPICE,    4,    1,   -1  },
    {  "liquorice root",        "jar",         T_SPICE,    3,    1,   -1  },
    {  "lovage",                "jar",         T_SPICE,    6,    1,   -1  },
    {  "marigold",              "jar",         T_SPICE,   12,    1,   -1  },
    {  "marjoram",              "jar",         T_SPICE,    6,    1,   -1  },
    {  "mint",                  "jar",         T_SPICE,    7,    1,   -1  },
    {  "mustard seed",          "jar",         T_SPICE,    3,    1,   -1  },
    {  "oregano",               "jar",         T_SPICE,    4,    1,   -1  },
    {  "parsley",               "jar",         T_SPICE,    4,    1,   -1  },
    {  "poppy seed",            "jar",         T_SPICE,    2,    1,   -1  },
    {  "rose hips",             "jar",         T_SPICE,   15,    1,   -1  },
    {  "rosemary",              "jar",         T_SPICE,    5,    1,   -1  },
    {  "sage",                  "jar",         T_SPICE,    6,    1,   -1  },
    {  "salt",                  "jar",         T_SPICE,    1,    1,   -1  },
    {  "winter savory",         "jar",         T_SPICE,    7,    1,   -1  },
    {  "summer savory",         "jar",         T_SPICE,    7,    1,   -1  },
    {  "sweet cicely",          "jar",         T_SPICE,    8,    1,   -1  },
    {  "tarragon",              "jar",         T_SPICE,    7,    1,   -1  },
    {  "thyme",                 "jar",         T_SPICE,    8,    1,   -1  },
    {  "woodruff",              "jar",         T_SPICE,   10,    1,   -1  },

    /* Spices */
    {  "canella",               "sachet",      T_SPICE,    6,    1,   -1  },
    {  "cardamon",              "sachet",      T_SPICE,    6,    1,   -1  },
    {  "cinnamon",              "fagot",       T_SPICE,    5,    1,   -1  },
    {  "cloves",                "sachet",      T_SPICE,    7,    1,   -1  },
    {  "cubeb",                 "sachet",      T_SPICE,    6,    1,   -1  },
    {  "galingale",             "sachet",      T_SPICE,    8,    1,   -1  },
    {  "ginger",                "sachet",      T_SPICE,    3,    1,   -1  },
    {  "mace",                  "sachet",      T_SPICE,    2,    1,   -1  },
    {  "nutmeg",                "sachet",      T_SPICE,    5,    1,   -1  },
    {  "black pepper",          "sachet",      T_SPICE,    7,    1,   -1  },
    {  "red pepper",            "sachet",      T_SPICE,    8,    1,   -1  },
    {  "white pepper",          "sachet",      T_SPICE,    9,    1,   -1  },
    {  "saffron",               "sachet",      T_SPICE,    4,    1,   -1  },
    {  "turmeric",              "sachet",      T_SPICE,    6,    1,   -1  },

    /* Sweeteners */
    {  "rose petal honey",      "jar",         T_SUGAR,   20,    1,   -1  },
    {  "parsley honey",         "jar",         T_SUGAR,   12,    1,   -1  },
    {  "lavander honey",        "jar",         T_SUGAR,   15,    1,   -1  },
    {  "marzipan",              "jar",         T_SUGAR,   30,    1,   -1  },
    {  "molasses",              "jar",         T_SUGAR,   20,    1,   -1  },
    {  "sorghum",               "jar",         T_SUGAR,   10,    1,   -1  },
    {  "brown sugar",           "sack",        T_SUGAR,   10,    3,   -1  },
    {  "lavender sugar",        "sack",        T_SUGAR,   15,    2,   -1  },
    {  "loaf sugar",            "sack",        T_SUGAR,   12,    3,   -1  },
    {  "lemon sugar",           "sack",        T_SUGAR,   15,    2,   -1  },
    {  "mace sugar",            "sack",        T_SUGAR,   14,    2,   -1  },
    {  "orange sugar",          "sack",        T_SUGAR,   18,    2,   -1  },
    {  "powdered sugar",        "sack",        T_SUGAR,   20,    1,   -1  },
    {  "raw sugar",             "sack",        T_SUGAR,    8,    5,   -1  },
    {  "rose sugar",            "sack",        T_SUGAR,   25,    2,   -1  },
    {  "violet sugar",          "sack",        T_SUGAR,   22,    3,   -1  },

    /* Oils */
    {  "almond oil",            "flask",       T_OIL,     30,    1,   -1  },
    {  "hazelnut oil",          "flask",       T_OIL,     30,    1,   -1  },
    {  "herbal oil",            "flask",       T_OIL,     50,    1,   -1  },
    {  "olive oil",             "flask",       T_OIL,     10,    1,   -1  },
    {  "rapeseed oil",          "flask",       T_OIL,     20,    1,   -1  },
    {  "safflower oil",         "flask",       T_OIL,     30,    1,   -1  },
    {  "sesame oil",            "flask",       T_OIL,     50,    1,   -1  },
    {  "sunflower oil",         "flask",       T_OIL,     40,    1,   -1  },
    {  "walnut oil",            "flask",       T_OIL,     50,    1,   -1  },

    { "",                       "",               -1,      0,    0,   -1  },
};

                                                            
PROP *create_good( int good )
{
   PROP *prop;
   char buf[MAX_STRING_LENGTH];
    
   prop = create_prop( get_prop_template(PROP_VNUM_GOODS), 0 );
     
   sprintf( buf, "%s %s", goods_table[good].name,
                          goods_table[good].unit );
   free_string( prop->name );
   prop->name = str_dup( buf );
   
   sprintf( buf, "a %s of %s", goods_table[good].unit, 
                               goods_table[good].name );
   free_string( prop->short_descr );
   prop->short_descr = str_dup( buf );
   
   sprintf( buf, "%s was placed here.\n\r", STR(prop,short_descr) );
   free_string( prop->description );
   prop->description = str_dup( buf );

   sprintf( buf, "Several %ss of %s are stored here.\n\r",
      goods_table[good].unit, goods_table[good].name );
   free_string( prop->description );
   prop->description = str_dup( buf );

   prop->item_type = ITEM_GOODS;
   prop->weight    = goods_table[good].weight;
   prop->cost      = goods_table[good].cost;
   prop->timer     = goods_table[good].life;
   prop->value[0]  = good;

   return prop;
}


PROP *create_comp( int comp )
{
   PROP *prop;
   char buf[MAX_STRING_LENGTH];
    
   prop = create_prop( get_prop_template(PROP_VNUM_COMP), 0 );
     
   sprintf( buf, "%s %s component",
                 smash_article( smash_arg( comp_table[comp].usage, "of" ) ),
                 comp_table[comp].name );
   free_string( prop->name );
   prop->name = string_unpad( str_dup(buf) );
   
   sprintf( buf, "%s %s", comp_table[comp].usage,
                          comp_table[comp].name );
   free_string( prop->short_descr );
   prop->short_descr = str_dup( buf );
   
   sprintf( buf, "%s components",   comp_table[comp].name );
   free_string( prop->short_descr_plural );
   prop->short_descr_plural = str_dup( buf );

   sprintf( buf, "%s was left here.\n\r", STR(prop,short_descr) );
   free_string( prop->description );
   prop->description = str_dup( buf );

   sprintf( buf, "%%s %s was left here.\n\r", smash_article(STR(prop,short_descr)) );
   free_string( prop->description_plural );
   prop->description_plural = str_dup( buf );
   
   prop->item_type = ITEM_COMPONENT;
   prop->weight    = 1;
   prop->cost      = comp_table[comp].cost;
   prop->timer     = -1;
   prop->value[0]  = comp;
   prop->value[1]  = comp_table[comp].type;

   return prop;
}


/*
struct  shop
*/


/*
 * Show the shop list to a character.
 * Can coalesce duplicated items.
 */
void shop_list_to_actor( PLAYER *keeper, PLAYER *ch )
{
    char buf[MAX_STRING_LENGTH];
    char final[MAX_STRING_LENGTH*2];
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    PROP **lastprop;
    PROP *prop;
    int nShow, iShow, vShow;
    int count;
    bool fCombine;
    PROP *list = keeper->carrying;

    if ( ch->desc == NULL )
	return;
    
    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( prop = list; prop != NULL; prop = prop->next_content )
	count++;
    prgpstrShow	= alloc_mem( count * sizeof(char *) );
    prgnShow    = alloc_mem( count * sizeof(int)    );
    lastprop     = alloc_mem( count * sizeof(PROP *) );
    nShow = 0;

    /*
     * Format the list of props.
     */
    for ( prop = list; prop != NULL; prop = prop->next_content )
    {
        if ( prop->wear_loc == WEAR_NONE
          && can_see_prop( ch, prop ) )
        {
            pstrShow = format_prop_to_actor( prop, ch, TRUE );
            fCombine = FALSE;

            /*
             * Look for duplicates, case sensitive.
             * Matches tend to be near end so run loop backwards.
             */
            for ( iShow = nShow - 1; iShow >= 0; iShow-- )
            {
                if ( ( !strcmp( prgpstrShow[iShow], pstrShow )
                     || lastprop[iShow] == prop )
                   && prop->item_type != ITEM_MONEY )
                {
                ++prgnShow[iShow];
                lastprop[iShow] = prop;
                fCombine = TRUE;
                break;
                }
            }

            /*
             * Couldn't combine, or didn't want to.
             */
            if ( !fCombine )
            {
            prgpstrShow [nShow] = str_dup( pstrShow );
            prgnShow    [nShow] = 1;
            lastprop     [nShow] = prop;
            nShow++;
            }
        }
    }

    /*
     * Output the formatted list.
     */
    vShow = 1;
    final[0] = '\0';
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
        if ( lastprop[iShow] == NULL )
        {
            wtf_logf( "Shop_list_to_actor: lastprop == NULL.", 0 );
            continue;
        }

        if ( lastprop[iShow]->item_type == ITEM_MONEY )
        continue;

        if ( prgnShow[iShow] > 1
          && !IS_SET(keeper->pIndexData->pShop->shop_flags, SHOP_TAVERN) )
            sprintf( buf, "#%-2d %s for %s each.\n\r",
                 vShow,
                 capitalize(
                    pluralize( (lastprop[iShow]->item_type == ITEM_DRINK_CON
                             && lastprop[iShow]->value[0] > 0)
                          ? format_prop_to_actor(lastprop[iShow],ch,TRUE)
                          : STR(lastprop[iShow],short_descr)
                             )
                           ),
                 name_amount( get_cost( keeper, lastprop[iShow], TRUE ) )
                  );
/*
            sprintf( buf, "*%-2d  %s %s for %s each.\n\r",
                          iShow+1,
                          capitalize(numberize(prgnShow[iShow])),
                          pluralize( ( lastprop[iShow]->item_type == ITEM_DRINK_CON
                                    && lastprop[iShow]->value[0] > 0 )
                                       ? format_prop_to_actor(lastprop[iShow],ch,TRUE)
                                       : STR(lastprop[iShow],short_descr)),
                          name_amount( get_cost( keeper, lastprop[iShow], TRUE ) ) );
 */
        else
        sprintf( buf, "#%-2d %s for %s.\n\r",
                 vShow,
                 capitalize( format_prop_to_actor(lastprop[iShow], ch, TRUE) ),
                 name_amount( get_cost( keeper, lastprop[iShow], TRUE ) ) );

        strcat( final, buf );
        free_string( prgpstrShow[iShow] );
        prgpstrShow[iShow] = NULL;
        lastprop[iShow] = NULL;
        vShow += prgnShow[iShow];
    }

    if ( nShow == 0 )
    strcat( final, "Nothing.\n\r" );

    page_to_actor( final, ch );

    /*
     * Clean up.
     */
    free_mem( prgpstrShow, count * sizeof(char *)      );
    free_mem( prgnShow,    count * sizeof(int)         );
    free_mem( lastprop,     count * sizeof(PROP *)  );

    return;
}


#if defined(never)
/*
 * Show the shop list to a character.
 * Can coalesce duplicated items.
 */
PROP *get_shop_list_prop( PLAYER *keeper, PLAYER *ch, int number )
{
    PROP **lastprop;
    PROP *prop;
    int nShow;
    int iShow;
    int count;
    bool fCombine;
    PROP *list = keeper->carrying;

    if ( ch->desc == NULL )
    return NULL;
    
    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( prop = list; prop != NULL; prop = prop->next_content )
	count++;
    lastprop     = alloc_mem( count * sizeof(PROP *) );
    nShow	= 0;

    /*
     * Format the list of props.
     */
    for ( prop = list; prop != NULL; prop = prop->next_content )
    {
        if ( prop->wear_loc == WEAR_NONE && can_see_prop( ch, prop ) )
        {
            fCombine = FALSE;

            /*
             * Look for duplicates, case sensitive.
             * Matches tend to be near end so run loop backwards.
             */
            for ( iShow = nShow - 1; iShow >= 0; iShow-- )
            {
                if ( lastprop[iShow] == prop && prop->item_type != ITEM_MONEY )
                {
                lastprop[iShow] = prop;
                fCombine = TRUE;
                break;
                }
            }

            /*
             * Couldn't combine, or didn't want to.
             */
            if ( !fCombine )
            {
            lastprop     [nShow] = prop;
            nShow++;
            }
        }
    }

    /*
     * Output the formatted list.
     */
    prop = NULL;
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
        if ( lastprop[iShow] == NULL )
        {
            wtf_logf( "Shop_list_to_actor: lastprop == NULL.", 0 );
            continue;
        }

        if ( lastprop[iShow]->item_type == ITEM_MONEY )
        continue;

        if ( iShow == number-1 )
        prop = lastprop[iShow];

        lastprop[iShow] = NULL;
    }

    /*
     * Clean up.
     */
    free_mem( lastprop,     count * sizeof(PROP *)  );

    return prop;
}
#endif


PROP *get_shop_list_prop( PLAYER *keeper, PLAYER *ch, int number )
{
    PROP *prop;
    int count;
    PROP *list = keeper->carrying;

    if ( ch->desc == NULL )
    return NULL;
    
    count = 0;
    for ( prop = list; prop != NULL; prop = prop->next_content )
    {
        count++;

        if ( count == number
          && prop->wear_loc == WEAR_NONE
          && can_see_prop( ch, prop ) )
            break;
    }

    return prop;
}



PLAYER *find_keeper( PLAYER *ch, char *arg, bool Report )
{
    PLAYER *keeper = NULL;
    SHOP *pShop = NULL;

    if ( *arg != 0 )
    {
        if ( (keeper = get_actor_scene( ch, arg )) != NULL
          && (pShop = keeper->pIndexData->pShop) == NULL )
        {
            if ( Report ) to_actor( "They aren't here.\n\r", ch );
            return NULL;
        }
    }

    if ( pShop == NULL )
    {
    for ( keeper = ch->in_scene->people; keeper; keeper = keeper->next_in_scene )
    {
	if ( NPC(keeper) && (pShop = keeper->pIndexData->pShop) != NULL )
	    break;
    }
    }

    if ( pShop == NULL )
    {
        if ( Report ) to_actor( "There is no one to serve you.\n\r", ch );
	return NULL;
    }

    /*
     * Shop hours.
     */
    if ( weather.hour == 12 && number_range(0,2) == 1 )
    {
        cmd_smote( keeper, "#grumble I'm on my lunch break, leave me be!" );
        return NULL;
    }

    if ( weather.hour < pShop->open_hour || weather.hour > pShop->close_hour )
    {
        char buf[MAX_STRING_LENGTH];
        char a[MAX_INPUT_LENGTH];
        char b[MAX_INPUT_LENGTH];
      
	sprintf( a, "%d %s", pShop->open_hour > 12 ? pShop->open_hour-12 : 
pShop->open_hour,  pShop->open_hour < 12 ? "in the morning" : 
"in the evening" );
	sprintf( b, "%d %s", pShop->close_hour > 12 ? pShop->close_hour-12 : 
pShop->close_hour,  pShop->close_hour < 12 ? "in the morning" : 
"in the evening" );
        sprintf( buf, pShop->hours_excuse, a, b );
	if ( Report ) cmd_smote( keeper, buf );
	return NULL;
    }

    /*
     * Invisible or hidden people.
     */
    if ( !can_see( keeper, ch ) )
    {
    if ( Report ) cmd_smote( keeper, "What?  Who said that?" );
	return NULL;
    }

    /*
     * Undesirables.
     */
    if ( ch->bounty > 10000 )
    {
        if ( Report ) cmd_smote( keeper, "I do not service criminals." );
        return NULL;
    }

    return keeper;
}



/*
 * Careful of the recursion.
 */
int get_cost( PLAYER *keeper, PROP *prop, bool fBuy )
{
    SHOP *pShop;
    int cost;

    if ( prop == NULL || ( pShop = keeper->pIndexData->pShop ) == NULL )
	return 0;

    if ( fBuy )
    {
        PROP *pProp;
        
        cost = prop->cost;

/*        if ( prop->item_type == ITEM_GOODS )
        {
            if ( has_arg( pShop->buy_list, goods_table[prop->value[0]].name ) )
                cost /= 2;
            if ( has_arg( pShop->sell_list, goods_table[prop->value[0]].name ) )
                cost += (prop->cost*15)/100;
        }*/

        for ( pProp = prop->contains; pProp != NULL; pProp = pProp->next_content )
          cost += get_cost( keeper, pProp, fBuy );
          
        cost = (cost * pShop->profit_buy) / 100;
    }
    else
    {
        PROP *prop2;
        int itype;

        cost = 0;
        for ( itype = 0; itype < MAX_TRADE; itype++ )
        {
            if ( prop->item_type == pShop->buy_type[itype] )
            {
            cost = (prop->cost * pShop->profit_sell) / 100;
            break;
            }
        }

        for ( prop2 = keeper->carrying; prop2; prop2 = prop2->next_content )
        {
            if ( prop->pIndexData == prop2->pIndexData )
            cost -= (cost * 15)/100;
        }

/*        if ( prop->item_type == ITEM_GOODS )
        {
            if ( has_arg( pShop->buy_list, goods_table[prop->value[0]].name ) )
                cost += (prop->cost*15)/100;
            if ( has_arg( pShop->sell_list, goods_table[prop->value[0]].name ) )
                cost /= 2;
        } */
    }

    /*
     * Usage adjustments.
     */
    if ( prop->item_type == ITEM_STAFF || prop->item_type == ITEM_WAND )
	cost = cost * prop->value[2] / prop->value[1];

    return cost;
}



/*
 * Syntax: repair
 *         repair [prop]
 *         repair appraise [prop]
 *         repair appraisal [prop]
 */
void cmd_repair( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    PLAYER *keeper;
    SHOP *pShop;
    PROP *pProp;
    char *p;
    int cost;
    bool AppraiseOnly = FALSE;

    if ( ch->position == POS_FIGHTING ) {
            to_actor( "Not while fighting.\n\r", ch );
            return;
       }
    argument = one_argument( argument, arg );

    if ( !str_prefix( arg, "appraise" ) || !str_prefix(arg,"appraisal") )
    {
         AppraiseOnly = TRUE;
         argument = one_argument( argument, arg );
    }

    pProp = get_prop_here( ch, arg );
    if ( pProp != NULL && pProp->item_type != ITEM_ARMOR ) {
       to_actor( "That is not a piece of armor.\n\r", ch );
       return;
    }

    if ( pProp == NULL ) { to_actor( "Repair what?\n\r", ch); 
            return; }


        if ( pProp->value[1] == pProp->value[2] ) {
             to_actor( "It's in perfect shape.\n\r", ch );
             return;
        }


    /*
     * Repair shop?
     */
    if ( (keeper = find_keeper( ch, arg, FALSE )) == NULL )
    {
        /*
         * No? Use skill.
         */

        SKILL *pSkill = skill_lookup( "repair" );
        WAIT_STATE( ch, pSkill ? pSkill->delay : 0 );

        if ( skill_check( ch, find_skill_pc( ch, pSkill->dbkey ), 50 ) )
        {
             advance_skill( ch, find_skill_pc( ch, pSkill->dbkey ), 1, 0 );
             pProp->value[1] = pProp->value[1] + 10;
             if ( pProp->value[1] > pProp->value[2] ) {
                  pProp->value[1] = pProp->value[2];
                  act( "$n repairs $p.", ch, pProp, NULL, TO_SCENE );
             }

             to_actor( "You perform some repairs on ", ch );
             to_actor( STR(pProp,short_descr), ch );
             to_actor( ".\n\r", ch );
        }            
        else to_actor( "You don't know how to make repairs.\n\r", ch );
        return;
    }

    if ( (pShop = keeper->pIndexData->pShop) == NULL )
    {
        wtf_logf( "Find_keeper: returned a NULL pShop.", 0 );
        return;
    }

    /*
     * Repair at cost.
     */
    if ( IS_SET(pShop->shop_flags, SHOP_REPAIR) )
    {   

    if ( pProp == NULL )
    {
        cmd_smote( keeper, "Repair what?" );
        return;
    }

    cost = pShop->repair_rate * ( pProp->value[2] - pProp->value[1] );
    /*
     * Cap cost at item cost * 2/3.
     */
    if ( cost > pProp->cost ) cost = (pProp->cost);

    if ( AppraiseOnly ) {
        sprintf( buf, "I will repair %s for %s.", STR(pProp,short_descr), 
                 name_amount( cost ) );
        cmd_smote( keeper, buf );
        return;
    }

    if ( tally_coins( ch ) < cost )
    {
        int diff=cost-tally_coins(ch);
        sprintf( buf, "#remark %s, you are short %s.", NAME(ch), name_amount( diff ) );
        cmd_smote( keeper, buf );
        return;
    }

    pProp->value[1] = pProp->value[2];
    p = sub_coins(cost, ch );

    sprintf( buf, "%s repairs %s for %s.\n\r", capitalize(NAME(keeper)), 
             STR(pProp,short_descr), name_amount( cost ) );
    to_actor( buf, ch );

    sprintf( buf, "You pay %s and receive %s in change.\n\r", 
             NAME(keeper), p );

    to_actor( buf, ch );
    act( "$N repairs $p for $n.", ch, pProp, keeper, TO_SCENE );
   
    }

    return;
}

/*
 * Syntax:  list
 *          list [person]
 */
void cmd_list( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    PLAYER *keeper;
    SHOP *pShop;

    argument = one_argument( argument, arg );

    if ( (keeper = find_keeper( ch, arg, TRUE )) == NULL )
    return;

    if ( (pShop = keeper->pIndexData->pShop) == NULL )
    {
        wtf_logf( "Find_keeper: returned a NULL pShop.", 0 );
        return;
    }

    if ( IS_SET(pShop->shop_flags, SHOP_TRADER) )
    {
        int t,c=0;

        buf[0] = '\0';
        for ( t = 0;  t<MAX_TRADE;  t++ )
        {
            if ( pShop->trades[t] )
            {   c++;
                sprintf( buf2, "%s%s", c==1 ? "" : ", ",  capitalize(name_good_code(t)) );
                strcat( buf, buf2 );
            }
        }

        if ( buf[0] != '\0' )
        {
            sprintf( buf2, "%s is looking to buy: ", capitalize(NAME(keeper)) );
            display_interp( ch, "^B" );
            page_to_actor( buf2, ch );
            display_interp( ch, "^N" );
            page_to_actor( buf, ch );
            page_to_actor( "\n\r", ch );
        }
    }

    if ( IS_SET(pShop->shop_flags, SHOP_PEDDLER) )
    {
        sprintf( buf, "%s is selling:\n\r", capitalize(NAME(keeper)) );
        display_interp( ch, "^B" );
        page_to_actor( buf, ch );
        display_interp( ch, "^N" );
        shop_list_to_actor( keeper, ch );
    }

    if ( IS_SET(pShop->shop_flags, SHOP_COMPONENTS) )
    {
        if ( buf[0] != '\0' )
        {
        sprintf( buf2, "%s will buy spell components.\n\r", capitalize(NAME(keeper)) );
        display_interp( ch, "^B" );
        page_to_actor( buf2, ch );
        display_interp( ch, "^N" );
        }
    }

    if ( IS_SET(pShop->shop_flags, SHOP_REPAIR) ) {
        display_interp( ch, "^BRepair services are offered.^N\n\r" );
    }

    return;
}





bool transact( PLAYER *keeper, PROP *prop, PLAYER *ch, int price )
{
    char buf[MAX_STRING_LENGTH];
    char *p;

    if ( prop == NULL )
    {
        wtf_logf( "Transact: NULL prop.", 0 );
        return FALSE;
    }

    if ( tally_coins( ch ) < price )
    {
        int diff=price-tally_coins(ch);
        sprintf( buf, "You are short %s.\n\r", name_amount( diff ) );
        to_actor( buf, ch );
        return FALSE;
    }

/*
    if ( hand_empty( ch ) == WEAR_NONE )
    {
        to_actor( "You tuck it away for a moment.\n\r", ch );
        return FALSE;
    }
 */

    if ( prop->carried_by != NULL )
    prop_from_actor( prop );

    REMOVE_BIT(prop->extra_flags, ITEM_INVENTORY);
    prop_to_actor( prop, ch );
    prop->wear_loc = hand_empty( ch );

    p = sub_coins(price, ch );

    if ( !str_cmp( p, "nothing" ) )
    sprintf( buf, "You buy %s.\n\r", STR(prop,short_descr) );
    else
    sprintf( buf, "You buy %s and receive %s in change.\n\r", STR(prop,short_descr), p );

    to_actor( buf, ch );
    act( "$n buys $p from $N.", ch, prop, keeper, TO_SCENE );
    return TRUE;
}



void buy_ai( PLAYER *ch, PLAYER *keeper, int offer )
{
/*    char buf[MAX_STRING_LENGTH];
    SHOP *pShop = keeper->pShop;
    PROP *prop;
    int i;

    if ( (prop = ch->haggling) == NULL )
    {
        to_actor( "You are not trading for anything.\n\r", ch );
        return;
    }

    if ( pShop == NULL )
    {
        wtf_logf( "Buy_ai: No shop on keeper.", 0 );
        return;
    }

    if ( ch->keeper != keeper )
    {
        to_actor( "You are trading with someone else.\n\r", ch );
        return;
    }

    if ( ch->in_scene != keeper->in_scene )
    {
        to_actor( "You are not in the same scene with the trader.\n\r", ch );
        return;
    }

    if ( offer < 0 )
    {
        ch->original_cost = ch->haggled_cost;
        sprintf( buf, "I can offer you %s for %s copper.",
                 STR(prop,short_descr), numberize(ch->haggling_cost) );
        say_to( keeper, ch, buf );
        return;
    }

    i = (ch->original_cost + pShop->max_flux)/100;
    if ( offer > i/2 )
    {
        if ( number_bits( 2 ) == 0 )
        SET_BIT( ch->haggle_bits, HAGGLE_ANGRY );

        if ( IS_SET(ch->haggle_bits, HAGGLE_ANGRY )
        sprintf( buf, "Your offer is outrageous

  */
}

/*
 * Syntax:  buy [number] [keeper]
 *          buy [component] [keeper]
 *          buy [prop] [keeper]
 */
void cmd_buy( PLAYER *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    PROP *prop;
    PLAYER *keeper;
    int t, number;


    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( (keeper = find_keeper( ch, arg2, TRUE )) == NULL )
    return;

    if ( is_number(arg1) )
    {
        prop = get_shop_list_prop( keeper, ch, atoi(arg1) );

        if ( prop == NULL )
        {
            sprintf( buf, keeper->pIndexData->pShop->no_such_item, NAME(ch) );
            say_to( keeper, ch, buf );
        }
        else
        transact( keeper, prop, ch, get_cost( keeper, prop, TRUE ) );
        return;
    }

    for ( t = 0;  comp_table[t].type != -1; t++ )
    {
        if ( !str_cmp( comp_table[t].name, arg1 ) )
        break;
    }

    if ( comp_table[t].type != -1 )
    {
        prop = create_comp(t);

        transact( keeper, prop, ch, get_cost( keeper, prop, TRUE ) );
        return;
    }

    number = number_argument( arg1, arg2 );
    t  = 0;
    for ( prop = keeper->carrying; prop != NULL; prop = prop->next_content )
    {
        if ( can_see_prop( ch, prop )
          && is_name( arg1, STR(prop, name) )
          && prop->wear_loc == WEAR_NONE
          && ++t == number )
            break;
    }

    if ( prop == NULL )
    {
        sprintf( buf, keeper->pIndexData->pShop->no_such_item, NAME(ch) );
        say_to( keeper, ch, buf );
    }
    else
    transact( keeper, prop, ch, get_cost( keeper, prop, TRUE ) );

    return;
}




/*
 * Syntax:  trade [item] [keeper]
 */
void cmd_trade( PLAYER *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
/*    char buf[MAX_STRING_LENGTH]; */
    PLAYER *keeper;
    SHOP *pShop;
    int t;

    return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( (keeper = find_keeper( ch, arg2, TRUE )) == NULL )
    return;

    pShop = keeper->pIndexData->pShop;

    for ( t = 0;  goods_table[t].code != -1; t++ )
    {
        if ( !str_cmp( goods_table[t].name, arg1 ) )
        break;
    }

/*    if ( goods_table[t].code == -1
      && pShop->trade_sell[goods_table[t].code] != 0 )
    {
        sprintf( buf, keeper->pIndexData->pShop->no_such_item, NAME(ch) );
        say_to( keeper, ch, buf );
    }*/

    if ( ch->haggling != NULL )
    {
        if ( ch->haggling->carried_by == NULL )
        extractor_prop( ch->haggling );

        ch->haggling = NULL;
        to_actor( "You stop haggling for the current item.\n\r", ch );
    }

    ch->keeper = keeper;
    ch->haggling = create_good(t);

/*    ch->haggled_cost = (get_cost(keeper, ch->haggling, TRUE)
                       * pShop->trade_sell[goods_table[t].code])/100; */

    ch->haggle_bits  =  HAGGLE_BUY;

    buy_ai( ch, keeper, -1 );
    return;
}




void cmd_sell( PLAYER *ch, char *argument ) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char tells[MAX_INPUT_LENGTH];
    PLAYER *keeper;
    PROP *prop;
    SHOP *pShop;
    int cost, x, willBuy = FALSE;

    one_argument( argument, arg );

    if ( arg[0] == '\0' ) {
        to_actor( "Sell what?\n\r", ch );
        return;
    }

    if ( ( keeper = find_keeper( ch, arg, TRUE ) ) == NULL )
        return;

    pShop = keeper->pIndexData->pShop;

    if ( ( prop = get_prop_carry( ch, arg ) ) == NULL ) {
        sprintf( tells, "#grumble %s, you aren't holding that item!", STR(ch,name) );
        cmd_smote( keeper, tells );
        return;
    }

    if ( !can_drop_prop( ch, prop ) ) {
        to_actor( "It is impossible to let go of it.\n\r", ch );
        return;
    }

    if ( ( cost = get_cost( keeper, prop, TRUE ) ) <= 0 ) {
        act( "$n looks at $p and shrugs.", keeper, prop, ch, TO_VICT );
        return;
    }

    for ( x = 0; x < MAX_TRADE; x++ )
    {
        if ( pShop->buy_type[x] == prop->item_type ) {
           willBuy = TRUE;
           break;
        }
        if ( prop->item_type == ITEM_GOODS
          && pShop->trades[x] > 0 
          && goods_table[URANGE(0,prop->value[0],MAX_GOODS-1)].code == x ) {
           willBuy = TRUE;
           break;
        }
    }

    if ( !willBuy )
    {
	cmd_smote( keeper, pShop->cmd_not_buy );
        return;
    }

    act( "$n sells $p to $N.", ch, prop, keeper, TO_SCENE );
    sprintf( buf, "You sell $p for %s to $N.", name_amount( cost ) );
    act( buf, ch, prop, keeper, TO_ACTOR );
    create_amount( cost, ch, NULL, NULL );

    if ( prop->item_type == ITEM_TRASH ) {
                extractor_prop( prop );
    } else {
        prop_from_actor( prop );
        prop_to_actor( prop, keeper );
    }
    return;
}



/*
 * Syntax:  barter [number] [keeper]
 */
void cmd_barter( PLAYER *ch, char *argument )
{
    return;
}




/*
 * Syntax:  offer [price]
 *          offer end
 *          offer abort
 *          offer agree [quantity]
 *          offer accept [quantity]
 */
void cmd_offer( PLAYER *ch, char *argument )
{
    return;
}



void cmd_appraise( PLAYER *ch, char *argument )
{
    return;
}



