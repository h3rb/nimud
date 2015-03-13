/******************************************************************************
 * Locke's   __                      __         NIM Server Software           *
 * ___ ___  (__)__    __ __   __ ___|  | v5     Version 5 (ALPHA)             *
 * |  /   \  __|  \__/  |  | |  |      |        unreleased+revamped 2004      *
 * |       ||  |        |  \_|  | ()   |                                      *
 * |    |  ||  |  |__|  |       |      |                                      *
 * |____|__|___|__|  |__|\___/__|______|        http://www.nimud.org/nimud    *
 *   n a m e l e s s i n c a r n a t e          dedicated to chris cool       *
 ******************************************************************************
 *                                                                           *
 * Within this code you will find a lot of variables that seem to be setting *
 * a pointer only to use the pointer and throw it away.  (See func_dream)    *
 *                                                                           *
 * This happens because the compile requires us to dereference only once,and *
 * it causes itself confusion because the compiler aren't able tell if you are 
 * referring to its original void * or some void * down a chain of -> ->'s   *
 *                                                                           *
 * Wierd, huh?  -locke                                                       *
 *                                                                           *
 *****************************************************************************/

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
#include "net.h"
#include "functions.h"
#include "script.h"
#include "edit.h"
#include "defaults.h"

// URL prepended to all sound() calls:
#define SOUND_WEB  "http://nimud.divineright.org/sounds/"

/* Kludgy work-around. */
extern char param_buf[MSL];
extern PROP_DATA *pPropFreeList;
extern PLAYER_DATA *pActorFreeList;

extern int gotoloops;  /* for stopping infinitely recursive scripts */

/* -------------------------------------------------------------*/

/*
 * Scene-based value functions
 */

dSET_FUNCTIONSS(func_setscenetitle, name);
dGET_FUNCTIONSS(func_scenetitle,    name);
dSET_FUNCTIONSS(func_setscenedesc,  description);
dGET_FUNCTIONSS(func_scenedesc,     description);

dSET_FUNCTIONSI(func_setscenepos, position);
dGET_FUNCTIONSI(func_scenepos,    position);
dSET_FUNCTIONSI(func_settemplate, template);
dGET_FUNCTIONSI(func_template,    template);
dSET_FUNCTIONSI(func_setmaxpeople,max_people);
dGET_FUNCTIONSI(func_maxpeople,   max_people);
dSET_FUNCTIONSI(func_setlight,    light);
dGET_FUNCTIONSI(func_light,       light);
dSET_FUNCTIONSI(func_setsector,   sector_type);
dGET_FUNCTIONSI(func_sector,      sector_type);  /* add SB */
dSET_FUNCTIONSI(func_setterrain,  terrain);
dGET_FUNCTIONSI(func_terrain,     terrain);
dSET_FUNCTIONSI(func_setwagon,    wagon);
dGET_FUNCTIONSI(func_wagon,       wagon);

/*
 * Actor-based value functions. 
 * func(target); or func(target,value);
 */

dSET_FUNCTIONAS(func_setactorname,   name);
dGET_FUNCTIONAS(func_actorname,      name);
dSET_FUNCTIONAS(func_setdescription, description);
dGET_FUNCTIONAS(func_description,    description);
dSET_FUNCTIONAS(func_setshort,       short_descr);
dGET_FUNCTIONAS(func_short,          short_descr);
dSET_FUNCTIONAS(func_setlong,        long_descr);
dGET_FUNCTIONAS(func_long,           long_descr);

dSET_FUNCTIONAI(func_setposition, position);
dGET_FUNCTIONAI(func_position,    position);
dSET_FUNCTIONAI(func_setkarma,    karma);
dGET_FUNCTIONAI(func_karma,       karma);

dSET_FUNCTIONAI(func_setmodstr,   mod_str);
dGET_FUNCTIONAI(func_modstr,      mod_str);
dSET_FUNCTIONAI(func_setmodint,   mod_int);
dGET_FUNCTIONAI(func_modint,      mod_int);
dSET_FUNCTIONAI(func_setmoddex,   mod_dex);
dGET_FUNCTIONAI(func_moddex,      mod_dex);
dSET_FUNCTIONAI(func_setmodcon,   mod_con);
dGET_FUNCTIONAI(func_modcon,      mod_con);
dSET_FUNCTIONAI(func_setmodwis,   mod_wis);
dGET_FUNCTIONAI(func_modwis,      mod_wis);

dSET_FUNCTIONAI(func_setarmor,    armor);
dGET_FUNCTIONAI(func_armor,       armor);

dSET_FUNCTIONAI(func_sethitroll,  hitroll);
dGET_FUNCTIONAI(func_hitroll,     hitroll);
dSET_FUNCTIONAI(func_setdamroll,  damroll);
dGET_FUNCTIONAI(func_damroll,     damroll);

dSET_FUNCTIONAI(func_setexp,      exp);
dGET_FUNCTIONAI(func_exp,         exp);



/*
 * Prop-based value functions.
 */
dSET_FUNCTIONPS(func_setpropname,  name);
dGET_FUNCTIONPS(func_propname,     name);
dSET_FUNCTIONPS(func_setpropdesc,  real_description);
dGET_FUNCTIONPS(func_propdesc,     real_description);
dSET_FUNCTIONPS(func_setpropshort, short_descr);
dGET_FUNCTIONPS(func_propshort,    short_descr);
dSET_FUNCTIONPS(func_setproplong,  description);
dGET_FUNCTIONPS(func_proplong,     description);
dSET_FUNCTIONPS(func_setpropaction,action_descr);
dGET_FUNCTIONPS(func_propaction,   action_descr);

dSET_FUNCTIONPI(func_setv1,    value[0]);
dGET_FUNCTIONPI(func_v1,       value[0]);
dSET_FUNCTIONPI(func_setv2,    value[1]);
dGET_FUNCTIONPI(func_v2,       value[1]);
dSET_FUNCTIONPI(func_setv3,    value[2]);
dGET_FUNCTIONPI(func_v3,       value[2]);
dSET_FUNCTIONPI(func_setv4,    value[3]);
dGET_FUNCTIONPI(func_v4,       value[3]);

dSET_FUNCTIONPI(func_setitemtype,   item_type);
dGET_FUNCTIONPI(func_itemtype,      item_type);
dSET_FUNCTIONPI(func_setextraflags, extra_flags); /* SB */
dGET_FUNCTIONPI(func_extraflags,    extra_flags);
dSET_FUNCTIONPI(func_setwearloc,    wear_loc);
dGET_FUNCTIONPI(func_wearloc,       wear_loc);
dSET_FUNCTIONPI(func_setweight,     weight);      /* add crosstyping */
dGET_FUNCTIONPI(func_weight,        weight);      /* actors=100stones*/
dSET_FUNCTIONPI(func_setcost,       cost);
dGET_FUNCTIONPI(func_cost,          cost);
dSET_FUNCTIONPI(func_settimer,      timer);
dGET_FUNCTIONPI(func_timer,         timer);

dSET_FUNCTIONPI(func_setsize,    size);           /* add crosstyping */
dGET_FUNCTIONPI(func_size,       size);

/*
 * Global function declarations for setting global variables
 */

dGET_FUNCTIONI(func_month,   weather_info.month); 
dSET_FUNCTIONI(func_setmonth,weather_info.month);

dGET_FUNCTIONI(func_day,     weather_info.day); 
dSET_FUNCTIONI(func_setday,  weather_info.day);

dGET_FUNCTIONI(func_dayofweek,    (weather_info.day%7) ); 

dGET_FUNCTIONI(func_time,    weather_info.hour); 
dSET_FUNCTIONI(func_settime, weather_info.hour);

dGET_FUNCTIONI(func_year,    weather_info.year); 
dSET_FUNCTIONI(func_setyear, weather_info.year);

dGET_FUNCTIONI(func_sky,     weather_info.sky); 
dSET_FUNCTIONI(func_setsky,  weather_info.sky);

dGET_FUNCTIONI(func_moon,    weather_info.moon_phase); 
dSET_FUNCTIONI(func_setmoon, weather_info.moon_phase);

dGET_FUNCTIONI(func_temperature,     weather_info.temperature); 
dSET_FUNCTIONI(func_settemperature,  weather_info.temperature);

dGET_FUNCTIONI(func_phase,   weather_info.next_phase); 
dSET_FUNCTIONI(func_setphase,weather_info.next_phase);

/*
 * For access to math functions
 */

dMATH_FUNCTIONI(func_not,       (!x)  );
dMATH_FUNCTIONI(func_or,      (x || y));
dMATH_FUNCTIONI(func_and,     (x && y));
dMATH_FUNCTIONI(func_less,    (x <  y));
dMATH_FUNCTIONI(func_lte,     (x <= y));
dMATH_FUNCTIONI(func_greater, (x >  y));
dMATH_FUNCTIONI(func_gte,     (x >= y));
dMATH_FUNCTIONI(func_bor,     (x |  y));
dMATH_FUNCTIONI(func_xor,     (x ^  y));
dMATH_FUNCTIONI(func_bxor,    (x ^  y));
dMATH_FUNCTIONI(func_band,    (x &  y));
dMATH_FUNCTIONI(func_add,     (x +  y));
dMATH_FUNCTIONI(func_mult,    (x *  y));
dMATH_FUNCTIONI(func_sub,     (x -  y));
dMATH_FUNCTIONI(func_mod,     (x %  y));
dMATH_FUNCTIONI(func_div,     (y != 0 ? (x / y) : 0) );
dMATH_FUNCTIONI(func_inc,     (x +  1));
dMATH_FUNCTIONI(func_dec,     (x -  1));

dMATH_FUNCTIONI(func_eq,      (x == y));

dMATH_FUNCTIONI(func_random,  bstr?number_range(x,y):number_range(0,x));
dMATH_FUNCTIONI(func_percent, number_percent() );
dMATH_FUNCTIONI(func_bits,    number_bits(x)   );
dMATH_FUNCTIONI(func_fuzzy,   number_fuzzy(x)  );
dMATH_FUNCTIONI(func_gelify,  number_gelify(x) );
dMATH_FUNCTIONI(func_dice,    dice(x,y));

dMATH_FUNCTIONI(func_interpolate,  interpolate(x,y,z) );
dMATH_FUNCTIONI(func_INTERPOLATE,  INTERPOLATE(x,y,z) );
dMATH_FUNCTIONI(func_min,          UMIN(x,y)          );
dMATH_FUNCTIONI(func_max,          UMAX(x,y)          );
dMATH_FUNCTIONI(func_urange,       URANGE(x,y,z)      );
dMATH_FUNCTIONI(func_range,        (x <= y && y <= z) );
dMATH_FUNCTIONI(func_lineate,      LINEATE(x,y,z)     );
dMATH_FUNCTIONI(func_ratio,        PERCENTAGE(x,y)    );

/*
 * For access to functions in graphics.c
 */ 

dGRAPHICS_FUNCTION(func_box,       box(a,b,c,d)         );
dGRAPHICS_FUNCTION(func_rect,      rect(a,b,c,d)        );
dGRAPHICS_FUNCTION(func_point,     point(a,b)           );
dGRAPHICS_FUNCTION(func_hline,     hline(a,b,c)         );
dGRAPHICS_FUNCTION(func_tline,     textline(a,b,c,d,_e) );
dGRAPHICS_FUNCTION(func_vline,     vline(a,b,c)         );
dGRAPHICS_FUNCTION(func_circle,    circle(a,b,c)        );
dGRAPHICS_FUNCTION(func_line,      line(a,b,c,d)        );
dGRAPHICS_FUNCTION(func_gmode,     gmode(a,b)           );
dGRAPHICS_FUNCTION(func_pageclear, clear_page()         );
dGRAPHICS_FUNCTION(func_text,      text( a, b, _c )     );
dGRAPHICS_FUNCTION(func_stroke,    stroke=a             );
dGRAPHICS_FUNCTION(func_brush,     paintbrush = is_number(_a) ? atoi(_a)%255 :
                                             (_a[0] == '\0' ? ' ' : _a[0]) );
dGRAPHICS_FUNCTION(func_style,     style=a              );
dGRAPHICS_FUNCTION(func_mode,      mode=a               );

dGRAPHICS_FUNCTION(func_popup,     popup(a,b,c,d,_e)    );
dGRAPHICS_FUNCTION(func_button,    button(a,b,_c)       );
dGRAPHICS_FUNCTION(func_fill,      fill(a,b,c,d)        );
dGRAPHICS_FUNCTION(func_fills,     fills(a,b,_c)        );
dGRAPHICS_FUNCTION(func_column,    column(a,b,c,d,_e)   );
dGRAPHICS_FUNCTION(func_wrap,  RETURNS(TYPE_STRING,str_dup(wrap(a,_b,_c))) );
dGRAPHICS_FUNCTION(func_color, RETURNS(TYPE_STRING,str_dup(color(stroke))) );
dGRAPHICS_FUNCTION(func_ruler,     ruler=a              ); 
dGRAPHICS_FUNCTION(func_rulers, if ( ruler ) 
                              {RETURNS(TYPE_STRING,str_dup("1"));} );

dGRAPHICS_FUNCTION(func_clear,     clrscr( type == TYPE_ACTOR ? ACTOR(owner) :
                                  ((va && va->type == TYPE_ACTOR) ? 
                                   ACTOR(va->value) : NULL )) );

dGRAPHICS_FUNCTION(func_draw,      draw_to_buffer( type == TYPE_ACTOR ? 
                               ACTOR(owner) :
                                  ((va && va->type == TYPE_ACTOR) ? 
                                   ACTOR(va->value) : NULL )) );

#define V  VARIABLE_DATA

/*
 * Locals
 */

int prop_where( PROP_INDEX_DATA *p );

/*
 * MSP audio-related functions
 */

VARIABLE_DATA *func_check( void *owner, int type, V *n ) {
  char _v[MSL];
  STR_PARAM(n,_v);
  if ( number_range(0,100) < atoi(_v) ) {
  RETURNS(TYPE_STRING, str_dup("1"));
  } else {
  RETURNS(TYPE_STRING, str_dup("0"));
  }  
}


VARIABLE_DATA *func_sound( void *owner, int type, V *n, V *t, V *v, V * p, V *l, V *b ) {
   char buf[MSL];
   char _n[MSL]; // name
   char _t[MSL]; // target or type
   char _v[MSL]; // volume
   char _p[MSL]; // parameter
   char _l[MSL]; // loops
   char _b[MSL]; // balance

   PLAYER_DATA *a=NULL;
   SCENE_INDEX_DATA *s=NULL;
   PROP_DATA *o=NULL;
 
   int loops;

   STR_PARAM(v, _v);
   STR_PARAM(p, _p);
   STR_PARAM(b, _b);
   STR_PARAM(n, _n);
   STR_PARAM(l, _l);
   
   loops = atoi(_l);  if ( loops == 0 ) loops=1; if ( loops < 0 ) loops=-1;
   snprintf( buf, MSL, "%s\r!!SOUND %s P=%s L=%d T=nimscripts V=%s%s U=%s%s \r%s", 
                       "^N^0", _n, _p, loops, _v, LOWER(*_b) == 'l' ? ",left" : 
                                                 LOWER(*_b) == 'r' ? ",right" : " ", SOUND_WEB, _n, "^N" );

   if ( t->type == TYPE_ACTOR ) {
        a=ACTOR(t->value);
        if ( a ) {
            if ( !IS_NPC(a) && IS_SET(a->act2,PLR_MSP) ) send_to_actor( buf, a );
            return NULL;
        }
   }
   else
   if ( t->type == TYPE_SCENE ) {
        s=SCENE(t->value);
        if ( s ) {
            for ( a = s->people; a!=NULL; a=a->next ) 
            if ( !IS_NPC(a) && IS_SET(a->act2,PLR_MSP) ) send_to_actor( buf, a );
        }
   }
   else
   if ( t->type == TYPE_PROP ) {
        o=PROP(t->value);
        if ( o->item_type == ITEM_FURNITURE ) {
        } else
        if ( t ) {
            a=o->carried_by;
            if ( a ) { send_to_actor( buf, a ); return NULL; }
            s=o->in_scene;
            if ( s ) { 
             for ( a = s->people; a!=NULL; a=a->next ) 
            if ( !IS_NPC(a) && IS_SET(a->act2,PLR_MSP) ) send_to_actor( buf, a );
            }
            o = o->in_prop;
            if ( o ) {
             int vol=atoi(_v)/3; // muffled sound

   snprintf( buf, MSL, "%s\r!!SOUND %s P=%s L=%d T=nimscripts V=%d%s U=%s%s \r%s", 
                       "^N^0", _n, _p, loops, vol, LOWER(*_b) == 'l' ? ",left" : 
                                                  LOWER(*_b) == 'r' ? ",right" : " ", SOUND_WEB, _n, "^N" );

             a=o->carried_by;
             if ( a ) { send_to_actor( buf, a ); return NULL; }
             s=o->in_scene;
             if ( s ) { 
               for ( a = s->people; a!=NULL; a=a->next ) 
                 if ( !IS_NPC(a) && IS_SET(a->act2,PLR_MSP) ) send_to_actor( buf, a );
              }
            }
        }
   }
   else {
      STR_PARAM(t, _t);
      if (type == TYPE_ACTOR) {
        a=get_actor_world( ACTOR(owner), _t );
      } else
      if ( !str_cmp(_t,"me") ) {
        if (type==TYPE_ACTOR) {
             send_to_actor( buf, a );
        }
      } else
      if ( !str_cmp(_t,"all") ) {
      }
   } 

   return NULL;
}

VARIABLE_DATA *func_music( void *owner, int type, V *n, V *t, V *v, V * p, V *l, V *b ) {
   char buf[MSL];
   char _n[MSL]; // name
   char _t[MSL]; // target or type
   char _v[MSL]; // volume
   char _p[MSL]; // parameter
   char _l[MSL]; // loops
   char _b[MSL]; // balance

   PLAYER_DATA *a=NULL;
   SCENE_INDEX_DATA *s=NULL;
   PROP_DATA *o=NULL;
 
   int loops;

   STR_PARAM(v, _v);
   STR_PARAM(p, _p);
   STR_PARAM(b, _b);
   STR_PARAM(n, _n);
   STR_PARAM(l, _l);
   
   loops = atoi(_l);  if ( loops == 0 ) loops=1; if ( loops < 0 ) loops=-1;
   snprintf( buf, MSL, "%s\r!!MUSIC %s P=%s L=%d T=nimscripts V=%s%s U=%s%s \r%s", 
                       "^N^0", _n, _p, loops, _v, LOWER(*_b) == 'l' ? ",left" : 
                                                 LOWER(*_b) == 'r' ? ",right" : " ", SOUND_WEB, _n, NTEXT );

   if ( t->type == TYPE_ACTOR ) {
        a=ACTOR(t->value);
        if ( a ) {
            if ( !IS_NPC(a) && IS_SET(a->act2,PLR_MSP) ) send_to_actor( buf, a );
            return NULL;
        }
   }
   else
   if ( t->type == TYPE_SCENE ) {
        s=SCENE(t->value);
        if ( s ) {
            for ( a = s->people; a!=NULL; a=a->next ) 
            if ( !IS_NPC(a) && IS_SET(a->act2,PLR_MSP) ) send_to_actor( buf, a );
        }
   }
   else
   if ( t->type == TYPE_PROP ) {
        o=PROP(t->value);
        if ( o->item_type == ITEM_FURNITURE ) {
        } else
        if ( t ) {
            a=o->carried_by;
            if ( a ) { send_to_actor( buf, a ); return NULL; }
            s=o->in_scene;
            if ( s ) { 
             for ( a = s->people; a!=NULL; a=a->next ) 
            if ( !IS_NPC(a) && IS_SET(a->act2,PLR_MSP) ) send_to_actor( buf, a );
            }
            o = o->in_prop;
            if ( o ) {
             int vol=atoi(_v)/3; // muffled sound

   snprintf( buf, MSL, "%s\r!!MUSIC %s P=%s L=%d T=nimscripts V=%d%s U=%s%s \r%s", 
                       "^N^0", _n, _p, loops, vol, LOWER(*_b) == 'l' ? ",left" : 
                                                  LOWER(*_b) == 'r' ? ",right" : " ", SOUND_WEB, _n, NTEXT );

             a=o->carried_by;
             if ( a ) { send_to_actor( buf, a ); return NULL; }
             s=o->in_scene;
             if ( s ) { 
               for ( a = s->people; a!=NULL; a=a->next ) 
                 if ( !IS_NPC(a) && IS_SET(a->act2,PLR_MSP) ) send_to_actor( buf, a );
              }
            }
        }
   }
   else {
      STR_PARAM(v, _t);
      if (type == TYPE_ACTOR) {
        a=get_actor_world( ACTOR(owner), _t );
      } else
      if ( !str_cmp(_t,"all") ) {
      }
   } 

   return NULL;
}



/* TODO: pi? floats? */

/* -------------------------------------------------------------*
 * begin custom functions                                       *
 * -------------------------------------------------------------*/


/*
 * Don't do anything, but provide a place for goto to go to.
 */
VARIABLE_DATA *func_label( void * owner, int type )
{
    return NULL;
}



/* -------------------------------------------------------------*/
/*
 * Goto a specified label.
 */
