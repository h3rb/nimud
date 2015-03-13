/*
******************************************************************************
* Locke's   __ -based on merc v2.2-____        NIM Server Software           *
* ___ ___  (__)__    __ __   __ ___| G| v4.0   Version 4.0 GOLD EDITION      *
* |  /   \  __|  \__/  |  | |  |     O|        documentation release         *
* |       ||  |        |  \_|  | ()  L|        Hallow's Eve 1999             *
* |    |  ||  |  |__|  |       |     D|                                      *
* |____|__|___|__|  |__|\___/__|______|        http://www.nimud.org/nimud    *
*   n a m e l e s s i n c a r n a t e          dedicated to chris cool       *
******************************************************************************
 */


/*
 * Macros for declaring string and integer modifiers.
 */
#define dSET_FUNCTIONI(fname)\
VARD * fname args ( ( void * owner, int type, VARD *v ) );

#define dGET_FUNCTIONI(fname)\
VARD * fname args ( ( void * owner, int type ) );

#define dSET_FUNCTIONI(fname)\
VARD * fname args ( ( void * owner, int type, VARD *v ) );

#define dGET_FUNCTIONI(fname)\
VARD * fname args ( ( void * owner, int type ) );

#define dMATH_FUNCTIONI(fname)\
VARD * fname args ( ( void * owner, int type, VARD *astr, VARD *bstr, VARD *cstr ) );

#define dGRAPHICS_FUNCTION(fname)\
VARD * fname args ( ( void * owner, int type, VARD *va, VARD *vb, VARD *vc, VARD *vd, VARD *ve, VARD *vf ) )

#define dGET_FUNCTION(fname)\
VARD * fname args ( ( void * owner, int type, VARD *t, VARD *v ) );
#define dSET_FUNCTION(fname)\
VARD * fname args ( ( void * owner, int type, VARD *t, VARD *v ) );


/*
 * Variable modifier functions.
 */
dGET_FUNCTIONI(func_month);
dSET_FUNCTIONI(func_setmonth);

dGET_FUNCTIONI(func_day);
dSET_FUNCTIONI(func_setday);

dGET_FUNCTIONI(func_dayofweek);

dGET_FUNCTIONI(func_time);
dSET_FUNCTIONI(func_settime);

dGET_FUNCTIONI(func_year);
dSET_FUNCTIONI(func_setyear);

dGET_FUNCTIONI(func_sky);
dSET_FUNCTIONI(func_setsky);

dGET_FUNCTIONI(func_moon);
dSET_FUNCTIONI(func_setmoon);

dGET_FUNCTIONI(func_temperature);
dSET_FUNCTIONI(func_settemperature);

dGET_FUNCTIONI(func_phase);
dSET_FUNCTIONI(func_setphase);

VARD *func_check  args( ( void * owner, int type, VARD *n ) );

/*
 * MSP support.
 */

VARD *func_sound  args( ( void * owner, int type, VARD *n, VARD *t, VARD *v, VARD * p, VARD *l, VARD *b ) );
VARD *func_music  args( ( void * owner, int type, VARD *n, VARD *t, VARD *v, VARD * p, VARD *l, VARD *b ) );

/*
 * Reserved functions for internal parsing.
 */
VARD * func_goto     args( ( void * owner, int type, VARD *label ) );
VARD * func_ifgoto   args( ( void * owner, int type, VARD *a, VARD *yes, VARD *no ) );
VARD * func_if       args( ( void * owner, int type, VARD *exp, VARD *iftrue, VARD *iffalse ) );
VARD * func_label    args( ( void * owner, int type ) );
VARD * func_return   args( ( void * owner, int type, VARD *value ) );
VARD * func_halt     args( ( void * owner, int type, VARD *value ) );
VARD * func_permhalt args( ( void * owner, int type ) );
VARD * func_wait     args( ( void * owner, int type, VARD *value ) );
VARD * func_autowait args( ( void * owner, int type, VARD *value ) );
VARD * func_call     args( ( void * owner, int type, VARD *trigname, VARD *target ) );
VARD * func_install  args( ( void * owner, int type, VARD *target, VARD *script ) );
VARD * func_uninstall args( ( void * owner, int type, VARD *target, VARD *script ) );

