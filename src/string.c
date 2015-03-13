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

#include "net.h"
#include "nimud.h"


char *itoa( int x ) {
   static char buf[MSL];
   snprintf( buf, MSL, "%d", x );
   return buf;
}


// We assume a is a pointer to char[MSL]
void strjoin( char *a, char *b ) {
    char *p=a;
    if ( strlen(a)+strlen(b) > MSL ) p=a;
    else
    while ( *p != '\0' ) p++;

    while ( *b != '\0' ) *p++=*b++;   
    *p='\0';
    return; 
}


/*
 * Cuts a sentence or phrase with "the hole to the surface."
 * to be "the hole."
 */
char *cut_to( char *p ) {
  static char buf[MSL];
  char *t=buf;
  char *o=p;
  bool found=FALSE;

  while ( *o != '\0' && (found=(*o!='t' && *(o+1)!='o' && *(o+1) != '\0' && *(o+2)!=' ')) ) {
    *t=*o; t++;
    o++;
    if ( *o==' ' ) { *t=' '; t++; o++; }
    o=skip_spaces(o);
    if ( *o == 't' && *(o+1) == 'o' && *(o+2) == ' ' ) { found=true; break; }
  }

  if ( found ) {
    *t='.'; t++; *t='\0';
    return buf;
  }
  else return p;
}


bool char_isof( char c, char *list ) {
  while ( *list != '\0' ) if ( c==*list ) return TRUE; else list++;
  return FALSE;
}


/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde( char *str )
{
    for ( ; *str != '\0'; str++ )
    {
	if ( *str == '~' )
	    *str = '-';
    }

    return;
}


/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr )
{
    if ( astr == NULL && bstr == NULL ) return TRUE;
    if ( astr == NULL || bstr == NULL ) return FALSE;

    for ( ; *astr || *bstr; astr++, bstr++ )
        if ( LOWER(*astr) != LOWER(*bstr) )
            return TRUE;

    return FALSE;
}


/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
    if ( astr == NULL )
    {
    bug( "Str_prefix: null astr.", 0 );
	return TRUE;
    }

    if ( bstr == NULL )
    {
    bug( "Str_prefix: null bstr.", 0 );
	return TRUE;
    }

    for ( ; *astr; astr++, bstr++ )
    {
	if ( LOWER(*astr) != LOWER(*bstr) )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;
    int ichar;
    char c0;

    if ( astr == NULL )
    {
        char buf[MAX_STRING_LENGTH];

        snprintf( buf, MAX_STRING_LENGTH, "Str_infix: null astr (compared against '%s').", bstr );
        bug( buf, 0 );
        return TRUE;
    }

    if ( bstr == NULL )
    {
        char buf[MAX_STRING_LENGTH];

        snprintf( buf, MAX_STRING_LENGTH, "Str_infix: null bstr (compared against '%s').", astr );
        bug( buf, 0 );
        return TRUE;
    }

    if ( ( c0 = LOWER(astr[0]) ) == '\0' )
	return FALSE;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);

    for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
    {
	if ( c0 == LOWER(bstr[ichar]) && !str_prefix( astr, bstr + ichar ) )
	    return FALSE;
    }

    return TRUE;
}



/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;

    if ( !astr || !bstr ) return TRUE;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);
    if ( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
	return FALSE;
    else
	return TRUE;
}






/*
 * Returns an initial-capped string.
 */
