

/***************************************************************************
 * Mud Telopt Handler 1.2 by Igor van den Hoven.               16 Mar 2009 *
 * Adopted for NiMUD Wed 18 Mar 2009 16:51 by Herb Gilliland               *
 ***************************************************************************/


#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#if defined(BSD)
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/unistd.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <zlib.h>

#include "nimud.h"
#include "net.h"
#include "defaults.h"
//#include "sigsegv.h"

#define NOINCLUDE 1
#include "mssp.h"
#include "telnet.h"

#define TELOPT_DEBUG 1

char iac_will_ttype[]       = { IAC, WILL, TELOPT_TTYPE, 0 };
char iac_sb_ttype_is[]      = { IAC, SB,   TELOPT_TTYPE, ENV_IS, 0 };

char iac_sb_naws[]          = { IAC, SB,   TELOPT_NAWS, 0 };

char iac_will_new_environ[] = { IAC, WILL, TELOPT_NEW_ENVIRON, 0 };
char iac_sb_new_environ[]   = { IAC, SB,   TELOPT_NEW_ENVIRON, ENV_IS,0 };

char iac_do_mssp[]          = { IAC, DO,   TELOPT_MSSP, 0 };

char iac_do_mccp[]          = { IAC, DO,   TELOPT_MCCP, 0 };
char iac_dont_mccp[]        = { IAC, DONT, TELOPT_MCCP, 0 };



extern int         top_bonus          ;
extern int         top_zone           ;
extern int         top_ed             ;
extern int         top_exit           ;
extern int         top_help           ;
extern int         top_actor_template    ;
extern int         top_prop_template     ;
extern int         top_spawn          ;
extern int         top_scene          ;
extern int         top_spell          ;
extern int         top_skill          ;
extern int         top_shop           ;
extern int         top_variable       ;
extern int         top_event          ;
extern int         top_instance       ;
extern int         top_script         ;
extern int         top_player    ;
extern int         top_userdata       ;
extern int         top_alias     ;
extern int         top_attack         ;
extern int         top_prop           ;
extern int         top_connection     ;
extern int         top_note           ;

extern int         top_dbkey_help      ;
extern int         top_dbkey_spell     ;
extern int         top_dbkey_skill     ;

extern int         top_dbkey_script    ;
extern int         top_dbkey_actor     ;
extern int         top_dbkey_prop      ;
extern int         top_dbkey_scene     ;
extern int         top_dbkey_terrain   ;

extern int         num_players        ;

struct telopt_type
{
	int      size;
	char   * code;
	int   (* func) (CONNECTION *d, unsigned char *src, int srclen);
};

const struct telopt_type telopt_table [] =
{
	{ 3, iac_will_ttype,       &process_will_ttype},
	{ 4, iac_sb_ttype_is,      &process_sb_ttype_is},

	{ 3, iac_sb_naws,          &process_sb_naws},

	{ 3, iac_will_new_environ, &process_will_new_environ},
	{ 4, iac_sb_new_environ,   &process_sb_new_environ},

	{ 3, iac_do_mccp,          &process_do_mccp},
	{ 3, iac_dont_mccp,        &process_dont_mccp},

	{ 3, iac_do_mssp,          &process_do_mssp},

	{ 0, NULL,                 NULL}
};

