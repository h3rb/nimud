/***************************************************************************
 * Mud Telopt Handler 1.0 by Igor van den Hoven.               27 Feb 2009 *
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


/*
	For clients and crawlers that want to parse a MSSP string. This routine
	does not check for packet fragmenation.
*/

int recv_sb_mssp(unsigned char *src, int srclen)
{
	char var[MAX_STRING_LENGTH], val[MAX_STRING_LENGTH];
	char *pto;
	int i;

	var[0] = val[0] = i = 0;

	while (i < srclen && src[i] != SE)
	{
		switch (src[i])
		{
			case MSSP_VAR:
				i++;
				pto = var;

				while (i < srclen && src[i] >= 3 && src[i] != IAC)
				{
					*pto++ = src[i++];
				}
				*pto = 0;
				break;

			case MSSP_VAL:
				i++;
				pto = val;

				while (i < srclen && src[i] >= 3 && src[i] != IAC)
				{
					*pto++ = src[i++];
				}
				*pto = 0;

				printf("%-20s %s\n", var, val);
				break;

			default:
				i++;
				break;
		}
	}
	return UMIN(i + 1, srclen);
}