char *capitalize( const char *str )
{
    static char strcap[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
    strcap[i] = str[i];
    strcap[i] = '\0';
    strcap[0] = UPPER(strcap[0]);
    return strcap;
}



/*
 * Append a string to a file.
 */
void append_file( PLAYER *ch, char *file, char *str )
{
    FILE *fp;

    if ( NPC(ch) || str[0] == '\0' )
	return;

    fclose( fpReserve );
    if ( ( fp = fopen( file, "a" ) ) == NULL )
    {
	perror( file );
	to_actor( "Could not open the file!\n\r", ch );
    }
    else
    {
	fprintf( fp, "[%5d] %s: %s\n",
	    ch->in_scene ? ch->in_scene->dbkey : 0, ch->name, str );
	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return;
}



/*
 * Smash all \r's and tildes for file writes.
 */
char *fix_string( const char *str )
{
    static char strfix[MAX_STRING_LENGTH];
    int i;
    int o;

    if ( str == NULL )
        return '\0';

    for ( o = i = 0; str[i+o] != '\0'; i++ )
    {
        if (str[i+o] == '\r' || str[i+o] == '~')
            o++;
        strfix[i] = str[i+o];
    }
    strfix[i] = '\0';
    return strfix;
}



/*
 * Sees if last char is 's' and returns 'is' or 'are' 
 * if the word is not like 'dress' 
 */
char * is_are( char *t )
{
    while ( *t != '\0' )
    {
        t++;
    }

    /*
     * Special cases.
     */

    if ( !str_infix( t, "kris" ) ) return "is";

    t--;

    if ( LOWER(*t) == 's' && LOWER(*(--t)) != 's' )  
        return "are";
   else return "is";
}


/*
 * Smashes all occurances of a string from a larger string.
 * Arguments must be seperated by a space.
 */
char *smash_arg( char *t, char *name )
{
    char *arg;
    char buf[MAX_STRING_LENGTH];
    static char buf2[MAX_STRING_LENGTH];

    buf2[0] = '\0';

    arg = one_argument( t, buf );

    while ( buf[0] != '\0' )
    {
        if ( str_cmp( buf, name ) )
        {
             strcat( buf2, buf );
             strcat( buf2, " " );
        }

        arg = one_argument( arg, buf );
    }

    return buf2;
}




/*
 * Grab 'a' 'an's and 'the's and smash the living hell out of them.
 * Good for use with manipulation of prop names.  Only destroys first
 * occurance.
 */
char *smash_article( char *t )
{
    char *arg;
    char buf[MAX_STRING_LENGTH];
    static char buf2[MAX_STRING_LENGTH];

    one_argument( t, buf );

    if ( !str_cmp( buf, "the" ) ||
         !str_cmp( buf, "an"  ) ||
         !str_cmp( buf, "a"   ) )
    {
        arg = one_argument( t, buf );
        sprintf( buf2, "%s", arg );
    }
    else strcpy( buf2, t );

    return buf2;
}



/*
 * Pluralize object short descriptions properly.
 */
char *pluralize( char *argument )
{
    static char buf[MAX_STRING_LENGTH];
    char *v;

    snprintf( buf, MAX_STRING_LENGTH, "%s", smash_article(argument) );

    if ( strstr( buf, "some" ) ) return buf;
    if ( strstr( buf, "key" ) ) {
       strcat( buf, "s" ); return buf;
     }
    v = strstr( buf, " of " );
    if ( v==NULL ) v = strstr( buf, " to " );

    if ( v == NULL )
    {
        if ( LOWER(buf[strlen(buf)-1]) == 'y' )
        {
            if ( LOWER(buf[strlen(buf)-2]) == 'a' ) 
            {
              strcat( buf, "s" );
            }
            else {
              buf[strlen(buf)-1] = 'i';
              strcat( buf, "es" );
            }
        }
        else
        if ( LOWER(buf[strlen(buf)-1]) == 'f' 
          && !LOWER(buf[strlen(buf)-2]) == 'f'
          && !(LOWER(buf[strlen(buf)-2]) == 'i'
            && LOWER(buf[strlen(buf)-3]) == 'o') )
        {
            buf[strlen(buf)-1] = 'v';
            strcat( buf, "es" );
        }
        else
        if ( LOWER(buf[strlen(buf)-1]) == 'h' )
        strcat( buf, "es" );
        else
        if ( LOWER(buf[strlen(buf)-1]) == 'x' )
        strcat( buf, "es" );
        else            
        if ( LOWER(buf[strlen(buf)-1]) == 's' )
        {
            if ( LOWER(buf[strlen(buf)-2]) == 'u'
              && !IS_VOWEL(LOWER(buf[strlen(buf)-3])) )
            {
                buf[strlen(buf)-2] = 'i';
                buf[strlen(buf)-1] = '\0';
            }
            else
            strcat( buf, "es" );
        }
        else
        strcat( buf, "s" );
    }
    else
    {
        char xbuf[MAX_STRING_LENGTH];

        sprintf( xbuf, "%s", v );
        buf[strlen(buf)-strlen(v)] = '\0';

        if ( LOWER(buf[strlen(buf)-1]) == 'y' )
        {
            if ( LOWER(buf[strlen(buf)-2]) == 'a' ) 
            {
              strcat( buf, "s" );
            }
            else {
              buf[strlen(buf)-1] = 'i';
              strcat( buf, "es" );
            }
        }
        else
        if ( LOWER(buf[strlen(buf)-1]) == 'f' 
          && !LOWER(buf[strlen(buf)-2]) == 'f'
          && !(LOWER(buf[strlen(buf)-2]) == 'i'
            && LOWER(buf[strlen(buf)-3]) == 'o') )
        {
            buf[strlen(buf)-1] = 'v';
            strcat( buf, "es" );
        }
        else
        if ( LOWER(buf[strlen(buf)-1]) == 'h' )
        strcat( buf, "es" );
        else
        if ( LOWER(buf[strlen(buf)-1]) == 'x' )
        strcat( buf, "es" );
        else            
        if ( LOWER(buf[strlen(buf)-1]) == 's' )
        {
            if ( LOWER(buf[strlen(buf)-2]) == 'u'
              && !IS_VOWEL(LOWER(buf[strlen(buf)-3])) )
            {
                buf[strlen(buf)-2] = 'i';
                buf[strlen(buf)-1] = '\0';
            }
            else
            strcat( buf, "es" );
        }
        else
        strcat( buf, "s" );

        strcat( buf, xbuf );
    }

    return buf;
}




/*
 * Thanks to Kalgen for the new scriptedure (no more bug!)
 * Original wordwrap() written by Surreality.
 */
char *format_string( char *oldstring /*, bool fSpace */)
{
  char xbuf[MAX_STRING_LENGTH];
  char xbuf2[MAX_STRING_LENGTH];
  char *rdesc;
  int i=0;
  bool cap=TRUE;
  
  xbuf[0]=xbuf2[0]=0;
  
  i=0;
  
  for (rdesc = oldstring; *rdesc; rdesc++)
  {
    if (*rdesc=='\n')
    {
      if (i!=0&&xbuf[i-1] != ' ')
      {
        xbuf[i]=' ';
        i++; 
      }
    }
    else if (*rdesc=='\r') ;
    else if (*rdesc==' ')
    {
      if (i!=0&&xbuf[i-1] != ' ')
      {
        xbuf[i]=' ';
        i++;
      }
    }
    else if (*rdesc==')')
    {
      if (i>1&&xbuf[i-1]==' ' && xbuf[i-2]==' ' && 
          (xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!'))
      {
        xbuf[i-2]=*rdesc;
        xbuf[i-1]=' ';
        xbuf[i]=' ';
        i++;
      }
      else
      {
        xbuf[i]=*rdesc;
        i++;
      }
    }
    else if ( (*rdesc=='.' || *rdesc=='?' || *rdesc=='!') && !char_isof(*(rdesc+1),"0123456789" ) ) {
      if (i>2&&xbuf[i-1]==' ' && xbuf[i-2]==' ' && 
          (xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!')) {
        xbuf[i-2]=*rdesc;
        if (*(rdesc+1) != '\"')
        {
          xbuf[i-1]=' ';
          xbuf[i]=' ';
          i++;
        }
        else
        {
          xbuf[i-1]='\"';
          xbuf[i]=' ';
          xbuf[i+1]=' ';
          i+=2;
          rdesc++;
        }
      }
      else
      {
        xbuf[i]=*rdesc;
        if (*(rdesc+1) != '\"')
        {
          xbuf[i+1]=' ';
          xbuf[i+2]=' ';
          i += 3;
        }
        else
        {
          xbuf[i+1]='\"';
          xbuf[i+2]=' ';
          xbuf[i+3]=' ';
          i += 4;
          rdesc++;
        }
      }
      cap = TRUE;
    }
    else
    {
      xbuf[i]=*rdesc;
      if ( cap )
        {
          cap = FALSE;
          xbuf[i] = UPPER( xbuf[i] );
        }
      i++;
    }
  }
  xbuf[i]=0;
  strcpy(xbuf2,xbuf);
  
  rdesc=xbuf2;
  
  xbuf[0]=0;
  
  for ( ; ; )
  {
    for (i=0; i<77; i++)
    {
      if (!*(rdesc+i)) break;
    }
    if (i<77)
    {
      break;
    }
    for (i=(xbuf[0]?76:73) ; i ; i--)
    {
      if (*(rdesc+i)==' ') break;
    }
    if (i)
    {
      *(rdesc+i)=0;
      strcat(xbuf,rdesc);
      strcat(xbuf,"\n\r");
      rdesc += i+1;
      while (*rdesc == ' ') rdesc++;
    }
    else
    {
      bug ("No spaces", 0);
      *(rdesc+75)=0;
      strcat(xbuf,rdesc);
      strcat(xbuf,"-\n\r");
      rdesc += 76;
    }
  }
  while (*(rdesc+i) && (*(rdesc+i)==' '||
                        *(rdesc+i)=='\n'||
                        *(rdesc+i)=='\r'))
    i--;
  *(rdesc+i+1)=0;
  strcat(xbuf,rdesc);
  if (xbuf[strlen(xbuf)-2] != '\n')
    strcat(xbuf,"\n\r");

  free_string(oldstring);
  return(str_dup(xbuf));
}



/*
 * Adaptation of Kalgen's word-wrap scriptedure for neater effects.
 */
char *format_indent( char *oldstring, char *prefix, int w, bool fEnd )
{
    char xbuf[MAX_STRING_LENGTH];
    char xbuf2[MAX_STRING_LENGTH];
    char *rdesc;
    int i=0;
    bool cap=TRUE;

    xbuf[0]=xbuf2[0]=0;

    i = 0;

    for ( rdesc = oldstring; *rdesc; rdesc++ )
    {
        if ( *rdesc == '\n' )
        {
            if ( xbuf[i-1] != ' ' )
            {
                xbuf[i] = ' ';
                i++;
            }
        }
        else
        if ( *rdesc == '\r' ) ;
        else
        if ( *rdesc == ' ' )
        {
            if ( xbuf[i-1] != ' ' )
            {
                xbuf[i]=' ';
                i++;
            }
        }
        else
        if ( *rdesc == ')' )
        {
            if ( xbuf[i-1] == ' '
              && xbuf[i-2] == ' '
              && ( xbuf[i-3] == '.'
                || xbuf[i-3] == '?'
                || xbuf[i-3] == '!' ) )
            {
                xbuf[i-2]=*rdesc;
                xbuf[i-1]=' ';
                xbuf[i]=' ';
                i++;
            }
            else
            {
                xbuf[i]=*rdesc;
                i++;
            }
        }
        else
        if ( *rdesc == '.'
          || *rdesc == '?'
          || *rdesc == '!' )
        {
            cap = TRUE;
            if ( xbuf[i-1] == ' '
              && xbuf[i-2] == ' '
              && ( xbuf[i-3] == '.'
                || xbuf[i-3] == '?'
                || xbuf[i-3] == '!' ) )
            {
                xbuf[i-2]=*rdesc;
                if ( *(rdesc+1) != '\"' )
                {
                    xbuf[i-1]=' ';
                    xbuf[i]=' ';
                    i++;
                }
                else
                {
                    xbuf[i-1]='\"';
                    xbuf[i]=' ';
                    xbuf[i+1]=' ';
                    i+=2;
                    rdesc++;
                }
            }
            else
            {
                xbuf[i]=*rdesc;
                if (*(rdesc+1) != '\"')
                {
                    if ( *(rdesc+1) == ' ' )
                    {
                        xbuf[i+1]=' ';
                        xbuf[i+2]=' ';
                        i += 3;
                    }
                    else
                    {
                        cap = FALSE;
                        xbuf[i] = *rdesc;
                        i++;
                    }
                }
                else
                {
                  xbuf[i+1]='\"';
                  xbuf[i+2]=' ';
                  xbuf[i+3]=' ';
                  i += 4;
                  rdesc++;
                }
            }
        }
        else
        {
            xbuf[i]=*rdesc;
            if ( cap )
            {
                cap = FALSE;
                xbuf[i] = UPPER( xbuf[i] );
            }
            i++;
        }
    }

    xbuf[i]=0;
    strcpy(xbuf2,xbuf);

    rdesc=xbuf2;

    xbuf[0]=0;

    for ( ; ; )
    {
        for ( i = 0; i < w+1; i++ )
        {
            if ( !*(rdesc+i) )
            break;
        }

        if ( i < w+1 )
            break;

        for ( i = xbuf[0] ? w : w-3; i; i-- )
        {
            if ( *(rdesc+i) == ' ' )
            break;
        }

        if ( i )
        {
            *(rdesc+i)=0;
            strcat(xbuf,rdesc);
            strcat(xbuf,"\n\r");
            strcat(xbuf,prefix);
            rdesc += i+1;
            while (*rdesc == ' ') rdesc++;
        }
        else
        {
            bug ("No spaces", 0);
            *(rdesc+w-1)=0;
            strcat(xbuf,rdesc);
            strcat(xbuf,"-\n\r");
            rdesc += w;
        }
    }

    while (*(rdesc+i) && (*(rdesc+i)==' '||
                          *(rdesc+i)=='\n'||
                          *(rdesc+i)=='\r'))
      i--;
    *(rdesc+i+1)=0;
    strcat(xbuf,rdesc);
    if ( fEnd && xbuf[strlen(xbuf)-2] != '\n')
      strcat(xbuf,"\n\r");

    if ( xbuf[strlen(xbuf)-1] == ' ' )
         xbuf[strlen(xbuf)-1] = '\0';
    if ( xbuf[strlen(xbuf)-1] == ' ' )
         xbuf[strlen(xbuf)-1] = '\0';

    free_string(oldstring);
    return(str_dup(xbuf));
}


/*
 * Replace a substring with a new substring, return the new version.
 * (make sure to str_dup() when needed)
 */
char *replace_string( char *orig, char *old, char *new )
{
    static char xbuf[MAX_STRING_LENGTH];
    int i;

    xbuf[0]      = '\0';
      
    strcpy(xbuf, orig);
    if ( strstr(orig,old) != NULL )
    {
         i        = strlen( orig ) - strlen( strstr(orig, old) );
         xbuf[i]  = '\0';
         strcat( xbuf, new);
         strcat( xbuf, &orig[i+strlen( old )] );
    }

    return ( xbuf );
}


/*
 * Does the same thing as above but returns a str_dup()'d version.
 */
char * string_replace( char * orig, char * old, char * new )
{
    char xbuf[MAX_STRING_LENGTH];
    int i;

    xbuf[0] = '\0';
    strcpy( xbuf, orig );
    if ( strstr( orig, old ) != NULL )
    {
        i = strlen( orig ) - strlen( strstr( orig, old ) );
        xbuf[i] = '\0';
        strcat( xbuf, new );
        strcat( xbuf, &orig[i+strlen( old )] );
        free_string( orig );
    }

    return str_dup( xbuf );
}

void replace_char( char orig[], char old, char new)
{
   while (*orig)
      if (*orig == old)
         *orig++ = new;
      else
         orig++;
}

/*
 * Counts the number of "words" (strings of non-spaces separated by spaces)
 * in a string.     Not the best way (should use one_arg).
 */
int arg_count( char *argument )
{
    int total;
    char *s;

    total = 0;
    s = argument;

    while ( *s != '\0' )
    {
        if ( *s != ' ' )
        {
            total++;
            while ( *s != ' ' && *s != '\0' )
                s++;
        }
        else
        {
            s++;
        }
    }

    return total;
}



/*
 * Returns a string with one space between each argument.
 */
char * string_unpad( char * argument )
{
    char buf[MAX_STRING_LENGTH];
    char *s;

    s = argument;

    while ( *s == ' ' )
        s++;

    strcpy( buf, s );
    s = buf;

    if ( *s != '\0' )
    {
        while ( *s != '\0' )
            s++;
        s--;

        while( *s == ' ' )
            s--;
        s++;
        *s = '\0';
    }

    free_string( argument );
    return str_dup( buf );
}



/*
 * First char is capitol, the rest is lowercase or something to that effect.
 */
char * string_proper( char * argument )
{
    char *s;

    s = argument;

    while ( *s != '\0' )
    {
        if ( *s != ' ' )
        {
            *s =
                 (*s == 't' && *(s+1) == 'h' && *(s+2) == 'e' && *(s+3) == ' ')   //the
              || (*s == 't' && *(s+1) == 'o' && *(s+2) == ' ')                    //to
              || (*s == 'a' && *(s+1) == ' ')                                     //a
              || (*s == 'b' && *(s+1) == 'y' && *(s+2) == ' ')                    //by
              || (*s == 'a' && *(s+1) == 't' && *(s+2) == ' ')                    //at
              || (*s == 'i' && *(s+1) == 'n' && *(s+2) == ' ')                    //in
              || (*s == 'a' && *(s+1) == 'n' && *(s+2) == ' ')                    //an
              || (*s == 'o' && *(s+1) == 'f' && *(s+2) == ' ') ? *s : UPPER(*s);  //of
            while ( *s != ' ' && *s != '\0' )
                s++;
        }
        else
        {
            s++;
        }
    }

    return capitalize(argument);
}




void string_edit( PLAYER *ch, char **pString )
{
    to_actor( " Entering line editing mode, terminate with a ~ or @ on a blank line.\n\r", ch );
    to_actor( "_________________________________________________________________________\n\r", ch );
    to_actor( "  0...5....A....E...|20...,....|30..,....|....,....|....,....|60..,..70|\n\r", ch );

    if ( *pString == NULL )
    {
        *pString = str_dup( "" );
    }
    else
    {
        free_string( *pString );
        *pString = str_dup( "" );
    }

    ch->desc->pString = pString;
    return;
}



void string_append( PLAYER *ch, char **pString )
{
    if ( ch->desc->connected <= NET_PLAYING )
    to_actor( "_________________________[Entering APPEND Mode]__________________________\n\r", ch );
    to_actor( " Type \\h on a new line for help, terminate with a ~ or @ on a blank line.\n\r", ch );
    to_actor( "_________________________________________________________________________\n\r", ch );
    to_actor( "  0...5....A....E...|20...,....|30..,....|....,....|....,....|60..,..70|\n\r", ch );

    ch->desc->pString = pString;
    return;
}
                                        

void string_add( PLAYER *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( *argument == '\\' || *argument == '.' )
    {
        char arg1 [MAX_INPUT_LENGTH];
        char arg2 [MAX_INPUT_LENGTH];
        char arg3 [MAX_INPUT_LENGTH];

        argument = one_argument( argument, arg1 );
        argument = first_arg( argument, arg2, FALSE );
        argument = first_arg( argument, arg3, FALSE );

        if ( !str_cmp( arg1, "\\c" ) 
          || !str_cmp( arg1, ".c" ) )
        {
            to_actor( "String cleared.\n\r", ch );
            free_string( *ch->desc->pString );
            *ch->desc->pString = str_dup( "" );
            return;
        }

/*
#if defined(unix)
        FILE *out;
        char *pf;

        out=fopen(".scenedesc","w");
        if ( *argument )
        fprintf(out,"%s\n", argument );
        else
        fprintf(out,"%s\n", *ch->desc->pString );
        fflush(out);
        fclose(out);

        out = popen( "ispell -a -S < .scenedesc", "r" );
        to_actor( "The following words are misspelled:\n\r", ch );

        while (fgets(buf, MAX_STRING_LENGTH-1, out) != NULL)
        {
            if ( buf[0] != '&' ) continue;
            pf = format_string( str_dup( buf ) );
            to_actor( pf, ch );
            free_string( pf );
        }
   
        pclose( out );
#else
        to_actor( "Unix systems only.\n\r", ch );
#endif
*/

        if ( !str_cmp( arg1, "\\p" ) 
          || !str_cmp( arg1, ".p" ) )
        {
#if defined(unix)
        FILE *out;
        char *pf;

        out=fopen(".scenedesc","w");
        if ( *argument )
        fprintf(out,"%s\n", argument );
        else
        fprintf(out,"%s\n", *ch->desc->pString );
        fflush(out);
        fclose(out);

        out = popen( "ispell -a -S < .scenedesc", "r" );
        to_actor( "The following words are misspelled:\n\r", ch );
        
        while (fgets(buf, MAX_STRING_LENGTH-1, out) != NULL)
        {
            if ( buf[0] != '&' ) continue;
            pf = format_string( str_dup( buf ) );
            to_actor( pf, ch );
            free_string( pf );
        }
            
        pclose( out );
#else
        to_actor( "Unix systems only.\n\r", ch );
#endif
        return;
        }

        if ( !str_cmp( arg1, "\\s" ) 
          || !str_cmp( arg1, ".s" ) )
        {
            to_actor( "String so far:\n\r", ch );
            to_actor( *ch->desc->pString, ch );
            return;
        }

        if ( !str_cmp( arg1, "\\r" ) 
          || !str_cmp( arg1, ".r" ) )
        {
            if ( arg2[0] == '\0' )
            {
                to_actor( "usage:  .r \"old string\" \"new string\"\n\r", ch );
                return;
            }

            *ch->desc->pString = string_replace( *ch->desc->pString, arg2, arg3 );
            snprintf( buf, MAX_STRING_LENGTH, "'%s' replaced with '%s'.\n\r", arg2, arg3 );
            to_actor( buf, ch );
            return;
        }

#if defined(TRANSLATE)
        if ( !str_cmp( arg1, "\\t" ) 
          || !str_cmp( arg1, ".t" ) ) {

             char *b = translate( ch->desc, *ch->desc->pString );
             int oldlingua = ch->desc->lingua;

             if ( arg2 != '\0' ) ch->desc->lingua = find_language( arg2 );
             free_string( *ch->desc->pString );
             *ch->desc->pString = b;
             ch->desc->lingua=oldlingua;
             to_actor( "Translated.\n\r", ch );
             return;
        }
#endif

        if ( ( !str_cmp( arg1, "\\x" ) || !str_cmp( arg1, ".x" ) )
             && IS_IMMORTAL(ch) ) {
             char buf2[MAX_STRING_LENGTH];

             sprintf( buf2, "../export/%s.txt", arg2 );
             save_string_to_file( buf2, *ch->desc->pString );
             to_actor( "File exported to ", ch );
             to_actor( buf2, ch ); to_actor( "\n\r", ch );
             return;
        }

        if ( ( !str_cmp( arg1, "\\i" ) || !str_cmp( arg1, ".i" ) )
             && IS_IMMORTAL(ch) ) {
             FILE *temp;
             if ( !(temp = fopen(arg2,"r")) ) { to_actor("File not found.\n\r",ch); return;}
             fclose(temp);
             to_actor( "String cleared.\n\r", ch );
             free_string( *ch->desc->pString );
             *ch->desc->pString = fread_file( arg2 );
             to_actor( "File imported.\n\r", ch );
             return;
        }

        if ( !str_cmp( arg1, "\\f" ) || !str_cmp( arg1, ".f" ) )
        {
            *ch->desc->pString = format_string( *ch->desc->pString );
            to_actor( "String formatted.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg1, "\\q" ) || !str_cmp( arg1, ".q" ) )
        {
            if ( IS_IMMORTAL(ch) ) cmd_zsave( ch, "cha" ) ;
            ch->desc->pString = NULL;
            if ( ch->desc->connected > NET_PLAYING ) 
              to_actor( "Press return to continue: ", ch );
            return;
        }
        
        if ( !str_cmp( arg1, "\\h" ) || !str_cmp( arg1, ".h" ) || !str_cmp( arg1, "help" ) )
        {       

            to_actor( "Editor help (commands available from the beginning of a new line only):  \n\r", ch );
            to_actor( "\\r 'old string' 'new string'   - replace a substring with a new string  \n\r", ch );
            to_actor( "\\m                             - spell check string (w/ unix spell)     \n\r", ch );
            to_actor( "\\h                             - get sedit help (this)                  \n\r", ch );
            to_actor( "\\s                             - show string so far                     \n\r", ch );
            to_actor( "\\f                             - format (word wrap) string              \n\r", ch );
            to_actor( "\\c                             - clear string so far                    \n\r", ch );
            to_actor( "\\p                             - check spelling of the entire string    \n\r", ch );
            to_actor( "\\p 'word'                      - check spelling of a word               \n\r", ch );
#if defined(TRANSLATE)
            to_actor( "\\t <language>                  - translate into a foriegn language\n\r", ch );
#endif
            if ( IS_IMMORTAL(ch) ) {
            to_actor( "\\x 'filename'                  - exports to export/filename.txt\n\r", ch );
            to_actor( "\\i 'filename'                  - imports from filename.txt\n\r", ch );
            }
            to_actor( "\\q              (also: @ or ~) - exit string editor (end string)        \n\r", ch );
            return;
        }

        if ( !str_cmp( arg1, "done" ) )
        {
          ch->desc->pString = NULL;
          if ( ch->desc->connected > NET_PLAYING ) 
            to_actor( "Press return to continue: ", ch );
          return;
        }


        to_actor( "String so far:\n\r", ch );
        to_actor( *ch->desc->pString, ch );
        return;
    }

    if ( *argument == '~' || *argument == '@' )
    {
        ch->desc->pString = NULL;
          if ( ch->desc->connected > NET_PLAYING ) 
            to_actor( "Press return to continue: ", ch );
        return;
    }

    snprintf( buf, MAX_STRING_LENGTH, "%s", *ch->desc->pString );

    if ( strlen( buf ) + strlen( argument ) >= ( MAX_STRING_LENGTH - 4 ) )
    {
        to_actor( "String too long, truncating.\n\r", ch );
        strncat( buf, argument, MAX_STRING_LENGTH );
        free_string( *ch->desc->pString );
        *ch->desc->pString = str_dup( buf );
        ch->desc->pString = NULL;
        return;
    }

    strcat( buf, argument );
    strcat( buf, "\n\r" );
    free_string( *ch->desc->pString );
    *ch->desc->pString = str_dup( buf );

    if ( IS_SET(ch->flag2,PLR_CLRSCR) ) {
        to_actor( CLRSCR, ch );
    to_actor( "_________________________________________________________________________\n\r", ch );
    to_actor( "  0...5....A....E...|20...,....|30..,....|....,....|....,....|60..,..70|\n\r", ch );        
        to_actor( *ch->desc->pString, ch );
    }
    return;
}



/*
 * Return true if an argument is completely numeric.
 */
bool is_number( char *arg )
{
    if ( *arg == '\0' )
        return FALSE;
        
    if ( *arg == '-' )
      {
       arg++;
       if ( !isdigit(*arg) ) return FALSE;
       arg++;
	 }           /* legalize your minus */
 
    for ( ; *arg != '\0'; arg++ )
      {
        if ( !isdigit(*arg) )
            return FALSE;
      }
 
    return TRUE;
}

/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
    char *pdot;
    int number;
    
    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
	if ( *pdot == '.' )
	{
	    *pdot = '\0';
	    number = atoi( argument );
	    *pdot = '.';
	    strcpy( arg, pdot+1 );
	    return number;
	}
    }

    strcpy( arg, argument );
    return 1;
}



/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;

    if ( argument == NULL ) return "";

    while ( isspace(*argument) || *argument == '\n' || *argument == '\r' )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
        if ( *argument == '\n' || *argument == '\r' ) break;
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = LOWER(*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}





/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.    No case manipulations.
 */
char *one_argcase( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' && *argument != '\n' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = *argument;
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}


char * strupr( char * s )
{
   char * u;

   for( u = s; *u != '\0'; u++ )
   {
       *u = UPPER(*u);
   }

   return s;
}

char * strlwr( char * s )
{
   char * u;

   for( u = s; *u != '\0'; u++ )
   {
       *u = LOWER(*u);
   }

   return s;
}


/*
 * Take a string like <[3m<[1mhi and make it <[3m<[1mHi
 * Thanx to Arcane@max.tiac.net from TIACMUD
 * Changes made by Locke.
 */
char *ansi_uppercase(char *txt)
{
  char *str;

  str = txt;

  while ( *str )
  {
  if ( *str == ESC )  str++;
     else break;
  if ( *str == '[' )  str++;
     else break;
  while( isdigit(*str) ) str++;
  if ( *str == 'm' )  str++;
     else break;
  }

  *str = UPPER(*str);

  return txt;
}


/*
 * See if a string is one of the names of a prop.
 */
bool is_name( const char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];
    char *p;

    if ( str == NULL || namelist == NULL ) return FALSE;

    if ( strlen(str) <= 3 ) {

    p = namelist;
    for ( ; ; )
    {
	p = one_argument( p, name );
	if ( name[0] == '\0' )
	    return FALSE;
    if ( !str_cmp( str, name ) )            /* str_cmp for full matching */
	    return TRUE;
    }

    p = namelist;
    for ( ; ; )
    {
	p = one_argument( p, name );
	if ( name[0] == '\0' )
	    return FALSE;
    if ( !str_cmp( str, name ) )            /* str_cmp for full matching */
	    return TRUE;
    }
    }
    else {

    p = namelist;
    for ( ; ; )
    {
	p = one_argument( p, name );
	if ( name[0] == '\0' )
	    return FALSE;
    if ( !str_infix( str, name ) )            
	    return TRUE;
    }

    p = namelist;
    for ( ; ; )
    {
	p = one_argument( p, name );
	if ( name[0] == '\0' )
	    return FALSE;
    if ( !str_infix( str, name ) )  
	    return TRUE;
    }
    }
}


/*
 * See if a string is one of the keywords in a string.
 * Checks as prefixes.
 */
bool is_prename( const char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];

    if ( str == NULL || namelist == NULL ) return FALSE;

    for ( ; ; )
    {
	namelist = one_argument( namelist, name );
	if ( name[0] == '\0' )
	    return FALSE;
    if ( !str_prefix( str, name ) )
	    return TRUE;
    }
}


/*
 * Take a number like 43 and make it forty-three.
 */
char *   const  ones_numerals [10] =
{
    "",
    "one",
    "two",
    "three",
    "four",
    "five",
    "six",
    "seven",
    "eight",
    "nine"
};

char *   const  tens_numerals [10] =
{
    "",
    "",
    "twenty",
    "thirty",
    "forty",
    "fifty",
    "sixty",
    "seventy",
    "eighty",
    "ninety"
};

char *   const  meta_numerals [4] =
{
    "hundred",
    "thousand",
    "million",
    "billion"
};

char *   const  special_numbers [10] =
{
    "ten",
    "eleven",
    "twelve",
    "thirteen",
    "fourteen",
    "fifteen",
    "sixteen",
    "seventeen",
    "eighteen",
    "nineteen"
};



      
char *numberize( int n )
{
    static char buf[MAX_STRING_LENGTH];
    short int    digits[3];
    int t = abs(n);
        
    buf[0] = '\0';
        
    /*
     * Special cases numbers
     */
    if ( n == 0 )
    {
        snprintf( buf, MAX_STRING_LENGTH, "zero" );
        return buf;
    } else
    if ( n >= 10 && n <= 19 )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%s", special_numbers[n-10] );
        return buf;
    } else     
    if ( n < 10 && n >= 0 )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%s", ones_numerals[n] );
        return buf;
    } else
    if ( n >= 20 && n < 100 ) {
        if ( n % 10 == 0 ) {        
           snprintf( buf, MSL, "%s", tens_numerals[n/10] );
           return buf;
        } else {
           snprintf( buf, MSL, "%s-%s", tens_numerals[n/10], ones_numerals[n%10] );
           return buf;
        }
    } else
    {
        snprintf( buf, MAX_STRING_LENGTH, "%d", n );
        return buf;
    }
     
}

char *numberize_old( int n )
{
    static char buf[MAX_STRING_LENGTH];
    int    digits[3];
    int t = abs(n);

    buf[0] = '\0';

    /*
     * Special cases (10-19)
     */
    if ( n >= 10 && n <= 19 )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%s", special_numbers[n-10] );
        return buf;
    }

    if ( n < 10 && n >= 0 )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%s", ones_numerals[n] );
        return buf;
    }

    /*
     * Cha.
     */
    if ( n >= 10000 || n < 0 )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%d", n );
        return buf;
    }


    digits[3] = t / 1000;
    t -= 1000*digits[3];
    digits[2] = t / 100;
    t -= 100*digits[2];
    digits[1] = t / 10;
    t -= 10*digits[1];
    digits[0] = t;

    if ( digits[3] > 0 )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%s%s", buf, ones_numerals[digits[3]] );
        snprintf( buf, MAX_STRING_LENGTH, "%s thousand ", buf );
    }

    if ( digits[2] > 0 )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%s%s", buf, ones_numerals[digits[2]] );
        snprintf( buf, MAX_STRING_LENGTH, "%s hundred ", buf );
    }

    if ( digits[1] > 0 )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%s%s", buf, tens_numerals[digits[1]] );
        if ( digits[0] > 0 ) snprintf( buf, MAX_STRING_LENGTH, "%s-", buf );
    }

    if ( digits[0] > 0 )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%s%s", buf, ones_numerals[digits[0]] );
    }

    if ( buf[(t = strlen(buf)-1)] == ' ' )  buf[t] = '\0';
    if ( buf[(t = strlen(buf)-1)] == ' ' )  buf[t] = '\0';
    return buf;
}





