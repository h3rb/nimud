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
#include <time.h>

#include <zlib.h>

#include "nimud.h"
#include "skills.h"
#include "magic.h"


/*               DUMs (3)
Name             Address                 Numeric Address  Port  Status Notes
----------------------------------------------------------------------------
CanDUM II        cheetah.vlsi.uwaterloo.ca                2001  up 
                 cheetah.vlsi-cheetahnet.uwaterloo.ca 
                                         129.97.180.1 
                                         129.97.94.45 
DUM II           elektra.ling.umu.se     130.239.24.66    2001  up 
FranDUM          mousson.enst.fr         137.194.160.48   2001  up 
----------------------------------------------------------------------------
*/



/* 
* Bust a race. 
*/ 
const struct race_type race_table [MAX_RACE] = 
{ 
 {
   "human",   "the zengalli tribe", 
   13, 13, 13, 13, 13,  /* S I W D C */
   0,                  /* dbkey */          
   0,  
   5000,               /* Scene */ 
    50,                /* Max Skill */ 
     0,                /* Bits */ 
  SIZE_AVERAGE,        /* Size */ 
   125,                /* Base age */ 
    17,                /* Starting age */ 
 },

 {
   "dwarf",   "small size",
   15, 11, 13, 10, 15,  /* S I W D C */
   0,
   0,
     5000,                /* Scene */
    30,                /* Max Skill */
     0,                /* Bits */
  SIZE_SMALL,          /* Size */
   125,                /* Base age */
    17                 /* Starting age */
 },

 {
   "giant",   "gemmaster's workshop",
   14, 14, 14, 14, 14,  /* S I W D C */
   0,
   0,
    5000,                /* Scene */
    30,                /* Max Skill */
     0,                /* Bits */
  SIZE_LARGE,          /* Size */
   125,                /* Base age */
    17,                /* Starting age */
 },

 {
   "mutant",   "radioactive origin",
   15, 11, 13, 10, 15,  /* S I W D C */
   0,
   0,
   10200,                /* Scene */
    30,                /* Max Skill */
     0,                /* Bits */
  SIZE_LARGE,          /* Size */
   125,                /* Base age */
    17                 /* Starting age */
 },

 {
   "offspring",   "genetic manipulation",
   13, 14, 14, 15, 13,  /* S I W D C */
   0,
   0, 
   5000,                /* Scene */
    40,                /* Max Skill */
     0,                /* Bits */
  SIZE_PETITE,         /* Size */
   125,                /* Base age */
    17                 /* Starting age */
 },

 {
   "aborigine",   "tribal origins",
   13, 13, 14, 14, 13,  /* S I W D C */
    0,
    0,
   10100,                /* Scene */
    50,                /* Max Skill */
     0,                /* Bits */
  SIZE_AVERAGE,        /* Size */
   125,                /* Base age */
    17                 /* Starting age */
 },

 {
   "shi-ito",   "a long dead race",
   13, 15, 17, 13, 18,  /* S I W D C */
     0,   
     0,
    7710,                /* Scene */
    80,                /* Max Skill */
     0,                /* Bits */
  SIZE_AVERAGE,        /* Size */
   325,                /* Base age */
   100                 /* Starting age */
 },

 {
   "Muidar",   "evil ranks",
   13, 13, 13, 13, 13,  /* S I W D C */
    0,     /* Zengalli */
    0,        /* Argot */ 
    10300,                /* Scene */
    50,                /* Max Skill */
     0,                /* Bits */
  SIZE_AVERAGE,        /* Size */
   125,                /* Base age */
    17                 /* Starting age */
 },

 {
   "golem",   "ancient brood",
   18, 13, 13, 10, 18,  /* S I W D C */
    0,
    0,
   10310,                /* Scene */
    50,                /* Max Skill */
     0,                /* Bits */
  SIZE_HUGE,        /* Size */
   125,                /* Base age */
    17                 /* Starting age */
 },

 {
   "blood samurai",   "an ancient order",
   20, 13, 13, 8, 20,  /* S I W D C */ 
    0,
    0,
   10300,                /* Scene */
    50,                /* Max Skill */
     0,                /* Bits */
  SIZE_TITANIC,        /* Size */
   125,                /* Base age */
    17                 /* Starting age */
 },

 {
   "Baal",   "the ancient brood",
   20, 20, 20, 20, 20,  /* S I W D C */
    0,
    0, 
    10305,                /* Scene */
    10,                /* Max Skill */
     0,                /* Bits */
  SIZE_GARGANTUAN,     /* Size */
   125,                /* Base age */
    17                 /* Starting age */
 },


 {
   "Alsidi",   "peaceful fellowship",
   8, 15, 13, 25, 10,  /* S I W D C */
    0, 
    0,
   5000,                /* Scene */
    50,                /* Max Skill */
     0,                /* Bits */
  SIZE_MINUTE,         /* Size */
   125,                /* Base age */
    17                 /* Starting age */
 },


 {
   "Hilldwarf",   "short and friendly people",
   18, 13, 13, 13, 18,  /* S I W D C */
    0, 
    0,
   5000,                /* Scene */
    50,                /* Max Skill */
     0,                /* Bits */
  SIZE_SMALL,          /* Size */
   125,                /* Base age */
    17                 /* Starting age */
 },


 {
   "leprechuan",   "a misty Irish stream",
   8, 15, 13, 25, 10,  /* S I W D C */
   0, 
   0,
   5000,                /* Scene */
   50,                /* Max Skill */
   0,                /* Bits */
   SIZE_SMALL,         /* Size */
   150,                /* Base age */
   17                 /* Starting age */
 },


 {
   "cyborg",   "metal and flesh",
   8, 15, 13, 25, 10,  /* S I W D C */
   0, 
   0,
   5000,                /* Scene */
   50,                /* Max Skill */
   BONUS_INFRARED,                /* Bits */
   SIZE_AVERAGE,         /* Size */
   75,                /* Base age */
   17                 /* Starting age */
 },


 {
   "monoped",   "one legged",
   8, 15, 13, 25, 10,  /* S I W D C */
   0, 
   0,
   5000,                /* Scene */
   50,                /* Max Skill */
   0,                /* Bits */
   SIZE_MINUTE,         /* Size */
   125,                /* Base age */
   17                 /* Starting age */
 },
   
 {
   "medulfil",   "insectoid",
   8, 15, 13, 25, 10,  /* S I W D C */
   0, 
   0,
   5000,                /* Scene */
   50,                /* Max Skill */
   BONUS_INFRARED,                /* Bits */
   SIZE_MINUTE,         /* Size */
   12000,                /* Base age */
   17                 /* Starting age */
 }

};



