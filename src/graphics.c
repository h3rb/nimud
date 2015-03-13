/******************************************************************************
 * Locke's   __                      __         NIM Server Software           *
 * ___ ___  (__)__    __ __   __ ___|  | v5     Version 5 (ALPHA)             *
 * |  /   \  __|  \__/  |  | |  |      |        unreleased+revamped 2004      *
 * |       ||  |        |  \_|  | ()   |                                      *
 * |    |  ||  |  |__|  |       |      |                                      *
 * |____|__|___|__|  |__|\___/__|______|        http://www.nimud.org/nimud    *
 *   n a m e l e s s i n c a r n a t e          dedicated to chris cool       * 
 ******************************************************************************
 * Nimud Ascii Graphics Engine      copyright (c) 2005 Herb Gilliland - all   *
 * NAGE: version 0.6a                    rights reserved.  See liscense.txt   *
 ******************************************************************************/

/*.oOXbvjcDFefg.*/

/*
 * Locke's ANSI/ASCII Graphics routines.
 * Part of NiM 5
 */


#if defined(BSD)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <zlib.h>

#include "nimud.h"
#include "defaults.h"
#include "graphics.h"


/* "\x1b[#,#H"
   "\x1b[#,#f"  move cursor
   "\x
 */
#define CLEAR_SCR "\x1b[2J"
#define CLEAR_EOL "\x1b[K"
#define SAVE_CURS "\x1b[s"
#define LOAD_CURS "\x1b[u"

/*
 * Ansi color table with HSB code equivalents for MXP.
 */
const   struct  color color_table [] =
{
   { "\x1b[30m",     "$R$0",   "black",      "#000000", "^0" },
   { "\x1b[34m",     "$R$1",   "indigo",     "#000033", "^1" },
   { "\x1b[32m",     "$R$2",   "green",      "#007700", "^2" },
   { "\x1b[36m",     "$R$3",   "aquamarine", "#333300", "^3" },
   { "\x1b[31m",     "$R$4",   "crimsin",    "#770000", "^4" },
   { "\x1b[35m",     "$R$5",   "purple",     "#330033", "^5" },
   { "\x1b[33m",     "$R$6",   "orange",     "#006666", "^6" },
   { "\x1b[37m",     "$R$7",   "grey",       "#777777", "^7" },
   { "\x1b[30;1m",   "$B$0",   "charcoal",   "#000066", "^B^0" },
   { "\x1b[34;1m",   "$B$1",   "blue",       "#000077", "^B^1" },
   { "\x1b[32;1m",   "$B$2",   "lime",       "#880000", "^B^2" },
   { "\x1b[36;1m",   "$B$3",   "cyan",       "#330055", "^B^3" },
   { "\x1b[31;1m",   "$B$4",   "red",        "#FF0000", "^B^4" },
   { "\x1b[35;1m",   "$B$5",   "magenta",    "#FF00FF", "^B^5" },
   { "\x1b[33;1m",   "$B$6",   "yellow",     "#00FFFF", "^B^6" },
   { "\x1b[37;1m",   "$B$7",   "white",      "#FFFFFF", "^B^7" },
   { "\x1b[1m",      "$B",     "bold",       "#FFFFFF", "^B" },
   { "\x1b[5m",      "$F",     "flashing",   "#A6A6A6", "^F" },
   { "\x1b[7m",      "$I",     "inverse",    "#000088", "^I" },
   { "\x1b[0m",      "$R",     "normal",     "#777777", "^N" },
   { " ",            " ",      " ",          " " }
};



/*
 * Pointer to active page.
 */

int mode=MODE_ANSI;
int page=0;
char paintbrush='X';

char pages[MAX_X*MAX_Y][4];  /* 0, 1, 2, 3 */
int  cpages[MAX_X*MAX_Y][4];  /* 0, 1, 2, 3 */

int stroke;
int bgcolor;
int style=BOX_THIN;
int ruler=TRUE;