/*
 * Further stuff.
 */
VARD * func_vcpy     args( ( void * owner, int type, VARD *old, VARD *new, VARD *trigname ) );
VARD * func_self     args( ( void * owner, int type ) );
VARIABLE_DATA *func_peace  args( ( void * owner, int type, VARIABLE_DATA *dest ) );
VARIABLE_DATA * func_interpret  args( ( void * owner, int type,
                         VARIABLE_DATA *who,
                         VARIABLE_DATA *exp1,
                         VARIABLE_DATA *exp2,
                         VARIABLE_DATA *exp3,
                         VARIABLE_DATA *exp4 ) );
VARIABLE_DATA *func_bonus       args( ( void * owner, int type, VARIABLE_DATA *target,
                          VARIABLE_DATA *location,
                          VARIABLE_DATA *bitvector,
                          VARIABLE_DATA *modifier,
                          VARIABLE_DATA *duration,
                          VARIABLE_DATA *aftype ) );

/*
 * "Constants."
 */

VARD * func_null     args( ( void * owner, int type ) );

/*
 * Boolean conditionals.
 */
VARD * func_cmp      args( ( void * owner, int type, VARD *astr, VARD *bstr ) );
VARD * func_pre      args( ( void * owner, int type, VARD *astr, VARD *bstr ) );
VARD * func_in       args( ( void * owner, int type, VARD *astr, VARD *bstr ) );
VARD * func_strstr   args( ( void * owner, int type, VARD *astr, VARD *bstr ) );
VARD * func_pcmp     args( ( void * owner, int type, VARD *astr, VARD *bstr ) );

/*
 * Mathematics.
 */
dMATH_FUNCTIONI(func_not);
dMATH_FUNCTIONI(func_or);
dMATH_FUNCTIONI(func_and);
dMATH_FUNCTIONI(func_less);
dMATH_FUNCTIONI(func_lte);
dMATH_FUNCTIONI(func_greater);
dMATH_FUNCTIONI(func_gte);
dMATH_FUNCTIONI(func_bor);
dMATH_FUNCTIONI(func_xor);
dMATH_FUNCTIONI(func_bxor);
dMATH_FUNCTIONI(func_band);
dMATH_FUNCTIONI(func_add);
dMATH_FUNCTIONI(func_mult);
dMATH_FUNCTIONI(func_sub);
dMATH_FUNCTIONI(func_mod);
dMATH_FUNCTIONI(func_div);
dMATH_FUNCTIONI(func_inc);
dMATH_FUNCTIONI(func_dec);
dMATH_FUNCTIONI(func_eq);

dMATH_FUNCTIONI(func_random);
dMATH_FUNCTIONI(func_gelify);
dMATH_FUNCTIONI(func_percent);
dMATH_FUNCTIONI(func_bits);
dMATH_FUNCTIONI(func_fuzzy);
dMATH_FUNCTIONI(func_gelify);
dMATH_FUNCTIONI(func_dice);

dMATH_FUNCTIONI(func_interpolate);
dMATH_FUNCTIONI(func_INTERPOLATE);
dMATH_FUNCTIONI(func_min);
dMATH_FUNCTIONI(func_max);
dMATH_FUNCTIONI(func_urange);
dMATH_FUNCTIONI(func_range);
dMATH_FUNCTIONI(func_lineate);
dMATH_FUNCTIONI(func_ratio);

dGRAPHICS_FUNCTION(func_box);
dGRAPHICS_FUNCTION(func_rect);
dGRAPHICS_FUNCTION(func_line);
dGRAPHICS_FUNCTION(func_circle);
dGRAPHICS_FUNCTION(func_color);
dGRAPHICS_FUNCTION(func_stroke);
dGRAPHICS_FUNCTION(func_point);
dGRAPHICS_FUNCTION(func_draw);
dGRAPHICS_FUNCTION(func_clear);
dGRAPHICS_FUNCTION(func_pageclear);
dGRAPHICS_FUNCTION(func_text);
dGRAPHICS_FUNCTION(func_mode);
dGRAPHICS_FUNCTION(func_gmode);
dGRAPHICS_FUNCTION(func_style);
dGRAPHICS_FUNCTION(func_brush);
dGRAPHICS_FUNCTION(func_hline);
dGRAPHICS_FUNCTION(func_vline);
dGRAPHICS_FUNCTION(func_tline);

