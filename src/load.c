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

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <zlib.h>

#include "nimud.h"
#include "script.h"
#include "defaults.h"
#include "edit.h"
#include "net.h"

#if !defined(macintosh)
extern	int	_filbuf		args( (FILE *) );
#endif

/*
 * Globals.
 */
SHOP *     shop_first;
SHOP *     shop_last;


char            wtf_logf_buf         [2*MAX_INPUT_LENGTH];
PLAYER *   actor_list;
char *          help_greeting;
char            log_buf         [2*MAX_INPUT_LENGTH];
PROP *      prop_list;

#if defined(INTERGALACTIC_PLANETARY)
TROPOSPHERE    weather;
TIME_INFO  weather;
#else
WEATHER weather;
#endif


/*
 * Locals.
 */
ACTOR_TEMPLATE *	actor_template_hash    [MAX_KEY_HASH];
PROP_TEMPLATE *	prop_template_hash	    [MAX_KEY_HASH];
SCENE *	scene_hash    [MAX_KEY_HASH];
SCRIPT *           script__hash   [MAX_KEY_HASH];
SPELL *            spell__hash    [MAX_KEY_HASH];
SKILL *            skill__hash    [MAX_KEY_HASH];
HELP *             help__hash    [MAX_KEY_HASH];
char *                  string_hash         [MAX_KEY_HASH];

ZONE *		zone_first;
ZONE *		zone_last;

extern char *           string_space;
extern char *           top_string;
extern char             str_empty   [1];

int         LOG_LEVEL = LEVEL_IMMORTAL;

extern int         top_help;

extern int         top_dbkey_script;
extern int         top_dbkey_actor;
extern int         top_dbkey_prop;
extern int         top_dbkey_scene;
extern int         top_dbkey_terrain;

TERRAIN      *terrain_list = NULL;

/*
 * Semi-locals.
 */
bool			fBootDb;
FILE *			fpZone;
char            strzone[MAX_INPUT_LENGTH];

/* values for db2.c */
struct	social_type              social_table	[MAX_SOCIALS];
int     social_count  = 0;


/*
 * From worldgen.c
 */
/*
void    dspace_init      args( ( void ) );
void    fread_map        args( ( char *filename ) );
*/
char *  fread_file       args( ( char * filename ) );

void load_functions( void );

/*
 * Local booting scripts.
 */
void    load_zone         args( ( FILE *fp ) );
void    load_doc          args( ( FILE *fp ) );
void    load_actors       args( ( FILE *fp ) );
void    load_props        args( ( FILE *fp ) );
void    load_scenes       args( ( FILE *fp ) );
void    load_socials      args( ( FILE *fp ) );
void    load_boards       args( ( void ) );       /* board.c */
void    load_scripts      args( ( FILE *fp ) );
void    load_config       args( ( FILE *fp ) );
void    load_contents     args( ( FILE *fp ) );
void    load_terrain      args( ( FILE *fp ) );
void    load_spells       args( ( FILE *fp ) );
void    load_skills       args( ( FILE *fp ) );

void    dump_section_0         args( ( FILE *fp ) );
void    dump_section_s         args( ( FILE *fp ) );

void    fix_exits         args( ( void ) );
void    fix_mobs          args( ( void ) );

void    set_actor_hp        args( ( PLAYER *actor ) );

#define                 MAX_STRING      2097152
#define			MAX_PERM_BLOCK	131072
#define			MAX_MEM_LIST	11

extern void *          rgFreeList  [MAX_MEM_LIST];
extern const int       rgSizeList  [MAX_MEM_LIST];

extern int         nAllocString;
extern int         sAllocString;
extern int         nAllocPerm;
extern int         sAllocPerm;

extern bool fCopyOverride;
extern bool shut_down;

/*
 * Big mama top level function.
 */
void boot_db( int c, bool fCopyOver )
{
    
    /*
     * Init some data space stuff.
     */
    {
	if ( ( string_space = calloc( 1, MAX_STRING ) ) == NULL )
	{
	    wtf_logf( "Boot_db: aren't able alloc %d string space.", (void *) MAX_STRING );
	    exit( 1 );
	}
	top_string	= string_space;
	fBootDb		= TRUE;
    }

   

    /*
     * Init random number generator.
     */
	init_mm( );

    /*
     * Read in all the zone files.
     */
    {
        FILE *fpList;
        char *strzoneNames[MAX_ZONE];
        char buf1[MAX_STRING_LENGTH];
        char buf2[MAX_STRING_LENGTH];
        int x;

        sprintf( strzone, "%s", ZONE_LIST );

        for ( x = 0; x < MAX_ZONE; x++ ) strzoneNames[x] = NULL;
        
        if ( ( fpList = fopen( ZONE_LIST, "r" ) ) == NULL )
        {
	    perror( ZONE_LIST );
	    exit( 1 );
        }

        for ( x = 0; x < MAX_ZONE; x++ )
        {
            if ( feof( fpList ) ) break;
            strzoneNames[x] = str_dup( fread_word( fpList ) );
            if ( strzoneNames[x][0] == '$' ) break;
        }

        fclose( fpList );

        sprintf( buf1, "Loading zones: [%5d]\n\r", x );
        write_global( buf1 );

        for ( x = 0; x < MAX_ZONE; x++ )
        {

        if ( strzoneNames[x] == NULL ) break;

        sprintf( strzone, "%s", strzoneNames[x] );
            
        if ( strzone[0] == '$' )
                break;

        /*
         * Loading output.
         */
        {
#if defined(unix)
            poll_connections( c );
            clean_connections( );
#endif
            sprintf( buf2, "Loading file [%s]\n", strzone );
#if defined(unix)
            log_string( buf2 );
#endif
            write_global( buf2 );
            connection_output( );
        }


	    if ( strzone[0] == '-' )
	    {
            fpZone = stdin;
	    }
	    else
	    {
            if ( ( fpZone = fopen( strzone, "r" ) ) == NULL )
            {
		    perror( strzone );
		    exit( 1 );
            }
        }

        for ( ; ; )
	    {
        char *word;
                 

		if ( fread_letter( fpZone ) != '#' )
		{
		    wtf_logf( "Boot_db: # not found." );
		    exit( 1 );
		}

		word = fread_word( fpZone );

        if ( word[0] == '$'  )
                     break;

             if ( !str_cmp( word, "DOC" )        ) load_doc       (fpZone); 
        else if ( !str_cmp( word, "SOCIALS"    ) ) load_socials   (fpZone);

        else if ( !str_cmp( word, "CONFIG"     ) ) load_config    (fpZone);
        else if ( !str_cmp( word, "CONTENTS"   ) ) load_contents  (fpZone);
        else if ( !str_cmp( word, "TERRAIN"    ) ) load_terrain   (fpZone);
        else if ( !str_cmp( word, "SPELL"      ) ) load_spells    (fpZone);
        else if ( !str_cmp( word, "SKILL"      ) ) load_skills    (fpZone);

        else if ( !str_cmp( word, "ZONEDATA"   )
               || !str_cmp( word, "ZONE" ) 
               || !str_cmp( word, "AREADATA" ) )   load_zone      (fpZone);
        else if ( !str_cmp( word, "MOBDATA"   )
               || !str_cmp( word, "ACTORS" )
               || !str_cmp( word, "ACTORDATA" ) )  load_actors   (fpZone);
        else if ( !str_cmp( word, "OBJDATA"  )
               || !str_cmp( word, "PROPS"    )
               || !str_cmp( word, "PROP"     ) 
               || !str_cmp( word, "PROPDATA" ) )   load_props   (fpZone);
        else if ( !str_cmp( word, "SCENES" )
               || !str_cmp( word, "SCENEDATA" )
               || !str_cmp( word, "ROOMDATA"  ) )  load_scenes     (fpZone);
        else if ( !str_cmp( word, "SCRIPTDATA" ) ) load_scripts   (fpZone);

/* Diku support removed */      

		else
		{
		    wtf_logf( "Boot_db: bad section name %s.", word );
		    exit( 1 );
		}
	    }

	    if ( fpZone != stdin )
		fclose( fpZone );
	    fpZone = NULL;
	}

    }

    /*
     * Fix up exits.
     * Declare db booting over.
     * Set up d-space.
     * Spawn all zones once.
     * Load up the notes file.
     */
#if defined(unix)
    poll_connections( c );
    clean_connections( );
#endif

    write_global( "Performing consistency check.\n\r" );
#if defined(unix)
    log_string( "Performing consistency check." );
#endif
    connection_output( );
    fix_exits( );
    //fix_mobs( );

    update_player_statistics();    

#if defined(unix)
    poll_connections( c );
    clean_connections( );
#endif


#if defined(unix)
    poll_connections( c );
    clean_connections( );
#endif

/*
 * Support for web export.
 * Commented out because there's a memory glitch.
 */
#if defined(HTML)
    log_string( "Exporting HTML content." );
    html_export_helps();
#endif

    help_greeting = fread_file( GREETING_FILE );

    write_global( "Spawning world.\n\r" );
#if defined(unix)
    log_string( "Spawning world." );
#endif
    connection_output( );
    update_handler( );

#if defined(unix)
    poll_connections( c );
    clean_connections( );
#endif
    

    write_global( "Loading bulletin boards.\n\r" );
#if defined(unix)
    log_string( "Loading bulletin boards." );
#endif
    connection_output( );
    load_boards( );

#if defined(unix)
    poll_connections( c );
    clean_connections( );
#endif

    fBootDb = FALSE;
    /*
     * Send the greeting.
     */
    {
	extern char * help_greeting;
	if ( help_greeting[0] == '.' )
        write_global( help_greeting+1 );
	else
        write_global( help_greeting   );
    }

    if (fCopyOver) copyover_recover();
    return;
}


/*
 * Be careful giving users access to the filename.
 * Truncates large files to MSL-2.
 */
char *fread_file( char *filename ) {
      FILE *fl;
      char buf[MAX_STRING_LENGTH];
/*      char *text;*/
      int i;

      buf[0] = '\0';

      if ( ( fl = fopen( filename, "r" ) ) == NULL )
      {
          return str_dup(buf);
      }

   /* Read in the text from the file.  Make sure it doesn't overflow what
      we've allocated for it. */

      i =0;
      while( i < MSL-4 )
      {
          if (feof(fl) || ferror(fl) )
              break;

          buf[i] = fgetc(fl);
          if ( buf[i++] == '\n' ) buf[i++] = '\r';   /* unix2dos */
      }
      buf[i] = '\0';

      fclose(fl);
      return str_dup( buf );
}


#if defined(KEY)
#undef KEY
#endif

#if defined(SKEY)
#undef SKEY
#endif

#define KEY( literal, field, value )                \
                if ( !str_cmp( word, literal ) )    \
                {                                   \
                    field  = value;                 \
                    fMatch = TRUE;                  \
                    break;                          \
                }

#define SKEY( string, field )                       \
                if ( !str_cmp( word, string ) )     \
                {                                   \
                    free_string( field );           \
                    field = fread_string( fp );     \
                    fMatch = TRUE;                  \
                    break;                          \
                }


/*
 * Snarf an 'zone' header line.
 */
void load_zone( FILE *fp )
{
    ZONE *pZone;
    char      *word;
    bool      fMatch;

    pZone               = new_zone( );
    pZone->age          = 15;
    pZone->nplayer      = 0;
    free_string( pZone->filename );
    pZone->filename     = str_dup( strzone );

    for ( ; ; )
    {
       word   = feof( fp ) ? "End" : fread_word( fp );
       fMatch = FALSE;

       switch ( UPPER(word[0]) )
       {
           case 'N':
            SKEY( "N", pZone->name );
            break;
           case 'S':
             KEY( "S", pZone->security, fread_number( fp ) );
            if ( !str_cmp( word, "Static" ) )
            {
                SET_BIT( pZone->zone_flags, ZONE_STATIC );
                fMatch = TRUE;
            }
            break;
           case 'V':
            if ( !str_cmp( word, "V" ) )
            {
                pZone->ldbkey = fread_number( fp );
                pZone->udbkey = fread_number( fp );
                fMatch = TRUE;
            }
            break;
           case 'E':
             if ( !str_cmp( word, "End" ) )
             {
                 fMatch = TRUE;
				 if ( zone_first == NULL )	 zone_first = pZone;
				 if ( zone_last  != NULL )	 zone_last->next = pZone;
                 zone_last   = pZone;
                 pZone->next = NULL;
                 return;
            }
            break;
           case 'B':
            SKEY( "B", pZone->builders );
            break;
           case 'R':
            SKEY( "R",  pZone->repop );
            break;
       }

       if ( !fMatch )
       {
           char buf[80];
           sprintf( buf, "Fread_zone: '%s', incorrect keyword: %s",
                         strzone, word );
           wtf_logf( buf );
           fread_to_eol( fp );
       }
    }
}


/*
 * Date and time information.
 */
