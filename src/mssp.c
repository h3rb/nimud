#include <stdarg.h>
  
#include "mssp.h"
#include "telnet.h"

void wtf_logf(char *fmt, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list args;

	va_start(args, fmt);

	vsprintf(buf, fmt, args);

	va_end(args);

	printf("%s\n", buf);

	return;
}

/*
	Call translate_telopts() in telopt.c with a MSSP request.
*/

int main(int argc, char **argv)
{
	CONNECTION_DATA *d;
	char input[MAX_INPUT_LENGTH], output[MAX_INPUT_LENGTH];

	ALLOCMEM(mud, MUD_DATA, 1);

	ALLOCMEM(mud->mccp_buf, unsigned char, COMPRESS_BUF_SIZE);

	ALLOCMEM(d, CONNECTION_DATA, 1);

	d->host = strdup("internal");

	printf("\n\n\033[1;31m-- receiving announce_support\033[0m\n\n");

	announce_support(d);

	printf("\n\n\033[1;31m-- sending IAC DO MSSP as a broken packet:\033[0m\n\n");

	sprintf(input, "%c%c", IAC, DO);

	translate_telopts(d, input, 2, output);

	sprintf(input, "%c", TELOPT_MSSP);

	translate_telopts(d, input, 1, output);

	return 0;
}

/*
	Send response generated above to recv_sb_mssp() in client.c for interpretation
*/

int write_to_descriptor(CONNECTION_DATA *d, char *txt, int length)
{
	debug_telopts(d, txt, length);

	if (txt[0] == (char) IAC && txt[1] == (char) SB && txt[2] == (char) TELOPT_MSSP)
	{
		recv_sb_mssp(txt, length);
	}

	return 0;
}

