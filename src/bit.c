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
#include "edit.h"
#include "script.h"
#include "shop.h"


char *target_bit_name( int vector ) {
  switch ( vector ) {
  case TAR_CHAR_DEFENSIVE: return "defensive";
  case TAR_CHAR_OFFENSIVE: return "offensive";
  case TAR_CHAR_SELF:      return "self";
  case TAR_PROP_INV:       return "prop";
  case TAR_IGNORE:         return "ignore";
    default:           return "unknown";
  }
}


int target_name_bit( char *buf ) {
  if ( !str_cmp( buf, "ignore"    )) return TAR_IGNORE;
  if ( !str_cmp( buf, "offensive" )) return TAR_CHAR_OFFENSIVE;
  if ( !str_cmp( buf, "defensive" )) return TAR_CHAR_DEFENSIVE;
  if ( !str_cmp( buf, "self"      )) return TAR_CHAR_SELF;
  if ( !str_cmp( buf, "prop"      )) return TAR_PROP_INV;
  return TAR_IGNORE;
}


char *mana_bit_name( int vector )
{
    static char buf[512];

    if ( vector == MANA_NONE ) return "none";
    if ( vector == MANA_ANY  ) return "any";

    buf[0] = '\0';
    strcat( buf, ( vector & MANA_AIR       ) ? " air"    : "" );
    strcat( buf, ( vector & MANA_EARTH     ) ? " earth"  : "" );
    strcat( buf, ( vector & MANA_FIRE      ) ? " fire"   : "" );
    strcat( buf, ( vector & MANA_WATER     ) ? " water"  : "" );
    return (buf[0] == '\0') ? buf : buf+1;;
}



/*
 * Return bit vector
 */
int mana_name_bit( char* buf )
{
    if ( is_number(buf) ) return atoi(buf);
    if (!str_cmp( buf, "any"      )) return MANA_ANY;
    if (!str_cmp( buf, "none"     )) return MANA_NONE;
    if (!str_cmp( buf, "earth"    )) return MANA_EARTH;
    if (!str_cmp( buf, "air"      )) return MANA_AIR;
    if (!str_cmp( buf, "fire"     )) return MANA_FIRE;
    if (!str_cmp( buf, "water"    )) return MANA_WATER;
    return MANA_NONE;
};


/*
 * Return ascii name of an item type.
 */
char *item_type_name( int item_type )
{
    switch ( item_type )
    {
    case ITEM_DELETE:        return "delete";
    case ITEM_TRASH:         return "trash";
    case ITEM_LIGHT:         return "light";
    case ITEM_SCROLL:        return "scroll";
    case ITEM_WAND:          return "wand";
    case ITEM_STAFF:         return "staff";
    case ITEM_WEAPON:        return "weapon";
    case ITEM_RANGED_WEAPON: return "ranged";
    case ITEM_AMMO:          return "ammo";
    case ITEM_TREASURE:      return "treasure";
    case ITEM_ARMOR:         return "armor";
    case ITEM_POTION:        return "potion";
    case ITEM_BOOK:          return "book";
    case ITEM_FURNITURE:     return "furniture";
    case ITEM_CLOTHING:      return "clothing";
    case ITEM_PAPER:         return "paper";
    case ITEM_CONTAINER:     return "container";
    case ITEM_THROWN:        return "thrown";
    case ITEM_DRINK_CON:     return "drink";
    case ITEM_KEY:           return "key";
    case ITEM_FOOD:          return "food";
    case ITEM_MONEY:         return "money";
    case ITEM_GEM:           return "gem";
    case ITEM_VEHICLE:       return "vehicle";
    case ITEM_CORPSE_NPC:    return "npc corpse";
    case ITEM_CORPSE_PC:     return "pc corpse";
    case ITEM_FOUNTAIN:      return "fountain";
    case ITEM_PILL:          return "pill";
    case ITEM_TOOL:          return "tool";
    case ITEM_LIST:          return "list";
    case ITEM_BOARD:         return "board";
    case ITEM_COMPONENT:     return "component";
    case ITEM_GOODS:         return "goods";
    case ITEM_JEWELRY:       return "jewelry";
    case ITEM_SPELLBOOK:     return "spellbook";
    case ITEM_QUILL:         return "quill";
    }

    bug( "Item_type_name: unknown type %d.", item_type );
    return "(unknown)";
}


int item_valflag( char *name )
{
    int v=0;
    if ( strlen(name) < 3 ) return 0;
    if ( is_number(name) ) return atoi(name);
    if ( !str_infix( name, "tinderbox" ) ) v |= TOOL_TINDERBOX;
    if ( !str_infix( name, "lockpick"  ) ) v |= TOOL_LOCKPICK;
    if ( !str_infix( name, "bandages"  ) ) v |= TOOL_BANDAGES;
    if ( !str_infix( name, "bounty"    ) ) v |= TOOL_BOUNTY;
    if ( !str_infix( name, "repair"    ) ) v |= TOOL_REPAIR;
    if ( !str_infix( name, "lit"       ) ) v |= LIGHT_LIT;
    if ( !str_infix( name, "fillable"  ) ) v |= LIGHT_LIT;
    if ( !str_infix( name, "closeable" ) ) v |= FURN_CLOSEABLE;
    if ( !str_infix( name, "pickproof" ) ) v |= FURN_PICKPROOF;
    if ( !str_infix( name, "closed"    ) ) v |= FURN_CLOSED;
    if ( !str_infix( name, "locked"    ) ) v |= FURN_LOCKED;
    if ( !str_infix( name, "nomount"   ) ) v |= FURN_NOMOUNT;
    if ( !str_infix( name, "sit"       ) ) v |= FURN_SIT;
    if ( !str_infix( name, "sleep"     ) ) v |= FURN_SLEEP;
    if ( !str_infix( name, "exit"      ) ) v |= FURN_EXIT;
    if ( !str_infix( name, "put"       ) ) v |= FURN_PUT;
    if ( !str_infix( name, "move"      ) ) v |= FURN_MOVE;
    if ( !str_infix( name, "noshow"    ) ) v |= FURN_NOSHOW;
    if ( !str_infix( name, "home"      ) ) v |= FURN_HOME;
    if ( !str_infix( name, "poison"    ) ) v |= DRINK_POISON;
    if ( !str_infix( name, "tavern"    ) ) v |= DRINK_TAVERN;
    if ( !str_infix( name, "secret"    ) ) v |= CONT_SECRET;
    if ( !str_infix( name, "bow"       ) ) v |= RNG_BOW;
    if ( !str_infix( name, "crossbow"  ) ) v |= RNG_CROSSBOW;
    if ( !str_infix( name, "catapult"  ) ) v |= RNG_CATAPULT;
    return v;
}