int width =MAX_X;
int height=MAX_Y;


/* to be added: ascii true-type fonts support */

/*
 * Called at the beginning of each cycle of nage.
 */
void gmode( int x, int y ) {
    width = URANGE(0,x,MAX_X);
    height = URANGE(0,y,MAX_Y);
}


void clear_page( ) {
     int x, y;

     for ( x = 0; x < MAX_X; x++ )
      for ( y = 0; y < MAX_Y; y++ ) {
       pages[x+(y*width)][page] = ' ';
      cpages[x+(y*width)][page] = 0;
     }
}


/*
 * Draws text on a page at x,y.
 */
void text(int x, int y, char *arg ) {
     char oldpb = paintbrush;
     char *p;

     p = arg;
     while ( *p != '\0' ) {
        paintbrush = *p++;
        point( x++, y );
     }

     paintbrush = oldpb;
}



char *color( int c ) {

    static char buf[MSL];

    if ( mode == MODE_ANSI ) {
        strcpy( buf, color_table[c%16].di );
        return buf;
    } 
    else if ( mode == MODE_MXP ) {
        /* hex conversions */
    }

    return "^N";
}


/*
 * Drops current page to client.
 */
void draw_to_buffer( PLAYER *ch ) {
     int x, y;
     int lastStroke =0;

     if ( !IS_SET(ch->flag2,PLR_NAGE) ) return;

     clrscr( ch );

     if ( ruler ) 
text(0,0, "0...5....A....E...|20...,....|30..,....|....,....|....,....|60..,....|...77|");

     /*
      * Safeguard.
      */
     page = URANGE(0,page,3);
     width = URANGE(0,width,MAX_X);
     height = URANGE(0,height,MAX_Y);

     for ( y = 0; y < height; y++ )
     {
     for ( x = 0; x < width; x++ ) {

          /* avoid repeated foreground color bashings */
          if ( lastStroke != cpages[x+(y*width)][page] ) {
          lastStroke = cpages[x+(y*width)][page];
          to_actor( color( cpages[x+(y*width)][page] ), ch );
          }

          /* add check to end of line for spaces here to decrease output */

          to_actor( &pages[x+(y*width)][page], ch ); 
     }
          to_actor( "\n\r", ch );
     }
     return;
}

/*
 * Draws a point with the current paintbrush.
 */
void point( int x, int y ) {
     pages[URANGE(0,x+(y*width),MAX_X*MAX_Y)][page] = paintbrush;
    cpages[URANGE(0,x+(y*width),MAX_X*MAX_Y)][page] = stroke;
}

/*
 * Draws a rectangle at x,y to dx,dy (corners)
 * using the current paintbrush.
 *  x,y
 *    xxxxxxxxxx
 *    x        x  <- on a page
 *    xxxxxxxxxx
 *            dx,dy
 */
void rect( int x, int y, int dx, int dy ) {
     x = URANGE(0,x,width);
     y = URANGE(0,y,height);

     dx = URANGE(0,dx,width);
     dy = URANGE(0,dy,height);
     
     point( x,  y  );
     point( dx, dy );
     point( x,  dy );
     point( dx, y  );

     hline( x+1,  y, (dx-x)-1 );  /* top        */
     vline( x+1, dy, (dx-x)-1 );  /* bottom     */
     vline(   x,  y, (dy-y)-1 );  /* left side  */
     vline(  dx,  y, (dy-y)-1 );  /* right side */
}



/*
 * Draws a box (ex:linestyle thin):
 *  x,y
 *    +--------+
 *    |        |  <- on a page
 *    +--------+
 *            dx,dy
 */