int translate_telopts(CONNECTION *d, char *src, int srclen, char *out)
{
	int cnt, skip;
	unsigned char *pti, *pto;

	pti = src;
	pto = out;

	if (d->teltop)
	{
		if (d->teltop + srclen + 1 < MAX_INPUT_LENGTH)
		{
			memcpy(d->telbuf + d->teltop, src, srclen);

			srclen += d->teltop;

			pti = d->telbuf;
		}
		d->teltop = 0;
	}

	while (srclen > 0)
	{
		switch (*pti)
		{
			case IAC:
				skip = 2;

				debug_telopts(d, pti, srclen);

				for (cnt = 0 ; telopt_table[cnt].code ; cnt++)
				{
					if (srclen < telopt_table[cnt].size)
					{
						if (!memcmp(pti, telopt_table[cnt].code, srclen))
						{
							skip = telopt_table[cnt].size;

							break;
						}
					}
					else
					{
						if (!memcmp(pti, telopt_table[cnt].code, telopt_table[cnt].size))
						{
							skip = telopt_table[cnt].func(d, pti, srclen);

							break;
						}
					}
				}

				if (telopt_table[cnt].code == NULL && srclen > 1)
				{
					switch (pti[1])
					{
						case WILL:
						case DO:
						case WONT:
						case DONT:
							skip = 3;
							break;

						case SB:
							skip = skip_sb(d, pti, srclen);
							break;

						case IAC:
							*pto++ = *pti++;
							srclen--;
							skip = 1;
							break;

						default:
							if (TELCMD_OK(pti[1]))
							{
								skip = 2;
							}
							else
							{
								skip = 1;
							}
							break;
					}
				}

				if (skip <= srclen)
				{
					pti += skip;
					srclen -= skip;
				}
				else
				{
					memcpy(d->telbuf, pti, srclen);
					d->teltop = srclen;

					*pto = 0;
					return strlen(out);
				}
				break;

			case '\0':
				pti++;
				srclen--;
				break;

			case '\n':
				*pto++ = *pti++;
				srclen--;
				break;

			default:
				*pto++ = *pti++;
				srclen--;
				break;
		}
	}
	*pto = 0;

	return strlen(out);
}

void debug_telopts( CONNECTION *d, unsigned char *src, int srclen )
{
	if (srclen > 1 && TELOPT_DEBUG)
	{
		switch(src[1])
		{
			case IAC:
				wtf_logf("D%d@%s RCVD IAC IAC", d->connection, d->host);
				break;

			case DO:
			case DONT:
			case WILL:
			case WONT:
			case SB:
				if (srclen > 2)
				{
					wtf_logf("D%d@%s RCVD IAC %s %s", d->connection, d->host, TELCMD(src[1]), TELOPT(src[2]));
				}
				else
				{
					wtf_logf("D%d@%s RCVD IAC %s ?", d->connection, d->host, TELCMD(src[1]));
				}
				break;

			default:
				if (TELCMD_OK(src[1]))
				{
					wtf_logf("D%d@%s RCVD IAC %s", d->connection, d->host, TELCMD(src[1]));
				}
				else
				{
					wtf_logf("D%d@%s RCVD IAC %d", d->connection, d->host, src[1]);
				}
				break;
		}
	}
}

/*
	Send to client to have it disable local echo
*/

void send_echo_off( CONNECTION *d )
{
	connection_printf(d, "%c%c%c", IAC, WILL, TELOPT_ECHO);
}

/*
	Send to client to have it enable local echo
*/

void send_echo_on( CONNECTION *d )
{
	connection_printf(d, "%c%c%c", IAC, WONT, TELOPT_ECHO);
}


/*
	Send when client connects to enable TTYPE
*/

void send_do_ttype( CONNECTION *d )
{
	connection_printf(d, "%c%c%c", IAC, DO, TELOPT_TTYPE);
}

void send_char_mode( CONNECTION *d ) 
{
        connection_printf( d, "%c%c%c%c%c%c%c", IAC, WILL, SUPPRESS_GA, IAC, WILL, TELOPT_ECHO );
}

int process_will_ttype( CONNECTION *d, unsigned char *src, int srclen )
{
	if (*d->terminal_type == 0)
	{
		connection_printf(d, "%c%c%c%c%c%c", IAC, SB, TELOPT_TTYPE, ENV_SEND, IAC, SE);
	}
	return 3;
}