/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes, parenthesis (barring ()'s) and percentages.
 */
char *first_arg( char *argument, char *arg_first, bool fCase )
{
    char cEnd;

    while ( *argument == ' ' )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"'
      || *argument == '%'  || *argument == '(' )
    {
        if ( *argument == '(' )
        {
            cEnd = ')';
            argument++;
        }
        else cEnd = *argument++;
    }

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
    if ( fCase ) *arg_first = LOWER(*argument);
            else *arg_first = *argument;
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( *argument == ' ' )
	argument++;

    return argument;
}



/*
 * Skips forward to the next non-space.
 */
char *skip_spaces( char *argument )
{
    while ( *argument == ' ' )  argument++;

    return argument;
}



/*
 * Takes the top line off a multi-line string, returns a pointer to the next.
 */
char *grab_line( char *argument, char *arg_first )
{
    while ( *argument != '\n' && *argument != '\0' )
    *arg_first++ = *argument++;

    if ( *argument == '\n' ) argument++;
    if ( *argument == '\r' ) argument++;

    *arg_first++ = '\0';

    return argument;
}



/*
 * Takes the top line off a multi-line string, returns a pointer to the next.
 */
char *one_line( char *argument, char *arg_first )
{
    while ( *argument != '\0' )
    {
        if ( *argument == ';'  )
        {
            argument++;
            break;
        }

        /*
         * Skip comments.
         */
        if ( *argument == '*' )
        while ( *argument != '\n' && *argument != '\0' ) argument++;

        /*
         * Jump past literals.
         */
        if ( *argument == '{' )
        {
            int curlies = 0;

            *arg_first++ = *argument++;
            while( *argument != '\0' )
            {

                /*
                 * Ok, get the hell out of here, we're done.
                 * { .... }
                 *        ^
                 */
                if ( curlies == 0 )
                   if ( *argument == '}'
                     || *argument == '\0' )
                     break;

                if ( *argument == '{' ) curlies++;
                else
                if ( *argument == '}' ) curlies--;

                *arg_first++ = *argument++;
            }

            if ( curlies != 0 )
            NOTIFY("Notify>  One_line: Unmatched {}s in literal.", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT);

            if ( *argument == '}' ) *arg_first++ = *argument++;
        }
        else
        if ( *argument == '\n' || *argument == '\r' ) argument++;
        else
        *arg_first++ = *argument++;
    }

    *arg_first = '\0';
    return argument;
}