int item_name_type( char *name )
{
    if ( is_number(name) ) return atoi(name);
    if ( !str_cmp( name, "trash"     ) ) return ITEM_TRASH;
    if ( !str_cmp( name, "light"     ) ) return ITEM_LIGHT;
    if ( !str_cmp( name, "scroll"    ) ) return ITEM_SCROLL;
    if ( !str_cmp( name, "wand"      ) ) return ITEM_WAND;
    if ( !str_cmp( name, "staff"     ) ) return ITEM_STAFF;
    if ( !str_cmp( name, "weapon"    ) ) return ITEM_WEAPON;
    if ( !str_cmp( name, "ranged"    ) ) return ITEM_RANGED_WEAPON;
    if ( !str_cmp( name, "ammo"      ) ) return ITEM_AMMO;
    if ( !str_cmp( name, "treasure"  ) ) return ITEM_TREASURE;
    if ( !str_cmp( name, "armor"     ) ) return ITEM_ARMOR;
    if ( !str_cmp( name, "potion"    ) ) return ITEM_POTION;
    if ( !str_cmp( name, "book"      ) ) return ITEM_BOOK;
    if ( !str_cmp( name, "furniture" ) ) return ITEM_FURNITURE;
    if ( !str_cmp( name, "clothing"  ) ) return ITEM_CLOTHING;
    if ( !str_cmp( name, "paper"     ) ) return ITEM_PAPER;
    if ( !str_cmp( name, "container" ) ) return ITEM_CONTAINER;
    if ( !str_cmp( name, "thrown"    ) ) return ITEM_THROWN;
    if ( !str_cmp( name, "drink"     ) ) return ITEM_DRINK_CON;
    if ( !str_cmp( name, "key"       ) ) return ITEM_KEY;
    if ( !str_cmp( name, "food"      ) ) return ITEM_FOOD;
    if ( !str_cmp( name, "money"     ) ) return ITEM_MONEY;
    if ( !str_cmp( name, "gem"       ) ) return ITEM_GEM;
    if ( !str_cmp( name, "vehicle"   ) ) return ITEM_VEHICLE;
    if ( !str_cmp( name, "corpse"    ) ) return ITEM_CORPSE_NPC;
    if ( !str_cmp( name, "fountain"  ) ) return ITEM_FOUNTAIN;
    if ( !str_cmp( name, "pill"      ) ) return ITEM_PILL;
    if ( !str_cmp( name, "tool"      ) ) return ITEM_TOOL;
    if ( !str_cmp( name, "list"      ) ) return ITEM_LIST;
    if ( !str_cmp( name, "board"     ) ) return ITEM_BOARD;
    if ( !str_cmp( name, "component" ) ) return ITEM_COMPONENT;
    if ( !str_cmp( name, "goods"     ) ) return ITEM_GOODS;
    if ( !str_cmp( name, "jewelry"   ) ) return ITEM_JEWELRY;
    if ( !str_cmp( name, "index"     ) ) return ITEM_INDEX;
    if ( !str_cmp( name, "trash"     ) ) return ITEM_TRASH;
    if ( !str_cmp( name, "spellbook" ) ) return ITEM_SPELLBOOK;
    if ( !str_cmp( name, "quill"     ) ) return ITEM_QUILL;
    return -1;
}


/*
 * Return ascii name of an affect location.
 */
char *bonus_loc_name( int location )
{
    SKILL_DATA *pIndex;

    switch ( location )
    {
    case APPLY_NONE:            return "none";
    case APPLY_STR:             return "strength";
    case APPLY_DEX:             return "dexterity";
    case APPLY_INT:             return "intelligence";
    case APPLY_WIS:             return "wisdom";
    case APPLY_CON:             return "constitution";
    case APPLY_SEX:             return "sex";
    case APPLY_SIZE:            return "size";
    case APPLY_AGE:             return "age";
    case APPLY_AC:              return "armor class";
    case APPLY_HITROLL:         return "hit roll";
    case APPLY_DAMROLL:         return "damage roll";
    case APPLY_SAVING_THROW:    return "saving throw";
    }

    if ( (pIndex=get_skill_index( location )) )
    {
        return pIndex->name;
    }

    bug( "Bonus_location_name: unknown location %d.", location );
    return "(unknown)";

}

int bonus_name_loc( char* name )
{
    SKILL_DATA *pSkill;

    if ( is_number(name) ) return atoi(name);
    if ( !str_cmp( name, "none"          ) ) return APPLY_NONE;
    if ( !str_cmp( name, "strength"      ) ) return APPLY_STR;
    if ( !str_cmp( name, "dexterity"     ) ) return APPLY_DEX;
    if ( !str_cmp( name, "intelligence"  ) ) return APPLY_INT;
    if ( !str_cmp( name, "wisdom"        ) ) return APPLY_WIS;
    if ( !str_cmp( name, "constitution"  ) ) return APPLY_CON;
    if ( !str_cmp( name, "sex"           ) ) return APPLY_SEX;
    if ( !str_cmp( name, "age"           ) ) return APPLY_AGE;
    if ( !str_cmp( name, "size"          ) ) return APPLY_SIZE;
    if ( !str_cmp( name, "ac"            ) ) return APPLY_AC;
    if ( !str_cmp( name, "hit"           ) ) return APPLY_HITROLL;
    if ( !str_cmp( name, "dam"           ) ) return APPLY_DAMROLL;
    if ( !str_cmp( name, "saving_throw"  ) ) return APPLY_SAVING_THROW;
    if ( (pSkill=skill_lookup( name )    ) ) return pSkill->vnum;
    return APPLY_NONE;
}



/*
 * Return ascii name of an affect bit vector.
 */
