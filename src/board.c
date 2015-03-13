/******************************************************************************
 *   ___  ___  ___  __    __                                                  *
 *  |   ||   ||   ||  |\/|  | 5      NiMUD is a software currently under      *
 *   |  \ | |  | |  | |\/| |         development.  It is based primarily on   *
 *   | |\\| |  | |  | |  | |         the discontinued package, NiMUD 2-4.     *
 *   | | \  |  | |  | |  | |         NiMUD is being written and developed     *
 *  |___||___||___||__|  |__|        By Locke and Surreality as a new,        *
 *   NAMELESS INCARNATE *MUD*        frequently updated package similar to    *
 *        S O F T W A R E            the original NiMUD.                      *
 *                                                                            *
 *  Just look for the Iron Maiden skull wherever NiMUD products are found.    *
 *                                                                            *
 *  Much time and thought has gone into this software and you are             *
 *  benefitting.  We hope that you share your changes too.  What goes         *
 *  around, comes around.                                                     *
 ******************************************************************************
 * Locke's   __ -based on merc v5.0-____        NIM Server Software           *
 * ___ ___  (__)__    __ __   __ ___| G| v4.0   Version 4.0 GOLD EDITION      *
 * |  /   \  __|  \__/  |  | |  |     O|        documentation release         *
 * |       ||  |        |  \_|  | ()  L|        Hallow's Eve 1999             *
 * |    |  ||  |  |__|  |       |     D|                                      *
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

#include "net.h"
#include "nimud.h"
#include "board.h"
 
const struct board board_table [MAX_BOARD] =
{
 /* Name of Board           Filename        Write Level     Read Level      */
  { "(OOC) Immortal",       "board1.txt",   LEVEL_IMMORTAL, LEVEL_IMMORTAL   },
  { "(OOC) Announcements",  "board2.txt",   LEVEL_IMMORTAL, LEVEL_MORTAL     },
  { "(OOC) Public",         "board3.txt",   LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "(OOC) Bugs",           "board4.txt",   LEVEL_MORTAL,   LEVEL_IMMORTAL   },
  { "(OOC) Building",       "board5.txt",   LEVEL_HERO,     LEVEL_HERO       },
  { "Rules",                "board6.txt",   LEVEL_IMMORTAL, LEVEL_MORTAL     },
  { "(OOC) Roleplay",       "board7.txt",   MAX_LEVEL,      LEVEL_MORTAL },
  { "(OOC) Judges",         "board8.txt",   LEVEL_ANGEL,    LEVEL_MORTAL     },
  { "Applications",         "board9.txt",   0,              LEVEL_IMMORTAL   },
  { "(OOC) Roleplay",       "board10.txt",  LEVEL_MORTAL,   LEVEL_MORTAL     },

  { "IC bulletin",          "bboard1.txt",  LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "IC bulletin",          "bboard2.txt",  LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "IC bulletin",          "bboard3.txt",  LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "IC bulletin",          "bboard4.txt",  LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "IC bulletin",          "bboard5.txt",  LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "IC bulletin",          "bboard6.txt",  LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "IC bulletin",          "bboard7.txt",  LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "IC bulletin",          "bboard8.txt",  LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "IC bulletin",          "bboard9.txt",  LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "IC bulletin",          "bboard10.txt", LEVEL_MORTAL,   LEVEL_MORTAL     },

  { "Clan 1",               "cboard1.txt",  LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "Clan 2",               "cboard2.txt",  LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "Clan 3",               "cboard3.txt",  LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "Clan 4",               "cboard4.txt",  LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "Clan 5",               "cboard5.txt",  LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "Clan 6",               "cboard6.txt",  LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "Clan 7",               "cboard7.txt",  LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "Clan 8",               "cboard8.txt",  LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "Clan 9",               "cboard9.txt",  LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "Clan 10",              "cboard10.txt", LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "Clan 11",              "cboard11.txt", LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "Clan 12",              "cboard12.txt", LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "Clan 13",              "cboard13.txt", LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "Clan 14",              "cboard14.txt", LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "Clan 15",              "cboard15.txt", LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "Clan 16",              "cboard16.txt", LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "Clan 17",              "cboard17.txt", LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "Clan 18",              "cboard18.txt", LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "Clan 19",              "cboard19.txt", LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "Clan 20",              "cboard20.txt", LEVEL_MORTAL,   LEVEL_MORTAL     },

  { "Bulletins (OOC)",      "bboard1.txt",  LEVEL_MORTAL,   LEVEL_MORTAL     },
  { "Housing (OOC)",        "bboard2.txt",  LEVEL_MORTAL,   LEVEL_HERO       }

};



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

