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

#include <stdbool.h>

/*
 * Administrative email.
 */
#define MUD_NAME     "The Isles: Lands of Kltara"
#define ADMIN_EMAIL  "locke@nimud.divineright.org"
#define MUD_ADDRESS  "nimud.divineright.org"
#define SHELL_NAME   "locke"
#define EMAIL(r,s,b)  { char mbuf[MSL];  save_string_to_file( ".mailout", b );\
                        sprintf( mbuf, "mail -c %s %s -s \"%s\" -u %s < .mailtemp > .shellout", ADMIN_EMAIL, r, s, SHELL_NAME ); system( mbuf ); }

#define EXE_FILE "nimud"

/*
 * For machines that have absolutely no cryptography whatsoever.
 * Commented out or define crypt for crypt.
 */

#if defined(NOCRYPT) || !defined(crypt)
#define crypt( a, b )   (a)
#endif

/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */


/* Turn On Hotboot
 */
/*#define HOTBOOT*/

/* Turn on HTML export.
 */
#define HTML


/*
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define const
#define args( list )            ( )
#define COMMAND( fun )          void fun( )
#define DECLARE_HIT_FUN( fun )          void fun( )
#else
#define args( list )            list
#define COMMAND( fun )          CMD_FUN   fun
#define DECLARE_HIT_FUN( fun )          HIT_FUN   fun
#endif



/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if    !defined(FALSE)
#define FALSE     0
#endif

#if    !defined(TRUE)
#define TRUE     1
#endif

#if    defined(_AIX)
#if    !defined(const)
#define const
#endif
#define unix
#else
#endif

/*
 * For FreeBSD.
 */
#if !defined(BSD) 
#define BSD
#endif

/*
 * Structure types.
 */
typedef struct  bonus_data         BONUS_DATA;
typedef struct  zone_data          ZONE_DATA;
typedef struct  ban_data           BAN_DATA;
typedef struct  player_data        PLAYER_DATA;
typedef struct  user_data          USER_DATA;
typedef struct  connection_data    CONNECTION_DATA;
typedef struct  exit_data          EXIT_DATA;
typedef struct  extra_descr_data   EXTRA_DESCR_DATA;
typedef struct  help_data          HELP_DATA;
typedef struct  kill_data          KILL_DATA;
typedef struct  actor_index_data   ACTOR_INDEX_DATA;
typedef struct  number_data        NUMBER_DATA;
typedef struct  attack_data        ATTACK_DATA;
typedef struct  note_data          NOTE_DATA;
typedef struct  prop_data          PROP_DATA;
typedef struct  prop_index_data    PROP_INDEX_DATA;
typedef struct  command_queue      COMMAND_QUEUE;
typedef struct  alias_data         ALIAS_DATA;
typedef struct  spell_data         SPELL_DATA;
typedef struct  skill_data         SKILL_DATA;
typedef struct  spell_book_data    SPELL_BOOK_DATA;
typedef struct  scene_index_data   SCENE_INDEX_DATA;
typedef struct  spawn_data         SPAWN_DATA;
typedef struct  shop_data          SHOP_DATA;
typedef struct  weather_info_data  TIME_INFO_DATA;
typedef struct  weather_data       WEATHER_DATA;
typedef struct  star_data          STAR_DATA;    
typedef struct  script_data        SCRIPT_DATA;
typedef struct  instance_data      INSTANCE_DATA;
typedef struct  variable_data      VARIABLE_DATA;
typedef struct  event_data         EVENT_DATA;
typedef struct  terrain_data       TERRAIN_DATA;
typedef struct  castle_data        CASTLE_DATA;

/*
 * Specialty lists for numbers and strings.
 */

typedef struct  number_list        NUMBER_LIST;
typedef struct  string_list        STRING_LIST;       

/*
 * For crunching lists of numbers.
 */ 
struct number_list {
       int num;
       NUMBER_LIST *next;
};

/*
 * Use str_dup when assigning str.
 */
struct string_list {
       char *str;
       STRING_LIST *next;
};


/*
 * Player filename structure.
 */
typedef struct  pc_name            PC_NAME;


/*
 * Function types.
 */
typedef void CMD_FUN    args( ( PLAYER_DATA *ch, char *argument ) );
typedef bool HIT_FUN    args( ( PLAYER_DATA *ch, PLAYER_DATA *victim, int hit, int dam ) );



/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH         1024
#define MAX_STRING_LENGTH    131092
#define MAX_INPUT_LENGTH     4096
#define WORK_STRING_LENGTH   4096

/* I am lazy :) */
#define MSL MAX_STRING_LENGTH
#define MIL MAX_INPUT_LENGTH


 /*  1/21/95 */
 /*  3/26/95 */
 /*  7/17/95 */
 /*  9/13/95 */
 /* 10/29/95 */
 /* 03/02/96 */
 /* 03/27/96 */
 /* 04/12/96 */
 /* 10/27/96 */
 /* 10/17/98 */
 /* 04/23/01 */
 /* 01/03/02 */

#define VERSION_STR       "NiM5 Build 12773"

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_ZONE                  200
#define MAX_ATTACK_DATA            10
#define MAX_COIN                    5
#define MAX_COLORS                 10
#define MAX_USED_COLORS             3
#define MAX_LEVEL                  10
#define MAX_RACE                   17
#define MAX_GROUP                  19
#define MAX_SOCIALS               256
#define MAX_TRADE                  10
#define MAX_ATTACK                 23
#define MAX_LANGUAGE               10
#define MAX_DEBUG                   5
#define EXP_PER_LEVEL            1000
   
#define MAX_MORTAL_LEVEL          666

#define RACE(a,b)              (race_table[a].b)

#define LEVEL_GOD                  MAX_LEVEL                        /* 10 */
#define LEVEL_DEMIGOD              9
#define LEVEL_SUPREME              8
#define LEVEL_ANGEL                7

#define LEVEL_BUILDER              6
#define LEVEL_AGENT                5
#define LEVEL_IMMORTAL             4
#define LEVEL_HERO                 3

#define LEVEL_MORTAL               2 /* 2 */
#define LEVEL_NEW                  1 /* 1 */

#define LEVEL_SCRIPT               (LEVEL_BUILDER)

/*
 * Time synchronization.
 */
#define PULSE_PER_SECOND             4                      /* 1/4 second */
#define PULSE_VIOLENCE            (  3 * PULSE_PER_SECOND)  /* 3 seconds  */
#define PULSE_MOV              (  4 * PULSE_PER_SECOND)  /* 4 seconds  */
#define PULSE_TICK                (300 * PULSE_PER_SECOND)  /* 5 minutes  */
#define PULSE_ZONE                ( 60 * PULSE_PER_SECOND)  /* 1 minute   */

#if defined(IMC)
#include "imc.h"
#endif

/*
 * TO types for act.
 */
#define TO_SCENE            0
#define TO_NOTVICT          1
#define TO_VICT             2
#define TO_ACTOR            3
#define TO_ALL              4


/*
 * Fill bits for argument_interpret.  Never implemented.
 */
#define FA_IN       0x00000001
#define SA_IN       0x00000002
#define FA_ON       0x00000004
#define SA_ON       0x00000008
#define FA_HERE     0x00000010
#define SA_HERE     0x00000020
#define FA_MY       0x00000040
#define SA_MY       0x00000080

/*
 * Site ban structure.
 */
struct    ban_data
{
    BAN_DATA *    next;
    char *    name;
};

/*
 * Time and weather stuff.
 * Note that weather changes according to the month (winter).
 */
#define SUN_DARK                    0
#define SUN_RISE                    1
#define SUN_LIGHT                   2
#define SUN_SET                     3
#define MOON_RISE                   4
#define MOON_SET                    5

#define SKY_CLOUDLESS               0
#define SKY_CLOUDY                  1
#define SKY_RAINING                 2
#define SKY_LIGHTNING               3

#define MOON_NEW                    0
#define MOON_WAXING_CRESCENT        1
#define MOON_WAXING_HALF            2
#define MOON_WAXING_THREE_QUARTERS  3
#define MOON_FULL                   4
#define MOON_WANING_THREE_QUARTERS  5
#define MOON_WANING_HALF            6
#define MOON_WANING_CRESCENT        7
#define MOON_MAX                    8


/*
 * Starmap
 *
 * A vnum table populated by HEASARC data; though you might
 * be able to use other .tdat style NASA files.  The supplied
 * starmap.tdat is actually a copy of woolley.tdat, a publically
 * accessible stellar map of stellar masses within the nearest
 * 25 parsecs.
 *
 * Most of the data is unused or ignored, and is held in memory.
 * This file is never written, only read.
 *
 * The U/V/W velocities, or the lii and bii values can be used
 * to generate starmaps.  The spectral types are seeds for the
 * generation of stellar systems; spectral types and colors provide
 * a generation point for theoretic or seeded planetary systems.
 *
 * In real life, we are still uncertain or are convinced that
 * planetary systems are invisible to the naked eye or to
 * telescopic observations; in the game, the data can be used to
 * seed planetary systems.
 */
struct star_data {
  STAR_DATA *next;
  char *name;
  int sequence_number;  // Woolley Catalog Sequence Number
  char component_id;    // Component Identification Suffix
  int parallax;         // Trigonometric Parallax
  int parallax_error;   // Probable Error in Parallax
  char parallax_error_code;   // Flag Indicates Parallax Error Percentage
                              // 8 if < 15%, 9 if <10%; S=spectroscopitc
  float proper_motion_ra;  // Proper Motion in Right Ascension (s/yr)
  float proper_motion_dec; // Proper Motion in Declination (arcsec/yr)
  float radial_velocity;   // Radial Velocity (km/s)
  char radial_velocity_code;   // Flag Indicates Source of Radial Velocity:
                               // J=mean of binary; V=variable
  int u_velocity;       // U-component of Space Velocity (Toward GC) (km/s)
  int v_velocity;       // V-component of Space Velocity (Toward GR) (km/s)
  int w_velocity;       // W-component of Space Velocity (Toward GNP) (km/s)
  
  float box_omega;      // Box Epicenter Distance from GC
			// Units: Distance from Sun to GC
  float box_e;          // Box Orbit Eccentricity
  float box_i;          // Box Orbit Angle (Highest Inclination of the Orbit)
  int luminosity_class; // Luminosity Class Code

  char *spectral_type;  // Spectral Type
  char spect_type_code; // Flag Indicates Source of Spectral Type:
                        // 1=MK, 2=combined MK, 4=combined non-MK
  float app_mag;        // Magnitude: m(v) or m(pg)
  char app_mag_code;    // Flag Indicates Type of Magnitude:
                        // J=combined, P=photographic, V=variable

  float bv_color;       // B-V Color
  float ub_color;       // U-B Color
  float abs_vmag;       // Absolute Visual Magnitude
  float ra;             // Right Ascension
  float dec;            // Declination
  float lii;            // Galactic Longitude
  float bii;            // Galactic Latitude

  char *gctp_number;    // General Catalog of Trignometric Parallaxes <I/60>
  int hd_number;        // Henry Draper Catalog (HD) Number
  char *dm_name;        // Durchmusterung (BD, CD or CP) Designation
  int gcrv_number;      // General Catalog of Radial Velocities (GCRV) Number
  char *alt_name;       // Name from Other Proper-Motion Catalog
  int hr_number;        // Bright Star Harvard Revised Catalog <V.50>
  int vyss_number;      // Number in Red Dwarf Lists of Vyssotsky

  char remark_code_1;   // First Remark Code
  char remark_code_2;   // Second Remark Code
  int class;            // Browse Object Classification
};


struct    weather_data 
{
    int      moon_phase;
    int      next_phase;
    int     sky;
    int     sunlight;    
    int windspeed;
    int winddir;
    int temperature;
    int hour;
    int day;
    int month;
    int year;
    int        mmhg;
    int        change;
};


/*
 * For world generator.
 */
struct    terrain_data
{
    TERRAIN_DATA  *next;
    int        vnum;
    int        sector;
    char       map_char;
    char *     fall;
    char *     spring;
    char *     summer;
    char *     winter;
    char *     name;
};




/*
 * Connection (channel) structure.
 */
struct    connection_data
{
    CONNECTION_DATA *    next;
    CONNECTION_DATA *    snoop_by;
    PLAYER_DATA *        character;
    PLAYER_DATA *        original;
    char *        host;
    int          lingua;
    int        connection;
    int        connected;
    bool        fcommand;
    bool                client;
    bool                fpromptok;
    int                showing;  // for simulated local echo
    char                inbuf       [4 * MAX_INPUT_LENGTH];
    char                incomm      [MAX_INPUT_LENGTH];
    char                inlast      [MAX_INPUT_LENGTH];
    char                screen[80*50];      /* for graphical windows       */
    int            repeat;
    char *              showstr_head;
    char *              showstr_point;
    char *        outbuf;
    int            outsize;
    int            outtop;
    void *              pEdit;
    char **             pString;

/*
 * Information gathered with MSSP snippet.
 */

	char              * terminal_type;
	char                msoutbuf[MAX_STRING_LENGTH];
	int                 msouttop;
	char                telbuf[MAX_INPUT_LENGTH];
	int                 teltop;
	short               comm_flags;
	short               cols;
	short               rows;
	z_stream          * mccp;
};



/*
 * Attribute bonus structures.
 */
struct    str_app_type
{
    int    tohit;
    int    todam;
    int    carry;
    int    wield;
};

struct    int_app_type
{
    int    spell_level;
    int    learn;
    int    mana;
};

struct    wis_app_type
{
    int    spell_level;
    int    practice;
};

struct    dex_app_type
{
    int    defensive;
};

struct    con_app_type
{
    int    hitp;
    int      resistance;
    int    shock;
};


/*
   comment out the next line if you don't have perl, aren't on unix,
   or don't want to mess with the multi-language support. 
   see: translate.c for more information
#define TRANSLATE
 */
#define MAX_LINGUA 119
#if defined(TRANSLATE)
struct lingua_type
{
    char *      code;
    char *      name;
};
#endif


struct lang_type
{
    char *      name;
    int      pgsn;
};

struct  comp_type
{
    char *      name;
    char *      usage;
    int      type;
    int      cost;
};

struct goods_type
{
    char *      name;
    char *      unit;
    int      code;
    int      cost;
    int      weight;
    int      life;
};

struct gem_list_type
{
    char * name;
    int mana_value;
    int mana_flags;
    int cost;
    char * plural;
};


/*
 * Help table types.
 */
struct help_data
{
   HELP_DATA *    next;
   int vnum;
   int    level;
   char *    keyword;
   char *    title;
   char *    text;
   int    class;
   char *    name;
   char *    syntax;
   char *    example;
   char *    immtext;
   char *    seealso;
};

#define HELP_CLASS_GENERAL         0
#define HELP_CLASS_MOVEMENT        1
#define HELP_CLASS_INVENTORY       2
#define HELP_CLASS_STATUS          3
#define HELP_CLASS_SKILLS          4
#define HELP_CLASS_COMBAT          5
#define HELP_CLASS_COMMMUNICATION  6
#define HELP_CLASS_INFORMATION     7
#define HELP_CLASS_BUILDER         8
#define HELP_CLASS_IMMORTAL        9
#define HELP_CLASS_NANNY           10
#define HELP_CLASS_IMC             11
#define HELP_CLASS_TIPS            12
#define MAX_HELP_CLASS             HELP_CLASS_NANNY + 3

struct help_class
{
   char *    name;
   char *    filename;
   char *    desc;
};


/*
 * Shop types.
 */
#define SHOP_TRADER         0x00000001
#define SHOP_PEDDLER        0x00000002
#define SHOP_REPAIR         0x00000004
#define SHOP_COMPONENTS     0x00000008
#define SHOP_TAVERN         0x00000010
#define SHOP_CREDITS        0x00000020 // shop deals in credits
#define SHOP_BUCKS          0x00000040 // shop deals in bucks

struct    shop_data
{
    SHOP_DATA * next;                   /* Next Shop in List            */
    int    keeper;            /* Vnum of shop keeper actor    */
    int      buy_type [MAX_TRADE];   /* Item types shop will buy     */
    int      trades   [MAX_TRADE];   /* Item types shop will buy     */
    int    profit_buy;        /* Cost multiplier for buying    */
    int    profit_sell;        /* Cost multiplier for selling    */
    int    open_hour;        /* First opening hour        */
    int    close_hour;        /* First closing hour        */
    int      shop_flags;             /* Bits                         */
    int      repair_rate;            /* Cost per unit of armor       */
    int         buy_index;              /* Index of trade items bought  */
    int         sell_index;             /* Index of trade items sold    */
    int         comp_index;             /* Index of components sold     */
    char *      no_such_item;           /* Keeper hasn't got that item  */
    char *      cmd_not_buy;             /* Keeper won't buy such things */
    char *      list_header;            /* Displayed on peddler list    */
    char *      hours_excuse;           /* Keeper explains why not open */
};



/*
 * Race archetypes for actors and players.
 */
struct    race_type
{
   char *   race_name;
   char *   race_name_type;

   int   start_str;             /* Starting actor statistics   */
   int   start_int;
   int   start_wis;
   int   start_dex;
   int   start_con;

   int   primary;               /* language                    */
   int   secondary;             /* langauge                    */

   /* PC races only use these, below */

   int      start_scene;           /* PC start scene              */
   int      max_skills;            /* PC max skills can learn     */
   int      bonus_bits;            /* PC affect bits              */
   int   size;                  /* size                        */
   int      base_age;              /* base age                    */
   int      start_age;             /* PC initial age              */
};


/*
 * One coin.
 */
struct coin_type
{
   int      multiplier;
   int      convert;
   int      weight;
   char *   long_name;
   char *   short_name;
};





/*
 * Un colour type.
 */
struct color_data
{
   char         code[10];
   char         act_code[5];
   char         name[15];
   char         mxp[10];
   char *       di;  // display interp
};