dGRAPHICS_FUNCTION(func_popup);
dGRAPHICS_FUNCTION(func_button);
dGRAPHICS_FUNCTION(func_wrap);
dGRAPHICS_FUNCTION(func_fill);
dGRAPHICS_FUNCTION(func_fills);
dGRAPHICS_FUNCTION(func_column);
dGRAPHICS_FUNCTION(func_color);
dGRAPHICS_FUNCTION(func_ruler);
dGRAPHICS_FUNCTION(func_rulers);

/*
 * Actor-based functions.
 */

dSET_FUNCTION(func_setactorname);
dGET_FUNCTION(func_actorname);
dSET_FUNCTION(func_setdescription);
dGET_FUNCTION(func_description);
dSET_FUNCTION(func_setshort);
dGET_FUNCTION(func_short);
dSET_FUNCTION(func_setlong);
dGET_FUNCTION(func_long);

dSET_FUNCTION(func_setposition);
dGET_FUNCTION(func_position);
dSET_FUNCTION(func_setkarma);
dGET_FUNCTION(func_karma);

dSET_FUNCTION(func_setmodstr);
dGET_FUNCTION(func_modstr);
dSET_FUNCTION(func_setmodint);
dGET_FUNCTION(func_modint);
dSET_FUNCTION(func_setmoddex);
dGET_FUNCTION(func_moddex);
dSET_FUNCTION(func_setmodcon);
dGET_FUNCTION(func_modcon);
dSET_FUNCTION(func_setmodwis);
dGET_FUNCTION(func_modwis);

dSET_FUNCTION(func_setarmor);
dGET_FUNCTION(func_armor);

dSET_FUNCTION(func_sethitroll);
dGET_FUNCTION(func_hitroll);
dSET_FUNCTION(func_setdamroll);
dGET_FUNCTION(func_damroll);

dSET_FUNCTION(func_setexp);
dGET_FUNCTION(func_exp);

/*
 * Prop-based functions.
 */
dSET_FUNCTION(func_setpropname);
dGET_FUNCTION(func_propname);
dSET_FUNCTION(func_setpropdesc);
dGET_FUNCTION(func_propdesc);
dSET_FUNCTION(func_setpropshort);
dGET_FUNCTION(func_propshort);
dSET_FUNCTION(func_setproplong);
dGET_FUNCTION(func_proplong);
dSET_FUNCTION(func_setpropaction);
dGET_FUNCTION(func_propaction);

dSET_FUNCTION(func_setv1);  
dGET_FUNCTION(func_v1);
dSET_FUNCTION(func_setv2);
dGET_FUNCTION(func_v2);
dSET_FUNCTION(func_setv3);
dGET_FUNCTION(func_v3);
dSET_FUNCTION(func_setv4);
dGET_FUNCTION(func_v4);   

dSET_FUNCTION(func_setitemtype);
dGET_FUNCTION(func_itemtype);
dSET_FUNCTION(func_setextraflags);
dGET_FUNCTION(func_extraflags);
dSET_FUNCTION(func_setwearloc);
dGET_FUNCTION(func_wearloc);
dSET_FUNCTION(func_setweight);
dGET_FUNCTION(func_weight);
dSET_FUNCTION(func_setcost);
dGET_FUNCTION(func_cost);
dSET_FUNCTION(func_settimer);
dGET_FUNCTION(func_timer);
dSET_FUNCTION(func_setsize);
dGET_FUNCTION(func_size);

/* 
 * Scene-based functions.
 */