char *bonus_bit_name( int vector )
{
    static char buf[512];

    buf[0] = '\0';
    strcat( buf, ( vector & AFF_BLIND         ) ? " blind"         : "" );
    strcat( buf, ( vector & AFF_METAMORPH     ) ? " metamorph"     : "" );
    strcat( buf, ( vector & AFF_INVISIBLE     ) ? " invisible"     : "" );
    strcat( buf, ( vector & AFF_DETECT_EVIL   ) ? " detect-evil"   : "" );
    strcat( buf, ( vector & AFF_DETECT_INVIS  ) ? " detect-invis"  : "" );
    strcat( buf, ( vector & AFF_DETECT_MAGIC  ) ? " detect-magic"  : "" );
    strcat( buf, ( vector & AFF_DETECT_HIDDEN ) ? " detect-hidden" : "" );
    strcat( buf, ( vector & AFF_HOLD          ) ? " hold"          : "" );
    strcat( buf, ( vector & AFF_SANCTUARY     ) ? " sanctuary"     : "" );
    strcat( buf, ( vector & AFF_FAERIE_FIRE   ) ? " faerie-fire"   : "" );
    strcat( buf, ( vector & AFF_INFRARED      ) ? " infrared"      : "" );
    strcat( buf, ( vector & AFF_CURSE         ) ? " curse"         : "" );
    strcat( buf, ( vector & AFF_FLAMING       ) ? " flaming"       : "" );
    strcat( buf, ( vector & AFF_POISON        ) ? " poison"        : "" );
    strcat( buf, ( vector & AFF_PROTECT       ) ? " protect"       : "" );
    strcat( buf, ( vector & AFF_PARALYSIS     ) ? " paralysis"     : "" );
    strcat( buf, ( vector & AFF_SLEEP         ) ? " sleep"         : "" );
    strcat( buf, ( vector & AFF_SNEAK         ) ? " sneak"         : "" );
    strcat( buf, ( vector & AFF_HIDE          ) ? " hide"          : "" );
    strcat( buf, ( vector & AFF_CHARM         ) ? " charm"         : "" );
    strcat( buf, ( vector & AFF_FLYING        ) ? " flying"        : "" );
    strcat( buf, ( vector & AFF_PASS_DOOR     ) ? " pass-door"     : "" );
    strcat( buf, ( vector & AFF_FREEACTION    ) ? " free-action"   : "" );
    strcat( buf, ( vector & AFF_BREATHING     ) ? " water-breath"  : "" );
    return (buf[0] == '\0') ? buf : buf+1;;
}



/*
 * Return bit vector
 */
int bonus_name_bit( char* buf )
{
    int v=0;
    if ( strlen(buf) <= 3 ) return 0;
    if ( is_number(buf) ) return atoi(buf);
    if (!str_infix( buf, "blind"         )) v|= AFF_BLIND;
    if (!str_infix( buf, "invisible"     )) v|= AFF_INVISIBLE;
    if (!str_infix( buf, "detect-evil"   )) v|= AFF_DETECT_EVIL;
    if (!str_infix( buf, "detect-invis"  )) v|= AFF_DETECT_INVIS;
    if (!str_infix( buf, "detect-magic"  )) v|= AFF_DETECT_MAGIC;
    if (!str_infix( buf, "detect-hidden" )) v|= AFF_DETECT_HIDDEN;
    if (!str_infix( buf, "hold"          )) v|= AFF_HOLD;
    if (!str_infix( buf, "sanctuary"     )) v|= AFF_SANCTUARY;
    if (!str_infix( buf, "faerie-fire"   )) v|= AFF_FAERIE_FIRE;
    if (!str_infix( buf, "infrared"      )) v|= AFF_INFRARED;
    if (!str_infix( buf, "curse"         )) v|= AFF_CURSE;
    if (!str_infix( buf, "flaming"       )) v|= AFF_FLAMING;
    if (!str_infix( buf, "poisoned"      )) v|= AFF_POISON;
    if (!str_infix( buf, "protect"       )) v|= AFF_PROTECT;
    if (!str_infix( buf, "paralysis"     )) v|= AFF_PARALYSIS;
    if (!str_infix( buf, "sleep"         )) v|= AFF_SLEEP;
    if (!str_infix( buf, "sneak"         )) v|= AFF_SNEAK;
    if (!str_infix( buf, "hide"          )) v|= AFF_HIDE;
    if (!str_infix( buf, "charm"         )) v|= AFF_CHARM;
    if (!str_infix( buf, "flying"        )) v|= AFF_FLYING;
    if (!str_infix( buf, "pass-door"     )) v|= AFF_PASS_DOOR;
    if (!str_infix( buf, "free-action"   )) v|= AFF_FREEACTION;
    if (!str_infix( buf, "water-breath"  )) v|= AFF_BREATHING;
    return v;
}



/*
 * Return ascii name of extra flags vector.
 */
char *extra_bit_name( int extra_flags )
{
    static char buf[512];

    buf[0] = '\0';
    strcat( buf, ( extra_flags & ITEM_GLOW         ) ? " glow"         : "" );
    strcat( buf, ( extra_flags & ITEM_HUM          ) ? " hum"          : "" );
    strcat( buf, ( extra_flags & ITEM_DARK         ) ? " dark"         : "" );
    strcat( buf, ( extra_flags & ITEM_LOCK         ) ? " lock"         : "" );
    strcat( buf, ( extra_flags & ITEM_EVIL         ) ? " evil"         : "" );
    strcat( buf, ( extra_flags & ITEM_INVIS        ) ? " invis"        : "" );
    strcat( buf, ( extra_flags & ITEM_MAGIC        ) ? " magic"        : "" );
    strcat( buf, ( extra_flags & ITEM_NODROP       ) ? " nodrop"       : "" );
    strcat( buf, ( extra_flags & ITEM_BLESS        ) ? " bless"        : "" );
    strcat( buf, ( extra_flags & ITEM_NOREMOVE     ) ? " noremove"     : "" );
    strcat( buf, ( extra_flags & ITEM_INVENTORY    ) ? " inventory"    : "" );
    strcat( buf, ( extra_flags & ITEM_NOSAVE       ) ? " nosave"       : "" );
    strcat( buf, ( extra_flags & ITEM_BURNING      ) ? " burning"      : "" );
    strcat( buf, ( extra_flags & ITEM_BURNT        ) ? " burnt"      : "" );
    strcat( buf, ( extra_flags & ITEM_HITCH        ) ? " hitch"        : "" );
    strcat( buf, ( extra_flags & ITEM_USED         ) ? " used"         : "" );
    return (buf[0] == '\0') ? buf : buf+1;;
}


int extra_name_bit( char* buf )
{
    int v=0;
    if ( strlen(buf) < 3 ) return 0;
    if ( is_number(buf) ) return atoi(buf);
    if (!str_infix( buf, "glow"        ) ) v|= ITEM_GLOW;
    if (!str_infix( buf, "hum"         ) ) v|= ITEM_HUM;
    if (!str_infix( buf, "dark"        ) ) v|= ITEM_DARK;
    if (!str_infix( buf, "lock"        ) ) v|= ITEM_LOCK;
    if (!str_infix( buf, "evil"        ) ) v|= ITEM_EVIL;
    if (!str_infix( buf, "invis"       ) ) v|= ITEM_INVIS;
    if (!str_infix( buf, "magic"       ) ) v|= ITEM_MAGIC;
    if (!str_infix( buf, "nodrop"      ) ) v|= ITEM_NODROP;
    if (!str_infix( buf, "bless"       ) ) v|= ITEM_BLESS;
    if (!str_infix( buf, "noremove"    ) ) v|= ITEM_NOREMOVE;
    if (!str_infix( buf, "inventory"   ) ) v|= ITEM_INVENTORY;
    if (!str_infix( buf, "nosave"      ) ) v|= ITEM_NOSAVE;
    if (!str_infix( buf, "burning"     ) ) v|= ITEM_BURNING;
    if (!str_infix( buf, "burnt"       ) ) v|= ITEM_BURNT;
    if (!str_infix( buf, "hitch"       ) ) v|= ITEM_HITCH;
    if (!str_infix( buf, "used"        ) ) v|= ITEM_USED;
    return v;
}