/*
 * Board information.
 */
struct board_data
{
    char *name;
    char *filename;
    int writelevel;
    int readlevel;
};


#define NOTE_ANONYMOUS          1

/*
 * Data structure for notes.
 */
struct    note_data
{
    NOTE_DATA *    next;
    char *    sender;
    char *    date;
    char *      to_list;
    char *    subject;
    char *    text;
    time_t      date_stamp;
    int      note_flags;
};



/*
 * An affect.
 */
struct    bonus_data
{
    BONUS_DATA *    next;
    char *         msg_off;
    int        location;
    int        modifier;
    int              type;
    int              duration;
    int            bitvector;
};



/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_HIT                     1000
#define TYPE_SLASH                   1001
#define TYPE_PIERCE                  1002
#define TYPE_WHIP                    1003
#define TYPE_EXPLODE                 1004
#define TYPE_POUND                   1005
#define TYPE_SUCTION                 1006
#define TYPE_SHOT                    1007

/*
 * For extra attacks on actors.
 */
struct attack_data
{
     ATTACK_DATA * next;
     int           idx;       /* See attack_table for this value */
     int        dam1;
     int        dam2;
};

                     /* AMI DOLENZ I LOVE YOU! */

/*
 * For attack table.
 */
struct attack_type
{
     char     * name;
     int        hit_type;
     int     wpgsn;                 /* which slot? */
     HIT_FUN  * hit_fun;
};


/*
 * Ansi colors and VT100 codes.
 * On most machines that use ANSI, namely the IBM PCs, the decimal value for
 * the escape character is 27 (1B hex).     Change this value when needed.
 */

#define ESC        '\x1b'

#define BLACK      "\x1b[30m"    /* These are foreground color codes */
#define RED        "\x1b[31m"
#define GREEN      "\x1b[32m"
#define YELLOW     "\x1b[33m"
#define AMBER      "\x1b[33m"
#define BLUE       "\x1b[34m"
#define PURPLE     "\x1b[35m"
#define CYAN       "\x1b[36m"
#define GREY       "\x1b[37m"

#define B_BLACK    "\x1b[40m"    /* These are background color codes */
#define B_RED      "\x1b[41m"
#define B_GREEN    "\x1b[42m"
#define B_YELLOW   "\x1b[43m"
#define B_BLUE     "\x1b[44m"
#define B_PURPLE   "\x1b[45m"
#define B_CYAN     "\x1b[46m"
#define B_GREY     "\x1b[47m"

/* Below are VT100 and ANSI codes (above are ANSI exclusively)       */

#define EEEE       "\x1b#8"                 /* Turns screen to EEEEs */
#define CLRSCR     "\x1b[2J\x1b[H"          /* Clear screen, pos 1,1 */
#define CLREOL     "\x1b[K"                 /* Clear to end of line  */

#define UPARR      "\x1b[A"                 /* Up one line           */
#define DOWNARR    "\x1b[B"                 /* Down one line         */
#define RIGHTARR   "\x1b[C"                 /* Right one column      */
#define LEFTARR    "\x1b[D"                 /* Left one column       */
#define HOMEPOS    "\x1b[H"                 /* Home (upper left)     */

#define BOLD       "\x1b[1m"                /* High intensity        */
#define FLASH      "\x1b[5m"                /* Flashing text         */
#define INVERSE    "\x1b[7m"                /* XORed back and fore   */
#define NTEXT      "\x1b[0m"                /* Normal text  (grey)   */
#define ANSI_NORMAL "\x1b[0m"               /* ""                    */


/*
 * Other codes of note for future ANSI development:
 * The <esc>[y;xH code works nicely for positioning the cursor.
 */


/************************************************************************
 *                      Actor information.                             *
 ************************************************************************/

/*
 * Act bits.  (Behavior)
 */
#define ACT_IS_NPC           0x00000001         /* Auto set for actors  */
#define ACT_NPC              0x00000001         /* Auto set for actors  */
#define ACT_SENTINEL         0x00000002         /* Stays in one scene   */
/* was scavenger */
#define ACT_HALT             0x00000008         /* die script die!      */

#define ACT_GOOD             0x00000010         /* Can't be attacked    */
#define ACT_AGGRESSIVE       0x00000020         /* Attacks PC's         */
#define ACT_STAY_ZONE        0x00000040         /* Won't leave zone     */
#define ACT_WIMPY            0x00000080         /* Flees when hurt      */
#define ACT_PET              0x00000100         /* Auto set for pets    */
#define ACT_PRACTICE         0x00000400         /* Can practice PC's    */
#define ACT_MERCY            0x00000800         /* Mob will retreat     */
#define ACT_BOUNTY           0x00001000
#define ACT_MOUNT            0x00002000         /* can be mounted       */
#define ACT_NOSCAN           0x00004000         /* not seen on scan     */

#define ACT_LYCANTHROPE      0x00008000         /* lycanthropic         */
#define ACT_VAMPIRE          0x00010000         /* vampiric             */

#define ACT_NOCORPSE         0x00020000         /* death does not produce a corpse */


/*
 * Bonus bits.
 */
#define AFF_BLIND            0x00000001
#define AFF_INVISIBLE        0x00000002
#define AFF_DETECT_EVIL      0x00000004
#define AFF_DETECT_INVIS     0x00000008
#define AFF_DETECT_MAGIC     0x00000010
#define AFF_DETECT_HIDDEN    0x00000020
#define AFF_HOLD             0x00000040
#define AFF_SANCTUARY        0x00000080
#define AFF_FAERIE_FIRE      0x00000100
#define AFF_INFRARED         0x00000200
#define AFF_CURSE            0x00000400
#define AFF_FLAMING          0x00000800        /* Unused       */
#define AFF_POISON           0x00001000
#define AFF_PROTECT          0x00002000
#define AFF_PARALYSIS        0x00004000        /* Unused       */
#define AFF_SNEAK            0x00008000
#define AFF_HIDE             0x00010000
#define AFF_SLEEP            0x00020000

#define AFF_METAMORPH        0x00040000

#define AFF_FLYING           0x00080000
#define AFF_PASS_DOOR        0x00100000
#define AFF_FREEACTION       0x00200000
#define AFF_BREATHING        0x00400000

#define AFF_FALLING          0x00800000        /* Don't ask.. :P */
#define AFF_CHARM            0x01000000


/*
 * Sex.
 */
#define SEX_NEUTRAL                0
#define SEX_MALE                   1
#define SEX_FEMALE                 2

/*
 * Hoods for Clothing.
 */
#define HOODED                     1
#define HOOD_UP                    2

/*
 * Item types.
 */
#define ITEM_DELETE               -1
#define ITEM_TRASH                 0
#define ITEM_LIGHT                 1
#define ITEM_SCROLL                2
#define ITEM_WAND                  3 /* unused */
#define ITEM_STAFF                 4 /* unused */
#define ITEM_WEAPON                5
#define ITEM_RANGED_WEAPON         6
#define ITEM_AMMO                  7
#define ITEM_TREASURE              8
#define ITEM_ARMOR                 9
#define ITEM_POTION                10
#define ITEM_BOOK                  11
#define ITEM_FURNITURE             12
#define ITEM_CLOTHING              13
#define ITEM_PAPER                 14
#define ITEM_CONTAINER             15
#define ITEM_THROWN                16
#define ITEM_DRINK_CON             17
#define ITEM_KEY                   18
#define ITEM_FOOD                  19
#define ITEM_MONEY                 20
#define ITEM_GEM                   21
#define ITEM_VEHICLE               22
#define ITEM_CORPSE_NPC            23
#define ITEM_CORPSE_PC             24
#define ITEM_FOUNTAIN              25
#define ITEM_PILL                  26
#define ITEM_TOOL                  27
#define ITEM_LIST                  28
#define ITEM_BOARD                 29
#define ITEM_COMPONENT             30
#define ITEM_GOODS                 31
#define ITEM_JEWELRY               32
#define ITEM_SPELLBOOK             33 
#define ITEM_QUILL                 34 

#define ITEM_INDEX                 35

#define ITEM_MAX                   36

/*
 * Object values.

 Item Type          Value[0]        Value[1]        Value[2]       Value[3]
------------------------------------------------------------------------------
 ITEM_TRASH         -               -               -              -
 ITEM_LIGHT         Current Light   Max Light(-1)   -              Light flags
 ITEM_SCROLL        Spell Level     Spell SN        Spell SN       Spell SN
 ITEM_WAND          Spell Level     Min Charge      Max Charge     Spell SN
 ITEM_STAFF         Spell Level     Min Charge      Max Charge     Spell SN
 ITEM_WEAPON        Poison (b)      Min Damage      Max Damage     Weap Type
 ITEM_RANGED_WEAPON Ammo Type       Min Damage      Max Damage     Ranged Type
 ITEM_AMMO          Ammo Quantity   Bonus to Dam    Spell Level    Spell SN
 ITEM_TREASURE      -               -               -              -
 ITEM_ARMOR         Armor Bonus     Current Damage  Max Damage     -
 ITEM_POTION        Spell Level     Spell SN        Spell SN       Spell SN
 ITEM_BOOK          -               -               -              -
 ITEM_FURNITURE     Capacity        Furniture Flags Key (vnum)     Scene (vnum)
 ITEM_CLOTHING      -               -               -              -
 ITEM_PAPER         -               -               -              -
 ITEM_CONTAINER     Maximum Weight  Container Flags Key (vnum)     -
 ITEM_THROWN        Spell Level     Spell SN        Spell SN       Spell SN
 ITEM_DRINK_CON     Current Volume  Capacity        Liquid Type    Drink Flags
 ITEM_KEY           -               -               -              -
 ITEM_FOOD          Ticks of Food   -               Reference Vnum Poison (b)
 ITEM_MONEY         Amount          Type            Set            -
 ITEM_GEM           Mana Flags      Current Mana    Max Mana       -
 ITEM_VEHICLE       Sector Pass     Sector Pass     Sector Pass    Sector Pass
 ITEM_CORPSE_NPC    Actor's Race   -               -              -
 ITEM_CORPSE_PC     -               -               -              -
 ITEM_FOUNTAIN      Spell Level     Spell SN        Spell SN       Spell SN
 ITEM_PILL          Spell Level     Spell SN        Spell SN       Spell SN
 ITEM_TOOL          Tool Flags      Number of uses  Max uses       -
 ITEM_LIST          Number of vnums -               -              -
 ITEM_BOARD         Board Index     -               -              -
 ITEM_COMPONENT     Component Type  -               -              -
 ITEM_GOODS         Goods Type      -               -              -
 ITEM_JEWELRY       -               -               -              -

 */

/* Object Value Notes:
 *
 * On drink containers, liquid -1 = oil
 * On Pois values, if pois=0 it means no poison, otherwise it is the potency
 * of poison (effects duration)
 */

/*
 * Values for ranged weapons (value[3])
 */

#define RNG_BOW               0
#define RNG_CROSSBOW          1
#define RNG_CATAPULT          2

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE        1
#define CONT_PICKPROOF        2
#define CONT_CLOSED           4
#define CONT_LOCKED           8
#define CONT_SECRET          16

/*
 * Values for drink containers (value[3]).
 * Used in #OBJECTS.
 */
#define DRINK_POISON          1
#define DRINK_TAVERN          2

/*
 * Values for furniture (value[1]).
 * Used in #OBJECTS
 */
#define FURN_CLOSEABLE       CONT_CLOSEABLE
#define FURN_PICKPROOF       CONT_PICKPROOF
#define FURN_CLOSED          CONT_CLOSED
#define FURN_LOCKED          CONT_LOCKED

#define FURN_NOMOUNT         0x00001000
#define FURN_SIT             0x00002000
#define FURN_SLEEP           0x00004000
#define FURN_EXIT            0x00008000
#define FURN_PUT             0x00010000
#define FURN_MOVE            0x00020000
#define FURN_NOSHOW          0x00040000
#define FURN_HOME            0x00080000


/*
 * Values for lights (value[3]).
 * Used in #OBJECTS
 */
#define LIGHT_LIT             1
#define LIGHT_FILLABLE        2

/*
 * Values for tools (value[0]).
 * Used in #OBJECTS
 */
#define TOOL_TINDERBOX                1
#define TOOL_LOCKPICK                 2
#define TOOL_BANDAGES                 4
#define TOOL_BOUNTY                   8
#define TOOL_REPAIR                   16

/*
 * Armor sizes.
 */
#define SIZE_ANY             0
#define SIZE_MINUTE          2 * 4
#define SIZE_SMALL           4 * 4
#define SIZE_PETITE          5 * 4
#define SIZE_AVERAGE         6 * 4
#define SIZE_MEDIUM          7 * 4
#define SIZE_LARGE           8 * 4
#define SIZE_HUGE           10 * 4
#define SIZE_TITANIC        14 * 4
#define SIZE_GARGANTUAN     16 * 4

/*
 * Awkward sizes (For actors..)
 */
#define SIZE_HORSE         -2



/*
 * Apply types (for affects).             Can't use these as slot numbers.
 * Used in #OBJECTS.
 */
#define APPLY_NONE                10000
#define APPLY_STR                 10001
#define APPLY_DEX                 10002
#define APPLY_INT                 10003
#define APPLY_WIS                 10004
#define APPLY_CON                 10005
#define APPLY_SEX                 10006

#define APPLY_AGE                 10007
#define APPLY_SIZE                10008

#define APPLY_AC                  10009
#define APPLY_HITROLL             10010
#define APPLY_DAMROLL             10011
#define APPLY_SAVING_THROW        10012
  


/*
 * Scene flags.
 * Used in #SCENES.
 */
#define SCENE_DARK            0x00000001
#define SCENE_NO_ACTOR        0x00000002
#define SCENE_INDOORS         0x00000004
#define SCENE_IMMORTAL        0x00000008
#define SCENE_MARK            0x00000010
#define SCENE_SAVE            0x00000040
                               /***/
#define SCENE_WAGON           0x00000080
                               /***/
#define SCENE_SAFE            0x00000400
#define SCENE_PET_SHOP        0x00001000
#define SCENE_TEMPLATE        0x00002000
#define SCENE_BUILDABLE       0x00004000     /* Scene can support player houses */
#define SCENE_RECYCLE         0x00008000


/*
 * Directions.
 * Used in #SCENES.
 */
#define DIR_NORTH                     0
#define DIR_EAST                      1
#define DIR_SOUTH                     2
#define DIR_WEST                      3
#define DIR_UP                        4
#define DIR_DOWN                      5
#define DIR_NW                        6
#define DIR_NE                        7
#define DIR_SW                        8
#define DIR_SE                        9
#define MAX_DIR                      10




/*
 * Exit flags.
 * Used in #SCENES.
 */
#define EX_ISDOOR            0x00000001
#define EX_CLOSED            0x00000004
#define EX_LOCKED            0x00000002
#define EX_PICKPROOF         0x00000008
#define EX_EAT_KEY           0x00000010
#define EX_BASHPROOF         0x00000020
#define EX_JAMMED            0x00000040
#define EX_SECRET            0x00000080          /* can see only when open  */
#define EX_TRANSPARENT       0x00000100          /* see through when closed */
#define EX_WINDOW            0x00000200          /* shoot through, not move */
#define EX_CONCEALED         0x00000400          /* can't see at all        */
#define EX_NOMOVE            0x00000800          /* only look through       */



/*
 * Sector types.
 * Used in #SCENES.
 */
#define SECT_INSIDE                   0          /* cant see weather        */
#define SECT_CITY                     1          /* outdoor mv as inside    */
#define SECT_FIELD                    2          /* illuminated by moon     */
#define SECT_FOREST                   3          /* not illuminated         */
#define SECT_HILLS                    4          /* slower moving           */
#define SECT_MOUNTAIN                 5          /* requires grapple        */
#define SECT_WATER_SWIM               6          /* dropped props float away */
#define SECT_WATER_NOSWIM             7          /* 4 pulse time on props    */
#define SECT_UNDERWATER               8          /* needs breathing help    */
#define SECT_AIR                      9          /* requires FLY or vehicle */
#define SECT_DESERT                  10          /* high movement loss      */
#define SECT_ICELAND                 11          /* colder ( or something ) */
#define SECT_CLIMB                   12          /* things fall, no grapple */
#define SECT_MAX                     13




/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW            0x00000001
#define ITEM_HUM             0x00000002
#define ITEM_DARK            0x00000004
#define ITEM_LOCK            0x00000008
#define ITEM_EVIL            0x00000010
#define ITEM_INVIS           0x00000020
#define ITEM_MAGIC           0x00000040
#define ITEM_NODROP          0x00000080
#define ITEM_BLESS           0x00000100
#define ITEM_NOREMOVE        0x00000200
#define ITEM_INVENTORY       0x00000400
#define ITEM_NOSAVE          0x00000800
#define ITEM_BURNING         0x00001000
#define ITEM_HITCH           0x00002000
#define ITEM_WAGON           0x00004000
#define ITEM_USED            0x00008000

#define ITEM_BURNT           0x00010000


/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE            0x00000001
#define ITEM_HOLD            0x00000001
#define ITEM_WEAR_FINGER     0x00000002
#define ITEM_WEAR_NECK       0x00000004
#define ITEM_WEAR_BODY       0x00000008
#define ITEM_WEAR_HEAD       0x00000010
#define ITEM_WEAR_LEGS       0x00000020
#define ITEM_WEAR_FEET       0x00000040
#define ITEM_WEAR_HANDS      0x00000080
#define ITEM_WEAR_ARMS       0x00000100
#define ITEM_WEAR_SHIELD     0x00000200
#define ITEM_WEAR_ABOUT      0x00000400
#define ITEM_WEAR_WAIST      0x00000800
#define ITEM_WEAR_WRIST      0x00001000
#define ITEM_WEAR_BELT       0x00002000

