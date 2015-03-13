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
#include "script.h"



bool	check_social	args( ( PLAYER *ch, char *command,
			    char *argument ) );



/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2



/*
 * Log-all switch.
 */
bool				fLogAll		= FALSE;



/*
 * Command table.
 */
const	struct	cmd_type	command_table	[] =
{
    /*
     * Common movement commands.
     */
    { "Movement",   NULL,           POS_FIGHTING,         -2,  LOG_NEVER  },

    { "north",      cmd_north,      POS_FIGHTING,          0,  LOG_NORMAL },
    { "east",       cmd_east,       POS_FIGHTING,          0,  LOG_NORMAL },
    { "south",      cmd_south,      POS_FIGHTING,          0,  LOG_NORMAL },
    { "west",       cmd_west,       POS_FIGHTING,          0,  LOG_NORMAL },
    { "up",         cmd_up,         POS_FIGHTING,          0,  LOG_NORMAL },
    { "down",       cmd_down,       POS_FIGHTING,          0,  LOG_NORMAL },
    { "northwest",  cmd_nw,         POS_FIGHTING,          0,  LOG_NORMAL },
    { "northeast",  cmd_ne,         POS_FIGHTING,          0,  LOG_NORMAL },
    { "southwest",  cmd_sw,         POS_FIGHTING,          0,  LOG_NORMAL },
    { "southeast",  cmd_se,         POS_FIGHTING,          0,  LOG_NORMAL },
    { "nw",         cmd_nw,         POS_FIGHTING,          0,  LOG_NORMAL },
    { "ne",         cmd_ne,         POS_FIGHTING,          0,  LOG_NORMAL },
    { "sw",         cmd_sw,         POS_FIGHTING,          0,  LOG_NORMAL },
    { "se",         cmd_se,         POS_FIGHTING,          0,  LOG_NORMAL },
    { "go",         cmd_enter,      POS_FIGHTING,          0,  LOG_NORMAL },
    { "home",       cmd_home,       POS_FIGHTING,          0,  LOG_NORMAL },

    /*
     * Combat commands.
     */
    { "Combat and Fighting",  NULL,          POS_FIGHTING,  -2,  LOG_NEVER  },

    { "engage",     cmd_kill,       POS_FIGHTING,          0,  LOG_NORMAL  },
    { "backstab",   cmd_backstab,   POS_STANDING,          0,  LOG_NORMAL  },
    { "bs",         cmd_backstab,   POS_STANDING,          0,  LOG_NORMAL  },
    { "flee",       cmd_flee,       POS_FIGHTING,          0,  LOG_NORMAL  },
    { "bite",       cmd_bite,       POS_FIGHTING,          0,  LOG_NORMAL  },
/*    { "disarm",   cmd_disarm,     POS_FIGHTING,          0,  LOG_NORMAL  },*/
/*    { "kick",     cmd_kick,       POS_FIGHTING,          0,  LOG_NORMAL  },*/
    { "retreat",    cmd_retreat,    POS_FIGHTING,          0,  LOG_NORMAL  },
    { "fight",      cmd_fight,       -1,         -1,  LOG_NORMAL  },
    { "tactic",     cmd_fight,      POS_FIGHTING,          0,  LOG_NORMAL  },
    { "group",      cmd_group,      POS_RESTING,           0,  LOG_NORMAL  },
    { "monitor",    cmd_monitor,    POS_RESTING,           0,  LOG_NORMAL  },
    { "assist",     cmd_assist,     POS_RESTING,           0,  LOG_NORMAL  },
    { "rescue",     cmd_rescue,     POS_RESTING,           0,  LOG_NORMAL  },
/*    { "special",  cmd_special,    POS_RESTING,           0,  LOG_NORMAL  },*/
    { "mercy",      cmd_mercy,      POS_RESTING,           0,  LOG_NORMAL  },
    { "stop",       cmd_retreat,    POS_FIGHTING,          0,  LOG_NORMAL  },
    { "recall",     cmd_home,        -1,                  -1,  LOG_NORMAL  },
    { "order",      cmd_order,     POS_RESTING,           0,  LOG_NORMAL  },

    /*
     * Common other commands.
     * Placed here so one and two letter abbreviations work.
     */

    { "Common",     NULL,             POS_FIGHTING,         -2,  LOG_NEVER  },

    { "buy",        cmd_buy,          POS_RESTING,           0,  LOG_NORMAL },
    { "cast",       cmd_cast,         POS_SLEEPING,          0,  LOG_NORMAL },
    { "exits",      cmd_exits,        POS_RESTING,           0,  LOG_NORMAL },
    { "get",        cmd_get,          POS_RESTING,           0,  LOG_NORMAL },
    { "inventory",  cmd_inventory,    POS_DEAD,              0,  LOG_NORMAL },
    { "kill",       cmd_kill,         POS_FIGHTING,          0,  LOG_NORMAL },
    { "scan",       cmd_scan,         POS_RESTING,           0,  LOG_NORMAL },
    { "look",       cmd_look,         POS_RESTING,           0,  LOG_NORMAL },
    { "examine",    cmd_look,         POS_RESTING,           0,  LOG_NORMAL },
    { "read",       cmd_look,         POS_RESTING,           0,  LOG_NORMAL },
    { "rest",       cmd_rest,         POS_RESTING,           0,  LOG_NORMAL },
    { "sit",        cmd_sit,          POS_SLEEPING,          0,  LOG_NORMAL },
    { "stand",      cmd_stand,        POS_SLEEPING,          0,  LOG_NORMAL },
    { "sheathe",    cmd_sheath,       POS_RESTING,           0,  LOG_NORMAL },
    { "shoot",      cmd_shoot,        POS_STANDING,          0,  LOG_NORMAL },
    { "fire",       cmd_shoot,        POS_STANDING,          0,  LOG_NORMAL },
    { "wield",      cmd_wield,        POS_RESTING,           0,  LOG_NORMAL },

    /*
     *  Magic and spellcasting commands.
     */

    { "Magick",  NULL,  POS_FIGHTING, -2,  LOG_NEVER  },

    { "spells",     cmd_spellbook,          -1,   -1,  LOG_NORMAL  },
    { "scribe",     cmd_scribe,    POS_RESTING,    0,  LOG_NORMAL  },
    { "spellbook",  cmd_spellbook, POS_RESTING,    0,  LOG_NORMAL  },
    { "quaff",      cmd_quaff,     POS_RESTING,    0,  LOG_NORMAL  },
    { "perform",    cmd_cast,      POS_SLEEPING,   0,  LOG_NORMAL  },
    { "invoke",     cmd_cast,      POS_SLEEPING,   0,  LOG_NORMAL  },
    { "cast",       cmd_cast,         -1,          0,  LOG_NORMAL  },
    { "reagents",   cmd_reagents,     -1,          0,  LOG_NORMAL  },

    /*
     * Informational commands.
     */
    { "Informative",    NULL,         POS_FIGHTING,   -2,  LOG_NEVER  },

    { "commands",   cmd_commands,     POS_DEAD,        0,  LOG_NORMAL    },
    { "compare",    cmd_compare,      POS_RESTING,     0,  LOG_NORMAL    },
    { "consider",   cmd_consider,     POS_RESTING,     0,  LOG_NORMAL    },
    { "credits",    cmd_credits,      POS_DEAD,        0,  LOG_NORMAL    },
    { "equipment",  cmd_equipment,    POS_DEAD,        0,  LOG_NORMAL    },
    { "help",       cmd_help,         POS_DEAD,        0,  LOG_NORMAL    },
    { "info",       cmd_help,         POS_DEAD,        0,  LOG_NORMAL    },
    { "history",    cmd_history,      POS_DEAD,        0,  LOG_NORMAL    }, 
    { "idea",       cmd_idea,         POS_DEAD,        0,  LOG_NORMAL    },
    { "report",     cmd_report,       POS_DEAD,        0,  LOG_NORMAL    },
    { "money",      cmd_money,        POS_DEAD,        0,  LOG_NORMAL    },
    { "map",        cmd_map,          POS_DEAD,        0,  LOG_NORMAL    },
    { "mana",       cmd_mana,         POS_DEAD,        0,  LOG_NORMAL    },

    { "worth",      cmd_money,        POS_DEAD,       -1,  LOG_NORMAL    },
    { "wealth",     cmd_money,        POS_DEAD,       -1,  LOG_NORMAL    },

    { "score",      cmd_score,        POS_DEAD,        0,  LOG_NORMAL    },
    { "socials",    cmd_socials,      POS_DEAD,        0,  LOG_NORMAL    },
    { "skills",     cmd_skills,       POS_DEAD,        0,  LOG_NORMAL    },
    { "time",       cmd_time,         POS_DEAD,        0,  LOG_NORMAL    },
    { "typo",       cmd_typo,         POS_DEAD,        0,  LOG_NORMAL    },
    { "weather",    cmd_time,         POS_DEAD,        0,  LOG_NORMAL    },
    { "who",        cmd_who,          POS_DEAD,        0,  LOG_NORMAL    },
    { "where",      cmd_where,     POS_RESTING,        0,  LOG_NORMAL    },
    { "wizlist",    cmd_wizlist,      POS_DEAD,        0,  LOG_NORMAL    },
                    
    /*
     * Configuration commands.
     */

    { "Configuration",  NULL,            POS_FIGHTING,   -2,  LOG_NEVER  },

    { "set",        cmd_config,       POS_DEAD,        0,  LOG_NORMAL    },
    { "flag",       cmd_flag,         POS_DEAD,        0,  LOG_NORMAL    },
    { "config",     cmd_config,             -1,       -1,  LOG_NORMAL    },
    { "alias",      cmd_alias,        POS_DEAD,        0,  LOG_NORMAL    },
    { "title",      cmd_title,       POS_DEAD,         0,  LOG_NORMAL    },
#if defined(TRANSLATE) 
    { "translate",  cmd_translate,   POS_DEAD,         0,  LOG_NORMAL    },
#endif
    { "compact",    cmd_compact,      POS_DEAD,        0,  LOG_NORMAL    },
    { "brief",      cmd_brief,        POS_DEAD,        0,  LOG_NORMAL    },
    { "blank",      cmd_blank,        POS_DEAD,        0,  LOG_NORMAL    },
    { "ansi",       cmd_ansi,         POS_DEAD,        0,  LOG_NORMAL    },
    { "tips",       cmd_tips,         POS_DEAD,        0,  LOG_NORMAL    },
    { "color",      cmd_ansi,         POS_DEAD,        0,  LOG_NORMAL    },
    { "pager",      cmd_pager,        POS_DEAD,        0,  LOG_NORMAL    },
    { "prompt",     cmd_prompt,       POS_DEAD,        0,  LOG_NORMAL    },

    { "clear",      cmd_clear,        POS_DEAD,        0,  LOG_NORMAL    },

    { "password",   cmd_password,     POS_DEAD,        0,  LOG_NEVER     },

    /*
     * Communication commands.
     */

    { "Communication",  NULL,        POS_FIGHTING,       -2,  LOG_NEVER   },

    { "emote",      cmd_emote,       POS_RESTING,         0,  LOG_NORMAL  },
    { ":",          cmd_emote,       POS_RESTING,         0,  LOG_NORMAL  },
    { "note",       cmd_note,        POS_RESTING,         0,  LOG_NORMAL  },
    { "say",        cmd_say,         POS_RESTING,         0,  LOG_NORMAL  },
    { "ask",        cmd_say,         POS_RESTING,         0,  LOG_NORMAL  },
    { "talk",       cmd_talk,        POS_RESTING,         0,  LOG_NORMAL  },
    { "'",          cmd_say,         POS_RESTING,         0,  LOG_NORMAL  },
    { "\"",         cmd_say,         POS_RESTING,         0,  LOG_NORMAL  },
    { "shout",      cmd_shout,       POS_RESTING,         0,  LOG_NORMAL  },
    { "speak",      cmd_speak,       POS_DEAD,            0,  LOG_NORMAL  },
    { "contact",    cmd_tell,        -1,                 -1,  LOG_NORMAL  },
    { "tell",       cmd_tell,        POS_RESTING,         0,  LOG_NORMAL  },
    { "reply",      cmd_reply,       POS_RESTING,         0,  LOG_NORMAL  },
    { "chat",       cmd_chat,        POS_DEAD,            0,  LOG_NORMAL  },
    { "wish",       cmd_wish,        POS_RESTING,         0,  LOG_NORMAL  },
    { "whisper",    cmd_whisper,     POS_RESTING,         0,  LOG_NORMAL  },
    { "gtell",      cmd_gtell,       POS_RESTING,         0,  LOG_NORMAL  },
    { "rp",         cmd_smote,       POS_RESTING,         0,  LOG_NORMAL  },
    { "smote",      cmd_smote,       -1,                  0,  LOG_NORMAL  },
    { ";",          cmd_gtell,       POS_RESTING,         0,  LOG_NORMAL  },
    { "ooc",        cmd_ooc,         POS_RESTING,         0,  LOG_NORMAL  },

    /*
     * Object manipulation commands.
     */

    { "Manipulation",  NULL,         POS_FIGHTING,       -2,  LOG_NEVER   },

    { "close",      cmd_close,       POS_RESTING,         0,  LOG_NORMAL  },
    { "dump",       cmd_dump,        POS_RESTING,         0,  LOG_NORMAL  },
    { "drag",       cmd_drag,        POS_RESTING,         0,  LOG_NORMAL  },
    { "draw",       cmd_draw,        POS_RESTING,         0,  LOG_NORMAL  },
    { "drink",      cmd_drink,       POS_RESTING,         0,  LOG_NORMAL  },
    { "drop",       cmd_drop,        POS_RESTING,         0,  LOG_NORMAL  },
    { "eat",        cmd_eat,         POS_RESTING,         0,  LOG_NORMAL  },
    { "extinguish", cmd_extinguish,  POS_RESTING,         0,  LOG_NORMAL  },
    { "fill",       cmd_fill,        POS_RESTING,         0,  LOG_NORMAL  },
    { "give",       cmd_give,        POS_RESTING,         0,  LOG_NORMAL  },
    { "hold",       cmd_hold,        POS_RESTING,         0,  LOG_NORMAL  },
    { "light",      cmd_light,       POS_RESTING,         0,  LOG_NORMAL  },
    { "list",       cmd_list,        POS_RESTING,         0,  LOG_NORMAL  },
    { "lock",       cmd_lock,        POS_RESTING,         0,  LOG_NORMAL  },
    { "lower",      cmd_lower,       POS_RESTING,         0,  LOG_NORMAL  },
    { "open",       cmd_open,        POS_RESTING,         0,  LOG_NORMAL  },
    { "pick",       cmd_pick,        POS_RESTING,         0,  LOG_NORMAL  },
    { "pour",       cmd_pour,        POS_RESTING,         0,  LOG_NORMAL  },
    { "put",        cmd_put,         POS_RESTING,         0,  LOG_NORMAL  },
    { "raise",      cmd_raise,       POS_RESTING,         0,  LOG_NORMAL  },
    { "remove",     cmd_remove,      POS_RESTING,         0,  LOG_NORMAL  },
    { "reload",     cmd_reload,      POS_RESTING,         0,  LOG_NORMAL  },
    { "repair",     cmd_repair,      POS_RESTING,         0,  LOG_NORMAL  },
    { "sell",       cmd_sell,        POS_RESTING,         0,  LOG_NORMAL  },
    { "swap",       cmd_swap,        POS_RESTING,         0,  LOG_NORMAL  },
    { "take",       cmd_get,         POS_RESTING,         0,  LOG_NORMAL  },
    { "unlock",     cmd_unlock,      POS_RESTING,         0,  LOG_NORMAL  },
    { "use",        cmd_use,         POS_RESTING,         0,  LOG_NORMAL  },
    { "wear",       cmd_wear,        POS_RESTING,         0,  LOG_NORMAL  },

    { "Skill Improvement",  NULL,  POS_FIGHTING, -2,  LOG_NEVER   },

    { "learn",      cmd_learn,     POS_RESTING,          0,  LOG_NORMAL  },
    { "train",      cmd_learn,   -1,          0,  LOG_NORMAL  },
    { "practice",   cmd_learn,   -1,          0,  LOG_NORMAL  },
    { "apprentice", cmd_learn,   -1,          0,  LOG_NORMAL  },
    { "teach",      cmd_train,     POS_RESTING,           0,  LOG_NORMAL  },

    /*
     * Miscellaneous commands.
     */
    { "Miscellaneous",  NULL,      POS_FIGHTING,         -2,  LOG_NEVER   },

    { "dismount",   cmd_dismount,  POS_RESTING,           0,  LOG_NORMAL  },
    { "dismiss",    cmd_dismiss,   POS_RESTING,           0,  LOG_NORMAL  },
    { "follow",     cmd_follow,    POS_RESTING,           0,  LOG_NORMAL  },
    { "peek",       cmd_peek,      POS_DEAD,              0,  LOG_NORMAL  },

    { "hide",       cmd_hide,      POS_RESTING,           0,  LOG_NORMAL  },
    { "hitch",      cmd_hitch,     POS_STANDING,          0,  LOG_NORMAL  },
    { "ride",       cmd_mount,     POS_STANDING,          0,  LOG_NORMAL  },
    { "mount",      cmd_mount,     POS_STANDING,          0,  LOG_NORMAL  },
    { "rush",       cmd_rush,      POS_STANDING,          0,  LOG_NORMAL  },
    { "trample",    cmd_trample,   POS_STANDING,          0,  LOG_NORMAL  }, 
    { "mount",      cmd_mount,     POS_STANDING,          0,  LOG_NORMAL  },

    { "qui",        cmd_qui,             -1,             -1,  LOG_NORMAL  },
    { "quit",       cmd_quit,      POS_DEAD,              0,  LOG_NORMAL  },
/*#if defined(NEVER) */
    { "save",       cmd_save,      POS_DEAD,              0,  LOG_NORMAL  },
/*#endif*/
    { "sleep",      cmd_sleep,     POS_SLEEPING,          0,  LOG_NORMAL  },
    { "sneak",      cmd_sneak,     POS_STANDING,          0,  LOG_NORMAL  },
    { "steal",      cmd_steal,     POS_STANDING,          0,  LOG_NORMAL  },
    { "track",      cmd_track,     POS_STANDING,          0,  LOG_NORMAL  },
    { "hunt",       cmd_hunt,      POS_STANDING,          0,  LOG_NORMAL  },
    { "unhitch",    cmd_unhitch,   POS_STANDING,          0,  LOG_NORMAL  },
    { "wake",       cmd_wake,      POS_SLEEPING,          0,  LOG_NORMAL  },
    { "enter",      cmd_enter,     POS_FIGHTING,          0,  LOG_NORMAL  },
    { "leave",      cmd_leave,     POS_FIGHTING,          0,  LOG_NORMAL  },
    { "write",      cmd_write,     POS_RESTING,           0,  LOG_NORMAL  },
    { "game",       cmd_stats,     POS_DEAD,              0,  LOG_NORMAL  },

    /*
     * Immortal commands.  Sorted by level for wizhelp.
     */
    { "Producing",       NULL,      POS_FIGHTING,  MAX_LEVEL,  LOG_NEVER  },

#if defined(NEVER)
    { "hotboot",    cmd_hotboot,    POS_DEAD,      MAX_LEVEL, LOG_ALWAYS  },
#endif
    { "shell",      cmd_shell,     POS_DEAD,    MAX_LEVEL,  LOG_ALWAYS  },
    { "events",     cmd_events,    POS_DEAD,    MAX_LEVEL,  LOG_ALWAYS  },
    { "return",     cmd_return,    POS_DEAD,    MAX_LEVEL,  LOG_NORMAL  },
    { "snoop",      cmd_snoop,     POS_DEAD,    MAX_LEVEL,  LOG_NORMAL  },
    { "switch",     cmd_switch,    POS_DEAD,    MAX_LEVEL,  LOG_ALWAYS  },
    { "hedit",      cmd_hedit,     POS_DEAD,    MAX_LEVEL,  LOG_NORMAL  }, 

    { "Directing",  NULL,          POS_FIGHTING, LEVEL_DEMIGOD, LOG_NEVER },

    { "advance",    cmd_advance,   POS_DEAD,  LEVEL_DEMIGOD,  LOG_ALWAYS  },
    { "zedit",      cmd_zedit,     POS_DEAD,  LEVEL_DEMIGOD,  LOG_NORMAL  },
    { "shutdow",    cmd_shutdow,         -1,  LEVEL_DEMIGOD,  LOG_NORMAL  },
    { "shutdown",   cmd_shutdown,  POS_DEAD,  LEVEL_DEMIGOD,  LOG_ALWAYS  },
    { "allow",      cmd_allow,     POS_DEAD,  LEVEL_DEMIGOD,  LOG_ALWAYS  },
    { "ban",        cmd_ban,       POS_DEAD,  LEVEL_DEMIGOD,  LOG_ALWAYS  },
    { "deny",       cmd_deny,      POS_DEAD,  LEVEL_DEMIGOD,  LOG_ALWAYS  },
    { "oset",       cmd_oset,      POS_DEAD,  LEVEL_DEMIGOD,  LOG_ALWAYS  },
    { "mset",       cmd_mset,      POS_DEAD,  LEVEL_DEMIGOD,  LOG_ALWAYS  },
    { "sset",       cmd_sset,      POS_DEAD,  LEVEL_DEMIGOD,  LOG_ALWAYS  },
    { "log",        cmd_log,       POS_DEAD,  LEVEL_DEMIGOD,  LOG_ALWAYS  },
    { "global",     cmd_global,    POS_DEAD,  LEVEL_DEMIGOD,  LOG_NORMAL  },

    { "Writing",    NULL,          POS_FIGHTING, LEVEL_ANGEL,  LOG_NEVER },

    { "scedit",     cmd_sedit,     POS_DEAD,  LEVEL_ANGEL,  LOG_NORMAL  }, 
    { "skedit",     cmd_skedit,    POS_DEAD,  LEVEL_ANGEL,  LOG_NORMAL  }, 
    { "spedit",     cmd_spedit,    POS_DEAD,  LEVEL_ANGEL,  LOG_NORMAL  }, 
    { "debug",      cmd_script,    POS_DEAD,  LEVEL_ANGEL,  LOG_NORMAL  },
    { "bounty",     cmd_bounty,    POS_DEAD,  LEVEL_ANGEL,  LOG_ALWAYS  },
    { "peace",      cmd_peace,     POS_DEAD,  LEVEL_ANGEL,  LOG_NORMAL  },
    { "recho",      cmd_recho,     POS_DEAD,  LEVEL_ANGEL,  LOG_NORMAL  },
    { "system",     cmd_system,    POS_DEAD,  LEVEL_ANGEL,  LOG_ALWAYS  },
    { "cload",      cmd_charload,  POS_DEAD,  LEVEL_ANGEL,  LOG_NORMAL  },
    { ".",          cmd_gspeak,    POS_DEAD,  LEVEL_ANGEL,  LOG_ALWAYS  },
    { "\\",         cmd_gspeak,    POS_DEAD,  LEVEL_ANGEL,  LOG_ALWAYS  },

    { "Building",   NULL,          POS_FIGHTING, LEVEL_BUILDER, LOG_NEVER  },

    { "cues",       cmd_spawns,    POS_DEAD,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "sedit",      cmd_redit,     POS_DEAD,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "pedit",      cmd_oedit,     POS_DEAD,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "aedit",      cmd_aedit,     POS_DEAD,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "actor",      cmd_aindex,    POS_DEAD,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "prop",       cmd_pindex,    POS_DEAD,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "scene",      cmd_rstat,     POS_DEAD,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "aload",      cmd_mload,     POS_DEAD,  LEVEL_BUILDER,  LOG_ALWAYS  },
    { "pload",      cmd_oload,     POS_DEAD,  LEVEL_BUILDER,  LOG_ALWAYS  },
    { "library",    cmd_library,   POS_DEAD,  LEVEL_BUILDER,  LOG_NEVER   },

    { "stat",       cmd_stat,      POS_DEAD,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "zstat",      cmd_astat,     POS_DEAD,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "zsave",      cmd_zsave,     POS_DEAD,  LEVEL_BUILDER,  LOG_ALWAYS  },
    { "terrain",    cmd_terrain,   POS_DEAD,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "sfind",      cmd_rfind,     POS_DEAD,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "pfind",      cmd_pfind,     POS_DEAD,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "afind",      cmd_afind,     POS_DEAD,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "scfind",     cmd_scfind,    POS_DEAD,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "purge",      cmd_purge,     POS_DEAD,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "zones",      cmd_zones,     POS_DEAD,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "tables",     cmd_table,     POS_DEAD,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "memory",     cmd_memory,    POS_DEAD,  LEVEL_BUILDER,  LOG_NORMAL  },

/* legacy commands */
    { "mload",      cmd_mload,     -1,  LEVEL_BUILDER,  LOG_ALWAYS  },
    { "oload",      cmd_oload,     -1,  LEVEL_BUILDER,  LOG_ALWAYS  },
    { "ofind",      cmd_pfind,     -1,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "mfind",      cmd_afind,     -1,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "rfind",      cmd_rfind,     -1,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "redit",      cmd_redit,     -1,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "oedit",      cmd_oedit,     -1,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "medit",      cmd_aedit,     -1,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "mstat",      cmd_aindex,    -1,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "ostat",      cmd_pindex,    -1,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "rstat",      cmd_rstat,     -1,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "resets",     cmd_spawns,    -1,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "spawns",     cmd_spawns,    -1,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "room",       cmd_rstat,     -1,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "obj",        cmd_pindex,    -1,  LEVEL_BUILDER,  LOG_NORMAL  },
    { "mobile",     cmd_aindex,    -1,  LEVEL_BUILDER,  LOG_NORMAL  },

    { "Agent",    NULL,           POS_FIGHTING, LEVEL_AGENT,  LOG_NEVER },

    { "discon",     cmd_disconnect,POS_DEAD,  LEVEL_AGENT,    LOG_ALWAYS  },
    { "freeze",     cmd_freeze,    POS_DEAD,  LEVEL_AGENT,    LOG_ALWAYS  },
    { "noemote",    cmd_noemote,   POS_DEAD,  LEVEL_AGENT,    LOG_NORMAL  },
    { "sla",        cmd_sla,             -1,  LEVEL_AGENT,    LOG_NORMAL  },
    { "slay",       cmd_slay,      POS_DEAD,  LEVEL_AGENT,    LOG_ALWAYS  },
    { "at",         cmd_at,        POS_DEAD,  LEVEL_AGENT,    LOG_NORMAL  },
    { "echo",       cmd_echo,      POS_DEAD,  LEVEL_AGENT,    LOG_NORMAL  },
    { "reboo",      cmd_reboo,           -1,  LEVEL_SUPREME,  LOG_NORMAL  },
    { "reboot",     cmd_reboot,    POS_DEAD,  LEVEL_SUPREME,  LOG_ALWAYS  },
    { "wizlock",    cmd_wizlock,   POS_DEAD,  LEVEL_SUPREME,  LOG_ALWAYS  },
    { "force",      cmd_force,     POS_DEAD,  LEVEL_SUPREME,  LOG_ALWAYS  },

    { "Legendary",    NULL,     POS_FIGHTING, LEVEL_IMMORTAL,  LOG_NEVER },

    { "users",      cmd_users,     POS_DEAD, LEVEL_IMMORTAL,  LOG_NORMAL  },
    { "sockets",    cmd_sockets,   POS_DEAD, LEVEL_IMMORTAL,  LOG_NORMAL  },
    { "comlist",    cmd_comlist,   POS_DEAD, LEVEL_IMMORTAL,  LOG_NORMAL  },
    { "restore",    cmd_restore,   POS_DEAD, LEVEL_IMMORTAL,  LOG_ALWAYS  },
    { "transfer",   cmd_transfer,  POS_DEAD, LEVEL_IMMORTAL,  LOG_ALWAYS  },
    { "wizify",     cmd_wizify,    POS_DEAD, LEVEL_IMMORTAL,  LOG_ALWAYS  },
    { "bamfin",     cmd_bamfin,    POS_DEAD, LEVEL_IMMORTAL,  LOG_NORMAL  },
    { "bamfout",    cmd_bamfout,   POS_DEAD, LEVEL_IMMORTAL,  LOG_NORMAL  },
    { "goto",       cmd_goto,      POS_DEAD, LEVEL_IMMORTAL,  LOG_NORMAL  },
    { "holylight",  cmd_holylight, POS_DEAD, LEVEL_IMMORTAL,  LOG_NORMAL  },
    { "invis",      cmd_invis,     POS_DEAD, LEVEL_IMMORTAL,  LOG_NORMAL  },
    { "notify",     cmd_notify,    POS_DEAD, LEVEL_IMMORTAL,  LOG_NORMAL  },

    { "Heros",          NULL,         POS_FIGHTING, LEVEL_HERO,  LOG_NEVER  },

    { "immtalk",    cmd_immtalk,   POS_DEAD,    LEVEL_HERO,   LOG_NORMAL  },
    { "god",        cmd_immtalk,         -1,    -1,           LOG_NORMAL  },
    { "]",          cmd_immtalk,   POS_DEAD,    LEVEL_HERO,   LOG_NORMAL  },
    { "wizhelp",    cmd_wizhelp,   POS_DEAD,    LEVEL_HERO,   LOG_NORMAL  },

    /*
     * End of list.  (this is the sentinel)
     */
    { "",           NULL,         POS_DEAD,              0,  LOG_NORMAL }
};



