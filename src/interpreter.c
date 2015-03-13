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
 ******************************************************************************
 *   n a m e l e s s i n c a r n a t e          dedicated to chris cool       *
 ******************************************************************************/

/*
 * Please note the following terminology used throughout the script parser.
 *
 * owner   -> pointer to the original prop, actor or scene that is running
 *            this particular instance of a script
 *
 * type     ->  of TYPE_SCENE, TYPE_STRING, TYPE_PROP or TYPE_ACTOR
 *
 * Q: What's a literal statement?   Items enclosed in {,} brackets that
 * do not evaluate their variables until the second pass by a function.
 * Nest for other desired effects. {{}}
 *
 * Q: What is an instance versus a script?
 *
 * Instances hold information that points to code and lists of variables.
 * It is, in essence, a copy of the script's template.
 *
 * Think of them as seperate privatized processes. 
 *
 * Each instance is at some stage of script parsing: 
 * either it is parsing a line of code,
 * or its performing a wait state, or it is halted 
 * or not running.
 *
 * A script or script template is the "script index data" which is the 
 * database template entry for the script.  It contains the code and 
 * the trigger specifications for a particular script, but is a static 
 * entity (unless a builder is editing it).  It provides a way to make 
 * multiple instances of the same code.  
 *
 * Q: How is this language organized?
 *
 * There are two types of commands a scripter can write: 
 * one is a variable assignment, and the other is a function call.
 *
 * It is versatile enough to make a lever or a simple spec script, but can
 * be extensible through spec_scripts or by the addition of new functions
 * that provide special actions.
 *
 * if() statements are limited because they do not allow delays in their
 * direct branches, but can brance to subroutines.
 *
 * point:  if statements should use goto()s as their branches, unless
 *         you are doing something simple.
 *
 * for instance:
 * if ( cmp(random(4),1), goto(label1), goto(label2) );
 * 
 * a switch statement is a partial matching goto statement
 *
 * Q: What are 'global' variables versus 'instance' or 'local' variables?
 *
 * 'Global' variables are on the main variable list of an actor, prop,
 * or scene.  Players even have a 'globals' list.  Each instance of
 * a script, however, has its own 'locals' list and this is the priority
 * list for an iterating script.  To reference specifically a global
 * variable, use the 'global()' function when scripting.
 */

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
#include "functions.h"
#include "script.h"

/*
 * Included in functions.h 
 */

int gotoloops=0;
bool fFoundTrigger=FALSE;

/*
 * Eat(Self); support
 *
 * Self-consuming objects and mobs can't be immediately
 * extracted in the Eat() function; this is because
 * props and actors which are running a script
 * that has a eat(Self) call might end up free'd
 * before the script is done executing.
 * To solve the problem, props are given the type
 * ITEM_DELETE.  Items of ITEM_DELETE execute once
 * before the pulse destroys them.  You can use
 * these items to execute scripts with the born tag
 * on an item, before it is destroyed, or you
 * you can just add destroy; or eat; to your born
 * scripts on objects for them to become Complex Cues.
 * 
 * Complex Cues:
 * Scripts that load mobs, objects, manipulate the
 * weather, set defaults, trigger scripts.  You could
 * use this to modify the mud's starting parameters by
 * creating non-ITEM_DELETE objects that load and
 * execute a script once.
 *
 * This is only an issue because instances don't delete
 * themselves after execution, except on players, and
 * only on players after they quit.
 */

/*
 * Locals.
 */
char * eval_expression        args( ( void * owner, int type, char *exp ) );


/*
 * Locating things from owner/type; used in functions to locate things 
 */

SCENE *find_scene_here( void * owner, int type, char *exp ) {
       SCENE *pScene;

       pScene = get_scene( atoi(exp) );
       if ( pScene ) return pScene;

       FOREACH(type,return ACTOR(owner)->in_scene,

 /* one level deep */ 
                    return PROP(owner)->in_scene != NULL 
                         ? PROP(owner)->in_scene  
                         : (PROP(owner)->carried_by != NULL ? 
                            PROP(owner)->carried_by->in_scene : 
                            (PROP(owner)->in_prop != NULL ? 
                             PROP(owner)->in_prop->in_scene : NULL)),

                    return SCENE(owner), return NULL );

       return NULL;
}

PLAYER *find_actor_here( void * owner, int type, char *exp ) {
       PLAYER *rch;
       PLAYER *found=NULL;

       if ( type == TYPE_ACTOR ) found=get_actor_scene( ACTOR(owner), exp );
       if ( type == TYPE_PROP ) { 
            if ( PROP(owner)->carried_by ) {
                if ( !str_cmp( PROP(owner)->carried_by->name, exp ) ) return PROP(owner)->carried_by;
                found=get_actor_scene( PROP(owner)->carried_by, exp );
            }         
            if ( found ) return found;
            if ( PROP(owner)->in_prop ) {
                if ( PROP(owner)->in_prop->carried_by != NULL ) {
                if ( !str_cmp( PROP(owner)->in_prop->carried_by->name, exp ) ) return PROP(owner)->in_prop->carried_by;
                found = get_actor_scene( PROP(owner)->in_prop->carried_by, exp );
                }
            }
            if ( found ) return found;
            if ( PROP(owner)->in_scene ) {
                for ( rch = PROP(owner)->in_scene->people; 
                      rch!=NULL; 
                      rch = rch->next_in_scene )
                    if ( !str_cmp( rch->name, exp ) ) return rch;
            } 
       }
       if ( type == TYPE_SCENE ) {
                for ( rch = SCENE(owner)->people; 
                      rch!=NULL; rch = rch->next_in_scene )
                    if ( !str_cmp( rch->name, exp ) ) return rch;          
       }
       
       return found;
}

PROP   *find_prop_here( void * owner, int type, char *exp ) {

       if ( type == TYPE_ACTOR ) {
           return get_prop_here( ACTOR(owner), exp );
       }
       else
       if ( type == TYPE_PROP ) {
            if ( PROP(owner)->carried_by != NULL ) {
                return get_prop_here( PROP(owner)->carried_by, exp );
            }         
            else if ( PROP(owner)->in_prop != NULL ) {
                if ( PROP(owner)->in_prop->carried_by != NULL )
                return get_prop_here( PROP(owner)->in_prop->carried_by, exp );
                if ( PROP(owner)->in_prop->in_scene != NULL )
                return find_prop_list( PROP(owner)->in_prop->in_scene->contents, exp );
            }
            else if ( PROP(owner)->in_scene != NULL ) {
                return find_prop_list( PROP(owner)->in_scene->contents, exp );
            } 
       }
       else
       if ( type == TYPE_SCENE ) {
             return find_prop_list( SCENE(owner)->contents, exp );
       }

       return NULL;
}


/* Interpreter */

/*
 * Return a pointer to the globals of associated owner.
 *
 * In other words, determine the type of the variable and
 * return the correct variable list.
 */
VARIABLE **globals( void * owner, int type )
{
    PROP *prop;
    PLAYER *ch;
    SCENE *scene;

    switch ( type )
    {
       case TYPE_PROP: prop  = (PROP *)owner;        return &prop->globals; break;
       case TYPE_ACTOR: ch   = (PLAYER *)owner;       return &ch->globals; break;
      case TYPE_SCENE: scene = (SCENE *)owner; return &scene->globals; break;
        default: return NULL; break;
    }
}




/* 
 * Remove a variable from the list.
 */
void rem_variable( VARIABLE **vlist, char *name )
{
   VARIABLE *v, *vp;

    vp = NULL;
    for ( v = *vlist;  v != NULL;  v = v->next )
    {
        if ( !str_cmp( v->name, name ) )
        break;
        vp = v;
    }

    if ( v == NULL ) return;

    if ( vp == NULL && v != NULL )
         (*vlist) = (*vlist)->next;

    if ( vp != NULL && v != NULL )
         vp->next = v->next;
        
    free_variable(v);
    return;
}



/*
 * Searches a variable list for a variable with supplied name.
 * This is useful but is never used anywhere in this code.
 */
VARIABLE *get_variable( VARIABLE *vlist, char *name )
{
    VARIABLE *v;

    for ( v = vlist;  v != NULL;  v = v->next )
    {
        if ( !str_cmp( v->name, name ) )
        break;
    }
    return v;
}




/*
 * Searches the owner's list for a variable with supplied name.
 *
 * Called by translate_variables for locating variables assocated
 * with an owner.
 */