#define ITEM_WIELD           0x00004000

#define ITEM_WEAR_SHOULDER   0x00008000
#define ITEM_WEAR_ANKLE      0x00010000
#define ITEM_WEAR_EAR        0x00020000
#define ITEM_WEAR_HELM       0x00040000
#define ITEM_WEAR_SHIRT      0x00080000
#define ITEM_WEAR_STOCKING   0x00100000
#define ITEM_WEAR_FACE       0x00200000
#define ITEM_WEAR_BACK       0x00400000
#define ITEM_WEAR_LOIN       0x00800000
#define ITEM_WEAR_FOREHEAD   0x01000000
#define ITEM_WEAR_NOSE       0x02000000
#define ITEM_WEAR_PANTS      0x04000000

#define ITEM_WEAR_PIN        0x08000000
#define ITEM_WEAR_ASBELT     0x10000000
#define ITEM_WEAR_SHOULDERS  0x20000000

#define ITEM_TWO_HANDED      0x80000000         /* force two handed */



/*
 * Equipment wear locations.
 * Used in #SPAWNS.
 */
#define WEAR_NONE                    -1
#define WEAR_EAR_L                    0
#define WEAR_EAR_R                    1
#define WEAR_HEAD                     2
#define WEAR_HELM                     3
#define WEAR_FOREHEAD                 4
#define WEAR_FACE                     5
#define WEAR_NOSE                     6
#define WEAR_NECK_1                   7
#define WEAR_NECK_2                   8
#define WEAR_SHIRT                    9
#define WEAR_BODY                    10
#define WEAR_BACK                    11
#define WEAR_LEGS                    12
#define WEAR_PANTS                   13
#define WEAR_STOCKING                14
#define WEAR_FEET                    15
#define WEAR_ANKLE_L                 16
#define WEAR_ANKLE_R                 17
#define WEAR_ARMS                    18
#define WEAR_WRIST_L                 19
#define WEAR_WRIST_R                 20
#define WEAR_HANDS                   21
#define WEAR_FINGER_L                22
#define WEAR_FINGER_R                23
#define WEAR_SHIELD                  24
#define WEAR_ABOUT                   25
#define WEAR_FLOATING                26
#define WEAR_WAIST                   27
#define WEAR_LOIN                    28
#define WEAR_PIN                     29
#define WEAR_BELT_1                  30
#define WEAR_BELT_2                  31
#define WEAR_BELT_3                  32
#define WEAR_BELT_4                  33
#define WEAR_BELT_5                  34
#define WEAR_HOLD_1                  35
#define WEAR_HOLD_2                  36
#define WEAR_SHOULDER_L              37
#define WEAR_SHOULDER_R              38
#define WEAR_WIELD_1                 39
#define WEAR_WIELD_2                 40
#define WEAR_ASBELT_1                41
#define WEAR_ASBELT_2                42
#define WEAR_ASBELT_3                43
#define WEAR_SHOULDERS               44
#define MAX_WEAR                     45





/***************************************************************************
 * *********************************************************************** *
 *  **************** VALUES OF INTEREST TO ZONE BUILDERS ****************  *
 *   *************** (End of this section ... stop here) ***************   *
 *    *****************************************************************    *
 ***************************************************************************/

/*
 * Conditions.
 */
#define COND_DRUNK               0
#define COND_FULL                1
#define COND_THIRST              2



/*
 * Positions.
 */
#define POS_DEAD                      0
#define POS_MORTAL                    1
#define POS_INCAP                     2
#define POS_STUNNED                   3
#define POS_SLEEPING                  4
#define POS_RESTING                   5
#define POS_SITTING                   6
#define POS_FIGHTING                  7
#define POS_STANDING                  8
#define MAX_POSITION                  POS_STANDING


/*
 * Color codings for
 * channels+says
 */
#define COLOR_COMM                    0
#define COLOR_IMMTALK                 1
#define COLOR_SCENE_TITLE             2
#define COLOR_GROUPS                  3                /* Unused */
#define COLOR_CHAT                    4                /* Unused */
#define COLOR_PROP_LIST               5                /* Unused */
#define COLOR_UNUSED6                 6                /* Unused */
#define COLOR_UNUSED7                 7                /* Unused */
#define COLOR_UNUSED8                 8                /* Unused */
#define COLOR_UNUSED9                 9                /* Unused */




/*
 * ACT bits for players.
 */
#define PLR_IS_NPC           0x00000001         /* Don't EVER set.      */

#define PLR_LOG              0x00000010 
#define PLR_DENY             0x00000020 
#define PLR_FREEZE           0x00000040 

#define PLR_SILENCE          0x00000080
#define PLR_NO_EMOTE         0x00000100
#define PLR_NO_TELL          0x00000200
#define PLR_NO_WISH          0x00000400
#define PLR_IDLE             0x00000800

#define PLR_LYCANTHROPE      0x00008000    /* not settable by player */
#define PLR_VAMPIRE          0x00010000    /* not settable by player */

#define WIZ_NOTIFY           0x00080000
#define WIZ_NOTIFY_1         0x00100000
#define WIZ_NOTIFY_DAMAGE    0x00200000
#define WIZ_NOTIFY_LOGIN     0x00400000
#define WIZ_NOTIFY_DEATH     0x00800000
#define WIZ_NOTIFY_BUG       0x01000000
#define WIZ_NOTIFY_LOG       0x02000000
#define WIZ_NOTIFY_SCRIPT    0x04000000
#define WIZ_NOTIFY_EVENT     0x08000000

#define PLR_APPLIED          0x10000000

/*
 *  Act2 Bits
 * Channel / Config bits.
 */
#define CHANNEL_IMMTALK      0x00000001
#define CHANNEL_WISHES       0x00000002
#define CHANNEL_CHAT         0x00000004

#define PLR_WIZBIT           0x00000100

#define PLR_AUTOEXIT         0x00000200
#define PLR_AUTOMAP          0x00000400
#define PLR_VERBOSE          0x00000800

#define PLR_BLANK            0x00001000
#define PLR_BRIEF            0x00002000
#define PLR_COMBINE          0x00004000
#define PLR_PROMPT           0x00008000
#define PLR_TELNET_GA        0x00010000
#define PLR_ALIASES          0x00020000
#define PLR_TIPS             0x00040000
#define PLR_TIMECOLOR        0x00080000

#define PLR_CLRSCR           0x00100000
#define PLR_VT100            0x00200000
#define PLR_ANSI             0x00400000
#define PLR_MXP              0x00800000

#define WIZ_HOLYLIGHT        0x01000000
#define WIZ_EQUIPMENT        0x02000000
#define WIZ_TRANSLATE        0x04000000

#define PLR_ASSIST           0x10000000  // autoassist
#define PLR_MSP              0x40000000  // toggle for msp
#define PLR_NAGE             0x60000000  // toggle for nage
#define PLR_CLIENT           0x80000000     /* For Web Client */


/* event.c */
struct event_data {
        void * owner;
        int    type;
        int    time;
        INSTANCE_DATA *instance;    /* The instance of the script. */
	EVENT_DATA *next;
};


/* interpreter.c */
struct script_data
{
   SCRIPT_DATA *        next;       
   int               type;       /* When is this script triggered? */
   char *               commands;   /* The code */
   char *               name;       
   int                  vnum;
   ZONE_DATA *          zone;
};


/* function.c */
struct instance_data
{
   INSTANCE_DATA *      next;
   SCRIPT_DATA *        script;            /* From Index                 */
   VARIABLE_DATA *      locals;            /* Local variables            */
   char *               location;          /* Read Head                  */
   int                  state;             /* Instance State             */
   int                  wait;              /* Countdown to next parse    */
   int                  autowait;          /* Iterative countdown delay  */
   VARIABLE_DATA *      returned;          /* Returned value             */
   int                  last_conditional;  /* Last evaluated conditional */
};


struct variable_data
{
   VARIABLE_DATA *      next;
   VARIABLE_DATA *      next_master_var;   /* Mud-wide variable list */
   char *               name;              /* Variable's name */
   int               type;              /* String, Int, Float, Scene, Actor or Prop */
   void *               value;             /* Current value */
};



/*
 * Prototype for a actor.
 * This is the in-memory version of #ACTORS.   ACTORIND <- quick search key
 */
struct    actor_index_data
{
    ACTOR_INDEX_DATA *  next;
    char *              owner; // for build()
    char *              name;
    char *              short_descr;
    char *              long_descr;
    char *              description;
    SHOP_DATA *         pShop;
    ZONE_DATA *         zone;
    INSTANCE_DATA *     instances;
    VARIABLE_DATA *     globals;            /* For setting global variables */
    int                 vnum;
    int              count;
    int              race;
    int              sex;
    int              exp;                /* experience point value      */
    int              karma;              /* karma value                 */
    SPELL_BOOK_DATA     *pSpells;          
    int                 act;
    int              timer;
    int                 bonuses;
    int                 money;
    int                 bucks;
    int                 credits;
    int              size;
    int                 cost;
    int              perm_str;
    int              perm_int;
    int              perm_wis;
    int              perm_dex;
    int              perm_con;
    ATTACK_DATA *       attacks         [MAX_ATTACK_DATA];
    SKILL_DATA         *learned;
    int              fmode;
};


struct    skill_data
{
    SKILL_DATA *next;
    int         vnum;                   /* Vnum                         */
    char *      name;                   /* Name of skill                */

    int      level;                  /* level required to learn/use  */

    int      skill_level;            /* max learn from teacher       */

                                        /* This variable, when used in
                                           PLAYER_DATA's learned list,
                                           holds the % learned of the
                                           skill. */

    int      skill_time;             /* time since last learn (player) */
    int      last_used;              /* time since last use (player) */
    int         teacher;

    int      target;                 /* Legal targets                */
    int      minimum_position;       /* Position for caster / user   */

    char *      msg_off;                /* Wear off message/to Victim   */

    int      group_code;             /* Group vnum 0=none            */
    int      required_percent;       /* % required in group          */

    int      req_str;                /* Required stats               */
    int      req_int;
    int      req_wis;
    int      req_dex;
    int      req_con;

    int      slot;                   /* Slot for props               */

    int      delay;                  /* Waiting time after use       */
    int      max_prac;               /* Max % gained in a practice   */
    int      max_learn;              /* Max % gained overall         */
    int      learn_rate;             /* Game-hours between learns    */
    int      cost;                   /* cost (in copper) to learn    */
};


struct spell_data
{ 
       SPELL_DATA *next;
       int     vnum;
       char   *name;
       int     group_code;
       int  level;

       int  mana_cost;
       int  mana_type;  /* see bit.c */
       int  delay;
       int  target;

       int  slot;       /* special link for objects */

       int     minimum_position; 
       INSTANCE_DATA *instances;
};

struct spell_book_data
{
       SPELL_BOOK_DATA *next;
       char* name;
       int vnum;
};


/*
 * Player name list structure.
 */
struct pc_name
{
        PC_NAME *      next;
        char *         name;
};

struct alias_data {
	char *name;
	char *exp;
	ALIAS_DATA *next;
};

struct command_queue {
	char *command;
	COMMAND_QUEUE *next;
};

/*manifesto*/
/*****************************************************************
 * hirelings                                                     *
 *****************************************************************/

/*****************************************************************
 * Castle Combat Tactics                                         *
 *                                                               *
 * A castle may be attacked at any time during the Spring and    *
 * Summer months, and may not be attacked in Autumn or Winter.   *
 *                                                               *
 * During combat, a portion of the vassals engage the enemy      *
 * at the enemy's stronghold, and a portion remain behind.       *
 *                                                               *
 * It is possible that during an assault, neighboring allied     *
 * kingdoms will mount a counter-assault on the lord's           *
 * stronghold, so increasing the number of castles and the       *
 *                                                               *
 * Ruling the World, or a Majority of It                         *
 *                                                               *
 * The player with the largest number of lands is perfectly      *
 * invincible and is deemed Emperor.  Emperors are given the     *
 * ability to commit benevolent acts of God, control the weather *
 * and hodge-podge like this.                                    *
 *                                                               *
 * The 'worth' command shows the value of the Castle and its     *
 * associated assets, which are sold with the Castle.            *
 *                                                               *
 *****************************************************************/

/*
 * Advanced war machine data.
 */

struct castle_data {

    CASTLE_DATA *next;

    int vnum;             /* Castle VNUM                        */

  /* Kingdom */
    int peasants;      /* Number of farming peasants         */
    int harvest;       /* Excess goods harvested and levied  */
    int tax;           /* Tax rate, a percentage             */
    int acreage;       /* Size of the kingdom's active farms */

  /* Banner */
    char   banner[3];     /* A 3 symbol flag like: [X> or [(]   */
    int    fgcolors[3];   /* foreground colors                  */
    int    bgcolors[3];   /* background colors                  */

  /* Castle */
    int defense;          /* Defense accessories                */
    char   *name;         /* Player-defined Castle Name         */
    int year_founded;     /* The year the castle was built      */
    int walls;         /* A defensive statistic like hp      */
    int armoury;       /* An offensive statistic             */
    int seige;         /* Size of your seige battery         */
    int connected;     /* VNUM Castle is connected in        */
    int size;          /* Size of castle commissioned        */

  /* Seige Army */
    int cannon;        /* Number of cannon                   */
    int trebuchet;     /* Number of trebuchet                */
    int catapult;      /* Number of catapults                */

  /* Standard army */
    int archers;       /* Number of readied archers          */
    int vassals;       /* Number of vassals                  */

  /* Special units */
    int cavaliers;     /* Number of cavaliers                */
    int assassin;      /* Number of hired assassins          */
    int gemmasters;    /* Number of gemmaster war-mages      */
};

/*
 * Associated equations
 */

#define DEFENSE(c)   ( ( IS_SET(c->defense,DEF_OIL) ? 1 : 0 +       \
                       IS_SET(c->defense,DEF_MOAT) ? 1 : 0 +        \
                       IS_SET(c->defense,DEF_SPIKES) ? 1 : 0 ) *    \
                       (size*c->walls + c->armoury + c->archers*2   \
                       + c->vassals + c->cavaliers*2 + c->cannon) )

#define OFFENSE(c)   ( (c->cannon*2) + c->catapult + c->trebuchet +  \
                        c->assassin + c->vassals + c->cavaliers +    \
                        c->archers + c->gemmasters )

#define POPULATION(c)  ( c->archers + c->cavaliers + c->vassals + \
                         c->assassin + c->gemmasters + c->peasants + 1 )

#define UPKEEP(c)    ( (c->size)    * POPULATION(c)  )

#define TAXES(c)     ( (c->peasants * c->rate) / 100 )

#define HARVEST(c)   ( (c->acreageb * c->rate) / 100 )

/*
 * Costs associated with castles:
 *
 * Moat 200*size
 * Oil 100*size
 * Spikes 50*size
 */


/*
 * Commands associated with castles:
 *
 * Spy: expends 1 assassin, reports back on foreign non-aligned kingdoms
 * Parley: sends a decree to a neighboring kingdom, expends 1 vassal
 * Taxes: changes taxation rate
 * Build/commission: orders construction of a new castle or component,
 *                   hires vassals or special units
 * Kingdom: reports information about the status of your kingdom
 */

/*
 * Castle sizes.
 */

#define SIZE_HALL     0
#define SIZE_KEEP     1
#define SIZE_CASTLE   2
#define SIZE_FORTRESS 3

/*
 * Castle defenses.
 */

#define DEF_OIL    1
#define DEF_MOAT   2
#define DEF_SPIKES 4

/*
 * Memory saver - data only PC's have.    PCDATA
 */
struct user_data
{
    USER_DATA *         next;
    char      tell_last[MSL];
    char      chat_last[MSL];
    char      say_last[MSL];
    char      immt_last[MSL];

    char      tell_buf[MSL];
    char      chat_buf[MSL];
    char      ooc_buf[MSL];
    char      say_buf[MSL];
    char      immt_buf[MSL];

    char      * flag;        // 3 printable characters + color codes 

#if defined(IMC)
    IMC_CHARDATA *      imcchardata;
#endif

    time_t              logon;
    time_t              save_time;
    time_t              last_note;
    int                 logins;
    int              mapsize;
    int              condition       [3];
    int              deaf;                          /* channel bits       */
    int              security;                      /* builder security   */
    int              level;                         /* command security   */
    int                 exp;                           /* experience? nyi    */
    int                 stat_points; 
    char *              email;
    char *              bamfin;
    char *              bamfout;
    char *              constellation;
    char *              pwd;
    char *              title;  
    char *              denial;
    char *              history;
    char *              startup;     /* commands run when a player logs in */
    int                 wizinvis;
    int                 age;
    int                 played;
    int                 home;
    int                 death;
    int              colors          [MAX_COLORS];
    int              birth_day;
    int              birth_month;
    int              birth_year;
    int                 warmth_rating;
    INSTANCE_DATA       *d_owner        [MAX_DEBUG];   /* debugger */
    INSTANCE_DATA       *d_type         [MAX_DEBUG];   /* debugger */
    SCRIPT_DATA        *trace;                         /* script trace gen   */
    int                 trace_wait;
    void *              trackscr;
    int                 trackscr_type;
    ALIAS_DATA         *aliases;                       /* player defined aliases */
    int              app_time;                      /* for delayed approval system */
};

/*
 * One character (PC or NPC).   CHDATA  <-quick search key
 */
struct player_data
{
    PLAYER_DATA *         next;           /* For actor_list              */
    PLAYER_DATA *         next_in_scene;  /* For scene->people           */
    CONNECTION_DATA *   desc;             /* Channel Connection Data     */
    COMMAND_QUEUE  *    commands;         /* For queue-based commands    */