int process_sb_ttype_is( CONNECTION *d, unsigned char *src, int srclen )
{
	char val[MAX_INPUT_LENGTH];
	char *pto;
	int i;

	if (skip_sb(d, src, srclen) > srclen)
	{
		return srclen + 1;
	}

	pto = val;

	for (i = 4 ; i < srclen && src[i] != SE ; i++)
	{
		switch (src[i])
		{
			default:			
				*pto++ = src[i];
				break;

			case IAC:
				*pto = 0;
				RESTRING(d->terminal_type, val);
				break;
		}
	}
	return i + 1;
}

/*
	Send when client connects to enable NAWS
*/

void send_do_naws( CONNECTION *d )
{
	connection_printf(d, "%c%c%c", IAC, DO, TELOPT_NAWS);
}

int process_sb_naws( CONNECTION *d, unsigned char *src, int srclen )
{
	if (srclen > 6 && src[3] != IAC && src[4] != IAC && src[5] != IAC && src[6] != IAC)
	{
		d->cols = src[3] * 255 + src[4];
		d->rows = src[5] * 255 + src[6];
	}
	return skip_sb(d, src, srclen);
}

/*
	Send when client connects to enable NEW ENVIRON
*/

void send_do_new_environ( CONNECTION *d )
{
	connection_printf(d, "%c%c%c", IAC, DO, TELOPT_NEW_ENVIRON);
}

int process_will_new_environ( CONNECTION *d, unsigned char *src, int srclen )
{
	connection_printf(d, "%c%c%c%c%c%s%c%c", IAC, SB, TELOPT_NEW_ENVIRON, ENV_SEND, ENV_VAR, "SYSTEMTYPE", 
IAC, SE);

	return 3;
}

int process_sb_new_environ( CONNECTION *d, unsigned char *src, int srclen )
{
	char var[MAX_INPUT_LENGTH], val[MAX_INPUT_LENGTH];
	char *pto;
	int i;

	if (skip_sb(d, src, srclen) > srclen)
	{
		return srclen + 1;
	}

	var[0] = val[0] = 0;

	i = 4;

	while (i < srclen && src[i] != SE)
	{
		switch (src[i])
		{
			case ENV_VAR:
			case ENV_USR:
				i++;
				pto = var;

				while (i < srclen && src[i] >= 32 && src[i] != IAC)
				{
					*pto++ = src[i++];
				}
				*pto = 0;
				break;

			case ENV_VAL:
				i++;
				pto = val;

				while (i < srclen && src[i] >= 32 && src[i] != IAC)
				{
					*pto++ = src[i++];
				}
				*pto = 0;

				if (!strcasecmp(var, "SYSTEMTYPE") && !strcasecmp(val, "WIN32"))
				{
					RESTRING(d->terminal_type, "WIN32");
				}
				break;

			default:
				i++;
				break;
		}
	}
	return i + 1;
}

/*
	Send when client connects to enable MSSP
*/

void send_will_mssp( CONNECTION *d )
{
	connection_printf(d, "%c%c%c", IAC, WILL, TELOPT_MSSP);
}