VARIABLE *find_variable( void * owner, int type, char *name )
{
    VARIABLE *v;
    INSTANCE *instance = PARSING(owner,type);

    /*
     * Search priority: locals then globals.
     */
    if ( instance )
    {
        for ( v = instance->locals;  v != NULL;  v = v->next )
        {
            if ( !str_cmp( v->name, name ) )
            break;
        }

        if ( v ) return v;
    }

    /*
     * Search for a global.
     */
    for ( v = *globals(owner,type);  v != NULL;  v = v->next )
    {
        if ( !str_cmp( v->name, name ) )
        break;
    }

    return v;
}





/*
 * Translates all variables on said owner's expression if "%string%,"
 * ignoring parsing of literals.  (Strips the literals out)
 */
char *translate_variables_noliterals( void * owner, int type, char *exp )
{
    VARIABLE *var;
    static char buf[MAX_STRING_LENGTH];
    char *p;
    char *vpoint;
    char vname[MAX_STRING_LENGTH];

    buf[0] = '\0';
    p = buf;
    while ( *exp != '\0' )
    {
        /*
         * Ignore literals.
         */
        if ( *exp == '}' || *exp == '{' || *exp == ']' || *exp == '[' )
        { 
           exp++;
           continue;
        }
        
        if ( *exp == '%' )
        {
            vname[0] = '\0';
            vpoint = vname;

            /*
             * %variable%
             * ^
             */

            *vpoint++ = *exp++;
            while ( *exp != '%' && *exp != '\0' ) *vpoint++ = *exp++;

            *vpoint++ = '%';
            *vpoint++ = '\0';

            /*
             * %variable%
             *           ^
             */

            /*
             * Locate the variable with this name.
             */
            var = find_variable( owner, type, vname );

            if ( var != NULL
             && var->type == TYPE_STRING )
            {
                /* Insert value */
                vpoint = var->value;
                while ( !MTD(vpoint) ) *p++ = *vpoint++;
            }
            else
            {
                switch( var->type ) {
                 case TYPE_ACTOR: vpoint = NAME(ACTOR(var->value)); break;
                  case TYPE_PROP: vpoint = STR(PROP(var->value),short_descr); break;
                 case TYPE_SCENE: vpoint = SCENE(var->value)->name; break;
                   default: vpoint = NULL; break;
                 }
                if ( vpoint != NULL )
                while ( *vpoint != '\0' ) *p++ = *vpoint++;
            }
        }
        else
        *p++ = *exp++;
    }

    *p = '\0';

    return buf;
}





/*
 * Translates all variables on said owner's expression if "%string%,"
 * ignoring parsing of literals.  (Strips the literals out)
 */
char *translate_variables_noliterals_list( VARIABLE *list, char *exp )
{
    VARIABLE *var;
    static char buf[MAX_STRING_LENGTH];
    char *p;
    char *vpoint;
    char vname[MAX_STRING_LENGTH];

    buf[0] = '\0';
    p = buf;
    while ( *exp != '\0' )
    {
        /*
         * Ignore literals.
         */
        if ( *exp == '}' || *exp == '{' || *exp == ']' || *exp == '[' )
        { 
           exp++;
           continue;
        }
        
        if ( *exp == '%' )
        {
            vname[0] = '\0';
            vpoint = vname;

            /*
             * %variable%
             * ^
             */

            *vpoint++ = *exp++;
            while ( *exp != '%' && *exp != '\0' ) *vpoint++ = *exp++;

            *vpoint++ = '%';
            *vpoint++ = '\0';

            /*
             * %variable%
             *           ^
             */

            /*
             * Locate the variable with this name.
             */
            var = get_variable( list, vname );

            if ( var != NULL
             && var->type == TYPE_STRING )
            {
                /* Insert value */
                vpoint = var->value;
                while ( !MTD(vpoint) ) *p++ = *vpoint++;
            }
            else
            {
                switch( var->type ) {
                 case TYPE_ACTOR: vpoint = NAME(ACTOR(var->value)); break;
                  case TYPE_PROP: vpoint = STR(PROP(var->value),short_descr); break;
                 case TYPE_SCENE: vpoint = SCENE(var->value)->name; break;
                   default: vpoint = NULL; break;
                 }
                if ( vpoint != NULL )
                while ( *vpoint != '\0' ) *p++ = *vpoint++;
            }
        }
        else
        *p++ = *exp++;
    }

    *p = '\0';

    return buf;
}



/*
 * Translates all variables on an expression if "%string%,"
 * except those contained within literal statements.   { like this }
 *
 * Requires the owner to be specified for variable translation,
 * as the variables are stored on the owner.
 */
char *translate_variables( void * owner, int type, char *exp )
{
    VARIABLE *var;
    static char buf[MAX_STRING_LENGTH];
    char *p;
    char *vpoint;
    char vname[MAX_STRING_LENGTH];


    p = buf;
    while ( *exp != '\0' )
    {
        if ( *exp == '%' )
        {
            vname[0] = '\0';
            vpoint = vname;

            /*
             * %variable%
             * ^
             */
            while ( *exp != '\0' )
            {
                *vpoint++ = *exp++;

                if ( *exp == '%' )
                    break;
            }

            *vpoint++ = *exp++;
            *vpoint = '\0';

            var = find_variable( owner, type, vname );

            if ( var != NULL
             && var->type == TYPE_STRING )
            {
                vpoint = var->value;
                while ( !MTD(vpoint) ) *p++ = *vpoint++;
            }
            else
            {
                if ( var ) {
                switch( var->type ) {
                 case TYPE_ACTOR: vpoint = NAME(ACTOR(var->value)); break;
                  case TYPE_PROP: vpoint = STR(PROP(var->value),short_descr); break;
                 case TYPE_SCENE: vpoint = SCENE(var->value)->name; break;
                   default: vpoint = NULL; break;
                 }
                } else vpoint = NULL;
                if ( vpoint != NULL )
                while ( *vpoint != '\0' ) *p++ = *vpoint++;
            }
        }
        else
        {
            /*
             * Jump past literals.
             */
            if ( *exp == '{' || *exp == '[' )
            {
                int curlies = 0;

                *p++ = *exp++;
                while( *exp != '\0' )
                {

                    /*
                     * Ok, get the hell out of here, we're done.
                     * { .... }
                     *        ^
                     */
                    if ( curlies == 0 )
                       if ( *exp == '}' || *exp == ']'
                         || *exp == '\0' )
                         break;

                    if ( *exp == '{' || *exp == '[' ) curlies++;
                    else
                    if ( *exp == '}' || *exp == ']' ) curlies--;

                    *p++ = *exp++;
                }

                if ( curlies != 0 )
                NOTIFY("Notify>  Translate_variables: Unmatched {}s in literal.", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT);

                if ( *exp == '}' || *exp == ']' ) *p++ = *exp++;
            }
            else
            *p++ = *exp++;
        }
    }

    *p = '\0';

    return buf;
}





/*
 * Receives: func(param, .... )
 * Breaks up parameters, and recurses into each.
 *
 * To add a new function, simply add it to the long if-statements.
 */