    PLAYER_DATA *         master;         /* For following               */
    PLAYER_DATA *         leader;
    PLAYER_DATA *         monitor;
    PLAYER_DATA *         fighting;       /* For combat                  */
    PLAYER_DATA *         riding;         /* For mount                   */
    PLAYER_DATA *         rider;          /* For being mounted           */
    PLAYER_DATA *         reply;
    USER_DATA *           userdata;       /* Additional PC only info     */
    PROP_DATA *           hitched_to;     /* For hitching/dragging       */
    PROP_DATA *           furniture;      /* For sitting/sleeping/wagons */
    char *                tracking;       /* For track                   */
    char *                hunting;        /* For track                   */
    char *                owner;          /* For hireling                */

    INSTANCE_DATA *      instances;       /* scripts                     */
    INSTANCE_DATA *      current;         /* scripts                     */
    VARIABLE_DATA *      globals;

    ACTOR_INDEX_DATA *  pIndexData;       /* For NPCs                    */
    BONUS_DATA *        bonus;            /* Special affects.            */
    NOTE_DATA *         pnote;            /* For bulletin boards.        */
    PROP_DATA *         carrying;         /* For inventory/equipment     */
    SCENE_INDEX_DATA *  in_scene;         /* To be in a location.        */
    SPELL_BOOK_DATA *   spells;           /* Learned spells.             */

    SKILL_DATA *        learned;          /* Skill List                  */

    char *              name;             /* name (matchable keywords)   */
    char *              keywords;         /* matchable keywords ()       */
    char *              hkeywords;        /* matchable hooded keywords   */
    char *              short_descr;      /* short description           */
    char *              long_descr;       /* long description            */
    char *              description;

    char *              prompt;
    int              sex;
    int              race;
    int              timer;
    int              wait;
    int              hit;
    int              move;
    int              mana;
    int              karma;                 /* Karma              */
    int              credits;               /* credits for scifi part */
    int              bucks;                 /* bucks for real part */
    int                 exp;                   /* Experience         */  
    int              exp_level;             /* Exp Level          */
    int                 bounty;                /* Price on your head */
    int                 owed;                  /* Rewards owed       */
    int                 act;
    int                 act2;
    int                 fbits;                 /* Fighting bits      */
    int              fmode;                 /* fmode              */
    int                 bonuses;
    int              position;
    int              carry_weight;
    int              carry_number;
    int              saving_throw;
    int              hitroll;
    int              damroll;
    int              armor;
    int              size;
    int              pagelen;
    int              perm_str;
    int              perm_int;
    int              perm_wis;
    int              perm_dex;
    int              perm_con;
    int              mod_str;
    int              mod_int;
    int              mod_wis;
    int              mod_dex;
    int              mod_con;

    PLAYER_DATA *       keeper;
    PROP_DATA *         haggling;
    int                 haggled_cost;
    int              haggle_bits;
    int                 original_cost;
    int              speaking;
};



/*
 * Liquids.
 */
#define LIQ_WATER        0
#define LIQ_OIL         17
#define LIQ_MAX         34

struct    liq_type
{
    char *    liq_name;
    char *    liq_color;
    int    liq_bonus[3];     /* 0=drunk  1=food  2=thirst */
};

/*
 * Extra description data for a scene or prop.
 */
struct    extra_descr_data
{
    EXTRA_DESCR_DATA *next;    /* Next in list                     */
    char *keyword;              /* Keyword in look/examine          */
    char *description;          /* What to see                      */
};


/*
 * Prototype for an prop.         OBJIND
 */
struct    prop_index_data
{
    PROP_INDEX_DATA *   next;
    EXTRA_DESCR_DATA *  extra_descr;
    BONUS_DATA *        bonus;
    ZONE_DATA *         zone;
    INSTANCE_DATA *     instances;
    char *              owner; // for build()
    char *              name;
    char *              short_descr;
    char *              short_descr_plural;
    char *              description;
    char *              description_plural;
    char *              action_descr;
    char *              real_description;
    int                 vnum;
    int              item_type;
    int              timer;
    int                 extra_flags;
    int                 wear_flags;
    int              count;
    int              weight;
    int              level;
    int                 cost;
    int                 size;
    int              material;
    int              material_value;
    int                 value   [4];
};





/*
 * One prop.                            OBDAT
 */
struct    prop_data
{
    PROP_DATA *         next;
    PROP_DATA *         next_content;
    PROP_DATA *         contains;
    PROP_DATA *         in_prop;
    PLAYER_DATA *       carried_by;
    EXTRA_DESCR_DATA *  extra_descr;
    BONUS_DATA *        bonus;
    PROP_INDEX_DATA *   pIndexData;
    SCENE_INDEX_DATA *  in_scene;
    INSTANCE_DATA *     instances;           /* for scripts */
    INSTANCE_DATA *     current;
    VARIABLE_DATA *     globals;            /* for scripts */
    char *              name;
    char *              short_descr;
    char *              short_descr_plural;
    char *              description;
    char *              description_plural;
    char *              action_descr;
    char *              real_description;
    int              item_type;
    int                 extra_flags;
    int                 wear_flags;
    int              wear_loc;
    int              weight;
    int                 cost;
    int              level;
    int              timer;
    int              size;
    int              material;
    int              material_value;
    int                 value   [4];
};                                                   


/*
 * Exit data.
 */
struct    exit_data
{
    SCENE_INDEX_DATA *    to_scene;
    EXIT_DATA *         next;
    int                 vnum;
    int              exit_info;
    int                 key;
    char *              keyword;
    char *              description;
    int              rs_flags;
};



/*
 * Cues or Spawn commands (referred to as cue in the help text and command):
 *   '*': comment
 *   'M': read a actor 
 *   'O': read an prop  (figures out where to put it)
 */

/*
 * zone-reset definition.
 */
struct    spawn_data
{
    SPAWN_DATA *    next;
    char                command; /* M=Mob O=Obj                              */
    int              rs_vnum; /* vnum of actor/prop to load                  */
    int                 loc;     /* If actor = max actors else wear_loc or below */
    int              percent; /* chance it will load                      */
    int              vnum;
    int              num;     /* num = num to load                        */
};


#define SPAWN_LOC_INSIDE    -3      /* prop loads as if a content of prop    */
#define SPAWN_LOC_ONTOP    -4      /* prop loads as if sitting on prop      */
#define SPAWN_LOC_INSCENE   -5      /* just let it load in the scene           */


/*
 * zone definition.
 */
struct    zone_data
{
    ZONE_DATA *         next;
    char *              name;
    char *              repop;
    int              zone_flags;
    int              security;
    char *              builders;
    int                 lvnum;
    int                 uvnum;
    int              age;
    int              nplayer;
    char *              filename;
    int                 vnum;
};

/*
 * Scene type.                                       SCENEIND
 */
struct    scene_index_data
{
    SCENE_INDEX_DATA *   next;
    SPAWN_DATA *         spawn_first;
    SPAWN_DATA *         spawn_last;
    SCENE_INDEX_DATA *   leave_to;
    PLAYER_DATA *        people;
    PROP_DATA *          contents;
    EXTRA_DESCR_DATA *   extra_descr;
    ZONE_DATA *          zone;
    EXIT_DATA *          exit    [MAX_DIR];
    INSTANCE_DATA *      instances;           /* for scripts */
    INSTANCE_DATA *      current;
    VARIABLE_DATA *      globals;            /* for scripts */
    char *               owner; // for build()
    char *               name;
    char *               description;
    char *               client;             /* client code                 */
    int                  vnum;
    int                  position;       /* vrml */
    int                  template;
    int                  scene_flags;
    int               max_people;
    int               light;
    int               sector_type;
    int                  terrain;
    int                  wagon;
};


/*
 * Mana types.
 * These are bits for GEMs, these are also used to check if a spell needs
 * that certain mana type.  Used in def.c under skill defines.

"aquatic mounts"?
 */

#define MANA_ANY           -1
#define MANA_NONE           0
#define MANA_EARTH          1
#define MANA_AIR            2
#define MANA_FIRE           4
#define MANA_WATER          8

#define M_ANY              -1
#define MN                  0
#define ME                  1
#define MA                  2
#define MF                  4
#define MW                  8


/*
 *  Target types.
 */
#define TAR_IGNORE            0
#define TAR_CHAR_OFFENSIVE    1
#define TAR_CHAR_DEFENSIVE    2
#define TAR_CHAR_SELF         3
#define TAR_PROP_INV          4


struct  group_type
{
    char *      name;                   /* Name of group                */
    int      group_code;             /* Some neato-code (group slot) */
    int      pgsn;                   /* Pointer to percent skill gsn */
};




/*
 * Structure for a command in the command lookup table.
 */
struct    cmd_type
{
    char * const    name;
    CMD_FUN *            cmd_fun;
    int        position;
    int        level;
    int        log;
};



/*
 * Structure for a social in the socials table.  (Read from zones)
 */
struct    social_type
{
    char      name[20];
    char *    actor_no_arg;
    char *    others_no_arg;
    char *    actor_found;
    char *    others_found;
    char *    vict_found;
    char *    actor_not_found;
    char *    actor_auto;
    char *    others_auto;
};




/*
 * Utility macros.
 */
#define interpolate(l, v0, v32) ( v0 + l * (v32-v0) / 32 )
#define INTERPOLATE(min, max, percent)   ( min + (max - min / percent == 0 ? 1 : percent ) )
#define UMIN(a, b)        ((a) < (b) ? (a) : (b))
#define UMAX(a, b)        ((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)        ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)        ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)        ((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_VOWEL(c)             (c == 'A' || c == 'a' ||                     \
                                 c == 'E' || c == 'e' ||                     \
                                 c == 'I' || c == 'i' ||                     \
                                 c == 'O' || c == 'o' ||                     \
                                 c == 'U' || c == 'u'      )

#define IS_SET(flag, bit)    ((flag) & (bit))
#define SET_BIT(var, bit)    ((var) |= (bit))
#define REMOVE_BIT(var, bit)    ((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))
#define STR(dat, field)         (( (dat)->field != NULL                      \
                                             ? (dat)->field                  \
                                             : (dat)->pIndexData->field ))
#define NST(pointer)            (pointer == NULL ? "" : pointer)
#define MTD(str)                ((str == NULL || str[0] == '\0' ))
#define MTP(str)                ((str == NULL || *str   == '\0' ))
#define STRASN(dat1, dat2)      (free_string( dat1 );   dat1 = str_dup( dat2 ))

#define LINEATE(x,y,x_size)     ( (x+y*x_size) )
#define FINDDESC(x,y,tnum)      ( (x*y%tnum) )

/*
 * Articles.
 */
#define HE_SHE(ch)           (((ch)->sex == SEX_MALE  ) ? "he"   :  \
                           (  ((ch)->sex == SEX_FEMALE) ? "she"  : "it" ) )
#define HIS_HER(ch)          (((ch)->sex == SEX_MALE  ) ? "his"  :  \
                           (  ((ch)->sex == SEX_FEMALE) ? "her"  : "its" ) )
#define HIM_HER(ch)          (((ch)->sex == SEX_MALE  ) ? "him"  :  \
                           (  ((ch)->sex == SEX_FEMALE) ? "her"  : "it" ) )

/*
 * Character macros.
 */
#define IS_NPC(ch)        (IS_SET((ch)->act,ACT_IS_NPC))
#define IS_IMMORTAL(ch)         (get_trust(ch) >= LEVEL_IMMORTAL)
#define IS_HERO(ch)        (get_trust(ch) >= LEVEL_HERO)
#define IS_AFFECTED(ch, sn)    (IS_SET((ch)->bonuses, (sn)))

#define IS_LYCANTHROPIC(ch)  (IS_SET((ch)->act, ACT_LYCANTHROPE))

#define IS_VAMPIRE(ch)       (IS_SET((ch)->act, ACT_VAMPIRE))

#define SHOW_TIPS(ch)     (!IS_NPC(ch) && IS_SET(ch->act2, PLR_TIPS) )
#define HAS_ANSI(ch)            (!IS_NPC(ch) && IS_SET(ch->act2, PLR_ANSI) && \
                             ch->desc && ch->desc->client != TRUE && ch->desc->lingua==0)
#define HAS_CLIENT(ch)          (!IS_NPC(ch) && ch->desc && ch->desc->client)
#define IS_AWAKE(ch)        (ch->position > POS_SLEEPING)
#define PC(ch,field)            ((ch)->userdata->field)
#define GET_PC(ch,field,nopc)   ((ch->userdata != NULL ? ch->userdata->field     \
                                                     : (nopc)) )
#define GET_AC(ch)              ((ch)->armor                                 \
                                    + ( IS_AWAKE(ch)                         \
                                    ? dex_app[get_curr_dex(ch)].defensive    \
                    : 0 ))
#define GET_HITROLL(ch)        ((ch)->hitroll+str_app[get_curr_str(ch)].tohit)
#define GET_DAMROLL(ch)        ((ch)->damroll+str_app[get_curr_str(ch)].todam)
#define MAXHIT(ch)              (get_curr_con(ch) * 10)
#define MAXMOVE(ch)             (get_curr_con(ch) * 20 + 300)
#define MAXMANA(ch)             (get_curr_int(ch) * 15 + 100)
#define GET_AGE(ch)             (IS_NPC(ch) ? 17 :                           \
                                       (weather_info.year - PC(ch,birth_year)))

#define IS_OUTSIDE(ch)          (!IS_SET(                                    \
                                    (ch)->in_scene->scene_flags,               \
                    SCENE_INDOORS))

#define WAIT_STATE(ch, npulse)  ((ch)->wait = UMAX((ch)->wait, (npulse)))

#define MANA_COST(ch, sn)       (IS_NPC(ch) ? 0                              \
                                 : number_fuzzy(skill_table[sn].min_mana) )

#define STRING_HITS(ch)         (percent_hit[ URANGE(0,                      \
                                     PERCENTAGE((ch)->hit, MAXHIT(ch))       \
                                                   ,101)/10 ]  )             \

#define STRING_MOVES(ch)        (percent_tired[ URANGE(0,                    \
                                     PERCENTAGE((ch)->move, MAXMOVE(ch))     \
                                                   ,101)/10 ]  )

#define STRING_MANA(ch)        (percent_mana[ URANGE(0,                    \
                                     PERCENTAGE((ch)->mana, MAXMANA(ch))     \
                                                   ,101)/10 ]  )

#define HAS_SCRIPT(ch)          ( ch->instances != NULL )
#define BTW(align)       ( (align) >= race_table[ch->race].minalign &&       \
                           (align) <= race_table[ch->race].maxalign )
#define SAME_ALIGN(ch, victim)  ( (IS_GOOD( ch ) && IS_GOOD( victim ))       \
                               || (IS_EVIL( ch ) && IS_EVIL( victim ))       \
                               || (IS_NEUTRAL(ch) && IS_NEUTRAL( victim )) )
#define TRACK_DEPTH(ch)         ( IS_NPC(ch) ? 50 : 10 )

/*
 * Object macros.
 */

/*
 * TOO_BIG -> Its too big for you to wear.
 * TOO_SMALL -> Its too small for you to wear.
 */
#define TOO_SMALL(ch,prop)    ( (prop->size) && (ch->size - prop->size) >  4 \
                                ? TRUE : FALSE )
#define TOO_BIG(ch,prop)      ( (prop->size) && (ch->size - prop->size) < -4 \
                                ? TRUE : FALSE )
#define CAN_WEAR(prop, part)     (IS_SET((prop)->wear_flags,  part))
#define IS_PROP_STAT(prop, stat)  (IS_SET((prop)->extra_flags, stat))
#define NOT_WORN( prop )         ((prop)->wear_loc == WEAR_NONE   ||     \
                                 (prop)->wear_loc == WEAR_HOLD_1 ||     \
                                 (prop)->wear_loc == WEAR_HOLD_2 ||     \
                                 (prop)->wear_loc == WEAR_WIELD_1 ||     \
                                 (prop)->wear_loc == WEAR_WIELD_2 ||     \
                                 (prop)->wear_loc == WEAR_BELT_1 ||     \
                                 (prop)->wear_loc == WEAR_BELT_2 ||     \
                                 (prop)->wear_loc == WEAR_BELT_3 ||     \
                                 (prop)->wear_loc == WEAR_BELT_4 ||     \
                                 (prop)->wear_loc == WEAR_BELT_5    )
#define IS_WORN( prop )          ( !NOT_WORN( prop ) )
#define IS_LIT( prop )           ( (prop)->value[0] > 0 &&               \
                                 IS_SET((prop)->value[3], LIGHT_LIT) )
#define VAL_SET(prop, num, bit)  (IS_SET((prop)->value[num], bit))
#define OCCUPADO(prop)           ( prop->value[0]/100 <= 0               \
                                || count_occupants(prop) >= prop->value[0]/100 )

/*
 * Description macros.
 */
#define NAME( ch )          ( is_hooded( (ch) ) ? "the hooded figure" : \
                             (IS_NPC(ch)                                 \
                            || ((ch)->short_descr != NULL                 \
                              && !MTD((ch)->short_descr)                 \
                              && !str_cmp((ch)->short_descr, "(null)")  \
                              && !str_cmp((ch)->short_descr, ""      ) ) \
                            ? STR(ch,short_descr) : (ch)->name) )

#define PERS(ch, looker)     ( can_see( looker, (ch) ) ? NAME(ch) : "someone" )
#define PERSO(prop, looker)   ( can_see_prop(looker, prop) ? format_prop_to_actor(prop,looker,TRUE): "something" )

/*
 * Other macros.
 */
#define NOTIFY(buf, lvl, tog)       ( global( buf, lvl, tog, WIZ_NOTIFY ) )