int process_do_mssp( CONNECTION *d, unsigned char *src, int srclen )
{
	char buffer[MAX_STRING_LENGTH] = { 0 };

	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "NAME",              MSSP_VAL, MUD_NAME);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "PLAYERS",           MSSP_VAL, num_players);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "UPTIME",            MSSP_VAL, 24);

	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "CODEBASE",          MSSP_VAL, "NiMUD");
	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "CONTACT",           MSSP_VAL, "locke@nimud.divineright.org");
	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "CREATED",           MSSP_VAL, "1993");
	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "FAMILY",            MSSP_VAL, "DikuMUD");
	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "HOSTNAME",          MSSP_VAL, "nimud.divineright.org");
	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "ICON",              MSSP_VAL, "http://nimud.divineright.org/isles/contact.jpg");
	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "IP",                MSSP_VAL, "12.180.48.166");
	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "LANGUAGE",          MSSP_VAL, "English (Primary)");
	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "LOCATION",          MSSP_VAL, "USA");
	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "MINIMUM AGE",       MSSP_VAL, "13");
	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "PORT",              MSSP_VAL, "5333");
	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "WEBSITE",           MSSP_VAL, "http://nimud.divineright.org");

	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "GENRE",             MSSP_VAL, "Science Fiction / Fantasy");
	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "GAMEPLAY",          MSSP_VAL, "RP/H&S");
	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "GAMESYSTEM",        MSSP_VAL, "Tyselorrd");
	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "INTERMUD",          MSSP_VAL, "");
	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "STATUS",            MSSP_VAL, "LIVE");
	cat_sprintf(buffer, "%c%s%c%s", MSSP_VAR, "SUBGENRE",          MSSP_VAL, "Multiple");

	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "AREAS",             MSSP_VAL, top_zone);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "HELPFILES",         MSSP_VAL, top_dbkey_help);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "MOBILES",           MSSP_VAL, top_dbkey_actor);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "OBJECTS",           MSSP_VAL, top_dbkey_prop);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "ROOMS",             MSSP_VAL, top_dbkey_scene);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "RESETS",            MSSP_VAL, top_spawn);

	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "MUDPROGS",          MSSP_VAL, 0);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "MUDTRIGS",          MSSP_VAL, top_dbkey_script);

	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "CLASSES",           MSSP_VAL, 0);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "LEVELS",            MSSP_VAL, MAX_MORTAL_LEVEL);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "RACES",             MSSP_VAL, MAX_RACE);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "SKILLS",            MSSP_VAL, top_skill);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "WORLDS",            MSSP_VAL, 666);

	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "ANSI",              MSSP_VAL, 1);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "MCCP",              MSSP_VAL, 0);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "MCP",               MSSP_VAL, 0);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "MSP",               MSSP_VAL, 1);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "MXP",               MSSP_VAL, 1);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "PUEBLO",            MSSP_VAL, 0);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "VT100",             MSSP_VAL, 1);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "XTERM 256 COLORS",  MSSP_VAL, 1);

	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "PAY TO PLAY",       MSSP_VAL, 0);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "PAY FOR PERKS",     MSSP_VAL, 0);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "HIRING BUILDERS",   MSSP_VAL, 1);
	cat_sprintf(buffer, "%c%s%c%d", MSSP_VAR, "HIRING CODERS",     MSSP_VAL, 1);

	connection_printf(d, "%c%c%c%s%c%c", IAC, SB, TELOPT_MSSP, buffer, IAC, SE);

	return 3;
}

/*
	Send when client connects to enable MCCP.
*/

void send_will_mccp( CONNECTION *d )
{
	connection_printf(d, "%c%c%c", IAC, WILL, TELOPT_MCCP);
}

void *zlib_alloc( void *opaque, unsigned int items, unsigned int size )
{
	return calloc(items, size);
}


void zlib_free( void *opaque, void *address ) 
{
	free(address);
}


int start_compress( CONNECTION *d )
{
	char start_mccp[] = { IAC, SB, TELOPT_MCCP, IAC, SE, 0 };
	z_stream *stream;

	if (d->mccp)
	{
		return TRUE;
	}

	ALLOCMEM(stream, z_stream, 1);

	stream->next_in	    = NULL;
	stream->avail_in    = 0;

	stream->next_out    = mud->mccp_buf;
	stream->avail_out   = COMPRESS_BUF_SIZE;

	stream->data_type   = Z_ASCII;
	stream->zalloc      = zlib_alloc;
	stream->zfree       = zlib_free;
	stream->opaque      = Z_NULL;

	/*
		12, 5 = 32K of memory, more than enough
	*/

	if (deflateInit2(stream, Z_BEST_COMPRESSION, Z_DEFLATED, 12, 5, Z_DEFAULT_STRATEGY) != Z_OK)
	{
		wtf_logf("start_compress: failed deflateInit2 D%d@%s", d->connection, d->host);
		FREEMEM(stream);

		return FALSE;
	}

	write_to_descriptor(d, start_mccp, 0);

	/*
		The above call must send all pending output to the connection, since from now on we'll be compressing.
	*/

	d->mccp = stream;

	return TRUE;
}