void cmd_library( PLAYER *ch, char *argument ) {

    if ( *argument == '\0' ) { cmd_at( ch, "2000 pfind zone" ); return; }

    if ( is_number(argument) ) {
        int dbkey=0;  int level=atoi(argument);

        for( ; dbkey <= top_dbkey_actor; dbkey++ ) {
           char buf[MSL];
           ACTOR_TEMPLATE *pActor = get_actor_template( dbkey );
           if ( pActor 
             && pActor->exp > (level/2) 
             && pActor->exp < (level+level/2) ) {
              sprintf( buf, "[%d] %s | %dk %dgp %dxp\n\r", 
                         pActor->dbkey, pActor->short_descr,
                         pActor->karma, pActor->money, pActor->exp );
              to_actor( buf, ch );
           }
        }
    }

    if ( !str_cmp( argument, "scripted" ) ) {
        int dbkey=0;

        for( ; dbkey <= top_dbkey_actor; dbkey++ ) {
           char buf[MSL];
           ACTOR_TEMPLATE *pActor = get_actor_template( dbkey );
           if ( pActor && pActor->instances && pActor->instances->script ) {
              sprintf( buf, "A[%d] %s (%s)\n\r", 
                         pActor->dbkey, pActor->short_descr, 
                         pActor->instances->script->name );
              to_actor( buf, ch );
           }
        }
    }

    if ( !str_cmp( argument, "items" ) ) {
        int dbkey=0;

        for( ; dbkey <= top_dbkey_prop; dbkey++ ) {
           char buf[MSL];
           PROP_TEMPLATE *pProp = get_prop_template( dbkey );

           if ( pProp && pProp->instances && pProp->instances->script ) {
              sprintf( buf, "P[%d] %s (%s)\n\r", 
                         pProp->dbkey, pProp->short_descr, 
                         pProp->instances->script->name );
              to_actor( buf, ch );
           }
        }
    }

    if ( !str_cmp( argument, "wardrobe" ) ) {
        int dbkey=0;

        for( ; dbkey <= top_dbkey_prop; dbkey++ ) {
           char buf[MSL];
           PROP_TEMPLATE *pProp = get_prop_template( dbkey );
           if ( pProp && pProp->item_type == ITEM_CLOTHING ) {
              sprintf( buf, "[%d] %s (%d to AC bonus) %s\n\r", 
                         pProp->dbkey, pProp->short_descr, pProp->value[1],
                         wear_bit_name(pProp->wear_flags) );
              to_actor( buf, ch );
           }
        }
    }

    if ( !str_cmp( argument, "armor" ) ) {
        int dbkey=0;

        for( ; dbkey <= top_dbkey_prop; dbkey++ ) {
           char buf[MSL];
           PROP_TEMPLATE *pProp = get_prop_template( dbkey );
           if ( pProp && pProp->item_type == ITEM_ARMOR ) {
              sprintf( buf, "[%d] %s (%d to AC bonus) %s\n\r", 
                         pProp->dbkey, pProp->short_descr, pProp->value[0],
                         wear_bit_name(pProp->wear_flags) );
              to_actor( buf, ch );
           }
        }
    }

    if ( !str_cmp( argument, "weapons" ) ) {
        int dbkey=0;

        for( ; dbkey <= top_dbkey_prop; dbkey++ ) {
           char buf[MSL];
           PROP_TEMPLATE *pProp = get_prop_template( dbkey );
           if ( pProp && pProp->item_type == ITEM_WEAPON ) {
              sprintf( buf, "[%d] %s (%d to %d) %s\n\r", 
                         pProp->dbkey, pProp->short_descr, pProp->value[1],
                         pProp->value[2], attack_table[pProp->value[3]].name );
              to_actor( buf, ch );
           }
        }
    }
}