/*
DECLARE_SPEC_FUN(    spec_breath_any         );
DECLARE_SPEC_FUN(    spec_breath_acid        );
DECLARE_SPEC_FUN(    spec_breath_fire        );
DECLARE_SPEC_FUN(    spec_breath_frost       );
DECLARE_SPEC_FUN(    spec_breath_gas         );
DECLARE_SPEC_FUN(    spec_breath_lightning   );
DECLARE_SPEC_FUN(    spec_cast_adept         );
DECLARE_SPEC_FUN(    spec_cast_cleric        );
DECLARE_SPEC_FUN(    spec_cast_judge         );
DECLARE_SPEC_FUN(    spec_cast_mage          );
DECLARE_SPEC_FUN(    spec_cast_undead        );
DECLARE_SPEC_FUN(    spec_executioner        );
DECLARE_SPEC_FUN(    spec_fido               );
DECLARE_SPEC_FUN(    spec_guard              );
DECLARE_SPEC_FUN(    spec_janitor            );
DECLARE_SPEC_FUN(    spec_mayor              );
DECLARE_SPEC_FUN(    spec_poison             );
DECLARE_SPEC_FUN(    spec_thief              );
 */

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if    defined(_AIX)
char *    crypt        args( ( const char *key, const char *salt ) );
#endif

#if    defined(apollo)
int    atoi        args( ( const char *string ) );
void *    calloc        args( ( unsigned nelem, size_t size ) );
char *    crypt        args( ( const char *key, const char *salt ) );
#endif

#if    defined(hpux)
char *    crypt        args( ( const char *key, const char *salt ) );
#endif

#if    defined(interactive)
#endif

#if    defined(linux)
char *    crypt        args( ( const char *key, const char *salt ) );
#endif

#if    defined(macintosh)
#define NOCRYPT
#if    defined(unix)
#undef    unix
#endif
#endif

#if    defined(MIPS_OS)
char *    crypt        args( ( const char *key, const char *salt ) );
#endif

#if    defined(MSDOS)
#define NOCRYPT
#if    defined(unix)
#undef    unix
#endif
#endif

#if    defined(NeXT)
char *    crypt        args( ( const char *key, const char *salt ) );
#endif

#if    defined(sequent)
char *    crypt        args( ( const char *key, const char *salt ) );
int    fclose        args( ( FILE *stream ) );
int    fprintf        args( ( FILE *stream, const char *format, ... ) );
int    fread        args( ( void *ptr, int size, int n, FILE *stream ) );
int    fseek        args( ( FILE *stream, long offset, int ptrname ) );
void    perror        args( ( const char *s ) );
int    ungetc        args( ( int c, FILE *stream ) );
#endif

#if    defined(sun)
char *    crypt        args( ( const char *key, const char *salt ) );
int    fclose        args( ( FILE *stream ) );
int    fprintf        args( ( FILE *stream, const char *format, ... ) );
#if     defined(SYSV)
size_t     fread        args( ( void *ptr, size_t size, size_t n,
                FILE *stream ) );
#else
/*int     fread           args( ( void *ptr, int size, int n, FILE *stream ) );*/
#endif
int    fseek        args( ( FILE *stream, long offset, int ptrname ) );
void    perror        args( ( const char *s ) );
int    ungetc        args( ( int c, FILE *stream ) );
#endif

#if    defined(ultrix)
char *    crypt        args( ( const char *key, const char *salt ) );
#endif

/*
 * Copyover by Erwin Andreasen.
 */
void copyover_recover  args( ( void ) );


/*
 * Data files used by the server.
 *
 * ZONE_LIST contains a list of zones to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other connections.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#if defined(macintosh)
#define PLAYER_DIR    ""              /* Player files            */
#define BOARD_DIR     ""              /* Board files                  */
#define NULL_FILE    "proto.zone"     /* To reserve one stream    */
#endif

#if defined(MSDOS)
#define PLAYER_DIR    ""        /* Player files                 */
#define BOARD_DIR       ""      /* Board files                  */
#define NULL_FILE    "nul"      /* To reserve one stream        */
#endif

#if defined(unix)
#define PLAYER_DIR    "../player/"    /* Player files            */
#define BOARD_DIR       "./board/"    /* Board files             */
#define NULL_FILE       "/dev/null"   /* To reserve one stream   */
#endif

#if defined(HTML)
#define HTML_DIR      "../export/html/"      /* Path to html export dir  */
#endif

#define SCRIPT_DIR    "../export/scripts/"   /* path to script export dir */
#define SOURCE_DIR    "../src/"              /* path to the source code, used by helps */

#if defined(linux)
#define PLAYER_DIR    "../player/"    /* Player files             */
#define BOARD_DIR       "./board/"    /* Board files              */
#define NULL_FILE    "/dev/null"      /* To reserve one stream    */
#endif

#define GREETING_FILE   "greeting.txt"   /* Greeting                    */
#define EXEC_FILE       "../src/current" /* Executable                  */  
#define MUD_FILE        "config.db"      /* Mud time config.            */
#define SCENES_FILE     "appendix.db"    /* Scene contents save file.   */
#define SOCIALS_FILE    "socials.hlp"    /* Socials file.               */
#define ZONE_LIST       "zone.lst"       /* List of zones               */
 
#define BUG_FILE       "bugs.txt"        /* For 'bug' and bug( )  */
#define IDEA_FILE      "ideas.txt"       /* For 'idea'            */
#define TYPO_FILE      "typos.txt"       /* For 'typo'            */
#define SHUTDOWN_FILE  "shutdown.txt"    /* For 'shutdown'        */

/*
 * Main global declarations.
 */
 
#define CD      PLAYER_DATA
#define MID     ACTOR_INDEX_DATA
#define OD      PROP_DATA
#define OID     PROP_INDEX_DATA
#define RID     SCENE_INDEX_DATA
#define SF      SPEC_FUN
#define AD      ZONE_DATA
#define VARD    VARIABLE_DATA
#define RD      SPAWN_DATA
#define ID      INSTANCE_DATA
#define SD      SHOP_DATA
#define EDD     EXTRA_DESCR_DATA
#define AFD     BONUS_DATA
#define ED      EXIT_DATA
#define ATD     ATTACK_DATA
#define PCD     USER_DATA
#define SCD     SCRIPT_DATA
#define DD      CONNECTION_DATA

/*****************************************************************************
 * Variable and functional contents for file config.c                        *
 * Contains mostly user-specific options for connection and gameplay.        *
 *****************************************************************************/
extern char *default_color_variable;     /* make sure to reset to NTEXT */
extern char *default_color_variable_di;     /* make sure to reset to NTEXT */

COMMAND( cmd_clear     );
COMMAND( cmd_tips      );
COMMAND( cmd_blank     );
COMMAND( cmd_brief     );
COMMAND( cmd_compact   );
COMMAND( cmd_holylight );
COMMAND( cmd_pager     );
COMMAND( cmd_prompt    );
COMMAND( cmd_password  );
COMMAND( cmd_ansi      );
COMMAND( cmd_config    );
COMMAND( cmd_notify    );
COMMAND( cmd_client    );

/* alias.c */
void add_alias    args( ( PLAYER_DATA *ch, char *name, char *exp ) );
ALIAS_DATA *find_alias  args( ( PLAYER_DATA *ch, char *exp ) );
COMMAND( cmd_alias     );

COMMAND( cmd_flag      );

/*****************************************************************************
 * Variable and functional contents for file net.c                           *
 * Contains mostly in-game communication routines, not network parameters.   *
 *****************************************************************************/

void error_handler  args( ( int s ) );

void save_copyover  args( ( void ) );
void gen_bar_graph  args( ( PLAYER_DATA *ch, int percent ) );
void sendbuf        args( ( CONNECTION_DATA *d, const char *txt, int length ) );
 
COMMAND( cmd_email     );
COMMAND( cmd_immtalk   );
COMMAND( cmd_chat      );
COMMAND( cmd_tell      );
COMMAND( cmd_reply     );
COMMAND( cmd_whisper   );
COMMAND( cmd_wish      );
void shout         args( ( CD *ch, CD *pScene, char *message, int dir ) );
COMMAND( cmd_shout     );
void say_to        args( ( PLAYER_DATA *ch, PLAYER_DATA *victim, char *argument ));
COMMAND( cmd_say       );
COMMAND( cmd_talk      );
COMMAND( cmd_smote     );
COMMAND( cmd_speak     );
COMMAND( cmd_ooc       );
COMMAND( cmd_emote     );
COMMAND( cmd_bug       );
COMMAND( cmd_idea      );
COMMAND( cmd_typo      );
COMMAND( cmd_qui       );
COMMAND( cmd_quit      );
COMMAND( cmd_save      );
COMMAND( cmd_follow    );
COMMAND( cmd_dismiss   );
COMMAND( cmd_group     );
COMMAND( cmd_monitor   );
COMMAND( cmd_gtell     );

// social.c

bool in_group      args( ( PLAYER_DATA *ch, PLAYER_DATA *victim ) );
void ungroup       args( ( PLAYER_DATA *ch ) );
void add_follower  args( ( PLAYER_DATA *ch, PLAYER_DATA *master ) ); 
void stop_follower args( ( PLAYER_DATA *ch ) );
void die_follower  args( ( PLAYER_DATA *ch ) );
COMMAND( cmd_order     );
void    print_stat_menu    args( ( PLAYER_DATA *ch ) );

void 	wtf_logf 			args((char * fmt, ...));
void 	var_to_actor 			args((char * fmt, PLAYER_DATA *ch, ...));

/*****************************************************************************
 * Variable and functional contents for file info.c                          *
 * Contains mostly informational routines for the player.                    *
 *****************************************************************************/

extern char *  const   where_name  [];

char *time_color           args( ( PLAYER_DATA *ch ) ); 
void scan_direction        args( ( PLAYER_DATA *ch, int dir ) );
char *format_prop_to_actor args( ( OD *prop, CD *ch, bool fShort ) );
void show_scene_to_actor   args( ( PLAYER_DATA *ch, SCENE_INDEX_DATA *pScene,
                                   int dist, int dir ) );
void show_list_to_actor    args( ( OD *list, CD *ch, bool fShort,
                                  bool fShowNothing ) );
char *show_list_to_actor2  args( ( OD *list, CD *ch, char *prefix ) );
void show_peek_to_actor    args( ( OD *list, CD *ch, bool fShort,
                                  bool fShowNothing, int percent ) );
  
void show_equipment        args( ( PLAYER_DATA *ch, PLAYER_DATA *tch ) );
void show_equipment_table  args( ( PLAYER_DATA *ch, PLAYER_DATA *victim ) );
void show_size             args( ( PLAYER_DATA *ch, PLAYER_DATA *tch ) );

void show_actor_to_actor    args( ( PLAYER_DATA *list, PLAYER_DATA *ch ) );
void show_actor_to_actor_0  args( ( PLAYER_DATA *list, PLAYER_DATA *ch ) );
void show_actor_to_actor_1  args( ( PLAYER_DATA *list, PLAYER_DATA *ch ) );

bool check_blind          args( ( PLAYER_DATA *ch ) );
bool actor_look_list      args( ( PLAYER_DATA *ch, PROP_DATA *list, char *arg ) );
void help_to_actor        args( ( char *t, PLAYER_DATA *ch, bool fPage ) );
char *get_name_level  args( ( int level ) );

COMMAND( cmd_scan      );
COMMAND( cmd_look      );
COMMAND( cmd_examine   );
COMMAND( cmd_exits     );
COMMAND( cmd_title     );
COMMAND( cmd_score     );
COMMAND( cmd_time      );
COMMAND( cmd_help      );
COMMAND( cmd_who       );
void show_inventory       args( ( CD *ch, CD *tch, bool fPeek ) );
COMMAND( cmd_inventory );
void show_belt            args( ( PLAYER_DATA *victim, PLAYER_DATA *ch ) );
COMMAND( cmd_peek      );
COMMAND( cmd_equipment );
COMMAND( cmd_compare   );
COMMAND( cmd_credits   );
COMMAND( cmd_consider  );
COMMAND( cmd_report    );
COMMAND( cmd_socials   );
COMMAND( cmd_commands  );
COMMAND( cmd_wizlist   );
COMMAND( cmd_history   );
void add_history         args( ( PLAYER_DATA *ch, char *_out ) );

/*****************************************************************************
 * Variable and functional contents for file move.c                          *
 * Contains movement and positional scripts, and some skill routines.        *
 *****************************************************************************/
 
extern char *  const   dir_name        [];
extern char *  const   dir_rev         [];
extern char *  const   dir_letter      [];
extern const   int  rev_dir         [];
extern const   int  movement_loss   [SECT_MAX];

void hide_check      args( ( PLAYER_DATA *ch, SCENE_INDEX_DATA *in_scene ) );
/* void flee            args( ( PLAYER_DATA *ch, PLAYER_DATA *fighting, int d ) ); */
void leave_strings   args( ( CD *ch, OD *prop, int sect, int door, bool fWindow ) );
void arrive_strings  args( ( CD *ch, OD *prop, int sect, int door, bool fWindow ) );
bool lose_movement   args( ( CD *ch, RID *in_scene, RID *to_scene ) );
bool check_move      args( ( CD *ch, int door, int depth, RID *in_scene,
                             RID *to_scene, OD **prop ) );
void move_char       args( ( PLAYER_DATA *ch, int door ) );
COMMAND( cmd_north     );
COMMAND( cmd_east      );
COMMAND( cmd_south     );
COMMAND( cmd_west      );
COMMAND( cmd_up        );
COMMAND( cmd_down      );
COMMAND( cmd_nw        );
COMMAND( cmd_ne        );
COMMAND( cmd_sw        );
COMMAND( cmd_se        );
COMMAND( cmd_drag      );
COMMAND( cmd_hitch     );
COMMAND( cmd_unhitch   );
COMMAND( cmd_enter     );
COMMAND( cmd_leave     );
COMMAND( cmd_stand     );
COMMAND( cmd_rest      );
COMMAND( cmd_sit       );
COMMAND( cmd_sleep     );
COMMAND( cmd_wake      );
COMMAND( cmd_sneak     );
COMMAND( cmd_hide      );
COMMAND( cmd_home      );

/*****************************************************************************
 * Variable and functional contents for file props.c                          *
 * Contains prop manipulation functions and commands.                        *
 *****************************************************************************/

void inv_to_hand_check    args( ( PLAYER_DATA *ch ) );
bool can_wield            args( ( PLAYER_DATA *ch, PROP_DATA *prop, bool fSilent ));
bool draw_prop            args( ( PLAYER_DATA *ch, PROP_DATA *prop ) );
bool sheath_prop          args( ( PLAYER_DATA *ch, PROP_DATA *prop, bool fSilent ) );
bool remove_prop          args( ( PLAYER_DATA *ch, int iWear, bool fReplace, bool fMsg ) );
int  get_wear_location    args( ( int loc ) );
void wear_prop            args( ( PLAYER_DATA *ch, PROP_DATA *prop, bool fReplace, int loc ) );

COMMAND( cmd_wear     );
COMMAND( cmd_sheath   );
COMMAND( cmd_draw     );
COMMAND( cmd_remove   );
COMMAND( cmd_wield    );
COMMAND( cmd_hold     );
int get_prop          args( ( PLAYER_DATA *ch, PROP_DATA *prop, PROP_DATA *container ) );
COMMAND( cmd_get      );
COMMAND( cmd_put      );
COMMAND( cmd_use      );
void wield_prop       args( ( PROP_DATA *prop, PLAYER_DATA *ch ) );
PROP_DATA *find_pack  args( ( PLAYER_DATA *ch, PROP_DATA *prop ) );
COMMAND( cmd_dump     );
COMMAND( cmd_drop     );
COMMAND( cmd_give     );
COMMAND( cmd_fill     );
COMMAND( cmd_pour     );
COMMAND( cmd_drink    );
COMMAND( cmd_eat      );
COMMAND( cmd_quaff    );
COMMAND( cmd_recite   );
COMMAND( cmd_brandish );
COMMAND( cmd_zap      );
COMMAND( cmd_steal    );
COMMAND( cmd_light    );
COMMAND( cmd_extinguish );
COMMAND( cmd_swap     );
COMMAND( cmd_raise    );
COMMAND( cmd_lower    );
COMMAND( cmd_write    );

/*****************************************************************************
 * Variable and functional contents for file bit.c                           *
 * Contains string-to-numeral and numeral-to-string conversions.             *
 *****************************************************************************/
 
int    target_name_bit  args( ( char *buf       ) );
char * target_bit_name  args( ( int vector ) );
int    mana_name_bit    args( ( char *buf       ) );
char * mana_bit_name    args( ( int vector ) );
char * item_type_name   args( ( int item_type   ) );
int    item_name_type   args( ( char *name      ) );
int    item_valflag     args( ( char *name      ) );
char * bonus_loc_name   args( ( int location    ) );
int    bonus_name_loc   args( ( char *name      ) );
char * bonus_bit_name   args( ( int vector      ) );
int    bonus_name_bit   args( ( char *buf       ) );
char * extra_bit_name   args( ( int extra_flags ) );
int    extra_name_bit   args( ( char *buf       ) );
char * scene_bit_name   args( ( int scene_flags  ) );
int    scene_name_bit   args( ( char *buf       ) );
char * act_bit_name     args( ( int actb        ) );
char * plr_bit_name     args( ( int actb        ) );
int    act_name_bit     args( ( char *buf       ) );
char * wear_bit_name    args( ( int wear        ) );
int    wear_name_bit    args( ( char *buf       ) );
char * wear_loc_name    args( ( int wearloc     ) );
int    wear_name_loc    args( ( char *buf       ) );
char * sector_name      args( ( int sect        ) );
int    sector_number    args( ( char *argument  ) );
char * position_name    args( ( int pos         ) );
int    name_position    args( ( char *pos       ) );
int    size_number      args( ( char *argument  ) );
char * size_name        args( ( int size        ) );
int    get_dir          args( ( char *arg       ) );
int    exit_name_bit    args( ( char *arg       ) );
char * exit_bit_name    args( ( int flag        ) );
int    get_actor_sex_number args(( char *arg       ) );
int    get_script_type  args( ( char *arg       ) );
char * show_script_type args( ( int script_type   ) );
char * shop_bit_name    args( ( int flag        ) );
int    shop_name_bit    args( ( char *name      ) );
char * name_good_code   args( ( int good_code   ) );
char * name_stat_range  args( ( int stat        ) );