void load_config( FILE *fp )
{
    char      *word;
    bool      fMatch;
    extern BAN *      ban_free;
    extern BAN *      ban_list;
    BAN *pban;

    /*
     * Preset time and weather.
     */
    weather.hour  = 0;
    weather.day   = 1;
    weather.month = 0;
    weather.year  = 453;

	     if ( weather.hour <  5 ) weather.sunlight = SUN_DARK;
	else if ( weather.hour <  6 ) weather.sunlight = SUN_RISE;
	else if ( weather.hour < 19 ) weather.sunlight = SUN_LIGHT;
	else if ( weather.hour < 20 ) weather.sunlight = SUN_SET;
	else                            weather.sunlight = SUN_DARK;

    weather.change  = 0;
    weather.winddir = 0;
        
    if ( weather.month <= 4 || weather.month >= 12 )
         weather.temperature = number_fuzzy( 20 );
    else weather.temperature = number_fuzzy( 50 );
             
    weather.windspeed = number_range( 1, 10 );
    weather.mmhg       = 960;
        
	if ( weather.month >= 7 && weather.month <=12 )
         weather.mmhg += number_range( 1, 50 );
    else weather.mmhg += number_range( 1, 80 );

	     if ( weather.mmhg <=  980 ) weather.sky = SKY_LIGHTNING;
	else if ( weather.mmhg <= 1000 ) weather.sky = SKY_RAINING;
	else if ( weather.mmhg <= 1020 ) weather.sky = SKY_CLOUDY;
	else                                  weather.sky = SKY_CLOUDLESS;

    for ( ; ; )
    {
       word   = feof( fp ) ? "End" : fread_word( fp );
       fMatch = FALSE;

       switch ( UPPER(word[0]) )
       {
         case 'D':
            if ( !str_cmp( word, "Date" ) )
            {
                weather.hour  = fread_number( fp );
                weather.day   = fread_number( fp );
                weather.month = fread_number( fp );
                weather.year  = fread_number( fp );
                fMatch = TRUE;
            }
            break;
         case 'B':
            if ( !str_cmp( word, "Ban" ) )
            {
                if ( ban_free == NULL )
                {
                pban        = alloc_perm( sizeof(*pban) );
                }
                else
                {
                pban        = ban_free;
                ban_free    = ban_free->next;
                }

                pban->name  = fread_string( fp );
                pban->next  = ban_list;
                ban_list    = pban;
            }
         case 'W':
            if ( !str_cmp( word, "WizLocked" ) )
            {
                wizlock = TRUE;
                fMatch = TRUE;
            }
            break;
         case 'M':
            if ( !str_cmp( word, "Moon" ) )
            {
                weather.moon_phase = fread_number( fp );
                weather.next_phase = fread_number( fp );
                fMatch = TRUE;
            }
            break;
         case 'N':
            if ( !str_cmp( word, "NewLocked" ) )
            {
                newlock = TRUE;
                fMatch = TRUE;
            }
            break;
         case 'E':
            if ( !str_cmp( word, "End" ) )
            {
                  if ( weather.hour <  5 ) weather.sunlight = SUN_DARK;
             else if ( weather.hour <  6 ) weather.sunlight = SUN_RISE;
             else if ( weather.hour < 19 ) weather.sunlight = SUN_LIGHT;
             else if ( weather.hour < 20 ) weather.sunlight = SUN_SET;
             else                            weather.sunlight = SUN_DARK;

                  if ( weather.month <= 4 || weather.month >= 12 )
                  weather.temperature = number_fuzzy( 20 );
             else weather.temperature = number_fuzzy( 50 );
                  return;
            }
            break;
       }

       if ( !fMatch )
       {
           char buf[80];
           sprintf( buf, "load_config: incorrect keyword %s", word );
           wtf_logf( buf );
           fread_to_eol( fp );
       }
    }
    return;
}


/*
 * Load the terrain data.
 */
void load_terrain( FILE *fp )
{
    TERRAIN *pTerrain;
    char      *word;
    bool      fMatch;

    pTerrain                    = new_terrain( );
    pTerrain->dbkey              = fread_number( fp );

    pTerrain->next              = terrain_list;
    terrain_list                = pTerrain;

    if ( pTerrain->dbkey > top_dbkey_terrain )
    top_dbkey_terrain = pTerrain->dbkey;

/*    wtf_logf ( "begin new terrain (%d)", pTerrain->dbkey );      */

    for ( ; ; )
    {
       word   = feof( fp ) ? "End" : fread_word( fp );
       fMatch = FALSE;

       switch ( UPPER(word[0]) )
       {
          case '*': fread_to_eol( fp ); break;
          case 'E':
            if ( !str_cmp( word, "End" ) )
            {
                 return;
            }
           break;
          case 'N':
           SKEY( "N",      pTerrain->name );
           break;
          case 'C':
               {
                /*
                 * Skip blanks.
                 * Read first char.
                 */
                do
                {
                pTerrain->map_char = getc( fp );
                }
                while ( isspace(pTerrain->map_char) );
                fMatch = TRUE;
               }
           break;
          case 'D':
           SKEY( "Dwi",       pTerrain->winter );
           SKEY( "Dsu",       pTerrain->summer );
           SKEY( "Dfa",       pTerrain->fall   );
           SKEY( "Dsp",       pTerrain->spring );
          case 'S':
            KEY( "S",        pTerrain->move, fread_number( fp ) );
       }

       if ( !fMatch )
       {
           char buf[80];
           sprintf( buf, "fread_terrain: %d incorrect: %s",
                         pTerrain->dbkey, word );
           wtf_logf( buf );
           fread_to_eol( fp );
       }
    };
}



/*
 * Load spell data.
 * Ex:

#SPELL 1
N find familiar~
Lvl 0
Target 0
Delay 0
Mana 20
Type 0
Group 1400
Sc 10
End

 */
void load_spells( FILE *fp )
{
    SPELL *pSpell;
    char      *word;
    bool      fMatch;
    int iHash, dbkey;

    pSpell                      = new_spell( );
    dbkey=pSpell->dbkey           = fread_number( fp );

    if ( pSpell->dbkey > top_dbkey_spell )
    top_dbkey_spell = pSpell->dbkey;

    for ( ; ; )
    {
       word   = feof( fp ) ? "End" : fread_word( fp );
       fMatch = FALSE;

       switch ( UPPER(word[0]) )
       {
          case '*': fread_to_eol( fp ); break;
          case 'E':
            if ( !str_cmp( word, "End" ) )
            {
               iHash                   = dbkey % MAX_KEY_HASH;
               pSpell->next            = spell__hash[iHash];
               spell__hash[iHash] = pSpell;
               return;
            }
           break;
          case 'N':
           SKEY( "N",      pSpell->name );
           break;
          case 'L':
           KEY( "Lvl", pSpell->level, fread_number( fp )     ); break;
          case 'M':
           KEY( "Mana",  pSpell->mana_cost, fread_number(fp) ); break;
          case 'S':
           if ( !str_cmp( word, "Sc" ) ) {
               SCRIPT *pScript;
               pScript = get_script_index( fread_number(fp) );
               if ( pScript ) {
               INSTANCE *pInstance;
               pInstance = new_instance();
               pInstance->script = pScript;
               pInstance->next = pSpell->instances;
               pSpell->instances = pInstance;                 
               }
               fMatch=TRUE;
           } break;
          case 'G':
           KEY( "Group",     pSpell->group_code, fread_number(fp) ); break;
          case 'T':
           KEY( "Target",    pSpell->target, fread_number(fp) ); 
           KEY( "Type",      pSpell->mana_type, fread_number(fp) ); break;
          case 'D':
           KEY( "Delay",     pSpell->delay, fread_number(fp) ); break;
       }

       if ( !fMatch )
       {
           char buf[80];
           sprintf( buf, "fread_spell: %d incorrect: %s",
                         pSpell->dbkey, word );
           wtf_logf( buf );
           fread_to_eol( fp );
       }
    };
}


/*
 * Load skill data.
 */
void load_skills( FILE *fp )
{
    SKILL *pSkill;
    char      *word;
    bool      fMatch;
    int iHash, dbkey;

    pSkill                      = new_skill( );
    dbkey=pSkill->dbkey           = fread_number( fp );

    if ( pSkill->dbkey > top_dbkey_skill )
    top_dbkey_skill = pSkill->dbkey;

    for ( ; ; )
    {
       word   = feof( fp ) ? "End" : fread_word( fp );
       fMatch = FALSE;

       switch ( UPPER(word[0]) )
       {
          case '*': fread_to_eol( fp ); break;
          case 'E':
            if ( !str_cmp( word, "End" ) )
            {
               iHash                   = dbkey % MAX_KEY_HASH;
               pSkill->next            = skill__hash[iHash];
               skill__hash[iHash] = pSkill;
               return;
            }
           break;
          case 'N':SKEY( "N",    pSkill->name );                        break;
          case 'L': KEY( "Lvl",  pSkill->level,   fread_number( fp ) ); break;
          case 'C': KEY( "Con",  pSkill->req_con, fread_number(fp) ); 
                    KEY( "Cost", pSkill->cost,    fread_number(fp) ); break;
          case 'S': KEY( "Slot", pSkill->slot,    fread_number(fp) ); 
                    KEY( "Str",  pSkill->req_str, fread_number(fp) ); break;
          case 'I': KEY( "Int",  pSkill->req_int, fread_number(fp) ); break;
          case 'W': KEY( "Wis",  pSkill->req_wis, fread_number(fp) ); break;
          case 'R': KEY( "Rate", pSkill->learn_rate, fread_number(fp) );
                    KEY( "ReqPer", pSkill->required_percent, fread_number(fp) );
                   break;
          case 'M': KEY( "MaxPrac", pSkill->max_prac, fread_number(fp) );
                   SKEY( "MsgOff",  pSkill->msg_off );
                    KEY( "MaxLearn", pSkill->max_learn, fread_number(fp) ); break;
          case 'G':
           KEY( "Group",     pSkill->group_code, fread_number(fp) ); break;
          case 'T':
           KEY( "Target",    pSkill->target, fread_number(fp)     ); break;
          case 'D':
           KEY( "Dex",       pSkill->req_dex, fread_number(fp)    );
           KEY( "Delay",     pSkill->delay, fread_number(fp)      ); break;
       }

       if ( !fMatch )
       {
           char buf[80];
           sprintf( buf, "fread_skill: %d incorrect: %s",
                         pSkill->dbkey, word );
           wtf_logf( buf );
           fread_to_eol( fp );
       }
    };
}


/*
 * Loads function documentation data from the functions.c file
 */
void load_functions( ) {
  FILE *fp;
  char buf[MSL];

  sprintf( buf, "%sfunctions.c", SOURCE_DIR );

  if ( ( fp = fopen( buf, "r" ) ) == NULL ) return;

    
}


/*
 * Load a help DOC data.
 */
void load_doc( FILE *fp )
{
    HELP *pHelp;
    char      *word;
    bool      fMatch;
    int iHash, dbkey;

    pHelp                       = new_help( );
    dbkey=pHelp->dbkey            = fread_number( fp );

    if ( pHelp->dbkey > top_dbkey_help )
    top_dbkey_help = pHelp->dbkey;

    for ( ; ; )
    {
       word   = feof( fp ) ? "End" : fread_word( fp );
       fMatch = FALSE;

       switch ( UPPER(word[0]) )
       {
          case '*': fread_to_eol( fp ); break;
          case 'E':SKEY( "EX", pHelp->example  );
            if ( !str_cmp( word, "End" ) )
            {
               iHash                   = dbkey % MAX_KEY_HASH;
               pHelp->next             = help__hash[iHash];
               help__hash[iHash]  = pHelp;
               return;
            }
           break;
          case 'N':SKEY( "N",  pHelp->name     ); break;
          case 'I':SKEY( "IT", pHelp->immtext  ); break;
          case 'K':SKEY( "KW", pHelp->keyword  ); break;
          case 'S':SKEY( "SA", pHelp->seealso  ); 
                   SKEY( "SY", pHelp->syntax   ); break;
          case 'T':SKEY( "TX", pHelp->text     ); break;
          case 'L': KEY( "L",  pHelp->level, fread_number(fp) ); break;
          case 'C': KEY( "C",  pHelp->class, fread_number(fp) ); break;
       }

       if ( !fMatch )
       {
           char buf[80];
           sprintf( buf, "fread_help: %d incorrect: %s",
                         pHelp->dbkey, word );
           wtf_logf( buf );
           fread_to_eol( fp );
       }
    };
}



void load_contents( FILE *fp )
{
    char      *word;
    bool      fMatch;
    int dbkey = SCENE_VNUM_TEMPLATE;
    SCENE *pScene = get_scene( dbkey );

    for ( ; ; )
    {
       word   = feof( fp ) ? "End" : fread_word( fp );
       fMatch = FALSE;

       switch ( UPPER(word[0]) )
       {
         case 'S':
            if ( !str_cmp( word, "Scene" ) )
            {
                int iNest;

                dbkey = fread_number( fp );
                pScene = get_scene( dbkey );
                if ( pScene == NULL )
                {
                wtf_logf( "Load_contents: Invalid scene %d.", dbkey );
                pScene = get_scene( SCENE_VNUM_TEMPLATE );
                }

                for ( iNest = 0; iNest < MAX_NEST; iNest++ )
                rgObjNest[iNest] = NULL;

                fMatch = TRUE;
            } else
            if ( !str_cmp( word, "Stop" ) )
            {
                return;
            }
            break;
         case 'W':
            if ( !str_cmp( word, "Wagon" ) )
            {
                int wdbkey;
                PROP_TEMPLATE *pPropIndex;
                PROP *pProp;

                wdbkey = fread_number( fp );
                pPropIndex = get_prop_template( pScene->wagon );
                if ( pPropIndex != NULL
                  && get_scene( wdbkey ) != NULL )
                {
                    for ( pProp = prop_list; pProp != NULL; pProp = pProp->next )
                    {
                        if ( pProp->pIndexData == pPropIndex
                          && pProp->in_scene != NULL )
                             break;
                    }

                    if ( pProp == NULL )
                    {
                        pProp = create_prop( pPropIndex, 0 );
                        prop_to_scene( pProp, get_scene( wdbkey ) );
                    }
                }

                fMatch = TRUE;
            }
         case '#':
            if ( !str_cmp( word+1, "OBJECT" )
              || !str_cmp( word+1, "PROP" ) 
              || !str_cmp( word+1, "PROPS" ) )
            {
                fread_prop( pScene, TYPE_SCENE, fp );
                fMatch = TRUE;
            }
       }

       if ( !fMatch )
       {
           char buf[80];
           sprintf( buf, "Load_contents: incorrect keyword %s", word );
           wtf_logf( buf );
           fread_to_eol( fp );
       }
    }
    return;
}