void box( int x, int y, int dx, int dy ) {
     char oldpb = paintbrush;

     x = URANGE(0,x,width);
     y = URANGE(0,y,height);

     dx = URANGE(0,dx,width);
     dy = URANGE(0,dy,height);

     paintbrush = style == BOX_THICK ? '=' : '-';
     hline( x,   y, (dx-x) ); /* top    */
     hline( x,  dy, (dx-x) ); /* bottom */

     paintbrush = style == BOX_THIN ? '|' : '|';
     vline(  x, y, (dy-y) );          /* left side  */
     vline( dx, y, (dy-y) );          /* right side */
     
     paintbrush = style == BOX_THIN ? '+' : '*';
     point( x,  y  );
     point( dx, dy );
     point( x,  dy );
     point( dx, y  );

     paintbrush = oldpb;
}


/*
 * Fills a region with the current paintbrush.
 */
void fill( int x, int y, int dx, int dy ) {
     int sx,sy,ex,ey;

     sx = URANGE(0,x,MAX_X);
     sy = URANGE(0,y,MAX_Y);
     ex = URANGE(0,dx,MAX_X);
     ey = URANGE(0,dy,MAX_Y);

     for ( ; sx < ex; sx++ )
       for ( ; sy < ey; sy++ ) point(sx,sy);
}


/*
 * Writes a string from top left (x,y) to lower right
 */
void fills( int x, int y, char *c ) {
    char oldpb = paintbrush;
    int dx,dy=URANGE(0,y,MAX_Y);

    while ( *c != '\0' && dy < MAX_Y ) {

     for ( dx=URANGE(0,x,MAX_X); dx < MAX_X && *c != '\n'; dx++ )
       {
           paintbrush=*c; c++;
           point(dx,dy);
       }

       if ( *c == '\n' ) c++;
       else
       if ( *c == '\r' ) c++;
       else dy++;
    }
    paintbrush = oldpb;
}

/*
 * Truncates a string to fit a region.
 */
void column( int x, int y, int w, int h, char *c ) {
    char oldpb= paintbrush;

    w = URANGE(0,x+w,MAX_X);
    h = URANGE(0,y+h,MAX_Y);

    while ( *c != '\0' && y < w ) {
     for ( x=URANGE(0,x,MAX_X); x < MAX_X && *c != '\n'; x++ )
       {
           paintbrush=*c;
           point(x,y);
       }
       c++;

       if ( *c == '\n' ) { c++; y++; }
       if ( *c == '\r' ) c++;
    }       
    paintbrush = oldpb;
}

/*
 * Wraps a string to fit a certain width.
 */
char *wrap( int w, char *c, char *d ) {
     return format_indent( c, d, w, TRUE ); 
}


/*
 * Draws a button at x,y with label c.
 * Add MXP-link support here.
 *
 *  x,y
 *    +---------+
 *    | text(c) |  <- on a page
 *    +---------+
 *            dx+strlen(c)+2,dy+3
 */
void button( int x, int y, char *c ) {
     box(x,y,x+strlen(c)+2,y+3);
     text(x+2,y+1,c);
}

/*
 * Draws a horizontal line with current paintbrush.
 *  x,y
 *    xxxxxxxxxx
 *             x+w,y
 */
void hline( int x, int y, int w ) {
     int hx, sx, ex;
    
     sx = URANGE(0,x,width);
     ex = URANGE(0,x+w,width);
     y = URANGE(0,y,height);

     for ( hx = sx;  hx < ex;  hx++ ) point( hx, y );
}


/*
 * Draws a horizontal line with current paintbrush.
 *  x,y
 *    x
 *    x 
 *    x x,y+h
 */
void vline( int x, int y, int h ) {
     int hy, sy, ey;
    
     sy = URANGE(0,y,height);
     ey = URANGE(0,y+h,height);
     x = URANGE(0,x,width);

     for ( hy = sy;  hy < ey;  hy++ ) point( x, hy );
}


/*
 * Draws a pop-up box with a title.
 */