VARIABLE *eval_function( void * owner, int type, char *exp )
{
    VARIABLE *value=NULL;
    char name[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    VARIABLE *ppoint[MAX_PARAMS];
    char params [MAX_PARAMS][MAX_STRING_LENGTH];
    char *p;
    char *original;
    int x;

    if ( strlen(exp) <= 0 ) return NULL;
    if ( *exp == '\0' ) return NULL;
    if ( *exp == '{' || *exp == '['  )
    return new_variable( TYPE_STRING, str_dup( exp ) );

    original = exp;

    /*
     * Prepare the pointers for the parameters.
     */
    for ( x = 0; x < MAX_PARAMS; x++ )
    {
        params[x][0] = '\0';
        ppoint[x] = NULL;
    }

    /*
     * Grab the function name.
     * func(param, .... )
     * ^
     */
    exp = skip_spaces( exp );
    p = name;
    while ( *exp != '('
         && *exp != ' '
         && *exp != '\0' ) *p++ = *exp++;
    *p = '\0';
    exp = skip_spaces( exp );

    /*
     * Grab and evaluate, recursively, each parameter.
     * func(param, .... )
     *     ^
     */
    if ( *exp != '\0' ) exp++;
    for ( x = 0;  (*exp != '\0') && (x < MAX_PARAMS);  x++ )
    {
        int param_paren = 0;

        /*
         * Grab the parameters.
         * func(param, .... )
         *      ^---^  ^--^
         *        1  to  x
         */

        p = params[x];
        while( *exp != '\0' )
        {
            /*
             * Jump past literals.
             */
            if ( *exp == '{' || *exp == '[' )
            {
                int curlies = 0;

                *p++ = *exp++;
                while( *exp != '\0' )
                {

                    /*
                     * Ok, get the hell out of here, we're done.
                     * { .... }
                     *        ^
                     */
                    if ( curlies == 0 )
                       if ( *exp == '}' || *exp == ']'
                         || *exp == '\0' )
                         break;

                    if ( *exp == '{' || *exp == '[' ) curlies++;
                    else
                    if ( *exp == '}' || *exp == ']' ) curlies--;

                    *p++ = *exp++;
                }

                if ( curlies != 0 )
                NOTIFY("Notify>  Eval_function: Unmatched {}s in literal.", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT);

                if ( *exp == '}' || *exp == ']' ) *p++ = *exp++;
            }

            /*
             * Ok, get the hell out of here, we're done.
             * func(param, .... )
             *           ^  or  ^
             */
            if ( param_paren == 0 )
                if ( *exp == ','
                  || *exp == ')'
                  || *exp == '\0' )
                {
                    if ( *exp == ',' || *exp == ')' ) exp++;
                    break;
                }


            if ( *exp == '(' ) param_paren++;
            else
            if ( *exp == ')' ) param_paren--;

/*            if ( *exp == ' ' ) exp++;
            else */ *p++ = *exp++;
        }

        *p = '\0';

        /*
         * Evaluate each one.
         */
        if ( strlen(params[x]) > 0 )  ppoint[x] = eval_function( owner, type, params[x] );
    }


    buf[0] = '\0';

    /*
     * Remember, ppoint[] now contains the evaluated values.
     * Place more often called functions first.
     *
     * Idea for improvement: make this list dynamic (a linked list of
     * functions, defined at startup as pointers to table indexes),
     * and save call counts.  Sort the list during database saves.
     * Then iterate the list here.
     */

    /* Note to self: change functionality of goto() here
     * If the goto() function is called, it jumps the code immediately
     * This will need to be changed to accomplish statements like:
     * if (this, goto(this), goto(that) ); 
     */

value = NULL;
switch ( name[0] ) {

 case 'i':
    FUNC("if",       func_if      (owner, type, ppoint[0], ppoint[1], ppoint[2]) );
    else
    FUNC("ifgoto",   func_ifgoto  (owner, type, ppoint[0], ppoint[1], ppoint[2]) );
    else
    FUNC("in",       func_in      (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("is",       func_cmp     (owner, type, ppoint[0], ppoint[1]) );
    else
    MATH_FUNCTIONI(func_inc, "inc");
    else
    MATH_FUNCTIONI(func_INTERPOLATE, "int");
    else
    FUNC("install",func_install (owner, type, ppoint[0], ppoint[1] ) );
    else
    FUNC("inside", func_inside  (owner, type, ppoint[0], ppoint[1] ) );
    else
    MATH_FUNCTIONI(func_interpolate, "interpolate");
    else
    GET_FUNCTIONPI(func_itemtype,    "itemtype");
 break;

 case 'g':
    FUNC("gem",      func_gem     (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("goto",     func_goto    (owner, type, ppoint[0]) );
    else
    MATH_FUNCTIONI(func_greater, "gt");
    else
    MATH_FUNCTIONI(func_gte,     "gte");
    else
    MATH_FUNCTIONI(func_greater, "greater");
    else
    MATH_FUNCTIONI(func_gelify,  "gelify");
    else
    FUNC("getdir",   func_getdir  (owner, type, ppoint[0], ppoint[1]) );
    else
    GRAPHICS_FUNCTION(func_gmode,"gmode");
 break;

 case 'a':
    FUNC("astr",     func_astr    (owner, type) );
    else
    FUNC("actor",    func_actor   (owner, type) );
    else
    FUNC("aset",     func_aset    (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("as",  func_as(owner, type, ppoint[0], ppoint[1], ppoint[2], ppoint[3], ppoint[4], ppoint[5]) );
    else
    FUNC("autowait", func_autowait(owner, type, ppoint[0]) );
    else
    MATH_FUNCTIONI(func_and, "and");
    else
    MATH_FUNCTIONI(func_add, "add");
    else
    FUNC("alert",    func_alert   (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("addowed",  func_addowed (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("addbounty",func_addbounty (owner, type, ppoint[0], ppoint[1]) );
    else
    GET_FUNCTIONAS(func_actorname, "actorname" );
    else
    GET_FUNCTIONAI(func_armor,     "armor"     );
 break;

 case 'b':
    FUNC("bstr",     func_bstr    (owner, type) );
    else
    FUNC("broadcast", func_broadcast( owner, type, ppoint[0] ) );
    else
    FUNC("breed",    func_breed( owner, type, ppoint[0], ppoint[1] ) );
    else
    FUNC("bounty",   func_bounty  (owner, type, ppoint[0]) );
    else
    FUNC("bomb",     func_bomb( owner, type, ppoint[0], ppoint[1]) );
    else
    MATH_FUNCTIONI(func_band, "band");
    else
    MATH_FUNCTIONI(func_bor,  "bor" );
    else
    MATH_FUNCTIONI(func_bxor, "bxor");
    else
    MATH_FUNCTIONI(func_bits, "bits");
    else
    GRAPHICS_FUNCTION(func_box,       "box");
    else
    GRAPHICS_FUNCTION(func_brush,     "brush");
    else
    GRAPHICS_FUNCTION(func_button,    "button");
    else
    GRAPHICS_FUNCTION(func_pageclear, "blank");
    else
    FUNC("build",     func_build( owner, type, ppoint[0], ppoint[1], ppoint[2], ppoint[3], ppoint[4], ppoint[5]) );
 break;

 case 'f':
    FUNC("following",  func_following     (owner, type) );
    else
    FUNC("foreach",func_foreach   (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("force",    func_force   (owner, type, ppoint[0]) );
    else
    FUNC("foe",      func_foe     (owner, type) );
    else
    MATH_FUNCTIONI(func_fuzzy,    "fuzzy");
    else
    GRAPHICS_FUNCTION(func_fill,  "fill");
    else
    GRAPHICS_FUNCTION(func_fills, "fills");
 break;


 case 'c':
    FUNC("check",    func_check   (owner, type, ppoint[0]) );
    else
    FUNC("create",   func_create  (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("close",    func_close   (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("charge",   func_charge  (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("call",     func_call    (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("cmp",      func_cmp     (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("cat",      func_cat     (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("cue",      func_cue     (owner, type, ppoint[0]) );
    else
    GRAPHICS_FUNCTION(func_circle, "circle");
    else
    GRAPHICS_FUNCTION(func_clear,  "clear");
    else
    GRAPHICS_FUNCTION(func_color,  "color");
    else
    GRAPHICS_FUNCTION(func_column, "column");
    else
    GET_FUNCTIONPI(func_cost,      "cost" );
 break;


case 'o':
    MATH_FUNCTIONI(func_or, "or");
    else
    FUNC("open",     func_open     (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("owed",     func_owed     (owner, type, ppoint[0]) );
    else
    FUNC("oecho",    func_oecho    (owner, type, ppoint[0], ppoint[1]) );
break;


case 'm':
    MATH_FUNCTIONI(func_mult,"mult");
    else
    MATH_FUNCTIONI(func_mod, "mod");
    else
    MATH_FUNCTIONI(func_min, "min");
    else
    MATH_FUNCTIONI(func_max, "max");
    else
    FUNC("music",   func_music (owner, type, ppoint[0], ppoint[1], ppoint[2], ppoint[3], ppoint[4], ppoint[5]) );
    else
    FUNC("move",     func_move    (owner, type, ppoint[0], ppoint[1]) );
    else
    GET_FUNCTIONI(func_month,    "month");
    else
    FUNC("moon",     func_moon    (owner, type) );
    else
    FUNC("moveall",  func_moveall  (owner, type, ppoint[0], ppoint[1], ppoint[2]) );
    else
    FUNC("mix",      func_mix (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("mana",     func_mana    (owner, type, ppoint[0], ppoint[1]) );
    else
    GRAPHICS_FUNCTION(func_mode, "mode");
    else
    GET_FUNCTIONSI(func_maxpeople, "maxpeople" );
    else
    GET_FUNCTIONAI(func_modcon, "modcon" );
    else
    GET_FUNCTIONAI(func_modstr, "modstr" );
    else
    GET_FUNCTIONAI(func_modwis, "modwis" );
    else
    GET_FUNCTIONAI(func_moddex, "moddex" );
    else
    GET_FUNCTIONAI(func_modint, "modint" );
break;

case 'w':
    FUNC("wait",     func_wait    (owner, type, ppoint[0]) );
    else
    GET_FUNCTIONI(func_sky,"weather");
    else
    FUNC("word",     func_word    (owner, type, ppoint[0], ppoint[1]) );
    else
    GRAPHICS_FUNCTION(func_wrap, "wrap");
    else
    GET_FUNCTIONSI(func_wagon,   "wagon");
    else
    GET_FUNCTIONPI(func_wearloc, "wearloc" );
    else
    GET_FUNCTIONPI(func_weight,  "weight" );
break;

case 'n':
    FUNC("name",     func_name    (owner, type, ppoint[0]) );
    else
    MATH_FUNCTIONI(func_not, "not");
    else
    FUNC("numw",     func_numw    (owner, type, ppoint[0]) );
break;

case 'd':
    FUNC("do",       func_do      (owner, type, ppoint[0], ppoint[1], ppoint[2], ppoint[3], ppoint[4], ppoint[5] ) );
    else
    FUNC("disarm",   func_disarm  (owner, type, ppoint[0]) );
    else
    FUNC("dispense", func_dispense (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("damage",   func_hurt    (owner, type, ppoint[0], ppoint[1]) );
    else
    MATH_FUNCTIONI(func_div,  "div");
    else
    MATH_FUNCTIONI(func_dec,  "dec");
    else
    MATH_FUNCTIONI(func_dice, "dice");
    else
    FUNC("dream",    func_dream   (owner, type, ppoint[0]) );
    else
    FUNC("death",    func_death   (owner, type, ppoint[0], ppoint[1]) );
    else
    GET_FUNCTIONI(func_day,        "day");
    else
    GET_FUNCTIONI(func_dayofweek,  "dayofweek");
    else
    FUNC("destroy",  func_eat     (owner, type, ppoint[0]) );
    else
    FUNC("dig",      func_dig     (owner, type, ppoint[0], ppoint[1], ppoint[2] ) );
    else
    GRAPHICS_FUNCTION(func_draw,     "draw" );
    else
    GET_FUNCTIONAS(func_description, "description" );
    else
    GET_FUNCTIONAI(func_damroll,     "damroll" );
break;

case 'j':
    FUNC("jump",     func_jump    (owner, type, ppoint[0]) );
break;

case 'v':
    FUNC("dbkey",     func_dbkey    (owner, type, ppoint[0]) );
    else
    GRAPHICS_FUNCTION(func_vline, "vline");
    else
    GET_FUNCTIONPI(func_v1,       "v1" );
    else
    GET_FUNCTIONPI(func_v2,       "v2" );
    else
    GET_FUNCTIONPI(func_v3,       "v3" );
    else
    GET_FUNCTIONPI(func_v4,       "v4" );
break;

case 'e':
    FUNC("empty", func_empty (owner, type, ppoint[0]) );
    else
    FUNC("equip", func_equip (owner, type, ppoint[0], ppoint[1], ppoint[2]) );
    else
    FUNC("each",func_each (owner, type, ppoint[0]) );
    else
    FUNC("elude",    func_elude   (owner, type) );
    else
    FUNC("echo",     func_echo    (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("emit",     func_emit    (owner, type, ppoint[0]) );
    else
    FUNC("eat",      func_eat     (owner, type, ppoint[0]) );
    else
    FUNC("e",        func_eval    (owner, type, ppoint[0]) );
    else
    MATH_FUNCTIONI(func_eq,         "eq"  );
    else
    GET_FUNCTIONAI(func_exp,        "exp" );
    else
    GET_FUNCTIONPI(func_extraflags, "extraflags" );
break;

case 'h':
    FUNC("here",     func_here    (owner, type) );
    else
    FUNC("heal",     func_heal    (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("healall",  func_healall (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("hurt",     func_hurt    (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("home",     func_home    (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("halt",     func_halt    (owner, type, ppoint[0]) );
    else
    FUNC("history",  func_history (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("has",      func_has     (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("holds",    func_holds     (owner, type, ppoint[0], ppoint[1]) );
    else
    GRAPHICS_FUNCTION(func_hline, "hline");
    else
    GET_FUNCTIONAI(func_hitroll,  "hitroll" );
break;

case 's':
    FUNC("self",     func_self    (owner, type) );
    else
    FUNC("strstr",   func_strstr  (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("skill", func_skill( owner, type, ppoint[0], ppoint[1], ppoint[2]) );
    else
    FUNC("setskill", func_setskill(owner,type,ppoint[0],ppoint[1],ppoint[2]) );
    else
    FUNC("samescene", func_samescene(owner, type, ppoint[0]) );
    else
    MATH_FUNCTIONI(func_sub,          "sub");
    else
    FUNC("strp",     func_strp    (owner, type, ppoint[0], ppoint[1], ppoint[2]) );
    else
    FUNC("replace",  func_strp    (owner, type, ppoint[0], ppoint[1], ppoint[2]) );
    else
    FUNC("strip",    func_strip   (owner, type, ppoint[0]) );
    else
    FUNC("sort",func_sort (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("sound",   func_sound (owner, type, ppoint[0], ppoint[1], ppoint[2], ppoint[3], ppoint[4], ppoint[5]) );
    else
    FUNC("spawn",    func_cue  (owner, type, ppoint[0]) );
    else
    FUNC("scenetitle", func_scenetitle  (owner, type, ppoint[0], ppoint[1]) );
    else
    GET_FUNCTIONI(func_sky,           "sky");
    else
    SET_FUNCTIONI(func_setmonth,      "setmonth");
    else
    SET_FUNCTIONI(func_setday,        "setday");
    else
    SET_FUNCTIONI(func_settime,       "settime");
    else
    SET_FUNCTIONI(func_setyear,       "setyear");
    else
    SET_FUNCTIONI(func_setsky,        "setsky");
    else
    SET_FUNCTIONI(func_setmoon,       "setmoon");
    else
    SET_FUNCTIONI(func_setphase,      "setphase");
    else
    SET_FUNCTIONI(func_settemperature,"settemp");
    else
    SET_FUNCTIONI(func_settemperature,"settemperature");
    else
    SET_FUNCTIONI(func_setsky,        "setweather");
    else
    GRAPHICS_FUNCTION(func_stroke,    "stroke");
    else
    GRAPHICS_FUNCTION(func_style,     "style");
    else
    SET_FUNCTIONSS(func_setscenetitle,"setscenetitle" );
    else
    GET_FUNCTIONSS(func_scenetitle,   "scenetitle" );
    else
    SET_FUNCTIONSS(func_setscenedesc, "setscenedesc" );
    else 
    GET_FUNCTIONSS(func_scenedesc,    "scenedesc");
    else
    SET_FUNCTIONSI(func_setscenepos,  "setscenepos" );
    else
    GET_FUNCTIONSI(func_scenepos,     "scenepos" );
    else
    SET_FUNCTIONSI(func_settemplate,  "settemplate" );
    else
    SET_FUNCTIONSI(func_setmaxpeople, "setmaxpeople" );
    else
    SET_FUNCTIONSI(func_setlight,     "setlight" );
    else        
    SET_FUNCTIONSI(func_setmove,    "setmove" );
    else
    SET_FUNCTIONSI(func_setterrain,   "setterrain" );
    else
    SET_FUNCTIONSI(func_setwagon,     "setwagon" );
    else
    GET_FUNCTIONSI(func_move,       "move" );
    else
    SET_FUNCTIONAS(func_setactorname, "setactorname" );
    else
    SET_FUNCTIONAS(func_setdescription, "setdescription" );
    else
    SET_FUNCTIONAS(func_setshort,   "setshort" );
    else
    SET_FUNCTIONAS(func_setlong,    "setlong" );
    else
    SET_FUNCTIONAI(func_setposition,"setposition" );
    else 
    SET_FUNCTIONAI(func_setkarma,   "setkarma" );
    else
    SET_FUNCTIONAI(func_setmodint,  "setmodint" );
    else
    SET_FUNCTIONAI(func_setmoddex,  "setmoddex" );
    else
    SET_FUNCTIONAI(func_setmodcon,  "setmodcon" );
    else
    SET_FUNCTIONAI(func_setmodwis,  "setmodwis" );
    else
    SET_FUNCTIONAI(func_setmodstr,  "setmodstr" );
    else
    SET_FUNCTIONAI(func_setarmor,   "setarmor" );
    else
    SET_FUNCTIONAI(func_sethitroll, "sethitroll" );
    else
    SET_FUNCTIONAI(func_setexp,     "setexp" );
    else
    SET_FUNCTIONAI(func_setdamroll, "setdamroll" );
    else
    GET_FUNCTIONAS(func_short,      "short" );
    else
    SET_FUNCTIONPS(func_setpropname, "setpropname" );
    else
    SET_FUNCTIONPS(func_setpropdesc, "setpropdesc" );
    else
    SET_FUNCTIONPS(func_setpropshort,"setpropshort" );
    else
    SET_FUNCTIONPS(func_setproplong, "setproplong" );
    else
    SET_FUNCTIONPS(func_setpropaction, "setpropaction" );
    else
    SET_FUNCTIONPI(func_setv1,    "setv1" );
    else
    SET_FUNCTIONPI(func_setv2,    "setv2" );
    else
    SET_FUNCTIONPI(func_setv3,    "setv3" );
    else
    SET_FUNCTIONPI(func_setv4,    "setv4" );
    else
    SET_FUNCTIONPI(func_setitemtype,   "setitemtype" );
    else
    SET_FUNCTIONPI(func_setextraflags, "setextraflags" );
    else
    SET_FUNCTIONPI(func_setwearloc,    "setwearloc" );
    else
    SET_FUNCTIONPI(func_setweight,     "setweight" );
    else
    SET_FUNCTIONPI(func_setcost,       "setcost" );
    else
    SET_FUNCTIONPI(func_settimer,      "settimer" );
    else
    SET_FUNCTIONPI(func_setsize,       "setsize" );
    else
    GET_FUNCTIONPI(func_size,          "size" );
break;


case 't':
    GET_FUNCTIONI(func_time, "time");
    else
    GET_FUNCTIONPI(func_timer, "timer");
    else
    FUNC("transform", func_transform( owner, type, ppoint[0] ) );
    else
    GET_FUNCTIONI(func_temperature, "temperature");
    else  
    GET_FUNCTIONI(func_temperature, "temp");
    else
    GRAPHICS_FUNCTION(func_text,  "text");
    else
    GRAPHICS_FUNCTION(func_tline, "tline");
    else
    GET_FUNCTIONSI(func_terrain,  "terrain");
break;

case 'p':
    FUNC("permhalt", func_permhalt(owner, type ) );
    else
    FUNC("push",func_push (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("pay",      func_pay   (owner, type, ppoint[0], ppoint[0]) );
    else
    FUNC("purge",    func_purge   (owner, type, ppoint[0]) );
    else
    FUNC("pre",      func_pre     (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("pos",      func_pos   (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("pop",  func_pop (owner, type, ppoint[0]) );
    else
    FUNC("pset",     func_pset     (owner, type, ppoint[0], ppoint[1]) );
    else
    MATH_FUNCTIONI(func_percent,"percent");
    else
    GRAPHICS_FUNCTION(func_pageclear,"pageclear");
    else
    GRAPHICS_FUNCTION(func_point,"point");
    else
    GRAPHICS_FUNCTION(func_popup,"popup");
    else
    GET_FUNCTIONAI(func_position,   "position" );
    else
    GET_FUNCTIONPS(func_propname,   "propname" );
    else
    GET_FUNCTIONPS(func_propdesc,   "propdesc" );
    else
    GET_FUNCTIONPS(func_propshort,  "propshort" );
    else
    GET_FUNCTIONPS(func_proplong,   "proplong" );
    else
    GET_FUNCTIONPS(func_propaction, "propaction" );
/*
    else
    FUNC("players",  func_players (owner, type) );
 */
break;

case 'k':
    GET_FUNCTIONAI(func_karma, "karma" ); 
break;

case 'l':
    FUNC("label",    func_label   (owner, type) );
    else
    MATH_FUNCTIONI(func_less,"less");
    else
    MATH_FUNCTIONI(func_less,"lt");
    else
    MATH_FUNCTIONI(func_lte, "lte");
    else
    FUNC("lrnd",func_lrnd (owner, type, ppoint[0]) );
    else
    FUNC("level", func_level (owner, type, ppoint[0]) );
    else
    FUNC("lshift",func_lshift (owner, type, ppoint[0]) );
    else
    MATH_FUNCTIONI(func_lineate,"lineate");
    else
    GRAPHICS_FUNCTION(func_line,"line");
    else
    GET_FUNCTIONSI(func_light, "light" );
    else
    GET_FUNCTIONAS(func_long,  "long" );
   break;

case 'r':
    FUNC("reagents", func_reagents (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("reset",    func_cue  (owner, type, ppoint[0]) );
    else
    FUNC("recho",     func_oecho    (owner, type, ppoint[0], ppoint[1]) );
    else
    FUNC("rshift",   func_rshift  (owner, type, ppoint[0]) );
    else
    FUNC("return",   func_return  (owner, type, ppoint[0]) );
    else
    FUNC("rndplr",   func_rndplr  (owner, type, ppoint[0]) );
    else
    FUNC("rnddir",   func_rnddir  (owner, type) );
    else
    MATH_FUNCTIONI(func_ratio,"ratio");
    else
    MATH_FUNCTIONI(func_random,"random");
    else
    MATH_FUNCTIONI(func_range,"range");
    else
    GRAPHICS_FUNCTION(func_rect,"rect");
    else
    GRAPHICS_FUNCTION(func_ruler,"ruler");
    else
    GRAPHICS_FUNCTION(func_rulers,"rulers");
    else
    GET_FUNCTIONSI(func_template, "reference" );
break;

case 'y':
    GET_FUNCTIONI(func_year,"year");
break;

case 'u':
    FUNC("users", func_users (owner, type, ppoint[0]) );
    else
    FUNC("undig", func_undig( owner, type, ppoint[0], ppoint[1] ) );
    else
    FUNC("uninstall",func_uninstall (owner, type, ppoint[0], ppoint[1] ) );
    else
    MATH_FUNCTIONI(func_urange,"urange");
break;

default:
    {
        value = new_variable( TYPE_STRING, str_dup( original ) );
    }
break;
}
    /*
     * Flush the parameters.
     */
    for ( x = 0; x < MAX_PARAMS; x++ ) free_variable( ppoint[x] );
    return value;
}



/*
 * Assigns a variable, creating a new one if required.
 * Note string routines embedded in scriptedure.
 */
void assign_var( void * owner, int type, VARIABLE *var, char *name )
{
    VARIABLE *v;
    INSTANCE *trig = PARSING(owner,type);

    if ( !trig )
    return;

    v = find_variable( owner, type, name );

    if ( v )
    {
        if ( v->type == TYPE_STRING )
        {
            if ( v->value ) free_string( (char *)(v->value) );
            v->value = str_dup( "" );
        }
    }
    else
    {
        v = new_var( );
        v->next = trig->locals;
        trig->locals = v;
        v->name = str_dup( name );
    }

    if (var == NULL) {
        v->type = TYPE_STRING;
        v->value = str_dup( "0" );
    } else {
        v->type = var->type;
        v->value = var->type == TYPE_STRING ? str_dup( var->value ) : var->value;
    }

    return;
}


/*
 * Assigns a variable, creating a new one if required.
 * Puts it on an instance as a local variable.
 * Does not check for duplicates.
 */
void assign_local( INSTANCE *pInstance, VARIABLE *var, char *name )
{
    VARIABLE *v;

    if ( !pInstance )
    return;

    v = new_var( );
    v->next = pInstance->locals;  /* add to list */
    pInstance->locals = v;
    v->name = str_dup( name );

    if (var == NULL) {
        v->type = TYPE_STRING;
        v->value = str_dup( "0" );
    } else {
        v->type = var->type;
        v->value = var->type == TYPE_STRING ? str_dup( var->value ) : var->value;
    }

    return;
}


/*
 * Handles assignment statements.
 * Assumes the statement has already been identified as an assignment
 * statement.
 */
void parse_assign( char *l, void * owner, int type )
{
    char vname[MAX_STRING_LENGTH];
    char *p;
    VARIABLE *value;

    /*
     * Grab the variable name from the beginning of the line.
     */

    p = vname;
    l = skip_spaces( l );    /* Ignore start of line whitespace. */
/*    if ( *l != '%' ) 
        return; */

    /* Copy the variable name.  Advance line to end of whitespace
     * following the variable name */

    *p++ = *l++;
    while ( *l != '%' && *l != ' ' && *l != '\0' ) 
    *p++ = *l++;

    *p++ = '%';
    *p++ = '\0';  /* Close up the end of vname string */

    skip_spaces( ++l );

    /*
     * Evaluate functions in the assignment.
     * Use literals to contain code or variable names.
     */

    value = eval_function( owner, type, l );

    assign_var( owner, type, value, vname );      /* mem leak? */
    free_variable(value); 
/*    wtf_logf( "assigned var", 0 ); */
    return;
}




/*
 * The script parser; this is the main string parsing statement.
 * It's all one long function; it scans through a bit of script code
 * and decides what to do with what it finds.  Watch here for
 * memory leaks; this function makes calls to all script language
 * functions with eval_function() and parse_assign().
 */
void parse_script( INSTANCE *instance, void * owner, int type )
{
    PLAYER *ch = NULL;
    PROP *prop;
    SCENE *scene;
    char *commands;

    int rdbkey = 0;

    if ( instance == NULL )
    {
        wtf_logf( "Parse_script: NULL instance.", 0 );
        return;
    }

    if ( owner == NULL )  {
    wtf_logf( "Parse_script: NULL owner.", 0 );
    return;
    }

    /*
     * Before the interpreter evaluates code, it must set
     * the 'current' instance variable on a prop, actor or scene
     * so underlying functions can recall which instance the
     * interpreter is working on.  This is an 'instantaneous'
     * pointer and is only here to simplify writing of
     * functions in this file.
     */
    switch ( type )
    {
       case TYPE_PROP:
         prop  = (PROP *)owner;
         prop->current  = instance;
         rdbkey = prop->in_scene ? prop->in_scene->dbkey : 0;
        break;
       case TYPE_ACTOR:
         ch   = (PLAYER *)owner;
         ch->current   = instance;
         rdbkey = ch->in_scene ? ch->in_scene->dbkey : 0;
        break;
       case TYPE_SCENE:
         scene = (SCENE *)owner;
         scene->current = instance;
         rdbkey = scene->dbkey;
        break;
       default:
         wtf_logf( "Parse_script: Invalid requested owner-type.", 0 );
        return;
    }

     /*
      * In-game Dewtf_logfger Code
      */
    if ( !MTD(instance->location) ) instance_track( instance, owner );
    else {

        /*
         * The cursor has reached the end of the code.
         * Are we done?
         * If so, remove all variables associated with this script.
         */

        VARIABLE *pVar, *pVar_next;

        for ( pVar = instance->locals;  pVar != NULL; pVar = pVar_next )
        {
            pVar_next = pVar->next;
            free_variable( pVar );
        }

        instance->locals = NULL;
        instance->location = NULL;
        return;
    }

    if ( instance->wait != 0 ) instance->wait--;
    gotoloops = 0;

    /*
     * Iterate through a set of code lines.
     * Stop when the script is halted, finished or delayed.
     */
    while ( !IS_SET(instance->state, SCRIPT_HALT)
         && !MTD(instance->location)
         && instance->wait == 0 )
    {
        char buf[MAX_STRING_LENGTH];

        /*
         * Advance a line.
         */
        instance->location = one_line( instance->location, buf );
        commands = skip_spaces( buf );

        if ( *commands == '\0' ) continue;

        /*
         * Actually call a function or assign a variable.
         */
        if ( *commands == '%' )
        parse_assign( commands, owner, type );
        else
        {
            VARIABLE *value;

            value = eval_function( owner, type, commands );
            free_variable( value );
        }

        if ( instance->autowait) instance->wait = instance->autowait;
    }

    return;
}



/*
 * Simple triggering function, design to trigger an already
 * identified instance of a script.
 *
 * Sets a trigger on the owner by setting the interpreter to
 * the beginning the of the code to be parsed.
 *
 * Assumes its already in the instance list for the owner.
 */
int trigger( void * owner, int type, INSTANCE *trig )
{
    int returned;

    if ( trig->returned ) {
       free_variable(trig->returned); 
       trig->returned=NULL; 
    }
    trig->location = trig->script->commands;

    parse_script( trig, owner, type );
    if ( trig->returned && trig->returned->type == TYPE_STRING )
      returned=atoi((char *)(trig->returned->value));
    else returned=0;
    return returned;
}



/*
 * Quick and dirty multi-purpose script processor.
 * Scans through a list of instances and instances matching
 * trigger type are triggered.  Creates associated default
 * 'parameter' variables.
 *
 * Script_update is called from other functions throughout the code,
 * specifically in cmd_cast, as well as the update routines
 * in update.c, and some of the other cmd_ functions.
 */
int script_update( void * owner, int type, int ttype, PLAYER *actor,
                    PROP *catalyst, char *astr, char *bstr  )
{
    INSTANCE *trig;
    SCENE *scene =NULL;
    PLAYER *ch =NULL;
    PROP *prop =NULL;
    int returned=0;
    
    switch ( type )
    {
       case TYPE_PROP:  prop = (PROP *)owner; 
                        trig = prop->instances; 
                      break;

       case TYPE_ACTOR:  ch  = (PLAYER *)owner; 
                         trig = ch->instances; 
                      break;

       case TYPE_SCENE: scene = (SCENE *)owner; 
                        trig = scene->instances;
                       break;

       default: trig = NULL;   break;
    }
    
    if ( prop && prop->item_type == ITEM_DELETE ) return 0;

    for ( ; trig != NULL; trig = trig->next )
    {

        if ( !trig 
          || trig->script->type != ttype
          || trig->location != NULL )
        continue;

        {
            INSTANCE *oldtrig = PARSING(owner,type);
            VARIABLE *var;

            switch ( type )
            {
               case TYPE_PROP:  prop->current  = trig; break;
               case TYPE_ACTOR:  ch->current   = trig; break;
               case TYPE_SCENE: scene->current = trig; break;
               default: break;
            }

    if ( trig != NULL && trig->script->type != TRIG_EACH_PULSE ) {
          char buf[MAX_STRING_LENGTH];
          sprintf( buf, "Notify> Script %d, %s triggered by %s%s%s%s%s%s%s%s", 
                   trig->script->dbkey, 
                   trig->script->name, actor ? NAME(actor) : "",
                   catalyst != NULL ? " " : "",
		   catalyst != NULL  ? STR(catalyst,short_descr) : "",	   
                   astr != NULL || 
                   bstr != NULL ? "\n\rWith the following variables:" : ".",
                   astr ? "\n%astr% = " : "", astr != NULL ? astr : "",
                   bstr ? "\n%bstr% = " : "", 
                   bstr != NULL ? bstr : "" );
	  NOTIFY( buf, LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT);
     }

/*
 * Create default parameter variables.
 */
            var = new_var( );
            if ( actor )
            {
                var->type  = TYPE_ACTOR;
                var->value = (PLAYER *)actor;
                assign_var( owner, type, var, "%actor%" );

                var->type = TYPE_STRING;
                var->value = (char *)str_dup( STR((PLAYER *)actor,name));
                assign_var( owner, type, var, "%name%" );
            }

            if ( catalyst )
            {
                var->type = TYPE_PROP;
                var->value = (PROP *)catalyst;
                assign_var( owner, type, var, "%catalyst%" );
            }

            if ( astr )
            {
                var->type = TYPE_STRING;
                var->value = str_dup( astr );
                assign_var( owner, type, var, "%astr%" );
            }

            if ( bstr )
            {
                var->type = TYPE_STRING;
                var->value = str_dup( bstr );
                assign_var( owner, type, var, "%bstr%" );
            }

            switch ( type )
            {
               case TYPE_PROP:  prop->current  = oldtrig; break;
               case TYPE_ACTOR:  ch->current   = oldtrig; break;
               case TYPE_SCENE: scene->current = oldtrig; break;
               default: break;
            }

            free_variable( var );
        }
          
        if ( (returned = trigger(owner, type, trig)) ) break;
    }
    return returned;
}



/*
 * This function attempts to trigger all props in a list.
 * Used primarily in searching for command functions.
 * Doesn't care if it successfully or unsuccessfully
 * triggers a script.  May create command conflicts
 * (multiple copies of the same item in a list).
 *
 * Called in a variety of locations.
 */
void trigger_list( PROP *list, int ttype, PLAYER  *actor,
                  PROP *catalyst, char *astr, char *bstr )
{
    PROP *prop, *prop_next;

    for ( prop = list;  prop != NULL;  prop = prop_next )
    {
        prop_next = prop->next_content;
        script_update( prop, TYPE_PROP, ttype, actor, catalyst, astr, bstr );
    }

    return;
}


extern VARIABLE *mud_var_list;  /* from mem.c */


/*
 * This little command helps a user debug the scripts online.  It can give
 * information about a single instance, or general info on the actor's current
 * state of execution.
 */
void cmd_script( PLAYER *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    PROP *prop;
    PLAYER *victim;
/*
    void * owner;
    int type;
 */

    if ( NPC(ch) || ch->userdata == NULL )
        return;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        to_actor( "Tracking stopped.\n\r", ch );
        ch->userdata->trackscr = NULL;
        ch->userdata->trackscr_type = TYPE_STRING;
	return;
    }

    if ( !str_cmp( arg, "variables" ) )
    {
        VARIABLE *pVar;

        for ( pVar = mud_var_list;  pVar != NULL;  pVar = pVar->next_master_var )
        {
            sprintf( buf, "  [%2d] %s ", pVar->type, pVar->name );
            to_actor( buf, ch );

            switch ( pVar->type )
            {
                case TYPE_STRING:
sprintf( buf, " = \"%s\"\n\r", (char *)(pVar->value) ); break;
                case TYPE_ACTOR:    
sprintf( buf, " = a:%s\n\r", NAME( (PLAYER *)(pVar->value) ) ); break;
                case TYPE_PROP:    
sprintf( buf, " = p:%d\n\r", ((PROP *)(pVar->value))->pIndexData->dbkey );
 break;
                case TYPE_SCENE:   
sprintf( buf, " = s:%d\n\r", ((SCENE *)(pVar->value))->dbkey ); 
 break;
                       default:   sprintf( buf, " = <unknown:%d>\n\r", pVar->type ); break;
            }
            to_actor( buf, ch );
        }
        
        return;
    }


    if ( (victim = get_actor_world(ch,arg)) == NULL )
    {
        bool fFound = FALSE;
        INSTANCE *pTrig;
        VARIABLE *pVar;

        prop = get_prop_here( ch, arg );

        if ( !prop || !prop->pIndexData ) {
            to_actor( "They, or that, isn't here.\n\r", ch );
            return;
        } 

        ch->userdata->trackscr = (void *)prop;
        ch->userdata->trackscr_type = TYPE_PROP;

        sprintf( buf, "DEBUG %s from %s:\n\r", STR(prop,short_descr),
                 prop->in_scene ? 
                 prop->in_scene->name
              : (prop->in_prop  ? STR((prop->in_prop),short_descr) 
              : (prop->carried_by ? NAME(prop->carried_by) : NULL) ) );
        to_actor( buf, ch );

        for ( pTrig = prop->instances; pTrig != NULL;  pTrig = pTrig->next )
        {
            fFound = TRUE;

            sprintf( buf, "-==[WATCHING]==---------[%5d] %s (%d, auto %d)\n\r%s",
                     pTrig->script->dbkey, pTrig->script->name,
                     pTrig->wait, pTrig->autowait,
                     pTrig->location ? pTrig->location : "" );
            to_actor( buf, ch );

            for ( pVar = pTrig->locals;  pVar != NULL;  pVar = pVar->next )
            {
            sprintf( buf, "  [%2d] %s ", pVar->type, pVar->name );
            to_actor( buf, ch );

            switch ( pVar->type )
            {
                case TYPE_STRING:
sprintf( buf, " = \"%s\"\n\r", (char *)(pVar->value) ); break;
                case TYPE_ACTOR:    
sprintf( buf, " = a:%s\n\r", NAME( (PLAYER *)(pVar->value) ) ); break;
                case TYPE_PROP:    
sprintf( buf, " = p:%d\n\r", ((PROP *)(pVar->value))->pIndexData->dbkey );
 break;
                case TYPE_SCENE:   
sprintf( buf, " = s:%d\n\r", ((SCENE *)(pVar->value))->dbkey ); 
 break;
                       default:   sprintf( buf, " = <unknown:%d>\n\r", pVar->type ); break;
            }
            to_actor( buf, ch );
            }
        }
        if ( !fFound )
        to_actor( "No variables.\n\r", ch );
        return;            
        
    }

    if ( NPC(victim) )
    {
        bool fFound = FALSE;
        INSTANCE *pTrig;
        VARIABLE *pVar;

        ch->userdata->trackscr = (void *)victim;
        ch->userdata->trackscr_type = TYPE_ACTOR;

        sprintf( buf, "Tracking %s in %s, scripts:\n\r", NAME(victim),
                                                       victim->in_scene->name );
        to_actor( buf, ch );

        for ( pTrig = victim->instances; pTrig != NULL;  pTrig = pTrig->next )
        {
            fFound = TRUE;

            sprintf( buf, "-==[WATCHING]==---[%5d] %s (%d, aw:%d)\n\r%s",
                     pTrig->script->dbkey, pTrig->script->name,
                     pTrig->wait, pTrig->autowait,
                     pTrig->location ? pTrig->location : "" );
            to_actor( buf, ch );

            for ( pVar = pTrig->locals;  pVar != NULL;  pVar = pVar->next )
            {
            sprintf( buf, "  [%2d] %s ", pVar->type, pVar->name );
            to_actor( buf, ch );

            switch ( pVar->type )
            {
                case TYPE_STRING:
sprintf( buf, " = \"%s\"\n\r", (char *)(pVar->value) ); break;
                case TYPE_ACTOR:    
sprintf( buf, " = a:%s\n\r", NAME( (PLAYER *)(pVar->value) ) ); break;
                case TYPE_PROP:    
sprintf( buf, " = p:%d\n\r", ((PROP *)(pVar->value))->pIndexData->dbkey );
 break;
                case TYPE_SCENE:   
sprintf( buf, " = s:%d\n\r", ((SCENE *)(pVar->value))->dbkey ); 
 break;
                       default:   sprintf( buf, " = <unknown:%d>\n\r", pVar->type ); break;
            }
            to_actor( buf, ch );
            }
        }
        if ( !fFound )
        to_actor( "No variables.\n\r", ch );
    }

    return;
}

/*
 * Godspeak.  Tests scripting language for debugging purposes.
 */  
void cmd_gspeak( PLAYER *ch, char *argument )
{
      VARIABLE *var; 
      char arg[MAX_STRING_LENGTH];
      char arg1[MAX_STRING_LENGTH];
      char arg2[MAX_STRING_LENGTH];
      char *p;

      p = argument;
      argument = one_argument( argument, arg ); 
      argument = one_argument( argument, arg1 ); 
      argument = one_argument( argument, arg2 ); 

      if ( arg[0] == '\0' || *p == '\0' ) {
           INSTANCE *pInst, *pInstNext;

           to_actor( "Any scripts running on yourself are now halted.\n\r", ch );
           to_actor( "To test an expression, try .expression()\n\r", ch );
           to_actor( "To run a script on yourself, type .run dbkey\n\r", ch );

           for ( pInst = ch->instances;  pInst != NULL;  pInst = pInstNext )
           { pInstNext = pInst->next;  free_instance( pInst ); }
           ch->instances = NULL;
           return;
      }

      if ( !str_cmp( arg, "run" ) && ch->instances == NULL ) {
           SCRIPT *pScript;

           pScript = get_script( atoi(arg1) );
           if ( pScript == NULL ) {
                to_actor( p, ch );
                to_actor( " is not a valid script.\n\r", ch );
                return;
           }
/*
 * Add fake variables and the instance.
 */ 
           ch->instances = new_instance();
           ch->instances->script = pScript;
           ch->instances->next   = NULL;
           ch->instances->locals = NULL;
           ch->instances->location = pScript->commands;
           ch->current = ch->instances;

           var = new_var( );  /* temporary variable */

           var->type  = TYPE_ACTOR;
           var->value = (PLAYER *)ch;
           assign_var( ch, TYPE_ACTOR, var, "%actor%" );

           var->type = TYPE_STRING;
           var->value = str_dup( NAME(ch) );
           assign_var( ch, TYPE_ACTOR, var, "%name%" );
           free_string((char *)(var->value));

           var->type = TYPE_PROP;
           var->value = ch->carrying;
           assign_var( ch, TYPE_ACTOR, var, "%prop%" );

           var->type = TYPE_STRING;
           var->value = str_dup( arg1 );
           assign_var( ch, TYPE_ACTOR, var, "%astr%" );

           var->type = TYPE_STRING;
           var->value = str_dup( arg2 );
           assign_var( ch, TYPE_ACTOR, var, "%bstr%" );

           free_variable( var );      /* free temporary variable */
           return;
      }

      var = eval_function( ch, TYPE_ACTOR, p );
      if ( var != NULL ) {
       to_actor( "Expression:\n\r\n\r", ch );
       to_actor( p, ch );
       to_actor( "\n\r\n\rEvaluates to:\n\r\n\r", ch );
       to_actor( var->type == TYPE_STRING ? var->value : 
                     var->type == TYPE_ACTOR ? STR((PLAYER *)(var->value),name) : 
                     var->type == TYPE_PROP ? STR((PROP *)(var->value), short_descr) :
                     var->type == TYPE_SCENE ? ((SCENE *)(var->value))->name :
                      "(unknown)", ch );
       to_actor( "\n\r", ch ); 
       free_variable( var );
      } else to_actor( "Evaluated to NULL\n\r", ch );
      return;
}

/*
 * Script Function-related algorithms.
 */

/* Kludgy work-around.  extern param_buf[MSL];  in functions.c */
char param_buf[MSL];

/*
 * Convert a variable reference (%somevar%) or a string variable
 * (the most common type) into its value.
 *
 * This is used in converting script function parameters into
 * a string, but relies on the translate_variables function which
 * returns a string that has been str_dup'd.  Used in all math
 * functions.
 */

/*
 * Convert a variable to its string equivalent.
 */
void STR_PARAM_parse( VARIABLE *var, char *_val, void *owner, int type ) {
    char *p;
    int i;

    if ( !var ) strcpy(param_buf,"0" );
    else 
    if ( var->type== TYPE_STRING) {
    p=(char *) (var->value);
    if ( *p == '%' ) {
     VARIABLE *v;
     char b[256];  i=0;
     while ( *p != '\0' ) if ( *p != '%' ) b[i++]=*(p++); else p++;
     b[i]='\0';
     v=find_variable(owner,type,b);
     if ( v && v->type == TYPE_STRING ) {
      strcpy(_val,(char *) (v->value) );
      return;
     }
    }
    p = translate_variables( owner, type,
              strip_curlies((char *)(var->value)) );
    for ( i=0; i < MSL; i++ ) { param_buf[i]=*p; p++; }
    } 
    else
    if ( var && var->type != TYPE_STRING ) {
       if ( var->value ) strcpy(param_buf, "1");
       else strcpy(param_buf,"0");
    }

    strcpy(_val,param_buf);
    return;
}

/*
 * Return a target owner information based on dbkey and type,
 * such that for scenes you would have R340 and props O514 etc.
 */
void * get_target( void * owner, int type, int * target_type, char *exp )
{
    int dbkey;

	switch (  *exp	)
	{
		case 'r':
        case 'R': *target_type = TYPE_SCENE;
				  if ( type == TYPE_SCENE ) return owner;
                  if ( type == TYPE_ACTOR  ) return ((PLAYER *)owner)->in_scene;
                  if ( type == TYPE_PROP  ) return ((PROP *)owner)->in_scene;
				  break;
		case 'o':
        case 'O': *target_type = TYPE_PROP;  break;
		case 'm':
        case 'M': *target_type = TYPE_ACTOR;  break;
		 default: return NULL;
	}

	exp++;

	dbkey = atoi(exp);

	if ( type == TYPE_PROP )
	{
		PROP *cprop = (PROP *) owner;

        if ( *target_type == TYPE_PROP )
		{
			PROP *prop;

			for ( prop = cprop->in_prop;
				  prop != NULL && prop->pIndexData->dbkey != dbkey;
				  prop = prop->next_content );

			if ( prop != NULL ) return prop;

			for ( prop = cprop->in_scene->contents;
				  prop != NULL && prop->pIndexData->dbkey != dbkey;
				  prop = prop->next_content );

			if ( prop != NULL ) return prop;

			for ( prop = cprop->contains;
				  prop != NULL && prop->pIndexData->dbkey != dbkey;
				  prop = prop->next_content );

			if ( prop != NULL ) return prop;

			for ( prop = prop_list;
				  prop != NULL && prop->pIndexData->dbkey != dbkey;
				  prop = prop->next );

			return prop;
		}

        if ( *target_type == TYPE_ACTOR )
		{
			PLAYER *actor;

			for ( actor = cprop->carried_by->in_scene->people;
				  actor != NULL && NPC(actor) && actor->pIndexData->dbkey != dbkey;
				  actor = actor->next_in_scene );

            if ( actor != NULL ) return actor;

			for ( actor = actor_list;
				  actor != NULL && NPC(actor) && actor->pIndexData->dbkey != dbkey;
				  actor = actor->next );

			return actor;
		}
	}

	if ( type == TYPE_ACTOR )
	{
        PLAYER *cactor = (PLAYER *) owner;

        if ( *target_type == TYPE_PROP )
		{
			PROP *prop;

			for ( prop = cactor->carrying;
				  prop != NULL && prop->pIndexData->dbkey != dbkey;
				  prop = prop->next_content );

			if ( prop != NULL ) return prop;

			for ( prop = cactor->in_scene->contents;
				  prop != NULL && prop->pIndexData->dbkey != dbkey;
				  prop = prop->next_content );

			if ( prop != NULL ) return prop;

			for ( prop = prop_list;
				  prop != NULL && prop->pIndexData->dbkey != dbkey;
				  prop = prop->next );

			return prop;
		}

        if ( *target_type == TYPE_ACTOR )
		{
			PLAYER *actor;

			for ( actor = cactor->in_scene->people;
				  actor != NULL && NPC(actor) && actor->pIndexData->dbkey != dbkey;
				  actor = actor->next_in_scene );

            if ( actor != NULL ) return actor;

			for ( actor = actor_list;
				  actor != NULL && NPC(actor) && actor->pIndexData->dbkey != dbkey;
				  actor = actor->next );

			return actor;
		}
	}

	return NULL;
}




/*
 * For the in-line parser.
 */
void mini_parse_script( void * owner, int type, char *exp )
{
    char buf[MAX_STRING_LENGTH];
    char *commands;

    while ( *exp != '\0' )
    {
        /*
         * Advance a line.
         */
        exp = one_line( exp, buf );
        commands = skip_spaces( buf );

        if ( *commands == '*' || *commands == '\0' )
        continue;

        if ( *commands == '%' )
        parse_assign( commands, owner, type );
        else
        {
            VARIABLE *value;

            value = eval_function( owner, type, commands );
            free_variable( value );
        }
    }

    return;
}





void instance_track( INSTANCE *instance, void * owner ) {
{
    VARIABLE *pVar;
    PLAYER *bch;
    char buf[MSL];
    char *p;

    for ( bch = actor_list;  bch != NULL;  bch = bch->next )
    {
            if ( bch->userdata == NULL )
                continue;

            if ( bch->userdata->trackscr != owner )
                continue;

//            if ( instance->wait > 0 ) continue;

            /*
             * Isolate one line of code.
             */

            clrscr(bch);
            display_interp( bch, "^2" );
            ansi_color( BOLD, bch );

            p = instance->script->commands;
            while ( p != instance->location ) {
                sprintf( buf, "%c", *p++ );
                to_actor( buf, bch );
            }

            display_interp( bch, "^7" );
            display_interp( bch, "^F" );
            to_actor("_", bch );  /* cursor location */
            display_interp( bch, "^N" );
            display_interp( bch, "^2" );
            to_actor( instance->location, bch );
            display_interp( bch, "^6" );

            sprintf( buf, 
"_______[%5d] %s (%d/aw:%d) Last-If: %d_______________\n\r",
                     instance->script->dbkey, 
                     instance->script->name,
                     instance->wait, instance->autowait, 
                     instance->last_conditional );
            to_actor( buf, bch );
            display_interp( bch, "^N" );
            display_interp( bch, "^4" );

            if ( !instance->locals )
            to_actor( "No local variables defined.\n\r", bch );

            for ( pVar = instance->locals;  pVar != NULL;  pVar = pVar->next )
            {
                sprintf( buf, "  [%2d] %s ", pVar->type, pVar->name );
                to_actor( buf, bch );

                switch ( pVar->type )
                {
                    case TYPE_STRING: 
sprintf( buf, " = \"%s\"\n\r", (char *)(pVar->value) ); break;
                    case TYPE_ACTOR:    
sprintf( buf, " = a:%s\n\r", NAME( (PLAYER *)(pVar->value) ) ); break;
                    case TYPE_PROP:    
sprintf( buf, " = p:%d\n\r", ((PROP  *)(pVar->value))->pIndexData->dbkey ); break;
                    case TYPE_SCENE:   
sprintf( buf, " = s:%d\n\r", ((SCENE *)(pVar->value))->dbkey ); break;
                    default:          
sprintf( buf, " = <unknown:%d>\n\r", pVar->type ); break;
                }
                to_actor( buf, bch );
            }
            display_interp( bch, "^N" );
   }
   return;
}
}
