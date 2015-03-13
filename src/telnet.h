/***************************************************************************
 * Mud Telopt Handler 0.1 by Igor van den Hoven.               26 Feb 2009 *
 ***************************************************************************/

/*
	telnet protocol
*/

#define     IAC                 255
#define     DONT                254
#define     DO                  253
#define     WONT                252
#define     WILL                251
#define     SB                  250
#define     GA                  249   /* used for prompt marking */
#define     EL                  248
#define     EC                  247
#define     AYT                 246
#define     AO                  245
#define     IP                  244
#define     BREAK               243
#define     DM                  242
#define     NOP                 241   /* used for keep alive messages */
#define     SE                  240
#define     EOR                 239   /* used for prompt marking */
#define     ABORT               238
#define     SUSP                237
#define     xEOF                236

#define     SUPPRESS_GA         3

/*
	telnet options
*/

#define     TELOPT_ECHO           1   /* used to toggle local echo */
#define     TELOPT_SGA            3   /* used to toggle character mode */
#define     TELOPT_TTYPE         24   /* used to send client terminal type */
#define     TELOPT_EOR           25   /* used to toggle eor mode */
#define     TELOPT_NAWS          31   /* used to send client window size */
#define     TELOPT_NEW_ENVIRON   39   /* used to send information */
#define     TELOPT_MSSP          70   /* used to send mud server information */
#define     TELOPT_MCCP          86   /* used to toggle Mud Client Compression Protocol */
#define     TELOPT_MSP           90   /* used to toggle Mud Sound Protocol */
#define     TELOPT_MXP           91   /* used to toggle Mud Extention Protocol */

#define	    ENV_IS                0   /* option is... */
#define	    ENV_SEND              1   /* send option */
#define	    ENV_INFO              2   /* not sure */
#define	    ENV_VAR               0
#define	    ENV_VAL               1
#define	    ENV_ESC               2
#define     ENV_USR               3

#define     MSSP_VAR              1
#define     MSSP_VAL              2

#define     TELCMD_OK(c)    ((unsigned char) (c) >= xEOF)
#define     TELCMD(c)       (telcmds[(unsigned char) (c) - xEOF])
#define     TELOPT(c)       (telnet_table[(unsigned char) (c)].name)