/*
 * Snarf a socials file.
 * This code was freely distributed with the Rivers of Mud source code.
 *                                                 -Locke
 */
void load_socials( FILE *fp)
{
    for ( ; ; ) 
    {
    	struct social_type social;
    	char *temp;
        /* clear social */
        social.actor_no_arg    = NULL;
        social.others_no_arg  = NULL;
        social.actor_found     = NULL;
        social.others_found   = NULL;
        social.vict_found     = NULL; 
        social.actor_not_found = NULL;
        social.actor_auto      = NULL;
        social.others_auto    = NULL;

        temp = fread_word(fp);
        if ( *temp == '*' )
        {
        fread_to_eol(fp);
        continue;
        }

    	if (!strcmp(temp,"#0"))
	    return;  /* done */

    	strcpy(social.name,temp);
    	fread_to_eol(fp);

	temp = fread_string_eol(fp);
	if (!strcmp(temp,"$"))
	     social.actor_no_arg = NULL;
	else if (!strcmp(temp,"#"))
	{
	     social_table[social_count] = social;
	     social_count++;
	     continue; 
	}
        else
	    social.actor_no_arg = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.others_no_arg = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
        social.others_no_arg = str_dup( temp );

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.actor_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
       	else
        social.actor_found = str_dup( temp );

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.others_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
        social.others_found = str_dup( temp );

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.vict_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
        social.vict_found = str_dup( temp );

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.actor_not_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
        social.actor_not_found = str_dup( temp );

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.actor_auto = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;

             social_count++;
             continue;
        }
        else
	    social.actor_auto = temp;
         
        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.others_auto = NULL;
        else if (!strcmp(temp,"#"))
        {
             social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
        social.others_auto = str_dup( temp );
	
	social_table[social_count] = social;
    	social_count++;
   }
   return;
}


/*
 * Snarf the Woolley Catalog file; a publically distributed file
 * from NASA's HEARSARC array
 * File format is;
 * # -> comment
 * | -> seperator character
 *                                                 -Locke
 */
void load_starmap( FILE *fp)
{
    for ( ; ; ) 
    {
    	struct social_type social;
    	char *temp;
        /* clear social */
        social.actor_no_arg    = NULL;
        social.others_no_arg  = NULL;
        social.actor_found     = NULL;
        social.others_found   = NULL;
        social.vict_found     = NULL; 
        social.actor_not_found = NULL;
        social.actor_auto      = NULL;
        social.others_auto    = NULL;

        temp = fread_word(fp);
        if ( *temp == '*' )
        {
        fread_to_eol(fp);
        continue;
        }

    	if (!strcmp(temp,"#0"))
	    return;  /* done */

    	strcpy(social.name,temp);
    	fread_to_eol(fp);

	temp = fread_string_eol(fp);
	if (!strcmp(temp,"$"))
	     social.actor_no_arg = NULL;
	else if (!strcmp(temp,"#"))
	{
	     social_table[social_count] = social;
	     social_count++;
	     continue; 
	}
        else
	    social.actor_no_arg = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.others_no_arg = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
        social.others_no_arg = str_dup( temp );

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.actor_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
       	else
        social.actor_found = str_dup( temp );

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.others_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
        social.others_found = str_dup( temp );

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.vict_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
        social.vict_found = str_dup( temp );

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.actor_not_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
        social.actor_not_found = str_dup( temp );

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.actor_auto = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;

             social_count++;
             continue;
        }
        else
	    social.actor_auto = temp;
         
        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.others_auto = NULL;
        else if (!strcmp(temp,"#"))
        {
             social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
        social.others_auto = str_dup( temp );
	
	social_table[social_count] = social;
    	social_count++;
   }
   return;
}



void fread_shop( FILE *fp, ACTOR_TEMPLATE *pActorIndex )
{
    SHOP *pShop;
    char *word;
    bool fMatch;

    pShop           = new_shop( );

    for ( ; ; )
    {
        word   = feof( fp ) ? "EndShop" : fread_word( fp );
        fMatch = FALSE;

        switch( UPPER(word[0]) )
        {
         case 'P':
            if ( !str_cmp( word, "P" ) )
            {
                pShop->profit_buy   = fread_number( fp );
                pShop->profit_sell  = fread_number( fp );
                fMatch = TRUE;
            }
           break;
         case 'T':
            if ( !str_cmp( word, "T" ) )
            {

                pShop->buy_type[fread_number(fp)] = fread_number( fp );
                fMatch = TRUE;
            }
            if ( !str_cmp( word, "Tr" ) )
            {

                pShop->trades[fread_number(fp)] = fread_number( fp );
                fMatch = TRUE;
            }
           break;
         case 'H':
            if ( !str_cmp( word, "H" ) )
            {
                pShop->open_hour    = fread_number( fp );
                pShop->close_hour   = fread_number( fp );
                fMatch = TRUE;
            }
           break;
         case 'F':
            KEY( "F",        pShop->shop_flags,     fread_number( fp ) );
           break;
         case 'R':
            KEY( "R",        pShop->repair_rate,    fread_number( fp ) );
           break;
         case 'B':
            KEY( "BI",       pShop->buy_index,      fread_number( fp ) );
          break;
         case 'C':
            KEY( "CI",       pShop->comp_index,     fread_number( fp ) );
          break;
         case 'S':
           SKEY( "Str1",     pShop->no_such_item    );
           SKEY( "Str2",     pShop->cmd_not_buy      );
           SKEY( "Str3",     pShop->list_header     );
           SKEY( "Str4",     pShop->hours_excuse    );
            KEY( "SI",       pShop->sell_index,     fread_number( fp ) );
          break;
         case 'E':
            if ( !str_cmp( word, "EndShop" ) )
            {
                pActorIndex->pShop    = pShop;
                pShop->keeper       = pActorIndex->dbkey;

                if ( shop_first == NULL )
                     shop_first = pShop;
                if ( shop_last  != NULL )
                     shop_last->next = pShop;

                shop_last   = pShop;
                pShop->next = NULL;
                return;
            }
            if ( !str_cmp( word, "End" ) )
            {
                wtf_logf( "Fread_shop: incomplete shop %d",  (pActorIndex->dbkey) );
                free_shop( pShop );
                pActorIndex->pShop = NULL;
                return;
            }
           break;
        }

        if ( !fMatch )
        {
            char buf[80];
            sprintf( buf, "Fread_shop: incorrect word %s (%d)", word, pActorIndex->dbkey );
            wtf_logf( buf );
            fread_to_eol( fp );
        }
    }

    return;
}



/*
 * Load a single actor declaration.
 */
void fread_actor2( FILE *fp, int dbkey )
{
    ACTOR_TEMPLATE *pActorIndex;
    char      *word;
    bool      fMatch;
    int iHash;

    pActorIndex                   = new_actor_template( );
    pActorIndex->dbkey             = dbkey;
    pActorIndex->zone             = zone_last;

/*    wtf_logf ( "begin new actor (%d)", dbkey );      */

    for ( ; ; )
    {
       word   = feof( fp ) ? "End" : fread_word( fp );
       fMatch = FALSE;

       switch ( UPPER(word[0]) )
       {
          case '*': fread_to_eol( fp ); break;
          case 'E':
            if ( !str_cmp( word, "End" ) )
            {
                 iHash                                   = dbkey % MAX_KEY_HASH;
                 pActorIndex->next                 = actor_template_hash[iHash];
                 actor_template_hash[iHash]   = pActorIndex;
                 return;
            }
            KEY( "E", pActorIndex->exp, fread_number( fp ) );
           break;
          case 'K':
            KEY( "K", pActorIndex->karma, fread_number( fp ) );
           break;
          case 'A':
            KEY( "A",   pActorIndex->flag,         fread_number( fp ) );
            KEY( "AB", pActorIndex->bonuses, fread_number( fp ) );
            if ( !str_cmp( word, "AP" ) )
            {
                pActorIndex->perm_str = fread_number( fp );
                pActorIndex->perm_int = fread_number( fp );
                pActorIndex->perm_wis = fread_number( fp );
                pActorIndex->perm_dex = fread_number( fp );
                pActorIndex->perm_con = fread_number( fp );
                fMatch = TRUE;
            }
            if ( !str_cmp( word, "At" ) )
            {
                int num;
                ATTACK *attack;

                fMatch = TRUE;
                num    = fread_number( fp );
                if ( num >= MAX_ATTACK )
                {
                    wtf_logf( "Invalid attack index (%d)", num );
                }
                else
                {

                if ( pActorIndex->attacks[num] == NULL )
                {
                    pActorIndex->attacks[num]  =
                     alloc_perm( sizeof(*pActorIndex->attacks[num]) );
                }
                else
                {
                    wtf_logf( "Overwrote pre-existing attack (%d).", num );
                }

                attack = pActorIndex->attacks[num];
                attack->dam1             = fread_number( fp );
                attack->dam2             = fread_number( fp );
                attack->idx              = fread_number( fp );
                pActorIndex->attacks[num]        = attack;
                }
            }
           break;
          case 'M':
            KEY( "M",     pActorIndex->money,   fread_number( fp ) );
           break;
          case 'N':
           SKEY( "N",      pActorIndex->name );
           break;
          case 'S':
            if ( !str_cmp( word, "Spell" ) )
            {
                 SPELL_BOOK *pSpell;
                 pSpell = new_spell_book( );
                 pSpell->next = pActorIndex->pSpells;
                 pSpell->dbkey = fread_number( fp );
                 pActorIndex->pSpells = pSpell;
                 fMatch = TRUE;
            }  
            KEY( "S",       pActorIndex->sex,         fread_number( fp ) );
            KEY( "Sz",       pActorIndex->size,        fread_number( fp ) );
           SKEY( "SD",       pActorIndex->short_descr );
            if ( !str_cmp( word, "Shop" ) )
            {
                fread_shop( fp, pActorIndex );
                fMatch = TRUE;
            }

            if ( !str_cmp( word, "Sk" ) )
            {
            SKILL *pSkill;
            int value;

            value = fread_number( fp );
            pSkill = skill_lookup( fread_word(fp) );

            if ( !pSkill )
            wtf_logf( "Fread_actor2: unknown skill" );
            else { 
                pSkill = skill_copy( pSkill );
                pSkill->next = pActorIndex->learned;
                pSkill->skill_level = value;
                pActorIndex->learned = pSkill;
            }
            fMatch = TRUE;
            }

            if ( !str_cmp( word, "Sc" ) )
            {
                INSTANCE *pTrig;
                SCRIPT *pScript = get_script_index( fread_number(fp) );

                if ( pScript ) {
                pTrig = new_instance( );
                pTrig->script = pScript;  pTrig->next = pActorIndex->instances;
                pActorIndex->instances = pTrig;
                }
                fMatch = TRUE;
            }
           break;
           case 'O': SKEY( "O", pActorIndex->owner ); break;
          case 'L':
           SKEY( "LD",      pActorIndex->long_descr );       break;
          case 'D':
           SKEY( "D",     pActorIndex->description );      break;
          case 'B':
            KEY( "Bucks",     pActorIndex->bucks,   fread_number( fp ) ); break;

          case 'C':
            KEY( "Credits",     pActorIndex->credits,   fread_number( fp ) ); break;
       }

       if ( !fMatch )
       {
           char buf[80];
           sprintf( buf, "fread_actor2: %d incorrect: %s",
                         dbkey, word );
           wtf_logf( buf );
           fread_to_eol( fp );
       }
    };
}


/*
 * Snarf a actor section.
 */
void load_actors( FILE *fp )
{
    if ( zone_last == NULL )
    {
    wtf_logf( "Load_actors: no #ZONE seen yet." );
	exit( 1 );
    }

    for ( ; ; )
    {
    int dbkey;
	char letter;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
        wtf_logf( "Load_actors: '#' not found." );
	    exit( 1 );
	}

	dbkey				= fread_number( fp );
    if ( dbkey == 0 )
            break;

	fBootDb = FALSE;
	if ( get_actor_template( dbkey ) != NULL )
	{
	    wtf_logf( "Load_actors: dbkey %d duplicated.", dbkey );
	    exit( 1 );
	}
	fBootDb = TRUE;

    fread_actor2( fp, dbkey );
    top_dbkey_actor = top_dbkey_actor < dbkey ? dbkey : top_dbkey_actor;
    }
    return;
}