extern char                    strzone[MAX_INPUT_LENGTH]; 
extern FILE *                  fpZone;
extern NOTE *             note_free;

/*
 * Careful when creating a verbose bulletin list, as each one will end
 * up with only a few words on it.  Now, the Clan Boards are accessible
 * by only a few people out of the norm, so it makes sense to make as
 * many as you are going to need.  Remember the number and modify the
 * table if you want to change the name of the board according to the
 * clan name.  Just remember that you wont a whole bunch of the other
 * ones, so keep it to a minimum.
 */
 
struct note     *note_list[MAX_BOARD];
struct note     note_zero;
 
/*
 * this structure is as good as any, might as well use it...
struct  note
{
    NOTE * next;
    char *      sender;
    char *      date;
    char *      to_list;
    char *      subject;
    char *      text;
    time_t      date_stamp;
  };
 */
 
 
bool is_note_to  args( ( PLAYER *ch, int board, NOTE *pnote ) );

#define CAN_WRITE(ch, boardnum)  ( get_trust( ch ) >=                    \
                                   board_table[boardnum].writelevel )
 
#define list      note_list[b]
#define filename  board_table[b].filename

void fread_board( int b )
{
    FILE *fp;
    NOTE *pnote;
    NOTE *pnotelast;
    char *word;
    bool fMatch;
    int count = 0;
    char buf[MAX_STRING_LENGTH];
 
    sprintf( buf, "%s%s", BOARD_PATH, filename );
    if ( ( fp = fopen( buf, "r" ) ) == NULL )
      {
        list = NULL;
        return;
      }
     
    pnote = alloc_mem( sizeof(*pnote) );    /* create a new note      */
    pnote->sender   = str_dup( "" );        /* defaults */
    pnote->date     = str_dup( "" );
    pnote->to_list  = str_dup( "" );
    pnote->subject  = str_dup( "" );
    pnote->text     = str_dup( "" );
    pnote->next     = NULL;                 /* clip it off            */
    list = pnote;                           /* put it on the fore     */
    pnotelast = NULL;                       /* first != last          */
        
    for ( ; ; )
      {
        fMatch = FALSE;                     /* lame screwups */
        
        word = feof(fp) ? "#END" : fread_word( fp );
 
        switch ( UPPER(word[0]) )
        {
           case 'F':
             KEY( "From:",    pnote->sender,     fread_string( fp ) );
             KEY( "Flags",    pnote->note_flags, fread_number( fp ) );
            break;
           case 'S':                                   /* fill */
             KEY( "Stamp",    pnote->date_stamp, fread_number( fp ) );
             KEY( "Subject:", pnote->subject,    fread_string( fp ) );
             KEY( "Sent:",    pnote->date,       fread_string( fp ) );
            break;
           case 'T':
             KEY( "To:",      pnote->to_list,    fread_string( fp ) );
             KEY( "Text:",    pnote->text,       fread_string( fp ) );
            break;
           case 'E':  
             if ( !str_cmp( word, "End" ) )
             {                                        /* not first, second */
                 if ( pnotelast != NULL ) 
                 pnotelast->next = pnote;             /* add to list       */
                 pnotelast = pnote;                   /* prepare for next  */
                 pnote = alloc_mem( sizeof(*pnote) ); /* create the next   */
                 pnote->sender   = str_dup( "" );     /* defaults */
                 pnote->date     = str_dup( "" );
                 pnote->to_list  = str_dup( "" );
                 pnote->subject  = str_dup( "" );
                 pnote->text     = str_dup( "" );
                 pnote->next     = NULL;              /* clip it off       */
                 fMatch = TRUE;
                 count++;
             }
            break;
           case '#':
             if ( !str_cmp( word, "#END" ) )      
             {
                 if (list == pnote) list = NULL;     /* no notes = no list */
                 pnote->next = note_free;            /* add to free list   */
                 note_free   = pnote;
                 if (pnotelast != NULL) 
                 pnotelast->next = NULL;             /* clip list          */
                 fclose( fp );
                 return;
             }
            break;
        }
        
        if ( !fMatch )
        {
            strcpy( strzone, filename );
            sprintf( log_buf, "load_notes: bad keyword '%s'", word );
            wtf_logf( log_buf, 0 );                                       
            fclose( fp );
            return;
        }
    }
}

#undef list 
#undef filename
 
