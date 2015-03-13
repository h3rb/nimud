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
#define NET_PLAYING                      0
#define NET_SHOW_TITLE                   1
#define NET_READ_MOTD                    2

#define NET_LOGIN_MENU                  10

#define NET_GEN_MENU                    20
#define NET_GEN_RACES                   21
#define NET_GEN_SEX                     22
#define NET_GEN_NAME                    23

/*
 * Connected state for a channel.
 */
#define NET_PLAYING			 0
#define NET_SHOW_TITLE                   1
#define NET_READ_MOTD                    2

#define NET_LOGIN_MENU                  10
#define NET_DOC_MENU                    15

#define NET_STAT_MENU                   30
#define NET_STAT_STR                    31
#define NET_STAT_INT                    32
#define NET_STAT_WIS                    33
#define NET_STAT_DEX                    34
#define NET_STAT_CON                    35
#define NET_STAT_AGE                    36
#define NET_STAT_SIZE                   37

#define NET_CONFIRM_NEW_NAME            59
#define NET_GET_NEW_PASSWORD            60
#define NET_CONFIRM_NEW_PASSWORD        61
#define NET_SHOW_INTRO                  62

#define NET_CHAR_GEN_NAME               63
#define NET_CHAR_GEN_EMAIL              64
#define NET_CHAR_GEN_BUILDER            65
#define NET_CHAR_GEN_WRITER             66
#define NET_CHAR_GEN_COLOR              67
#define NET_CHAR_GEN_APPLY_BUILDER      68

#define NET_CHAR_GEN_SEX                69
#define NET_CHAR_GEN_RACE               70
#define NET_CHAR_GEN_RACE_CONFIRM       71

#define NET_GET_NAME                   100
#define NET_GET_OLD_PASSWORD           101
#define NET_HERO_RECON                 102
#define NET_HERO_RENET_ABORT           103

#define NET_MESSAGES                    -1

#define NET_SKEDITOR                    -3    /* not yet implemented */
#define NET_SPEDITOR                    -4
#define NET_ZEDITOR                     -5
#define NET_REDITOR                     -6
#define NET_AEDITOR                     -7
#define NET_OEDITOR                     -8
#define NET_SEDITOR                     -9
#define NET_HEDITOR                     -10

#define NET_HOTBOOT_RECOVER	666
#define NET_RECOVER             666  /* Hotboot Recover */

#define CONNECTED(d)        ( d->connected <= NET_PLAYING )