/*
 * Fixes the slot numbers on certain props.
void fix_prop( PROP_TEMPLATE *pPropIndex )
{
    fBootDb = FALSE;
	switch ( pPropIndex->item_type )
	{
	case ITEM_PILL:
	case ITEM_POTION:
	case ITEM_SCROLL:
    case ITEM_FOUNTAIN:
	    pPropIndex->value[1] = slot_lookup( pPropIndex->value[1] );
	    pPropIndex->value[2] = slot_lookup( pPropIndex->value[2] );
	    pPropIndex->value[3] = slot_lookup( pPropIndex->value[3] );
	    break;

	case ITEM_STAFF:
	case ITEM_WAND:
    case ITEM_AMMO:
	    pPropIndex->value[3] = slot_lookup( pPropIndex->value[3] );
	    break;

	}
    fBootDb = TRUE;
    return;
}
 */


void fread_prop_template( FILE *fp, int dbkey )
{
    PROP_TEMPLATE *pPropIndex;
    int iHash;
    char      *word;
    bool      fMatch;

    pPropIndex                   = new_prop_template( );
    pPropIndex->dbkey             = dbkey;
    pPropIndex->zone             = zone_last;

    for ( ; ; )
    {
       word   = feof( fp ) ? "End" : fread_word( fp );
       fMatch = FALSE;

       switch ( UPPER(word[0]) )
       {
          case '*': fread_to_eol( fp ); break;
          case 'E':
            KEY( "E", pPropIndex->extra_flags, fread_number( fp ) );
            if ( !str_cmp( word, "ED" ) )
            {
                EXTRA_DESCR *ed;

                ed                      = new_extra_descr( );
                ed->keyword             = fread_string( fp );
                ed->description         = fread_string( fp );
                ed->next                = pPropIndex->extra_descr;
                pPropIndex->extra_descr  = ed;
                fMatch = TRUE;
            }
            if ( !str_cmp( word, "End" ) )
            {
                iHash                   = dbkey % MAX_KEY_HASH;
                pPropIndex->next         = prop_template_hash[iHash];
                prop_template_hash[iHash]   = pPropIndex;
                return;
            }
           break;
          case 'A':
           SKEY( "A", pPropIndex->action_descr );
            if ( !str_cmp( word, "Af" ) )
            {
                BONUS *paf;

                paf                     = new_bonus( );
                paf->location           = fread_number( fp );
                paf->modifier           = fread_number( fp );
                paf->type               = fread_number( fp );
                paf->duration           = fread_number( fp );
                paf->bitvector          = fread_number( fp );
                paf->next               = pPropIndex->bonus;
                pPropIndex->bonus     = paf;
                fMatch = TRUE;
            }
           break;
          case 'D':
           SKEY( "D", pPropIndex->description );
           SKEY( "DR", pPropIndex->real_description );
           break;
          case 'L':
            KEY( "L", pPropIndex->level,              fread_number( fp ) );
           break;
          case 'T':
            KEY( "Ti", pPropIndex->timer,              fread_number( fp ) );
            KEY( "T", pPropIndex->item_type,           fread_number( fp ) );
           break;
          case 'N':
            SKEY( "N", pPropIndex->name  );
           break;
          case 'S':
            KEY( "Sz", pPropIndex->size,                fread_number( fp ) );
           SKEY( "SD", pPropIndex->short_descr );
            if ( !str_cmp( word, "Sc" ) )
            {
                INSTANCE *pTrig;

                pTrig = new_instance( );
                pTrig->script = get_script_index( fread_number( fp ) );
                pTrig->next = pPropIndex->instances;
                pPropIndex->instances = pTrig;
                fMatch = TRUE;
            }
           break;
          case 'O': SKEY( "O", pPropIndex->owner ); break;
          case 'P':
           SKEY( "P", pPropIndex->short_descr_plural );
           SKEY( "PD", pPropIndex->description_plural );
           break;
          case 'W':
            KEY( "W", pPropIndex->wear_flags,         fread_number( fp ) );
            KEY( "Wt", pPropIndex->weight,           fread_number( fp ) );
           break;
          case 'V':
            if ( !str_cmp( word, "V" ) )
            {
               fMatch = TRUE;
               pPropIndex->value[0] = fread_number( fp );
               pPropIndex->value[1] = fread_number( fp );
               pPropIndex->value[2] = fread_number( fp );
               pPropIndex->value[3] = fread_number( fp );
            }
           break;
          case 'C':
            KEY( "C", pPropIndex->cost,               fread_number( fp ) );
           break;

       }
               if ( !fMatch )
               {
                  sprintf( log_buf, "fread_prop_template: dbkey %d incorrect:  '%s'",
                                    dbkey, word );
                  wtf_logf( log_buf );
                  fread_to_eol( fp );
               }
    };

    return;
}




/*
 * Snarf a prop section.
 */
void load_props( FILE *fp )
{

    if ( zone_last == NULL )
    {
    wtf_logf( "Load_props: no #ZONE seen yet." );
	exit( 1 );
    }

    for ( ; ; )
    {
    int dbkey;
	char letter;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    wtf_logf( "Load_props: # not found." );
	    exit( 1 );
	}

	dbkey				= fread_number( fp );
	if ( dbkey == 0 )
	    break;

	fBootDb = FALSE;
	if ( get_prop_template( dbkey ) != NULL )
	{
	    wtf_logf( "Load_props: dbkey %d duplicated.", dbkey );
	    exit( 1 );
	}
	fBootDb = TRUE;

    fread_prop_template( fp, dbkey );
    top_dbkey_prop = top_dbkey_prop < dbkey ? dbkey : top_dbkey_prop;
    }

    return;
}



void fread_scene( FILE *fp, int dbkey )
{
    SCENE *pSceneIndex;
    int iHash;
    int door;
    char *word;
    bool fMatch;


    pSceneIndex              = new_scene( );
	pSceneIndex->zone		= zone_last;
	pSceneIndex->dbkey		= dbkey;

    for ( ; ; )
    {
       word   = feof( fp ) ? "End" : fread_word( fp );
       fMatch = FALSE;

       switch ( UPPER(word[0]) )
       {
          case '*': fread_to_eol( fp ); break;
          case 'E':
            if ( !str_cmp( word, "End" ) )
            {
                 iHash           = dbkey % MAX_KEY_HASH;
                 pSceneIndex->next    = scene_hash[iHash];
                 scene_hash[iHash]  = pSceneIndex;
                 return;
            }
            if ( !str_cmp( word, "ED" ) )
            {
                 EXTRA_DESCR *ed;

                 ed                       = new_extra_descr( );
                 ed->keyword              = fread_string( fp );
                 ed->description          = fread_string( fp );
                 ed->next                 = pSceneIndex->extra_descr;
                 pSceneIndex->extra_descr  = ed;
                 fMatch = TRUE;
            }
           break;
          case 'C':
		SKEY( "C", pSceneIndex->client );
           break;
          case 'R':
            if ( !str_cmp( word, "Ref" ) )
            {
                 pSceneIndex->template     = fread_number( fp );
                 fMatch = TRUE;
            }
            if ( !str_cmp( word, "R" ) )
            {
                   SPAWN *pSpawn;

                   pSpawn      = new_spawn( );
                   pSpawn->command = fread_letter( fp );
                   pSpawn->rs_dbkey = fread_number( fp );
                   pSpawn->loc     = fread_number( fp );
                   pSpawn->percent = fread_number( fp );
                   pSpawn->num     = fread_number( fp );
                   fread_to_eol( fp );

                   if ( pSceneIndex->spawn_first == NULL )
                        pSceneIndex->spawn_first  = pSpawn;
                   if ( pSceneIndex->spawn_last  != NULL )
                        pSceneIndex->spawn_last->next = pSpawn;

                   pSceneIndex->spawn_last   = pSpawn;
                   pSpawn->next             = NULL;
                   fMatch = TRUE;
            }
           break;
          case 'D':
           SKEY( "D", pSceneIndex->description );
            if ( !str_cmp( word, "Dr" ) )
            {
               EXIT *pexit;

               door                = fread_number( fp );

               if ( door < 0 || door >= MAX_DIR )
               {
                   wtf_logf( "Fread_scenes: dbkey %d has bad door number.", dbkey );
                   exit( 1 );
               }

               pexit           = new_exit( );

               pexit->rs_flags     = fread_number( fp );
               pexit->key          = fread_number( fp );
               pexit->dbkey         = fread_number( fp );

               pexit->description  = fread_string( fp );
               pexit->keyword      = fread_string( fp );

               pSceneIndex->exit[door]  = pexit;
               fMatch = TRUE;
            }

           break;
          case 'M':
            KEY( "M", pSceneIndex->max_people, fread_number( fp ) );
           break;
          case 'N':
           SKEY( "N", pSceneIndex->name );
           break;
          case 'F':
            KEY( "F", pSceneIndex->scene_flags,   fread_number( fp ) );
           break;
          case 'W':
            KEY( "W", pSceneIndex->wagon,        fread_number( fp ) );
           break;
          case 'T':
            if ( !str_cmp( word, "T" ) )
            {
                pSceneIndex->terrain    = fread_number( fp );
                fMatch = TRUE;
            }
           break;
          case 'O': SKEY( "O", pSceneIndex->owner ); break;
          case 'S':
            KEY( "S", pSceneIndex->move, fread_number( fp ) );
            if ( !str_cmp( word, "Sc" ) )
            {
                INSTANCE *pTrig;

                pTrig = new_instance( );
                pTrig->script = get_script_index( fread_number( fp ) );
                pTrig->next = pSceneIndex->instances;
                pSceneIndex->instances = pTrig;
                fMatch = TRUE;
            }
           break;
       }
               if ( !fMatch )
               {
                  char buf[80];
                  sprintf( buf, "fread_scenes: incorrect titler %s on v%d",
                                word, dbkey );
                  wtf_logf( buf );
                  fread_to_eol( fp );
               }
    };

}

/*
 * Snarf a scene section.
 */
void load_scenes( FILE *fp )
{

    if ( zone_last == NULL )
    {
    wtf_logf( "Load_scenes: no #ZONE seen yet." );
        exit( 1 );
    }

    for ( ; ; )
    {
    int dbkey;
    char letter;

        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            wtf_logf( "Load_scenes: # not found." );
            exit( 1 );
        }

        dbkey                            = fread_number( fp );
        if ( dbkey == 0 )
            break;

        fBootDb = FALSE;
        if ( get_scene( dbkey ) != NULL )
        {
            wtf_logf( "Load_scenes: dbkey %d duplicated.", dbkey );
            exit( 1 );
        }
        fBootDb = TRUE;

    fread_scene( fp, dbkey );
    top_dbkey_scene = top_dbkey_scene < dbkey ? dbkey : top_dbkey_scene;
    }

    return;
}



void fread_script( FILE *fp, int dbkey )
{
    SCRIPT *script;
    char        *word;
    bool        fMatch;
    int         iHash;

    script                =  new_script( );
    script->dbkey          =  dbkey;
    script->zone          =  zone_last;

    for ( ; ; )
    {
       word   = feof( fp ) ? "End" : fread_word( fp );
       fMatch = FALSE;

       switch ( UPPER(word[0]) )
       {
          case '*': fread_to_eol( fp ); fMatch = TRUE; break;
          case 'N':
           SKEY( "N", script->name );
           break;
          case 'E':
            if ( !str_cmp( word, "End" ) )
            {
               iHash                    = dbkey % MAX_KEY_HASH;
               script->next             = script__hash[iHash];
               script__hash[iHash] = script;
               script->zone             = get_dbkey_zone(dbkey);
               return;
            }
           break;
          case 'C':
           SKEY( "C", script->commands );
           break;

          case 'T':
            KEY( "T", script->type, fread_number( fp ) );
           break;
       }
               if ( !fMatch )
               {
                  char buf[80];
                  sprintf( buf, "fread_script: incorrect '%s'", word );
                  wtf_logf( buf );
                  fread_to_eol( fp );
               }
    };
}


/*
 * Snarf a script section.
 */
void load_scripts( FILE *fp )
{
    if ( zone_last == NULL )
    {
        wtf_logf( "Load_scripts: no #ZONE seen yet." );
        exit( 1 );
    }

    for ( ; ; )
    {
        int dbkey;
        char letter;

        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            wtf_logf( "Load_scripts: # not found." );
            exit( 1 );
        }

        dbkey                            = fread_number( fp );
        if ( dbkey == 0 )
            break;

        fBootDb = FALSE;
        if ( get_script_index( dbkey ) != NULL )
        {
            wtf_logf( "Load_scripts: dbkey %d duplicated.", dbkey );
            exit( 1 );
        }
        fBootDb = TRUE;

        fread_script( fp, dbkey );
        top_dbkey_script = top_dbkey_script < dbkey ? dbkey : top_dbkey_script;
    }

    return;
}


/*
 * Fix mobs with attrib=0, which means they have been imported or otherwise corrupted.
 */
