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

#if defined(HTML)

/*
 * Replaces \n's with <BR>s
 */
char *add_br( const char *str ) {
    static char strfix[MAX_STRING_LENGTH];
    int i;
    int o;

    if ( str == NULL )
        return '\0';

    for ( o=i=0; str[i] != '\0'; i++ ) {

      if ( str[i] == '\n' ) {
         strfix[o++] = '<';
         strfix[o++] = 'B';
         strfix[o++] = 'R';
         strfix[o++] = '>';
      } else
      if ( str[i] == '&' ) {
         strfix[o++] = '&';
         strfix[o++] = 'a';
         strfix[o++] = 'm';
         strfix[o++] = 'p';
         strfix[o++] = ';';
      } else
      if ( str[i] == '<' ) {
         strfix[o++] = '&';
         strfix[o++] = 'l';
         strfix[o++] = 't';
         strfix[o++] = ';';
      } else
      if ( str[i] == '>' ) {
         strfix[o++] = '&';
         strfix[o++] = 'g';
         strfix[o++] = 't';
         strfix[o++] = ';';
      } else
      strfix[o++] = str[i];        

      if ( o>=MAX_STRING_LENGTH-1 ) {strfix[MAX_STRING_LENGTH-1]='\0';break;}
    }
      if (str[i]=='\0')strfix[o]='\0';

    return strfix;
}

/*
 * Exports help documentation to html.
 * See settings in nimud.h to control location of output.
 * Manipulate the code to change the html template.
 */

void html_export_helps( ) {

  HELP * pHelp;

  FILE * fp;
  FILE * ifp;

  char HFILE  [MAX_STRING_LENGTH];
  char IFILE  [MAX_STRING_LENGTH];

  int cidx,dbkey;

  sprintf(IFILE, "%sindex.html", HTML_DIR );
 
  if (! (ifp = fopen( IFILE, "w") ) )
  {
     bug( "Export_html: index.html", 0);
     perror( IFILE );
  }

  fprintf( ifp, "<HEAD><TITLE>NiM5 Documentation Index</TITLE>" );
  fprintf( ifp, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n" );
  fprintf( ifp, "<meta keywords=\"MUD MU MUSH MUCK GAME ROLEPLAYING FANTASY SCIENCE FICTION SCI-FI GNU ANSI C MULTIPLAYER MULTI GAMING ONLINE\">\n" ); 
  fprintf( ifp, "<STYLE TYPE=\"text/css\">\n" );
  fprintf( ifp, "<!--A{text-decoration: none; font-weight: bold;}-->\n" );
  fprintf( ifp, "A:link {text-decoration:none; color: #FFCC66; font-weight: bold }\n" );
  fprintf( ifp, "A:visited {text-decoration:none; color: #FFCC66; font-weight: bold}\n" );
  fprintf( ifp, "A:active {color: #FFCC66; font-weight: bold}\n" );
  fprintf( ifp, "A:hover {text-decoration:underline; color: #FFFF99; font-weight: bold}</STYLE></HEAD>\n" );
  fprintf( ifp, "<BODY bgcolor=\"#000000\" text=\"#6699FF\" link=\"#CCCCCC\" vlink=\"#FFFFFF\" alink=\"#FF9966\">" );
  fprintf( ifp, "<B>NiM5 Documentation</B>\n<br><hr height=1>\n\n\n" );

  for ( cidx=0; cidx < MAX_HELP_CLASS; cidx++ ) {
     char buf [MAX_STRING_LENGTH];

     fprintf( ifp, "<I>%s</I> <FONT SIZE=1><A HREF=\"%s.html\">%s</A></FONT><BR><FONT SIZE=-1>\n", 
              help_class_table[cidx].desc,
              help_class_table[cidx].filename,     
              help_class_table[cidx].name );
  

     sprintf(HFILE, "%s%s.html", HTML_DIR, help_class_table[cidx].filename);

     if (! (fp = fopen( HFILE, "w") ) )
     {
         bug( "Export_html: fopen", 0);
         perror( HFILE );
     }

     fprintf(fp, "<HTML><HEAD>\n");
     fprintf(fp, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">" );
     fprintf(fp, "<meta keywords=\"MUD MU MUSH MUCK GAME ROLEPLAYING FANTASY SCIENCE FICTION SCI-FI GNU ANSI C MULTIPLAYER MULTI GAMING ONLINE\">" ); 
     fprintf(fp, "<TITLE>NiM5 Documentation: %s</TITLE></HEAD>\n", 
                 help_class_table[cidx].name );
     fprintf(fp, "<BODY bgcolor=\"#000000\" text=\"#6699FF\" link=\"#CCCCCC\" vlink=\"#FFFFFF\" alink=\"#FF9966\">" );
     fprintf(fp, "\n<B>NiM5: %s</B><BR><BR>", help_class_table[cidx].desc );

     for ( dbkey=0; dbkey <= top_dbkey_help; dbkey++ ) {
        pHelp =get_help_index(dbkey); 
        if ( !pHelp ) continue;

        if ( pHelp->class == cidx ) {

        one_argcase( pHelp->name, buf );

        replace_char(buf, ' ', '_');
/*
        while ( strstr (buf, " " ) ) {
           string_replace ( buf, " ", "_" );
        }
*/

        fprintf(ifp, "<A HREF=\"%s.html#%s\">%s</A>\n ", 
                     help_class_table[cidx].filename, buf, buf );

        fprintf(fp, "\n<A NAME=\"%s\">\n", buf );
        fprintf(fp, "\n<BR><HR HEIGHT=1>Topic: <B>%s</B><BR>\n", buf );
        fprintf(fp, "Keywords: <B>%s</B><BR>\n", pHelp->keyword);
        fprintf(fp, "<BR><B>Syntax:</B><BR>%s<BR>", add_br(fix_string(pHelp->syntax)));

        if ( pHelp->text[0] != '.' ) {
           fprintf(fp, "<BR>\n%s<BR>\n", add_br((pHelp->text)) );
        } else {
           fprintf(fp, "<BR>\n%s<BR>\n", add_br((pHelp->text+1)));
        }

        fprintf(fp, "<BR>%s<BR>\n", add_br((pHelp->immtext)));
        fprintf(fp, "<B>Example:</B><BR>%s\n", add_br((pHelp->example)));
        fprintf(fp, "<BR><B>See Also:</B> %s<BR>\n", pHelp->seealso);
            /*
             * Add auto-linking of see-also here
             */
        fprintf(fp, "<BR>\n" );

       }
     }

     fprintf(fp,"<i>NiM5: Multi-user gaming software.<br>Copyright (c) 1992-2004 Herb Gilliland.  All Rights reserved.</i>\n" );
     fprintf(fp,"</BODY><!--AUTOGENERATED BY NiM5></HTML>\n\n");
     fclose(fp);

     fprintf(ifp,"</FONT><BR><BR>\n" );
   }

   fprintf(ifp, "\n</BODY></HTML>\n" );

   fclose(ifp);
   return;
}

#endif