void load_boards( void )
{
   int b;
   
   for ( b = 0; b < MAX_BOARD; b++ ) fread_board( b );
   return;
}
 

void note_remove( PLAYER *ch, NOTE *pnote, int b )
{
    char to_new[MAX_INPUT_LENGTH];
    char to_one[MAX_INPUT_LENGTH];
    NOTE *prev;
    char *to_list;
 
    /*
     * Build a new to_list.
     * Strip out this recipient.
     */
    to_new[0]   = '\0';
    to_list     = pnote->to_list;
    while ( *to_list != '\0' )
      {
        to_list = one_argument( to_list, to_one );
    if ( to_one[0] != '\0' && str_cmp( STR(ch,name), to_one ) )
      {
            strcat( to_new, " " );
            strcat( to_new, to_one );
      }
      }
 
    /*
     * Just a simple recipient removal?
     */
    if ( !is_note_to( ch, b, pnote ) )
      {
    if ( str_cmp( STR(ch,name), pnote->sender ) && to_new[0] != '\0' )
      {
        free_string( pnote->to_list );
        pnote->to_list = str_dup( to_new + 1 );
        return;
      }
  }
 
    /*
     * Remove note from linked list.
     */
    if ( pnote == note_list[b] )
      {
        note_list[b] = pnote->next;
      }
    else
      {
        for ( prev = note_list[b]; prev != NULL; prev = prev->next )
          {
            if ( prev->next == pnote )
                break;
          }
 
        if ( prev == NULL )
          {
            wtf_logf( "Note_remove: pnote not found.", 0 );
            return;
          }
 
        prev->next = pnote->next;
      }
 
    free_string( pnote->text    );
    free_string( pnote->subject );
    free_string( pnote->to_list );
    free_string( pnote->date    );
    free_string( pnote->sender  );
    pnote->next = note_free;
    note_free   = pnote; 
    return;
  }

 
void save_board( int b )
{
    char buf[MAX_STRING_LENGTH];
    NOTE *pnote;
    FILE *fp;
    
    fclose( fpReserve );
    sprintf( buf, "%s%s", BOARD_PATH, board_table[b].filename );
    if ( ( fp = fopen( buf, "w" ) ) == NULL )
      {
        perror( buf );
        return;
      }
     
    for ( pnote = note_list[b]; pnote != NULL; pnote = pnote->next )
      {
        smash_tilde( pnote->text );
 
        fprintf( fp, "From: %s~\n",    fix_string( pnote->sender ) );
        fprintf( fp, "To:   %s~\n",    fix_string( pnote->to_list ) );
        fprintf( fp, "Sent: %s~\n",    pnote->date );
        fprintf( fp, "Stamp %ld\n",     pnote->date_stamp );
        fprintf( fp, "Flags %d\n",     pnote->note_flags );
        fprintf( fp, "Subject: %s~\n", fix_string( pnote->subject ) );
        fprintf( fp, "Text:\n%s~\n",   fix_string( pnote->text ) );
        fprintf( fp, "End\n\n" );
      }
    fprintf( fp, "\n#END\n" );
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
  }
 
 
 
bool is_note_to( PLAYER *ch, int board, NOTE *pnote )
{
    if ( !str_cmp( STR(ch,name), pnote->sender ) )
        return TRUE;
 
    if ( get_trust( ch ) < board_table[board].readlevel )
        return FALSE;
 
    if ( is_name( "all", pnote->to_list ) )
        return TRUE;
 
    if ( IS_HERO(ch) && is_name( "immortal", pnote->to_list ) )
        return TRUE;
 
    if ( is_name( STR(ch,name), pnote->to_list ) )
        return TRUE;
        
    if ( get_trust( ch ) >= LEVEL_DEMIGOD )
        return TRUE;
 
    return FALSE;
}
 
 
void note_attach( PLAYER *ch )
{
    NOTE *pnote;
 
    if ( ch->pnote != NULL )
        return;
 
    if ( note_free == NULL )
      {
        pnote     = alloc_perm( sizeof(*ch->pnote) );
        top_note++;
      }
    else
      {
        pnote     = note_free;
        note_free = note_free->next;
      }
 
    *pnote              = note_zero;
    pnote->next         = NULL;
    pnote->sender       = str_dup( STR(ch,name) );
    pnote->date         = str_dup( "" );
    pnote->to_list      = str_dup( "" );
    pnote->subject      = str_dup( "" );
    pnote->text         = str_dup( "" );
    ch->pnote           = pnote;
    return;
}
 
 