void fix_mobs( void ) {
   int v=0;  int fDidSomething=0;
   fBootDb=FALSE;
   ATTACK *attack;
   for ( v=0; v<top_dbkey_actor; v++ ) {
      ACTOR_TEMPLATE *a=get_actor_template(v);
      if ( a ) { 
      a->name = strlwr(a->name);
         if ( a->exp < 30 && a->exp != 1 ) { a->exp = number_range(0,1000); 
a->perm_str=a->perm_int=a->perm_wis=a->perm_con=a->perm_dex=0; }
         if ( a->perm_str == 0
          &&  a->perm_int == 0
          &&  a->perm_wis == 0 
          &&  a->perm_con == 0 
          &&  a->perm_dex == 0 ) {  fDidSomething++;
             if ( a->exp < 250 ) {
               a->perm_str = 15;
               a->perm_int = 15;
               a->perm_wis = 15;
               a->perm_dex = 15;
               a->perm_con = 15;

                attack =  alloc_perm( sizeof(*a->attacks[0]) );
                attack->dam1  = 10;       attack->dam2  = number_range(11,25);
                attack->idx   = number_range(0,14); if ( attack->idx == 6 ) attack->idx=5;
                a->attacks[0] = attack;
             } else
             if ( a->exp < 400 ) {
               a->perm_str = 19;
               a->perm_int = 19;
               a->perm_wis = 19;
               a->perm_dex = 19;
               a->perm_con = 19;

                attack =  alloc_perm( sizeof(*a->attacks[0]) );
                attack->dam1  = 10;       attack->dam2  = number_range(11,25);
                attack->idx   = number_range(0,14); if ( attack->idx == 6 ) attack->idx=5;
                a->attacks[0] = attack;

                attack =  alloc_perm( sizeof(*a->attacks[1]) );
                attack->dam1  = 5;       attack->dam2  = number_range(15,25);
                attack->idx   = number_range(0,14); if ( attack->idx == 6 ) attack->idx=5;
                a->attacks[1] = attack;

                if ( number_range(0,10) < 5 ) {
                attack =  alloc_perm( sizeof(*a->attacks[2]) );
                attack->dam1  = 1;       attack->dam2  = number_range(11,45);
                attack->idx   = number_range(0,14); if ( attack->idx == 6 ) attack->idx=5;
                a->attacks[2] = attack;
                }

             } else
             if ( a->exp <= 500 ) {
               a->perm_str = 25;
               a->perm_int = 25;
               a->perm_wis = 25;
               a->perm_dex = 25;
               a->perm_con = 25;

                attack =  alloc_perm( sizeof(*a->attacks[0]) );
                attack->dam1  = 10;       attack->dam2  = number_range(15,20);
                attack->idx   = number_range(0,14); if ( attack->idx == 6 ) attack->idx=5;
                a->attacks[0] = attack;

                attack =  alloc_perm( sizeof(*a->attacks[1]) );
                attack->dam1  = 5;       attack->dam2  = number_range(5,25);
                attack->idx   = number_range(0,14); if ( attack->idx == 6 ) attack->idx=5;
                a->attacks[1] = attack;

                if ( number_range(0,10) < 4 ) {
                attack =  alloc_perm( sizeof(*a->attacks[2]) );
                attack->dam1  = 1;       attack->dam2  = number_range(11,45);
                attack->idx   = number_range(0,14); if ( attack->idx == 6 ) attack->idx=5;
                a->attacks[2] = attack;
                }

                if ( number_range(0,10) < 4 ) {
                attack =  alloc_perm( sizeof(*a->attacks[3]) );
                attack->dam1  = 1;       attack->dam2  = number_range(15,40);
                attack->idx   = number_range(0,22);
                a->attacks[3] = attack;
                }

             } else
             if ( a->exp <= 700 ) {
               a->perm_str = 25;
               a->perm_int = 25;
               a->perm_wis = 25;
               a->perm_dex = 25;
               a->perm_con = 35;

                attack =  alloc_perm( sizeof(*a->attacks[0]) );
                attack->dam1  = 10;       attack->dam2  = number_range(15,20);
                attack->idx   = number_range(0,14); if ( attack->idx == 6 ) attack->idx=5;
                a->attacks[0] = attack;

                attack =  alloc_perm( sizeof(*a->attacks[1]) );
                attack->dam1  = 5;       attack->dam2  = number_range(5,25);
                attack->idx   = number_range(0,14); if ( attack->idx == 6 ) attack->idx=5;
                a->attacks[1] = attack;

                if ( number_range(0,10) < 4 ) {
                attack =  alloc_perm( sizeof(*a->attacks[2]) );
                attack->dam1  = 1;       attack->dam2  = number_range(11,45);
                attack->idx   = number_range(0,14); if ( attack->idx == 6 ) attack->idx=5;
                a->attacks[2] = attack;
                }

                if ( number_range(0,10) < 4 ) {
                attack =  alloc_perm( sizeof(*a->attacks[3]) );
                attack->dam1  = 1;       attack->dam2  = number_range(15,40);
                attack->idx   = number_range(0,22);
                a->attacks[3] = attack;
                }

                if ( number_range(0,10) < 4 ) {
                attack =  alloc_perm( sizeof(*a->attacks[4]) );
                attack->dam1  = 1;       attack->dam2  = number_range(15,40);
                attack->idx   = number_range(0,22);
                a->attacks[4] = attack;
                }

                if ( number_range(0,10) < 4 ) {
                attack =  alloc_perm( sizeof(*a->attacks[5]) );
                attack->dam1  = 1;       attack->dam2  = number_range(15,40);
                attack->idx   = number_range(0,22);
                a->attacks[5] = attack;
                }

             } else
             if ( a->exp <= 1000 ) {
               a->perm_str = 25;
               a->perm_int = 25;
               a->perm_wis = 25;
               a->perm_dex = 25;
               a->perm_con = 40;

                attack =  alloc_perm( sizeof(*a->attacks[0]) );
                attack->dam1  = 10;       attack->dam2  = number_range(15,80);
                attack->idx   = number_range(0,14); if ( attack->idx == 6 ) attack->idx=5;
                a->attacks[0] = attack;

                attack =  alloc_perm( sizeof(*a->attacks[1]) );
                attack->dam1  = 5;       attack->dam2  = number_range(5,100);
                attack->idx   = number_range(0,14); if ( attack->idx == 6 ) attack->idx=5;
                a->attacks[1] = attack;

                if ( number_range(0,10) < 4 ) {
                attack =  alloc_perm( sizeof(*a->attacks[2]) );
                attack->dam1  = 1;       attack->dam2  = number_range(11,95);
                attack->idx   = number_range(0,14); if ( attack->idx == 6 ) attack->idx=5;
                a->attacks[2] = attack;
                }

                if ( number_range(0,10) < 4 ) {
                attack =  alloc_perm( sizeof(*a->attacks[3]) );
                attack->dam1  = 1;       attack->dam2  = number_range(15,40);
                attack->idx   = number_range(0,22);
                a->attacks[3] = attack;
                }

                if ( number_range(0,10) < 4 ) {
                attack =  alloc_perm( sizeof(*a->attacks[4]) );
                attack->dam1  = 1;       attack->dam2  = number_range(15,40);
                attack->idx   = number_range(0,22);
                a->attacks[4] = attack;
                }

                if ( number_range(0,10) < 4 ) {
                attack =  alloc_perm( sizeof(*a->attacks[5]) );
                attack->dam1  = 1;       attack->dam2  = number_range(15,40);
                attack->idx   = number_range(0,22);
                a->attacks[5] = attack;
                }

                attack =  alloc_perm( sizeof(*a->attacks[6]) );
                attack->dam1  = 5;       attack->dam2  = number_range(5,100);
                attack->idx   = number_range(0,14); if ( attack->idx == 6 ) attack->idx=5;
                a->attacks[6] = attack;


             } else {
               a->perm_str = 45;
               a->perm_int = 25;
               a->perm_wis = 25;
               a->perm_dex = 25;
               a->perm_con = 45;

                attack =  alloc_perm( sizeof(*a->attacks[0]) );
                attack->dam1  = 10;       attack->dam2  = number_range(15,20);
                attack->idx   = number_range(0,14); if ( attack->idx == 6 ) attack->idx=5;
                a->attacks[0] = attack;

                attack =  alloc_perm( sizeof(*a->attacks[1]) );
                attack->dam1  = 5;       attack->dam2  = number_range(5,100);
                attack->idx   = number_range(0,14); if ( attack->idx == 6 ) attack->idx=5;
                a->attacks[1] = attack;

                if ( number_range(0,10) < 4 ) {
                attack =  alloc_perm( sizeof(*a->attacks[2]) );
                attack->dam1  = 1;       attack->dam2  = number_range(11,100);
                attack->idx   = number_range(0,14); if ( attack->idx == 6 ) attack->idx=5;
                a->attacks[2] = attack;
                }

                if ( number_range(0,10) < 4 ) {
                attack =  alloc_perm( sizeof(*a->attacks[3]) );
                attack->dam1  = 1;       attack->dam2  = number_range(15,100);
                attack->idx   = number_range(0,22);
                a->attacks[3] = attack;
                }

                if ( number_range(0,10) < 4 ) {
                attack =  alloc_perm( sizeof(*a->attacks[4]) );
                attack->dam1  = 1;       attack->dam2  = number_range(15,100);
                attack->idx   = number_range(0,22);
                a->attacks[4] = attack;
                }

                if ( number_range(0,10) < 4 ) {
                attack =  alloc_perm( sizeof(*a->attacks[5]) );
                attack->dam1  = 1;       attack->dam2  = number_range(15,100);
                attack->idx   = number_range(0,22);
                a->attacks[5] = attack;
                }

                if ( number_range(0,10) < 4 ) {
                attack =  alloc_perm( sizeof(*a->attacks[5]) );
                attack->dam1  = 1;       attack->dam2  = number_range(15,100);
                attack->idx   = number_range(0,22);
                a->attacks[6] = attack;
                }

                if ( number_range(0,10) < 4 ) {
                attack =  alloc_perm( sizeof(*a->attacks[5]) );
                attack->dam1  = 1;       attack->dam2  = number_range(15,100);
                attack->idx   = number_range(0,22);
                a->attacks[7] = attack;
                }

                if ( number_range(0,10) < 4 ) {
                attack =  alloc_perm( sizeof(*a->attacks[5]) );
                attack->dam1  = 1;       attack->dam2  = number_range(15,100);
                attack->idx   = number_range(0,22);
                a->attacks[8] = attack;
                }

             }
          }
      }
   }

   if ( fDidSomething > 0 ) wtf_logf( "Fixed %d unassigned actors.", fDidSomething );
   fBootDb=TRUE;
}


/*
 * Translate all scene exits from virtual to real.
 * Has to be done after all scenes are read in.
 * Check for bad reverse exits.
 */
void fix_exits( void )
{
    extern const int rev_dir [];
//    char buf[MAX_STRING_LENGTH];
    SCENE *pSceneIndex;
    SCENE *to_scene;
    EXIT *pexit;
    EXIT *pexit_rev;
    int iHash;
    int door;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pSceneIndex  = scene_hash[iHash];
	      pSceneIndex != NULL;
	      pSceneIndex  = pSceneIndex->next )
	{
	    bool fexit;

	    fexit = FALSE;
            for ( door = 0; door < MAX_DIR; door++ )
	    {
		if ( ( pexit = pSceneIndex->exit[door] ) != NULL )
		{
		    fexit = TRUE;
		    if ( pexit->dbkey <= 0 )
			pexit->to_scene = NULL;
		    else
			pexit->to_scene = get_scene( pexit->dbkey );
                    pexit->exit_flags = pexit->rs_flags;
		}
	    }

	    if ( !fexit )
		SET_BIT( pSceneIndex->scene_flags, SCENE_NO_ACTOR );
	}
    }

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pSceneIndex  = scene_hash[iHash];
	      pSceneIndex != NULL;
	      pSceneIndex  = pSceneIndex->next )
	{
        for ( door = 0; door < MAX_DIR; door++ )
	    {
		if ( ( pexit     = pSceneIndex->exit[door]       ) != NULL
		&&   ( to_scene   = pexit->to_scene               ) != NULL
		&&   ( pexit_rev = to_scene->exit[rev_dir[door]] ) != NULL
		&&   pexit_rev->to_scene != pSceneIndex )
		{
#if defined(FIX_EXITS_REPORTING)
		    sprintf( buf, "Fix_exits: %d:%d -> %d:%d -> %d.",
			pSceneIndex->dbkey, door,
			to_scene->dbkey,    rev_dir[door],
			(pexit_rev->to_scene == NULL)
			    ? 0 : pexit_rev->to_scene->dbkey );
		    wtf_logf( buf );
#endif
		}
	    }
        spawn_scene( pSceneIndex );
        script_update( pSceneIndex, TYPE_SCENE, TRIG_BORN, NULL, NULL, NULL, NULL );
	}
    }

    return;
}


/*
 * Create an instance of a actor.
 */