int comlist [512];

/*
 * Syntax:  comlist
 */
void cmd_comlist( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char final[4 * MAX_STRING_LENGTH];
    int cmd;

    final[0] = '\0';

    for ( cmd = 0; command_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( command_table[cmd].level == -2 )
        {
             to_actor( "\n\r", ch );
             display_interp( ch, "^B" );
             to_actor( command_table[cmd].name, ch );
             to_actor( "\n\r", ch );
             display_interp( ch, "^N" );
        }
        else {
        sprintf( buf, "%10s %2d%s", command_table[cmd].name,
                                    comlist[cmd],
                                    ((cmd+1) % 5 == 0) ? "\n\r" : " " );
        strcat( final, buf );
        }
    }

    page_to_actor( final, ch );
    return;
}

    

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 * Is not called with scripts anymore.
 */
void interpret( PLAYER *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char logline[MAX_INPUT_LENGTH];
    int cmd;
    int trust;
    bool found = FALSE;
    char *original;

    /*
     * Strip leading spaces.
     */
    while ( *argument == ' ' )
	argument++;
    if ( argument[0] == '\0' )
	return;

    original = argument;

    /*
     * Implement freeze command.
     */
    if ( !NPC(ch) && IS_SET(ch->flag, PLR_FREEZE) )
    {
    to_actor( "You are encased in a solid block of ice.\n\r", ch );
	return;
    }

   /* Save our souls. */
    if ( !NPC(ch) ) cmd_save( ch, "internal" );

    /*
     * Add to traced script if tracing.
     */
    if ( !NPC(ch) && ch->userdata->trace != NULL )
    {
        char cmmnd[MAX_INPUT_LENGTH+10];
        char buf[MAX_STRING_LENGTH];
        SCRIPT *trace;

        trace = ch->userdata->trace;

        if ( ch->userdata->trace_wait <= 0 )
        sprintf( cmmnd, "do({%s});\n\r", argument );
        else
        sprintf( cmmnd, "do({%s}); wait(%d);\n\r", argument,
                                                   ch->userdata->trace_wait );

        sprintf( buf, "%s", trace->commands );

        if ( strlen( buf ) + strlen( cmmnd ) >= ( MAX_STRING_LENGTH - 4 ) )
        {
            to_actor( "Script too long, truncating.\n\r", ch );
            sprintf( buf, "%s%s", buf, cmmnd ); //strncat( buf, cmmnd, MAX_STRING_LENGTH );
            free_string( trace->commands );
            trace->commands = str_dup( buf );
            cmd_sedit( ch, "trace" );
        }
        else
        {
            strcat( buf, cmmnd );
            free_string( trace->commands );
            trace->commands = str_dup( buf );
            to_actor( "TRACE --->  ", ch );
            to_actor( cmmnd, ch );
        }
    }

    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     */
    strcpy( logline, argument );
    if ( !isalpha(argument[0]) && !isdigit(argument[0]) )
    {
	command[0] = argument[0];
	command[1] = '\0';
	argument++;
	while ( isspace(*argument) )
	    argument++;
    }
    else
    {
	argument = one_argument( argument, command );
    }


    /*
     * Look for a command on a actor.
     * To avoid endless loops frozen in one spot, goto cannot be used
     * as the start of a command trigger.
     */
    if ( !NPC(ch) )  /* Mobs aren't able trigger actors this way, sorry. */
    {
        if ( ch->in_scene != NULL
          && str_cmp( command, "goto" )
          && !found )
        {
            PLAYER *actor;

            for ( actor = ch->in_scene->people;
                  actor != NULL;
                  actor = actor->next_in_scene )
            {
                if ( ch == actor ) continue;
                if ( !can_see( ch, actor ) ) continue;

                if ( script_update( actor, TYPE_ACTOR, TRIG_COMMAND,
                                    ch, NULL, command, argument ) )
                return; 
            }
        }
    }

    /*
     * Trigger commands on props in your inventory.
     */
    if ( ch->carrying != NULL
      && str_cmp( command, "goto" )
      && !found )
    {
        PROP *prop;

        for ( prop = ch->carrying;
              prop != NULL;
              prop = prop->next_content )
        {
            if ( !can_see_prop( ch, prop ) ) continue;

            if ( script_update( prop, TYPE_PROP, TRIG_COMMAND,
                                ch, NULL, command, argument ) )
            return;
        }
    }

    /*
     * Find a command on a prop in the scene or an exit prop match.
     */
    if ( ch->in_scene != NULL
      && str_cmp( command, "goto" )
      && !found )
    {
        PROP *prop;
        char *command_dup;

        for ( prop = ch->in_scene->contents;
              prop != NULL;
              prop = prop->next_content )
        {
            if ( !can_see_prop( ch, prop ) ) continue;

            if ( script_update( prop, TYPE_PROP, TRIG_COMMAND,
                                ch, NULL, command, argument ) )
            return;
        }
                
        command_dup = str_dup( command );
        
      for ( prop = ch->in_scene->contents; prop != NULL; prop = prop->next_content )
        {
   	    if ( can_see_prop( ch, prop ) && is_name( command_dup, STR(prop, name) ) )
	    {
            if ( prop != NULL 
              && prop->item_type == ITEM_FURNITURE 
              && IS_SET(prop->value[1], FURN_EXIT) )
	    {
	     	cmd_enter( ch, command_dup );
	       	free_string( command_dup );
	       	return;
            }
            }
        }

        free_string( command_dup );
    }
    
    /*
     * Find a command on a scene.
     */
    if ( !found
      && ch->in_scene != NULL
      && script_update( ch->in_scene, TYPE_SCENE, TRIG_COMMAND,
                        ch, NULL, command, argument ) )
    return;

    /*
     * Look for command in command table.
     */
    trust = get_trust( ch );
    if ( NPC(ch) ) trust = LEVEL_SUPREME;
    for ( cmd = 0; command_table[cmd].name[0] != '\0'; cmd++ )
    {
    if ( command_table[cmd].level == -2 ) continue;

	if ( command[0] == command_table[cmd].name[0]
    &&   !str_prefix( command, command_table[cmd].name ) )
	{
        if (NPC(ch) && (ch->desc == NULL)
                       && command_table[cmd].level <= FORCE_LEVEL )
        found = TRUE;
        else
        if (command_table[cmd].level <= trust)      found = TRUE;

	    break;
    }
    }

    /* Execute aliases */
    if ( !NPC(ch) && IS_SET(ch->flag2,PLR_ALIASES) ) {
      ALIAS *alias = find_alias( ch, command );
      if ( alias ) {
         interpret( ch, alias->exp );
         return;
      }  
    }

    /*
     * Log and snoop.
     */
    if ( command_table[cmd].log != LOG_NEVER )
    {
   
    if ( ( !NPC(ch) && IS_SET(ch->flag, PLR_LOG) )
    ||   fLogAll
    ||   ( command_table[cmd].log == LOG_ALWAYS && !NPC(ch) ) )
    {
        LOG_LEVEL = get_trust(ch);
        sprintf( log_buf, "Log %s: %s", STR(ch,name), logline );
        log_string( log_buf );
        LOG_LEVEL = LEVEL_IMMORTAL;
    }


    if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
    {
        char buf[MAX_STRING_LENGTH];
        sprintf( buf, "%s", STR(ch,name) );
        write_to_buffer( ch->desc->snoop_by, buf,     2 );
        sprintf( buf, "%% " );
        write_to_buffer( ch->desc->snoop_by, buf,     2 );
        write_to_buffer( ch->desc->snoop_by, logline, 0 );
        write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
    }
    
    }
    
    if ( !found )
    {
	/*
	 * Look for command in socials table.
	 */
	if ( !check_social( ch, command, argument ) 
#if defined(IMC)
   	&&   !imc_command_hook( ch, command, argument ) 
#endif
           ) {
          to_actor( original, ch );
          to_actor( ": ", ch );
         switch( number_range(0,6) ) {
	case 0:   to_actor( "I don't understand.\n\r", ch ); break;
	case 1:   to_actor( "Perhaps you typed a myth-nomer?\n\r", ch ); break;
	case 2:   to_actor( "What?\n\r", ch ); break;
	case 3:   to_actor( "Please explain in a different way.\n\r", ch ); break;
	case 4:   to_actor( "Qu'est-ce-que c'est?\n\r", ch ); break;
        case 5:   to_actor( "Huh?\n\r", ch ); break;
          default:  to_actor( "I don't think I can do that.\n\r", ch ); break;
             }
	}
        return;
    }
    /*
     * Character not in position for command?
     */
    if ( ch->position < command_table[cmd].position )
    {
	switch( ch->position )
	{
	case POS_DEAD:
            to_actor( "You're dead, you aren't able move.\n\r", ch );
	    break;

	case POS_MORTAL:
	case POS_INCAP:
	    to_actor( "You are hurt far too bad for that.\n\r", ch );
	    break;

	case POS_STUNNED:
	    to_actor( "You are too stunned to do that.\n\r", ch );
	    break;

	case POS_SLEEPING:
            to_actor( "You are asleep, off in dreamland.\n\r", ch );
	    break;

    case POS_SITTING:
	case POS_RESTING:
            to_actor( "You'll have to stand up first.\n\r", ch);
	    break;

	case POS_FIGHTING:
            to_actor( "You are in the middle of combat!\n\r", ch);
	    break;

	}
	return;
    }


    /*
     * Dispatch the command and fix FORCE_LEVEL for the next loser--er user.
     */
    (*command_table[cmd].cmd_fun) ( ch, argument );
    comlist[cmd]++;
    ch->timer = 0;
    FORCE_LEVEL = LEVEL_HERO-1;
    tail_chain( );
    return;
}