/*
 * Syntax:  note list
 *          note read [#]
 *          note remove [#]
 *          note delete [#]
 *          note to [person]
 *          note to all
 *          note to immort
 *          note subject [text]
 *          note write
 *          note enter
 *          note post
 *          note clear
 *          note show
 *          note anonymous
 */
void cmd_note( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    NOTE *pnote;
    int dbkey;
    int anum;
    PROP *prop;
    int bnum;
    
    if ( NPC(ch) )
        return;
 
    buf1[0] = '\0';
 
    argument = one_argument( argument, arg );
    smash_tilde( argument );
 
    if ( arg[0] == '\0' )
    {
        to_actor( "Syntax: note list            \n\r", ch );
        to_actor( "        note <num>           \n\r", ch );
        to_actor( "        note remove <num>    \n\r", ch );
        to_actor( "        note delete <num>    \n\r\n\r", ch );
        to_actor( "Syntax: note to <person(s)|all|immort>\n\r", ch );
        to_actor( "        note subject <text>  \n\r", ch );
        to_actor( "        note write           \n\r", ch );
        to_actor( "        note post            \n\r", ch );
        to_actor( "        note show            \n\r", ch );
        to_actor( "        note clear           \n\r", ch );
        to_actor( "        note anonymous       \n\r", ch );
        if ( IS_IMMORTAL(ch) )
        to_actor( "        note digest - make a digest of notes\n\r", ch );
        return;
    }
   
    for ( prop = ch->in_scene->contents; prop != NULL; prop = prop->next_content )
    {
        if ( prop->item_type == ITEM_BOARD
          && prop->value[0] >= 0
          && prop->value[0] < MAX_BOARD )
           break;
    }
    
    if ( prop == NULL ) 
    {
        to_actor( "There is no board here.\n\r", ch );
        return;
    }
    bnum = prop->value[0];

    if ( !str_cmp( arg, "remove" ) )
    {
        if ( !is_number( argument ) )
        {
            to_actor( "Note remove which number?\n\r", ch );
            return;
        }
 
        anum = atoi( argument );
        dbkey = 1;
        for ( pnote = note_list[bnum]; pnote != NULL; pnote = pnote->next )
        {
        if ( is_note_to( ch, bnum, pnote ) && dbkey++ == anum )
        {
                note_remove( ch, pnote, bnum ); 
                save_board( bnum );
                act( "$n tears a note off $p and crumples it up.", ch, prop, NULL, TO_SCENE );
                to_actor( "Note removed.\n\r", ch );
                return;
        }
        }
 
        to_actor( "No such note.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "list" ) )
    {
        dbkey = 1;
        sprintf( buf, "The following notes are on the %s board:\n\r", 
                      board_table[bnum].name );
        to_actor( buf, ch );
        for ( pnote = note_list[bnum]; pnote != NULL; pnote = pnote->next )
          {
            if ( is_note_to( ch, bnum, pnote ) )
              {
        sprintf( buf, "[%-3d %18s] %s to %s: %s\n\r",
            dbkey,
            pnote->date,
             IS_SET(pnote->note_flags, NOTE_ANONYMOUS)
                 ? "Anonymous" : pnote->sender,
            pnote->to_list, pnote->subject );
                strcat( buf1, buf );
                dbkey++;
    }
    }
        if ( buf1[0] == '\0' ) sprintf( buf1, "None.\n\r" );
        to_actor( buf1, ch );
        act( "$n looks over the notes on $p.", ch, prop, NULL, TO_SCENE );
        return;
    }

    if ( is_number( arg ) || is_number( argument ) )
    {
        bool fAll;
 
        if ( is_number( argument ) )
        {
            fAll = FALSE;
            anum = atoi( argument );
        }
        else if ( is_number( arg ) )
        {
            fAll = FALSE;
            anum = atoi( arg );
        }
        else if ( !str_cmp( argument, "all" ) )
        {
            fAll = TRUE;
            anum = 0;
        }
        else if ( argument[0] == '\0' || !str_prefix( argument, "next" ) )
          /* read next unread note */
        {
            dbkey = 1;
            for ( pnote = note_list[bnum]; pnote != NULL; pnote = pnote->next )
            {
                if ( is_note_to( ch, bnum, pnote )
                    && str_cmp( STR(ch,name), pnote->sender )
                    && PC(ch,last_note) < pnote->date_stamp )
                  {
                    sprintf( buf, "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r",
                            dbkey,
                            IS_SET(pnote->note_flags, NOTE_ANONYMOUS)
                             && !IS_IMMORTAL(ch)
                                 ? "Anonymous" : pnote->sender,
                            pnote->subject,
                            pnote->date,
                            pnote->to_list );
                    strcat( buf1, buf );
                    strcat( buf1, pnote->text );

                    PC(ch,last_note) = UMAX( PC(ch,last_note ), pnote->date_stamp );
                    to_actor( buf1, ch );
                    act( "$n studies a note on $p for a while.", ch, prop, NULL, TO_SCENE );
                    return;
                }
                else dbkey++;
            }
            to_actor( "You have no unread notes.\n\r", ch );
            return;
        }
        else
        {
            to_actor( "Note read which number?\n\r", ch );
            return;
        }
 
        dbkey = 1;
        for ( pnote = note_list[bnum]; pnote != NULL; pnote = pnote->next )
        {
            if ( is_note_to( ch, bnum, pnote ) && ( dbkey++ == anum || fAll ) )
            {
                sprintf( buf, "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r",
                      dbkey - 1,
                      IS_SET(pnote->note_flags, NOTE_ANONYMOUS)
                         && !IS_IMMORTAL(ch)  ? "Anonymous"
                                              : pnote->sender,
                      pnote->subject,
                      pnote->date,
                      pnote->to_list      );
                strcat( buf1, buf );
                strcat( buf1, pnote->text );
                page_to_actor( buf1, ch );
                act( "$n studies a note on $p for a while.", ch, prop, NULL, TO_SCENE );
                PC(ch,last_note) = UMAX( PC(ch,last_note), pnote->date_stamp );
                return;
            }
        }
 
        to_actor( "No such note.\n\r", ch );
        return;
    }
 
    if ( !str_cmp( arg, "write" ) || !str_cmp( arg, "enter" ) )
    {
    note_attach( ch );
    string_append( ch, &ch->pnote->text );
    return;
    }

    if ( !str_cmp( arg, "digest" ) && IS_IMMORTAL(ch) )
    {
        char buf3[MAX_STRING_LENGTH];
        char *strtime;
        buf1[0] = '\0';

        for ( pnote = note_list[bnum]; pnote != NULL; pnote = pnote->next )
          {
            if ( is_name( "all", pnote->to_list ) && str_cmp( pnote->subject, "[Digest]" ) )
            {
                buf3[0] = '\0';

        strcat( buf3, "================================================\n\r" );
        sprintf( buf, "From: %s\n\r",    fix_string( pnote->sender ) );
        strcat( buf3, buf );
        sprintf( buf, "To:   %s\n\r",    fix_string( pnote->to_list ) );
        strcat( buf3, buf );
        sprintf( buf, "Sent: %s\n\r",    pnote->date );
        strcat( buf3, buf );
        sprintf( buf, "Subject: %s\n\r", fix_string( pnote->subject ) );
        strcat( buf3, buf );
        sprintf( buf, "Text:\n\r%s\n\r",   fix_string( pnote->text ) );
        strcat( buf3, buf );
        strcat( buf3, "------------------------------------------------\n\r" );

               if ( strlen( buf3 ) + strlen( buf1 ) >= MAX_STRING_LENGTH )
               break;

               strcat( buf1, buf3 );
               save_board( bnum );
            }
          }

        for ( pnote = note_list[bnum]; pnote != NULL; )
          {
            NOTE *pnote_next;
            pnote_next = pnote->next;

            if ( is_name( "all", pnote->to_list ) && str_cmp( pnote->subject, "[Digest]" ) )
            {
               note_remove( ch, pnote, bnum );
            }
            pnote = pnote_next;
          }

        if (buf1[0] == '\0')
        {
            to_actor( "No notes to digest.\n\r", ch );
            return;
        }

        note_attach( ch );
        free_string( ch->pnote->to_list );
        SET_BIT(ch->pnote->note_flags, NOTE_ANONYMOUS);
        ch->pnote->to_list = str_dup( "all" );
        ch->pnote->subject = str_dup( "[Digest]" );
        ch->pnote->text = str_dup( buf1 );
        ch->pnote->next                 = NULL;
        strtime                         = ctime( &current_time );
        strtime[strlen(strtime)-1]      = '\0';
        ch->pnote->date                 = str_dup( strtime );
        ch->pnote->date_stamp           = current_time;
 
        ch->pnote->next   = note_list[bnum];
        note_list[bnum]   = ch->pnote;

        ch->pnote       = NULL;
        save_board( bnum );
        to_actor( "Digested.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "anonymous" ) )
    {
    note_attach( ch );
    TOGGLE_BIT(ch->pnote->note_flags, NOTE_ANONYMOUS);
    return;
    }
 
 
    if ( !str_cmp( arg, "subject" ) )
    {
        note_attach( ch );
        free_string( ch->pnote->subject );
        ch->pnote->subject = str_dup( argument );
        if ( ch->desc->connected <= NET_PLAYING ) to_actor( "Ok.\n\r", ch );
        return;
    }
 
    if ( !str_cmp( arg, "to" ) )
    {
        note_attach( ch );
        free_string( ch->pnote->to_list );
        ch->pnote->to_list = string_proper( str_dup(argument) );
        if ( ch->desc->connected <= NET_PLAYING ) to_actor( "Ok.\n\r", ch );
        return;
    }
 
    if ( !str_cmp( arg, "clear" ) )
    {
        if ( ch->pnote != NULL )
        {
            free_string( ch->pnote->text );
            free_string( ch->pnote->subject );
            free_string( ch->pnote->to_list );
            free_string( ch->pnote->date );
            free_string( ch->pnote->sender );
            ch->pnote->next     = note_free;
            note_free           = ch->pnote;
            ch->pnote           = NULL;
        }
 
        to_actor( "Ok.\n\r", ch );
        return;
    }
 
    if ( !str_cmp( arg, "show" ) )
    {
        if ( ch->pnote == NULL )
        {
            to_actor( "You have no note in progress.\n\r", ch );
            return;
        }
 
        sprintf( buf, "%s: %s\n\rTo: %s\n\r",
            IS_SET(ch->pnote->note_flags, NOTE_ANONYMOUS) ? "ANONYMOUS"
                                                          : ch->pnote->sender,
            ch->pnote->subject,
            ch->pnote->to_list );
        to_actor( buf, ch );
        to_actor( ch->pnote->text, ch );
        return;
    }
  
    if ( IS_IMMORTAL(ch) && !str_prefix( arg, "email" ) ) {
        if ( ch->pnote == NULL )
        {
            to_actor( "You have no note in progress.\n\r", ch );
            return;
        }
 
        if ( !str_cmp( ch->pnote->to_list, "" ) )
        {
            to_actor(
              "You need to provide an email recipient (who@domain).\n\r",
                         ch );
            return;
        }
 
        if ( !str_cmp( ch->pnote->subject, "" ) )
        {
            to_actor( "You need to provide a subject.\n\r", ch );
            return;
        }

        EMAIL(ch->pnote->to_list, ch->pnote->subject, ch->pnote->text);
        to_actor( "Your email has been sent to ", ch );
        to_actor( ch->pnote->to_list, ch );
        to_actor( ".\n\r", ch );
        cmd_note( ch, "clear" );
        return; 
    }
 
    if ( !str_cmp( arg, "post" ) || !str_prefix( arg, "send" ) )
    {
        char *strtime;

        if ( !CAN_WRITE(ch, bnum) )
        {
           to_actor( "It is impossible to to write any notes here.\n\r", ch );
           return;
        }
 
        if ( ch->pnote == NULL )
        {
            to_actor( "You have no note in progress.\n\r", ch );
            return;
        }
 
        if ( !str_cmp( ch->pnote->to_list, "" ) )
        {
            to_actor(
              "You need to provide a recipient (name, all, or immortal).\n\r",
                         ch );
            return;
        }
 
        if ( !str_cmp( ch->pnote->subject, "" ) )
        {
            to_actor( "You need to provide a subject.\n\r", ch );
            return;
        }
 
        ch->pnote->next                 = NULL;
        strtime                         = ctime( &current_time );
        strtime[strlen(strtime)-1]      = '\0';
        ch->pnote->date                 = str_dup( strtime );
        ch->pnote->date_stamp           = current_time;
 
        if ( note_list[bnum] == NULL )
        {
            note_list[bnum]   = ch->pnote;
        }
        else
        {
            for ( pnote = note_list[bnum]; pnote->next != NULL; pnote = pnote->next )
                ;
            pnote->next = ch->pnote;
        }
        pnote           = ch->pnote;
        ch->pnote       = NULL;
        save_board( bnum );
        if ( ch->desc->connected <= NET_PLAYING ) to_actor( "Ok.\n\r", ch );
        act( "$n tacks a note up on $p.", ch, prop, NULL, TO_SCENE );
        return;
    }
 
 
    to_actor( "Huh?  Type 'help note' for usage.\n\r", ch );
    return;
}



