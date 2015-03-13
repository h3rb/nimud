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

/*
 * Color modes.
 */
#define MODE_GREY  0
#define MODE_ANSI  1
#define MODE_MXP   2

/*
 * Thin and thick box drawing styles for box()
 */
#define BOX_THIN   0
#define BOX_THICK  1

/*.oOXbvjcDFefg.*/

#define stroke(i)             stroke = i
#define background(i)         bgcolor = i
#define SETPIXEL(x,y,c)       pages[x+(y*MAX_Y)][page] = c 
#define GETPIXEL(x, y)        pages[x+(y*MAX_Y)][page]
#define brush(c)              paintbrush = c
#define get(x, y)             pages[x+(y*MAX_Y)][page]
#define set(x, y)             point(x, y)