/*
 * Return ascii name of scene flags vector.
 */
char *scene_bit_name( int scene_flags )
{
    static char buf[512];

    buf[0] = '\0';
    strcat( buf, ( scene_flags & SCENE_DARK      ) ? " dark"      : ""   );
    strcat( buf, ( scene_flags & SCENE_NO_ACTOR    ) ? " no_actors"   : ""   );
    strcat( buf, ( scene_flags & SCENE_INDOORS   ) ? " enclosed"  : ""   );
    strcat( buf, ( scene_flags & SCENE_IMMORTAL  ) ? " immortal"  : ""   );
    strcat( buf, ( scene_flags & SCENE_MARK      ) ? " MARKED"    : ""   );
    strcat( buf, ( scene_flags & SCENE_SAFE      ) ? " safe"      : ""   );
    strcat( buf, ( scene_flags & SCENE_PET_SHOP  ) ? " pet_shop"  : ""   );
    strcat( buf, ( scene_flags & SCENE_WAGON     ) ? " wagon"     : ""   );
    strcat( buf, ( scene_flags & SCENE_SAVE      ) ? " saving"    : ""   );
    strcat( buf, ( scene_flags & SCENE_TEMPLATE  ) ? " template"  : ""   );
    strcat( buf, ( scene_flags & SCENE_BUILDABLE ) ? " buildable" : ""   );
    strcat( buf, ( scene_flags & SCENE_RECYCLE   ) ? " recycle"   : ""   );
    return (buf[0] == '\0') ? buf : buf+1;
}

int scene_name_bit( char* buf )
{
    int v=0;
    if ( strlen(buf) <= 3 ) return 0;
    if ( is_number(buf) ) return atoi(buf);
    if ( !str_infix( buf, "dark"         ) ) v|= SCENE_DARK;
    if ( !str_infix( buf, "no_actors"    ) ) v|= SCENE_NO_ACTOR;
    if ( !str_infix( buf, "enclosed"     ) ) v|= SCENE_INDOORS;
    if ( !str_infix( buf, "safe"         ) ) v|= SCENE_SAFE;
    if ( !str_infix( buf, "pet_shop"     ) ) v|= SCENE_PET_SHOP;
    if ( !str_infix( buf, "immortal"     ) ) v|= SCENE_IMMORTAL;
    if ( !str_infix( buf, "wagon"        ) ) v|= SCENE_WAGON;
    if ( !str_infix( buf, "saving"       ) ) v|= SCENE_SAVE;
    if ( !str_infix( buf, "template"     ) ) v|= SCENE_TEMPLATE;
    if ( !str_infix( buf, "buildable"    ) ) v|= SCENE_BUILDABLE;
    return v;
}


