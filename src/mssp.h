#if !defined(NOINCLUDE)
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include <zlib.h>
#endif

/*
	Utility macros.
*/

#define UMIN(a, b)              ((a) < (b) ? (a) : (b))
#define UMAX(a, b)              ((a) > (b) ? (a) : (b))
//#define URANGE(a, b, c)         ((b) < (a) ? (a) : (b) > (c) ? (c) : (b))

#define HAS_BIT(var, bit)       ((var)  & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define DEL_BIT(var, bit)       ((var) &= (~(bit)))
#define TOG_BIT(var, bit)       ((var) ^= (bit))

#define RESTRING(point, value) \
{ \
	STRFREE(point); \
	point = strdup((value)); \
}

#define STRALLOC(point)			strdup((point))

#define STRFREE(point) \
{ \
	free((point)); \
	point = NULL; \
}

#define ALLOCMEM(result, type, number) \
{ \
	result = calloc(1, sizeof(type) * (number)); \
}

#define FREEMEM(point) \
{ \
	free(point); \
	point = NULL; \
}

/*
	Typedefs
*/

typedef struct mud           MUD;

#define COMPRESS_BUF_SIZE                  8000

#define FALSE                                 0
#define TRUE                                  1

#define BV00            (0   <<  0)
#define BV01            (1   <<  0)
#define BV02            (1   <<  1)
#define BV03            (1   <<  2)
#define BV04            (1   <<  3)
#define BV05            (1   <<  4)
#define BV06            (1   <<  5)
#define BV07            (1   <<  6)
#define BV08            (1   <<  7)
#define BV09            (1   <<  8)
#define BV10            (1   <<  9)

#define COMM_FLAG_DISCONNECT    BV01
#define COMM_FLAG_MCCP          BV02
#define COMM_FLAG_COMMAND       BV03
#define COMM_FLAG_NOINPUT       BV04
#define COMM_FLAG_ECHO          BV05
#define COMM_FLAG_PASSWORD      BV06
#define COMM_FLAG_EOR           BV07

/*
	Mud data, structure containing global variables.
*/

struct mud
{
	int                   boot_time;
	int                   port;
	int                   total_mob;
	int                   total_obj;
	int                   total_plr;
	int                   total_desc;
	int                   top_exit;
	int                   top_affect;
	int                   top_area;
	int                   top_ed;
	int                   top_help;
	int                   top_mob_index;
	int                   top_obj_index;
	int                   top_room;
	int                   top_reset;
	int                   top_shop;
	int                   top_mprog;
	int                   top_oprog;
	int                   top_mtrig;
	unsigned char       * mccp_buf;
};

/*
	Descriptor (channel) partial structure.
*/

struct descriptor
{
	char              * host;
	char              * terminal_type;
	char                outbuf[MAX_STRING_LENGTH];
	int                 outtop;
	char                telbuf[MAX_INPUT_LENGTH];
	int                 teltop;
	short               descriptor;
	short               comm_flags;
	short               cols;
	short               rows;
	z_stream          * mccp;
};

MUD *mud;

/*
	mud.c
*/

void log_printf         args( (char *fmt, ...) );

int write_to_descriptor args( (CONNECTION *d, char *txt, int length) );

char *capitalize_all    args( (char *str) );

/*
	telopt.c
*/

int         translate_telopts        args( ( CONNECTION *d, char *src, int srclen, char *out ));
void        debug_telopts            args( ( CONNECTION *d, unsigned char *src, int srclen ));

int         process_will_ttype       args( ( CONNECTION *d, unsigned char *src, int srclen ));
int         process_sb_ttype_is      args( ( CONNECTION *d, unsigned char *src, int srclen ));
int         process_sb_naws          args( ( CONNECTION *d, unsigned char *src, int srclen ));
int         process_will_new_environ args( ( CONNECTION *d, unsigned char *src, int srclen ));
int         process_sb_new_environ   args( ( CONNECTION *d, unsigned char *src, int srclen ));
int         process_do_mssp          args( ( CONNECTION *d, unsigned char *src, int srclen ));
int         process_do_mccp          args( ( CONNECTION *d, unsigned char *src, int srclen ));
int         process_dont_mccp        args( ( CONNECTION *d, unsigned char *src, int srclen ));

int         skip_sb                  args( ( CONNECTION *d, unsigned char *src, int srclen ));
void        announce_support         args( ( CONNECTION *d ));

void        connection_printf        args( ( CONNECTION *d, char *fmt, ...));
char      * cat_sprintf              args( ( char *dest, char *fmt, ...));

int         start_compress           args( ( CONNECTION *d ));
void        end_compress             args( ( CONNECTION *d ));
void        process_compressed       args( ( CONNECTION *d ));
void        write_compressed         args( ( CONNECTION *d ));
void        send_will_mccp           args( ( CONNECTION *d ));
void        send_do_naws             args( ( CONNECTION *d ));
void        send_do_ttype            args( ( CONNECTION *d ));
void        send_char_mode           args( ( CONNECTION *d ));
void        send_do_new_environ      args( ( CONNECTION *d ));
void        send_will_eor            args( ( CONNECTION *d ));
void        send_echo_on             args( ( CONNECTION *d ));
void        send_echo_off            args( ( CONNECTION *d ));
void        send_ga                  args( ( CONNECTION *d ));
void        send_eor                 args( ( CONNECTION *d ));

/*
	client.c
*/

int         recv_sb_mssp                 args( (unsigned char *src, int srclen));

/*
	tables.c
*/

#define ANNOUNCE_WILL   BV01
#define ANNOUNCE_DO     BV02

struct telnet_type
{
	char      *name;
	int       flags;
};

extern char *telcmds[];
extern struct telnet_type telnet_table[];