PLAYER *create_actor( ACTOR_TEMPLATE *pActorIndex )
{
    PLAYER *actor;

    if ( pActorIndex == NULL )
    {
	wtf_logf( "Create_actor: NULL pActorIndex." );
    if ( fBootDb ) exit( 1 );

	pActorIndex = get_actor_template( ACTOR_VNUM_TEMPLATE );
    }
    
    actor = new_player( );
    free_user( actor->userdata );
    actor->userdata = NULL;

    actor->pIndexData	= pActorIndex;
 
    actor->learned = pActorIndex->learned;  /* do not free this list */

    actor->name           = NULL;
    actor->short_descr    = NULL;
    actor->long_descr     = NULL;
    actor->description    = NULL;

    actor->position       = POS_STANDING;
    actor->size           = pActorIndex->size;
    actor->race           = pActorIndex->race;
    actor->sex            = pActorIndex->sex;

    actor->perm_str       = pActorIndex->perm_str;
    actor->perm_int       = pActorIndex->perm_int;
    actor->perm_wis       = pActorIndex->perm_wis;
    actor->perm_dex       = pActorIndex->perm_dex;
    actor->perm_con       = pActorIndex->perm_con;
    actor->spells         = pActorIndex->pSpells;
    actor->exp            = pActorIndex->exp;
    actor->karma          = pActorIndex->karma;

    actor->fmode          = pActorIndex->fmode;

    actor->instances      = NULL;
    actor->current        = NULL;

    if ( pActorIndex->instances != NULL )
    {
    INSTANCE *trig, *pTrig;

    for ( pTrig = pActorIndex->instances;  pTrig != NULL;  pTrig = pTrig->next )
    {
        trig               = new_instance( );

        trig->script       = pTrig->script;
        trig->location     = NULL;

        trig->next       = actor->instances;
        actor->instances    = trig;
    }
    }

    actor->timer          = pActorIndex->timer;
    actor->flag            = pActorIndex->flag;
    actor->bonuses  	= pActorIndex->bonuses;

    actor->credits = pActorIndex->credits ? number_range(0,pActorIndex->credits/2) + pActorIndex->credits : 0;
    actor->bucks = pActorIndex->bucks ? number_range(0,pActorIndex->bucks/2) + pActorIndex->bucks : 0;

    if ( pActorIndex->money > 1 )
    create_amount( number_gelify( pActorIndex->money-1 )+1, actor, NULL, NULL );
    else
    if ( pActorIndex->money > 0 ) create_amount( 1, actor, NULL, NULL );

    actor->prompt     = str_dup( "Mob(%h %m): " );
    actor->armor      = 100;
    actor->hit        = MAXHIT(actor);
    actor->move       = MAXMOVE(actor);

    /*
     *  Randomize color, size, and anything else we want!
     */
    actor_strings( actor );

    /*
     * Insert in list.
     */
    actor->next		= actor_list;
    actor_list		= actor;
    pActorIndex->count++;

    return actor;
}



/*
 * Create an instance of a prop.
 */
PROP *create_prop( PROP_TEMPLATE *pPropIndex, int level )
{
    PROP *prop;

    if ( pPropIndex == NULL )
    {
	wtf_logf( "Create_prop: NULL pPropIndex." );
    if ( fBootDb ) exit( 1 );

	pPropIndex = get_prop_template( PROP_VNUM_TEMPLATE );
    }

    if ( pPropIndex->item_type == ITEM_LIST && level != -1 )
    {
        PROP_TEMPLATE *pIndex;
        char *scanarg;
        char buf[20];
        int num;
        int d;

        scanarg    = pPropIndex->description;
        num        = number_range( 1, arg_count( pPropIndex->description ) );
        buf[0]     = '\0';

        for ( d = 0; d < num; d++ )
        {
            scanarg = one_argument( scanarg, buf );
        }

        num       = atoi( buf );
        pIndex = get_prop_template( num );
        if ( pIndex != NULL ) pPropIndex = pIndex;
    }
    else level = 0;

    prop = new_prop( );
    prop->pIndexData	= pPropIndex;

    if (pPropIndex->level == 0)
         prop->level      = level;
    else prop->level      = pPropIndex->level;

    prop->wear_loc           = WEAR_NONE;

    prop->name               = NULL;
    prop->short_descr        = NULL;
    prop->description        = NULL;
    prop->action_descr       = NULL;
    prop->short_descr_plural = NULL;
    prop->description_plural = NULL;
    prop->real_description   = NULL;

    prop->in_scene           = NULL;
    prop->in_prop            = NULL;
    prop->carried_by         = NULL;

    /*
     * Copy triggers onto the prop.
     */
    if ( pPropIndex->instances != NULL )
    {
    INSTANCE *trig, *pTrig;

    for ( pTrig = pPropIndex->instances;  pTrig != NULL;  pTrig = pTrig->next )
    {
        trig               = new_instance( );

        trig->script       = pTrig->script;
        trig->location     = NULL;

        trig->next       = prop->instances;
        prop->instances    = trig;
    }
    }

    prop->item_type      = pPropIndex->item_type;
    prop->extra_flags	= pPropIndex->extra_flags;
    prop->wear_flags     = pPropIndex->wear_flags;
    prop->value[0]       = pPropIndex->value[0];
    prop->value[1]       = pPropIndex->value[1];
    prop->value[2]       = pPropIndex->value[2];
    prop->value[3]       = pPropIndex->value[3];
    prop->size           = pPropIndex->size;
    prop->weight         = pPropIndex->weight;
    if (pPropIndex->cost == 0)
     prop->cost          = number_fuzzy( 10 )
                        * number_fuzzy( level ) * number_fuzzy( level );
    else prop->cost      = number_range( pPropIndex->cost - pPropIndex->cost/8, 
                                        pPropIndex->cost + pPropIndex->cost/8 );

    /*
     * Mess with prop properties.
     */
    switch ( prop->item_type )
    {
    default:
    if ( prop->item_type >= ITEM_MAX )
        wtf_logf( "Read_prop: dbkey %d bad type.", pPropIndex->dbkey );
	break;

    case ITEM_SCROLL:
	prop->value[0]	= number_fuzzy( prop->value[0] );
	break;

    case ITEM_WAND:
    case ITEM_STAFF:
	prop->value[0]	= number_fuzzy( prop->value[0] );
	prop->value[1]	= number_fuzzy( prop->value[1] );
	prop->value[2]	= prop->value[1];
	break;

    case ITEM_WEAPON:
    if ( prop->value[1] != 0 && prop->value[2] != 0 )
    {
         prop->value[1]   = number_fuzzy( prop->value[1] );
         prop->value[2]   = number_fuzzy( prop->value[2] );
    }
    else
    {
         prop->value[1]   = number_fuzzy( number_fuzzy( 1 * level / 4 + 2 ) );
         prop->value[2]   = number_fuzzy( number_fuzzy( 3 * level / 4 + 6 ) );
    }
    break;

    case ITEM_ARMOR:
    if ( prop->value[0] != 0 )
    {
         prop->value[0]   = number_fuzzy( prop->value[0] );
    }
    else    prop->value[0]   = number_fuzzy( level / 4 + 2 );

    if ( prop->value[2] != 0 )
    {
         prop->value[2]   = number_fuzzy( prop->value[2] );
         prop->value[1]   = prop->value[2];
    }
	break;

    case ITEM_POTION:
    case ITEM_PILL:
	prop->value[0]	= number_fuzzy( number_fuzzy( prop->value[0] ) );
	break;

    case ITEM_MONEY:
	prop->value[0]	= prop->cost;
	break;
    }

    /*
     *  Randomize color, size, and anything else we want!
     */
    prop_strings( prop );

    prop->next		= prop_list;
    prop_list		= prop;
    pPropIndex->count++;

    return prop;
}




/*
 * Translates actor virtual number to its actor index struct.
 * Hash table lookup.
 */
ACTOR_TEMPLATE *get_actor_template( int dbkey )
{
    ACTOR_TEMPLATE *pActorIndex;

    if ( !fBootDb && dbkey > top_dbkey_actor ) return NULL;

    for ( pActorIndex  = actor_template_hash[dbkey % MAX_KEY_HASH];
	  pActorIndex != NULL;
	  pActorIndex  = pActorIndex->next )
    {
	if ( pActorIndex->dbkey == dbkey )
	    return pActorIndex;
    }

    if ( fBootDb )
    {
	wtf_logf( "Get_actor_template: bad dbkey %d.", dbkey );
	exit( 1 );
    }

    return NULL;
}



/*
 * Translates actor virtual number to its prop index struct.
 * Hash table lookup.
 */
PROP_TEMPLATE *get_prop_template( int dbkey )
{
    PROP_TEMPLATE *pPropIndex;

    if ( !fBootDb && dbkey > top_dbkey_prop ) return NULL;

    for ( pPropIndex  = prop_template_hash[dbkey % MAX_KEY_HASH];
	  pPropIndex != NULL;
	  pPropIndex  = pPropIndex->next )
    {
	if ( pPropIndex->dbkey == dbkey )
	    return pPropIndex;
    }

    if ( fBootDb )
    {
	wtf_logf( "Get_prop_template: bad dbkey %d.", dbkey );
	exit( 1 );
    }

    return NULL;
}


/*
 * Translates actor virtual number to its spell index struct.
 * Hash table lookup.
 */
SPELL *find_spell( char *buf )
{
    SPELL *pSpell=NULL;

    HASHSEARCH(spell__hash, 
               !str_cmp(pSpell->name, buf), 
               pSpell );

    return pSpell;
}

/*
 * Translates actor virtual number to its spell index struct.
 * Hash table lookup.
 */
SPELL *get_spell_index( int dbkey )
{
    SPELL *pSpell;

    if ( (!fBootDb && dbkey > top_dbkey_spell) || (dbkey == 0) ) return NULL;

    for ( pSpell  = spell__hash[dbkey % MAX_KEY_HASH];
          pSpell != NULL;
          pSpell  = pSpell->next )
    {
        if ( pSpell->dbkey == dbkey )
            return pSpell;
    }

//    if ( fBootDb )
//    {
  //      wtf_logf( "Get_spell_index: bad dbkey %d.", dbkey );
 //       exit( 1 );
 //   }

    return NULL;
}



/*
 * Translates actor virtual number to its spell index struct.
 * Hash table lookup.
 */
HELP *get_help_index( int dbkey )
{
    HELP *pHelp;

    if ( (!fBootDb && dbkey > top_dbkey_help) || (dbkey == 0) ) return NULL;

    for ( pHelp  = help__hash[dbkey % MAX_KEY_HASH];
          pHelp != NULL;
          pHelp  = pHelp->next )
    {
        if ( pHelp->dbkey == dbkey )
            return pHelp;
    }
/*
    if ( fBootDb )
    {
        wtf_logf( "Get_help_index: bad dbkey %d.", dbkey );
        exit( 1 );
    }*/

    return NULL;
}

/*
 * Translates actor virtual number to its skill index struct.
 * Hash table lookup.
 */
SKILL *get_skill_index( int dbkey )
{
    SKILL *pSkill;

    if ( (!fBootDb && dbkey > top_dbkey_skill) || (dbkey == 0) ) return NULL;


    for ( pSkill  = skill__hash[dbkey % MAX_KEY_HASH];
          pSkill != NULL;
          pSkill  = pSkill->next )
    {
        if ( pSkill->dbkey == dbkey )
            return pSkill;
    }

    if ( fBootDb )
    {
/*        wtf_logf( "Get_skill_index: bad dbkey %d.", dbkey );  exit(1); */
    }
    return NULL;
}

/*
 * Translates actor virtual number to its scene index struct.
 * Hash table lookup.
 */
SCENE *get_scene( int dbkey )
{
    SCENE *pSceneIndex;

/*
    if ( !fBootDb && dbkey > top_dbkey_scene ) return NULL;
*/
    if ( (!fBootDb && dbkey > top_dbkey_scene) || (dbkey == 0) ) return NULL;


    for ( pSceneIndex  = scene_hash[dbkey % MAX_KEY_HASH];
          pSceneIndex != NULL;
          pSceneIndex  = pSceneIndex->next )
    {
        if ( pSceneIndex->dbkey == dbkey )
            return pSceneIndex;
    }

    if ( fBootDb )
    {
        wtf_logf( "Get_scene: bad dbkey %d.", dbkey );
        exit( 1 );
    }

    return NULL;
}



/* Translates virtual number to its script index struct.
 * Hash table lookup.
 */
SCRIPT *get_script_index( int dbkey )
{
    SCRIPT *script;

    if ( !fBootDb && dbkey > top_dbkey_script ) return NULL;

    for ( script  = script__hash[dbkey % MAX_KEY_HASH];
          script != NULL;
          script  = script->next )
    {
        if ( script->dbkey == dbkey )
            return script;
    }

    if ( fBootDb )
    {
//        wtf_logf( "Get_script_index: bad dbkey %d.", dbkey );
//        exit( 1 );
    }

    return NULL;
}






/*
 * Randomized strings.
 */
#define MAX_COLOR_LIST        20
#define MAX_FINE_COLOR_LIST   42
#define MAX_CLOTH_LIST         7
#define MAX_FUR_LIST          11
#define MAX_GEM_LIST          111
#define MAX_SYL               48

char *   const  color_list [MAX_COLOR_LIST] =
{
    "red",               "blue",
    "black",             "turquoise",
    "aquamarine",        "yellow",
    "indigo",            "purple",
    "magenta",           "lavender",
    "green",             "white",
    "ochre",             "orange",
    "violet",            "maroon",
    "mauve",             "brown",
    "tan",               "grey"         /* 20 */
};

char *   const  fine_color_list [MAX_FINE_COLOR_LIST] =
{
    "red",              "ruby-red",       "blue",             "sky-blue",
    "black",            "jet-black",      "turquoise",        "cyan",
    "aquamarine",       "blue-green",     "yellow",           "canary yellow",
    "indigo",           "dark blue",      "purple",           "chrome-purple",
    "magenta",          "steel-grey",     "lavender",         "blood-red",
    "green",            "olive drab",     "white",            "ghost-white",
    "ochre",            "burnt-ochre",    "orange",           "leaf-green",
    "violet",           "orange-sienna",  "maroon",           "milky-white",
    "mauve",            "faded brown",    "brown",            "sienna-brown",
    "tan",              "ivy-green",      "grey",             "slate-grey",
    "cobalt-blue",      "forest-green"
};