void end_compress( CONNECTION *d )
{
	if (d->mccp == NULL)
	{
		return;
	}

	d->mccp->next_in	= NULL;
	d->mccp->avail_in	= 0;

	d->mccp->next_out	= mud->mccp_buf;
	d->mccp->avail_out	= COMPRESS_BUF_SIZE;

	if (deflate(d->mccp, Z_FINISH) != Z_STREAM_END)
	{
		wtf_logf("end_compress: failed to deflate D%d@%s", d->connection, d->host);
	}

	if (!SET_BIT(d->comm_flags, COMM_FLAG_DISCONNECT))
	{
		process_compressed(d);
	}

	if (deflateEnd(d->mccp) != Z_OK)
	{
		wtf_logf("end_compress: failed to deflateEnd D%d@%s", d->connection, d->host);
	}

	FREEMEM(d->mccp);

	return;
}


void write_compressed( CONNECTION *d )
{
	d->mccp->next_in    = d->outbuf;
	d->mccp->avail_in   = d->outtop;

	d->mccp->next_out   = mud->mccp_buf;
	d->mccp->avail_out  = COMPRESS_BUF_SIZE;

	d->outtop           = 0;

	if (deflate(d->mccp, Z_SYNC_FLUSH) != Z_OK)
	{
		return;
	}

	process_compressed(d);

	return;
}


void process_compressed( CONNECTION *d )
{
	int length;

	length = COMPRESS_BUF_SIZE - d->mccp->avail_out;

	if (write(d->connection, mud->mccp_buf, length) < 1)
	{
		wtf_logf("process_compressed D%d@%s", d->connection, d->host);
		SET_BIT(d->comm_flags, COMM_FLAG_DISCONNECT);

		return;
	}

	return;
}


int process_do_mccp( CONNECTION *d, unsigned char *src, int srclen )
{
	start_compress(d);

	return 3;
}

int process_dont_mccp( CONNECTION *d, unsigned char *src, int srclen )
{
	end_compress(d);

	return 3;
}

/*
	Returns the length of a telnet subnegotiation, return srclen + 1 for incomplete state.
*/

int skip_sb( CONNECTION *d, unsigned char *src, int srclen )
{
	int i;

	for (i = 1 ; i < srclen ; i++)
	{
		if (src[i] == SE && src[i-1] == IAC)
		{
			return i + 1;
		}
	}

	return srclen + 1;
}

/*
	Call this to announce support for telopts marked as such in tables.c
*/

void announce_support( CONNECTION *d)
{
	int i;

	for (i = 0 ; i < 255 ; i++)
	{
		if (telnet_table[i].flags)
		{
			if (HAS_BIT(telnet_table[i].flags, ANNOUNCE_WILL))
			{
				connection_printf(d, "%c%c%c", IAC, WILL, i);
			}
			if (HAS_BIT(telnet_table[i].flags, ANNOUNCE_DO))
			{
				connection_printf(d, "%c%c%c", IAC, DO, i);
			}
		}
	}
}

		
/*
	Utility function
*/

void connection_printf( CONNECTION *d, char *fmt, ... )
{
	char buf[MAX_STRING_LENGTH];
	int size;
	va_list args;

	va_start(args, fmt);

	size = vsprintf(buf, fmt, args);

	va_end(args);

	write_to_descriptor(d, buf, size);
}

char *cat_sprintf(char *dest, char *fmt, ...)
{
	char buf[MAX_STRING_LENGTH];

	va_list args;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	return strcat(dest, buf);
}


/*
        Send response generated above to recv_sb_mssp() in client.c for interpretation
*/

int write_to_descriptor(CONNECTION *d, char *txt, int length)
{
        debug_telopts(d, txt, length);

        if (txt[0] == (char) IAC && txt[1] == (char) SB && txt[2] == (char) TELOPT_MSSP)
        {
                recv_sb_mssp(txt, length);
        }

        return 0;
}