/*****************************************************************************
 * Variable and functional contents for file board.c                         *
 * Contains the current bulletin board and mail systems for internal mud use.*
 *****************************************************************************/
 
/* Omitted board listings. */

void fread_board   args( ( int b ) );
void load_boards   args( ( void ) );
void note_remove   args( ( PLAYER_DATA *ch, NOTE_DATA *pnote, int b ) );
void save_board    args( ( int b ) );
bool is_note_to    args( ( PLAYER_DATA *ch, int board, NOTE_DATA *pnote ) );
void note_attach   args( ( PLAYER_DATA *ch ) );
COMMAND( cmd_note    );

/*****************************************************************************
 * Functional and variable contents for file client.c                        *
 * Contains all hard client send commands                                    *
 ****************************************************************************/

bool sendcli       args( ( CONNECTION_DATA *c, char *buf ) );

/*****************************************************************************
 * Functional and variable contents for file comm.c                          *
 * Contains all hard-comm routines with each connection.                     *
 *****************************************************************************/

/*
 * Daurven's lazy ass.
 */
#define STC(arg,ch)         ( send_to_actor(arg,ch) )                

extern CONNECTION_DATA * connection_list;
extern FILE *          fpReserve;
extern bool            newlock;
extern bool            wizlock;
extern bool            shut_down;
extern char            str_boot_time[MAX_INPUT_LENGTH];
extern time_t          current_time;
extern int             packet[60];
extern bool            QUIET_STC;

 /* Omitted: init_socket          */
void connection_output   args( ( void ) );
void process_input       args( ( void ) );
 /* Omitted: game_loop_mac_msdos  */
#if defined(unix)
void clean_connections   args( ( void ) );
void poll_connections    args( ( int c ) );
 /* Omitted: game_loop_unix       */
void new_connection      args( ( int c ) );
#endif
void close_socket        args( ( CONNECTION_DATA *dclose ) );
 /* Omitted: read_from_connection */
 /* Omitted: read_from_buffer     */
void display_interp      args( ( PLAYER_DATA *ch, const char *str ) );
void display_prompt      args( ( PLAYER_DATA *ch ) );
bool process_output      args( ( CONNECTION_DATA *d, bool fPrompt ) );
void write_to_buffer     args( ( CONNECTION_DATA *d, const char *txt, 
                                 int length ) );
bool write_to_connection args( ( int desc, char *txt, int length ) );
void parse_snoop         args( ( CONNECTION_DATA *d, const char *txt ) );
bool write_to_descr_nice args( ( CONNECTION_DATA *d ) );
void stop_idling         args( ( PLAYER_DATA *ch ) );
void send_to_actor       args( ( const char *txt, PLAYER_DATA *ch ) );
void page_to_actor       args( ( const char *txt, PLAYER_DATA *ch ) );
void show_string         args( ( CONNECTION_DATA *d, char *input) );
void ansi_color          args( ( const char *txt, PLAYER_DATA *ch ) );
void clrscr              args( ( PLAYER_DATA *ch ) );
void global              args( ( char *buf, int level, int toggler,
                                 int toggler2 ) );
void write_global        args( ( char *buf ) );
void act                 args( ( const char *format, CD *ch,
                                 const void *arg1, const void *arg2, 
                                 int type ) );
#if defined(macintosh)
int gettimeofday( struct timeval *tp, void *tzp )
#endif
COMMAND( cmd_global );

/***************************************************************************
 * Variable and functional contents for file hotboot.c                      
 * Erwin Andreasen's (http://www.andreasen.org/) Hotboot Hotboot          
 ***************************************************************************/
COMMAND(	cmd_hotboot	);
/* Needs to be global because of cmd_hotboot */
int port, control;
void go_mud_go  args( ( int p, int c ) );

/*****************************************************************************
 * Variable and functional contents for file load.c                          *
 * Contains all database, mud-wide configuration, and manipulation routines. *
 *****************************************************************************/

/* from help.c */

extern  const  struct help_class  help_class_table  [MAX_HELP_CLASS];
HELP_DATA *    new_help              args( ( void ) );
void           free_help             args( ( HELP_DATA *pHelp ) );
void    hedit                  args( ( PLAYER_DATA *ch, char *argument ) );
COMMAND( cmd_hedit  );

extern int port,control; /* for hot booting */
extern bool fHotBoot;

extern HELP_DATA *        help_first;
extern HELP_DATA *        help_last;
extern SHOP_DATA *        shop_first;
extern SHOP_DATA *        shop_last;
extern char               bug_buf           [2*MAX_INPUT_LENGTH];
extern PLAYER_DATA *        actor_list;
extern SPELL_DATA *       spell_data;
extern TERRAIN_DATA *     terrain_list;
extern char *             help_greeting;
extern char               log_buf           [2*MAX_INPUT_LENGTH];
extern PROP_DATA *         prop_list;
#if defined(INTERGALACTIC_PLANETARY)
extern TIME_INFO_DATA     weather_info;
extern TROPOSPHERE_DATA       weather_info;
#else
extern WEATHER_DATA       weather_info;
#endif

/*
 * Prop, Actor, Scene, Script, Skill and Spell Hashes.
 */
extern ACTOR_INDEX_DATA * actor_index_hash  [MAX_KEY_HASH];
extern PROP_INDEX_DATA *  prop_index_hash   [MAX_KEY_HASH];
extern SCENE_INDEX_DATA * scene_index_hash  [MAX_KEY_HASH];
extern SCRIPT_DATA *      script_index_hash [MAX_KEY_HASH];
extern SKILL_DATA *       skill_index_hash  [MAX_KEY_HASH];
extern SPELL_DATA *       spell_index_hash  [MAX_KEY_HASH];
extern HELP_DATA *        help_index_hash   [MAX_KEY_HASH];
extern STAR_DATA *        star_index_hash   [MAX_KEY_HASH];

#define HASHSEARCH(hash, conditional, var)\
{ int tvnum; for ( tvnum = 0; tvnum < MAX_KEY_HASH; tvnum++ ) {\
for ( var = hash[tvnum]; var != NULL; var  = var->next ) \
{if ( conditional ) break;} if ( var ) break; } }

extern char *             string_hash       [MAX_KEY_HASH];
extern ZONE_DATA *        zone_first;
extern ZONE_DATA *        zone_last;
extern int                LOG_LEVEL;
extern bool               fBootDb;
extern FILE *             fpZone;
extern char               strzone           [MAX_INPUT_LENGTH];
extern struct social_type social_table      [MAX_SOCIALS];
extern int                social_count;

void    boot_db          args( ( int c, bool fCopyOver ) );

void load_starmap      args( ( FILE *fp) );

void fread_shop        args( ( FILE *fp, ACTOR_INDEX_DATA *pActorIndex ) );
void fread_actor2      args( ( FILE *fp, int vnum ) );
void fread_prop_index  args( ( FILE *fp, int vnum ) );
void fread_scene       args( ( FILE *fp, int vnum ) );

void fread_script      args( ( FILE *fp, int vnum ) );


char *  fread_file       args( ( char *filename ) );
void    fix_exits        args( ( void ) );
CD *    create_actor    args( ( ACTOR_INDEX_DATA *pActorIndex ) );
OD *    create_prop    args( ( PROP_INDEX_DATA *pPropIndex, int level ) );
MID *   get_actor_index    args( ( int vnum ) );
OID *   get_prop_index    args( ( int vnum ) );
RID *   get_scene_index   args( ( int vnum ) );
SCD *   get_script_index args( ( int vnum ) );
void    prop_strings      args( ( PROP_DATA *prop ) );
void    actor_strings      args( ( PLAYER_DATA *actor ) );
void    bug              args( ( const char *str, int param ) );
void    bugs             args( ( const char *str, char *param ) );
void    log_string       args( ( const char *str ) );
/* Omitted: dump_section_0             */
/* Omitted: dump_section_s             */
void    tail_chain       args( ( void ) );

/*****************************************************************************
 * Variable and functional declarations for file def.c                       *
 * Contains most tables and mainly constant declarations.                    *
 *****************************************************************************/

extern char *   const                         percent_hit      [];
extern char *   const                         percent_tired    [];
extern char *   const                         percent_mana     [];
extern          const   struct coin_type      coin_table       [MAX_COIN];
extern          const   struct race_type      race_table       [MAX_RACE];
/*extern          const   struct lingua_type    lingua_table     
[MAX_LINGUA];*/
extern          const   struct lang_type      lang_table       [MAX_LANGUAGE];
extern          const   struct color_data     color_table      [];
extern          const   struct attack_type    attack_table     [];
extern          const   struct str_app_type   str_app          [26];
extern          const   struct int_app_type   int_app          [26];
extern          const   struct wis_app_type   wis_app          [26];
extern          const   struct dex_app_type   dex_app          [26];
extern          const   struct con_app_type   con_app          [26];
extern          const   struct liq_type       liq_table        [LIQ_MAX];
extern          const   struct comp_type      comp_table       [];
extern          const   struct group_type     group_table      [];

/*****************************************************************************
 * Variable and functional declarations for file door.c                      *
 * Contains most tables and mainly constant declarations.                    *
 *****************************************************************************/

char *  get_direction   args( ( char *arg ) );
int     find_door       args( ( PLAYER_DATA *ch, char *arg ) );
COMMAND( cmd_open     );
COMMAND( cmd_close    );
OD *    has_key         args( ( PLAYER_DATA *ch, int key ) );
COMMAND( cmd_lock     );
COMMAND( cmd_unlock   );
COMMAND( cmd_pick     );

/*****************************************************************************
 * Variable and function declarations for file combat.c                      *
 * This file constitutes the entire combat system.                           *
 *****************************************************************************/

int     fight_mode_lookup   args( ( const char *name ) );
void MSG_weapon_hit         args ( ( PLAYER_DATA *ch, PROP_DATA *prop, PLAYER_DATA *vi, int dam ) );
void MSG_weapon_partial_dodge   args( ( PLAYER_DATA *ch, PROP_DATA*prop, PLAYER_DATA *victim ) );
void MSG_weapon_parry       args( ( PLAYER_DATA *ch, PROP_DATA *prop, PROP_DATA *vo, PLAYER_DATA *victim ) );
void MSG_weapon_dodge       args( ( PLAYER_DATA *ch, PROP_DATA *prop, PLAYER_DATA *victim ) );
void MSG_weapon_miss        args( ( PLAYER_DATA *ch, PROP_DATA *prop, PLAYER_DATA *victim ) );
void MSG_actor_hit          args( ( PLAYER_DATA *ch, int idx, PLAYER_DATA *victim, int dam ) );
void MSG_partial_dodge      args( ( PLAYER_DATA *ch, int idx, PLAYER_DATA *victim ) );
void MSG_hand_hit           args( ( PLAYER_DATA *ch, PLAYER_DATA *victim, int dam ) );
void MSG_hand_miss          args( ( PLAYER_DATA *ch, PLAYER_DATA *victim ) );
void MSG_dodge              args( ( PLAYER_DATA *ch, int idx, PLAYER_DATA *victim ) );
int armor_deflect           args( ( PLAYER_DATA *victim, int idx ) );
int chance_to_hit           args( ( PLAYER_DATA *ch, PLAYER_DATA *victim ) );
int damage_adjust           args( ( PLAYER_DATA *ch, int dam, int deflected ) );

bool flee_check             args( ( PLAYER_DATA *ch, int d ) );

void    exp_gain        args( ( PLAYER_DATA *ch, int gain, bool fMessage ) ); 
void    death_cry       args( ( PLAYER_DATA *ch ) );
bool    is_safe         args( ( PLAYER_DATA *ch, PLAYER_DATA *victim ) );
void    make_corpse     args( ( PLAYER_DATA *ch ) );
void    oroc            args( ( PLAYER_DATA *ch, PLAYER_DATA *victim ) );
void    raw_kill        args( ( PLAYER_DATA *victim ) );
void    set_fighting    args( ( PLAYER_DATA *ch, PLAYER_DATA *victim ) );
/* void    disarm          args( ( PLAYER_DATA *ch, PLAYER_DATA *victim ) ); */

void    hand_attack     args( ( PLAYER_DATA *ch, PLAYER_DATA *victim ) );
void    actor_attack   args( ( PLAYER_DATA *ch, PLAYER_DATA *victim ) );
void    weapon_attack   args( ( PLAYER_DATA *ch, PROP_DATA *prop, PLAYER_DATA *victim ) );
void    beat_armor      args( ( PLAYER_DATA *victim, int idx ) );
void    damage          args( ( PLAYER_DATA *ch, PLAYER_DATA *victim, int dam ) );
void    update_pos      args( ( PLAYER_DATA *victim ) );
void    stop_fighting   args( ( PLAYER_DATA *ch, bool fBoth ) );
void    violence_update args( ( void ) );

COMMAND( cmd_throw    );
COMMAND( cmd_stop     );
COMMAND( cmd_kill     );
COMMAND( cmd_backstab );
COMMAND( cmd_flee     );
COMMAND( cmd_rescue   );
COMMAND( cmd_assist   );
COMMAND( cmd_mercy    );
COMMAND( cmd_retreat  );
COMMAND( cmd_fight    );
COMMAND( cmd_sla      );
COMMAND( cmd_slay     );
COMMAND( cmd_shoot    );
COMMAND( cmd_reload   );

DECLARE_HIT_FUN(   hit_vorpal        );
DECLARE_HIT_FUN(   hit_suck_disarm   );
DECLARE_HIT_FUN(   hit_fire          );   
DECLARE_HIT_FUN(   hit_cold          );
DECLARE_HIT_FUN(   hit_rot           );
DECLARE_HIT_FUN(   hit_acid          );
DECLARE_HIT_FUN(   hit_lightning     );
DECLARE_HIT_FUN(   hit_kick          );
DECLARE_HIT_FUN(   hit_poison        );

/*****************************************************************************
 * Variable and functional declarations for file furn.c                      *
 * Contains all furniture routines.                                          *
 *****************************************************************************/
 
void  show_occupants_to_actor args( ( PROP_DATA *prop, PLAYER_DATA *ch ) );
int     count_occupants      args( ( PROP_DATA *prop ) );
bool    has_occupant         args( ( PROP_DATA *prop ) );
bool    occupant             args( ( PROP_DATA *prop ) );
CD *    hitched              args( ( PROP_DATA *prop ) );
OD *    get_furn_here        args( ( PLAYER_DATA *ch, char *argument ) );
void    set_furn             args( ( PLAYER_DATA *ch, PROP_DATA *prop ) );

/*****************************************************************************
 * Variable and functional declarations for file graphics.c                  *
 * Contains all char/prop/scene manipulation scripts                         *
 *****************************************************************************/

COMMAND( cmd_map );

void gmode                           args( ( int x, int y ) );
void clear_page                      args( ( void ) );

void text                            args( (int x, int y, char *arg ) );
void textline                        args( (int x0, int y0, int x1, int y1, char *arg ) );

void draw_to_buffer                  args( ( PLAYER_DATA *ch ) );

void point                           args( ( int x, int y ) );
void rect                            args( ( int x, int y, int dx, int dy ) );
void box                             args( ( int x, int y, int dx, int dy ) );

void fill                    args( ( int x, int y, int dx, int dy ) ); 
void fills                   args( ( int x, int y, char *c ) ); 
void column                  args( ( int x, int y, int w, int h, char *c ) ); 
char *wrap                   args( ( int w, char *c, char *d ) );

void hline                   args( ( int x, int y, int w ) ); 
void vline                   args( ( int x, int y, int h ) );

void popup     args( ( int x, int y, int dx, int dy, char *title ) );
void button    args( ( int x, int y, char *c ) );

void CirclePoint                     args( (int cx,int cy,int x,int y) );
void circle                  args( ( int xcenter, int ycenter, int radius ) );

void line                    args( ( int x0, int y0, int x1, int y1 ) );
void goto_xy                 args(( PLAYER_DATA *ch, int x, int y ));

#define MAX_X 80
#define MAX_Y 50

extern int mode;
extern int page;
extern char paintbrush;

extern char pages[MAX_X*MAX_Y][4];  /* 0, 1, 2, 3 */
extern int  cpages[MAX_X*MAX_Y][4];  /* 0, 1, 2, 3 */

extern int stroke;
extern int bgcolor;
extern int style;
extern int ruler;

extern int width;
extern int height;

char *color                  args( ( int c ) );

/*****************************************************************************
 * Variable and functional declarations for file handler.c                   *
 * Contains all char/prop/scene manipulation scripts                     *
 *****************************************************************************/