dSET_FUNCTION(func_setscenetitle);
dGET_FUNCTION(func_scenetitle);
dSET_FUNCTION(func_setscenedesc);
dGET_FUNCTION(func_scenedesc);

dSET_FUNCTION(func_setscenepos);
dGET_FUNCTION(func_scenepos);
dSET_FUNCTION(func_settemplate);
dGET_FUNCTION(func_template);
dSET_FUNCTION(func_setmaxpeople);
dGET_FUNCTION(func_maxpeople);
dSET_FUNCTION(func_setlight);
dGET_FUNCTION(func_light);
dSET_FUNCTION(func_setsector);
dGET_FUNCTION(func_sector);  
dSET_FUNCTION(func_setterrain);
dGET_FUNCTION(func_terrain);
dGET_FUNCTION(func_setwagon);
dGET_FUNCTION(func_wagon);


/*
 * String things.
 */
VARD * func_cat      args( ( void * owner, int type, VARD *astr, VARD *bstr ) );
VARD * func_word     args( ( void * owner, int type, VARD *astr, VARD *value ) );

/*
 * Output functions.
 */
VARD * func_echo     args( ( void * owner, int type, VARD *target, VARD *out ) );
VARD * func_oecho    args( ( void * owner, int type, VARD *target, VARD *out ) );
VARD * func_dream    args( ( void * owner, int type, VARD *out ) );
VARD * func_history  args( ( void * owner, int type, VARD *target, VARD *out ) );

VARD * func_numw     args( ( void * owner, int type, VARD *astr ) );
VARD * func_strp     args( ( void * owner, int type, VARD *value, VARD *old, VARD *new ) );

/*
 * Game-related functions for actors.
 */
VARD * func_do         args( ( void * owner, int type, VARD *exp0, VARD *exp1, VARD *exp2, VARD *exp3, VARD *exp4, VARD *exp5 ) );
VARD * func_as         args( ( void * owner, int type, VARD *target, VARD *exp0, VARD *exp1, VARD *exp2, VARD *exp3, VARD *exp4 ) );
VARD * func_step       args( ( void * owner, int type, VARD *value ) );
VARD * func_aset       args( ( void * owner, int type, VARD *astr, VARD *bstr ) );
VARD * func_jump       args( ( void * owner, int type, VARD *location ) );
VARD * func_samescene  args( ( void * owner, int type, VARD *value ) );
VARD * func_foe        args( ( void * owner, int type ) );
VARD * func_following  args( ( void * owner, int type ) );
VARD * func_has        args( ( void * owner, int type, VARD *target, VARD *propvnum ) );
VARD * func_holds      args( ( void * owner, int type, VARD *target, VARD *propvnum ) );

/*
 * Game-related functions for props.
 */
VARD *func_pset        args( ( void * owner, int type, VARD *astr, VARD *bstr ) );

/*
 * Game-related functions for scenes.
 */
VARD *func_open      args( ( void * owner, int type, VARD *loc, VARD *dir ) );
VARD *func_close     args( ( void * owner, int type, VARD *loc, VARD *dir ) );

/*
 * General use functions.
 */

VARD *func_here      args( ( void * owner, int type ) );
VARD *func_purge     args( ( void * owner, int type, VARD *target ) );
VARD *func_force     args( ( void * owner, int type, VARD *target ) );
VARD *func_disarm    args( ( void * owner, int type, VARD *target ) );
VARD *func_strip     args( ( void * owner, int type, VARD *target ) );
VARD *func_pos       args( ( void * owner, int type, VARD *target, VARD *gain ) );
VARD *func_pay       args( ( void * owner, int type, VARD *target, VARD *gain ) );
VARD *func_charge    args( ( void * owner, int type, VARD *target, VARD *loss ) );
VARD *func_elude     args( ( void * owner, int type ) );
VARD *func_cue       args( ( void * owner, int type, VARD *target ) );
VARD *func_move      args( ( void * owner, int type, VARD *target, VARD *dest) );
VARD *func_moveall   args( ( void * owner, int type, VARD *from, VARD *dest, VARD *cmd ) );