char *   const  percent_hit [] =
{
    "slaughtered",
    "massacred",
    "bloodied",
    "beaten",
    "hurt",
    "slighted",
    "bruised",
    "scratched",
    "grazed",
    "fine",
    "fine"
};

char *   const  percent_tired [] =
{
    "exhausted",
    "beat",
    "tired",
    "weary",
    "haggard",
    "fatigued",
    "worked",
    "winded",
    "rested",
    "fresh",
    "fresh"
};

char *   const  percent_mana [] =
{
    "drained",
    "weakest",
    "weaker",
    "weak",
    "focused",
    "focused",
    "focused",
    "clear",
    "clear",
    "clear",
    "energized"
};
        



/*
 * Coin information
   int      multiplier;
   int      convert;
   int      weight;
   char *   long_name;
   char *   short_name;
 */
const struct coin_type coin_table[MAX_COIN] =
{
   {  10,    10000,   1, "platinum",            "pp"  },
   {  10,     1000,   1, "electrum",            "ep"  },
   {  10,      100,   1, "gold",                "gp"  },
   {  10,       10,   1, "silver",              "sp"  },
   {  10,        1,   1, "copper",              "cp"  }
};


#if defined(TRANSLATE)  
/*
 * See translate.c for more information
 * -locke
 */
const struct lingua_type lingua_table[MAX_LINGUA] =
{
    { "en",  "english"     },   /* Does not translate */
    { "AER", "arrernte"    },
    { "af",  "afrikaans"   },
    { "ah",  "amharic"     },
    { "ALH", "alawa"       },
    { "ar",  "arabic"      }, 
    { "ARE", "arrarnta"    }, 
    { "as",  "assamese"    }, 
    { "az",  "azerbaijani" }, 
    { "ban", "bali"        }, 
    { "be",  "belarusian"  },
    { "bg",  "bulgarian"   },
    { "bi",  "bislama"     },
    { "bn",  "bengali"     },
    { "bo",  "tibetan"     },
    { "bs",  "bosnian"     },
    { "ca",  "catalan"     },
    { "ce",  "chechen"     },
    { "cs",  "czech"       },
    { "cy",  "welsh"       },
    { "da",  "danish"      }, /* 20 */
    { "de",  "german"      },
    { "el",  "greek"       },
    { "eo",  "esperanto"   },
    { "es",  "spanish"     },
    { "et",  "estonian"    },
    { "eu",  "basque"      },
/*  { "fa",  "farsi"       }, */    /* Sadly, not supported. */
    { "eu",  "basque"      },       /* Limited implementation */
    { "fi",  "finnish"     },
    { "fj",  "fijian"      },
    { "fo",  "faroese"     },
    { "fr",  "french"      },
    { "ga",  "irish"       },
    { "GBZ", "dari"        },
    { "gl",  "galician"    },
    { "grc", "ancient greek" },
    { "gu",  "gujarati"    },
    { "haw", "hawaiian"    },
/*  { "he",  "hebrew"      }, */    /* Sadly, not supported. */
    { "hi",  "hindi"       },
    { "hr",  "croatian"    }, /* 40 */
    { "hu",  "hungarian"   },
    { "hy",  "armenian"    },
    { "ia",  "interlingua" },
    { "id",  "indonesian"  },
    { "is",  "icelandic"   },
    { "it",  "italian"     },
/*  { "ja",  "japanese"    }, */    /* Sadly, not supported. */
    { "jw",  "javanese"    },
    { "ka",  "georgian"    },
    { "kk",  "kazakh"      },
    { "kl",  "inuktitut"   },
    { "km",  "khmer"       },
    { "kn",  "kannada"     },
    { "ko",  "korean"      },
    { "ks",  "kashmiri"    },
    { "ky",  "kirghiz"     },
    { "KYL", "kabyle"      },
    { "la",  "latin"       },
    { "lb",  "luxembourgish" },
    { "lo",  "lao"         }, /* 60 */
    { "lt",  "lithuanian"  },
    { "lv",  "latvian"     },
    { "mi",  "maori"       },
    { "mk",  "macedonian"  },
    { "ml",  "malayalam"   },
    { "mr",  "marathi"     },
    { "ms",  "malay"       }, 
    { "mn",  "mongolian"   },
    { "mt",  "maltese"     },
    { "MWP", "kala lagaw ya" },
    { "my",  "myanmar"     },
    { "ne",  "nepali"      },
    { "nl",  "dutch"       },
    { "no",  "norwegian"   },
    { "oc",  "occitan"     },
    { "or",  "oriya"       },
    { "pa",  "punjabi"     },
    { "PJT", "pitjantjatjara" },
    { "pl",  "polish"      }, /* 80 */
    { "ps",  "pashto"      },
    { "pt",  "portuguese"  },
    { "rm",  "romansch"    },
    { "ro",  "romanian"    },
    { "ROP", "kriol"       },
    { "ru",  "russian"     },
    { "sa",  "sanskrit"    },
    { "se",  "saami"       },
    { "SHI", "tachelhit"   },
    { "si",  "sinhala"     },
    { "sk",  "slovak"      },
    { "sl",  "slovenian"   },
    { "sm",  "samoan"      },
    { "sq",  "albanian"    },
    { "sr",  "serbian"     },
    { "sv",  "swedish"     }, 
    { "sw",  "swahili"     },
    { "syr", "syriac"      },
    { "ta",  "tamil"       }, /* 100 */
    { "TCS", "torres strait cre" },
    { "te",  "teluga"      },
    { "tg",  "tajiki"      },
    { "th",  "thai"        },
    { "tk",  "turkmen"     },
    { "tl",  "tagalog"     },
    { "ton", "tongan"      },
    { "tpi", "tok pisin"   },
    { "tr",  "turkish"     },
    { "tvl", "tuvaluan"    },
    { "ty",  "tahitian"    },
    { "TZM", "tamazight"   },
    { "ug",  "uyghur"      },
    { "uk",  "ukrainian"   },
    { "ULK", "meriam"      },
    { "ur",  "urdu"        },
    { "uz",  "uzbek"       },
/*  { "vi",  "vietnamese"  }, */    /* Sadly, not supported. */
    { "WBP", "warlpiri"    },
    { "xh",  "xhosa"       }, /* 120 */
    { "yi",  "yiddish"     },
    { "zh",  "chinese"     },
    { "zu",  "zulu"        }  /* 123 */
};
#endif




