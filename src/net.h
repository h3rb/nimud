/******************************************************************************
 * Locke's   __ -based on merc v2.2-____        NIM Server Software           *
 * ___ ___  (__)__    __ __   __ ___|  | v5.1a  Version 5.1a                  *
 * |  /   \  __|  \__/  |  | |  |      |        documentation release         *
 * |       ||  |        |  \_|  | ()   |        Valentines Day Massacre 2003  *
 * |    |  ||  |  |__|  |       |      |                                      *
 * |____|__|___|__|  |__|\___/__|______|        http://www.nimud.org/nimud    *
 *   n a m e l e s s i n c a r n a t e          dedicated to chris cool       *
 ******************************************************************************/

/* Comment out if you don't want builders to apply; let's
   your mud automatically accept immortal players.  I tried
   this commented out for a while. */

#define APPLY_BUILDER                    1

/*
 * Connected state for a channel.
 */
#define CON_PLAYING                      0
#define CON_SHOW_TITLE                   1
#define CON_READ_MOTD                    2

#define CON_LOGIN_MENU                  10

#define CON_GEN_MENU                    20
#define CON_GEN_RACES                   21
#define CON_GEN_SEX                     22
#define CON_GEN_NAME                    23

/*
 * Connected state for a channel.
 */
#define CON_PLAYING			 0
#define CON_SHOW_TITLE                   1
#define CON_READ_MOTD                    2

#define CON_LOGIN_MENU                  10
#define CON_DOC_MENU                    15

#define CON_STAT_MENU                   30
#define CON_STAT_STR                    31
#define CON_STAT_INT                    32
#define CON_STAT_WIS                    33
#define CON_STAT_DEX                    34
#define CON_STAT_CON                    35
#define CON_STAT_AGE                    36
#define CON_STAT_SIZE                   37

#define CON_CONFIRM_NEW_NAME            59
#define CON_GET_NEW_PASSWORD            60
#define CON_CONFIRM_NEW_PASSWORD        61
#define CON_SHOW_INTRO                  62

#define CON_CHAR_GEN_NAME               63
#define CON_CHAR_GEN_EMAIL              64
#define CON_CHAR_GEN_BUILDER            65
#define CON_CHAR_GEN_WRITER             66
#define CON_CHAR_GEN_COLOR              67
#define CON_CHAR_GEN_APPLY_BUILDER      68

#define CON_CHAR_GEN_SEX                69
#define CON_CHAR_GEN_RACE               70
#define CON_CHAR_GEN_RACE_CONFIRM       71

#define CON_GET_NAME                   100
#define CON_GET_OLD_PASSWORD           101
#define CON_HERO_RECON                 102
#define CON_HERO_RECON_ABORT           103

#define CON_MESSAGES                    -1

#define CON_SKEDITOR                    -3    /* not yet implemented */
#define CON_SPEDITOR                    -4
#define CON_ZEDITOR                     -5
#define CON_REDITOR                     -6
#define CON_AEDITOR                     -7
#define CON_OEDITOR                     -8
#define CON_SEDITOR                     -9
#define CON_HEDITOR                     -10

#define CON_HOTBOOT_RECOVER	666
#define CON_RECOVER             666  /* Hotboot Recover */

#define CONNECTED(d)        ( d->connected <= CON_PLAYING )