char *   const mcp_color_list [MAX_FINE_COLOR_LIST] =
{
    "red",              "ruby-red",       "blue",             "sky-blue",
    "black",            "jet-black",      "turquoise",        "cyan",
    "aquamarine",       "blue-green",     "yellow",           "canary yellow",
    "indigo",           "dark blue",      "purple",           "chrome-purple",
    "magenta",          "steel-grey",     "lavender",         "blood-red",
    "green",            "olive drab",     "white",            "ghost-white",
    "ochre",            "burnt-ochre",    "orange",           "leaf-green",
    "violet",           "orange-sienna",  "maroon",           "milky-white",
    "mauve",            "faded brown",    "brown",            "sienna-brown",
    "tan",              "ivy-green",      "grey",             "slate-grey",
    "cobalt-blue",      "forest-green"
};



char *   const  cloth_list [MAX_CLOTH_LIST] =
{
    "satin",
    "linen",
    "silk",
    "cloth",
    "wool",
    "cotton",
    "patchcloth"            /* 7 */
};

int      const  cloth_list_inc [MAX_CLOTH_LIST] =
{
    150,
    120,
    200,
    100,
    110,
    100,
    50
};

char *   const  fur_list [MAX_FUR_LIST] =
{
    "bear",
    "mink",
    "squirrel",
    "fox",
    "grey fox",
    "beaver",
    "ermine",
    "weasel",
    "ferret",
    "raccoon",
    "rabbit",                 /* 11 */
};

char *   const syl_short [MAX_SYL]   =
{
    "id",   "ad",  "al",  "no", 
    "ma",   "ol",  "da",  "nod", 
    "edig", "eda", "an",  "nos",
    "as",   "bri", "bi",  "bos",
    "jha",  "vod", "red", "vla",
    "cae",  "nal", "aod", "elad",
    "si",   "aer", "zon", "za",
    "li",   "ila", "son", "pod",
    "od",   "az",   "il", "stra",
    "ae",   "ao",  "io",  "ia",
    "y",    "i",   "a",   "u",
    "o",    "oo",  "u`",  "i`"
};

char *   const syl_long [MAX_SYL]   =
{
    "riad",  "kad",     "dred",   "drin",
    "vil",   "vian",    "son",    "zorr",
    "met",   "brod",    "dro",    "gadro",
    "vos",   "vad",     "dos",    "dros",
    "nol",   "ton",     "dral",   "dron",
    "van",   "driad",   "striad", "lin",
    "lad",   "iad",     "ipia",   "iana",
    "iasta", "iliadro", "ilion",  "ilia",
    "stras", "tas",     "ad",     "id",
    "no",    "nosti",   "osti",   "aerd",
    "po",    "mo",      "ti",     "idi",
    "hi",    "ur`",     "nal",    "ni"
};

char *   const fem_syl_short [MAX_SYL]   =
{
    "id",   "ad",  "al",  "no", 
    "ma",   "ol",  "da",  "nod", 
    "edig", "eda", "an",  "nos",
    "as",   "bri", "bi",  "bos",
    "jha",  "vod", "red", "vla",
    "cae",  "nal", "aod", "elad",
    "si",   "aer", "zon", "za",
    "li",   "ila", "son", "po",
    "od",   "az",   "il", "stra",
    "ae",   "ao",  "io",  "ia"
    "ai",   "chik", "o",  "i"
};

char *   const fem_syl_long [MAX_SYL]   =
{
    "riad",  "kad",     "dred",   "drin",
    "vil",   "vian",    "son",    "zorr",
    "met",   "brod",    "dro",    "gadro",
    "vos",   "vad",     "dos",    "dros",
    "nol",   "ton",     "dral",   "dron",
    "van",   "driad",   "striad", "lin",
    "lad",   "iad",     "ipia",   "iana",
    "iasta", "iliadro", "ilion",  "ilia",
    "stras", "tas",     "ad",     "id",
    "no",    "nosti",   "osti",   "aerd", 
    "di",    "to",      "mi",     "fa",
    "so",    "la",      "ti",     "re"
};

const   struct  gem_list_type  gem_list[MAX_GEM_LIST]   =
{ /*    Name                        Mana              Cost  Plural                 */
  { "blue diamond",                 1000, MA|MW,      5000, NULL    },
  { "black diamond",                 600, MA|ME,      3000, NULL    },
  { "red corundum",                  600, MF,         3000, NULL    },
  { "ruby",                          600, MF,         3000, "rubies" },
  { "auburn diamond",                400, MA|MF|ME,   2000, NULL    },
  { "clear diamond",                 300, MA,         1500, NULL    },
  { "crimson topaz",                 300, MF,         1500, "crimson topaz" },
  { "blue sapphire",                 280, MW,         1400, NULL    },
  { "clear sapphire",                280, MA,         1400, NULL    },
  { "yellow sapphire",               280, MF,         1400, NULL    },
  { "green sapphire",                280, ME,         1400, NULL    },
  { "grass-green emerald",           280, ME,         1400, NULL    },
  { "flawed clear diamond",          250, MA,         1250, NULL    },
  { "flawed pink topaz",             250, MF|MA,      1250, "pink topaz" },
  { "gold-flecked lapis lazuli",     220, ME,         1100, NULL    },
  { "fire opal",                     200, MF,         1000, NULL    },
  { "blue lapis lazuli",             200, ME|MW,      1000, NULL    },
  { "black ceylonite",               200, ME,         1000, NULL    },
  { "golden beryl",                  200, MF|MA,      1000, NULL    },
  { "pink beryl",                    200, MF,         1000, NULL    },
  { "colorless topaz",               200, MA,         1000, "colorless topaz" },
  { "yellow topaz",                  200, MF,         1000, "yellow topaz" },
  { "blue topaz",                    200, MW,         1000, "blue topaz" },
  { "blood spinel",                  180, MF|ME,       900, NULL    },
  { "star ruby",                     150, MF,          750, "star rubies" },
  { "pale aquamarine",               150, MW|MA,       750, NULL    },
  { "red spinel",                    150, MF,          750, NULL    },
  { "indigo spinel",                 150, MW,          750, NULL    },
  { "violet spinel",                 150, MW,          750, NULL    },
  { "black opal",                    150, MF,          750, NULL    },
  { "bright green demantoid garnet", 140, ME,          700, NULL    },
  { "fire sphene",                   140, MF,          700, NULL    },
  { "earthen andradite",             130, ME,          650, NULL    },
  { "gray-blue spinel",              120, ME|MW,       600, NULL    },
  { "sea-green spinel",              120, MW,          600, NULL    },
  { "purple-red almandine garnet",   120, MF|MA|MW,    600, NULL    },
  { "cymophane cats-eye",            120, MF|ME,       600, NULL    },
  { "brown jadeite",                 120, ME,          600, NULL    },
  { "green sphene",                  120, ME,          600, NULL    },
  { "orange jadeite",                120, MF,          600, NULL    },
  { "jade",                          120, ME,          600, NULL    },
  { "mauve jadeite",                 110, MF|MW,       550, NULL    },
  { "yellow sphene",                 110, MF,          550, NULL    },
  { "deep red pyrope",               110, MF,          550, NULL    },
  { "pale gray jadeite",             110, ME,          550, NULL    },
  { "fine deep blue turquoise",      100, MW,          500, NULL    },
  { "brown sphene",                  100, ME,          500, NULL    },
  { "white opal",                    100, MA,          500, NULL    },
  { "greenish-blue turquoise",       100, ME|MW,       500, NULL    },
  { "pale blue turquoise",           100, MW|MA,       500, NULL    },
  { "green turquoise",               100, ME,          500, NULL    },
  { "light green jadeite",           100, ME|MA,       500, NULL    },
  { "apple green jadeite",           100, ME,          500, NULL    },
  { "emerald green jadeite",         100, ME,          500, NULL    },
  { "carbonado crystal",             100, ME|MF,       500, "crystals of carbonado" },
  { "star sapphire",                 100, MF,          500, NULL    },
  { "green grossularite garnet",     100, ME,          500, NULL    },
  { "yellow-red hessonite garnet",   100, MF,          500, NULL    },
  { "orange-red spessartite garnet", 100, MF,          500, NULL    },
  { "pale green chrysoberyl",        100, ME,          500, NULL    },
  { "pale yellow chrysoberyl",       100, MF,          500, NULL    },
  { "green alexandrite",             100, ME,          500, NULL    },
  { "green-marked moss agate",       100, ME,          500, NULL    },
  { "black-marked moss agate",       100, MF|ME,       500, NULL    },
  { "red and white banded sardonyx",  90, MF|MA,       450, NULL    },
  { "brown tourmaline",               90, ME,          450, NULL    },
  { "black and white banded onyx",    80, MF|ME|MA,    400, NULL    },
  { "black iron tourmaline",          80, MF,          400, NULL    },
  { "green tourmaline",               80, ME,          400, NULL    },
  { "lithium tourmaline",             80, MW,          400, NULL    },
  { "lilac kunzite spodumene",        80, MW|MF,       400, NULL    },
  { "green hiddenite",                80, ME,          400, NULL    },
  { "mocha stone",                    80, ME,          400, NULL    },
  { "red tourmaline",                 70, MF,          350, NULL    },
  { "green zircon",                   70, ME,          350, NULL    },
  { "banded agate",                   70, MF|MW|ME|MA, 350, NULL    },
  { "peridot",                        60, ME|MA,       300, NULL    },
  { "olivine",                        60, ME|MA,       300, NULL    },
  { "blue zircon",                    60, MW,          300, NULL    },
  { "rust zircon",                    60, ME,          300, NULL    },
  { "clear zircon",                   60, MA,          300, NULL    },
  { "citrine",                        60, MF,          300, NULL    },
  { "amethyst",                       50, MF|MW,       500, NULL    },
  { "fine green chrysoprase",         50, ME,          250, NULL    },
  { "orthoclase feldspar",            50, ME|MW|MA|MF, 250, NULL    },
  { "milky-blue moonstone",           50, MW|MA,       250, NULL    },
  { "plagioclase feldspar",           50, ME|MW|MA|MF, 250, NULL    },
  { "reddish spangled sunstone",      50, MF,          250, NULL    },
  { "gray shimmering labradorite",    50, ME,          250, NULL    },
  { "cairngorm",                      40, ME|MW|MA|MF, 200, NULL    },
  { "rose quartz",                    40, MF,          200, NULL    },
  { "quartz cats-eye",                40, MF|ME,       200, NULL    },
  { "tigers-eye",                     40, ME|MW,       200, NULL    },
  { "green spangled aventurine",      40, MW|ME,       200, NULL    },
  { "reddish spangled aventurine",    40, MW|MF,       200, NULL    },
  { "white chalcedony",               40, MA,          200, NULL    },
  { "dark-green opaque plasma stone", 40, MW|ME,       200, NULL    },
  { "rock quartz",                    30, MW,          150, NULL    },
  { "brown jasper",                   30, ME,          150, NULL    },
  { "yellow jasper",                  30, MF|MA,       150, NULL    },
  { "red jasper",                     30, MF,          150, NULL    },
  { "green jasper",                   30, ME,          150, NULL    },
  { "orange-red carnelian",           30, MF,          150, NULL    },
  { "common opal",                    20, MW,          100, NULL    },
  { "green fluorspar",                20, ME,          100, NULL    },
  { "yellow fluorspar",               20, MF,          100, NULL    },
  { "pink fluorspar",                 20, MF,          100, NULL    },
  { "purple fluorspar",               20, MF|MW,       100, NULL    },
  { "colorless fluorspar",            20, MA,          100, NULL    },
  { "brown fluorspar",                20, ME,          100, NULL    },
  { "banded blue-john",               10, MA|MW,        50, NULL    },
};