/*
 * Languages.
 */
const struct   lang_type  lang_table [MAX_LANGUAGE] =
{
    { "stark",    101   },     /* Common             */
    { "argot",    102   },     /* Thieves            */
    { "french",   103   },     /* Thilonenne (south) */
    { "russian",  104   },     /* Thilonenne (west)  */
    { "japanese", 105   },     /* Trioskenne         */
    { "german",   106   },     /* Dwarven            */
    { "gaelic",   107   },     /* Heveti             */
    { "imarind",  108   },     /* Ancient Language   */
    { "brood",    109   },
    { "alsidi",   110   },   
};


/* Help class table */
const struct help_class help_class_table [MAX_HELP_CLASS] =
{
   { "General",      "general.hlp",     "General topics"                      },
   { "Movement",     "move.hlp",        "Getting around"                      },
   { "Inventory",    "inventory.hlp",   "Inventory and equipment management"  },
   { "Status",       "status.hlp",      "Character stats and informations"    },
   { "Skills",       "skills.hlp",      "Upgrading and obtaining more skills" },
   { "Combat",       "combat.hlp",      "Attacking and surviving"             },
   { "Communication","comms.hlp",       "Communicating with other players"    },
   { "Story",        "story.hlp",       "Background info for the MUD"         },
   { "Script",       "script.hlp",      "Scripting language helps"            },
   { "Immortal",     "immref.hlp",      "Immortal specific helps"             },
   { "Newbie",        "newbie.hlp",       "Newbie helps"                         },
   { "Intermud",     "imc.hlp",         "Intermud Connection Protocol"        },
   { "Tips",         "tips.hlp",        "Helpful Tips"                        }
};


/*
 *  Attack damage type and string.
 */
const   struct  attack_type attack_table [] =         /* CHANGE MAX_ATTACK */
{
    { "hit",      TYPE_HIT,    604,   NULL              },  /*  0 */
    { "slice",    TYPE_SLASH,  603,   NULL              },  /*  1 */
    { "stab",     TYPE_PIERCE, 602,   NULL              },
    { "slash",    TYPE_SLASH,  601,   NULL              },
    { "whip",     TYPE_WHIP,   605,   NULL              },
    { "cleave",   TYPE_SLASH,  603,   NULL              },  /*  5 */
    { "vorpal",   TYPE_SLASH,  601,   hit_vorpal        },
    { "pound",    TYPE_POUND,  604,   NULL              },
    { "pierce",   TYPE_PIERCE, 602,   NULL              },
    { "lash",     TYPE_WHIP,   605,   NULL              },
    { "claw",     TYPE_SLASH,  601,   NULL              },  /* 10 */
    { "blast",    TYPE_EXPLODE,609,   NULL              },
    { "crush",    TYPE_POUND,  609,   NULL              },
    { "gore",     TYPE_PIERCE, 602,   NULL              },
    { "bite",     TYPE_PIERCE, 602,   NULL              },
    { "suction",  TYPE_SUCTION,609,   hit_suck_disarm   },  /* 15 */
    { "fire",     TYPE_EXPLODE,609,   hit_fire          },
    { "cold",     TYPE_HIT,    609,   hit_cold          },
    { "rot",      TYPE_HIT,    609,   hit_rot           },
    { "lightning",TYPE_HIT,    609,   hit_lightning     },
    { "acid",     TYPE_HIT,    609,   hit_acid          },  /* 20 */
    { "kick",     TYPE_HIT,    609,   hit_kick          },
    { "poison",   TYPE_PIERCE, 602,   hit_poison        },
};



/*
 * Attribute bonus tables.
 */
const   struct  str_app_type    str_app     [26]        =
{
 /*  +Hit +Dam Crr Wei */
    { -5, -4,   0,  0 },  /* 0  */
    { -5, -4,   3,  1 },  /* 1  */
    { -3, -2,   3,  2 },
    { -3, -1,  10,  3 },  /* 3  */
    { -2, -1,  25,  4 },
    { -2, -1,  55,  5 },  /* 5  */
    { -1,  0,  80,  6 },
    { -1,  0,  90,  7 },
    {  0,  0, 100,  8 },
    {  0,  0, 100,  9 },
    {  0,  0, 115, 10 }, /* 10  */
    {  0,  0, 115, 11 },
    {  0,  0, 140, 12 },
    {  0,  0, 140, 13 }, /* 13  */
    {  0,  1, 170, 14 },
    {  1,  1, 170, 15 }, /* 15  */
    {  1,  2, 195, 16 },
    {  2,  3, 220, 22 },
    {  2,  4, 250, 25 }, /* 18  */
    {  3,  5, 400, 30 },
    {  3,  6, 500, 35 }, /* 20  */
    {  4,  7, 600, 40 },
    {  5,  7, 700, 45 },
    {  6,  8, 800, 50 },
    {  8, 10, 900, 55 },
    { 10, 12, 999, 60 }  /* 25   */
};



const   struct  int_app_type    int_app     [26]        =
{  /* SLv  DB   Mana */
    {  0,  72, -90 },     /*  0 */
    {  0,  60, -80 },     /*  1 */
    {  0,  48, -70 },
    {  0,  36, -60 },     /*  3 */
    {  0,  36, -50 },
    {  1,  24, -40 },     /*  5 */
    {  1,  24, -30 },
    {  2,  12, -20 },
    {  3,  12, -10 },
    {  4,   6,   0 },
    {  5,   0,   0 },     /* 10 */
    {  6,   0,   0 },
    {  7,   0,   0 },
    {  8,   0,  10 },     /* 13 */
    {  9,   0,  20 },
    { 10,   0,  30 },     /* 15 */
    { 11,  -6,  40 },
    { 12,  -6,  50 },
    { 15, -12,  60 },     /* 18 */
    { 17, -12,  70 },
    { 20, -12,  80 },     /* 20 */
    { 23, -12,  90 },
    { 26, -12, 100 },
    { 30, -24, 150 },
    { 35, -48, 200 },
    { 40, -72, 300 }      /* 25 */
};