bool check_social( PLAYER *ch, char *command, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    PLAYER *victim;
    int cmd;
    bool found;

    found  = FALSE;
    for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == social_table[cmd].name[0]
	&&   !str_prefix( command, social_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
	return FALSE;

    if ( !NPC(ch) && IS_SET(ch->flag2, PLR_NO_EMOTE) )
    {
	to_actor( "You are anti-social!\n\r", ch );
	return TRUE;
    }

    switch ( ch->position )
    {
    case POS_DEAD:
        to_actor( "You're dead, you aren't able move.\n\r", ch );
	return TRUE;

    case POS_INCAP:
    case POS_MORTAL:
	to_actor( "You are hurt far too bad for that.\n\r", ch );
	return TRUE;

    case POS_STUNNED:
	to_actor( "You are too stunned to do that.\n\r", ch );
    return TRUE;

    case POS_SLEEPING:
	/*
	 * I just know this is the path to a 12" 'if' statement.  :(
	 * But two players asked for it already!  -- Furey
	 */
	if ( !str_cmp( social_table[cmd].name, "snore" ) )
	    break;
        to_actor( "You are asleep, off in dreamland.\n\r", ch );
	return TRUE;

    }

    one_argument( argument, arg );
    victim = NULL;
    if ( arg[0] == '\0' )
    {
	act( social_table[cmd].others_no_arg, ch, NULL, victim, TO_SCENE    );
	act( social_table[cmd].actor_no_arg,   ch, NULL, victim, TO_ACTOR    );
    }
    else if ( ( victim = get_actor_scene( ch, arg ) ) == NULL )
    {
	to_actor( "They aren't here.\n\r", ch );
    }
    else if ( victim == ch )
    {
	act( social_table[cmd].others_auto,   ch, NULL, victim, TO_SCENE    );
	act( social_table[cmd].actor_auto,     ch, NULL, victim, TO_ACTOR    );
    }
    else
    {
	act( social_table[cmd].others_found,  ch, NULL, victim, TO_NOTVICT );
	act( social_table[cmd].actor_found,    ch, NULL, victim, TO_ACTOR    );
	act( social_table[cmd].vict_found,    ch, NULL, victim, TO_VICT    );

    /*
	if ( !NPC(ch) && NPC(victim)
	&&   !IS_AFFECTED(victim, BONUS_CHARM)
    &&   IS_AWAKE(victim)
    &&   victim->desc == NULL )
	{
	    switch ( number_bits( 4 ) )
	    {
	    case 0:
        oroc( victim, ch );
        cmd_peace( ch, "" );
        break;

	    case 1: case 2: case 3: case 4:
	    case 5: case 6: case 7: case 8:
        act( social_table[cmd].others_found, victim, NULL, ch, TO_NOTVICT );
        act( social_table[cmd].actor_found,   victim, NULL, ch, TO_ACTOR    );
        act( social_table[cmd].vict_found,   victim, NULL, ch, TO_VICT    );
		break;

	    case 9: case 10: case 11: case 12:
		act( "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
		act( "You slap $N.",  victim, NULL, ch, TO_ACTOR    );
		act( "$n slaps you.", victim, NULL, ch, TO_VICT    );
		break;
	    }
	}
    */

    }

    return TRUE;
}




/*
 * Evaluates multiple line command parses and does them all in one pulse.
 * Kinda kludgy, but useful.
void multi_interpret ( PLAYER *actor, char *com_list )
{
    char buf[MAX_STRING_LENGTH];

    while ( com_list[0] != '\0' )
    {
        com_list = next_command( com_list, buf );
        interpret( actor, buf );
    }
    return;
}
 */




/*
 * figures out ansi keyboard commands and assigns them to keys
 */
void ansi_keyboard( PLAYER *ch, char *argument ) {
    /* support for arrow keys in non-character mode */
     char *p=argument; p++;
       switch ( *p ) {
          case 'A': interpret( ch, "north" ); return;
          case 'B': interpret( ch, "south" ); return;
          case 'C': interpret( ch, "east"  ); return;
          case 'D': interpret( ch, "west"  ); return;
          case '1': if ( *(++p) == '~' ) { interpret( ch, "northwest" ); return; }
                    else { // F-keys
                           switch ( *p ) {
                           case '1': interpret( ch, "inventory" ); return;
                           case '2': interpret( ch, "equipment" ); return;
                           case '3': interpret( ch, "score"     ); return;
                           case '4': interpret( ch, "look self" ); return;
                           case '5': interpret( ch, "skills"    ); return;
                           case '7': interpret( ch, "spells"    ); return;
                           case '8': interpret( ch, "group"     ); return;
                           case '9': interpret( ch, "who"       ); return;
                                default: break;
                            }
                         }
          case '2': if ( *(++p) == '~' ) { interpret( ch, "help keyboard" ); return; }
                    else { // F-keys
                           switch ( *p ) {
                           case '0': interpret( ch, "tactic"    ); return;
                           case '1': interpret( ch, "swap"      ); return;
                           case '2': interpret( ch, "consider"  ); return;
                           case '3': interpret( ch, "draw"      ); return;
                           case '4': interpret( ch, "sheath"    ); return;
                            default: break;
                            }
                         }
          case '5': interpret( ch, "northeast" ); return;
          case '6': interpret( ch, "southeast" ); return;
          case '4': interpret( ch, "southwest" ); return;
          case 'G': interpret( ch, "look" ); return;  // center key
          case '3': interpret( ch, "scan" ); return;  // del key
        }      
}