bool has_arg( char *argument, char *item )
{
    char arg[MAX_STRING_LENGTH];

    while ( *argument != '\0' )
    {
        argument = first_arg( argument, arg, TRUE );
        if ( !str_cmp( arg, item ) )
        return TRUE;
    }

    return FALSE;
}



char *trunc_fit( char *argument, int length )
{
    static char buf[MAX_STRING_LENGTH];
    int x;

    if ( argument == NULL )
    return argument;

    for ( x = 0;  (x < length) && (*argument != '\0');  x++ )
    {
        buf[x] = *argument;
        argument++;
    };
    buf[x] = '\0';

    return buf;
};



char *gotoxy( int x, int y )
{
    static char buf[30];

    snprintf( buf, MAX_STRING_LENGTH, "\x1b[%d;%dH", y, x );
    return buf;
};



char *skip_curlies( char *arg )
{
    char *p = arg;
    int curlies = 0;

    while( *arg != '\0' && *arg != '{' ) arg++;

    if ( *arg == '\0' ) return p;

    while( *arg != '\0' )
    {
        /*
         * Ok, get the hell out of here, we're done.
         * { .... }
         *        ^
         */
        if ( curlies == 0 )
        {
           if ( *arg == '}'
             || *arg == '\0' )
             break;
        }

        if ( *arg == '{' ) curlies++;
        else
        if ( *arg == '}' ) curlies--;

        arg++;
    }

    return arg;
}