const   struct  wis_app_type    wis_app     [26]        =
{  /* SLv  DB */
    {  0,  48 },     /*  0 */
    {  0,  36 },     /*  1 */
    {  0,  24 },
    {  0,  12 },     /*  3 */
    {  0,  12 },
    {  1,   6 },     /*  5 */
    {  1,   6 },
    {  2,   6 },
    {  3,   6 },
    {  4,   3 },
    {  5,   0 },     /* 10 */
    {  6,   0 },
    {  7,   0 },
    {  8,   0 },     /* 13 */
    {  9,   0 },
    { 10,   0 },     /* 15 */
    { 11,  -3 },
    { 12,  -3 },
    { 15,  -6 },     /* 18 */
    { 17,  -6 },
    { 20,  -6 },     /* 20 */
    { 23,  -6 },
    { 26,  -6 },
    { 30, -12 },
    { 35, -24 },
    { 40, -36 }      /* 25 */
};



const   struct  dex_app_type    dex_app     [26]        =
{
    {   60 },   /* 0 */
    {   50 },   /* 1 */
    {   50 },
    {   40 },
    {   30 },
    {   20 },   /* 5 */
    {   10 },
    {    0 },
    {    0 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    0 },
    { - 10 },   /* 15 */
    { - 15 },
    { - 20 },
    { - 30 },
    { - 40 },
    { - 50 },   /* 20 */
    { - 60 },
    { - 75 },
    { - 90 },
    { -105 },
    { -120 }    /* 25 */
};



const   struct  con_app_type    con_app     [26]        =
{
    { -80, 20, 20 },   /*  0 */
    { -70, 25, 25 },   /*  1 */
    { -60, 30, 30 },
    { -50, 35, 35 },   /*  3 */
    { -40, 40, 40 },
    { -35, 45, 45 },   /*  5 */
    { -30, 50, 50 },
    { -25, 55, 55 },
    { -20, 60, 60 },
    { -15, 65, 65 },
    { -10, 70, 70 },   /* 10 */
    {  -5, 75, 75 },
    {   0, 80, 80 },
    {   5, 85, 85 },
    {  10, 88, 88 },
    {  15, 90, 90 },   /* 15 */
    {  20, 95, 95 },
    {  25, 97, 97 },
    {  30, 99, 99 },   /* 18 */
    {  35, 99, 99 },
    {  40, 99, 99 },   /* 20 */
    {  50, 99, 99 },
    {  60, 99, 99 },
    {  70, 99, 99 },
    {  80, 99, 99 },
    { 100, 99, 99 }    /* 25 */
};



/*
 * Liquid properties.
 * Used in world.prop.
 */
const	struct	liq_type	liq_table	[LIQ_MAX]	=
{                                                 /* D  F  T */
    { "water",          "clear",                  {  0, 1, 10 } },  /*  0 */
    { "beer",           "amber",                  {  1, 1,  5 } },  /*  1 */
    { "wine",           "rose",                   {  1, 0,  5 } },  /*  2 */
    { "ale",            "brown",                  {  1, 1,  5 } },  /*  3 */
    { "brandy",         "dark",                   {  2, 0,  5 } },  /*  4 */

    { "dark ale",       "black",                  {  2, 0,  4 } },  /*  5 */
    { "port",           "deep red",               {  3, 0,  0 } },  /*  6 */
    { "blueberry wine", "azure",                  {  2, 0,  3 } },  /*  7 */
    { "salt water",     "clear",                  {  0, 1, -2 } },  /*  8 */
    { "bitters",        "dark grey",              { 20, 0,  5 } },  /*  9 */

    { "milk",           "white",                  {  0, 3,  6 } },  /* 10 */

    { "tea",            "tan",                    {  0, 0,  6 } },  /* 11 */

    { "blood",          "red",                    {  0, 2, -1 } },  /* 12 */
    { "blessed water",  "crystal clear",          {-10,10, 10 } },  /* 13 */

    { "lemonade",       "translucent yellow",     {  0, 0,  8 } },  /* 14 */
    { "broth",          "oily, tan",              {  0, 6,  2 } },  /* 15 */

    { "mead",           "thick, dark",            {  3, 2,  1 } },  /* 16 */
    { "lamp oil",       "waxy, yellow",           {  0, 0,  0 } },  /* 17 */

    { "ginseng tea",    "pale tan",               {  0, 0,  6 } },  /* 18 */
    { "orange tea",     "dark orange",            {  0, 0,  6 } },  /* 19 */
    { "cinnamon tea",   "speckled tan",           {  0, 0,  6 } },  /* 20 */
    { "spiced tea",     "reddish tan",            {  0, 0,  6 } },  /* 21 */
    { "salt tea",       "bubbling tan",           {  4, 0,  6 } },  /* 22 */
    { "kio sap tea",    "greenish tan",           {  4, 0,  6 } },  /* 23 */

    { "apple cider",    "aromatic brown",         {  2, 0,  8 } },  /* 24 */
    { "cherry cider",   "rich red",               {  1, 0,  8 } },  /* 25 */
    { "hard cider",     "bitter, brown",          {  5, 0,  8 } },  /* 26 */

    { "mullinspice cider", "speckled brown",      {  0, 0,  2 } },  /* 27 */
    { "mullinspice tea",   "speckled yellow-tan", {  0, 0,  6 } },  /* 28 */
    { "mullinspice brandy", "dark, speckled",     {  2, 0,  6 } },  /* 29 */

    { "peach wine",     "clear, orange",          {  2, 0,  3 } },  /* 30 */
    { "dandilion wine", "clear, yellow",          {  1, 0,  4 } },  /* 31 */
    { "apricot wine",   "clear, orange-red",      {  3, 0,  5 } },  /* 32 */

    { "mud",            "thick, gritty, brown",   {  0, 1,-10 } },  /* 33 */
};


/*
 * Spell components.
 */