VARIABLE_DATA *func_goto( void * owner, int type, VARIABLE_DATA *label )
{
    INSTANCE_DATA *instance = PARSING(owner,type);
    char buf[MAX_STRING_LENGTH];
    char _label[MSL];
    char *exp;
    char *l;
    char *slabel;

    STR_PARAM(label,_label);

    exp = translate_variables( owner, type, _label );
    slabel = str_dup( exp );  

    l = instance->script->commands;
    while ( !MTD(l) )
    {
        /*
         * Advance a line.
         */
        l = one_line( l, buf );
        exp = skip_spaces( buf );

        if ( LOWER(*exp) == 'l' )
        {
            int param_paren = 0;
            char name[MAX_STRING_LENGTH];
            char *p;

            /*
             * Grab the function name.
             * func(param, .... )
             * ^
             */
            p = name;
            while ( *exp != '('
                 && *exp != ' '
                 && *exp != '\0' ) *p++ = *exp++;
            *p = '\0';
            exp = skip_spaces( exp );
            if ( *exp != '\0' ) exp++;

            if ( str_cmp( name, "label" ) )
            continue;

            p = name;
            while( *exp != '\0' )
            {
                /*
                 * Ok, get the hell out of here, we're done.
                 * func(param, .... )
                 *           ^
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

                if ( *exp == ' ' ) exp++;
                else *p++ = *exp++;
            }
            *p = '\0';

            p = translate_variables(owner, type, name);
            if ( !str_cmp( p, slabel ) )
            {
                instance->location = l;
                break;
            } 
        }
    }

    free_string(slabel);

    if ( ++gotoloops > MAX_LOOPS )
    {
        NOTIFY( "Notify>  Exceeded maximum gotos per script (Halted).\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
        instance->location = NULL;
    }
    return NULL;
}



/* -------------------------------------------------------------*/
/*
 * Goto a specified label.
 */
VARIABLE_DATA *func_ifgoto( void * owner, int type, VARIABLE_DATA *a, VARIABLE_DATA *yes, VARIABLE_DATA *no )
{
    INSTANCE_DATA *instance = PARSING(owner,type);
    char buf[MAX_STRING_LENGTH];
    char _a[MSL];
    char _yes[MSL];
    char _no[MSL];
    char *exp;
    char *l;
    char *slabel;

    STR_PARAM(yes,_yes);
    STR_PARAM(no,_no);
    STR_PARAM(a,_a);

    exp=translate_variables_noliterals(owner,type,_a);
    
    slabel = str_dup( (atoi(_a)) ? _yes : _no );  

    l = instance->script->commands;
    while ( !MTD(l) )
    {
        /*
         * Advance a line.
         */
        l = one_line( l, buf );
        exp = skip_spaces( buf );

        if ( LOWER(*exp) == 'l' )
        {
            int param_paren = 0;
            char name[MAX_STRING_LENGTH];
            char *p;

            /*
             * Grab the function name.
             * func(param, .... )
             * ^
             */
            p = name;
            while ( *exp != '('
                 && *exp != ' '
                 && *exp != '\0' ) *p++ = *exp++;
            *p = '\0';
            exp = skip_spaces( exp );
            if ( *exp != '\0' ) exp++;

            if ( str_cmp( name, "label" ) )
            continue;

            p = name;
            while( *exp != '\0' )
            {
                /*
                 * Ok, get the hell out of here, we're done.
                 * func(param, .... )
                 *           ^
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

                if ( *exp == ' ' ) exp++;
                else *p++ = *exp++;
            }
            *p = '\0';

            p = translate_variables(owner, type, name);
            if ( !str_cmp( p, slabel ) )
            {
                instance->location = l;
                break;
            } 
        }
    }

    free_string(slabel);

    if ( ++gotoloops > MAX_LOOPS )
    {
        NOTIFY( "Notify>  Exceeded maximum gotos per script (Halted).\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
        instance->location = NULL;
    }
    return NULL;
}




/*
 * If( expression, ==0, !=0 );
 *
 * Conditional statement.
 *
 * Returns evaluated function calls based on condition.
 */
VARIABLE_DATA *func_if( void * owner, int type,
                        VARIABLE_DATA *exp,
                        VARIABLE_DATA *iftrue,
                        VARIABLE_DATA *iffalse )
{
    int val;
    char _exp[MSL];
    char _iftrue[MSL];
    char _iffalse[MSL]; 
    char *__iftrue;
    char *__iffalse;
    char *commands;
    INSTANCE_DATA *instance = PARSING(owner,type);
    char buf[MAX_STRING_LENGTH];

    if ( !instance ) return NULL;
    
    STR_PARAM(exp, _exp);
    val = atoi(_exp);
    instance->last_conditional = val;

    if ( val > 0 )
    {
        /*
         * Evaluates as true.
         */
        STR_PARAM(iftrue,_iftrue);

        __iftrue = strip_curlies( _iftrue );

        while ( !IS_SET(instance->state, SCRIPT_HALT)
             && !MTD(__iftrue) )
        {
            /*
             * Advance a line.
             */
            __iftrue = one_line( __iftrue, buf );
            commands = skip_spaces( buf );

            if ( *commands == '\0' ) continue;

            if ( *commands == '%' )
            parse_assign( commands, owner, type );
            else
            {
                VARIABLE_DATA *value;

                value = eval_function( owner, type, commands );
                free_variable( value );
            }

            if ( instance->autowait) instance->wait = instance->autowait;
        }

    }
    else
    {
        STR_PARAM(iffalse,_iffalse);

        __iffalse = strip_curlies( _iffalse );

        while ( !IS_SET(instance->state, SCRIPT_HALT)
             && !MTD(__iffalse) )
        {
            /*
             * Advance a line.
             */
            __iffalse = one_line( __iffalse, buf );
            commands = skip_spaces( buf );
            if ( *commands == '\0' ) continue;

            if ( *commands == '%' )
            parse_assign( commands, owner, type );
            else
            {
                VARIABLE_DATA *value;  /* mem leak ? */

                value = eval_function( owner, type, commands );
                free_variable( value );
            }

            if ( instance->autowait) instance->wait = instance->autowait;
        }
    }

    RETURNS(TYPE_STRING,str_dup(_exp) );
}





/*------------------------------------------------------------*/

/*
 * Return(value)
 *
 * Set the value returned by the function.
 */
VARIABLE_DATA *func_return( void * owner, int type, VARIABLE_DATA *value )
{
    INSTANCE_DATA *instance = PARSING( owner, type );
    int x;
    char _value[MSL];

    if ( value==NULL ) {
       if ( instance->returned ) free_variable( instance->returned );
       instance->returned=NULL;
       return NULL;
    }

    STR_PARAM(value,_value);

    x = atoi(_value);

    if ( instance->returned == NULL ) 
    instance->returned = new_variable( TYPE_STRING, NULL );

    /*  
     * Must duplicate strings.
     */    
    if ( instance->returned->type == TYPE_STRING )
    instance->returned->value = str_dup( _value );
    else instance->returned->value = value->value;

    instance->returned->type = value->type;
    return NULL;
}



/*
 * Halt(errno)
 *
 * Halts a script, optionally sets the return value
 */
VARIABLE_DATA *func_halt( void * owner, int type, VARIABLE_DATA *value )
{
    INSTANCE_DATA *instance = PARSING(owner,type);
/*    VARIABLE_DATA *pVar, *pVar_next; */
    int x;
    char _value[MSL];

    STR_PARAM(value,_value);

    x = atoi(_value);
    if ( instance->returned != NULL ) free_variable(instance->returned);
    instance->returned = new_variable( TYPE_STRING, str_dup(_value) );

/*
    for ( pVar = instance->locals;  pVar != NULL; pVar = pVar_next )
    {
        pVar_next = pVar->next;
        free_variable( pVar );
    }

    instance->locals = NULL;
 */
    instance->location = NULL;

    return NULL;
}



/*
 * Permhalt()
 *
 * Halt without prettiness (for debugging purposes).
 */
VARIABLE_DATA *func_permhalt( void * owner, int type )
{
    INSTANCE_DATA *instance = PARSING(owner,type);

    instance->location = NULL;
    SET_BIT(instance->state, SCRIPT_HALT);
    return NULL;
}



/*
 * Wait(pulses);
 *
 * Delay execution by number of pulses.
 */
VARIABLE_DATA *func_wait( void * owner, int type, VARIABLE_DATA *value )
{
    INSTANCE_DATA *instance = PARSING(owner,type);
    char _value[MSL];
    int x;

    STR_PARAM(value,_value);

    x = atoi(_value);
    instance->wait = x;
    return NULL;
}




/*
 * Autowait(value);
 *
 * Set the instances' iterative delay counter value.
 */
VARIABLE_DATA *func_autowait( void * owner, int type, VARIABLE_DATA *value )
{
    INSTANCE_DATA *instance = PARSING(owner,type);
    char _value[MSL];
    int x;

    STR_PARAM(value,_value);

    x = atoi(_value);
    instance->autowait = x;
    return NULL;
}


/* -------------------------------------------------------------- */

/*
 * Install(target,vnum);
 * Install(vnum);
 *
 * Installs a new script on a scene, prop or actor target.  Great
 * for special items or curse-type scripts.  Associated function:
 * Uninstall();
 */

VARIABLE_DATA *func_install( void *owner, int type, 
                             VARIABLE_DATA *target, 
                             VARIABLE_DATA *script )
{
   INSTANCE_DATA *pInstance;
   PLAYER_DATA *ch=NULL;
   SCENE_INDEX_DATA *pScene=NULL;
   PROP_DATA *pProp=NULL;
   SCRIPT_DATA *pScript;
   char _target[MSL];
   char _script[MSL];

   if ( !target || !script ) return NULL;
   STR_PARAM(script,_script);

   if ( !(pScript = get_script_index( atoi(_script) )) ) return NULL;

   STR_PARAM(target,_target);
   FOREACH( target->type,
            ch = ACTOR(target->value),
            pProp = PROP(target->value),
            pScene = SCENE(target->value),
            FOREACH( type,
                     ch = get_actor_world( ACTOR(owner), _target ),
                     pProp = PROP(owner), 
                     pScene = SCENE(owner), return NULL ) );


   if ( !ch && !pProp && !pScene ) return NULL;

   pInstance = new_instance();
   pInstance->script = pScript;
   pInstance->location = pInstance->script->commands;

   if ( ch ) {
   pInstance->next = ch->instances;
   ch->instances = pInstance;
   } else
   if ( pProp ) {
   pInstance->next = pProp->instances;
   pProp->instances = pInstance;
   }
   else if ( pScene ) {
   pInstance->next = pScene->instances;
   pScene->instances = pInstance;
   }

   return NULL;
}

/*
 * UnInstall(target,vnum);
 * UnInstall(vnum);
 *
 * Removes all instances of a script with a certain vnum.
 */
VARIABLE_DATA *func_uninstall( void *owner, int type, 
                               VARIABLE_DATA *target,
                               VARIABLE_DATA *script ) {
   
   return NULL;
}

/* -------------------------------------------------------------- */

/*
 * Return an instance of supplied name on supplied owner.
 * Used in Call();
 */
INSTANCE_DATA *find_instance( void * owner, int type, char *trigname )
{
	INSTANCE_DATA *trig;

	switch( type )
	{
        case TYPE_ACTOR:  trig = ACTOR(owner)->instances;     break;
        case TYPE_PROP:   trig = PROP(owner)->instances;        break;
        case TYPE_SCENE:  trig = SCENE(owner)->instances; break;
         default: return NULL;
	}

	for ( ; trig != NULL;  trig = trig->next )
	{
        if ( !str_cmp( trig->script->name, trigname ) )
		return trig;
	}

	return trig;
}

/*
 * Call(vnum,target);
 * Call(name,target);
 * Call(vnum);
 * Call(name);
 * Begins another script instance by name or vnum.
 */
VARIABLE_DATA *func_call( void * owner, int type, VARIABLE_DATA *trigname,
                          VARIABLE_DATA *target )
{
    INSTANCE_DATA *instance;
    char buf[MAX_STRING_LENGTH];
    char _trigname[MSL];
    void *target_owner;
    int target_type;

    STR_PARAM(trigname,_trigname);

    target_owner = owner;
    target_type   = type;

    if ( PARAMETER(target,TYPE_STRING) )
    {
        target_owner = get_target( owner, type, &target_type, (char *) target->value );
        if ( target_owner == NULL ) return NULL;
    }

    instance = find_instance( target_owner, target_type, _trigname );

    if ( instance == NULL || instance == PARSING(owner,type) ) return NULL;

    snprintf( buf, MAX_STRING_LENGTH, "%d", trigger( owner, type, instance ) );

    RETURNS(TYPE_STRING,str_dup(buf));    
}

/* -------------------------------------------------------------- */



/*
 * Self();
 * Return a pointer to oneself.
 */
VARIABLE_DATA *func_self( void * owner, int type )
{
    RETURNS(type,owner);
}

/* -------------------------------------------------------------- */

/*
 * Foe();
 * Return a pointer to a foe.  RESERVED
 */
VARIABLE_DATA *func_foe( void * owner, int type )
{
    PLAYER_DATA *ch;
  
    if ( type != TYPE_ACTOR ) return NULL;

    ch = ACTOR(owner)->fighting;
    if ( !ch ) {
       RETURNS(TYPE_STRING,str_dup("0"));
       }
    else
    {    RETURNS(TYPE_ACTOR,ch);    }
}

/* -------------------------------------------------------------- */

/*
 * Following();
 * Return a pointer to a master.  RESERVED
 */
VARIABLE_DATA *func_following( void * owner, int type )
{
    PLAYER_DATA *ch;
  
    if ( type != TYPE_ACTOR ) return NULL;

    ch = ACTOR(owner)->master;
    if ( !ch ) {
       RETURNS(TYPE_STRING,str_dup("0"));
       }
    else
    {    RETURNS(TYPE_ACTOR,ch);    }
}

/* -------------------------------------------------------------- */


/*
 * Cmp(a,b);  
 * Is(a,b);
 * Compare two values.
 */
VARIABLE_DATA *func_cmp( void * owner, int type, VARIABLE_DATA *astr,
                         VARIABLE_DATA *bstr )
{
    static char buf[MAX_STRING_LENGTH];
    char _astr[MSL];
    char _bstr[MSL];

    STR_PARAM(astr,_astr);
    STR_PARAM(bstr,_bstr);

    if ( is_number(_astr) && is_number(_bstr) ) {
       snprintf( buf, MAX_STRING_LENGTH, "%d", atoi(_astr) == atoi(_bstr) ? 1 : 0 );
    }
    else
    snprintf( buf, MAX_STRING_LENGTH, "%d", (int) !str_cmp( _astr, _bstr ) );

    RETURNS(TYPE_STRING,str_dup(buf));    
}


/* -------------------------------------------------------------- */
/*
 * Pre(a,b);
 * Str_prefix(a,b) - returns true if a is a prefix of b
 */
VARIABLE_DATA *func_pre( void * owner, int type, VARIABLE_DATA *astr, VARIABLE_DATA *bstr )
{
    static char buf[MAX_STRING_LENGTH];
    char _astr[MSL];
    char _bstr[MSL];

    STR_PARAM(astr,_astr);
    STR_PARAM(bstr,_bstr);

    snprintf( buf, MAX_STRING_LENGTH, "%d", !str_prefix( _astr, _bstr ) );

    RETURNS(TYPE_STRING,str_dup(buf));    
}


/* -------------------------------------------------------------- */
/*
 * In(a,b);
 * Str_infix.  Returns true if a is in b
 */
VARIABLE_DATA *func_in( void * owner, int type, VARIABLE_DATA *astr, VARIABLE_DATA *bstr )
{
    char buf[MAX_STRING_LENGTH];
    char _astr[MSL];
    char _bstr[MSL];

    STR_PARAM(astr,_astr);
    STR_PARAM(bstr,_bstr);

    snprintf( buf, MAX_STRING_LENGTH, "%d", ( int ) ( (buf+strlen(buf))-strstr( _astr, _bstr ) ) );

    RETURNS(TYPE_STRING,str_dup(buf));    
}


/* -------------------------------------------------------------- */
/*
 * Seek(a,b);
 * Returns a number value of the index of a,b.
 * Strstr.
 */
VARIABLE_DATA *func_strstr( void * owner, int type, VARIABLE_DATA *astr, VARIABLE_DATA *bstr )
{
    char buf[MAX_STRING_LENGTH];
    char _astr[MSL];
    char _bstr[MSL];

    STR_PARAM(astr,_astr);
    STR_PARAM(bstr,_bstr);

    snprintf( buf, MAX_STRING_LENGTH, "%d", ( int ) ( (buf+strlen(buf))-strstr( _astr, _bstr ) ) );

    RETURNS(TYPE_STRING,str_dup(buf));    
}

/* -------------------------------------------------------------- */
/*
 * Pcmp(a,b);
 * Pointer comparisons.
 */
VARIABLE_DATA *func_pcmp( void * owner, int type, VARIABLE_DATA *astr, VARIABLE_DATA *bstr )
{
    VARIABLE_DATA *v1, *v2;
    char buf[MAX_STRING_LENGTH];

    if( PARAMETER(astr,TYPE_STRING) )
    v1 = astr;
    else
    v1 = find_variable( owner, type, strip_curlies((char *)astr->value) );

    if( PARAMETER(bstr,TYPE_STRING) )
    v2 = bstr;
    else
    v2 = find_variable( owner, type, strip_curlies((char *)bstr->value) );

    buf[0] = '\0';
    if ( v1 == v2 )               buf[0] = '1';
    if ( v1->type != v2->type )   buf[0] = '0';
    if ( v1->value == v2->value ) buf[0] = '1';
    buf[1] = '\0';

    RETURNS(TYPE_STRING,str_dup(buf));    
}

/* -------------------------------------------------------------- */
/*
 * Cat(a,b);
 * Concatenate two strings.
 */
VARIABLE_DATA *func_cat( void * owner, int type, VARIABLE_DATA *astr, VARIABLE_DATA *bstr )
{
    static char buf[MAX_STRING_LENGTH];
    char _astr[MSL];
    char _bstr[MSL];

    STR_PARAM(astr,_astr);
    STR_PARAM(bstr,_bstr);

    snprintf( buf, MAX_STRING_LENGTH, "%s", strip_curlies(_astr) );
    snprintf( buf, MAX_STRING_LENGTH, "%s%s", buf, strip_curlies(_bstr) );

    RETURNS(TYPE_STRING,str_dup(buf));    
}

/* -------------------------------------------------------------- */
/*
 * Word(a,b);
 * One_arg returning a number.
 */
VARIABLE_DATA *func_word( void * owner, int type, VARIABLE_DATA *astr, VARIABLE_DATA *value )
{
    static char buf[MAX_STRING_LENGTH];
    char _astr[MSL];
    char _value[MSL];
    char *spoint;
    int x;

    STR_PARAM(astr,_astr);
    STR_PARAM(value,_value);

    spoint = _astr;
    x = atoi(_value);

    while( x-- > 0 ) spoint = one_argument(spoint, buf);

    RETURNS(TYPE_STRING,str_dup(buf));    
}

/* -------------------------------------------------------------- */
/*
 * Echo(text);           echoes to caller
 * Echo(scene,text);     echoes to scene by vnum
 * Echo(target,text);    echoes to target
 *
 * Displays some text.
 */
VARIABLE_DATA *func_echo( void * owner, int type, VARIABLE_DATA *target, 
                           VARIABLE_DATA *out )
{
    char _out[MSL];
    char _target[MSL];
    SCENE_INDEX_DATA *pScene=NULL;
    PLAYER_DATA *rch=NULL;
    char *p;

    STR_PARAM(out,_out);
    STR_PARAM(target,_target);  // wtf_logf( "func_echo: %s, %s", _out, _target );

    if ( strlen(_out) > 0 ) { 
     p = _out;
     if ( target->type == TYPE_ACTOR ) rch = ACTOR(target->value); 
     else
     if ( target->type == TYPE_SCENE ) pScene = SCENE(target->value);
     else
     if ( target->type == TYPE_PROP ) pScene = PROP(target->value)->in_scene;
     else
     {
         rch = find_actor_here( owner, type, _target );
         if ( !rch ) pScene = find_scene_here( owner, type, _target );
         if ( !rch && !pScene && is_number( _target ) ) pScene=get_scene_index(atoi(_target));
     }
    }
    else {
          p = _target;
          switch ( type ) {
              case TYPE_SCENE:                     pScene = SCENE(owner);                break;
              case TYPE_PROP:                      pScene = PROP(owner)->carried_by ? 
                                                            PROP(owner)->carried_by->in_scene : 
                                                            PROP(owner)->in_scene;       break;
              case TYPE_ACTOR:                     rch = ACTOR(owner);                break;
                default: break;
             }
    }

    if ( !rch && !pScene && type == TYPE_PROP ) {
         rch = PROP(owner)->carried_by;
         if ( !rch ) pScene = PROP(owner)->in_scene;
    }

    p = translate_variables_noliterals( owner, type, p ); 

    if ( rch ) {
      display_interp( rch, capitalize(p) );
      send_to_actor( "\n\r", rch );
    } else
    if ( pScene ) 
    for ( rch = pScene->people; rch != NULL; rch = rch->next_in_scene ) 
     { display_interp( rch, capitalize(p) ); send_to_actor( "\n\r", rch ); } 
    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * oecho(text);
 * oecho(scene,text);
 * oecho(target,text);
 *
 * Displays some text to all but the target.
 */
VARIABLE_DATA *func_oecho( void * owner, int type, VARIABLE_DATA *target, 
                           VARIABLE_DATA *out )
{
    char _out[MSL];
    char _target[MSL];
    SCENE_INDEX_DATA *pScene=NULL;
    PLAYER_DATA *rch=NULL;
    PLAYER_DATA *nch=NULL;
    char *p;

    STR_PARAM(out,_out); p=_out;
    STR_PARAM(target,_target);  

    if ( is_number(_target) ) {
         pScene = get_scene_index( atoi(_target) );
    }
    else {         
         nch = find_actor_here( owner, type, _target ); 
         if ( nch ) { pScene = nch->in_scene; /*wtf_logf( "Valid target: %s", NAME(nch) );*/ }
    }

    if ( !pScene ) 
       return NULL;    

    p=translate_variables(owner, type, p);
   // wtf_logf( "Translated: %s", p );
    for ( rch = pScene->people;
          rch != NULL;
          rch = rch->next_in_scene ) {
     if ( rch != nch ) { display_interp( rch, p ); 
                        send_to_actor( "\n\r", rch ); }
    }
    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Transform(vnum);
 * Transform from one prop into another
 * Possible memory leak so be careful with this one (untested)
 */
VARIABLE_DATA *func_transform( void * owner, int type, VARIABLE_DATA *target )
{
    char _target[MSL];
    int vnum;
    PROP_INDEX_DATA *pPropIndex;
    PROP_DATA *pMe;

    if ( type != TYPE_PROP ) return NULL;

    STR_PARAM(target,_target);
    vnum = atoi(_target);

    pPropIndex = get_prop_index( vnum );
    if ( !pPropIndex ) return NULL;

    pMe = PROP(owner);

    if ( pPropIndex->item_type == ITEM_LIST )
    {
        PROP_INDEX_DATA *pIndex;
        char *scanarg;
        char buf[20];
        int num;
        int d;

        scanarg    = pPropIndex->description;
        num        = number_range( 1, arg_count( pPropIndex->description ) );
        buf[0]     = '\0';

        for ( d = 0; d < num; d++ )
        {
            scanarg = one_argument( scanarg, buf );
        }

        num       = atoi( buf );
        num        = number_range( 1, arg_count( pPropIndex->description ) );
        buf[0]     = '\0';

        for ( d = 0; d < num; d++ )
        {
            scanarg = one_argument( scanarg, buf );
        }

        num       = atoi( buf );
        pIndex = get_prop_index( num );
        if ( pIndex != NULL ) pPropIndex = pIndex;
    }

    pMe->pIndexData     = pPropIndex;


    /*
     * Copy triggers onto the prop.
     */
    if ( pPropIndex->instances != NULL )
    {
    INSTANCE_DATA *inst, *pInst;

    for ( pInst = pPropIndex->instances;  pInst != NULL;  pInst = pInst->next )
    {
        inst               = new_instance( );

        inst->script       = pInst->script;
        inst->location     = NULL;

        inst->next       = pMe->instances;
        pMe->instances    = inst;
    }
    }

    pMe->item_type      = pPropIndex->item_type;
    pMe->extra_flags    = pPropIndex->extra_flags;
    pMe->wear_flags     = pPropIndex->wear_flags;
    pMe->value[0]       = pPropIndex->value[0];
    pMe->value[1]       = pPropIndex->value[1];
    pMe->value[2]       = pPropIndex->value[2];
    pMe->value[3]       = pPropIndex->value[3];
    pMe->size           = pPropIndex->size;
    pMe->weight         = pPropIndex->weight;

    if (pPropIndex->cost == 0)
     pMe->cost          = number_fuzzy( 10 )
                        * number_fuzzy( 1 ) * number_fuzzy( 1 );
    else pMe->cost      = number_range( pPropIndex->cost - pPropIndex->cost/8,
                                        pPropIndex->cost + pPropIndex->cost/8 );

    /*
     * Mess with prop properties.
     */
    switch ( pMe->item_type ) {
    default:
    if ( pMe->item_type >= ITEM_MAX )
        bug( "Read_prop: vnum %d bad type.", pPropIndex->vnum );
        break;

    case ITEM_SCROLL:
        pMe->value[0]   = number_fuzzy( pMe->value[0] );
        break;

    case ITEM_WAND:
    case ITEM_STAFF:
        pMe->value[0]   = number_fuzzy( pMe->value[0] );
        pMe->value[1]   = number_fuzzy( pMe->value[1] );
        pMe->value[2]   = pMe->value[1];
        break;

    case ITEM_WEAPON:
    if ( pMe->value[1] != 0 && pMe->value[2] != 0 )
    {
         pMe->value[1]   = number_fuzzy( pMe->value[1] );
         pMe->value[2]   = number_fuzzy( pMe->value[2] );
    }
    else
    {
         pMe->value[1]   = number_fuzzy( number_fuzzy( 1 * 2 / 4 + 2 ) );
         pMe->value[2]   = number_fuzzy( number_fuzzy( 3 * 2 / 4 + 6 ) );
    }
    break;
    case ITEM_ARMOR:
    if ( pMe->value[0] != 0 )
    {
         pMe->value[0]   = number_fuzzy( pMe->value[0] );
    }
    else    pMe->value[0]   = number_fuzzy( 1 / 4 + 2 );

    if ( pMe->value[2] != 0 )
    {
         pMe->value[2]   = number_fuzzy( pMe->value[2] );
         pMe->value[1]   = pMe->value[2];
    }
        break;

    case ITEM_POTION:
    case ITEM_PILL:
        pMe->value[0]   = number_fuzzy( number_fuzzy( pMe->value[0] ) );
        break;

        pMe->value[0]   = pMe->cost;
        break;
    }

    /*
     *  Randomize color, size, and anything else we want! (do the strings)
     */
    free_string( pMe->name );               pMe->name               = NULL;
    free_string( pMe->short_descr );        pMe->short_descr        = NULL;
    free_string( pMe->description );        pMe->description        = NULL;
    free_string( pMe->action_descr );       pMe->action_descr       = NULL;
    free_string( pMe->short_descr_plural ); pMe->short_descr_plural = NULL;
    free_string( pMe->description_plural ); pMe->description_plural = NULL;
    free_string( pMe->real_description );   pMe->real_description   = NULL;

    prop_strings( pMe );

    return NULL;
}

/* -------------------------------------------------------------- */

/*
 * Reverb(text);
 * Emits text from all props with a matching vnum.
 */
VARIABLE_DATA *func_broadcast( void * owner, int type, VARIABLE_DATA *out )
{
    char _out[MSL];
    PROP_DATA *p;
    PLAYER_DATA *rch;
    SCENE_INDEX_DATA *pScene=NULL;
    int vnum;

    if ( type != TYPE_PROP ) return NULL;  /* catch for scripts running on other things */

    STR_PARAM(out,_out);

    p = PROP(owner);

    if ( p == NULL ) return NULL;

    vnum = p->pIndexData->vnum;

    for ( p = prop_list;  p != NULL;  p = p->next ) {

    if ( p->pIndexData->vnum != vnum ) continue;

    if ( p->carried_by != NULL ) {
         send_to_actor( _out, p->carried_by );
         return NULL;
    }

    pScene = p->in_scene;
    if ( pScene != NULL ) {
           for ( rch = pScene->people;
                 rch != NULL;
                 rch = rch->next_in_scene )
             send_to_actor( _out, rch );
    }

    }

    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Has(prop vnum);
 * Has(target,prop vnum);
 */
VARIABLE_DATA *func_has( void *owner, int type, VARIABLE_DATA *target, 
                                                VARIABLE_DATA *propvnum ) {
    char _propvnum[MSL];
    char _target[MSL];
    PROP_INDEX_DATA *pIndexData=NULL;
    PROP_DATA *prop=NULL;

    STR_PARAM(propvnum,_propvnum);
    STR_PARAM(target,_target);

           FOREACH( target->type, 
                    prop = ACTOR(target->value)->carrying, 
                    prop =  PROP(target->value)->contains,
                    prop = SCENE(target->value)->contents,
                    prop = NULL; pIndexData = get_prop_index(atoi(_target)) );

           if ( !prop ) {     
           FOREACH( type,
                    prop = ACTOR(owner)->carrying, 
                    prop =  PROP(owner)->contains,
                    prop = SCENE(owner)->contents,
                    prop = NULL; );        
           }

    if ( !pIndexData ) {
        pIndexData = get_prop_index(atoi(_propvnum));
    }

    if ( !prop || !pIndexData ) return NULL;

    for( ; prop; prop = prop->next_content ) {
       if ( prop->pIndexData->vnum == pIndexData->vnum ) break;
    }

    RETURNS(TYPE_PROP,prop);
}


/* -------------------------------------------------------------- */
/*
 * Holds(prop vnum);
 * Holds(target,prop vnum);
 */
VARIABLE_DATA *func_holds( void *owner, int type, VARIABLE_DATA *target, 
                                                  VARIABLE_DATA *propvnum ) {
    char _propvnum[MSL];
    char _target[MSL];
    PROP_INDEX_DATA *pIndexData=NULL;
    PROP_DATA *prop=NULL;

    STR_PARAM(propvnum,_propvnum);
    STR_PARAM(target,_target);

    if ( _propvnum[0] == '\0' ) {
        switch ( type ) {
           case TYPE_ACTOR: prop=ACTOR(owner)->carrying; break;
            case TYPE_PROP: prop=PROP(owner)->contains;  break;
           case TYPE_SCENE: prop=SCENE(owner)->contents; break;
              default: prop=NULL;
        }
        if ( prop == NULL ) { RETURNS(TYPE_STRING,str_dup("0")); }
        pIndexData = get_prop_index(atoi(_target));
     } else {
        switch ( target->type ) {
           case TYPE_ACTOR: prop=ACTOR(target->value)->carrying; break;
            case TYPE_PROP: prop=PROP(target->value)->contains;  break;
           case TYPE_SCENE: prop=SCENE(target->value)->contents; break;
              default: prop=NULL;
        }
        if ( prop == NULL ) { RETURNS(TYPE_STRING,str_dup("0")); }
        pIndexData = get_prop_index(atoi(_propvnum));
     } 

    if ( !prop || !pIndexData ) { RETURNS(TYPE_STRING,str_dup("0")); }

    for( ; prop; prop = prop->next_content ) {
       if ( prop->pIndexData->vnum == pIndexData->vnum ) break;
    }

    if ( prop ) { RETURNS(TYPE_STRING,str_dup("1")); }
    RETURNS(TYPE_STRING,str_dup("0"));
}


/* -------------------------------------------------------------- */
/*
 * History(target,text)
 * For the history command.
 */
VARIABLE_DATA *func_history( void * owner, int type, VARIABLE_DATA *target, 
VARIABLE_DATA *out )
{
    char buf[MAX_STRING_LENGTH];
    char _out[MSL];
    char _target[MSL];
    PLAYER_DATA *ch;

    if ( type != TYPE_ACTOR) return NULL;

    STR_PARAM(out,_out);
    STR_PARAM(target,_target);

    ch = (target && target->type==TYPE_ACTOR) ? ACTOR(target) : 
         get_actor_world( ACTOR(owner), _target );
    if ( !ch ) return NULL;

    if ( !IS_NPC(ch) ) {
       if ( PC(ch,history) != NULL )
       snprintf( buf, MAX_STRING_LENGTH, "%s\n\r%s\n\r", PC(ch,history),  _out );  /* leak? */
       else strcpy( _out, buf );

       free_string( PC(ch,history) );
       PC(ch,history) = str_dup( buf );
    }    

    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Dream(text)
 * Outputs to all sleeping players in the zone.
 */
VARIABLE_DATA *func_dream( void * owner, int type, VARIABLE_DATA *out )
{
    ZONE_DATA *pZone;
    PLAYER_DATA *ch;
    SCENE_INDEX_DATA *pScene;
    PROP_DATA *pProp;
    char _out[MSL];

    STR_PARAM(out,_out);

    switch( type ) {

      case TYPE_ACTOR: ch = ACTOR(owner); 
                  pZone = ch->in_scene->zone; break;
      case TYPE_PROP: pProp = PROP(owner); 
           if ( pProp->in_scene != NULL ) pZone = pProp->in_scene->zone;
           else pZone = pProp->pIndexData->zone; break;
     case TYPE_SCENE: pScene = SCENE(owner);
                     pZone = pScene->zone; break;
          default: pZone = NULL; break;
    }

    if ( pZone == NULL ) return NULL;

{
    CONNECTION_DATA *d;

    for ( d = connection_list; d; d = d->next )
    {
        if ( d->connected == NET_PLAYING
          && d->character->in_scene != NULL
          && d->character->in_scene->zone == pZone 
          && d->character->position == POS_SLEEPING )
        {
            send_to_actor( _out, d->character );
            send_to_actor( "\n\r",   d->character );
        }
    }
}
  
    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Numberize(value);
 * Converts from a number to an English equivalent.
 */
VARIABLE_DATA * func_numw( void * owner, int type, VARIABLE_DATA *astr )
{
    char _astr[MSL];
    int v;

    STR_PARAM(astr,_astr);

    v = atoi(_astr);

    RETURNS(TYPE_STRING,str_dup(numberize(v)));    
}



/* -------------------------------------------------------------- */
/*
 * Replace(old,new);
 * String_replace().
 */
VARIABLE_DATA * func_strp( void * owner, int type, VARIABLE_DATA *value,
                           VARIABLE_DATA *old, VARIABLE_DATA *new )
{
    char buf[MAX_STRING_LENGTH];
    char _value[MSL];
    char _old[MSL];
    char _new[MSL];
    char *p;

    STR_PARAM(value,_value);
    STR_PARAM(old,_old);
    STR_PARAM(new,_new);

    if ( _value[0] == '\0' || _old[0] == '\0' )
    {
        return NULL;
    }

    p = _value;
    while ( strstr( p, _old ) )
    {
        p = string_replace( p, _old, _new );

        if ( *p == '\0' || *_old == '\0' )
        {
            snprintf( buf, MAX_STRING_LENGTH, "%s", p );
            return NULL;
        }
    }

    snprintf( buf, MAX_STRING_LENGTH, "%s", _value );

    RETURNS(TYPE_STRING,str_dup(buf));    
}




/* -------------------------------------------------------------- */
/*
 * Do(Command1,Command2 .. 6);
 * Self-Interpret.
 */
VARIABLE_DATA * func_do( void * owner, int type,
                         VARIABLE_DATA *exp0,
                         VARIABLE_DATA *exp1,
                         VARIABLE_DATA *exp2,
                         VARIABLE_DATA *exp3,
                         VARIABLE_DATA *exp4,
                         VARIABLE_DATA *exp5 )
{
    char _exp0[MIL];
    char _exp1[MIL];
    char _exp2[MIL];
    char _exp3[MIL];
    char _exp4[MIL];
    char _exp5[MIL];

    if ( type != TYPE_ACTOR )
        return NULL;

    STR_PARAM(exp0,_exp0);
    STR_PARAM(exp1,_exp1);
    STR_PARAM(exp2,_exp2);
    STR_PARAM(exp3,_exp3);
    STR_PARAM(exp4,_exp4);
    STR_PARAM(exp5,_exp5);

    if ( !MTD(_exp0) && _exp0[0] != '0' )
    interpret( ACTOR(owner), _exp0 );

    if ( !MTD(_exp1) && _exp1[0] != '0' )
    interpret( ACTOR(owner), _exp1 );

    if ( !MTD(_exp2) && _exp2[0] != '0' )
    interpret( ACTOR(owner), _exp2 );

    if ( !MTD(_exp3) && _exp3[0] != '0' )
    interpret( ACTOR(owner), _exp3 );

    if ( !MTD(_exp4) && _exp4[0] != '0' )
    interpret( ACTOR(owner), _exp4 );

    if ( !MTD(_exp5) && _exp5[0] != '0' )
    interpret( ACTOR(owner), _exp5 );

    return NULL;
}


/* -------------------------------------------------------------- */
/*
 * As(target,Command1,Command2 .. 5);
 * Do something as another actor (only)
 */
VARIABLE_DATA * func_as( void * owner, int type,
                         VARIABLE_DATA *target,
                         VARIABLE_DATA *exp0,
                         VARIABLE_DATA *exp1,
                         VARIABLE_DATA *exp2,
                         VARIABLE_DATA *exp3,
                         VARIABLE_DATA *exp4 )
{
    char _target[MSL];
    char _exp0[MIL];
    char _exp1[MIL];
    char _exp2[MIL];
    char _exp3[MIL];
    char _exp4[MIL];
    PLAYER_DATA *ch;

    if ( target && (target->type == TYPE_ACTOR) )   ch = ACTOR(target->value);
    else 
    { STR_PARAM(target,_target);
      ch = get_actor_scene( type == TYPE_ACTOR ? ACTOR(owner) : actor_list, _target );
    }

    if ( !ch ) return NULL;

    STR_PARAM(exp0,_exp0);
    STR_PARAM(exp1,_exp1);
    STR_PARAM(exp2,_exp2);
    STR_PARAM(exp3,_exp3);
    STR_PARAM(exp4,_exp4);

    if ( !MTD(_exp0) && _exp0[0] != '0' )
    interpret( ch, _exp0 );

    if ( !MTD(_exp1) && _exp1[0] != '0' )
    interpret( ch, _exp1 );

    if ( !MTD(_exp2) && _exp2[0] != '0' )
    interpret( ch, _exp2 );

    if ( !MTD(_exp3) && _exp3[0] != '0' )
    interpret( ch, _exp3 );

    if ( !MTD(_exp4) && _exp4[0] != '0' )
    interpret( ch, _exp4 );


    return NULL;
}


/* -------------------------------------------------------------- */
/*
 * Jump(location);
 * Moves to a scene.
 */
VARIABLE_DATA * func_jump( void * owner, int type, VARIABLE_DATA *location )
{
    PLAYER_DATA *actor;
    SCENE_INDEX_DATA *pScene=NULL;
    char _loc[MSL];
    int rvnum;

    if ( type != TYPE_ACTOR ) {
       if ( type == TYPE_PROP ) {
         // add support here
         return NULL;
       }
       else return NULL;
    }

    actor = ACTOR(owner);
 
    STR_PARAM(location,_loc);
    
    if ( location->type == TYPE_SCENE ) pScene = SCENE(location->value);
    rvnum = atoi(_loc);

    switch ( type ) {
      case TYPE_ACTOR:
       if ( !pScene ) pScene = get_scene_index( rvnum );
       if ( pScene ) {
    /*
     * Move all of your hirelings, mounts and conjured followers.
     */

    { PLAYER_DATA *pch;
      for ( pch = actor_list; pch!=NULL; pch=pch->next )
      if ( IS_NPC(pch) && pch->master == actor ) {
       actor_from_scene( pch );
       actor_to_scene( pch, pScene );
       if ( pch->riding ) {
       actor_from_scene( pch->riding );
       actor_to_scene( pch->riding, pScene );
       }
      }
     if ( (pch=actor->riding) ) {
      actor_from_scene( pch );
      actor_to_scene( pch, pScene );
     }
    }

       actor_from_scene( actor );
       actor_to_scene( actor, pScene );
       }      

      break;
      default: 
      break;
    }
    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * As(target,command,command,command,command);
 * Interpret as someone else.
 */
VARIABLE_DATA * func_interpret( void * owner, int type,
                         VARIABLE_DATA *who,
                         VARIABLE_DATA *exp1,
                         VARIABLE_DATA *exp2,
                         VARIABLE_DATA *exp3,
                         VARIABLE_DATA *exp4 )
{
    char _who[MSL];
    char _exp1[MSL];
    char _exp2[MSL];
    char _exp3[MSL];
    char _exp4[MSL];
    PLAYER_DATA *actor;

    if ( PARAMETER(who, TYPE_ACTOR) )
    actor = ACTOR(who->value);
    else {
       STR_PARAM(who,_who);
       actor=
       get_actor_world( type == TYPE_ACTOR ? ACTOR(owner) : actor_list, 
                   _who ); 
    }

    if ( !actor ) return NULL;
 
    STR_PARAM(exp1,_exp1);
    STR_PARAM(exp2,_exp2);
    STR_PARAM(exp3,_exp3);
    STR_PARAM(exp4,_exp4);

    if ( !MTD(_exp1) && _exp1[0] != '0' )
    interpret( ACTOR(owner), _exp1 );

    if ( !MTD(_exp2) && _exp2[0] != '0' )
    interpret( ACTOR(owner), _exp2 );

    if ( !MTD(_exp3) && _exp3[0] != '0' )
    interpret( ACTOR(owner), _exp3 );

    if ( !MTD(_exp4) && _exp4[0] != '0' )
    interpret( ACTOR(owner), _exp4 );

    return NULL;
}


/* -------------------------------------------------------------- */
/*
 * Aset();
 * Mset();
 * mob-set: unimplemented
 * target: itself
 */
VARIABLE_DATA *func_aset( void * owner, int type, VARIABLE_DATA *astr, VARIABLE_DATA *bstr )
{
    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Pset();
 * Oset();
 * object-set: partially implemented
 * target: variable
 */
VARIABLE_DATA *func_pset( void * owner, int type, VARIABLE_DATA *astr, VARIABLE_DATA *bstr )
{
    char buf[MAX_STRING_LENGTH];
    PROP_DATA *prop;
    char *p = "";
    char _bstr[MSL];

    if( PARAMETER(astr,TYPE_PROP) )
    return NULL;

    STR_PARAM(bstr,_bstr);

    prop = PROP(astr->value);

    if ( !str_cmp( _bstr, "short"       ) ) p = prop->short_descr;
    if ( !str_cmp( _bstr, "long"        ) ) p = prop->description;
    if ( !str_cmp( _bstr, "name"        ) ) p = prop->name;
    if ( !str_cmp( _bstr, "description" ) ) p = prop->real_description;
    if ( !str_cmp( _bstr, "plural"      ) ) p = pluralize( prop->short_descr );
    if ( !str_cmp( _bstr, "vnum"        ) )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%d", prop->pIndexData->vnum );
        p = buf;
    }
    if ( !str_cmp( _bstr, "scene" ) )
    {
        snprintf( buf, MAX_STRING_LENGTH, "%d", prop->in_scene->vnum );
        p = buf;
    }

    RETURNS(TYPE_STRING,str_dup(buf));    
}

/* -------------------------------------------------------------- */
/*
 * Heal(target,gain);
 * Heals a target for a certain amount.
 */
VARIABLE_DATA *func_heal( void * owner, int type, VARIABLE_DATA *target, 
                          VARIABLE_DATA *gain )
{
    PLAYER_DATA *ch;
    char _target[MSL];
    char _gain[MSL];

    if ( (target->type == TYPE_ACTOR) )
    ch = ACTOR(target->value);
    else 
    { STR_PARAM(target,_target);

    ch = 
    get_actor_world( type == TYPE_ACTOR ? ACTOR(owner) : actor_list, 
                   _target );
    }

    STR_PARAM(gain,_gain);
    
    if ( ch == NULL ) return NULL;

    ch->hit += atoi( _gain );
    if ( ch->hit > MAXHIT(ch) ) ch->hit = MAXHIT(ch);
    if ( ch->hit <= 0 ) ch->hit = 1;

    return NULL;
}


/* -------------------------------------------------------------- */
/*
 * HealAll(target,gain);
 * Heals a target and group for a certain amount.
 */
VARIABLE_DATA *func_healall( void * owner, int type, VARIABLE_DATA *target, 
                          VARIABLE_DATA *gain )
{
    PLAYER_DATA *ch;
    char _target[MSL];
    char _gain[MSL];

    if ( (target->type == TYPE_ACTOR) )
    ch = ACTOR(target->value);
    else 
    { STR_PARAM(target,_target);

    ch = 
    get_actor_world( type == TYPE_ACTOR ? ACTOR(owner) : actor_list, 
                   _target );
    }

    STR_PARAM(gain,_gain);
    
    if ( ch == NULL ) return NULL;

    ch->hit += atoi( _gain );
    if ( ch->hit > MAXHIT(ch) ) ch->hit = MAXHIT(ch);
    if ( ch->hit <= 0 ) ch->hit = 1;

    { PLAYER_DATA *gch;
      for ( gch = ch->in_scene->people;  gch != NULL; gch=gch->next ) 
      if ( in_group( ch, gch ) ) {
       gch->hit += atoi( _gain );
       if ( gch->hit > MAXHIT(gch) ) gch->hit = MAXHIT(gch);  
       if ( gch->hit <= 0 ) gch->hit = 1;
      }
    }

    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Bonus(target,location,bit,modifer,duration,aftype);
 * Creates a bonus on a target.  
 * Players / actors only.
 */
VARIABLE_DATA *func_bonus( void * owner, int type, VARIABLE_DATA *target, 
                          VARIABLE_DATA *location,
                          VARIABLE_DATA *bitvector, 
                          VARIABLE_DATA *modifier,
                          VARIABLE_DATA *duration,
                          VARIABLE_DATA *aftype )
{
    PLAYER_DATA *ch;
    char _target[MSL];
    char _location[MSL];
    char _bitvector[MSL];
    char _modifier[MSL];
    char _duration[MSL];
    char _aftype[MSL];

    if ( (target->type == TYPE_ACTOR) )
    ch = ACTOR(target->value);
    else 
    {
    STR_PARAM(target,_target);
    ch = 
    get_actor_world( type == TYPE_ACTOR ? ACTOR(owner) : actor_list, 
                   _target );
    }

    if ( ch==NULL ) return NULL;
 
    STR_PARAM(location,_location);
    STR_PARAM(bitvector,_bitvector);
    STR_PARAM(modifier,_modifier);
    STR_PARAM(duration,_duration);
    STR_PARAM(aftype,_aftype);
    {
    BONUS_DATA af;

    af.type      = bonus_name_bit(_aftype);
    af.duration  = atoi(_duration);
    af.location  = bonus_name_loc(_location);
    af.modifier  = atoi(_modifier);
    af.bitvector = act_name_bit(_bitvector);
    bonus_join( ch, &af );
    }    
    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Hurt();
 * You have been hurt.
 * Can kill you.
 * Known issue: variable corruption when this kills the target,
 * not yet fixed.  For now, don't refer to a target unless before
 * the hurt() call.  Could be fixed by traversing the master variable
 * list and all local instance variables, removing the pointers to
 * the dead char before the hurt() function ends.
 */
VARIABLE_DATA *func_hurt( void * owner, int type, VARIABLE_DATA *target, 
                          VARIABLE_DATA *gain )
{
    PLAYER_DATA *ch;
    char _target[MSL];
    char _gain[MSL];
    int dam;

    STR_PARAM(target,_target);
    STR_PARAM(gain,_gain);

    if ( !target || !gain ) return NULL;

    ch= ( target->type == TYPE_ACTOR ) ? ACTOR(target->value) : 
        find_actor_here( owner, type, _target );
    
    if ( ch == NULL ) { wtf_logf("func_hurt(): no target, passed: %s", _target); return NULL; }

    dam= atoi(_gain);

    /*
     * Fix lightly bug.
     */
    if ( dam  < 0 )  dam = 1;
    if ( dam == 0 ) { wtf_logf( "func_hurt(): dam=0!", 0 ); return NULL; }

/* notify */
{
    PLAYER_DATA *rch;
    char buf[12];

    for( rch = ch->in_scene->people; rch != NULL; rch = rch->next_in_scene )
    {
        if ( IS_IMMORTAL(rch) && IS_SET(rch->act,WIZ_NOTIFY_DAMAGE))
        {
            snprintf( buf, MAX_STRING_LENGTH, "[%3d->%c%c] ",
                     dam,
                     rch != ch ? STR(ch,name)[0] : '*',
                     rch != ch ? STR(ch,name)[1] : '*' );
            send_to_actor( buf, rch );
        }
    }

}

    /*
     * Hurt the victim.
     */
    ch->hit -= dam;
    if ( IS_IMMORTAL(ch) && ch->hit < 1 ) ch->hit = 1;

    update_pos( ch );

    if ( ch->hit > MAXHIT(ch) ) ch->hit = MAXHIT(ch);
    if ( ch->position== POS_DEAD ) raw_kill( ch );

    if ( !IS_AWAKE(ch) )
    stop_fighting( ch, !IS_NPC(ch) ? TRUE : FALSE ); 

    return NULL;
}

/* -------------------------------------------------------------- */

/*
 * Bomb();
 * You have been bombed.  Everyone in the scene, limited exceptions.
 * Can kill you.
 */
VARIABLE_DATA *func_bomb( void * owner, int type, VARIABLE_DATA *target, 
                          VARIABLE_DATA *gain )
{
    SCENE_INDEX_DATA *scene = NULL;
    PLAYER_DATA *us=NULL;
    char _target[MSL];
    char _gain[MSL];


    /*
     * Support for multiple parameter types.
     * String (target) or Scene
     */ 
    if ( (target->type == TYPE_SCENE) )
    scene = SCENE(target->value);
    else
    if ( (target->type == TYPE_ACTOR) ) {
    scene = ACTOR(target->value)->in_scene;
    us = ACTOR(target->value);
    }
    else 
    {
     STR_PARAM(target,_target);

     if ( type==TYPE_ACTOR && !us ) us=ACTOR(owner);
     us=find_actor_here(owner,type,_target);

     if ( ! us ) 
     scene = (get_scene_index( atoi(_target) ));
     else scene = us->in_scene;

     if ( scene == NULL ) 
          scene = type == TYPE_SCENE ? SCENE(owner) : NULL;
    }

    STR_PARAM(gain,_gain);    

//     wtf_logf( "bomb(): bombing in %s",scene ? scene->name : "nowhere" );
//     wtf_logf( "bomb(): checking against %s", us ? NAME(us) : "no one" );
     
    if ( scene == NULL ) return NULL;
    else { 
       PLAYER_DATA *ch;

             if ( !us ) {
       switch ( type ) {
         case TYPE_PROP: us=PROP(owner)->carried_by; break;
        case TYPE_SCENE: us=NULL; break;
        case TYPE_ACTOR: us=ACTOR(owner); break;
               default: us=NULL; break;
           } }

//     wtf_logf( "bomb(): checking against %s", us ? NAME(us) : "no one" );

       for ( ch = scene->people; ch != NULL; ch = ch->next_in_scene )
        if ( us->fighting ==ch || (ch != us && ch->master != us && !IS_IMMORTAL(ch)) ) {  
          if ( !IS_NPC(ch) && in_group( ch, us ) ) continue;
          if ( IS_NPC(ch) && in_group( us, ch->master ) ) continue;
          if ( IS_NPC(ch) && IS_SET(ch->act,ACT_GOOD) ) continue;
          ch->hit -= atoi( _gain );
          if ( ch->hit > MAXHIT(ch) ) ch->hit = MAXHIT(ch); 
          if ( ch->hit <= 0 ) ch->hit=1;
          oroc(us,ch);
          if ( us->position == POS_DEAD ) raw_kill(us);
//          wtf_logf(" bomb(): bombed against %s", NAME(ch) );  
       }
    }

    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Mana();
 * Change in mana.
 */
VARIABLE_DATA *func_mana( void * owner, int type, VARIABLE_DATA *target, 
                          VARIABLE_DATA *gain )
{
    PLAYER_DATA *ch=NULL;
    char _target[MSL];
    char _gain[MSL];

    if ( (target->type == TYPE_ACTOR) )
    ch = ACTOR(target->value);
    else 
    {
    STR_PARAM(target,_target);
    if ( type == TYPE_ACTOR )
    ch = 
    get_actor_world( type == TYPE_ACTOR ? ACTOR(owner) : actor_list, 
                   _target );
    }

    STR_PARAM(gain,_gain);
    
    if ( ch == NULL ) return NULL;

    ch->mana += atoi( _gain );
    if ( ch->mana > MAXMANA(ch) ) ch->mana = MAXMANA(ch);
    if ( ch->mana <= 0 ) ch->mana = 0;

    return NULL; 
}

/* -------------------------------------------------------------- */
/*
 * Regents
 * Returns TRUE if a player has everything required for a spell.
 * This function needs debugged.  Uses two keyed lists.
 */
VARIABLE_DATA *func_reagents( void * owner, int type,
                          VARIABLE_DATA *list, VARIABLE_DATA *quantity )
{
    PLAYER_DATA *ch;
    char _list[MSL];
    char _quantity[MSL];
    char *p, *point2;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    bool fOk = TRUE;

    if ( type == TYPE_ACTOR )
    ch = ACTOR(owner); 
    else {
    RETURNS(TYPE_STRING, (fOk? str_dup("1") : str_dup("0")) );
    }

    STR_PARAM(list,_list);
    STR_PARAM(quantity,_quantity);
    
    if ( ch == NULL ) return NULL;

    p = _list;
    point2 = _quantity;
    for ( p = one_argument( p, buf );  *p != '\0' && fOk; p=one_argument( p, buf )  )
    {
        PROP_DATA *pProp, *pPropNext, *pProp2, *pPropNext2;
        int quant;

        point2 = one_argument( p, buf2 );
        if ( buf2[0] == '\0' ) quant = 1;
	else quant = atoi(buf2);

        for( pProp = ch->carrying;  pProp != NULL;  pProp = pPropNext )
        {
            pPropNext = pProp->next;
                 for ( pProp2 = pProp->contains;  pProp2 != NULL;  pProp2 = pPropNext2 )
                 {
                     pPropNext2 = pProp2->next;
                     if ( pProp2->item_type == ITEM_COMPONENT 
                       && pProp2->value[0] == atoi(buf2) )
                         quant--;
                 }

            if ( pProp->item_type == ITEM_COMPONENT
              && pProp->value[0] == atoi(buf2) ) 
                 quant--;                         
        }

        if (quant > 0) fOk = FALSE;
        if ( !fOk ) break;
    }

    RETURNS(TYPE_STRING, (fOk? str_dup("1") : str_dup("0")) );
}

/* -------------------------------------------------------------- */
/*
 * Mix();
 * Used by "natural magic" spell scripts.
 * Mixes reagents.
 * This function needs debugged.   Uses two keyed lists.
 */
VARIABLE_DATA *func_mix( void * owner, int type, 
                          VARIABLE_DATA *list, VARIABLE_DATA *quantity )
{
    PLAYER_DATA *ch=NULL;
    char _list[MSL];
    char _quantity[MSL];
    char *p, *point2;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    bool fOk = TRUE;

    if ( type == TYPE_ACTOR ) ch = ACTOR(owner); 
    else return NULL;

    STR_PARAM(list,_list);
    STR_PARAM(quantity,_quantity);
    
    if ( ch == NULL ) return NULL;

//    wtf_logf( "mix(): ch found" );

    p = _list;
    point2 = _quantity;
    for ( p = one_argument( p, buf );  *p != '\0' && fOk; p = one_argument( p, buf ) )
    {
        PROP_DATA *pProp, *pPropNext, *pProp2, *pPropNext2;
        int quant;

        point2 = one_argument( p, buf2 );
        if ( buf2[0] == '\0' ) quant = 1;
	else quant = atoi(buf2);

        for( pProp = ch->carrying;  pProp != NULL;  pProp = pPropNext )
        {
            pPropNext = pProp->next_content;
                 for ( pProp2 = pProp->contains;  pProp2 != NULL;  pProp2 = pPropNext2 )
                 {
                     pPropNext2 = pProp2->next_content;
                     if ( pProp2->item_type == ITEM_COMPONENT 
                       && pProp2->value[0] == atoi(buf2) )
                       {
                         prop_from_prop( pProp2 );
//                         wtf_logf( "mix: found and extracting %s", STR(pProp2,short_descr) );
                         extract_prop( pProp2 ); 
                         quant--;
                       }
                 }

            if ( pProp->item_type == ITEM_COMPONENT
              && pProp->value[0] == atoi(buf2) ) 
               {
                 prop_from_actor( pProp );
//                 wtf_logf( "mix: found and extracting %s", STR(pProp,short_descr) );
                 extract_prop( pProp );
                 quant--;            
               }             
        }
        if ( quant == 0 ) fOk = fOk && TRUE;
    }

    if ( fOk && ch->exp_level < 10 ) {
        /*
         * Spellcasters gain experience from successful invocations.
         */
        exp_gain( ch, 1, TRUE );
    }

//    wtf_logf( "mix(): fOk=%d", fOk );

    RETURNS(TYPE_STRING, (fOk?str_dup("1"):str_dup("0")) );
}

/* -------------------------------------------------------------- */
/*
 * Gem();
 * Used by "gem magic" spell scripts.
 * Consumes gems, returns "1" if pass "0" if not enough mana
 * Assumes the caller is the one casting.
 */
VARIABLE_DATA *func_gem( void * owner, int type, 
                          VARIABLE_DATA *mana_type, VARIABLE_DATA *quantity )
{
    PLAYER_DATA *ch;
    char _mana_type[MSL];
    char _quantity[MSL];
    int amount;
    int available, mana;

    STR_PARAM(mana_type,_mana_type);
    STR_PARAM(quantity,_quantity);
    
    if ( type != TYPE_ACTOR ) return NULL;

    ch = ACTOR(owner);
    if ( ch == NULL ) { RETURNS(TYPE_STRING,str_dup("0")); }

    switch ( LOWER(*_mana_type) ) {
      case 'a': mana=MANA_AIR; break;
      case 'e': mana=MANA_EARTH; break;
      case 'f': mana=MANA_FIRE; break;
      case 'w': mana=MANA_WATER; break;
      default: mana=MANA_NONE; break;
    }   
    amount = atoi( _quantity );

    available = find_gem_mana( ch, mana );
    if ( available < mana ) {
       RETURNS(TYPE_STRING,str_dup("0"));
    }
 
    take_mana_gem( ch, amount, mana );
        /*
         * Spellcasters gain experience from successful invocations.
         */
        exp_gain( ch,  number_range(1,10), TRUE );
    
    RETURNS(TYPE_STRING, str_dup("1") );
}

/* -------------------------------------------------------------- */
/*
 * Pay();
 * You have been paid.
 */
VARIABLE_DATA *func_pay( void * owner, int type, VARIABLE_DATA *target, 
                          VARIABLE_DATA *gain )
{
    PLAYER_DATA *ch;
    char _target[MSL];
    char _gain[MSL];

    if ( (target->type == TYPE_ACTOR) )
    ch = ACTOR(target->value);
    else 
    {
    STR_PARAM(target,_target);
    ch = 
    get_actor_scene( type == TYPE_ACTOR ? ACTOR(owner) : actor_list, 
                   _target );
    }

    STR_PARAM(gain,_gain);
    
    if ( ch == NULL ) return NULL;

    /*
     * Pay a character.  Rounds to nearest gold.
     */  
    prop_to_actor( create_money( atoi(_gain), COIN_GOLD ), ch );
    merge_money( ch );

    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Charge();
 * You have been charged?  Returns "1" if successful, "0" if not
 */
VARIABLE_DATA *func_charge( void * owner, int type, VARIABLE_DATA *target, 
                          VARIABLE_DATA *loss )
{
    PLAYER_DATA *ch;
    char *p;
    char _target[MSL];
    char _loss[MSL];  
    char buf[MSL];  
    int cost;

    if ( (target->type == TYPE_ACTOR) )
    ch = ACTOR(target->value);
    else 
    {
    STR_PARAM(target,_target);
    ch = 
    get_actor_scene( type == TYPE_ACTOR ? ACTOR(owner) : actor_list, 
                   _target );
    }

    STR_PARAM(loss,_loss);  cost=atoi(_loss);
    
    if ( ch == NULL ) return NULL;

    /*
     * Charge a character.
     */  
    if ( tally_coins( ch ) < cost ) {
        RETURNS(TYPE_STRING,str_dup("0"));
    }

    p = sub_coins(cost, ch );

    if ( !str_cmp( p, "nothing" ) )
    snprintf( buf, MAX_STRING_LENGTH, "You pay.\n\r" );
    else
    snprintf( buf, MAX_STRING_LENGTH, "You pay and receive %s in change.\n\r", p );
    send_to_actor( buf, ch );

    RETURNS(TYPE_STRING,str_dup("1"));
}

/* -------------------------------------------------------------- */
/*
 * You have been positioned.
 */
VARIABLE_DATA *func_pos( void * owner, int type, VARIABLE_DATA *target, 
                          VARIABLE_DATA *gain )
{
    PLAYER_DATA *ch;
    char _target[MSL];
    char _gain[MSL];

    if ( (target->type == TYPE_ACTOR) )
    ch = ACTOR(target->value);
    else 
    {
    STR_PARAM(target,_target);
    ch = 
    get_actor_world( type == TYPE_ACTOR ? ACTOR(owner) : actor_list, 
                   _target );
    }

    STR_PARAM(gain,_gain);
    
    if ( ch == NULL ) return NULL;

    ch->position = atoi(_gain);

    return NULL;
}


/* -------------------------------------------------------------- */
/*
 * You have been stripped of all your worldly possessions (except money).
 */
VARIABLE_DATA *func_strip( void * owner, int type, VARIABLE_DATA *target )
{
    PLAYER_DATA *ch;
    PROP_DATA *pProp, *pPropNext;
    char _target[MSL];
    int count_coins;


    if ( (target->type == TYPE_ACTOR) )
    ch = ACTOR(target->value);
    else 
    {
    STR_PARAM(target,_target);
    ch = 
    get_actor_world( type == TYPE_ACTOR ? ACTOR(owner) : actor_list, 
                   _target );
    }

    if ( ch == NULL ) return NULL;

    count_coins =   tally_coins( ch );

    for ( pProp = ch->carrying;  pProp != NULL;  pProp = pPropNext ) {

        pPropNext = pProp->next_content;
        prop_from_actor( pProp );
        extract_prop( pProp );
    }

    create_amount( count_coins, ch, NULL, NULL );
    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Removes all weapons from player top level.
 */
VARIABLE_DATA *func_disarm( void * owner, int type, VARIABLE_DATA *target )
{
    PLAYER_DATA *ch;
    PROP_DATA *pProp, *pPropNext;
    char _target[MSL];

    if ( (target->type == TYPE_ACTOR) )
    ch = ACTOR(target->value);
    else 
    {
    STR_PARAM(target,_target);

    ch = 
    get_actor_world( type == TYPE_ACTOR ? ACTOR(owner) : actor_list, 
                   _target );
    }

    if ( ch == NULL ) return NULL;


    for ( pProp = ch->carrying;  pProp != NULL;  pProp = pPropNext ) {

        pPropNext = pProp->next_content;
        if ( pProp->item_type == ITEM_WEAPON 
          || pProp->item_type == ITEM_RANGED_WEAPON ) {
		prop_from_actor( pProp );
        	extract_prop( pProp );
        }
    }  

    return NULL;
}


/* -------------------------------------------------------------- */
/*
 * Checks verse a skill, 0=fail
 */
VARIABLE_DATA *func_skill( void * owner, int type, VARIABLE_DATA *target,
                           VARIABLE_DATA *sn, VARIABLE_DATA *v ) 
{
    PLAYER_DATA *ch=NULL;
    char _target[MSL];
    char _sn[MSL];
    char _value[MSL];
    SKILL_DATA *pSkill=NULL;
    int check=100;

    if ( (target->type == TYPE_ACTOR) )    ch = ACTOR(target->value);
    else 
    {
    STR_PARAM(target,_target);
    ch = find_actor_here(owner,type,_target);
    }

    if ( ch == NULL ) return NULL;

    STR_PARAM(sn,_sn);        pSkill = skill_lookup(_sn);
    STR_PARAM(v,_value);      if ( is_number(_value) ) check = atoi(_value); 

    if ( ch && pSkill ) {
      RETURNS(TYPE_STRING, (skill_check(ch,pSkill,check) ? str_dup("1") : str_dup("0")) );
    }

    RETURNS(TYPE_STRING,str_dup("1"));
}

/* -------------------------------------------------------------- */
/*
 * Sets the value of a skill on a player.
 */
VARIABLE_DATA *func_setskill( void * owner, int type, VARIABLE_DATA *target,
                           VARIABLE_DATA *sn, VARIABLE_DATA *v ) 
{
    PLAYER_DATA *ch;
    char _target[MSL];
    char _sn[MSL];
    char _value[MSL];
    int vsn;
    int value;

    if ( (target->type == TYPE_ACTOR) )
    ch = ACTOR(target->value);
    else 
    {
    STR_PARAM(target,_target);

    ch = 
    get_actor_world( type == TYPE_ACTOR ? ACTOR(owner) : actor_list, 
                   _target );
    }

    if ( ch == NULL ) return NULL;

    STR_PARAM(sn,_sn);    vsn = atoi(_sn);
    STR_PARAM(v,_value);  value = atoi(_value);

    update_skill(ch,vsn,value);
    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Cue()
 * Cues a scene.
 */
VARIABLE_DATA *func_cue( void * owner, int type, VARIABLE_DATA *dest )
{
    SCENE_INDEX_DATA *pScene;
    char _dest[MSL];

    STR_PARAM(dest,_dest);

       pScene = get_scene_index( atoi( _dest ) );
       if ( pScene != NULL ) {
          spawn_scene( pScene );
       }

    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Purge();
 * Purges the contents of a scene.
 */
VARIABLE_DATA *func_purge( void * owner, int type, VARIABLE_DATA *dest )
{
    char _dest[MSL];

    STR_PARAM(dest,_dest);

    cmd_purge( type == TYPE_ACTOR ? ACTOR(owner) : actor_list, _dest );    

    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Force();
 * Kludgy.  Icky.  Don't use.
 */
VARIABLE_DATA *func_force( void * owner, int type, VARIABLE_DATA *dest )
{
    char _dest[MSL];

    STR_PARAM(dest,_dest);

    cmd_force( type == TYPE_ACTOR ? ACTOR(owner) : actor_list, _dest );    

    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Peace();
 * Stops fights.
 */
VARIABLE_DATA *func_peace( void * owner, int type, VARIABLE_DATA *dest )
{
    char _dest[MSL];

    STR_PARAM(dest,_dest);

    cmd_peace( type == TYPE_ACTOR ? (PLAYER_DATA *)owner : actor_list, _dest );    

    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Elude();
 * Stops followers from following.
 * Called on itself.
 */
VARIABLE_DATA *func_elude( void * owner, int type )
{
    if ( type != TYPE_ACTOR ) return NULL;
    else
    {
       PLAYER_DATA *pActor = (PLAYER_DATA *)owner;
       PLAYER_DATA *pFollow;

       for ( pFollow = actor_list;  pFollow != NULL;  pFollow = pFollow->next )
       {
		if ( pFollow->master == pActor ) cmd_follow( pFollow, "self" );
       }
    }

    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Move();
 * Moves a target to a new scene.
 */
VARIABLE_DATA *func_move( void * owner, int type, VARIABLE_DATA *target, 
                                                  VARIABLE_DATA *dest )
{
    SCENE_INDEX_DATA *pScene;
    PLAYER_DATA *ch, *och;
    char _dest[MSL];
    char _target[MSL];

    STR_PARAM(dest,_dest);
    STR_PARAM(target,_target);
    
    och = type == TYPE_ACTOR ? (PLAYER_DATA *)owner : actor_list;

    if ( target != NULL )
    ch = target->type == TYPE_ACTOR ? target->value
                                  : get_actor_world( och, _target );
    else
    ch = get_actor_world( och, _target );

    if ( ch == NULL ) return NULL;

    if ( dest->type == TYPE_SCENE ) pScene = SCENE(dest->value);
    else
    pScene = get_scene_index( atoi( _dest ) );

       if ( pScene != NULL ) {

    /*
     * Move all of your hirelings, mounts and conjured followers.
     */

    { PLAYER_DATA *pch;
      for ( pch = actor_list; pch!=NULL; pch=pch->next )
      if ( IS_NPC(pch) && pch->master == ch ) {
       actor_from_scene( pch );
       actor_to_scene( pch, pScene );
       if ( pch->riding ) {
       actor_from_scene( pch->riding );
       actor_to_scene( pch->riding, pScene );
       }
      }
     if ( (pch=ch->riding) != NULL ) {
      actor_from_scene( pch );
      actor_to_scene( pch, pScene );
     }
    }

	     actor_from_scene( ch );
	     actor_to_scene( ch, pScene );
                cmd_look( ch, "" );          /* look? */
       script_update( pScene, TYPE_SCENE, TRIG_ENTER, ch, NULL, NULL, NULL );
       for ( och = pScene->people;  och != NULL; och=och->next_in_scene )
         if ( och->master != ch && ch->riding != och ) {
       script_update( och, TYPE_ACTOR, TRIG_ENTER, ch, NULL, NULL, NULL );
         }
       }

    return NULL;
}


/* -------------------------------------------------------------- */
/*
 * Here();
 * Returns vnum of "where" the owner is.
 */
VARIABLE_DATA *func_here( void *owner, int type )
{
   char buf[MAX_STRING_LENGTH];
   SCENE_INDEX_DATA *pScene;

   switch( type ) {
       case TYPE_STRING: pScene = NULL; break;
       case TYPE_ACTOR: pScene = ((PLAYER_DATA *)owner)->in_scene; break;
       case TYPE_PROP:
              {   PROP_DATA *pProp;

                      pProp = PROP(owner);
                      pScene = pProp->in_scene;
                      if ( !pScene && pProp->carried_by )
                             pScene = pProp->carried_by->in_scene;
                      if ( !pScene && pProp->in_prop )
                             pScene = pProp->in_prop->in_scene;
              }
       case TYPE_SCENE:
                 pScene = SCENE(owner);
              break;
       default: pScene = NULL; break;
   }

   if ( pScene == NULL ) return NULL;

   snprintf( buf, MAX_STRING_LENGTH, "%d", pScene->vnum );

   RETURNS(TYPE_STRING, str_dup(buf) );
}


/* -------------------------------------------------------------- */
/*
 * Moveall();
 * Moves all players from one location to another.
 * Forces all players to run cmd.
 * This function needs to be debugged.
 */
VARIABLE_DATA *func_moveall( void * owner, int type, VARIABLE_DATA *from,
                          VARIABLE_DATA *dest, VARIABLE_DATA *cmd )
{
    PLAYER_DATA *ch, *ch_next, *och;
    SCENE_INDEX_DATA *pScene, *pDest;
    char _from[MSL];
    char _dest[MSL];
    char _cmd[MIL];

    STR_PARAM(from,_from);

    if ( from->type == TYPE_SCENE ) pScene = SCENE(from->value);
    else
    pScene =     get_scene_index( atoi( _from ) );

    STR_PARAM(dest,_dest);

    if ( dest->type == TYPE_SCENE ) pDest = SCENE(dest->value);
    else
    pDest = get_scene_index( atoi( _dest ) );

    STR_PARAM(cmd,_cmd);

    if ( pScene == NULL || pDest == NULL || pScene==pDest ) return NULL;
    
    for ( ch = pScene->people;  ch != NULL;  ch = ch_next ) {

	ch_next = ch->next_in_scene;

          if ( IS_NPC(ch) ) continue;

    /*
     * Move all of your hirelings, mounts and conjured followers.
     */

    { PLAYER_DATA *pch, *pch_next;
      for ( pch = pScene->people; pch!=NULL; pch=pch_next ) {
       pch_next = pch->next_in_scene;
      if ( IS_NPC(pch) && pch->master == ch ) {       
       actor_from_scene( pch );  actor_to_scene( pch, pDest );
       if ( pch->riding ) {  actor_from_scene( pch->riding );  actor_to_scene( pch->riding, pDest );  }
      }
      }
    }

     if ( ch->riding ) {
      actor_from_scene( ch->riding );
      actor_to_scene( ch->riding, pDest );
     }

     actor_from_scene( ch );
     actor_to_scene( ch, pDest );
//                cmd_look( ch, "" );          /* look? */
     if (  !MTD(_cmd) ) interpret( ch, _cmd );
    }

    return NULL;
}

#if defined(NEVER)
/* -------------------------------------------------------------- */
/*
 * findall();
 * Finds all hirelings/followers of a player of a certain vnum
 * Returns the number of found followers for that player
 */
VARIABLE_DATA *func_findall( void * owner, int type, VARIABLE_DATA *who )
{

    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * forall();
 * Finds all hirelings/followers of a player of a certain vnum
 * Forces each to execute a command; not yet implemented
 */
VARIABLE_DATA *func_forall( void * owner, int type, VARIABLE_DATA *from,
                          VARIABLE_DATA *dest, VARIABLE_DATA *cmd )
{
    return NULL;
}
#endif

/* -------------------------------------------------------------- */


VARIABLE_DATA *func_home( void * owner, int type, VARIABLE_DATA *target,
                          VARIABLE_DATA *dest )
{
    PLAYER_DATA *ch;
    SCENE_INDEX_DATA *pScene;
    char _target[MSL];
    char _dest[MSL];


    if ( (target->type == TYPE_ACTOR) )
    ch = (PLAYER_DATA *)(target->value);
    else 
    {
    STR_PARAM(target,_target);

    ch = 
    get_actor_world( type == TYPE_ACTOR ? (PLAYER_DATA *)owner : actor_list, 
                   _target );
    }

    STR_PARAM(dest,_dest);
    
    if ( ch != NULL && !IS_NPC(ch) ) {
       pScene = get_scene_index( atoi( _dest ) );
       if ( pScene != NULL ) {
          PC(ch,home) = atoi( _dest );
       }
    }

    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Murder! Death(); Kill.
 * Slay somebody.
 */
VARIABLE_DATA *func_death( void * owner, int type, VARIABLE_DATA *target,
                           VARIABLE_DATA *dest )
{
    PLAYER_DATA *ch;
    SCENE_INDEX_DATA *pScene;
    char _target[MSL];
    char _dest[MSL];

    if ( (target->type == TYPE_ACTOR) )
    ch = (PLAYER_DATA *)(target->value);
    else 
    {
    STR_PARAM(target,_target);
    ch = 
    get_actor_world( type == TYPE_ACTOR ? (PLAYER_DATA *)owner : actor_list, 
                   _target );
    }

    STR_PARAM(dest,_dest);
    
    if ( ch != NULL && !IS_NPC(ch) ) {
       pScene = get_scene_index( atoi( _dest ) );
       if ( pScene != NULL ) {
          PC(ch,death) = atoi( _dest );
       }
    }

    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Open();
 * Opens a door quietly.
 * This function is not implemented.
 */
VARIABLE_DATA *func_open( void * owner, int type, VARIABLE_DATA *loc,
                          VARIABLE_DATA *dir )
{
    SCENE_INDEX_DATA *pScene;
    char _loc[MSL];
    char _dir[MSL];
    int direction;

    STR_PARAM(loc,_loc);
    STR_PARAM(dir,_dir);
    
    pScene = get_scene_index( atoi( _loc ) );
    direction = atoi( _dir );

    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Close();
 * Closes a door quietly.
 * This function is not implemented.
 */
VARIABLE_DATA *func_close( void * owner, int type, VARIABLE_DATA *loc,
                           VARIABLE_DATA *dir )
{
    SCENE_INDEX_DATA *pScene;
    char _loc[MSL];
    char _dir[MSL];
    int direction;

    STR_PARAM(loc,_loc);
    STR_PARAM(dir,_dir);
    
    pScene = get_scene_index( atoi( _loc ) );
    direction = atoi( _dir );

    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Dispense();
 * "Gives" a brand new copy of a prop template to a target actor or room vnum.
 */
VARIABLE_DATA *func_dispense( void * owner, int type, VARIABLE_DATA *target,
                              VARIABLE_DATA *disp )
{
    PLAYER_DATA *ch=NULL;
    SCENE_INDEX_DATA *pScene=NULL;
    PROP_DATA *pProp;
    char _target[MSL];
    char _disp[MSL];

    if ( (target->type == TYPE_ACTOR) ) ch = (PLAYER_DATA *)(target->value);
    else 
    {
    STR_PARAM(target,_target);
    if ( is_number( _target ) ) {
      pScene = get_scene_index( atoi(_target) );
    } else
    ch = find_actor_here(owner,type,_target);
//    get_actor_scene( type == TYPE_ACTOR ? (PLAYER_DATA *)owner : actor_list, 
//                   _target );
    }

    STR_PARAM(disp,_disp);
    
    if ( pScene != NULL ) {
       PROP_INDEX_DATA *pIndex;
       pIndex = get_prop_index( atoi( _disp ) );
       pProp = create_prop( pIndex, 1 );
       pProp->timer = pIndex->timer;
       if ( pProp != NULL ) prop_to_scene( pProp, pScene );
    } else
    if ( ch != NULL ) {
       PROP_INDEX_DATA *pIndex;
       pIndex = get_prop_index( atoi( _disp ) );
       pProp = create_prop( pIndex, 1 );
       pProp->timer = pIndex->timer;
       if ( pProp != NULL ) prop_to_actor( pProp, ch );
    }

    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Equip();
 * "Gives" a brand new copy of a prop template to a target actor on a
 * specified (wear location bit)- used mainly for hirelings, but you
 * could also use this to place a crown on a player's head or
 * stick a medal around a player's neck
 */
VARIABLE_DATA *func_equip( void * owner, int type, VARIABLE_DATA *target,
                              VARIABLE_DATA *disp, VARIABLE_DATA *loc )
{
    PLAYER_DATA *ch;
    PROP_DATA *pProp;
    char _target[MSL];
    char _disp[MSL];
    char _loc[MSL];

    if ( (target->type == TYPE_ACTOR) )
    ch = (PLAYER_DATA *)(target->value);
    else 
    {
    STR_PARAM(target,_target);
    ch = 
    get_actor_scene( type == TYPE_ACTOR ? (PLAYER_DATA *)owner : actor_list, 
                   _target );
    }

    STR_PARAM(disp,_disp);
    STR_PARAM(loc,_loc);
    
    if ( ch != NULL ) {
       PROP_INDEX_DATA *pIndex;
       pIndex = get_prop_index( atoi( _disp ) );
       pProp = create_prop( pIndex, 1 );
       if ( pProp != NULL ) {
           prop_to_actor( pProp, ch );
           wear_prop( ch, pProp, TRUE, wear_name_bit( _loc ) );
           RETURNS(TYPE_PROP,pProp);
        }
    }

    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Inside();
 *
 * "Inserts" a brand new copy of a prop template inside a target obj
 * (places it inside it) Does not check if it's a container or not.
 *
 * Expects to be given a valid target.  If the target is invalid,
 * assumes the target is a string describing an object in the script
 * owner's inventory using get_prop_carry().
 */
VARIABLE_DATA *func_inside( void * owner, int type, VARIABLE_DATA *target,
                              VARIABLE_DATA *disp )
{
    PROP_DATA *pProp, *prop=NULL;
    char _target[MSL];
    char _disp[MSL];

    if ( (target->type == TYPE_PROP) )
     prop = (PROP_DATA *)(target->value);
    else 
    {
     PLAYER_DATA *ch = type == TYPE_ACTOR ? (PLAYER_DATA *)owner : actor_list;
     STR_PARAM(target,_target);
     prop = get_prop_carry( ch, _target );
    }

    STR_PARAM(disp,_disp);
    
    if ( prop != NULL ) {
       PROP_INDEX_DATA *pIndex;
       pIndex = get_prop_index( atoi( _disp ) );
       pProp = create_prop( pIndex, 1 );
       if ( pProp != NULL ) {
           prop_to_prop( pProp, prop );
           RETURNS(TYPE_PROP, pProp);
        }
    }

    return NULL;
}



/* -------------------------------------------------------------- */
/*
 * Create();
 * Summons a new actor to a scene.
 */
VARIABLE_DATA *func_create( void * owner, int type, VARIABLE_DATA *vnum, VARIABLE_DATA *loc )
{
    PLAYER_DATA *ch;
    SCENE_INDEX_DATA *pScene, *pLoc;
    char _loc[MSL];
    char _vnum[MSL];
 
    STR_PARAM(vnum,_vnum);
    STR_PARAM(loc,_loc);

    pScene = NULL;

    if ( type == TYPE_ACTOR ) {
        ch = (PLAYER_DATA *)owner;
        if ( ch != NULL ) pScene = ch->in_scene;
    }
    else if ( type == TYPE_SCENE ) pScene = SCENE(owner);
    else if ( type == TYPE_PROP  ) pScene = PROP(owner)->in_scene;
    
    if ( (pLoc = get_scene_index( atoi(_loc) )) != NULL ) {
        pScene = pLoc;
    }
    
    if ( pScene != NULL ) {
       ACTOR_INDEX_DATA *pIndex;
       pIndex = get_actor_index( atoi( _vnum ) );
       ch = create_actor( pIndex );
       if ( ch != NULL ) actor_to_scene( ch, pScene );
    }

    return NULL;
}


/* -------------------------------------------------------------- */
/*
 * Breed();
 * Breeds a new hireling (pet), attaching it to a master.
 */
VARIABLE_DATA *func_breed( void * owner, int type, VARIABLE_DATA *vnum, VARIABLE_DATA *master )
{
    PLAYER_DATA *ch=NULL;
    SCENE_INDEX_DATA *pScene=NULL;
    char _master[MSL];
    char _vnum[MSL];
 
    STR_PARAM(vnum,_vnum);

    STR_PARAM(master,_master);

    if ( master && (master->type == TYPE_ACTOR) )
    ch = (PLAYER_DATA *)(master->value);
    else 
    {
    STR_PARAM(master,_master);
    ch=find_actor_here(owner,type,_master);
    if ( !ch )
    ch=get_actor_scene( type == TYPE_ACTOR ? (PLAYER_DATA *)owner : actor_list, _master );
    }

    if ( !ch ) {
         if ( type == TYPE_ACTOR )      ch = (PLAYER_DATA *)owner;    
    else if ( type == TYPE_PROP  )  ch = PROP(owner)->carried_by;
     }

    if ( ch ) pScene = ch->in_scene;

    if ( pScene && ch ) {
       ACTOR_INDEX_DATA *pIndex;
       PLAYER_DATA *pet;

       pIndex = get_actor_index( atoi( _vnum ) );
       pet = create_actor( pIndex );
       if ( pet != NULL ) {
         pet->owner = IS_NPC(ch) ? itoa(ch->pIndexData->vnum) : NAME(ch);
         actor_to_scene( pet, pScene );
         add_follower( pet, ch );
         SET_BIT( pet->act, ACT_PET );
         RETURNS(TYPE_ACTOR, pet);        
       }
    }

    return NULL;
}


       
/* -------------------------------------------------------------- */
/*
 * Eat();
 * Silent disposal of unnecessary things.  Works only on props.
 *
 * Similar to what drug cartels do in Mexico to their 'undesirables'
 */
VARIABLE_DATA *func_eat( void * owner, int type, VARIABLE_DATA *target )
{
    char _target[MSL];

    STR_PARAM(target,_target);
    
    if ( IS_PRESENT(target,_target) && target->type == TYPE_PROP ) {
      PROP(target)->item_type = ITEM_DELETE;
    } 
    else  
    if ( IS_PRESENT(target,_target) ) {
    PROP_DATA *prop=NULL;

    FOREACH(target->type,
            prop = (type == TYPE_ACTOR ? 
                       get_prop_here( ACTOR(owner), _target ) : NULL),
            prop = PROP(target),
            prop = NULL,
            prop = (type == TYPE_ACTOR ?
                       get_prop_here( ACTOR(owner), _target ) : NULL) 
           );

       if ( prop ) prop->item_type = ITEM_DELETE;
    }
    else  
    if ( type == TYPE_PROP ) PROP(owner)->item_type = ITEM_DELETE;
    
    return NULL;
}
        
    
/* -------------------------------------------------------------- */
/*
 * SameScene();
 * Returns true if in the same scene as the owner.
 */
VARIABLE_DATA *func_samescene( void * owner, int type, VARIABLE_DATA *var )
{
    char buf[MAX_STRING_LENGTH];
    int val;

    if ( var == NULL ) val = 0;
    else
    switch ( type )
    {
        case TYPE_PROP:
          if ( var->type == TYPE_PROP )
           val = PROP(var->value)->in_scene == PROP(owner)->in_scene;
          if ( var->type == TYPE_ACTOR )
           val = ACTOR(var->value)->in_scene == PROP(owner)->in_scene;
          if ( var->type == TYPE_SCENE )
           val = SCENE(var->value) == PROP(owner)->in_scene;
           val = 0;
         break;
        case TYPE_ACTOR:
          if ( var->type == TYPE_PROP )
           val = PROP(var->value)->in_scene == ACTOR(owner)->in_scene;
          if ( var->type == TYPE_ACTOR )
           val = PROP(var->value)->in_scene == ACTOR(owner)->in_scene;
          if ( var->type == TYPE_SCENE )
           val = SCENE(var->value) == ACTOR(owner)->in_scene;
           val = 0;
         break;
        case TYPE_SCENE:
          if ( var->type == TYPE_PROP )
           val = PROP(var->value)->in_scene == SCENE(owner);
          if ( var->type == TYPE_ACTOR )
           val = ACTOR(var->value)->in_scene == SCENE(owner);
          if ( var->type == TYPE_SCENE )
           val = SCENE(var->value) == SCENE(owner);
           val = 0;
         break;
        default: val = 0;
    }

    snprintf( buf, MAX_STRING_LENGTH, "%d", val );

    RETURNS(TYPE_STRING,str_dup(buf));    
}


/* -------------------------------------------------------------- */
/*
 * Alert();
 * Possibly a way to alert all other similar actors in a zone of
 * something.  Reminiscent of CityGuard scripts.  Not yet implemented.
 */
VARIABLE_DATA *func_alert( void * owner, int type, VARIABLE_DATA *name, VARIABLE_DATA *limits )
{
    return NULL;
}


/* -------------------------------------------------------------- */
/*
 * Name();
 * Converts any old parameter into its string 'name' and returns that string
 */
VARIABLE_DATA *func_name( void * owner, int type, VARIABLE_DATA *var )
{
    char *value;
    char _var[MSL];

    STR_PARAM(var,_var);

    switch ( type ) {
        case TYPE_STRING: 
            value = str_dup( (char *)(owner) );
          break;
        case TYPE_ACTOR:
            value = str_dup( STR(ACTOR(owner),name) );
          break;
        case TYPE_PROP:
            value = str_dup( STR(PROP(owner),short_descr) );
          break;
        case TYPE_SCENE:
            value = str_dup( SCENE(owner)->name );
          break;
        default: value = str_dup( "" ); break;
    }

    RETURNS(TYPE_STRING,value);    
}
/* -------------------------------------------------------------- */
/*
 * Vnum();         RESERVED
 * Vnum(target); 
 * Converts any old parameter into its vnum and returns that number
 */
VARIABLE_DATA *func_vnum( void * owner, int type, VARIABLE_DATA *target )
{
    char _target[MSL];
    char buf[MSL];
    int vnum;

    STR_PARAM(target,_target);

    if ( IS_PRESENT(target,_target) ) {
        FOREACH(target->type,
            vnum= IS_NPC(ACTOR(target->value)) ? 
                         ACTOR(target->value)->pIndexData->vnum : 0,
            vnum = PROP(target->value)->pIndexData->vnum,
            vnum = SCENE(target->value)->vnum,
            vnum = 0);
    }
    else
    {
         FOREACH(type,
            vnum= IS_NPC(ACTOR(owner)) ? 
                         ACTOR(owner)->pIndexData->vnum : 0,
            vnum = PROP(owner)->pIndexData->vnum,
            vnum = SCENE(owner)->vnum,
            vnum = 0);
    }

    snprintf( buf, MAX_STRING_LENGTH, "%d", vnum );

    RETURNS(TYPE_STRING,str_dup(buf));
}

/* -------------------------------------------------------------- */
/*
 * Eval();
 * Evaluates an expression.
 */
VARIABLE_DATA *func_eval( void * owner, int type, VARIABLE_DATA *value )
{
    char _value[MSL];

    STR_PARAM(value,_value);

    RETURNS(TYPE_STRING,str_dup(_value));    
}



/* -------------------------------------------------------------- */
/*
 * Rndplr();
 * Returns the name of a random player in the scene.
 */
VARIABLE_DATA *func_rndplr( void * owner, int type, VARIABLE_DATA *from )
{
    SCENE_INDEX_DATA *pScene;
    char buf[MAX_STRING_LENGTH];
    PLAYER_DATA *och, *rch = NULL;
    PLAYER_DATA *ch;
    char _from[MSL];

    STR_PARAM(from,_from);

    pScene =     get_scene_index( atoi( _from ) );

    och = actor_list;

    if ( pScene == NULL )
    switch ( type )
    {
        case TYPE_PROP: pScene = PROP(owner)->in_scene;
                       och = NULL;
         break;
        case TYPE_ACTOR: pScene = ACTOR(owner)->in_scene;
                       och = ACTOR(owner);
         break;
        case TYPE_SCENE: pScene = SCENE(owner);
                       och = NULL;
         break;
        default: pScene = NULL;  och = NULL; break;
    }

    if (pScene == NULL)
    {
        snprintf( buf, MAX_STRING_LENGTH, "me" );
    }
    else
    {
        int count;

        count = 0;
        for ( ch = pScene->people; ch != NULL; ch = ch->next_in_scene )
        {
            if ( IS_NPC(ch) )
            continue;

            if ( och != NULL )
            {
                if ( !can_see( och, ch ) )
                continue;
            }

            count++;
        }

        if ( count > 1 )
        count -= number_range(0,count-1);

        for( ch = pScene->people; count > 0 && ch != NULL; ch = ch->next_in_scene )
        {
            if ( IS_NPC(ch) )
            continue;

            /*
             * Check for invisible players or blinded actors.
             * (applies to TYPE_ACTOR owners only)
             */
            if ( och != NULL )
            {
                if ( !can_see( och, ch ) )
                continue;
            }


            if ( --count == 0 )
            {
                rch = ch;
            }
        }


        if ( rch == NULL )
        {
            snprintf( buf, MAX_STRING_LENGTH, "me" );
        }
        else
        {
            snprintf( buf, MAX_STRING_LENGTH, "%s", STR(rch, name) );
        }
    }

    RETURNS(TYPE_STRING,str_dup(buf));    
}



/* -------------------------------------------------------------- */
/*
 * Dig();
 * Digs an exit in the world.
 */
VARIABLE_DATA *func_dig( void * owner, int type, VARIABLE_DATA *loc, 
                                                 VARIABLE_DATA *dir,
	                                         VARIABLE_DATA *dest )
{
    SCENE_INDEX_DATA *pScene = NULL;
    SCENE_INDEX_DATA *toScene;
    char _loc[MSL];
    char _dest[MSL];
    char _dir[MSL];
    int door;
   
    STR_PARAM(loc,_loc);
    STR_PARAM(dest,_dest);
    STR_PARAM(dir,_dir);

    pScene =     get_scene_index( atoi( _loc ) );
    toScene =     get_scene_index( atoi( _dest ) );
    door = get_dir( _dir );

    if ( pScene != NULL && toScene != NULL ) {

    /* dig the exit */
         if ( pScene->exit[door] == NULL )
		 pScene->exit[door] = new_exit();

		 pScene->exit[door]->to_scene = toScene;
		 pScene->exit[door]->vnum =toScene->vnum;

	 door                           = rev_dir[door];

    /* dig the exit */
         if( toScene->exit[door]== NULL )
	 toScene->exit[door]             = new_exit();

	 toScene->exit[door]->to_scene    = pScene;
	 toScene->exit[door]->vnum       = pScene->vnum;
    }

    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Undig();
 * Removes an exit (unlink)
 */
VARIABLE_DATA *func_undig( void * owner, int type, VARIABLE_DATA *loc, 
                                                 VARIABLE_DATA *dir )
{
    SCENE_INDEX_DATA *pScene = NULL;
    char _loc[MSL];
    char _dir[MSL];
    int door;

    if ( (loc->type == TYPE_SCENE) )
    pScene = SCENE(loc->value);
    else
    {
    STR_PARAM(loc,_loc);
    pScene =     get_scene_index( atoi( _loc ) );
    }

    STR_PARAM(dir,_dir);

    door = get_dir( _dir );

    /* dig the exit */
    if ( pScene != NULL 
      && pScene->exit[door] != NULL )
    {
        EXIT_DATA *oexit;
        oexit = pScene->exit[door]->to_scene 
             && pScene->exit[door]->to_scene->exit[rev_dir[door]] 
             ? pScene->exit[door]->to_scene->exit[rev_dir[door]] : NULL;

	if ( oexit != NULL )
	{
	    free_exit( oexit );
	    pScene->exit[door]->to_scene->exit[rev_dir[door]] = NULL;
	}

	free_exit( pScene->exit[door] );
	pScene->exit[door] = NULL;
    }

    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * AddOwed();
 * Bills add up.  Add to bounty owed to player.
 */
VARIABLE_DATA *func_addowed( void * owner, int type, VARIABLE_DATA *target, 
                          VARIABLE_DATA *gain )
{
    PLAYER_DATA *ch;
    char _target[MSL];
    char _gain[MSL];

    if ( (target->type == TYPE_ACTOR) )
    ch = ACTOR(target->value);
    else 
    {
    STR_PARAM(target,_target);
    ch = 
    get_actor_world( type == TYPE_ACTOR ? ACTOR(owner) : actor_list, _target );
    }

    STR_PARAM(gain,_gain);
    
    if ( ch == NULL ) return NULL;

    ch->owed += atoi( _gain );
    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * AddBounty();
 * Add to bounty on player.
 */
VARIABLE_DATA *func_addbounty( void * owner, int type, VARIABLE_DATA *target, 
                          VARIABLE_DATA *gain )
{
    PLAYER_DATA *ch;
    char _target[MSL];
    char _gain[MSL];

    if ( (target->type == TYPE_ACTOR) )
    ch = ACTOR(target->value);
    else 
    {
    STR_PARAM(target,_target);
    ch = 
    get_actor_world( type == TYPE_ACTOR ? ACTOR(owner) : actor_list, 
                   _target );
    }

    STR_PARAM(gain,_gain);
    
    if ( ch == NULL ) return NULL;

    ch->bounty += atoi( _gain );
    return NULL;
}

/* -------------------------------------------------------------- */
/*
 * Bounty();
 * Return the value of bounty on a target.
 */
VARIABLE_DATA *func_bounty( void * owner, int type, VARIABLE_DATA *target )
{
    char buf[MAX_STRING_LENGTH];
    int val;
    PLAYER_DATA *ch;
    char _target[MSL];


    if ( (target->type == TYPE_ACTOR) )
    ch = ACTOR(target->value);
    else 
    {
    STR_PARAM(target,_target);
    ch = 
    get_actor_world( type == TYPE_ACTOR ? ACTOR(owner) : actor_list, 
                   _target );
    }

    val = ch == NULL ? -1 : ch->bounty;

    snprintf( buf, MAX_STRING_LENGTH, "%d", val );

    RETURNS(TYPE_STRING,str_dup(buf));    
}


/* -------------------------------------------------------------- */
/*
 * Owed();
 * Return amount owed to a player.
 */
VARIABLE_DATA *func_owed( void * owner, int type, VARIABLE_DATA *target )
{
    char buf[MAX_STRING_LENGTH];
    int val;
    PLAYER_DATA *ch = NULL;
    char _target[MSL];

    if ( (target->type == TYPE_ACTOR) )
    ch = ACTOR(target->value);
    else 
    {
    STR_PARAM(target,_target);
    ch = 
    get_actor_world( type == TYPE_ACTOR ? ACTOR(owner) : actor_list, 
                   _target );
    }

    val = ch == NULL ? -1 : ch->owed;

    snprintf( buf, MAX_STRING_LENGTH, "%d", val );

    RETURNS(TYPE_STRING,str_dup(buf));    
}

/* -------------------------------------------------------------- */
/*
 * Level();
 * Returns the level.
 */
VARIABLE_DATA *func_level( void * owner, int type, VARIABLE_DATA *target )
{
    char buf[MAX_STRING_LENGTH];
    int val;
    PLAYER_DATA *ch;
    char _target[MSL];

    if ( (target->type == TYPE_ACTOR) )
    ch = ACTOR(target->value);
    else 
    {
    STR_PARAM(target,_target);
    ch = 
    get_actor_world( type == TYPE_ACTOR ? ACTOR(owner) : actor_list, 
                   _target );
    }

    val = ch == NULL ? -1 : ch->exp_level;

    snprintf( buf, MAX_STRING_LENGTH, "%d", val );

    RETURNS(TYPE_STRING,str_dup(buf));    
}



/* -------------------------------------------------------------- *
 * This section of the source is bound under 
 * an agreement with its original author. (me)
 */


/* -------------------------------------------------------------- */
/*
 * List and stack functions.
 */
/* -------------------------------------------------------------- */

/*
 * Push();
 * Push a value onto a stack.
 */
VARIABLE_DATA *func_push( void* owner, int type,  VARIABLE_DATA *stack,
                          VARIABLE_DATA *value )
{
    char _stack[MSL];
    char _value[MSL];
    char buf[MAX_STRING_LENGTH];

    STR_PARAM(stack,_stack);
    STR_PARAM(value,_value);

    snprintf( buf, MAX_STRING_LENGTH, "%s;%s", _value, _stack );

    RETURNS(TYPE_STRING,str_dup(buf));    
}

/* -------------------------------------------------------------- */
/*
 * Pop();
 * Pop the top value off a stack.
 */
VARIABLE_DATA *func_pop( void * owner, int type, VARIABLE_DATA *stack )
{
    char buf[MAX_STRING_LENGTH];
    char _stack[MSL];
    char *argument;

    STR_PARAM(stack,_stack);
    if ( _stack[0] == '\0' ) return NULL;

    argument = one_argument( _stack, buf );

    RETURNS(TYPE_STRING,str_dup(argument));
}


/* -------------------------------------------------------------- */
/*
 * Sort();
 * Sorts a list.
 * Masks=Ascending, Descending
 * Not yet implemented.
 */
VARIABLE_DATA *func_sort( void * owner, int type, VARIABLE_DATA *stack,
                          VARIABLE_DATA *mask )
{
/*
    char _stack[MSL];
    char _mask[MSL];
 */
    return NULL;
}


/* -------------------------------------------------------------- */
/*
 * LRnd();
 * Returns a random value from a list stack.
 */
VARIABLE_DATA *func_lrnd( void * owner, int type, VARIABLE_DATA *stack )
{
    char argument[MAX_STRING_LENGTH];
    char _stack[MSL];
    char *p;
    int x;

    STR_PARAM(stack,_stack);

    p=_stack;
    for ( x = 0; argument[0] != '\0'; x++ ) p= one_line( p, argument );

    if ( x == 1 ) { RETURNS(TYPE_STRING,str_dup(_stack)); }

    p=_stack;
    for ( x = number_range(0,x-1);  x > 0;  x-- ) p=one_line(p,argument);

    RETURNS(TYPE_STRING,str_dup(argument));    
}


/* -------------------------------------------------------------- */
/*
 * LShift();
 * Shift a list left;
 */
VARIABLE_DATA *func_lshift( void * owner, int type,VARIABLE_DATA *stack )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char _stack[MSL];
    char *p, *q;
    int x;

    STR_PARAM(stack,_stack);
    p = one_line( _stack, buf );  /* returns "xxx;" i believe */

    x=0; q=buf;
    while( *q!='\0'&&*q!=';' ) q++;
    *q='\0';
    
    sprintf( buf2, "%s;%s", p, buf );
    RETURNS(TYPE_STRING,str_dup(buf2));    
}



/* -------------------------------------------------------------- */
/*
 * RShift();
 * Shifts list values right (wraps).
 */
VARIABLE_DATA *func_rshift( void * owner, int type, VARIABLE_DATA *stack )
{
    char _stack[MSL];
    char buf[MSL];
    char *p, *q;
    int x;

    STR_PARAM(stack,_stack);

    q = p = _stack; x=0;  
    while ( *p++!='\0' ) { x++; q=p; }

    while ( _stack[x] != ';' && x >= 0 ) { x--; q--; };
    if ( *q != '\0' ) q++;

    p = _stack;
    if ( x ) _stack[x]='\0'; 

    snprintf( buf, MAX_STRING_LENGTH, "%s;%s",  p, q );

    RETURNS(TYPE_STRING,str_dup(buf));    
}


/* -------------------------------------------------------------- */
/*
 * Empty();
 * List functions.
 * These function return list stacks.
 * Empty set is returned if nothing found.
 * Check for this using the empty() function.
 */
VARIABLE_DATA *func_empty( void * owner, int type, VARIABLE_DATA *stack )
{
    VARIABLE_DATA *vd = NULL;
    char _stack[MSL];

    STR_PARAM(stack,_stack);
    return vd;
}

/* -------------------------------------------------------------- */
/*
 * Users();
 * Returns a list stack of users currently online.
 * Masking not yet implemented.
 */
VARIABLE_DATA *func_users( void * owner, int type,  VARIABLE_DATA *mask )
{
    PLAYER_DATA *ch;
    char buf[MAX_STRING_LENGTH];
    char _mask[MSL];
    
    STR_PARAM(mask,_mask);

    buf[0] = '\0';
    for ( ch = actor_list;  ch != NULL;  ch = ch->next ) 
        if ( ch->desc != NULL ) 
            snprintf( buf, MAX_STRING_LENGTH, "%s %s", buf, NAME(ch) );
        
    RETURNS(TYPE_STRING,str_dup(buf));    
}


/* -------------------------------------------------------------- */
/* 
 * RndDir();
 * Ideal version: Gives you a random valid direction
 */
VARIABLE_DATA *func_rnddir( void * owner, int type )
{
/*    SCENE_INDEX_DATA *pScene; */

   /*
    int doors=0;
    int i;
 
    for ( i =0; i < MAX_DIR; i++ ) if ( pScene->exit[i] != NULL ) 
    doors++;

    doors = number_range(1,doors);
    */

    RETURNS(TYPE_STRING, str_dup( dir_name[URANGE(0,number_range(0,MAX_DIR),MAX_DIR-1)]) );
}

/* -------------------------------------------------------------- */
/*
 * GetDir();
 * Returns the vnum to the direction by name or by number.
 */
VARIABLE_DATA *func_getdir( void * owner, int type, VARD *loc, VARD *dir ) {
    VARIABLE_DATA *newvar;
    SCENE_INDEX_DATA *pScene;
    char buf[MAX_STRING_LENGTH];
    char _dir[MSL]; 
    char _loc[MSL];
    int door;

    STR_PARAM(loc,_loc);
    STR_PARAM(dir,_dir);

    pScene = get_scene_index( atoi(_loc) );
    if ( pScene ) {

    door = is_number(_dir) ? atoi(_dir) : get_dir( _dir );

    if ( pScene->exit[door] && pScene->exit[door]->to_scene ) {
        int vnum = pScene->exit[door]->to_scene->vnum;

        snprintf( buf, MAX_STRING_LENGTH, "%d", vnum );
        newvar = new_variable( TYPE_STRING, str_dup( buf ) );
    } else newvar=NULL;


    } else newvar=NULL;

    return (newvar);
}

/* -------------------------------------------------------------- */
/*
 * Returns a list stack of the event queue names.
VARIABLE_DATA *func_queue( VARIABLE_DATA *mask )
{
    VARIABLE_DATA *vd = NULL;
    QUEUE_DATA *qd;
    char buf[MAX_STRING_LENGTH];
    char _mask[MSL];

    STR_PARAM(mask,_mask);

    buf[0] = '\0';
    for ( qd = event_queue; qd != NULL;  qd = qd->next ) 
            snprintf( buf, MAX_STRING_LENGTH, "%s;%s", buf, qd->name );
        
    vd = new_variable( );
    vd->value = str_dup( buf );
    return vd;
    RETURNS(TYPE_STRING,str_dup(buf));    
}
	 */



/* -------------------------------------------------------------- */
/*
 * Returns a list stack of the prop names in the database.
 * (huge)
VARIABLE_DATA *func_props( VARIABLE_DATA *mask )
{
    PROP_DATA *prop;
    char buf[MAX_STRING_LENGTH];
    char _mask[MSL];

    STR_PARAM(mask,_mask);

    buf[0] = '\0';
        
    return new_variable( TYPE_STRING, str_dup(buf) );
}
 */




/* -------------------------------------------------------------- */
/*
 * Returns a list stack of the command list names.
VARIABLE_DATA *func_commands( VARIABLE_DATA *mask )
{
    COMMAND_DATA *cd;
    char buf[MAX_STRING_LENGTH];
    char _mask[MSL];

    STR_PARAM(mask,_mask);

    buf[0] = '\0';
    for ( cd = command_list; cd != NULL;  cd = cd->next ) 
            snprintf( buf, MAX_STRING_LENGTH, "%s;%s", buf, cd->name );
        
    return new_variable( TYPE_STRING, str_dup(buf) );
}
 */


/* -------------------------------------------------------------- */
/*
 * Iterates through each value in a list stack.
 * Sets a list of variables equal to the supplied value.
 */
VARIABLE_DATA *func_foreach( void * owner, int type, VARIABLE_DATA 
*stack, VARIABLE_DATA *code )
{
    char var_name[MAX_STRING_LENGTH];
    char _stack[MSL];
    char _code[MSL];
    char *argument;

    STR_PARAM(stack,_stack);
    STR_PARAM(code,_code);

    argument = _stack;
    while ( *argument != '\0' ) {
        argument = one_line( argument, var_name );
/*
        set_variable( var_name, _code );
 */
    }

    return NULL;
}



/* -------------------------------------------------------------- */
/*
 * Iterates through each value in a list stack.
 * Returns a list of variables equal to the stack.
 */
VARIABLE_DATA *func_each( void * owner, int type, VARIABLE_DATA *stack )
{
#if defined (NEVER)
    VARIABLE_DATA *vd;
    char var_name[MAX_STRING_LENGTH];
    ch/r astr[MAX_STRING_LENGTH];
    char bstr[MAX_STRING_LENGTH];
    char _stack[MSL];
    char _code[MSL];
    char *argument;

    STR_PARAM(stack,_stack);
    STR_PARAM(code,_code);

    final[0] = '\0';
    argument = _stack;
    while ( *argument != '\0' ) {
        argument = one_argument( argument, var_name );
        vd = find_variable( var_name );

        if ( vd == NULL ) continue;
        snprintf( buf, MAX_STRING_LENGTH, "%s;%s", buf, vd->value );
    }

    vd = new_variable( );
    vd->value = str_dup( buf );

    return new_variable( TYPE_STRING, str_dup(buf) );
#endif
   return NULL;
}



/*
 * Builder function.
 * A builder function accesses the vnum of a builder script.
 * VNUMs are auto-generated from a special dynamic space
 * area designated as "mainland.zone"
 *
 * "DSpace.zone" is probably a better name for it, as the
 * areas built within it are attached where-ever the MUD
 * permits, and are not necessarily a "mainland"
 *
 * Builder scripts are called in this form:
 * build( script vnum, location, owner, param1, param2, param3 );
 *
 * where: script vnum is the vnum of the script to execute,
 *        location in area we are attempting to build in,
 *        new target owner (player or facilitating object)
 *        param1, 2, 3 optional parameters for script set by scripter
 *
 * Params are defined in the builder script, but are usually
 * used in filling in descriptions or providing object lists.
 *
 * Builder scripts are executed all at once and are load heavy.
 *
 * Builder scripts look like this:
 *
 * * comment
 * # comment
 * title ==> called at the beginning, sets the name of the area
 * actor ==> creates new %actor1,2,3,4,5..n%
 * name <name>
 * short <short>
 * attrib <value>
 * attack <type> <value>
 * flags <flag list>
 * description 
 * <content>
 * @
 *
 * prop 
 *
 * scene ==> creates new %scene1,2,3,4...n%
 * name <title>
 * description
 * <content>
 * @
 *
 * <direction> <flags>
 * <direction> to <variable>
 * <direction> name <keyword name>
 * <direction> door
 * <direction> key <variable>
 * <direction> description
 * <content>
 * @
 * dig <direction> <scene vnum> ==> creates new scene in %scene2,3,4..n% digs from old scene w/o focus change
 * cue ==> populates 
 *
 * scene <variable> ==> changes focus to another scene
 * actor <variable> ==> changes focus to another actor
 * prop <variable> ==> changes focus to another prop
 *
 * <series of editing commands>
 * cue <scene variable> <prop or actor variable> ==> populates 
 *
 * Mass scene production:
 * cube <w> <d> <h>
 * grid <w> <h>
 * maze <w> <h>         generates a non-uniform grid
 * 3dmaze <w> <d> <h>   generates a non-uniform cube
 * labyrinth <list1>, <list2>   randomly populates all rooms with random variations NYI
 *
 * Calling the above two functions will generate a series of interconnected rooms,
 * and populate the %cube% and %grid% variables, where %cube1,2,3% refers to
 * the room at w=1, d=2, h=3, and %grid1,2% refers to the room at 1,2 -- no spaces
 * in the name of the variable is required, so no %grid 1, 2% or anything like that
 * 
 * Builder scripts output their results to NOTIFY/scripts
 * Builder scripts auto-format their descriptions
 * Returns scene object '%location%' with success, 0 with failure, where failure may be inability to 
 * connect to any locations in the same area as the given location parameter, or that the player
 * owns too much?
 *
Example 1: Generating a Player-Owned Castle

# Map of generator:
# 
# Level 1                 Level 2
#   +-+                    +-+
#   |x|                    | |   Bottom is a grid, top is a donut
#   |x|                    | |   + = tower interior | = rampart
#   |x|                    | |   x = courtyard
#   A-+                    +=+   A = guardhouse

title %owner%'s Castle

#Create a guard named after the player, known as %actor1%

actor
flags sentinel
script 19         
name %owner%'s guard
short %owner%'s guard
long %owner%'s guard is standing here, keeping watch.
description
%owner%'s guard is a burly, mean-looking ogre with a missing tooth.
One of his eyes is red and the other is blue!
@

#Create the guardpost of a larger castle

 # first create the key to the guardpost, %prop1%
prop
name %owner%'s guardhouse key
short %owner%'s guardhouse key
description
It's the key to the guardhouse of %owner%'s Castle.

 # now create the guard post room aka %scene1%, use the key on the door
scene
name Guardpost
description
This is a modest guard tower positioned at the southwestern corner
of %param1%'s castle.
@
dig east
east door wood door
east closed locked
east key %prop1%
flags enclosed
cue %actor1% %prop1% 

# switch to newly created room
scene %scene2%  # variable was populated with 'dig east' command above
name Gate at Guardpost
attach south %location%     # attempts to attach south, if south is not available, creates path connector
flags buildable

# generate a room template
scene
name Castle Ramparts
flags template
description
You are standing on the walls of %owner%'s castle, looking
over the fair lands that surround it.  The walls are made
of granite ramparts used as a defensive measure against
seige.
@

#generate a series of rooms using a template to outline the castle walls
scene
flags saving
ref %scene3%
name Western Ramparts
description
You are on the western side of the castle walls.
dig south
dig north
north description
You can see the wall extend to the north.
@
south description
You can see the wall extend to the south.
@

# create %prop2%, the master key
prop 
name %owner%'s castle key
short %owner%'s castle key
long %owner%'s castle key was foolishly dropped.
description
The master key to %owner%'s castle is fashioned from wrought iron.
@

scene %scene5%         # generated with dig south
name Southwest Ramparts Near Tower
dig north
north door iron shod door
north key %prop2%

scene %scene6%         # generated with dig north
name Northwest Ramparts Near Tower
description
You are on the northwest ramparts of %owner%'s castle, near the northwest tower.
@

scene %scene7%         # dug from %scene5%, so its the tower
name Northwest Tower
flags template enclosed saving
description
You are in a tower looking out over the lands that surround %owner%'s castle.
@
dig east
east door iron shod door
east key %prop2%
dig down
down name spiral stair

scene %scene8%
name Northern Ramparts
ref %scene3%
east dig                   # generates %scene10%

scene %scene9%    # base of the tower
flags template enclosed
description

.
.
. 
until it's all defined.  this script appears as 'castle' in the database
you can have it load scripts onto the rooms that simulate seiges during
certain periods of time or whatever; one of the last calls is 'attach',
which attaches the newly generated area to the nearest location.  
'attach' should really only be called once.

Example 2: Generating a Lagoon from a 3x3x3 Cube

title Lagoon
cube 3 3 3

 */

int prop_where( PROP_INDEX_DATA *p ) {
  if ( !IS_SET(p->wear_flags, ITEM_TAKE) ) 
    return SPAWN_LOC_INSCENE;

  if ( IS_SET(p->wear_flags, ITEM_WEAR_FINGER) ) return WEAR_FINGER_L;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_NECK) ) return WEAR_NECK_1;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_BODY) ) return WEAR_BODY;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_HEAD) ) return WEAR_HEAD;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_LEGS) ) return WEAR_LEGS;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_FEET) ) return WEAR_FEET;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_HANDS)) return WEAR_HANDS;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_ARMS) ) return WEAR_ARMS;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_LEGS) ) return WEAR_LEGS;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_SHIELD) ) return WEAR_SHIELD;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_ABOUT) ) return WEAR_ABOUT;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_WAIST) ) return WEAR_WAIST;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_WRIST) ) return WEAR_WRIST_R;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_SHOULDER) ) return WEAR_SHOULDER_L;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_ANKLE) ) return WEAR_ANKLE_L;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_EAR) ) return WEAR_EAR_L;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_FOREHEAD)) return WEAR_FOREHEAD;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_FACE)) return WEAR_FACE;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_NOSE)) return WEAR_NOSE;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_PANTS)) return WEAR_PANTS;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_SHIRT)) return WEAR_SHIRT;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_STOCKING)) return WEAR_STOCKING;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_SHOULDERS)) return WEAR_SHOULDERS;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_PIN)) return WEAR_PIN;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_BELT)) return WEAR_BELT_1;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_ASBELT)) return WEAR_ASBELT_1;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_PIN)) return WEAR_PIN;
  if ( IS_SET(p->wear_flags, ITEM_WEAR_LOIN)) return WEAR_PIN;
  return WEAR_NONE;
}