void popup( int x, int y, int dx, int dy, char *title ) {
     char oldpb = paintbrush;
     x = URANGE(0,x,width);
     y = URANGE(0,y,height);
     int w,h,len=strlen(title);

     dx = URANGE(0,dx,width);
     dy = URANGE(0,dy,height);
     w=dx-x;  if ( w < len+2 ) w=len+2;
     h=dy-y;

     paintbrush = '_';
     hline( x,   y, w ); /* top    */
     paintbrush = style == BOX_THICK ? '=' : '-';
     hline( x,  dy, w ); /* bottom */

     paintbrush = style == BOX_THIN ? '|' : '|';
     vline(  x, y, h );          /* left side  */
     vline( dx, y, h );          /* right side */
     
     paintbrush = ' ';
     point(  x,  y  );
     point( dx,  y  );
     paintbrush = style == BOX_THIN ? '+' : '*';
     point( dx, dy  );
     point(  x, dy  );

     text( x+( (len % 2 == 1) ? 1+((w-len)/2)
                              : (w-len)/2 ), 
           y, title );
     paintbrush = oldpb;
}




/*
 * Circle drawing routine; using 8-way symmetry
 */
void CirclePoint(int cx,int cy,int x,int y)
{ if(x==0)
  { 
  set(cx,cy+y);
  set(cx,cy-y);
  set(cx+y,cy);
  set(cx-y,cy);
  }
  else if(x==y)
  { 
  set(cx+x,cy+y);
  set(cx-x,cy+y);
  set(cx+x,cy-y);
  set(cx-x,cy-y);
  }
  else if(x<y)
  {
  set(cx+x,cy+y);
  set(cx-x,cy+y);
  set(cx+x,cy-y);
  set(cx-x,cy-y);
  set(cx+y,cy+x);
  set(cx-y,cy+x);
  set(cx+y,cy-x);
  set(cx-y,cy-x);
  }
}

void circle( int xcenter, int ycenter, int radius )
{
  int x=0;
  int y=radius;
  int p=(5-radius*4)/4;
  CirclePoint(xcenter,ycenter,x,y);
  while(x<y)
  { x++;
    if(p<0)
     {p+=2*x+1;
     }
    else{y--;p+=2*(x-y)+1;}
    CirclePoint(xcenter,ycenter,x,y);
  }
}







 /* 
  * This routine is just a simpler version of a line approximation.
  */
    void textline(int x0, int y0, int x1, int y1, char *arg )
    {
        int dx = x1 - x0;
        int dy = y1 - y0;

        set( x0, y0);
        if (dx != 0) {
            float m = (float) dy / (float) dx;
            float b = y0 - m*x0;
            dx = (x1 > x0) ? 1 : -1;
            while (x0 != x1) {
                x0 += dx;
                y0 = (int)(m*x0 + b);
                if ( *arg != '\0' ) { paintbrush = *arg; arg++; }
                set( x0, y0);
            }
        }
    }





/*
 * Life After Bresenham : Most books would have you believe that the 
 * development of line drawing algorithms ended with Bresenham's famous 
 * algorithm. But there has been some signifcant work since then. The 
 * following 2-step algorithm, developed by Xiaolin Wu, is a good example. 
 *
 * The interesting story of this algorithm's development is discussed in an 
 * article that appears in Graphics Gems I by Brian Wyvill.
 *
 * The two-step algorithm takes the interesting approach of treating line 
 * drawing as a automaton, or finite state machine. If one looks at the 
 * possible configurations that the next two pixels of a line, it is easy to 
 * see that only a finite set of possiblities exist.
 *
 * [Omitted: Possible configurations of the next two pixels, See Below]
 *
 * The two-step algorithm shown here also exploits the symmetry of 
 * line-drawing by simultaneously drawn from both ends towards the 
 * midpoint.
 *
 */
 