const struct   comp_type   comp_table  [] =
{
    /* Metals */
    {  "brass cylinder",   "a",                           C_SOLID,      50 },
    {  "bronze disk",      "a",                           C_SOLID,      50 },
    {  "copper square",    "a",                           C_SOLID,       2 },
    {  "copper wire",      "a",                           C_WIRE,       10 },
    {  "gold bell",        "a",                           C_SOLID,     500 },
    {  "gold caltrops",    "a pair of",                   C_SOLID,     300 },
    {  "gold chunk",       "a",                           C_SOLID,     500 },
    {  "gold dust",        "a small bit of",              C_POWDER,     20 },
    {  "gold needles",     "a set of",                    C_SOLID,    1500 },
    {  "gold powder",      "a pinch of",                  C_POWDER,    100 },
    {  "gold tissue",      "a",                           C_MEMBRANE, 1000 },
    {  "gold wire",        "a length of",                 C_WIRE,      300 },
    {  "iron chunk",       "an",                          C_SOLID,       5 },
    {  "iron filings",     "an amount of",                C_POWDER,      2 },
    {  "iron powder",      "a dash of",                   C_POWDER,     20 },
    {  "iron rod",         "an",                          C_ROD,        30 },
    {  "lead ball",        "a",                           C_SOLID,       6 },
    {  "lead chunk",       "a",                           C_SOLID,       8 },
    {  "mercury",          "an ounce of",                 C_LIQUID,     10 },
    {  "chipped mica",     "a few bits of",               C_POWDER,     10 },
    {  "ground mica",      "a dusting of",                C_POWDER,      8 },
    {  "platinum chunk",   "a",                           C_SOLID,    1500 },
    {  "platinum coin",    "a defaced",                   C_SOLID,     500 },
    {  "platinum die",     "a six-sided",                 C_SOLID,    3500 },
    {  "platinum mirror",  "a shiny",                     C_SOLID,    4000 },
    {  "platinum tissue",  "a",                           C_MEMBRANE, 1000 },
    {  "platinum wire",    "a length of",                 C_WIRE,      800 },
    {  "silver pin",       "a",                           C_ROD,        50 },
    {  "silver powder",    "a bit of",                    C_POWDER,     10 },
    {  "silver wire",      "a length of",                 C_WIRE,       40 },
    {  "tin square",       "a",                           C_SOLID,       5 },
    {  "zinc square",      "a",                           C_SOLID,      10 },

    /* Liquids */
    {  "acid",             "a vial of",                   C_LIQUID,    100 },
    {  "alcohol",          "a bottle of pure",            C_LIQUID,     10 },
    {  "attar of roses",   "a capful of",                 C_LIQUID,  20000 },
    {  "hickory oil",      "a spoonful of",               C_LIQUID,    100 },
    {  "ink",              "a squirt of",                 C_LIQUID,     50 },
    {  "molasses",         "a thick glob of",             C_GELATIN,    10 },
    {  "flammable oil",    "a flask of",                  C_LIQUID,     60 },
    {  "nonflammable oil", "a flask of",                  C_LIQUID,     60 },
    {  "spiced oil",       "a dash of",                   C_LIQUID,    100 },
    {  "sweet oil",        "a tasting of",                C_LIQUID,    100 },
    {  "rainwater",        "a half-cask of",              C_LIQUID,      3 },
    {  "vinegar",          "a bottle of",                 C_LIQUID,     50 },
    {  "distilled water",  "a hand keg of",               C_LIQUID,     10 },

    /* Minerals */
    {  "agate",            "an uncut",                    C_GEM,       100 },
    {  "alkaline salt",    "a pinch of",                  C_POWDER,     10 },
    {  "alum",             "a bit of",                    C_POWDER,      5 },
    {  "amber block",      "an",                          C_SOLID,      80 },
    {  "amber powder",     "a dash of",                   C_POWDER,    100 },
    {  "amber rod",        "an",                          C_ROD,      1000 },
    {  "bitumen",          "a handful of dry",            C_POWDER,    100 },
    {  "blue vitriol",     "a capful of",                 C_LIQUID,     20 },
    {  "borax paste",      "a dab of",                    C_LIQUID,     10 },
    {  "borax stone",      "a",                           C_SOLID,      10 },
    {  "bromine salt",     "a pinch of",                  C_POWDER,    100 },
    {  "amorphous carbon", "a glob of",                   C_GELATIN,   100 },
    {  "graphite",         "a soft piece of",             C_POWDER,    300 },
    {  "chalk",            "some bits of",                C_POWDER,      1 },
    {  "charcoal",         "the soot from",               C_POWDER,    100 },
    {  "fired clay sphere","a",                           C_SOLID,       1 },
    {  "raw clay",         "a slice of",                  C_SOLID,      10 },
    {  "coal chunk",       "a",                           C_SOLID,     100 },
    {  "coal dust",        "a handful of",                C_POWDER,     80 },
    {  "crystal bead",     "a",                           C_SOLID,     700 },
    {  "crystal rod",      "a",                           C_ROD,      1500 },
    {  "crystal square",   "a",                           C_SOLID,     300 },
    {  "crystal sphere",   "a",                           C_SOLID,    2000 },
    {  "diamond",          "an uncut",                    C_GEM,     15000 },
    {  "emerald",          "an uncut",                    C_GEM,     14000 },
    {  "feldspar chunk",   "a",                           C_SOLID,     100 },
    {  "flint",            "a chip of",                   C_SOLID,      10 },
    {  "glass bead",       "a",                           C_SOLID,       2 },
    {  "glass cone",       "a",                           C_SOLID,     500 },
    {  "glass rod",        "a",                           C_ROD,       500 },
    {  "glass sheet",      "a",                           C_SOLID,     200 },
    {  "glass sphere",     "a",                           C_SOLID,     500 },
    {  "granite block",    "a",                           C_SOLID,     200 },
    {  "granite dust",     "a stone's worth of",          C_POWDER,    100 },
    {  "gypsum",           "a",                           C_SOLID,      10 },
    {  "iron pyrite",      "a",                           C_SOLID,     100 },
    {  "lime powder",      "a bit of",                    C_POWDER,     20 },
    {  "litharge",         "a",                           C_SOLID,      30 },
    {  "black marble chunk",     "a",                     C_SOLID,      50 },
    {  "purbeck marble chunk",   "a",                     C_SOLID,      80 },
    {  "mineral sphere",   "a",                           C_SOLID,    1000 },
    {  "mineral prism",    "a",                           C_SOLID,     700 },
    {  "powdered mortar",  "a handful of",                C_SOLID,      50 },
    {  "obsidian chunk",   "a",                           C_SOLID,      30 },
    {  "obsidian cylinder",  "a",                         C_SOLID,      50 },
    {  "obsidian sphere",  "a",                           C_SOLID,      50 },
    {  "phosphorous",      "some",                        C_SOLID,    1000 },
    {  "pitch",            "some",                        C_POWDER,      5 },
    {  "clear quartz",     "a ",                          C_SOLID,      10 },
    {  "realgar",          "a chunk of",                  C_SOLID,      10 },
    {  "rough quartz",     "a",                           C_SOLID,       5 },
    {  "quartz powder",    "carefully refined",           C_POWDER,      3 },
    {  "ruby",             "an uncut",                    C_GEM,     15000 },
    {  "salt block",       "a",                           C_SOLID,      10 },
    {  "granular salt",    "some",                        C_POWDER,     50 },
    {  "sapphire",         "an uncut",                    C_GEM,     14000 },
    {  "fine sand",        "a handful of",                C_POWDER,     10 },
    {  "soot",             "a pouch of woodstoked",       C_POWDER,      2 },
    {  "stalactite shard", "a",                           C_SOLID,       4 },
    {  "tiny stalactite",  "a",                           C_SOLID,      10 },
    {  "sulfur",           "a",                           C_SOLID,     100 },
    {  "talc",             "a",                           C_SOLID,      70 },
    {  "polished turquoise", "a",                         C_SOLID,     500 },
    {  "raw turquoise",    "a",                           C_SOLID,      10 },
    {  "volcanic ash",     "a small pouchful of",         C_POWDER,      5 },

    /* Animal products */
    {  "bat fur",          "a bit of",                    C_SOLID,    3000 },
    {  "bat guano",        "a",                           C_SOLID,     200 },
    {  "beeswax",          "a tin of",                    C_GELATIN,    10 },
    {  "bee wing",         "a",                           C_SOLID,       3 },
    {  "bladder",          "a",                           C_SOLID,       5 },
    {  "powdered bone",    "a pouch of",                  C_POWDER,     10 },
    {  "bone chunk",       "a",                           C_SOLID,     150 },
    {  "blood",            "a jar of bovine",             C_LIQUID,     10 },
    {  "butterfly cocoon", "a",                           C_SOLID,      50 },
    {  "moth cocoon",      "a",                           C_SOLID,       3 },
    {  "wasp cocoon",      "a",                           C_SOLID,       4 },
    {  "coral chunk",      "a",                           C_SOLID,      10 },
    {  "eelskin",          "a large piece of",            C_MEMBRANE,   20 },
    {  "crushed eggshell", "a",                           C_POWDER,      3 },
    {  "eggshell shard",   "a",                           C_SOLID,       3 },
    {  "whole empty eggshell",  "a",                      C_SOLID,       3 },
    {  "fat",              "a glob of",                   C_LIQUID,     40 },
    {  "chicken tail feather",  "a",                      C_SOLID,       5 },
    {  "eagle feather",    "a",                           C_SOLID,    1000 },
    {  "hawk feather",     "a",                           C_SOLID,     500 },
    {  "owl feather",      "a",                           C_SOLID,    1000 },
    {  "raven feather",    "a",                           C_SOLID,     500 },
    {  "white feather",    "a",                           C_SOLID,      10 },
    {  "wing feather",     "a",                           C_SOLID,      50 },
    {  "vulture tail",     "a",                           C_SOLID,     100 },
    {  "dead firefly",     "a",                           C_SOLID,      10 },
    {  "fleece",           "some",                        C_MEMBRANE,    5 },
    {  "fish scale",       "a",                           C_SOLID,      20 },
    {  "frog leg",         "a",                           C_SOLID,       4 },
    {  "glowworm tail",    "a",                           C_SOLID,      10 },
    {  "dead glowworm",    "a",                           C_SOLID,      10 },
    {  "ape hair",         "a",                           C_SOLID,     300 },
    {  "bear hair",        "a",                           C_SOLID,     100 },
    {  "camel hair",       "a",                           C_SOLID,     500 },
    {  "draft horse hair", "a",                           C_SOLID,     300 },
    {  "child hair",       "a",                           C_SOLID,       5 },
    {  "ox hair",          "a",                           C_SOLID,      10 },
    {  "war horse hair",   "a",                           C_SOLID,     100 },
    {  "honeycomb",        "a",                           C_SOLID,     100 },
    {  "centipede leg",    "a",                           C_SOLID,       5 },
    {  "grasshopper hind leg",  "a",                      C_SOLID,      10 },
    {  "millipede leg",    "a",                           C_SOLID,       5 },
    {  "spider leg",       "a",                           C_SOLID,       2 },
    {  "leather strip",    "a",                           C_WIRE,        7 },
    {  "leather thong",    "a",                           C_SOLID,       5 },
    {  "lime",             "a whole",                     C_SOLID,      10 },
    {  "lye",              "some",                        C_POWDER,     10 },
    {  "milk fat",         "the curds of",                C_POWDER,     10 },
    {  "mouse whiskers",   "a",                           C_SOLID,       3 },
    {  "musk",             "a vial of",                   C_LIQUID,     40 },
    {  "powdered ox hoof", "a",                           C_POWDER,     50 },
    {  "pork rind",        "a",                           C_SOLID,       3 },
    {  "python bile",      "a drop of",                   C_LIQUID,    300 },
    {  "rat tail",         "a",                           C_SOLID,       7 },
    {  "rat whisker",      "a",                           C_SOLID,       5 },
    {  "seashell shard",   "a",                           C_SOLID,       5 },
    {  "seashell",         "a",                           C_SOLID,      20 },
    {  "skunk hair",       "a",                           C_SOLID,       5 },
    {  "snake skin",       "a",                           C_MEMBRANE,    4 },
    {  "silkworm egg",     "a",                           C_SOLID,     500 },
    {  "snake scale",      "a",                           C_SOLID,      20 },
    {  "sponge",           "a",                           C_SOLID,      10 },
    {  "tallow",           "a block of",                  C_SOLID,      30 },
    {  "tortoise shell",   "a",                           C_SOLID,      70 },
    {  "vellum",           "a large sheet of",            C_MEMBRANE,  500 },
    {  "white wax",        "some hot",                    C_LIQUID,     20 },
    {  "wool strand",      "a",                           C_WIRE,       50 },

    /* Plant Products */
    {  "acorn",            "a",                           C_SOLID,       3 },
    {  "dried amaryllis blossom",  "a",                   C_SOLID,      10 },
    {  "amaryllis bulb",   "a",                           C_SOLID,       5 },
    {  "amaryllis hypoxis bulb", "a",                     C_SOLID,      30 },
    {  "dried amaryllis stalk",  "a",                     C_SOLID,      30 },
    {  "apple blossom petal",    "a",                     C_SOLID,       3 },
    {  "arsenic",          "some",                        C_POWDER,      1 },
    {  "asafoetida",       "a packet of",                 C_POWDER,     50 },
    {  "aster seed",       "an",                          C_SOLID,       5 },
    {  "apple bark",       "some",                        C_SOLID,       6 },
    {  "ash bark",         "some",                        C_SOLID,       7 },
    {  "beech bark",       "some",                        C_SOLID,       6 },
    {  "cherry bark",      "some",                        C_SOLID,       7 },
    {  "elm bark",         "some",                        C_SOLID,       4 },
    {  "maple bark",       "some",                        C_SOLID,       3 },
    {  "oak bark",         "some",                        C_SOLID,       4 },
    {  "pine bark",        "some",                        C_SOLID,       3 },
    {  "spruce bark",      "some",                        C_SOLID,       5 },
    {  "sumac bark",       "some",                        C_SOLID,       3 },
    {  "walnut bark",      "some",                        C_SOLID,       8 },
    {  "burr",             "a",                           C_SOLID,       2 },
    {  "camphor resin",    "a packet of",                 C_POWDER,     10 },
    {  "dried carrot",     "a",                           C_SOLID,       2 },
    {  "cassia bark",      "some",                        C_SOLID,      60 },
    {  "cassia bud",       "a",                           C_SOLID,      20 },
    {  "castor beans",     "some",                        C_SOLID,       8 },
    {  "citric acid",      "a vial of",                   C_LIQUID,      8 },
    {  "cork",             "a",                           C_SOLID,      10 },
    {  "powdered corn extract",  "a tin of",              C_POWDER,     20 },
    {  "cotton mesh",      "a",                           C_MEMBRANE,   40 },
    {  "white felt",       "some",                        C_MEMBRANE,   10 },
    {  "fern frond",       "a",                           C_SOLID,       2 }, 
    {  "flower",           "a",                           C_SOLID,      10 },
    {  "foxfire fungus",   "a",                           C_SOLID,      20 },
    {  "galbanum",         "a packet of",                 C_POWDER,     50 },
    {  "garlic bud",       "a",                           C_SOLID,       5 },
    {  "garlic clove",     "a",                           C_SOLID,       3 },
    {  "ginger",           "a hand of",                   C_SOLID,      10 },
    {  "ginseng",          "some",                        C_POWDER,     30 },
    {  "holly berries",    "some",                        C_SOLID,      10 },
    {  "holly sprig",      "a",                           C_SOLID,       3 },
    {  "stick of incense", "a",                           C_SOLID,      50 },
    {  "ash leaf",         "an",                          C_SOLID,       3 },
    {  "oak leaf",         "a",                           C_SOLID,       2 },
    {  "poison ivy leaf",  "a",                           C_SOLID,       2 },
    {  "poison oak leaf",  "a",                           C_SOLID,       3 },
    {  "poison sumac leaf",  "a",                         C_SOLID,       3 },
    {  "raspberry leaf",   "a",                           C_SOLID,       7 },
    {  "rose leaf",        "a",                           C_SOLID,       3 },
    {  "rhubarb leaf",     "a",                           C_SOLID,       3 },
    {  "shamrock leaf",    "a",                           C_SOLID,       2 },
    {  "skunk cabbage leaf",  "a",                        C_SOLID,       4 },
    {  "stinging nettle leaf",  "a",                      C_SOLID,       3 },
    {  "sumac leaf",       "a",                           C_SOLID,       3 },
    {  "yew leaf",         "a",                           C_SOLID,       4 },
    {  "ground leek",      "a",                           C_POWDER,      7 },
    {  "legume seeds",     "a bunch of",                  C_POWDER,      2 },
    {  "gold-wefted linen cloth",   "a",                  C_MEMBRANE, 1000 },
    {  "lotus flower",     "a",                           C_SOLID,       4 },
    {  "lotus root",       "a",                           C_SOLID,      30 },
    {  "dry crushed marigold",  "a",                      C_POWDER,      7 },
    {  "moonseed",         "a",                           C_SOLID,     100 },
    {  "green moss",       "a dusk of",                   C_SOLID,       2 },
    {  "phosphorescent moss",  "a dusk of",               C_SOLID,       5 },
    {  "mushroom spores",  "a pouch of",                  C_POWDER,    100 },
    {  "mushroom",         "a",                           C_SOLID,       5 },
    {  "nut shell",        "a",                           C_SOLID,       2 },
    {  "pine bark",        "some",                        C_SOLID,       3 },
    {  "pine cone",        "a",                           C_SOLID,       3 },
    {  "pine needles",     "a few",                       C_POWDER,      4 },
    {  "pine sprig",       "a",                           C_SOLID,       2 },
    {  "pine tar",         "a glob of",                   C_GELATIN,    10 },
    {  "peach pit",        "a",                           C_SOLID,       1 },
    {  "reed stalk",       "a",                           C_SOLID,       5 },
    {  "resin",            "a packet of",                 C_POWDER,      4 },
    {  "rose petal",       "a",                           C_SOLID,       5 },
    {  "rose thorn",       "a",                           C_SOLID,       5 },
    {  "rhubarb root",     "a",                           C_SOLID,       8 },
    {  "dry seaweed",      "a",                           C_SOLID,      20 },
    {  "shallot bulb",     "a",                           C_SOLID,       5 },
    {  "soda ash",         "some",                        C_POWDER,     30 },
    {  "stargrass",        "a dusk of",                   C_SOLID,       5 },
    {  "tacamahac",        "a vial of",                   C_POWDER,     10 },
    {  "thistledown",      "a tuff of",                   C_SOLID,       1 },
    {  "thorns",           "a handful of",                C_POWDER,      2 },
    {  "tree sap",         "a blob of",                   C_GELATIN,   100 },
    {  "vermilion",        "a",                           C_SOLID,      10 },
    {  "willow bark",      "some",                        C_SOLID,       3 },
    {  "wolfsbane",        "a sprig of",                  C_SOLID,      30 },
    {  "wynchwood",        "a piece of",                  C_SOLID,      10 },
    {  "yam",              "a",                           C_SOLID,       8 },
    {  "zedoary",          "some",                        C_POWDER,     30 },
    
    /* Aromatics and Spices */
    {  "aloeswood chips",  "some",                        C_POWDER,    100 },
    {  "ambergris",        "some",                        C_POWDER,    100 },
    {  "benzoin",          "some",                        C_POWDER,    100 },
    {  "betony",           "some",                        C_POWDER,    300 },
    {  "calmus",           "some",                        C_POWDER,    100 },
    {  "civet",            "some",                        C_POWDER,   1000 },
    {  "emeli",            "some",                        C_POWDER,    200 },
    {  "frankincense",     "some",                        C_POWDER,   1000 },
    {  "myrrh",            "some",                        C_LIQUID,   1000 },
    {  "onycha",           "some",                        C_LIQUID,    300 },
    {  "dried orchid",     "some",                        C_POWDER,    300 },
    {  "patchouli",        "some",                        C_POWDER,     10 },
    {  "putchuk",          "some",                        C_POWDER,    100 },
    {  "rosewater",        "a petal-vial of",             C_LIQUID,    100 },
    {  "red sandalwood oil",   "a vial of",               C_LIQUID,    500 },
    {  "white sandalwood oil", "a vial of",               C_LIQUID,    300 },
    {  "storax",           "a vial of",                   C_LIQUID,    100 },

#if defined(DRUGS)
    /* Drugs */
    {  "cocaine",          "a kilo of",                   C_POWDER,   2000 },
    {  "methamphetamine",  "a jar of",                    C_SOLID,    1000 },
    {  "yopo",             "seeds of",                    C_SOLID,      50 },
    {  "speed",            "a jar of",                    C_SOLID,      50 },
    {  "pure mdma",        "a bag of",                    C_POWDER,   5000 },
    {  "crack cocaine",    "rocks of",                    C_SOLID,       5 }, /**/
    {  "marijauna",        "flowers of",                  C_SOLID,      50 },
    {  "shwag",            "a bag of",                    C_SOLID,      25 },
    {  "dmt",              "a satchet of",                C_POWDER,    100 },
    {  "dipt",             "a satchet of",                C_POWDER,    100 },
    {  "lisurgic acid",    "hits of",                     C_SOLID,      50 },
    {  "salvia divonorum", "leaves of",                   C_SOLID,      15 },
    {  "hawaiian baby woodrose", "seeds of",              C_SOLID,      15 },
    {  "psilocybin spores","a satchet of",                C_SOLID,      10 },
    {  "marijuana seeds",  "a bag of",                    C_SOLID,      10 },
    {  "mescaline",        "microdots of",                C_SOLID,      50 }, /**/
    {  "heroin",           "a stamp bag of",              C_SOLID,      20 },
    {  "speedball",        "a ",                          C_POWDER,    100 },
    {  "morphine",         "a shot of",                   C_LIQUID,     30 }, /**/
    {  "ketamine",         "a vial of",                   C_LIQUID,     70 },
    {  "angel dust",       "a bag of",                    C_LIQUID,    140 }, /**/
    {  "wet",              "a smoke of",                  C_SOLID,     100 }, /**/
    {  "psilocybin mushrooms", "a bag of",                C_SOLID,      35 },
    {  "keef",             "a bag of",                    C_POWDER,   1200 },
    {  "dextromethorphan", "a satchet of",                C_POWDER,     10 },
#endif
    {  "",                 "",                            -1,           -1 }
};