V *func_build( void *owner, int type, V *svnum, V *location, V *target, V *p1, V *p2, V *p3 ) {

   PLAYER_DATA *o=NULL;          // Demystified 'target' "owner"
   char _o[MSL];                 // Vnum if Actor, name if Player
   SCENE_INDEX_DATA *loc;        // Location, or next best location, to attach new area
   SCRIPT_DATA *bscript;         // The pointer to the demystified "builder script"
   char *k;                      // Pointer to the current location on the builder script
   char buf[MSL];                // General purpose string buffer
   char output[MSL];             // Contains output to NOTIFY/scripts
   char arg[MSL];                // Used to snap off pieces of the builder script.
   char title[MSL];              // Holds the "title" component for the building title command
   char path_title[MSL];         // Holds our "path connector" room title
   char path_descr[MSL];         // Holds our "path connector" room description
   V *vars=NULL;                 // All variables relevant to execution of the building script

   SCENE_INDEX_DATA *s=NULL;     // Pointers to active objects, initially "NULL"  
   ACTOR_INDEX_DATA *a=NULL;
   PROP_INDEX_DATA  *p=NULL;
   int active=TYPE_STRING;       // Which type are we currently building?  Initial nonsense value

   int tvnum[4];                 // Top Vnums increased when we generate new objects, one for each type
   int created[4];               // Tally of number created
   ZONE_DATA *w;                 // The "world" zone, usually "mainland.zone"

   // init string space
   title[0]='\0';
   output[0]='\0';

   // find our dynamic "world" zone where we've allocated a million plus vnums,
   // this is where we're generating the new database entries.
   for ( w = zone_first; w != NULL; w=w->next ) if ( !str_cmp( w->filename, "mainland.zone" ) ) break;
   if ( !w ) { // world zone is missing, i guess this feature is "off"
     NOTIFY( "Build(): No world zone 'mainland.zone' to be found!\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
     RETURNS(TYPE_STRING,str_dup("0"));
   }

   // figure out the top vnums in the world "zone"
   // tvnum[0] isn't really used and it's initialized to 0
   // start created[] counters at 0
   created[0]=created[1]=created[2]=created[3]=tvnum[0]=0;
   for ( tvnum[TYPE_PROP]=w->lvnum;  tvnum[TYPE_PROP]<w->uvnum; tvnum[TYPE_PROP]++ ) 
       if ( get_prop_index( tvnum[TYPE_PROP] ) == NULL ) break;
   for ( tvnum[TYPE_ACTOR]=w->lvnum;  tvnum[TYPE_ACTOR]<w->uvnum; tvnum[TYPE_ACTOR]++ ) 
       if ( get_actor_index( tvnum[TYPE_ACTOR] ) == NULL ) break;
   for ( tvnum[TYPE_SCENE]=w->lvnum;  tvnum[TYPE_SCENE]<w->uvnum; tvnum[TYPE_SCENE]++ ) 
       if ( get_scene_index( tvnum[TYPE_SCENE] ) == NULL ) break;

   // locate our script
   if ( svnum->type != TYPE_STRING ) bscript=NULL;
   else {
     int vnum=atoi((char *)(svnum->value));
     bscript = get_script_index( vnum );
     if ( bscript == NULL || bscript->type != TRIG_BUILDER ) {
         snprintf( buf, MSL, "Build(): Bad script vnum %d; does not exist or is not right type\n\r", vnum );
         NOTIFY( buf, LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
         RETURNS(TYPE_STRING,str_dup("0"));
     }
   }

   // find a suitable location to connect to, requires us to have at least
   // one buildable room with a free cardinal exit (not up or down)
   // inside the same area, or it fails.  we can use a path connector to
   // attach something to a room that has redundant exits.
   if ( location->type == TYPE_SCENE ) loc= (SCENE_INDEX_DATA *)(location->value);
   else
   if ( location->type == TYPE_STRING ) {
     int vnum=atoi( (char *) (location->value) );
     loc= get_scene_index( vnum );
   } else {
     switch ( location->type ) { case TYPE_ACTOR: loc= ACTOR(location->value)->in_scene; break;
                                 case TYPE_PROP:  loc= PROP(location->value)->in_scene;
                                 if ( !loc ) loc= PROP(location->value)->carried_by ? 
                                                  PROP(location->value)->carried_by->in_scene :
                                                  (PROP(location->value)->in_prop ? 
                                                   PROP(location->value)->in_prop->in_scene : NULL);
                                      break;
                                 default: loc=NULL;
                               }
   }
   if ( !loc ) {
         NOTIFY( "Build(): Unable to find a suitable attachment location with that target\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
         RETURNS(TYPE_STRING,str_dup("0"));
   } else { // determine if exits are available in the default loc
     int dir;
     bool found=FALSE;
     for ( dir=0; dir < MAX_DIR; dir++ ) {
       if ( dir == DIR_UP || dir == DIR_DOWN ) continue;
       if ( loc->exit[dir] == NULL ) { found=TRUE; break; }
     }
     if ( !found ) { // the default loc is full, try others in same area
       ZONE_DATA *pZone=loc->zone;
       int vnum=0;
       found=FALSE;
       for ( vnum = pZone->lvnum; vnum < pZone->uvnum; vnum++ ) { 
         loc = get_scene_index( vnum ); 
         if ( loc && IS_SET(loc->scene_flags,SCENE_BUILDABLE) ) 
          for ( dir =0; dir < MAX_DIR; dir++ ) {
            if ( dir == DIR_UP || dir == DIR_DOWN ) continue;
            if ( loc->exit[dir] == NULL ) { found=TRUE; break; }
          }
         if ( found ) break;
       }
       if ( !loc || !found ) {
         NOTIFY( "Build(): Unable to find an attachment location within the full? area\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
         RETURNS(TYPE_STRING,str_dup("0"));
       }
     }
   }

   // Figure out who the new owner is
   switch ( target->type ) {
       default: o=NULL; break;
       case TYPE_ACTOR: o=ACTOR(target->value); break;
   }
   if ( !o ) {
    switch ( type ) {
       default: o=NULL; break;
       case TYPE_ACTOR: o=ACTOR(target->value); break;
    }      
   }
   if ( !o ) {
     NOTIFY( "Build(): Unable to locate a suitable owner, aborting\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
     RETURNS(TYPE_STRING,str_dup("0"));
   }
   if ( IS_NPC(o) ) snprintf( _o, MSL, "%d", o->pIndexData->vnum );
   else snprintf( _o, MSL, "%s", o->name );

   // Duplicate parameters and populate our temporary variable list
   { V *v;

   if ( p1 ) { v=new_variable_data(); v->next=vars; vars=v; v->type=p1->type; v->name=str_dup( "%1%" );
   switch( p1->type ) {
        case TYPE_ACTOR: case TYPE_SCENE:
         case TYPE_PROP: v->value = p1->value; break;
       case TYPE_STRING: v->value = (void *) str_dup( (char *) p1->value ); break;
            default: break;
   }
   }

   if ( p2 ) { v=new_variable_data(); v->next=vars; vars=v; v->type=p2->type; v->name=str_dup( "%2%" );
   switch( p2->type ) {
        case TYPE_ACTOR: case TYPE_SCENE:
         case TYPE_PROP: v->value = p2->value; break;
       case TYPE_STRING: v->value = (void *) str_dup( (char *) p2->value ); break;
            default: break;
   }
   }

   if ( p3 ) { v=new_variable_data(); v->next=vars; vars=v; v->type=p3->type; v->name=str_dup( "%3%" );
   switch( p3->type ) {
        case TYPE_ACTOR: case TYPE_SCENE:
         case TYPE_PROP: v->value = p3->value; break;
       case TYPE_STRING: v->value = (void *) str_dup( (char *) p3->value ); break;
            default: break;
   }
   }

   }

   // End optional parameters

   /*
    * Verifier
    * You can add a verifier here between this comment and the next.
    * The verifier will find simple issues within the code such
    * as too few/many @ for every 'description' call.
    */
   
   // Ok, we've got a suitable attachment "loc" and a valid builder script.
   // Let's execute the script as best we can.

   {
   
     k=bscript->commands;   // let's put cursor k at the home position
     while ( 1 ) {

       // Did we fill the space?
       if ( tvnum[1] > w->uvnum || tvnum[2] > w->uvnum || tvnum[3] > w->uvnum ) break;

       // Skip ahead spaces
       while ( *k != '\0' && isspace(*k) ) k++;
       if ( *k == '\0' ) break;

       // Skip comments
       while ( *k == '#' || *k == '*' ) { 
         while ( *k != '\0' && *k != '\n' ) k++;
         if ( *k == '\0' ) break;
         if ( *k == '\r' ) k++;
         if ( *k == '\0' ) break;
         while ( *k != '\0' && isspace(*k) ) k++;
       }        
       if ( *k == '\0' ) break;

       k=one_argument(k,arg);         // tear off a command
       
       if ( !str_cmp( arg, "scene" ) ) {  // creates a new scene or switches focus

          one_argument( k, arg );
          if ( arg[0] == '%' ) { // then it's scene <variable>, so let's find it
            V *v;
            k=one_argument(k,arg);
            v=get_variable( vars, arg );
            if ( !v || !v->value ) {
              NOTIFY( "Build(): scene %var% is referencing invalid varname\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
            } else
            if ( v->type != TYPE_SCENE ) {
              NOTIFY( "Build(): scene %var% is referencing the wrong type\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
            } else {
              s=(SCENE_INDEX_DATA *)v->value;  active=TYPE_SCENE;
            }
          } else { // it's just the word scene, followed by a different command, so create
           int iHash;  V *n;
           s = new_scene_index();              s->zone =w;  s->owner = str_dup( _o );
           s->vnum=tvnum[TYPE_SCENE]++;        iHash = s->vnum % MAX_KEY_HASH; 
           s->next = scene_index_hash[iHash];  scene_index_hash[iHash]  = s;
           n = new_variable_data();
           snprintf( buf, MSL, "%%scene%d%%", ++created[TYPE_SCENE] );  // make a new variable
           n->name = str_dup( buf );     n->type = TYPE_SCENE;
           n->value = (void *) s;        n->next = vars; vars=n;
           active = TYPE_SCENE;
          }

       } else
       if ( !str_cmp( arg, "dig" ) ) {    // digs a new exit to a new scene w/o switching focus
          int iHash;  V *n; SCENE_INDEX_DATA *x;
          int dir;
          k=one_argument(k, arg);
          dir = get_dir( arg );
          if ( active == TYPE_SCENE ) {
           x = new_scene_index();              x->zone =w;  x->owner = str_dup( _o );
           x->vnum=tvnum[TYPE_SCENE]++;        iHash = x->vnum % MAX_KEY_HASH; 
           x->next = scene_index_hash[iHash];  scene_index_hash[iHash]  = x;
           n = new_variable_data();
           snprintf( buf, MSL, "%%scene%d%%", ++created[TYPE_SCENE] );  // make a new variable
           n->name = str_dup( buf );     n->type = TYPE_SCENE;
           n->value = (void *) x;        n->next = vars; vars=n;
           if ( !s->exit[dir] ) s->exit[dir] = new_exit(); s->exit[dir]->to_scene=x; // dig exit
           if ( !x->exit[rev_dir[dir]] ) x->exit[rev_dir[dir]] = new_exit(); x->exit[rev_dir[dir]]->to_scene=s;
          } else NOTIFY( "Build(): dig called with no active scene\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
       } else
       if ( !str_cmp( arg, "cube" ) ) {   // generates a cube with %cubeW,D,H% variables
       } else
       if ( !str_cmp( arg, "grid" ) ) {   // generates a grid with %cubeW,D% variables
       } else
       if ( !str_cmp( arg, "maze" ) ) {   // generates a maze with %mazeW,D% variables
       } else
       if ( !str_cmp( arg, "3dmaze" ) ) { // generates a non-uniform cubemap with %3dmazeW,D,H% variables
       } else
       if ( !str_cmp( arg, "labyrinth" ) ) { // generates randomized cues in each room using available stuff
       } else
       if ( !str_cmp( arg, "cue" ) ) {    // generates a "simpl3" cue (not like cue command)
            if ( s ) { 
              one_argument(k,arg);
              while( arg[0] == '%' ) {
               V *v; SPAWN_DATA *c; bool fAct=FALSE; bool fCont=FALSE; bool fFurn=FALSE;
               k=one_argument(k,arg);
               v=get_variable(vars,arg);
               if ( v && v->value ) {
                switch ( v->type ) {
     case TYPE_ACTOR: fAct=TRUE; c=new_spawn_data(); c->command='M'; c->rs_vnum=((ACTOR_INDEX_DATA *)(v->value))->vnum; c->loc=1; break;
      case TYPE_PROP: c=new_spawn_data(); c->command='O'; c->rs_vnum=((PROP_INDEX_DATA *)(v->value))->vnum; 
      c->loc=fAct ? prop_where((PROP_INDEX_DATA *)(v->value)) : (fCont ? SPAWN_LOC_INSIDE : (fFurn ? SPAWN_LOC_ONTOP : SPAWN_LOC_INSCENE)); 
   switch( ((PROP_INDEX_DATA *)(v->value))->item_type ) { case ITEM_FURNITURE: fFurn=TRUE; break; case ITEM_CONTAINER: fCont=TRUE; break; default: break; }
                 break;
                   default: c=NULL; break;
                }
                if (c) { c->percent = 100; add_spawn( s, c, 1 ); }
               }
               one_argument(k,arg);
              }
            }          
       } else
       if ( !str_cmp( arg, "actor" ) ) {  // creates a new actor or switches focus

          one_argument( k, arg );
          if ( arg[0] == '%' ) { // then it's scene <variable>, so let's find it
            V *v;
            k=one_argument(k,arg);
            v=get_variable( vars, arg );
            if ( !v || !v->value ) {
              NOTIFY( "Build(): actor %var% is referencing invalid varname\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
            } else
            if ( v->type != TYPE_ACTOR ) {
              NOTIFY( "Build(): actor %var% is referencing the wrong type\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
            } else {
              a=(ACTOR_INDEX_DATA *)v->value;  active=TYPE_ACTOR;
            }
          } else { // it's just the word actor, followed by a different command, so create
           int iHash;  V *n;
           a = new_actor_index();              a->zone =w;    a->owner = str_dup( _o );
           a->vnum=tvnum[TYPE_ACTOR]++;        iHash = s->vnum % MAX_KEY_HASH; 
           a->next = actor_index_hash[iHash];  actor_index_hash[iHash]  = a;
           n = new_variable_data();
           snprintf( buf, MSL, "%%actor%d%%", ++created[TYPE_ACTOR] );  // make a new variable
           n->name = str_dup( buf );     n->type = TYPE_ACTOR;
           n->value = (void *) a;        n->next = vars; vars=n;
           active = TYPE_ACTOR;
          }

       } else
       if ( !str_cmp( arg, "prop" ) ) {   // creates a new prop or switches focus

          one_argument( k, arg );
          if ( arg[0] == '%' ) { // then it's scene <variable>, so let's find it
            V *v;
            k=one_argument(k,arg);
            v=get_variable( vars, arg );
            if ( !v || !v->value ) {
              NOTIFY( "Build(): prop %var% is referencing invalid varname\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
            } else
            if ( v->type != TYPE_PROP ) {
              NOTIFY( "Build(): prop %var% is referencing wrong type\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
            } else {
              p=(PROP_INDEX_DATA *)v->value;  active=TYPE_PROP;
            }
          } else { // it's just the word prop, followed by a different command, so create
           int iHash;  V *n;
           p = new_prop_index();               p->zone =w;   p->owner = str_dup( _o );
           p->vnum=tvnum[TYPE_PROP]++;        iHash = p->vnum % MAX_KEY_HASH; 
           p->next = prop_index_hash[iHash];   prop_index_hash[iHash]  = p;
           n = new_variable_data();
           snprintf( buf, MSL, "%%prop%d%%", ++created[TYPE_PROP] );  // make a new variable
           n->name = str_dup( buf );     n->type = TYPE_PROP;
           n->value = (void *) p;        n->next = vars; vars=n;
           active = TYPE_PROP;
          }

       } else
       if ( !str_cmp( arg, "name" ) ) {   // set the name of the active object
           k=grab_to_eol( k, arg );
           switch ( active ) {
                case TYPE_ACTOR: if ( a ) { if ( a->name ) free_string(a->name); a->name=str_dup(arg); }
                          break;
                 case TYPE_PROP: if ( p ) { if ( p->name ) free_string(p->name); p->name=str_dup(arg); }
                          break;
                case TYPE_SCENE: if ( s ) { if ( s->name ) free_string(s->name); s->name=str_dup(arg); }
                          break;
                 default: NOTIFY( "Build(): name called before object was made active\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
                       break;
            }
       } else
       if ( !str_cmp( arg, "short" ) ) {  // sets the short description of the active object
           k=grab_to_eol( k, arg );
           switch ( active ) {
                case TYPE_ACTOR: if ( a ) { if ( a->short_descr ) free_string(a->short_descr); a->short_descr=str_dup(arg); }
                          break;
                 case TYPE_PROP: if ( p ) { if ( p->short_descr ) free_string(p->short_descr); p->short_descr=str_dup(arg); }
                          break;
                 default: NOTIFY( "Build(): short called before object was made active, or on wrong type\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
                       break;
            }
       } else
       if ( !str_cmp( arg, "long" ) ) {   // sets the long description of the active object
           k=grab_to_eol( k, arg );
           switch ( active ) {
                case TYPE_ACTOR: if ( a ) { if ( a->long_descr ) free_string(a->long_descr); a->long_descr=str_dup(arg); }
                          break;
                 case TYPE_PROP: if ( p ) { if ( p->description ) free_string(p->description); p->description=str_dup(arg); }
                          break;
                 default: NOTIFY( "Build(): long called before object was made active, or on wrong type\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
                      break;
            }
       } else
       if ( !str_cmp( arg, "description" ) || !str_cmp( arg, "desc" ) ) {  // sets the description of the active object
           k=grab_to_at(k,arg);
           switch ( active ) {
             case TYPE_ACTOR: if ( a ) { if ( a->description ) free_string( a->description ); a->description = format_string( arg ); } break;
             case TYPE_SCENE: if ( s ) { if ( s->description ) free_string( s->description ); s->description = format_string( arg ); } break;
              case TYPE_PROP: if ( p ) { if ( p->real_description ) free_string( p->real_description ); p->real_description = format_string( arg ); } break;
                 default: NOTIFY( "Build(): description called on wrong type or no active object\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
                      break;
           }
       } else
       if ( !str_cmp( arg, "plural" ) ) {  // sets the description of the active object
           k=grab_to_eol(k,arg);
           switch ( active ) {
              case TYPE_PROP: if ( p ) { if ( p->short_descr_plural ) free_string( p->short_descr_plural ); p->short_descr_plural = str_dup( arg ); } break;
                 default: NOTIFY( "Build(): plural called on wrong type or no active object\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
                      break;
           }
       } else
       if ( !str_cmp( arg, "pluraldescription" ) || !str_cmp( arg, "pluraldesc" ) ) {  // sets the description of the active object
           k=grab_to_at(k,arg);
           switch ( active ) {
             case TYPE_PROP: if ( p ) { if ( p->description_plural ) free_string( p->description_plural ); p->description_plural = format_string( arg ); } break;
                 default: NOTIFY( "Build(): pluraldesc called on wrong type or no active object\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
                      break;
           }
       } else
       if ( !str_cmp( arg, "ed" ) ) {     // adds an extra-description to the active object 
          EXTRA_DESCR_DATA *ed;
          k=grab_to_eol( k, arg );
          ed = new_extra_descr();
          ed->keyword = str_dup( arg );
          k=grab_to_at( k, arg );
          ed->description = format_string( arg );
          switch ( active ) {
              case TYPE_SCENE: if ( s ) { ed->next = s->extra_descr; s->extra_descr = ed; } break;
               case TYPE_PROP: if ( a ) { ed->next = p->extra_descr; p->extra_descr = ed; } break;
                 default: NOTIFY( "Build(): ed <keyword> called before object was made active, or on wrong type\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
                          free_string( ed->keyword ); free_string( ed->description ); free_extra_descr( ed ); 
                   break;
          }
       } else
       if ( !str_cmp( arg, "ref" ) ) {    // creates a template reference
         k=one_argument(k,arg);
         if ( active == TYPE_SCENE ) {
           if ( !is_number(arg) ) {
             V *v=get_variable(vars,arg);
             if ( v && v->type == TYPE_SCENE && v->value ) {
                 SCENE_INDEX_DATA *x=(SCENE_INDEX_DATA *)(v->value);
                 if ( IS_SET(x->scene_flags,SCENE_TEMPLATE) ) s->template=x->vnum;
             }
           } else {
             SCENE_INDEX_DATA *x=get_scene_index(atoi(arg));
             if ( x && IS_SET(x->scene_flags,SCENE_TEMPLATE) ) s->template=x->vnum;
           }
         } else {
           NOTIFY( "Build(): ref called on wrong type, or no active object", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
         }
       } else
       if ( !str_cmp( arg, "action" ) ) { // changes the action description on the active object
           k=grab_to_at(k,arg);
           switch ( active ) {
              case TYPE_PROP: if ( p ) { if ( p->action_descr ) free_string( p->action_descr ); p->action_descr = format_string( arg ); } break;
                 default: NOTIFY( "Build(): action called on wrong type or no active object\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
                      break;
           }
       } else
       if ( !str_cmp( arg, "flags" ) || !str_cmp( arg, "flag" ) ) {  // toggles flags smartly
          int value;  char *i;
          k=grab_to_eol(k,arg);
          i = one_argument( arg, buf );
          if ( active == TYPE_PROP && !p ) NOTIFY( "Build(): flags <list> with no active prop\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
          else
          if ( active == TYPE_SCENE && !s ) NOTIFY( "Build(): flags <list> with no active scene\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
          else
          if ( active == TYPE_ACTOR && !a ) NOTIFY( "Build(): flags <list> with no active actor\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
          else
          while ( buf[0] != '\0' ) {
          switch ( active ) {
            case TYPE_PROP: 
             if ( ( value = extra_name_bit(buf) ) != EXTRA_NONE ) TOGGLE_BIT(p->extra_flags, value);
             else
             if ( ( value = wear_name_bit(buf) ) != ITEM_WEAR_NONE ) TOGGLE_BIT(p->wear_flags, value);
             else { NOTIFY( "Build(): prop flag: ", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT ); NOTIFY( buf, LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
                    NOTIFY( " Used on wrong type or it's just not a flag\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT ); }
            break;
            case TYPE_ACTOR: 
             if ( ( value = act_name_bit(buf) ) != ACT_NONE ) { TOGGLE_BIT(a->act, value);  SET_BIT( a->act, ACT_IS_NPC ); }
             else
             if ( ( value = bonus_name_bit(buf) ) != AFFECT_NONE )  TOGGLE_BIT(a->bonuses, value);
             else { NOTIFY( "Build(): actor flag: ", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT ); NOTIFY( buf, LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
                    NOTIFY( " Used on wrong type or it's just not a flag\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT ); }
            break;
            case TYPE_SCENE: 
             if ( scene_name_bit(buf) != SCENE_NONE ) TOGGLE_BIT(s->scene_flags, scene_name_bit(buf));
             else
             if ( sector_number(buf) != SECT_MAX ) s->sector_type  = sector_number(buf);
             else { NOTIFY( "Build(): scene flag: ", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT ); NOTIFY( buf, LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
                    NOTIFY( " Used on wrong type or it's just not a flag\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT ); }
            break;
          default: NOTIFY( "Build(): flag called before an object was made active\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
            break;
          }
          i=one_argument(i,buf);
          }
       } else
       if ( !str_cmp( arg, "values" ) || !str_cmp( arg, "value" ) ) {  // sets the values on the active prop
            char *i;
            if ( active == TYPE_PROP && p ) {
             k=grab_to_eol(k,arg);
             i = one_argument( arg, buf ); if ( buf[0] != '\0' ) { if ( is_number(buf) ) p->value[0] = atoi(buf); else TOGGLE_BIT(p->value[0],item_valflag(buf)); }
             i = one_argument( i, buf ); if ( buf[0] != '\0' ) { if ( is_number(buf) ) p->value[1] = atoi(buf); else TOGGLE_BIT(p->value[1],item_valflag(buf)); }
             i = one_argument( i, buf ); if ( buf[0] != '\0' ) { if ( is_number(buf) ) p->value[2] = atoi(buf); else TOGGLE_BIT(p->value[2],item_valflag(buf)); }
             i = one_argument( i, buf ); if ( buf[0] != '\0' ) { if ( is_number(buf) ) p->value[3] = atoi(buf); else TOGGLE_BIT(p->value[1],item_valflag(buf)); }
            } else {
              NOTIFY( "Build(): values <v1,2,3,4> called on wrong type or no prop selected\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
            }
       } else
       if ( !str_cmp( arg, "path" ) ) {  // sets the attributes of the active actor
           k=grab_to_eol(k,arg);
           snprintf( path_title, MSL, "%s", arg );
           k=grab_to_at(k,arg);
           snprintf( path_descr, MSL, "%s", arg );
       } else
       if ( !str_cmp( arg, "attrib" ) ) {  // sets the attributes of the active actor
       } else
       if ( !str_cmp( arg, "money" ) || !str_cmp( arg, "cost" ) ) {  // sets cost or value on actor/prop
          k=one_argument(k,arg);
          if ( active == TYPE_ACTOR ) {
             if ( a ) a->money = atoi(arg);
          } else
          if ( active == TYPE_PROP ) {
             if ( p ) p->cost = atoi(arg);
          } else 
          NOTIFY( "Build(): cast/money called but scene was active, or no object active", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
       } else
       if ( !str_cmp( arg, "karma" ) ) {  // sets an actor's karma
          k=one_argument(k,arg);
          if ( active == TYPE_ACTOR ) {
             if ( a ) a->karma = atoi(arg);
          } else
          NOTIFY( "Build(): cast/money called on wrong type, or no object active", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
       } else
       if ( !str_cmp( arg, "experience" ) || !str_cmp( arg, "xp" ) || !str_cmp( arg, "exp" ) ) {  // sets actor's xp
          k=one_argument(k,arg);
          if ( active == TYPE_ACTOR ) {
             if ( a ) a->exp = atoi(arg);
          } else
          NOTIFY( "Build(): cast/money called on wrong type, or no object active", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
       } else
       if ( !str_cmp( arg, "script" ) ) {  // adds a script to the active object
        SCRIPT_DATA *script;
        k=one_argument(k,arg);
        if ( (script = get_script_index(atoi(arg))) == NULL ) NOTIFY( "Build(): script <vnum>; bad vnum\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
        else {
         INSTANCE_DATA *pTrig;
         pTrig = new_instance();
         pTrig->script    = script;
         switch ( active ) {
             case TYPE_ACTOR: pTrig->next = a->instances; a->instances = pTrig; break;
             case TYPE_SCENE: pTrig->next = s->instances; s->instances = pTrig; break;
              case TYPE_PROP: pTrig->next = p->instances; p->instances = pTrig; break;
                 default: free_instance( pTrig ); 
     NOTIFY( "Build(): script <vnum>; called before active object\n\r", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
                   break;
         }
        }
       } else
       if ( !str_cmp( arg, "e"  ) || !str_cmp( arg, "east" )    // sets exit info for exits
         || !str_cmp( arg, "w"  ) || !str_cmp( arg, "west" )    
         || !str_cmp( arg, "n"  ) || !str_cmp( arg, "north" ) 
         || !str_cmp( arg, "s"  ) || !str_cmp( arg, "south" ) 
         || !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast" ) 
         || !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest" ) 
         || !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast" ) 
         || !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest" ) 
         || !str_cmp( arg, "u"  ) || !str_cmp( arg, "up" ) 
         || !str_cmp( arg, "d"  ) || !str_cmp( arg, "down" ) ) {
          int val;
          int dir = get_dir( arg );
          k=one_argument(k,arg);
          if ( !str_cmp( arg, "to" ) ) k=one_argument(k,arg);
          if ( active != TYPE_SCENE || !s )
          NOTIFY( "Build(): exit call without a scene", LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
          else {
           if ( arg[0] == '%' ) {
            V *v=get_variable(vars,arg);
            if ( v && v->type == TYPE_SCENE && v->value ) {
               if ( !s->exit[dir] ) {
                s->exit[dir] = new_exit(); s->exit[dir]->to_scene = SCENE(v->value);
                if ( !s->exit[dir]->to_scene->exit[rev_dir[dir]] ) s->exit[dir]->to_scene->exit[rev_dir[dir]] = new_exit();
                s->exit[dir]->to_scene->exit[rev_dir[dir]]->to_scene = s;
               } 
            }             
           } else 
           if ( is_number(arg) ) {
            SCENE_INDEX_DATA *x=get_scene_index(atoi(arg));
            if ( x && !s->exit[dir] ) {
             s->exit[dir] = new_exit(); s->exit[dir]->to_scene = x;
             if ( !s->exit[dir]->to_scene->exit[rev_dir[dir]] ) s->exit[dir]->to_scene->exit[rev_dir[dir]] = new_exit();
             s->exit[dir]->to_scene->exit[rev_dir[dir]]->to_scene = s;
            }
           } else
           if ( !str_cmp( arg, "key" ) ) { k=one_argument(k,arg);
             if ( s->exit[dir] ) {
               if ( arg[0] == '%' ) {
                 V *v=get_variable(vars,arg);
                 if ( v && v->type == TYPE_PROP && v->value ) {
                  s->exit[dir]->key = ((PROP_INDEX_DATA *)(v->value))->vnum;
                 }
               } else { PROP_INDEX_DATA *x=get_prop_index(atoi(arg)); if (x) s->exit[dir]->key = x->vnum; }
             }
           } else
           if ( !str_cmp( arg, "door" ) ) {
             if ( s->exit[dir] ) { k=grab_to_eol(k,arg);
              SET_BIT( s->exit[dir]->rs_flags, EX_ISDOOR );
              SET_BIT( s->exit[dir]->exit_info, EX_ISDOOR ); 
              if ( s->exit[dir]->to_scene && s->exit[dir]->to_scene->exit[rev_dir[dir]] ) { 
               SET_BIT( s->exit[dir]->to_scene->exit[rev_dir[dir]]->rs_flags, EX_ISDOOR );
               SET_BIT( s->exit[dir]->to_scene->exit[rev_dir[dir]]->exit_info, EX_ISDOOR );
              }
             }
           } else
           if ( !str_cmp( arg, "name" ) ) { k=grab_to_eol(k,arg);
             if ( s->exit[dir] ) { free_string( s->exit[dir]->keyword ); s->exit[dir]->keyword = str_dup(arg); }
           } else
           if ( !str_cmp( arg, "desc" ) || !str_cmp(arg, "description") ) { k=grab_to_at(k,arg);
             if ( s->exit[dir] ) { free_string( s->exit[dir]->description ); s->exit[dir]->description = str_dup(arg); }
           } else
           if ( (val = exit_name_bit( arg )) != EX_NONE ) {
             if ( s->exit[dir] ) {
              SET_BIT(s->exit[dir]->rs_flags, val);
              SET_BIT(s->exit[dir]->exit_info, val);
              if ( s->exit[dir]->to_scene && s->exit[dir]->to_scene->exit[rev_dir[dir]] ) {
               SET_BIT(s->exit[dir]->to_scene->exit[rev_dir[dir]]->rs_flags, val);
               SET_BIT(s->exit[dir]->to_scene->exit[rev_dir[dir]]->exit_info, val);
              }
             }
           }
          }
       }
       else
       if ( !str_cmp( arg, "attach" ) ) {  // attaches a scene to the designated "loc"
        int dir; k=one_argument(k,arg); dir = get_dir(arg);
        if ( active == TYPE_SCENE && s ) {
         if ( loc->exit[rev_dir[dir]] != NULL ) { // find an alternative exit (first available), build path room if no available
           int dir2, f=-1; for( dir2=0; dir2 < MAX_DIR; dir2++ ) if ( s->exit[dir2] == NULL && loc->exit[rev_dir[dir2]] ) f=dir2; 
           if ( f >= 0 && f < MAX_DIR ) { 
             if ( !s->exit[f] ) s->exit[f]=new_exit(); s->exit[f]->to_scene=loc;
             if ( !loc->exit[rev_dir[f]] ) loc->exit[rev_dir[f]]=new_exit(); loc->exit[rev_dir[f]]->to_scene=s;
           } else { 
            // generate a path room because we can't make a direct connection
            int iHash;  V *n;  SCENE_INDEX_DATA *x;
            x = new_scene_index();              x->zone =w;  x->owner = str_dup( _o );
            x->vnum=tvnum[TYPE_SCENE]++;        iHash = x->vnum % MAX_KEY_HASH;
            x->next = scene_index_hash[iHash];  scene_index_hash[iHash]  = x;
            n = new_variable_data();
            snprintf( buf, MSL, "%%path%d%%", ++created[0] );  // make a new variable, we'll use counter[0] because its a path
            n->name = str_dup( buf );     n->type = TYPE_SCENE;
            n->value = (void *) x;        n->next = vars; vars=n;
            x->name = str_dup( path_title );
            x->description = format_string( path_descr );
            for ( dir2=0; dir2 < MAX_DIR; dir2++ ) if ( !s->exit[dir2] ) { s->exit[dir2]=new_exit(); s->exit[dir2]->to_scene=x; 
                     x->exit[rev_dir[dir2]]=new_exit(); x->exit[rev_dir[dir2]]->to_scene=s;  break; }
            for ( dir2=0; dir2 < MAX_DIR; dir2++ ) if ( !loc->exit[dir2] ) { loc->exit[dir2]=new_exit(); loc->exit[dir2]->to_scene=x; 
                     x->exit[rev_dir[dir2]]=new_exit(); x->exit[rev_dir[dir2]]->to_scene=loc;  break; }
           }
         } else {
           loc->exit[rev_dir[dir]] = new_exit(); loc->exit[rev_dir[dir]]->to_scene = s; 
           if ( !s->exit[dir] ) s->exit[dir]=new_exit(); s->exit[dir]->to_scene = loc;
         }
        }        
       } else {
         snprintf( buf, MSL, "Build(): bad command '%s', continuing\n\r", arg );
         NOTIFY( buf, LEVEL_IMMORTAL, WIZ_NOTIFY_SCRIPT );
       }
     }

   }

   // Dispose of our temporary variables after outputting to the NOTIFY/scripts

   // Force the DB to save (kooky)
   cmd_zsave( NULL, "" );

   RETURNS(TYPE_STRING,str_dup("1"));   // Success?
}



/* Even after I'm dead, I want you to keep playing this and think of me.  - Locke */