int        race_lookup         args( ( int vnum ) );
int        get_trust           args( ( PLAYER_DATA *ch ) );
int        get_curr_str        args( ( PLAYER_DATA *ch ) );
int        get_curr_int        args( ( PLAYER_DATA *ch ) );
int        get_curr_wis        args( ( PLAYER_DATA *ch ) );
int        get_curr_dex        args( ( PLAYER_DATA *ch ) );
int        get_curr_con        args( ( PLAYER_DATA *ch ) );
int        can_carry_w         args( ( PLAYER_DATA *ch ) );
void bonus_modify  args( ( PLAYER_DATA *ch, BONUS_DATA *paf, bool fAdd ) );
void       bonus_remove        args( ( PLAYER_DATA *ch, BONUS_DATA *paf ) );
void       bonus_strip         args( ( PLAYER_DATA *ch, int sn ) );
void       bonus_to_actor       args( ( PLAYER_DATA *ch, BONUS_DATA *paf ) );
bool       is_bonused          args( ( PLAYER_DATA *ch, int sn ) );
void       bonus_join          args( ( PLAYER_DATA *ch, BONUS_DATA *paf ) );
void       actor_from_scene    args( ( PLAYER_DATA *ch ) );
void       actor_to_scene      args( ( PLAYER_DATA *ch, SCENE_INDEX_DATA *pSceneIndex ) );
void       prop_to_actor_money args( ( PROP_DATA *prop, PLAYER_DATA *ch ) );
void       prop_to_actor        args( ( PROP_DATA *prop, PLAYER_DATA *ch ) );
void       prop_from_actor      args( ( PROP_DATA *prop ) );
int        apply_ac            args( ( PROP_DATA *prop, int iWear ) );
OD *       get_item_tool       args( ( PLAYER_DATA *prop, int nbit ) );
PROP_DATA *get_tool_char       args( ( PLAYER_DATA *ch, int nbit ) );
OD *       get_item_char       args( ( PLAYER_DATA *prop, int itype ) );
OD *       get_item_held       args( ( PLAYER_DATA *prop, int iWear ) );
OD *       get_eq_char         args( ( PLAYER_DATA *ch, int iWear ) );
int        hand_empty          args( ( PLAYER_DATA *ch ) );
int        wield_free          args( ( PLAYER_DATA *ch, PROP_DATA *prop ) );
int        belt_empty          args( ( PLAYER_DATA *ch ) );
bool       unequip_char        args( ( PLAYER_DATA *ch, PROP_DATA *prop ) );
void       equip_char          args( ( PLAYER_DATA *ch, PROP_DATA *prop, int iWear ) );
int        count_prop_list     args( ( PROP_INDEX_DATA *prop, PROP_DATA *list ) );
void       prop_from_scene     args( ( PROP_DATA *prop ) );
void       prop_to_scene       args( ( PROP_DATA *prop, SCENE_INDEX_DATA *pSceneIndex ) );
void        prop_to_prop       args( ( PROP_DATA *prop, PROP_DATA *prop_to ) );
void    prop_from_prop         args( ( PROP_DATA *prop ) );
void    extract_prop           args( ( PROP_DATA *prop ) );
void    extract_char           args( ( PLAYER_DATA *ch, bool fPull ) );
CD *    get_actor_scene        args( ( PLAYER_DATA *ch, char *argument ) );
CD *    get_actor_world        args( ( PLAYER_DATA *ch, char *argument ) );
OD *    get_prop_type          args( ( PROP_INDEX_DATA *pPropIndexData ) );
int     get_prop_number        args( ( PROP_DATA *prop ) );
int     get_prop_weight        args( ( PROP_DATA *prop ) );
int     scene_is_dark          args( ( SCENE_INDEX_DATA *pSceneIndex ) );
bool    scene_is_private       args( ( SCENE_INDEX_DATA *pSceneIndex ) );
bool    can_see                args( ( PLAYER_DATA *ch, PLAYER_DATA *victim ) );
bool    can_see_prop           args( ( PLAYER_DATA *ch, PROP_DATA *prop ) );
bool    can_drop_prop          args( ( PLAYER_DATA *ch, PROP_DATA *prop ) );
char *  get_extra_descr        args( ( const char *name, EXTRA_DESCR_DATA *ed ) );
bool    use_tool               args( ( PROP_DATA *prop, int bit ) );
int     PERCENTAGE             args( ( int amount, int max ) );
int     wield_weight           args( ( PLAYER_DATA *ch ) );
bool    is_hooded              args( ( PLAYER_DATA *ch ) );

/*****************************************************************************
 * Variable and function declarations for file html.c                        *
 * Web export features for exporting mud statistics and documentation.       *
 *****************************************************************************/

char * add_br                 args( ( const char *str ) );
void   html_export_helps      args( ( void ) );

/*****************************************************************************
 * Variable and function declarations for file horror.c                      *
 * Lycanthropes and vampires, perhaps other "horror film" features.          *
 *****************************************************************************/

void    change_lycanthropes    args( ( bool fWere ) );

COMMAND( cmd_bite );

#if defined(IMC)
/*****************************************************************************
 * Variable and function declarations for file imc.c                         *
 * Added support for Inter-Mud Communication v4.24b (IMC2)                   *
 *****************************************************************************/

bool    imc_command_hook args( ( PLAYER_DATA *ch, char *command, char *argument ) );
void    imc_startup      args( ( bool forced ) );
void    imc_shutdown     args( ( bool reconnect ) );
void    imc_initchar     args( ( PLAYER_DATA *ch ) );
bool    imc_loadchar     args( ( PLAYER_DATA *ch, FILE *fp, const char *word ) );
void    imc_savechar     args( ( PLAYER_DATA *ch, FILE *fp ) );
void    imc_freechardata args( ( PLAYER_DATA *ch ) );
void    imc_loop         args( ( void ) );
#endif

/*****************************************************************************
 * Variable and functional declarations for file parser.c                    *
 * Handles all input interpretation for the mail command set.                *
 *****************************************************************************/

extern  const   struct  cmd_type        command_table       [];

COMMAND( cmd_comlist  );
COMMAND( cmd_library  );

void    interpret       args( ( PLAYER_DATA *ch, char *argument ) );

void ansi_keyboard args( ( PLAYER_DATA *ch, char *argument ) );

 /* Omitted: check_social       */

/*****************************************************************************
 * Variable and functional declarations for file language.c                  *
 * Does the cool stuff for language garbling and stuff like that.            *
 *****************************************************************************/

char *  garble_text     args( ( char *t, int prcnt_comp ) );
int     check_speech    args( ( PLAYER_DATA *ch, PLAYER_DATA *speaker,
                               int gsn ) );

/*****************************************************************************
 * Variable and functional declarations for file mem.c                       *
 * Handles all memory creation and disposing, and the free lists.            *
 *****************************************************************************/

/* Omitted free lists and zero structures, they need not be global. */

extern int    num_pulse;       /* Remaining pulses until reboot. */

extern char * string_space;
extern char * top_string;
extern char   str_empty[1];

extern int top_bonus;
extern int top_zone;
extern int top_ed;
extern int top_exit;
extern int top_help;
extern int top_actor_index;
extern int top_prop_index;
extern int top_spawn;
extern int top_scene;
extern int top_shop;
extern int top_variable;
extern int top_instance;
extern int top_event;
extern int top_script;
extern int top_player_data;
extern int top_userdata;
extern int top_attack;
extern int top_prop;
extern int top_connection;
extern int top_note;

extern int top_vnum_help;
extern int top_vnum_spell;
extern int top_vnum_skill;
extern int top_vnum_script;
extern int top_vnum_actor;
extern int top_vnum_prop;
extern int top_vnum_scene;
extern int top_vnum_terrain;
extern int top_vnum_race;

NUMBER_LIST*    new_number_list       args( ( void ) );
STRING_LIST*    new_string_list       args( ( void ) );

void    free_string_list              args( ( STRING_LIST *pString ) );
void    free_number_list              args( ( NUMBER_LIST *pNumber ) );

DD *    new_connection_data   args( ( void ) );
void    free_connection       args( ( CONNECTION_DATA *d ) );

CD *    new_player_data       args( ( void ) );
void    free_char             args( ( PLAYER_DATA *ch ) );

SPELL_BOOK_DATA *   new_spell_book_data   args( ( void ) );
void    free_spell_book_data  args( ( SPELL_BOOK_DATA *pSpell ) );

HELP_DATA *   new_help_data args( ( void ) );
void    free_help_data       args( ( HELP_DATA *pHelp ) );

SPELL_DATA *   new_spell_data args( ( void ) );
void    free_spell_data       args( ( SPELL_DATA *pSpell ) );

SKILL_DATA *   new_skill_data args( ( void ) );
void    free_skill_data       args( ( SKILL_DATA *pSpell ) );

USER_DATA *   new_user_data   args( ( void ) );
void    free_user_data        args( ( USER_DATA *pc ) );

OD *    new_prop              args( ( void ) );
void    free_prop             args( ( PROP_DATA *pProp ) );

RD *    new_spawn_data        args( ( void ) );
void    free_spawn_data       args( ( SPAWN_DATA *pSpawn ) );

TERRAIN_DATA *  new_terrain   args( ( void ) );

AD *    new_zone              args( ( void ) );
void    free_zone             args( ( ZONE_DATA *pZone ) );

EDD *   new_extra_descr       args( ( void ) );
void    free_extra_descr      args( ( EXTRA_DESCR_DATA *pExtra ) );

ED *    new_exit              args( ( void ) );
void    free_exit             args( ( EXIT_DATA *pExit ) );

ATD *   new_attack            args( ( void ) );
void    free_attack           args( ( ATTACK_DATA *attack ) );

ALIAS_DATA * new_alias_data    args( ( void ) ) ;
void         free_alias_data   args( ( ALIAS_DATA *alias ) );

ATD *   new_alias             args( ( void ) );
void    free_alias            args( ( ALIAS_DATA *alias ) );

RID *   new_scene_index       args( ( void ) );
void    free_scene_index      args( ( SCENE_INDEX_DATA *pScene ) );

SPELL_DATA *   new_spell_index       args( ( void ) );
void    free_spell_index      args( ( SPELL_DATA *pSpell ) );

EVENT_DATA * new_event_data   args( ( void ) );
void    free_event_data       args( ( EVENT_DATA* pEvent ) );

AFD *   new_bonus             args( ( void ) );
void    free_bonus            args( ( BONUS_DATA* pAf ) );

OID *   new_prop_index        args( ( void ) );
void    free_prop_index       args( ( PROP_INDEX_DATA *pProp ) );

SD *    new_shop              args( ( void ) );
void    free_shop             args( ( SHOP_DATA *pShop ) );

MID *   new_actor_index       args( ( void ) );
void    free_actor_index      args( ( ACTOR_INDEX_DATA *pActor ) );

VARD *  new_variable_data     args( ( void ) );
VARD *  new_variable          args( ( int type, void * value ) );
void    free_variable         args( ( VARIABLE_DATA *var ) );

ID *    new_instance          args( ( void ) );
void    free_instance         args( ( INSTANCE_DATA *instance ) );

SCD *   new_script            args( ( void ) );
void    free_script           args( ( SCRIPT_DATA *scr ) );

char    fread_letter          args( ( FILE *fp ) );
int     fread_number          args( ( FILE *fp ) );
char *  fread_string          args( ( FILE *fp ) );
char *  fread_string_eol      args( ( FILE *fp ) );
void    fread_to_eol          args( ( FILE *fp ) );
char *  fread_word            args( ( FILE *fp ) );

void *  alloc_mem             args( ( int sMem ) );
void    ofree_mem             args( ( void *pMem, int sMem ) );
void    free_mem              args( ( void *pMem, int sMem ) );
void *  alloc_perm            args( ( int sMem ) );

char *  str_dup               args( ( const char *str ) );
void    free_string           args( ( char *pstr ) );

COMMAND( cmd_memory );

/*****************************************************************************
 * Variable and functional declarations for file money.c                     *
 * Contains all money/prop routines.                                       *
 *****************************************************************************/
 
void    update_money       args( ( PROP_DATA *prop ) );
OD *    create_money       args( ( int amount, int type ) );
void    merge_money        args( ( PLAYER_DATA *ch ) );
void    merge_money_prop   args( ( PROP_DATA *prop ) );
void    merge_money_scene  args( ( SCENE_INDEX_DATA *scene ) );
void    create_amount      args( ( int amount, PLAYER_DATA *pActor,
                                   SCENE_INDEX_DATA *pScene, PROP_DATA *pProp ) );
int     tally_coins        args( ( PLAYER_DATA *actor ) );
int     tally_one_coin     args( ( PROP_DATA *plist, int type, bool fContents ) );
char *  name_amount        args( ( int amount ) );
void    strip_empty_money  args( ( PLAYER_DATA *ch ) );
char *  sub_coins          args( ( int amount, PLAYER_DATA *ch ) );

/*****************************************************************************
 * Variable and functional declarations for file mount.c                     *
 * Contains all mount/dismount routines.                                     *
 *****************************************************************************/

void dismount_char      args( ( PLAYER_DATA *ch ) );
void mount_char         args( ( PLAYER_DATA *ch, PLAYER_DATA *victim ) );

COMMAND( cmd_mount     );
COMMAND( cmd_rush      );
COMMAND( cmd_trample   );
COMMAND( cmd_dismount  );

/*****************************************************************************
 * Variable and functional declarations for file nanny.c                     *
 * Handles all new-connection and new-character code.                        *
 *****************************************************************************/

void    setup_race      args( ( PLAYER_DATA *ch ) );
void    new_char        args( ( PLAYER_DATA *ch ) );
void    stat_menu       args( ( PLAYER_DATA *ch, char *argument ) );
void    stat_menu_choice  args( ( PLAYER_DATA *ch, char *argument ) );
void    actor_gen       args( ( PLAYER_DATA *ch, char *argument ) );

void    nanny              args( ( CONNECTION_DATA *d, char *argument ) );
 /* Omitted: nanny_check          */
 /* Omitted: check_parse_name     */
 /* Omitted: check_playing        */
 /* Omitted: check_reconnect      */
 /* Omitted: stop_idling          */
bool    apply_ok           args( ( PLAYER_DATA *ch ) );
 /* Omitted: print_gen_menu       */
 /* Omitted: print_doc_menu       */
void    print_login_menu   args( ( PLAYER_DATA *ch ) );

/*****************************************************************************
 * Variable and functional declarations for file magic.c                     *
 * This file is not really functioning at this time.                         *
 *****************************************************************************/
extern SPELL_DATA *spell_list;

SPELL_DATA *find_spell      args( ( char *argument ) );
HELP_DATA  *get_help_index  args( ( int vnum ) );
SPELL_DATA *get_spell_index args( ( int vnum ) );
SKILL_DATA *get_skill_index args( ( int vnum ) );

bool    saves_spell         args( ( int level, PLAYER_DATA *victim ) );
int     prop_cast_spell     args( ( PROP_DATA *item, int vnum, PLAYER_DATA *ch,
                                    PLAYER_DATA *victim, PROP_DATA *prop ) );
void   clear_spell_book args( ( SPELL_BOOK_DATA *pSpellbook ) );

bool has_spell     args( ( SPELL_BOOK_DATA *book, int vnum ) );
int find_gem_mana  args( ( PLAYER_DATA *ch, int bit ) );
void take_mana_gem args( ( PLAYER_DATA *ch, int mana, int bit ) );

COMMAND( cmd_mana );
COMMAND( cmd_cast );
COMMAND( cmd_spellbook );
COMMAND( cmd_scribe );
COMMAND( cmd_reagents );

/*****************************************************************************
 * Variable and functional declarations for file edit.c                      *
 * The online creation source code is contained herein.                      *
 *****************************************************************************/

AD *    get_zone_data          args( ( int vnum ) );
AD *    get_vnum_zone          args( ( int vnum ) );
char *  zone_bit_name          args( ( int zone_flags ) );
int     get_zone_flags_number  args( ( char *argument ) );
bool    redit_exit             args( ( CD *ch, RID *pScene, int door,
                                       char *arg1, char *arg2 ) );

void    zedit                  args( ( PLAYER_DATA *ch, char *argument ) );
void    redit                  args( ( PLAYER_DATA *ch, char *argument ) );
void    oedit                  args( ( PLAYER_DATA *ch, char *argument ) );
void    aedit                  args( ( PLAYER_DATA *ch, char *argument ) );
void    sedit                  args( ( PLAYER_DATA *ch, char *argument ) );
void    spedit                 args( ( PLAYER_DATA *ch, char *argument ) );
void    skedit                 args( ( PLAYER_DATA *ch, char *argument ) );

void value_breakdown           args( ( int type, int v1, int v2, int v3, int v4,
                                       PLAYER_DATA *ch ) );

void display_spawns           args( ( PLAYER_DATA *ch ) );
void add_spawn                args( ( SCENE_INDEX_DATA *scene, SPAWN_DATA *pSpawn, int i ) );


void save_zone_list            args( ( void ) );
void save_config               args( ( void ) );
void save_contents             args( ( void ) );
void save_actors               args( ( FILE *fp, ZONE_DATA *pZone ) );
void save_scripts              args( ( FILE *fp, ZONE_DATA *pZone ) );
void save_props                args( ( FILE *fp, ZONE_DATA *pZone ) );
void save_scenes               args( ( FILE *fp, ZONE_DATA *pZone ) );

void save_zone( ZONE_DATA *pZone );
void save_string_to_file( char *fname, char *content );
void save_helps( void );

COMMAND( cmd_zedit  );
COMMAND( cmd_redit  );
COMMAND( cmd_oedit  );
COMMAND( cmd_aedit  );
COMMAND( cmd_sedit  );
COMMAND( cmd_spedit );
COMMAND( cmd_skedit );
COMMAND( cmd_aindex );
COMMAND( cmd_sindex );
COMMAND( cmd_pindex );
COMMAND( cmd_spawns );
COMMAND( cmd_astat  );
COMMAND( cmd_hstat  );
COMMAND( cmd_zsave  );


/*****************************************************************************
 * Variable and functional declarations for file grammar.c                   *
 * Contains all parsing routines for user input.                             *
 *****************************************************************************/

bool filler_words  args( ( char *argument ) );

OD *    find_prop_list   args( ( PROP_DATA *list, char *argument ) );

OD *    get_prop_list    args( ( PLAYER_DATA *ch, char *argument,
                                 PROP_DATA *list ) );
OD *    get_prop_carry   args( ( PLAYER_DATA *ch, char *argument ) );
OD *    get_prop_wear    args( ( PLAYER_DATA *ch, char *argument ) );
OD *    get_prop_inv     args( ( PLAYER_DATA *ch, char *argument ) );
OD *    get_prop_inv2    args( ( PLAYER_DATA *ch, char *argument ) );
OD *    get_prop_here    args( ( PLAYER_DATA *ch, char *argument ) );
OD *    get_prop_world   args( ( PLAYER_DATA *ch, char *argument ) );