char *strip_curlies( char *arg )
{
    static char buf[MAX_STRING_LENGTH];
    char *p;
    int curlies = 0;

    snprintf( buf, MAX_STRING_LENGTH, "%s", arg );
    arg = skip_spaces( arg );
    if ( *arg++ != '{' ) return buf;

    p = buf;
    while( *arg != '\0' )
    {
        /*
         * Ok, get the hell out of here, we're done.
         * { .... }
         *        ^
         */
        if ( curlies == 0 )
        {
           if ( *arg == '}'
             || *arg == '\0' )
             break;
        }

        if ( *arg == '{' ) curlies++;
        else
        if ( *arg == '}' ) curlies--;

        *p++ = *arg++;
    }

    *p = '\0';
    return buf;
}




/*
 * Skip ahead spaces and grab from first char to \n
 */
char *grab_to_eol( char *argument, char *arg_first )
{
    char cEnd;

    if ( argument == NULL ) return "";

    while ( isspace(*argument) )
	argument++;

    cEnd = '\n';
    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = LOWER(*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) && ( *argument == '\n' || *argument =='\r' ) )
	argument++;

    return argument;
}


/*
 * Skip ahead spaces and grab from first char to \n
 */
char *grab_to_at( char *argument, char *arg_first )
{
    char cEnd;

    if ( argument == NULL ) return "";

    while ( isspace(*argument) )
	argument++;

    cEnd = '@';
    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = LOWER(*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) && ( *argument == '\n' || *argument =='\r' ) )
	argument++;

    return argument;
}