void line(int x0, int y0, int x1, int y1)
    {
        int dy = y1 - y0;
        int dx = x1 - x0;
        int stepx, stepy;
        int i;

        if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
        if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }

        set( x0, y0);
        set( x1, y1);
        if (dx > dy) {
            int length = (dx - 1) >> 2;
            int extras = (dx - 1) & 3;
            int incr2 = (dy << 2) - (dx << 1);
            if (incr2 < 0) {
                int c = dy << 1;
                int incr1 = c << 1;
                int d =  incr1 - dx;
                for ( i = 0; i < length; i++) {
                    x0 += stepx;
                    x1 -= stepx;
                    if (d < 0) {		// Pattern:
                        set( x0, y0);		//
                        set( x0 += stepx, y0);	//  x o o
                        set( x1, y1);		//
                        set( x1 -= stepx, y1);
                        d += incr1;
                    } else {
                        if (d < c) {				// Pattern:
                            set(x0, y0);			//      o
                            set(x0 += stepx, y0 += stepy);	//  x o
                            set( x1, y1);			//
                            set( x1 -= stepx, y1 -= stepy);
                        } else {
                            set( x0, y0 += stepy);	// Pattern:
                            set( x0 += stepx, y0);	//    o o 
                            set( x1, y1 -= stepy);	//  x
                            set( x1 -= stepx, y1);	//
                        }
                        d += incr2;
                    }
                }
                if (extras > 0) {
                    if (d < 0) {
                        set( x0 += stepx, y0);
                        if (extras > 1) set( x0 += stepx, y0);
                        if (extras > 2) set( x1 -= stepx, y1);
                    } else
                    if (d < c) {
                        set( x0 += stepx, y0);
                        if (extras > 1) set( x0 += stepx, y0 += stepy);
                        if (extras > 2) set( x1 -= stepx, y1);
                    } else {
                        set( x0 += stepx, y0 += stepy);
                        if (extras > 1) set( x0 += stepx, y0);
                        if (extras > 2) set( x1 -= stepx, y1 -= stepy);
                    }
                }
            } else {
                int c = (dy - dx) << 1;
                int incr1 = c << 1;
                int d =  incr1 + dx;
                for ( i = 0; i < length; i++) {
                    x0 += stepx;
                    x1 -= stepx;
                    if (d > 0) {
                        set( x0, y0 += stepy);		// Pattern:
                        set( x0 += stepx, y0 += stepy);	//      o
                        set( x1, y1 -= stepy);		//    o
                        set( x1 -= stepx, y1 -= stepy);	//  x
                        d += incr1;
                    } else {
                        if (d < c) {
                            set( x0, y0);			// Pattern:
                            set( x0 += stepx, y0 += stepy);     //      o
                            set( x1, y1);                       //  x o
                            set( x1 -= stepx, y1 -= stepy);     //
                        } else {
                            set( x0, y0 += stepy);	// Pattern:
                            set( x0 += stepx, y0);	//    o o
                            set( x1, y1 -= stepy);	//  x
                            set( x1 -= stepx, y1);	//
                        }
                        d += incr2;
                    }
                }
                if (extras > 0) {
                    if (d > 0) {
                        set( x0 += stepx, y0 += stepy);
                        if (extras > 1) set( x0 += stepx, y0 += stepy);
                        if (extras > 2) set( x1 -= stepx, y1 -= stepy);
                    } else
                    if (d < c) {
                        set( x0 += stepx, y0);
                        if (extras > 1) set( x0 += stepx, y0 += stepy);
                        if (extras > 2) set( x1 -= stepx, y1);
                    } else {
                        set( x0 += stepx, y0 += stepy);
                        if (extras > 1) set( x0 += stepx, y0);
                        if (extras > 2) {
                            if (d > c)
                                set( x1 -= stepx, y1 -= stepy);
                            else
                                set( x1 -= stepx, y1);
                        }
                    }
                }
            }
        } else {
            int length = (dy - 1) >> 2;
            int extras = (dy - 1) & 3;
            int incr2 = (dx << 2) - (dy << 1);
            if (incr2 < 0) {
                int c = dx << 1;
                int incr1 = c << 1;
                int d =  incr1 - dy;
                for ( i = 0; i < length; i++) {
                    y0 += stepy;
                    y1 -= stepy;
                    if (d < 0) {
                        set( x0, y0);
                        set( x0, y0 += stepy);
                        set( x1, y1);
                        set( x1, y1 -= stepy);
                        d += incr1;
                    } else {
                        if (d < c) {
                            set( x0, y0);
                            set( x0 += stepx, y0 += stepy);
                            set( x1, y1);
                            set( x1 -= stepx, y1 -= stepy);
                        } else {
                            set( x0 += stepx, y0);
                            set( x0, y0 += stepy);
                            set( x1 -= stepx, y1);
                            set( x1, y1 -= stepy);
                        }
                        d += incr2;
                    }
                }
                if (extras > 0) {
                    if (d < 0) {
                        set( x0, y0 += stepy);
                        if (extras > 1) set( x0, y0 += stepy);
                        if (extras > 2) set( x1, y1 -= stepy);
                    } else
                    if (d < c) {
                        set( stepx, y0 += stepy);
                        if (extras > 1) set( x0 += stepx, y0 += stepy);
                        if (extras > 2) set( x1, y1 -= stepy);
                    } else {
                        set( x0 += stepx, y0 += stepy);
                        if (extras > 1) set( x0, y0 += stepy);
                        if (extras > 2) set( x1 -= stepx, y1 -= stepy);
                    }
                }
            } else {
                int c = (dx - dy) << 1;
                int incr1 = c << 1;
                int d =  incr1 + dy;
                for ( i = 0; i < length; i++) {
                    y0 += stepy;
                    y1 -= stepy;
                    if (d > 0) {
                        set( x0 += stepx, y0);
                        set( x0 += stepx, y0 += stepy);
                        set( x1 -= stepy, y1);
                        set( x1 -= stepx, y1 -= stepy);
                        d += incr1;
                    } else {
                        if (d < c) {
                            set( x0, y0);
                            set( x0 += stepx, y0 += stepy);
                            set( x1, y1);
                            set( x1 -= stepx, y1 -= stepy);
                        } else {
                            set( x0 += stepx, y0);
                            set( x0, y0 += stepy);
                            set( x1 -= stepx, y1);
                            set( x1, y1 -= stepy);
                        }
                        d += incr2;
                    }
                }
                if (extras > 0) {
                    if (d > 0) {
                        set( x0 += stepx, y0 += stepy);
                        if (extras > 1) set( x0 += stepx, y0 += stepy);
                        if (extras > 2) set( x1 -= stepx, y1 -= stepy);
                    } else
                    if (d < c) {
                        set( x0, y0 += stepy);
                        if (extras > 1) set( x0 += stepx, y0 += stepy);
                        if (extras > 2) set( x1, y1 -= stepy);
                    } else {
                        set( x0 += stepx, y0 += stepy);
                        if (extras > 1) set( x0, y0 += stepy);
                        if (extras > 2) {
                            if (d > c)
                                set( x1 -= stepx, y1 -= stepy);
                            else
                                set( x1, y1 -= stepy);
                        }
                    }
                }
            }
        }
    }



void goto_xy( PLAYER *ch, int x, int y )
{

   char buf[MAX_STRING_LENGTH];

   sprintf( buf, "\x1b[%d,%dH", x, y );
/* sprintf( buf, "\x1b[%d,%df", x, y ); */
   to_actor( buf, ch ); 
}

/*
int horizline( PLAYER *ch, int length, char c ) {

     for ( ; length > 0; length-- )
     to_actor( c, ch ); 
}

int vertline( PLAYER *ch, int length, char c ) {
     for ( ; length > 0; length++ ) {
     to_actor( c, ch )
     to_actor( "\x1b[1A", ch ); 
     }
}
*/

/**************************************************
 * See Also: The mapping command (map.c) uses AGE *
 **************************************************/
/*
 * Draws an ascii map (rough) of the 2d plane on 
 * which the player is standing.  Great for large 
 * areas, saves those who don't have clients to 
 * automap.
 *
 * Not yet implemented,
 * Players can copy a map onto a piece of paper.
 */