VARD *func_home      args( ( void * owner, int type, VARD *target, VARD *dest) );
VARD *func_death     args( ( void * owner, int type, VARD *target, VARD *dest) );

VARD *func_heal      args( ( void * owner, int type, VARD *target, VARD *gain) );
VARD *func_healall   args( ( void * owner, int type, VARD *target, VARD *gain) );
VARD *func_hurt      args( ( void * owner, int type, VARD *target, VARD *gain) );
VARD *func_bomb      args( ( void * owner, int type, VARD *target, VARD *gain) );

VARD *func_dig       args ( ( void * owner, int type, VARD *loc, VARD *dir, VARD *dest ) );
VARD *func_undig     args ( ( void * owner, int type, VARD *loc, VARD *dir ) );
VARD *func_scenetitle args ( ( void * owner, int type, VARD *loc, VARD *dir ) );

VARD *func_dispense  args( ( void * owner, int type, VARD *target, VARD *disp) );
VARD *func_equip     args( ( void * owner, int type, VARD *target, VARD *disp, VARD *loc) );
VARD *func_inside    args( ( void * owner, int type, VARD *target, VARD *disp) );

VARD *func_create    args( ( void * owner, int type, VARD *vnum, VARD *loc) );
VARD *func_breed     args( ( void * owner, int type, VARD *vnum, VARD *master) );
VARD *func_eat       args( ( void * owner, int type, VARD *target )  );
VARD *func_eval      args( ( void * owner, int type, VARD *value ) );
VARD *func_vnum      args( ( void * owner, int type, VARD *target ) );

VARD *func_rndplr    args( ( void * owner, int type, VARD *from ) );
VARD *func_rnddir    args( ( void * owner, int type ) );
VARD *func_getdir    args( ( void * owner, int type, VARD *loc, VARD *dir ) );

VARD *func_players   args( ( void * owner, int type ) );
VARD *func_addbounty args( ( void * owner, int type, VARD *target, VARD *gain) );
VARD *func_bounty    args( ( void * owner, int type, VARD *target ) );
VARD *func_addowed   args( ( void * owner, int type, VARD *target, VARD *gain) );
VARD *func_owed      args( ( void * owner, int type, VARD *target ) );
VARD *func_level     args( ( void * owner, int type, VARD *target ) );

/*
 * Functions for creating and manipulating objects.
 */

VARD *func_transform  args( ( void * owner, int type, VARD *new ) );
VARD *func_broadcast args( ( void * owner, int type, VARD *out ) );

/*
 * Time, date and weather functions. (world-state functions)
 */

VARD *func_skill     args( ( void * owner, int type, VARD *target, VARD *sn, VARD *v ) );
VARD *func_setskill  args( ( void * owner, int type, VARD *target, VARD *sn, VARD *v ) );
VARD *func_alert     args( ( void * owner, int type, VARD *name, VARD *limits ) );

VARD *func_name     args( ( void * owner, int type, VARD *var ) );

/* 
 * Magic. (Spell-related)
 */
VARD *func_mana      args( ( void * owner, int type, VARD *target, VARD *gain) );
VARD *func_gem       args( ( void * owner, int type, VARD *mana_type, VARD *quantity ) );
VARD *func_mix       args( ( void * owner, int type, VARD *list, VARD *quantity ) );
VARD *func_reagents  args( ( void * owner, int type, VARD *list, VARD *quantity ) );

/*
 * List and stack functions.
 * Imported from COO2 and subject to a seperate license.
 * See: http://www.nationwideinteractive.com/coocoo
 */

VARD *func_push     args ( ( void * owner, int type, VARD *stack, VARD *value ) );
VARD *func_pop      args ( ( void * owner, int type, VARD *stack ) );
VARD *func_lrem     args ( ( void * owner, int type, VARD *stack, VARD *value ) );
VARD *func_sort     args ( ( void * owner, int type, VARD *stack, VARD *mask ) );
VARD *func_lrnd     args ( ( void * owner, int type, VARD *stack ) ); 
VARD *func_lshift   args ( ( void * owner, int type, VARD *stack ) );
VARD *func_rshift   args ( ( void * owner, int type, VARD *stack ) );
VARD *func_empty    args ( ( void * owner, int type, VARD *stack ) );
VARD *func_users    args ( ( void * owner, int type, VARD *mask ) );