/*****************************************************************************
 * Variable and functional declarations for file players.c                   *
 * Contains playerbase statistical routines.                                 *
 *****************************************************************************/
void   generate_statistics    args( ( void ) );

/*****************************************************************************
 * Variable and functional declarations for file save.c                      *
 * Includes saving and loading of characters, and savable scenes.             *
 *****************************************************************************/

#define MAX_NEST        1000

extern PROP_DATA *      rgObjNest       [MAX_NEST];

 /* Omitted: fread/fwrite char */
 /* Omitted: fread/fwrite actor  */

void    fwrite_prop      args( ( PROP_DATA *prop, FILE *fp, int iNest ) );
void    fread_prop       args( ( void *owner, int owner_type, FILE *fp ) );
void    save_actor_prop    args( ( PLAYER_DATA *ch ) );
bool    load_actor_prop    args( ( CONNECTION_DATA *d, char *name ) );

/*****************************************************************************
 * Variable and functional declarations for shop.c                           *
 * The shop system and trading code.                                         *
 *****************************************************************************/

#define MAX_GOODS 154
#define MAX_COMPONENTS 286
extern const     struct   goods_type    goods_table    [];

OD *    create_good        args( ( int good ) );
OD *    create_comp        args( ( int good ) );
void    shop_list_to_actor args( ( PLAYER_DATA *keeper, PLAYER_DATA *ch ) );
OD *    get_shop_list_prop args( ( PLAYER_DATA *keeper, PLAYER_DATA *ch, int number ) );
CD *    find_keeper        args( ( PLAYER_DATA *ch, char *arg, bool Report ) );
int     get_cost           args( ( PLAYER_DATA *keeper, PROP_DATA *prop, bool fBuy ) );
bool    transact           args( ( PLAYER_DATA *keeper, PROP_DATA *prop, PLAYER_DATA *ch, int price ) );
void    buy_ai             args( ( PLAYER_DATA *ch, PLAYER_DATA *keeper, int offer ) );

COMMAND( cmd_repair   );
COMMAND( cmd_list     );
COMMAND( cmd_buy      );
COMMAND( cmd_trade    );
COMMAND( cmd_sell     );
COMMAND( cmd_barter   );
COMMAND( cmd_offer    );
COMMAND( cmd_money    );
COMMAND( cmd_hire     );
COMMAND( cmd_appraise );

/*****************************************************************************
 * Variable and functional declarations for interpreter.c                    *
 * The script language and parser.                                           *
 *****************************************************************************/

COMMAND( cmd_script   );
COMMAND( cmd_gspeak   );

void rem_variable   args( ( VARIABLE_DATA **vlist, char *name ) );
char *translate_variables_noliterals_list  args( ( VARIABLE_DATA *list, char *exp ) );
char *translate_variables_noliterals  args( ( void * owner, int type, char *exp ) );
void mini_parse_script  args( ( void * owner, int type, char *exp ) );

int  script_update    args( ( void * owner, int type, int ttype,
                             PLAYER_DATA *actor, PROP_DATA *catalyst, char *astr,
                             char *bstr  ) );
void parse_script    args( ( INSTANCE_DATA *instance, void * owner, int type ) );
void trigger_list   args( ( PROP_DATA *list, int ttype, PLAYER_DATA  *actor,
                            PROP_DATA *catalyst, char *astr, char *bstr ) );
VARD *eval_function  args( ( void * owner, int type, char *exp ) );

INSTANCE_DATA *find_instance  args( ( void * owner, int type, char *trigname ) );  // from function.c

/*****************************************************************************
 * Variable and functional declarations for events.c                         *
 * The event queue functions.                                                *
 *****************************************************************************/

void add_event         args (( void * owner, int type, int vnum, int delay,
                char *special, PLAYER_DATA *actor, PLAYER_DATA *target,
                PROP_DATA *catalyst, char *astr, char *bstr   ));

void update_event                  args( ( void ) );
void rem_event                     args( ( EVENT_DATA *pEvent ) );
void clear_events                  args( ( void * owner, int type ) ); 

COMMAND( cmd_events );

/*****************************************************************************
 * Variable and functional declarations for skills.c                         *
 * Skill routines.                                                           *
 *****************************************************************************/

SKILL_DATA *skill_lookup      args( ( const char *name ) );
SKILL_DATA *skill_copy        args( ( SKILL_DATA *pSource ) );

void show_skills_list         args( ( PLAYER_DATA *ch, SKILL_DATA *pSkill ) );
int  practice_cost            args( ( int amount, int level ) );


bool has_skill                args( ( PLAYER_DATA *ch, SKILL_DATA *pSkill ) );

int learned   args( ( PLAYER_DATA *ch, int vnum ) );
int LEARNED   args( ( PLAYER_DATA *ch, char *name ) );

bool skill_check args( ( PLAYER_DATA *ch, SKILL_DATA *pSkill, int modifier ) );

extern bool slot_spell;
void *slot_lookup             args( ( int slot ) );
char *skill_level             args( ( SKILL_DATA *pSkill ) );
int skill_vnum                args( ( SKILL_DATA *pSkill ) );
char *skill_name              args( ( int vnum ) );

SKILL_DATA *find_skill_pc     args( ( PLAYER_DATA *ch, int vnum ) );
SKILL_DATA *find_group_pc     args( ( PLAYER_DATA *ch, SKILL_DATA *pSkill ) );

bool advance_skill   args( ( PLAYER_DATA *ch, SKILL_DATA *pSkill, \
                             int advance, int time_mod ) );
bool has_prereq      args( ( PLAYER_DATA *ch, SKILL_DATA *pSkill, bool fTell ) );
char *disp_group     args( ( PLAYER_DATA *ch, SKILL_DATA *pGroup ) );
char *disp_skill     args( ( PLAYER_DATA *ch, SKILL_DATA *pSkill ) );
void display_skills  args( ( PLAYER_DATA *ch, int group ) );
void show_skills_teacher    args( ( PLAYER_DATA *ch, SKILL_DATA *pSkill ) );
void show_skills     args( ( PLAYER_DATA *ch, SKILL_DATA *pSkill ) );

SKILL_DATA * update_skill    args( ( PLAYER_DATA *ch, int vnum, int value ) );
void add_skill       args( ( PLAYER_DATA *ch, char *name, int value ) );

COMMAND( cmd_skills    );
COMMAND( cmd_learn     );
COMMAND( cmd_practice  );
COMMAND( cmd_train     );

/*****************************************************************************
 * Variable and functional declarations for lists.c                          *
 * Numeric and string list handler functions.                                * 
 *****************************************************************************/

NUMBER_LIST * add_num_nodups    args( ( NUMBER_LIST **nlist, int num ) );
NUMBER_LIST * add_num_list      args( ( NUMBER_LIST **nlist, int num ) );
NUMBER_LIST * rem_num_list      args( ( NUMBER_LIST **nlist, int num ) );
bool num_in_list       args( ( NUMBER_LIST *nlist, int num ) );

STRING_LIST * add_str_nodups    args( ( STRING_LIST **nlist, char *str ) );
STRING_LIST * add_str_list      args( ( STRING_LIST **nlist, char *str ) );
STRING_LIST * rem_str_list      args( ( STRING_LIST **nlist, char *str ) ); 
bool str_in_list       args( ( STRING_LIST *nlist, char *str  ) );
 
/*****************************************************************************
 * Variable and functional declarations for string.c                         *
 * Handles most string routines.                                             *
 *****************************************************************************/

char *  itoa            args( ( int x ) ) ;
void    strjoin         args( ( char *a, char *b ) );
char *  cut_to          args( ( char *p ) );
bool    char_isof       args( ( char c, char *list ) );
void    smash_tilde     args( ( char *str ) );
bool    str_cmp         args( ( const char *astr, const char *bstr ) );
bool    str_prefix      args( ( const char *astr, const char *bstr ) );
bool    str_infix       args( ( const char *astr, const char *bstr ) );
bool    str_suffix      args( ( const char *astr, const char *bstr ) );
char *  capitalize      args( ( const char *str ) );
void    append_file     args( ( PLAYER_DATA *ch, char *file, char *str ) );
char *  fix_string      args( ( const char *str ) );
char *  is_are          args( ( char *buf ) );
char *  smash_arg       args( ( char *t, char *name ) );
char *  smash_article   args( ( char *t ) );
char *  pluralize       args( ( char *argument ) );
char *  format_string   args( ( char *oldstring /*, bool fSpace */ ) );
char *  format_indent   args( ( char *oldstring, char *prefix, int w,
                                bool fEnd ) );
void    replace_char    args( ( char orig[], char old, char new ) );
char *  replace_string  args( ( char *orig, char *old, char *new ) );
char *  string_replace  args( ( char * orig, char * old, char * new ) );
int     arg_count       args( ( char * argument ) );
char *  string_unpad    args( ( char * argument ) );
char *  string_proper   args( ( char * argument ) );
void    string_edit     args( ( PLAYER_DATA *ch, char **pString ) );
void    string_append   args( ( PLAYER_DATA *ch, char **pString ) );
void    string_add      args( ( PLAYER_DATA *ch, char *argument ) );
bool    is_number       args( ( char *arg ) );
int     number_argument args( ( char *argument, char *arg ) );
char *  one_argument    args( ( char *argument, char *arg_first ) );
char *  one_argcase     args( ( char *argument, char *arg_first ) );
char *  strupr          args( ( char * s ) );
char *  strlwr          args( ( char * s ) );
char *  ansi_uppercase  args( ( char *txt) );
bool    is_name         args( ( const char *str, char *namelist ) );
bool    is_prename      args( ( const char *str, char *namelist ) );
/* Omitted a few const declarations for numberize. */
char *  numberize       args( ( int n ) );
char *  numberize_old   args( ( int n ) );
char *  first_arg       args( ( char *argument, char *arg_first, bool fCase ));
char *  skip_spaces     args( ( char *argument ) );
char *  grab_line       args( ( char *argument, char *arg_first ) );
char *  one_line        args( ( char *argument, char *arg_first ) );
bool    has_arg         args( ( char *argument, char *item ) );
char *  trunc_fit       args( ( char *argument, int length ) );
char *  gotoxy          args( ( int x, int y ) );
char *  skip_curlies    args( ( char *arg ) );
char *  strip_curlies   args( ( char *arg ) );
char *  grab_to_eol     args( ( char *argument, char *arg_first ) );
char *  grab_to_at      args( ( char *argument, char *arg_first ) );

/*****************************************************************************
 * Variable and functional declarations for tips.c                           *
 * Displays helpful tips, an extension of the help system by Locke.          *
 *****************************************************************************/

void tip( PLAYER_DATA *ch );

/*****************************************************************************
 * Variable and functional declarations for track.c                          *
 * Handles the internal queue-based tracking routines submitted by jdinkel.  *
 *****************************************************************************/

void list_enqueue  args( ( SCENE_INDEX_DATA *scene ) );
void bfs_enqueue   args( ( SCENE_INDEX_DATA *scene, int dir, int depth ) );
void bfs_dequeue   args( (void) );
void list_dequeue  args( (void) );
void bfs_clear_queue args( (void) );
void list_clear_queue args((void));

int find_first_step     args( ( SCENE_INDEX_DATA *src, SCENE_INDEX_DATA *target,
                                int depth ) );
COMMAND( cmd_track    );
COMMAND( cmd_hunt     );


#if defined(TRANSLATE)
/*****************************************************************************
 * Variable and functional declarations for translate.c                      *
 * Multilanguage support (requires perl).                                    *
 *****************************************************************************/
COMMAND( cmd_translate );
int find_language( char *argument );
char *translate( CONNECTION_DATA *d, char *outbuf );
#endif

/*****************************************************************************
 * Variable and functional declarations for update.c                         *
 * Includes update handler and turn-to-turn information.                     *
 *****************************************************************************/
 
void    time_update        args( ( void ) );
void    prop_update_pulse  args( ( void ) );
void    prop_update_tick   args( ( void ) );
void    auto_reboot        args( ( void ) );

void    advance_level    args( ( PLAYER_DATA *ch ) );
int     hit_gain         args( ( PLAYER_DATA *ch ) );
int     move_gain        args( ( PLAYER_DATA *ch ) );
int     mana_gain        args( ( PLAYER_DATA *ch ) );
void    gain_condition   args( ( PLAYER_DATA *ch, int iCond, int value ) );
void    actor_update     args( ( void ) );
void    weather_update   args( ( void ) );
void    everybody_update args( ( bool fEach ) );
void    prop_update      args( ( bool fEach ) );
void    aggr_update      args( ( void ) );
bool    spawn_scene      args( ( SCENE_INDEX_DATA *pScene ) );
void    scene_update     args( ( void ) );
void    script_update_script args( ( void ) );
void    update_handler  args( ( void ) );

void    show_player_statistics  args( ( PLAYER_DATA *ch ) ) ;
void    update_player_statistics  args( ( void ) );
COMMAND( cmd_stats );

/*****************************************************************************
 * Variable and functional declarations for file wiz.c                       *
 * Contains generic immortal privledged commands and functions.              *
 *****************************************************************************/
extern BAN_DATA *ban_list;

void hotboot_recover args(( int c ));

RID   * find_location   args( ( PLAYER_DATA *ch, char *arg ) );

COMMAND( cmd_advance     );
COMMAND( cmd_restore     );
COMMAND( cmd_freeze      );
COMMAND( cmd_log           );
COMMAND( cmd_noemote     );
COMMAND( cmd_peace       );
COMMAND( cmd_ban           );
COMMAND( cmd_allow       );
COMMAND( cmd_wizlock     );
COMMAND( cmd_force       );
COMMAND( cmd_invis       );
COMMAND( cmd_holylight   );
COMMAND( cmd_bounty      );
COMMAND( cmd_wizify      );
COMMAND( cmd_bamfin      );
COMMAND( cmd_bamfout     );
COMMAND( cmd_deny        );
COMMAND( cmd_disconnect  );
COMMAND( cmd_system      );
COMMAND( cmd_echo        );
COMMAND( cmd_recho       );
COMMAND( cmd_transfer    );
COMMAND( cmd_at          );
COMMAND( cmd_as          );
COMMAND( cmd_goto        );
COMMAND( cmd_reboo       );
COMMAND( cmd_reboot      );
COMMAND( cmd_shutdow     );
COMMAND( cmd_shutdown    );
COMMAND( cmd_snoop       );
COMMAND( cmd_switch      );
COMMAND( cmd_return      );
COMMAND( cmd_charload    );
COMMAND( cmd_shell       );
COMMAND( cmd_mail        );
/*
COMMAND( cmd_ftp         );
COMMAND( cmd_sendfile    );
COMMAND( cmd_getfile     );
COMMAND( cmd_dir         );
 */
COMMAND( cmd_purge       );

/*****************************************************************************
 * Variable and functional declarations for file admin_info.c              *
 * Contains all prop/actor/scene commands for higher level immortals.      *
 *****************************************************************************/

extern int      FORCE_LEVEL;

bool   pstat    args( ( PLAYER_DATA *ch, char *argument ) );
bool   ostat    args( ( PLAYER_DATA *ch, char *argument ) );
bool   mstat    args( ( PLAYER_DATA *ch, char *argument ) );
bool   mwhere   args( ( PLAYER_DATA *ch, char *argument ) );
bool   owhere   args( ( PLAYER_DATA *ch, char *argument ) );
void   pwhere   args( ( PLAYER_DATA *ch ) );

char *connect_string args( ( int c ) );
void count_scenes( ZONE_DATA *pZone, int *scenes, int *unfinished );


COMMAND( cmd_wizhelp     );
COMMAND( cmd_rstat       );
COMMAND( cmd_index       );
COMMAND( cmd_stat        );
COMMAND( cmd_afind       );
COMMAND( cmd_pfind       );
COMMAND( cmd_rfind       );
COMMAND( cmd_scfind      );
COMMAND( cmd_where       );
COMMAND( cmd_users       );
COMMAND( cmd_sockets     );
COMMAND( cmd_slookup     );
COMMAND( cmd_zones       );
COMMAND( cmd_table       );

/*****************************************************************************
 * Variable and functional declarations for file wiz_set.c                   *
 * Contains immortal commands that allow "illegal" customization of props. *
 *****************************************************************************/

COMMAND( cmd_mload       );
COMMAND( cmd_oload       );
COMMAND( cmd_load        );
COMMAND( cmd_sset        );
COMMAND( cmd_mset        );
COMMAND( cmd_oset        );
COMMAND( cmd_pset        );

/*****************************************************************************
 * Variable and functional declarations for file worldgen.c                  *
 * Experimental World Creation Functions                                     *
 *****************************************************************************/

void    show_terrain    args( ( PLAYER_DATA *ch, SCENE_INDEX_DATA *pScene ) );
void    generate        args( ( PLAYER_DATA *ch, int lvnum, int hvnum, bool fOverwrite ) );

COMMAND( cmd_terrain   );
COMMAND( cmd_generate  );

/*****************************************************************************
 * Variable and functional declarations for file xrand.c                     *
 * Contains all random number generation routines.                           *
 *****************************************************************************/

int     number_fuzzy    args( ( int number ) );
int     number_gelify   args( ( int number ) );
int     number_range    args( ( int from, int to ) );
int     number_percent  args( ( void ) );
int     number_door     args( ( void ) );
int     number_bits     args( ( int w ) );
void    init_mm         args( ( void ) );
int     number_mm       args( ( void ) );
int     dice            args( ( int number, int size ) );


#undef    CD
#undef    MID
#undef    OD
#undef    OID
#undef    RID
#undef    SF
#undef  AD
#undef  VD
#undef  RD
#undef  TD
#undef  SD
#undef  EDD
#undef  AFD
#undef  ED
#undef  ATD
#undef  PCD
#undef  SCD
#undef  DD