void prop_strings( PROP *prop )
{
   char buf[MAX_STRING_LENGTH];
   char old[MAX_STRING_LENGTH];
   char *str;
   char *i;
   char *p;
   int pass;

   char c_str[WORK_STRING_LENGTH];
   char C_str[WORK_STRING_LENGTH];
   char l_str[WORK_STRING_LENGTH];
   char f_str[WORK_STRING_LENGTH];
   char g_str[WORK_STRING_LENGTH];
   int g_int, l_int;
   bool fPlural = FALSE;

   g_int = number_range( 0, MAX_GEM_LIST-1 );
   l_int = number_range( 0, MAX_CLOTH_LIST-1 );

   sprintf( c_str, "%s", color_list[number_range( 0, MAX_COLOR_LIST-1 )] );
   sprintf( C_str, "%s", fine_color_list[number_range( 0, MAX_FINE_COLOR_LIST-1 )] );
   sprintf( l_str, "%s", cloth_list[l_int] );
   sprintf( f_str, "%s", fur_list[number_range( 0, MAX_FUR_LIST-1 )] );
   sprintf( g_str, "%s", gem_list[g_int].name );

   for ( pass = 0; pass < 6; pass++ )
   {
      switch( pass )
      {
         default: str = STR( prop, short_descr );
                  wtf_logf( "prop_strings: bad pass %d", pass );   break;
          case 0: str = STR( prop, name );                    break;
          case 1: str = STR( prop, short_descr );             break;
          case 2: str = STR( prop, description );             break;
          case 3: str = STR( prop, short_descr_plural );      break;
          case 4: str = STR( prop, real_description  );       break;
          case 5: str = STR( prop, description_plural );      break;
      }

      sprintf( old, "%s", str );
      p = buf;

      if ( !strstr( old, "$" ) ) continue;

      while( *str != '\0' )
      {
         if( *str != '$' )
         {
            *p++ = *str++;
            continue;
         }
         ++str;
         switch( *str )
         {
            default : i = " "; break;
            case 'C': i = C_str; break;
            case 'c': i = c_str; break;
            case 'l': i = l_str; 
                      prop->cost = (prop->cost * cloth_list_inc[l_int])/100;
                      break;
            case 'f': i = f_str; break;
            case 'G': i = g_str; break;
            case 'g':
             {
                 i = g_str;
                 prop->value[0]  =  gem_list[g_int].mana_flags;
                 prop->value[1]  =  gem_list[g_int].mana_value;
                 prop->value[2]  =  gem_list[g_int].mana_value;
                 prop->cost      =  gem_list[g_int].cost;

                 if ( gem_list[g_int].plural != NULL )
                 {
                     free_string( prop->short_descr_plural );
                     prop->short_descr_plural = str_dup( gem_list[g_int].plural );
                     fPlural = TRUE;
                 }
             }
            break;
         }
         ++str;
         while( (*p = *i) != '\0' )
            ++p, ++i;
      }

      *p = '\0';

      if ( str_cmp( old, buf ) )
      {
      switch( pass )
      {
         default: break;
          case 0: if ( str_cmp( STR(prop,name), buf ) )
                  {
                      free_string( prop->name );
                      prop->name                = str_dup( buf );
                  }
                  break;
          case 1: if ( str_cmp( STR(prop,short_descr), buf ) )
                  {
                      free_string( prop->short_descr );
                      prop->short_descr         = str_dup( buf );
                  }
                  break;
          case 2: if ( str_cmp( STR(prop,description), buf ) )
                  {
                      free_string( prop->description );
                      prop->description         = str_dup( buf );
                  }
                  break;
          case 3: if (!fPlural && str_cmp( STR(prop,short_descr_plural), buf ) )
                  {
                      free_string( prop->short_descr_plural );
                      prop->short_descr_plural  = str_dup( buf );
                  }
                  break;
          case 4: if ( !str_cmp( STR(prop,real_description), buf ) )
                  {
                      free_string( prop->real_description );
                      prop->real_description    = str_dup( buf );
                  }
                  break;
          case 5: if ( !str_cmp( STR(prop,description_plural), buf ) )
                  {
                      free_string( prop->description_plural );
                      prop->description_plural  = str_dup( buf );
                  }
                  break;
      }
      }

   }
   
   p = one_argument( STR( prop, short_descr ), buf );
   if ( !str_cmp( buf, "a" ) && IS_VOWEL(*p) )
   {
       sprintf( buf, "an %s", p );
       free_string( prop->short_descr );
       prop->short_descr = str_dup( buf );
   }

   p = one_argument( STR( prop, short_descr_plural ), buf );
   if ( !str_cmp( buf, "a" ) && IS_VOWEL(*p) )
   {
       sprintf( buf, "an %s", p );
       free_string( prop->short_descr_plural );
       prop->short_descr_plural = str_dup( buf );
   }
   
   p = one_argument( STR( prop, description ), buf );
   if ( !str_cmp( buf, "a" ) && IS_VOWEL(*p) )
   {
       sprintf( buf, "An %s", p );
       free_string( prop->description );
       prop->description = str_dup( buf );
   }

   p = one_argument( STR( prop, real_description ), buf );
   if ( !str_cmp( buf, "a" ) && IS_VOWEL(*p) )
   {
       sprintf( buf, "An %s", p );
       free_string( prop->real_description );
       prop->description = str_dup( buf );
   }

   p = one_argument( STR( prop, description_plural ), buf );
   if ( !str_cmp( buf, "a" ) && IS_VOWEL(*p) )
   {
       sprintf( buf, "An %s", p );
       free_string( prop->description_plural );
       prop->description = str_dup( buf );
   }
   
   return;
}


void actor_strings( PLAYER *actor )
{
   char buf[MAX_STRING_LENGTH];
   char old[MAX_STRING_LENGTH];
   char *str;
   char *i;
   char *p;
   int pass;

   char c_str[WORK_STRING_LENGTH];
   char C_str[WORK_STRING_LENGTH];
   char l_str[WORK_STRING_LENGTH];
   char f_str[WORK_STRING_LENGTH];
   char g_str[WORK_STRING_LENGTH];
   char n_str[WORK_STRING_LENGTH];
   char N_str[WORK_STRING_LENGTH];
   char m_str[WORK_STRING_LENGTH];
   char M_str[WORK_STRING_LENGTH];
//   char T_str[WORK_STRING_LENGTH];   /* title string nyi  */

   sprintf(c_str, "%s", color_list[number_range(0,MAX_COLOR_LIST-1)]);
   sprintf(C_str, "%s", fine_color_list[number_range(0,MAX_FINE_COLOR_LIST-1)]);
   sprintf(l_str, "%s", cloth_list[number_range(0,MAX_CLOTH_LIST-1)]);
   sprintf(f_str, "%s", fur_list[number_range(0,MAX_FUR_LIST-1)]);
   sprintf(g_str, "%s", gem_list[number_range(0,MAX_GEM_LIST-1)].name);
   sprintf(n_str, "%s%s", capitalize(syl_short[number_range(0,MAX_SYL-1)]),
                          syl_short[number_range(0,MAX_SYL-1)] ); 
   sprintf(N_str, "%s%s%s", capitalize(syl_short[number_range(0,MAX_SYL-1)]),
                        syl_short[number_range(0,MAX_SYL-1)],
                        syl_long[number_range(0,MAX_SYL-1)] );
   sprintf(m_str, "%s",   syl_short[number_range(0,MAX_SYL-1)] );
   sprintf(M_str, "%s",   syl_long[number_range(0,MAX_SYL-1)] );

   for ( pass = 0; pass < 4; pass++ )
   {
      switch( pass )
      {
         default: str = STR( actor, short_descr );
                  wtf_logf( "Mob_strings: bad pass %d", pass );   break;
          case 0: str = STR( actor, name );                    break;
          case 1: str = STR( actor, short_descr );             break;
          case 2: str = STR( actor, long_descr );              break;
          case 3: str = STR( actor, description );             break;
      }

      sprintf( old, "%s", str );
      p = buf;

      if ( !strstr( old, "$" ) ) continue;

      while( *str != '\0' )
      {
         if( *str != '$' )
         {
            *p++ = *str++;
            continue;
         }
         ++str;
         switch( *str )
         {
            default : i = " "; break;
            case 'C': i = C_str; break;
            case 'c': i = c_str; break;
            case 'l': i = l_str; break;
            case 'f': i = f_str; break;
            case 'G': i = g_str; break;
            case 'n': { 
                         char *z, r, q = '\0';
                         i = n_str;
                         z = n_str;
                         r = q;
                         while ( ( *(++z) ) != '\0' )  q = *z; 

                         if ( q == 'a' ) actor->sex = SEX_FEMALE;
                         else actor->sex = (IS_VOWEL(q) 
                                       && number_range(0,4) == 3) 
                                      ? SEX_FEMALE : SEX_MALE;
                      } 
                     break;
            case 'N': i = N_str; break;
            case 'm': i = m_str; break;
            case 'M': i = M_str; break;
            break;
         }
         ++str;
         while( (*p = *i) != '\0' )
            ++p, ++i;
      }

      *p = '\0';

      if ( str_cmp( old, buf ) )
      {
      switch( pass )
      {
         default: break;
          case 0: if ( str_cmp( STR(actor,name), buf ) )
                  {
                      free_string( actor->name );
                      actor->name                = str_dup( buf );
                  }
                  break;
          case 1: if ( str_cmp( STR(actor,short_descr), buf ) )
                  {
                      free_string( actor->short_descr );
                      actor->short_descr         = str_dup( buf );
                  }
                  break;
          case 2: if ( str_cmp( STR(actor,long_descr), buf ) )
                  {
                      free_string( actor->long_descr );
                      actor->long_descr          = str_dup( buf );
                  }
                  break;
          case 3: if ( str_cmp( STR(actor,description), buf ) )
                  {
                      free_string( actor->description );
                      actor->description         = str_dup( buf ); break;
                  }
      }
      }

   }
   
   p = one_argument( STR( actor, short_descr ), buf );
   if ( !str_cmp( buf, "a" ) && IS_VOWEL(*p) )
   {
       sprintf( buf, "an %s", p );
       free_string( actor->short_descr );
       actor->short_descr = str_dup( buf );
   }

   p = one_argument( STR( actor, long_descr ), buf );
   if ( !str_cmp( buf, "a" ) && IS_VOWEL(*p) )
   {
       sprintf( buf, "An %s", p );
       free_string( actor->long_descr );
       actor->long_descr = str_dup( buf );
   }

   return;
}




/*
 * Writes a string to the log.
 */
void log_string( const char *str )
{
    char *strtime;
    char buf[MAX_STRING_LENGTH];

    strtime                    = ctime( &current_time );
    strtime[strlen(strtime)-1] = '\0';
    fprintf( stderr, "%s :: %s\n", strtime, str );

    sprintf( buf, "Notify> LOG: %s", str );
    NOTIFY( buf, LOG_LEVEL, WIZ_NOTIFY_LOG );
    return;
}




/*
 * Dump (ignore) a section that ends in #0
 */
void dump_section_0( FILE *fp )
{
    for ( ; ; )
    {
	char letter;
    int num;

    if ( ( letter = fread_letter( fp ) ) == '#' )
     if ( ( num = fread_number( fp ) ) == 0 )
        break;

    fread_to_eol( fp );
    continue;
    }

    return;
}

/*
 * Dump (ignore) a section that ends in 'S'
 */
void dump_section_s( FILE *fp )
{
    for ( ; ; )
    {
	char letter;

	if ( ( letter = fread_letter( fp ) ) == 'S' )
	    break;

    fread_to_eol( fp );
    continue;
    }

    return;
}


/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain( void )
{
    return;
}






#if defined(NEVER)
 volatile sig_atomic_t fatal_error_in_progress = 0;

 void
     fatal_error_signal (int sig)
     {
       /* Since this handler is established for more than one kind of signal, 
          it might still get invoked recursively by delivery of some other kind
          of signal.  Use a static variable to keep track of that. */

       if (fatal_error_in_progress)
         raise (sig);
       fatal_error_in_progress = 1;
     
       /* Now do the clean up actions:
          - reset terminal modes
          - kill child processes
          - remove lock files */       
     
       /* Now reraise the signal.  We reactivate the signal's
          default handling, which is to terminate the process.
          We could just call exit or abort,
          but reraising the signal sets the return status
          from the process correctly. */
       signal (sig, SIG_DFL);
       raise (sig);
     }
#endif

#define COPYOVER_FILE "recovery.nfo"

/* Recover from a copyover - load players */
void copyover_recover ( void )
{
        CONNECTION *d;
        FILE *fp;
        char name [100];
        char host[MSL];
        int desc;
        bool fOld;

        wtf_logf ("Recovering Connected Players");

        fp = fopen (COPYOVER_FILE, "r");

        if (!fp) /* there are some descriptors open which will hang forever then ? */
        {
                perror ("copyover_recover:fopen");
                wtf_logf ("Copyover: File not found. Exitting.\n\r");
                exit (1);
        }

        unlink (COPYOVER_FILE); /* In case something crashes - doesn't prevent reading  */

        for (;;)
        {
                fscanf (fp, "%d %s %s\n", &desc, name, host);

                if (desc == -1)
                        break;

                /* Write something, and check if it goes error-free */
                if (!write_to_connection (desc, "\n\rRestoring...\n\r",0))
                {
                        close (desc); /* nope */
                        continue;
                }

                d = new_connection();
                d->connection = desc;

                d->host = str_dup (host);
                d->next = connection_list;
                connection_list = d;
                d->connected = NET_HOTBOOT_RECOVER; /* -15, so close_socket frees the char */

                /* Now, find the pfile */

                fOld = load_actor_prop (d, name);

                if (!fOld) /* Player file not found?! */
                {
                        write_to_connection (desc, "\n\rSomehow, your character file was lost. Login as new..\n\r",0 );
                        close_socket(d);
                }
                else /* ok! */
                {
                        write_to_connection (desc, "\n\rRecovery complete.\n\r",0);

                        /* Just In Case */
                        if (!d->character->in_scene)
                                d->character->in_scene = get_scene (1);

                        /* Insert in the char_list */
                        d->character->next = actor_list;
                        actor_list = d->character;

                        actor_to_scene (d->character, d->character->in_scene);
                        cmd_look (d->character, "");
                        d->connected = NET_PLAYING;
                }

        }

        fclose (fp);
}

/*
 * Placed here because there is a conflict with another library.
 * Called from net.c during the signal handling.
 */

void go_mud_go( int p, int c ) 
         {
        char buf[50];
        char buf2[50];
        //if ( fCopyOverride ) return;
        sprintf (buf, "%d", p);
        sprintf (buf2, "%d", c);
        wtf_logf( "Copyover: %s, %s\n\r", buf, buf2 );
        execl (EXE_FILE, "nimud", buf, buf2, (char *) NULL);
         }