/*
VARD *func_queue    args ( ( void * owner, int type, VARD *mask ) );
VARD *func_props  args ( ( void * owner, int type, VARD *mask ) );
VARD *func_commands args ( ( void * owner, int type, VARD *mask ) );
 */

/*
 * These don't work.
 */
VARD *func_foreach  args ( ( void * owner, int type, VARD *stack, VARD *code ) );
VARD *func_each     args ( ( void * owner, int type, VARD *stack )  );


/*
 * Build function for trigger builder scripts.
 */

VARD *func_build       args ( ( void *owner, int type, VARD *svnum, VARD *location, VARD *target, VARD *p1, VARD *p2, VARD *p3 ) );


#undef dSET_FUNCTIONI
#undef dGET_FUNCTIONI
#undef dSET_FUNCTIONS
#undef dGET_FUNCTIONS
#undef dMATH_FUNCTIONI
#undef dGRAPHICS_FUNCTION

/*
 * For use in the interpreter's big case statement.
 */

#define SET_FUNCTIONI(fname,fnames)\
FUNC(fnames,     fname    (owner, type, ppoint[0]) )

#define SET_FUNCTIONS(fname,fnames)\
FUNC(fnames,     fname    (owner, type, ppoint[0]) )

#define GET_FUNCTIONI(fname,fnames)\
FUNC(fnames,     fname    (owner, type) )

#define GET_FUNCTIONS(fname,fnames)\
FUNC(fnames,     fname    (owner, type) )

#define MATH_FUNCTIONI(fname,fnames)\
FUNC(fnames,     fname    (owner, type, ppoint[0], ppoint[1], ppoint[2]) ) 

#define GRAPHICS_FUNCTION(fname,fnames)\
FUNC(fnames,     fname    (owner, type, ppoint[0], ppoint[1], ppoint[2],\
                                        ppoint[3], ppoint[4], ppoint[5]) )

/*
 * Actor-structure functions.
 */
#define SET_FUNCTIONAS(fname,fnames)\
FUNC(fnames,     fname    (owner, type, ppoint[0], ppoint[1]) )
#define GET_FUNCTIONAS(fname,fnames)\
FUNC(fnames,     fname    (owner, type, ppoint[0], ppoint[1]) )

#define SET_FUNCTIONAI(fname,fnames)\
FUNC(fnames,     fname    (owner, type, ppoint[0], ppoint[1]) )
#define GET_FUNCTIONAI(fname,fnames)\
FUNC(fnames,     fname    (owner, type, ppoint[0], ppoint[1]) )


/*
 * Prop-structure functions.
 */
#define SET_FUNCTIONPS(fname,fnames)\
FUNC(fnames,     fname    (owner, type, ppoint[0], ppoint[1]) )
#define GET_FUNCTIONPS(fname,fnames)\
FUNC(fnames,     fname    (owner, type, ppoint[0], ppoint[1]) )

#define SET_FUNCTIONPI(fname,fnames)\
FUNC(fnames,     fname    (owner, type, ppoint[0], ppoint[1]) )
#define GET_FUNCTIONPI(fname,fnames)\
FUNC(fnames,     fname    (owner, type, ppoint[0], ppoint[1]) )

/*
 * Scene-structure functions.
 */
#define SET_FUNCTIONSS(fname,fnames)\
FUNC(fnames,     fname    (owner, type, ppoint[0], ppoint[1]) )
#define GET_FUNCTIONSS(fname,fnames)\
FUNC(fnames,     fname    (owner, type, ppoint[0], ppoint[1]) )

#define SET_FUNCTIONSI(fname,fnames)\
FUNC(fnames,     fname    (owner, type, ppoint[0], ppoint[1]) )
#define GET_FUNCTIONSI(fname,fnames)\
FUNC(fnames,     fname    (owner, type, ppoint[0], ppoint[1]) )

