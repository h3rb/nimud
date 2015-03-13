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
 * To use any part of NiMUD, you must comply with the Merc, Diku and NiMUD    *
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


/*
 * This code is used for number and string list manipulations.
 * It lets you generate strings based on lists of strings,
 * do checks in a list, or add / remove elements from a list.
 *
 * This is for storing stacks of strings or numbers.
 */

NUMBER_LIST *add_num_nodups( NUMBER_LIST **nlist, int num )
{
    NUMBER_LIST *pNum;
 
    if ( num_in_list( *nlist, num ) ) return NULL;
    
    pNum = new_number_list( );
    pNum->num  = num;
    pNum->next = *nlist;
    return pNum;     
}     


NUMBER_LIST *add_num_list( NUMBER_LIST **nlist, int num )
{
    NUMBER_LIST *pNum;

    pNum = new_number_list( );
    pNum->num = num;
    pNum->next = *nlist;
    return pNum;     
}


NUMBER_LIST *rem_num_list( NUMBER_LIST **nlist, int num )
{
    return *nlist;
}

bool num_in_list( NUMBER_LIST *nlist, int num )
{
     NUMBER_LIST *pNum;

     for( pNum = nlist; pNum != NULL;  pNum = pNum->next )
     {
         if ( pNum->num == num ) return TRUE; 
     }      

     return FALSE;
}
  
/*
 * This code is used for number and string list manipulations.
 * It lets you generate strings based on lists of strings,
 * do checks in a list, or add / remove elements from a list.
 */

STRING_LIST *add_str_nodups( STRING_LIST **nlist, char *str )
{
    STRING_LIST *pStr;
 
    if ( str_in_list( *nlist, str ) ) return NULL;
    
    pStr = new_string_list( );
    pStr->str  = str_dup( str );
    pStr->next = *nlist;
    return pStr;     
}     

STRING_LIST *add_str_list( STRING_LIST **nlist, char *str )
{
    STRING_LIST *pStr;

    pStr = new_string_list( );
    pStr->str = str_dup( str );
    pStr->next = *nlist;
    return pStr;     
}

STRING_LIST *rem_str_list( STRING_LIST **nlist, char *str )
{
     return *nlist;
}

bool str_in_list( STRING_LIST *nlist, char *str )
{
     STRING_LIST *pStr;

     for( pStr = nlist; pStr != NULL;  pStr = pStr->next )
     {
         if ( !str_cmp( pStr->str, str ) ) return TRUE; 
     }      

     return FALSE;
}
  