/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 * In the damage messages:  %s = singular message      $t = plural
 */
#define null                NULL
#define LFUN(n)             n
#define SFUN(n)             n

#define GROUP(a,b)          a, b

#define STATS(a,b,c,d,e)    a, b, c, d, e

#define SLOT(n)             n
#define WAIT(n)             n

#define PRAC(a,b)           a, b

#define TIME(n)             n*5
#define COST(n)             n
#define NO_COMPONENTS       0

const   struct  group_type  group_table [MAX_GROUP] =
{                                                        /*dbkey*/
   { "craft",                         GRO_CRAFT,          700   },
   { "language",                      GRO_LANGUAGE,       100   },
   { "legerdemain",                   GRO_LEGERDEMAIN,    500   },
   { "offense",                       GRO_OFFENSE,        200   },
   { "defense",                       GRO_DEFENSE,        300   },
   { "wilderness survival",           GRO_SURVIVAL,       400   },
   { "weapon proficiencies",          GRO_WP,             600   },
   { "firecasting (San)",             GRO_FIRE_BASIC,     800   },
   { "watercasting (Nelse)",          GRO_WATER_BASIC,    900   },
   { "aircasting (Zeich)",            GRO_AIR_BASIC,      1000  },
   { "earthcasting (Kora)",           GRO_EARTH_BASIC,    1100  },
   { "spellcasting",                  GRO_SPC_GENERAL,    1200  },
   { "necromantics (Dadro)",          GRO_NECROMANTICS,   1300  },
   { "conjurations (Zad)",            GRO_CONJURATIONS,   1400  },
   { "maladictions (Gana)",           GRO_MALADICTIONS,   1500  },
   { "illusions (Taug)",              GRO_ILLUSIONS,      1600  },
   { "divinations (Wodan)",           GRO_DIVINATIONS,    1700  },
   { "enchantments (Nasang)",         GRO_ENCHANTMENTS,   1800  },
   { "",                              GRO_NONE,           0     }
};


#undef null