char *plr_bit_name( int actb )
{
    static char buf[512];

    buf[0] = '\0';
    if ( actb & PLR_IS_NPC     ) strcat( buf, " npc"           );
    if ( actb & PLR_SILENCE    ) strcat( buf, " silence"       );
    if ( actb & PLR_NO_EMOTE   ) strcat( buf, " no_emote"      );
    if ( actb & PLR_NO_TELL    ) strcat( buf, " no_tell"       );
    if ( actb & PLR_NO_WISH    ) strcat( buf, " no_wish"       );
    if ( actb & PLR_LOG        ) strcat( buf, " log"           );
    if ( actb & PLR_DENY       ) strcat( buf, " deny"          );
    if ( actb & PLR_FREEZE     ) strcat( buf, " freeze"        );
    if ( actb & PLR_LYCANTHROPE) strcat( buf, " lycanthrope"   );
    if ( actb & PLR_VAMPIRE    ) strcat( buf, " vampire"       );
    if ( actb & WIZ_NOTIFY     ) strcat( buf, " notify"        );
    if ( actb & PLR_APPLIED    ) strcat( buf, " NOT VALIDATED" );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *act_bit_name( int actb )
{
    static char buf[512];

    buf[0] = '\0';
    strcat( buf, ( actb & ACT_IS_NPC     ) ? " npc"          : ""  );
    strcat( buf, ( actb & ACT_SENTINEL   ) ? " sentinel"     : ""  );
    strcat( buf, ( actb & ACT_GOOD       ) ? " good"         : ""  );
    strcat( buf, ( actb & ACT_AGGRESSIVE ) ? " aggressive"   : ""  );
    strcat( buf, ( actb & ACT_MERCY )      ? " mercy"        : ""  );
    strcat( buf, ( actb & ACT_STAY_ZONE  ) ? " stay_zone"    : ""  );
    strcat( buf, ( actb & ACT_WIMPY      ) ? " wimpy"        : ""  );
    strcat( buf, ( actb & ACT_PET        ) ? " pet"          : ""  );
    strcat( buf, ( actb & ACT_PRACTICE   ) ? " practitioner" : ""  );
    strcat( buf, ( actb & ACT_HALT       ) ? " halted"       : ""  );
    strcat( buf, ( actb & ACT_BOUNTY     ) ? " bounty"       : ""  );
    strcat( buf, ( actb & ACT_MOUNT      ) ? " mountable"    : ""  );
    strcat( buf, ( actb & ACT_LYCANTHROPE) ? " lycanthrope"  : ""  );
    strcat( buf, ( actb & ACT_VAMPIRE)     ? " vampire"      : ""  );
    strcat( buf, ( actb & ACT_NOSCAN     ) ? " noscan"       : ""  );
    strcat( buf, ( actb & ACT_NOCORPSE   ) ? " nocorpse"     : ""  );
    return (buf[0] == '\0') ? buf : buf+1;;
}

int act_name_bit( char* buf )
{
    int v=0;
    if ( strlen(buf) < 3 ) return 0;
    if ( is_number(buf) ) return atoi(buf);
    if ( !str_infix( buf, "npc"          ) ) v |= ACT_IS_NPC;
    if ( !str_infix( buf, "sentinel"     ) ) v |= ACT_SENTINEL;
    if ( !str_infix( buf, "good"         ) ) v |= ACT_GOOD;
    if ( !str_infix( buf, "aggressive"   ) ) v |= ACT_AGGRESSIVE;
    if ( !str_infix( buf, "stay_zone"    ) ) v |= ACT_STAY_ZONE;
    if ( !str_infix( buf, "wimpy"        ) ) v |= ACT_WIMPY;
    if ( !str_infix( buf, "pet"          ) ) v |= ACT_PET;
    if ( !str_infix( buf, "practitioner" ) ) v |= ACT_PRACTICE;
    if ( !str_infix( buf, "halted"       ) ) v |= ACT_HALT;
    if ( !str_infix( buf, "bounty"       ) ) v |= ACT_BOUNTY;
    if ( !str_infix( buf, "mount"        ) ) v |= ACT_MOUNT;
    if ( !str_infix( buf, "lycanthrope"  ) ) v |= ACT_LYCANTHROPE;
    if ( !str_infix( buf, "vampire"      ) ) v |= ACT_VAMPIRE;
    if ( !str_infix( buf, "nocorpse"     ) ) v |= ACT_NOCORPSE;
    return v;
}

/*
 * Returns the name of a wear bit.
 */
char *wear_bit_name( int wear )
{
    static char buf[512];

    buf[0] = '\0';
    strcat( buf, ( wear & ITEM_TAKE        )   ? " take"       : ""  );
    strcat( buf, ( wear & ITEM_HOLD        )   ? " hold"       : ""  );
    strcat( buf, ( wear & ITEM_WIELD       )   ? " wield"      : ""  );
    strcat( buf, ( wear & ITEM_WEAR_SHIELD )   ? " shield"     : ""  );
    strcat( buf, ( wear & ITEM_WEAR_FINGER )   ? " finger"     : ""  );
    strcat( buf, ( wear & ITEM_WEAR_NECK   )   ? " neck"       : ""  );
    strcat( buf, ( wear & ITEM_WEAR_BODY   )   ? " body"       : ""  );
    strcat( buf, ( wear & ITEM_WEAR_HEAD   )   ? " head"       : ""  );
    strcat( buf, ( wear & ITEM_WEAR_LEGS   )   ? " legs"       : ""  );
    strcat( buf, ( wear & ITEM_WEAR_FEET   )   ? " feet"       : ""  );
    strcat( buf, ( wear & ITEM_WEAR_HANDS  )   ? " hands"      : ""  );
    strcat( buf, ( wear & ITEM_WEAR_ARMS   )   ? " arms"       : ""  );
    strcat( buf, ( wear & ITEM_WEAR_ABOUT  )   ? " about"      : ""  );
    strcat( buf, ( wear & ITEM_WEAR_WAIST  )   ? " waist"      : ""  );
    strcat( buf, ( wear & ITEM_WEAR_WRIST  )   ? " wrist"      : ""  );
    strcat( buf, ( wear & ITEM_WEAR_BELT   )   ? " belt"       : ""  );
    strcat( buf, ( wear & ITEM_WEAR_SHOULDER ) ? " shoulder"   : ""  );
    strcat( buf, ( wear & ITEM_WEAR_ANKLE    ) ? " ankle"      : ""  );
    strcat( buf, ( wear & ITEM_WEAR_EAR      ) ? " ear"        : ""  );
    strcat( buf, ( wear & ITEM_WEAR_HELM     ) ? " helm"       : ""  );
    strcat( buf, ( wear & ITEM_WEAR_SHIRT    ) ? " shirt"      : ""  );
    strcat( buf, ( wear & ITEM_WEAR_STOCKING ) ? " stockings"  : ""  );
    strcat( buf, ( wear & ITEM_WEAR_FACE     ) ? " face"       : ""  );
    strcat( buf, ( wear & ITEM_WEAR_BACK     ) ? " back"       : ""  );
    strcat( buf, ( wear & ITEM_WEAR_LOIN     ) ? " loins"      : ""  );
    strcat( buf, ( wear & ITEM_WEAR_FOREHEAD ) ? " forehead"   : ""  );
    strcat( buf, ( wear & ITEM_WEAR_NOSE     ) ? " nose"       : ""  );
    strcat( buf, ( wear & ITEM_WEAR_PANTS    ) ? " pants"      : ""  );
    strcat( buf, ( wear & ITEM_WEAR_ASBELT   ) ? " as_belt"    : ""  );
    strcat( buf, ( wear & ITEM_WEAR_PIN      ) ? " pin"        : ""  );
    strcat( buf, ( wear & ITEM_TWO_HANDED    ) ? " two-handed" : ""  );
    return (buf[0] == '\0') ? buf : buf+1;;
}


/*
 * Returns the bit, given a certain name.
 */
int wear_name_bit( char* buf )
{
    if ( strlen(buf) < 3 ) return 0;
    if ( is_number(buf) ) return atoi(buf);
    if (!str_cmp( buf, "take"       ) ) return ITEM_TAKE;
    if (!str_cmp( buf, "hold"       ) ) return ITEM_HOLD;
    if (!str_cmp( buf, "finger"     ) ) return ITEM_WEAR_FINGER;
    if (!str_cmp( buf, "neck"       ) ) return ITEM_WEAR_NECK;
    if (!str_cmp( buf, "body"       ) ) return ITEM_WEAR_BODY;
    if (!str_cmp( buf, "head"       ) ) return ITEM_WEAR_HEAD;
    if (!str_cmp( buf, "legs"       ) ) return ITEM_WEAR_LEGS;
    if (!str_cmp( buf, "feet"       ) ) return ITEM_WEAR_FEET;
    if (!str_cmp( buf, "hands"      ) ) return ITEM_WEAR_HANDS;
    if (!str_cmp( buf, "arms"       ) ) return ITEM_WEAR_ARMS;
    if (!str_cmp( buf, "shield"     ) ) return ITEM_WEAR_SHIELD;
    if (!str_cmp( buf, "about"      ) ) return ITEM_WEAR_ABOUT;
    if (!str_cmp( buf, "waist"      ) ) return ITEM_WEAR_WAIST;
    if (!str_cmp( buf, "wrist"      ) ) return ITEM_WEAR_WRIST;
    if (!str_cmp( buf, "wield"      ) ) return ITEM_WIELD;
    if (!str_cmp( buf, "shoulder"   ) ) return ITEM_WEAR_SHOULDER;
    if (!str_cmp( buf, "ankle"      ) ) return ITEM_WEAR_ANKLE;
    if (!str_cmp( buf, "ear"        ) ) return ITEM_WEAR_EAR;
    if (!str_cmp( buf, "helm"       ) ) return ITEM_WEAR_HELM;
    if (!str_cmp( buf, "shirt"      ) ) return ITEM_WEAR_SHIRT;
    if (!str_cmp( buf, "stockings"  ) ) return ITEM_WEAR_STOCKING;
    if (!str_cmp( buf, "face"       ) ) return ITEM_WEAR_FACE;
    if (!str_cmp( buf, "back"       ) ) return ITEM_WEAR_BACK;
    if (!str_cmp( buf, "loins"      ) ) return ITEM_WEAR_LOIN;
    if (!str_cmp( buf, "forehead"   ) ) return ITEM_WEAR_FOREHEAD;
    if (!str_cmp( buf, "nose"       ) ) return ITEM_WEAR_NOSE;
    if (!str_cmp( buf, "pants"      ) ) return ITEM_WEAR_PANTS;
    if (!str_cmp( buf, "belt"       ) ) return ITEM_WEAR_BELT;
    if (!str_cmp( buf, "pin"        ) ) return ITEM_WEAR_PIN;
    if (!str_cmp( buf, "as_belt"    ) ) return ITEM_WEAR_ASBELT;
    if (!str_cmp( buf, "two-handed"   ) ) return ITEM_TWO_HANDED;
    return 0;
}


/*
 * Return ascii name of wear location.
 */
char *wear_loc_name( int wearloc )
{
    switch( wearloc )
    {
                default: return "unknown";
         case WEAR_NONE: return "none";
     case WEAR_FINGER_L: return "left finger";
     case WEAR_FINGER_R: return "right finger";
       case WEAR_NECK_1: return "neck1";
       case WEAR_NECK_2: return "neck2";
         case WEAR_BODY: return "body";
         case WEAR_HEAD: return "head";
         case WEAR_LEGS: return "legs";
         case WEAR_FEET: return "feet";
        case WEAR_HANDS: return "hands";
         case WEAR_ARMS: return "arms";
       case WEAR_SHIELD: return "shield";
        case WEAR_ABOUT: return "about";
        case WEAR_WAIST: return "waist";
      case WEAR_WRIST_L: return "lwrist";
      case WEAR_WRIST_R: return "rwrist";
     case WEAR_FLOATING: return "floating";
       case WEAR_BELT_1: return "belt1";
       case WEAR_BELT_2: return "belt2";
       case WEAR_BELT_3: return "belt3";
       case WEAR_BELT_4: return "belt4";
       case WEAR_BELT_5: return "belt5";
       case WEAR_HOLD_1: return "hold1";
       case WEAR_HOLD_2: return "hold2";
   case WEAR_SHOULDER_L: return "left shoulder";
   case WEAR_SHOULDER_R: return "right shoulder";
          case MAX_WEAR: return "max_wear";
      case WEAR_ANKLE_L: return "left ankle";
      case WEAR_ANKLE_R: return "right ankle";
        case WEAR_EAR_L: return "left ear";
        case WEAR_EAR_R: return "right ear";
         case WEAR_HELM: return "helm";
        case WEAR_SHIRT: return "shirt";
     case WEAR_STOCKING: return "stocking";
         case WEAR_FACE: return "face";
         case WEAR_BACK: return "back";
         case WEAR_LOIN: return "loin";
     case WEAR_FOREHEAD: return "forehead";
         case WEAR_NOSE: return "nose";
        case WEAR_PANTS: return "pants";
      case WEAR_WIELD_1: return "wield1";
      case WEAR_WIELD_2: return "wield2";
     case WEAR_ASBELT_1: return "asbelt1";
     case WEAR_ASBELT_2: return "asbelt2";
     case WEAR_ASBELT_3: return "asbelt3";
    }
}


int wear_name_loc( char *buf )
{
    if ( is_number(buf) ) return atoi(buf);
    if ( !str_cmp( buf, "lfinger" ) )   return WEAR_FINGER_L;
    if ( !str_cmp( buf, "rfinger" ) )   return WEAR_FINGER_R;
    if ( !str_cmp( buf, "neck1" ) )     return WEAR_NECK_1;
    if ( !str_cmp( buf, "neck2" ) )     return WEAR_NECK_2;
    if ( !str_cmp( buf, "body" ) )      return WEAR_BODY;
    if ( !str_cmp( buf, "head" ) )      return WEAR_HEAD;
    if ( !str_cmp( buf, "legs" ) )      return WEAR_LEGS;
    if ( !str_cmp( buf, "feet" ) )      return WEAR_FEET;
    if ( !str_cmp( buf, "hands" ) )     return WEAR_HANDS;
    if ( !str_cmp( buf, "arms" ) )      return WEAR_ARMS;
    if ( !str_cmp( buf, "shield" ) )    return WEAR_SHIELD;
    if ( !str_cmp( buf, "about" ) )     return WEAR_ABOUT;
    if ( !str_cmp( buf, "waist" ) )     return WEAR_WAIST;
    if ( !str_cmp( buf, "lwrist" ) )    return WEAR_WRIST_L;
    if ( !str_cmp( buf, "rwrist" ) )    return WEAR_WRIST_R;
    if ( !str_cmp( buf, "floating" ) )  return WEAR_FLOATING;
    if ( !str_cmp( buf, "belt1" ) )     return WEAR_BELT_1;
    if ( !str_cmp( buf, "belt2" ) )     return WEAR_BELT_2;
    if ( !str_cmp( buf, "belt3" ) )     return WEAR_BELT_3;
    if ( !str_cmp( buf, "belt4" ) )     return WEAR_BELT_4;
    if ( !str_cmp( buf, "belt5" ) )     return WEAR_BELT_5;
    if ( !str_cmp( buf, "hold1" ) )     return WEAR_HOLD_1;
    if ( !str_cmp( buf, "hold2" ) )     return WEAR_HOLD_2;
    if ( !str_cmp( buf, "lshoulder" ) ) return WEAR_SHOULDER_L;
    if ( !str_cmp( buf, "rshoulder" ) ) return WEAR_SHOULDER_R;
    if ( !str_cmp( buf, "lear"  ) )     return WEAR_EAR_L;
    if ( !str_cmp( buf, "rear"  ) )     return WEAR_EAR_R;
    if ( !str_cmp( buf, "lankle" ) )    return WEAR_ANKLE_L;
    if ( !str_cmp( buf, "rankle" ) )    return WEAR_ANKLE_R;
    if ( !str_cmp( buf, "shirt" ) )     return WEAR_SHIRT;
    if ( !str_cmp( buf, "pants" ) )     return WEAR_PANTS;
    if ( !str_cmp( buf, "back" ) )      return WEAR_BACK;
    if ( !str_cmp( buf, "nose" ) )      return WEAR_NOSE;
    if ( !str_cmp( buf, "forehead" ) )  return WEAR_FOREHEAD;
    if ( !str_cmp( buf, "helm" ) )      return WEAR_HELM;
    if ( !str_cmp( buf, "stockings" ) ) return WEAR_STOCKING;
    if ( !str_cmp( buf, "face" ) )      return WEAR_FACE;
    if ( !str_cmp( buf, "wield1" ) )    return WEAR_WIELD_1;
    if ( !str_cmp( buf, "wield2" ) )    return WEAR_WIELD_2;
    if ( !str_cmp( buf, "asbelt1" ) )   return WEAR_ASBELT_1;
    if ( !str_cmp( buf, "asbelt2" ) )   return WEAR_ASBELT_2;
    if ( !str_cmp( buf, "asbelt3" ) )   return WEAR_ASBELT_3;
    return WEAR_NONE;
}
/*
 * Returns the bit, given a certain name.
 */


int sector_number( char *argument )
{
    if ( is_number(argument) ) return atoi(argument);
    if ( !str_cmp( argument, "inside" ) )       return SECT_INSIDE;
    if ( !str_cmp( argument, "city" ) )         return SECT_CITY;
    if ( !str_cmp( argument, "field" ) )        return SECT_FIELD;
    if ( !str_cmp( argument, "forest" ) )       return SECT_FOREST;
    if ( !str_cmp( argument, "hills" ) )        return SECT_HILLS;
    if ( !str_cmp( argument, "mountain" ) )     return SECT_MOUNTAIN;
    if ( !str_cmp( argument, "swim" ) )         return SECT_WATER_SWIM;
    if ( !str_cmp( argument, "noswim" ) )       return SECT_WATER_NOSWIM;
    if ( !str_cmp( argument, "underwater" ) )   return SECT_UNDERWATER;
    if ( !str_cmp( argument, "air" ) )          return SECT_AIR;
    if ( !str_cmp( argument, "desert" ) )       return SECT_DESERT;
    if ( !str_cmp( argument, "iceland" ) )      return SECT_ICELAND;
    if ( !str_cmp( argument, "climb" ) )        return SECT_CLIMB;
    return SECT_MAX;
}

char *sector_name( int sect )
{
    if ( sect == SECT_INSIDE )       return "inside";
    if ( sect == SECT_CITY )         return "city";
    if ( sect == SECT_FIELD )        return "field";
    if ( sect == SECT_FOREST )       return "forest";
    if ( sect == SECT_HILLS )        return "hills";
    if ( sect == SECT_MOUNTAIN )     return "mountain";
    if ( sect == SECT_WATER_SWIM )   return "swim";
    if ( sect == SECT_WATER_NOSWIM ) return "noswim";
    if ( sect == SECT_UNDERWATER )   return "underwater";
    if ( sect == SECT_AIR )          return "air";
    if ( sect == SECT_DESERT )       return "desert";
    if ( sect == SECT_ICELAND  )     return "iceland";
    if ( sect == SECT_CLIMB    )     return "climb";
    return "unknown";
}




char *position_name( int pos )
{
    if ( pos == POS_DEAD     )    return "dead";
    if ( pos == POS_MORTAL   )    return "mortal";
    if ( pos == POS_INCAP    )    return "incapacitated";
    if ( pos == POS_STUNNED  )    return "stunned";
    if ( pos == POS_SLEEPING )    return "sleeping";
    if ( pos == POS_RESTING  )    return "resting";
    if ( pos == POS_SITTING  )    return "sitting";
    if ( pos == POS_FIGHTING )    return "fighting";
    if ( pos == POS_STANDING )    return "standing";
    return "unknown";
}

int name_position( char *pos )
{
    if ( is_number(pos) ) return atoi(pos);
    if ( !str_cmp( pos, "dead" ) ) return POS_DEAD;
    if ( !str_cmp( pos, "mortal" ) ) return POS_MORTAL;
    if ( !str_cmp( pos, "incapacitated" ) ) return POS_INCAP;
    if ( !str_cmp( pos, "stunned" ) ) return POS_STUNNED;
    if ( !str_cmp( pos, "sleeping" ) ) return POS_SLEEPING;
    if ( !str_cmp( pos, "resting" )) return POS_RESTING;
    if ( !str_cmp( pos, "sitting" ) ) return POS_SITTING;
    if ( !str_cmp( pos, "fighting" ) ) return POS_FIGHTING;
    if ( !str_cmp( pos, "standing" ) ) return POS_STANDING;
    return -1;
}



int size_number( char *argument )
{
    if ( is_number(argument) ) return atoi(argument);
    if ( !str_cmp( argument, "any"         ) )    return SIZE_ANY;
    if ( !str_cmp( argument, "minute"      ) )    return SIZE_MINUTE;
    if ( !str_cmp( argument, "small"       ) )    return SIZE_SMALL;
    if ( !str_cmp( argument, "petite"      ) )    return SIZE_PETITE;
    if ( !str_cmp( argument, "average"     ) )    return SIZE_AVERAGE;
    if ( !str_cmp( argument, "medium"      ) )    return SIZE_MEDIUM;
    if ( !str_cmp( argument, "large"       ) )    return SIZE_LARGE;
    if ( !str_cmp( argument, "huge"        ) )    return SIZE_HUGE;
    if ( !str_cmp( argument, "titanic"     ) )    return SIZE_TITANIC;
    if ( !str_cmp( argument, "gargantuan"  ) )    return SIZE_GARGANTUAN;
    return SIZE_ANY-1;
}



char *size_name( int size )
{
    if ( size == SIZE_ANY        )    return "any";
    if ( size == SIZE_MINUTE     )    return "minute";
    if ( size == SIZE_SMALL      )    return "small";
    if ( size == SIZE_PETITE     )    return "petite";
    if ( size == SIZE_AVERAGE    )    return "average";
    if ( size == SIZE_MEDIUM     )    return "medium";
    if ( size == SIZE_LARGE      )    return "large";
    if ( size == SIZE_HUGE       )    return "huge";
    if ( size == SIZE_TITANIC    )    return "titanic";
    if ( size == SIZE_GARGANTUAN )    return "gargantuan";
    return "unknown";
}



/*
 * Takes a string and returns its designated direction.
 */
int get_dir( char *arg )
{
    int door = MAX_DIR;

       if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) door = DIR_NORTH;
  else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) door = DIR_EAST;
  else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) door = DIR_SOUTH;
  else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) door = DIR_WEST;
  else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = DIR_UP;
  else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = DIR_DOWN;
 else if ( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest" ) ) door = DIR_NW;
 else if ( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast" ) ) door = DIR_NE;
 else if ( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest" ) ) door = DIR_SW;
 else if ( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast" ) ) door = DIR_SE;

    return door;
}



int exit_name_bit( char *arg )
{
    int v=0;
    if ( strlen(arg) <= 3 ) return 0;
    if ( is_number(arg) ) return atoi(arg);
    if ( !str_infix( arg, "door" ) )              v|= EX_ISDOOR;
    if ( !str_infix( arg, "closed" ) )            v|= EX_CLOSED;
    if ( !str_infix( arg, "locked" ) )            v|= EX_LOCKED;
    if ( !str_infix( arg, "eat_key" ) )           v|= EX_EAT_KEY;
    if ( !str_infix( arg, "pickproof" ) )         v|= EX_PICKPROOF;
    if ( !str_infix( arg, "secret" ) )            v|= EX_SECRET;
    if ( !str_infix( arg, "jammed" ) )            v|= EX_JAMMED;
    if ( !str_infix( arg, "bashproof" ) )         v|= EX_BASHPROOF;
    if ( !str_infix( arg, "transparent" ) )       v|= EX_TRANSPARENT;
    if ( !str_infix( arg, "window"      ) )       v|= EX_WINDOW;
    if ( !str_infix( arg, "concealed"   ) )       v|= EX_CONCEALED;
    if ( !str_infix( arg, "nomove"      ) )       v|= EX_NOMOVE;
    return v;
}


char *exit_bit_name( int flag )
{
    static char buf[512];

    buf[0] = '\0';
    strcat( buf, ( flag & EX_ISDOOR      ) ? " door"        : "" );
    strcat( buf, ( flag & EX_CLOSED      ) ? " closed"      : "" );
    strcat( buf, ( flag & EX_LOCKED      ) ? " locked"      : "" );
    strcat( buf, ( flag & EX_EAT_KEY     ) ? " eat_key"     : "" );
    strcat( buf, ( flag & EX_PICKPROOF   ) ? " pickproof"   : "" );
    strcat( buf, ( flag & EX_SECRET      ) ? " secret"      : "" );
    strcat( buf, ( flag & EX_JAMMED      ) ? " jammed"      : "" );
    strcat( buf, ( flag & EX_BASHPROOF   ) ? " bashproof"   : "" );
    strcat( buf, ( flag & EX_TRANSPARENT ) ? " transparent" : "" );
    strcat( buf, ( flag & EX_WINDOW      ) ? " window"      : "" );
    strcat( buf, ( flag & EX_CONCEALED   ) ? " concealed"   : "" );
    strcat( buf, ( flag & EX_NOMOVE      ) ? " nomove"      : "" );
    return (buf[0] == '\0') ? buf : buf+1;;
}



int get_actor_sex_number( char* arg )
{
    if ( !str_cmp( arg, "neuter" ) )            return SEX_NEUTRAL;
    if ( !str_cmp( arg, "male" ) )              return SEX_MALE;
    if ( !str_cmp( arg, "female" ) )            return SEX_FEMALE;
    return SEX_NONE;
}


int get_script_type( char *arg )
{
    if ( is_number(arg) ) return atoi(arg);
    if ( !str_cmp( arg, "command" ) )     return TRIG_COMMAND;
    if ( !str_cmp( arg, "each"    ) )     return TRIG_EACH_PULSE;
    if ( !str_cmp( arg, "each_pulse" ) )  return TRIG_EACH_PULSE;
    if ( !str_cmp( arg, "combat"  ) )     return TRIG_COMBAT;
    if ( !str_cmp( arg, "tick"    ) )     return TRIG_TICK_PULSE;
    if ( !str_cmp( arg, "tick_pulse" ) )  return TRIG_TICK_PULSE;
    if ( !str_cmp( arg, "born"    ) )     return TRIG_BORN;
    if ( !str_cmp( arg, "gets"    ) )     return TRIG_GETS;
    if ( !str_cmp( arg, "say"     ) )     return TRIG_SAY;
    if ( !str_cmp( arg, "kills"   ) )     return TRIG_KILLS;
    if ( !str_cmp( arg, "dies"    ) )     return TRIG_DIES;
    if ( !str_cmp( arg, "enter"   ) )     return TRIG_ENTER;
    if ( !str_cmp( arg, "moves"   ) )     return TRIG_MOVES;
    if ( !str_cmp( arg, "looks"   ) )     return TRIG_LOOKS;
    if ( !str_cmp( arg, "alert"   ) )     return TRIG_ALERT;
    if ( !str_cmp( arg, "spell"   ) )     return TRIG_SPELL;
    if ( !str_cmp( arg, "never"   ) )     return TRIG_NEVER;
    return -1;
}


char *show_script_type( int script_type )
{
    switch ( script_type )
    {
        case TRIG_COMMAND:
         return "COMMAND (When a command is typed.)";
        case TRIG_EACH_PULSE:
         return "EACH_PULSE (Each pulse.)";
        case TRIG_COMBAT:
         return "COMBAT (At the beginning of new combat.)";
        case TRIG_TICK_PULSE:
         return "TICK_PULSE (A/P-Each tick.   S-When auto-cued.)";
        case TRIG_BORN:
         return "TRIG_BORN (A/P-Upon creation.  S-On startup.)";
        case TRIG_GETS:
         return "GETS (P-When taken.  A-When given. S-Dropped.)";
        case TRIG_SAY:
         return "SAY (Any scene-based communications.)";
        case TRIG_KILLS:
         return "KILLS (P/A-Kills something.)";
        case TRIG_DIES:
         return "DIES (A-Dies.  P/S-When someone dies nearby.)";
        case TRIG_ENTER:
         return "ENTER (Someone enters.)";
        case TRIG_MOVES:
         return "MOVES (Someone enters.)";
	case TRIG_LOOKS:
	 return "LOOKS (When looked at.)";
        case TRIG_ALERT:
         return "ALERT (Triggered by alert() function.)";
        case TRIG_SPELL:
         return "SPELL (When cast by a player.)";
        case TRIG_NEVER:
         return "NEVER (Script is turned off.)";
        default: return "UNKNOWN TRIGGER TYPE";
    }
}



char *shop_bit_name( int flag )
{
    char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';
    strcat( buf, ( flag & SHOP_TRADER     ) ? " trader"      : "" );
    strcat( buf, ( flag & SHOP_PEDDLER    ) ? " peddler"     : "" );
    strcat( buf, ( flag & SHOP_REPAIR     ) ? " repair"      : "" );
    strcat( buf, ( flag & SHOP_COMPONENTS ) ? " components"  : "" );
    return (buf[0] == '\0') ? buf : buf+1;;
}


int shop_name_bit( char *name )
{
    if ( !str_cmp( name, "trader"     ) ) return SHOP_TRADER;
    if ( !str_cmp( name, "peddler"    ) ) return SHOP_PEDDLER;
    if ( !str_cmp( name, "repair"     ) ) return SHOP_REPAIR;
    if ( !str_cmp( name, "components" ) ) return SHOP_COMPONENTS;
    return -1;
}


char *name_good_code( int good_code )
{
    switch ( good_code )
    {
        case T_CORN:  return "corn";
       case T_FLOUR:  return "flour";
      case T_FABRIC:  return "fabric";
        case T_HIDE:  return "hide";
         case T_DYE:  return "dye";
       case T_METAL:  return "metal";
         case T_ORE:  return "ore";
       case T_SPICE:  return "spice";
       case T_SUGAR:  return "sugar";
         case T_OIL:  return "oil";
            default:  return "unknown";
    }
}



char *name_stat_range( int stat )
{
    if ( stat > 18 ) return "extremely high";
    if ( stat > 16 ) return "very high";
    if ( stat > 14 ) return "high";
    if ( stat > 12 ) return "above average";
    if ( stat >  8 ) return "average";
    if ( stat >  6 ) return "below average";
    if ( stat >  4 ) return "low";
    if ( stat >  2 ) return "very low";
    return "extremely low";
}


