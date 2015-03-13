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
 * Trigger types for repetitive loops, command etc.
 */
#define TRIG_NEVER         99  /* To turn a script off                       */
#define TRIG_COMMAND        0  /* All       = When a Command is Typed        */
#define TRIG_EACH_PULSE     1  /* Mobs/Objs = Triggered Each Pulse           */
#define TRIG_COMBAT         2  /* All       = Each Round                     */
#define TRIG_TICK_PULSE     3  /* Mobs/Objs = Tick       Scene = When Spawn  */
#define TRIG_BORN           4  /* Mobs/Objs = Creation   Scene = On Startup  */
#define TRIG_GETS           5  /* Objs      = When Taken Mobs = when given   */
#define TRIG_SAY            6  /* All       = Any communications (NFI)       */

#define TRIG_KILLS          8  /* Mobs/Objs = Kills                          */
#define TRIG_DIES           9  /* Mobs      = Dies  Objs/Scene = Dies Nearby */
#define TRIG_ENTER         10  /* All       = Someone Enters                 */
#define TRIG_MOVES         11  /* Mobs/Scene = Someone Leaves                */

#define TRIG_LOOKS	   12  /* All	    = Called when looked at.	     */

#define TRIG_EXIT          13  /* Exits     = Called with parameter on exits */

#define TRIG_ALERT         14  /* All       = Called by an alert() function  */
#define TRIG_SPELL         15  /* Spell     = Called by a spell caster       */
#define TRIG_BUILDER       16  /* Builder   = Called by build() function     */

/*
 * Internally used bits for triggers.
 */
#define SCRIPT_HALT         0x00000001   /* current=NULL & disallow trig     */
#define SCRIPT_ONCE         0x00000002   /* terminates and disposes after 1  */

#define MAX_LOOPS           1000   /* Max number of total goto()s per script */


#define TYPE_PROP           1
#define TYPE_ACTOR          2
#define TYPE_SCENE          3
#define TYPE_STRING         0

#define MAX_PARAMS          6

#define FUNC(str, function_call)                \
 if ( !str_cmp(name, str) ) value = function_call

/*
 * Internal stuff.
 */

#define PARSING(o, t)   ( t==TYPE_PROP ? ((PROP *)o)->current :      \
                          (t==TYPE_ACTOR ? ((PLAYER *)o)->current :    \
                           (t==TYPE_SCENE ? ((SCENE *)o)->current : NULL )) )


VARIABLE **  globals   args( ( void * owner, int type ) );

void trigger_list      args( ( PROP *list, int ttype, PLAYER  *actor,
                               PROP *catalyst, char *astr, char *bstr ) );
void parse_script      args( ( INSTANCE *instance, void * owner, int type ) );
void parse_assign      args( ( char *line, void * owner, int type ) );


/*
 * Functions are written in a certain format.  The following macros
 * assist in writing the functions by minimizing the code needed
 * to translate a function's parameters.
 *
 * There are some quirks with translate_variables: when translate_variables
 * is called, it returns a pointer to a static char buf[MSL] which is
 * changed when it is called again.  If you are parsing more than 1
 * variable, then there is a need to duplicate this string before
 * calling translate_variables the next time.
 *
    STR_PARAM(astr,_astr);
    STR_PARAM(bstr,_bstr);
     .
     .
     .
 *
 * STR_PARAM() uses translate_variables, so using this macro
 * is like calling translate_variables. 
 */

/*
 * Script Function Writing Macros.
 */

/*
 * Is the variable NULL?
 * Parameters are sometimes sent to the functions as NULL if they
 * hold no value.
 */
#define PARAMETER(var, vtype)         ( !var || (var->type != vtype) )

#define RETURNS(t,val)   if ( !val ) { return NULL; } else {\
                            VARIABLE *vard; vard = new_var(); \
                            vard->value = val;  vard->type=t; return vard;  } 

char *translate_variables      	          args( ( void * owner, int type, char *exp ) );
VARIABLE * get_variable              args( ( VARIABLE *vlist, char *name ) );
VARIABLE * find_variable             args( ( void * owner, int type, char *name ) );
VARIABLE ** varlist	          args( ( void * owner, int type ) );
void assign_var                           args( ( void * owner, int type,
                                                  VARIABLE *var, char *name ) );
void assign_local                         args( ( INSTANCE *instance,
                                                  VARIABLE *var, char *name ) );
int            trigger                    args( ( void * owner, int type,
                                                  INSTANCE *instance ) );
void instance_track                       args( ( INSTANCE *instance, 
                                                  void * owner ) );

void * get_target args(( void * owner, int type, int * target_type, char *exp ));

/*
 * Convert a variable to its string equivalent.
 */
void STR_PARAM_parse  args( ( VARIABLE *var, char *_val, void *owner, int type ) );
#define STR_PARAM( var, _val )       STR_PARAM_parse(var,_val,owner,type)





/* Kludgy work-around. */
extern char param_buf[MSL];
extern PROP *pPropFreeList;
extern PLAYER *pActorFreeList;

extern int gotoloops;  /* for stopping infinitely recursive scripts */

/*
 * Modifying the Language: Rules on writing functions
 *
 * This is the starting place for adding functions to the language.
 * Most of the time, you won't need to edit the interpreter.c file
 * much, as it works and is air tight.  The only thing you may
 * have to do with this file is add your functions to the case
 * statement in eval_function().
 *
 * (The only exception is that, while you are porting the language,
 * you might need to adapt it, but I don't suggest changing the
 * way it works; all of the string parsing routines should work
 * regardless of what version of C you are using.)
 *
 * Writing Functions
 *
 * General form:
 *
 * Simple x + b, x - y uses the macro MATH_FUNCTIONI() to declare 
 * these types of functions for integers.
 *
 * If you are writing something other than a SET_ or GET_ function,
 * something specific that requires you to parse a loop or
 * manipulate a database more specifically, or has more than
 * two parameters, you will need to make use of the function
 * writing macros.
 *
 * VARIABLE *func_yourfunctionname( void * owner,   <- req'd
 *                                       int type,       <- req'd
 *                               VARIABLE *var, ..  <- optional
 *                                    .. var6 )
 * {
 *         char _var[MSL];     <- one for each parameter
 *            .
 *            .
 *         char _var6[MSL];  
 *         (your locals)
 *
 *         STR_PARAM(var,_var);   <- one for each parameter
 *            .                      Converts VARIABLE into strings.
 *            .
 *         STR_PARAM(var6,_var6);    
 *
 *           if ( IS_PRESENT(var) )  <- used to poll interpreter for 
 *                                      presence of
 *                                      variables in the script's
 *                                      call of the function. (see below)
 *
 *          code                  <- use Cast pointers for type conversion
 *
 *         RETURNS(TYPE_xxxx, local);    <- used in returning a variable
 *           or
 *         return NULL;                  <- used in 'void' functions
 *                                          which return no type or string
 * }
 *
 * Otherwise, use one of the GET_ or SET_FUNCTION macros for simple
 * variable transitions (giving a scripter access to a global, for
 * instance, or self-referential functions).
 */

/*
 * Macros.
 */

/*
 * Common test for existence of a variable.  Useful in
 * creation functions where there are two different ways to call it.
 * Such as:
 * func(this,that);
 * func(this);
 */
#define IS_PRESENT(var,_var)    ( var && _var[0] != '0' && _var[0] != '\0' )
#define REQUIRED_PARAMETER(t,_t) if ( !t || _t[0] == '\0' || _t[0] == '0' ) return NULL

/*
 * Multi-typing - Variables are of types 
 * ACTOR, PROP, STRING or SCENE 
 *
 * This macro uses a switch statement.  FOREACH() means
 * for each type, actor, prop, scene and string, do the
 * associated line of code.  It is only to make more
 * efficient the writing of functions, you could do this
 * manually.  This is used when a variable could be
 * of any type as a parameter to the function.
 *
 * FOREACH(local=ACTOR(owner), 
 *         local=PROP(owner), 
 *         local=SCENE(owner), 
 *         string/default);
 * 
 * Always in this order.  Code may vary, use {,} if necessary
 */
#define FOREACH(t,a,b,c,d)     \
switch (t) { case TYPE_ACTOR: a; break; case TYPE_PROP: b; break; case \
TYPE_SCENE: c; break; default: d; break; }

/*
 * Casts.  Used in the case statements present in every function.
 */
#define ACTOR(var)         ( (PLAYER *)(var)      )
#define PROP(var)          ( (PROP *)(var)        )
#define SCENE(var)         ( (SCENE *)(var) )

/*
 * Integer math function declarer.
 */
#define dMATH_FUNCTIONI(fname,OPERAND)  \
VARIABLE *fname( void * owner, int type, VARIABLE *astr, \
                                              VARIABLE *bstr, \
                                              VARIABLE *cstr )\
{   static char buf[MAX_STRING_LENGTH]; int x, y, z;\
    char _astr[MSL];    char _bstr[MSL];   char _cstr[MSL];\
    STR_PARAM(astr,_astr);    STR_PARAM(bstr,_bstr);  STR_PARAM(cstr,_cstr);\
    x = atoi(_astr);           y = atoi(_bstr);      z = atoi(_cstr);\
    sprintf( buf, "%d", (OPERAND) );\
    RETURNS(TYPE_STRING,str_dup(buf));    }\

/*
 * Set integer variable function declarer.
 */
#define dGET_FUNCTIONI(fname,vname)  \
VARIABLE *(fname)( void * owner, int type ){\
    char buf[MAX_STRING_LENGTH];\
    sprintf( buf, "%d", (vname) );\
    RETURNS(TYPE_STRING,str_dup(buf));    }

/*
 * Set integer variable function declarer.
 */
#define dSET_FUNCTIONI(fname,vname)  \
VARIABLE *fname( void * owner, int type, VARIABLE *v ){\
    char buf[MAX_STRING_LENGTH];\
    STR_PARAM(v,buf);\
    vname = atoi(buf); RETURNS(TYPE_STRING,str_dup(buf)); }

/*--*/

/*
 * Get integer on an actor variable function declarer.
 */
#define dGET_FUNCTIONAI(fname,vname)  \
VARIABLE *fname( void * owner, int type, VARIABLE *t, VARIABLE *v ){\
    char _v[MSL];   char _t[MSL];    PLAYER *ch;   \
    char buf[MSL];\
    STR_PARAM(v,_v);  STR_PARAM(t,_t); if ( !t ) return NULL;\
    ch=t->type==TYPE_ACTOR ? ACTOR(t->value) : find_actor_here(owner,type,_t);\
    if ( !ch ) return NULL;  sprintf( buf, "%d", ch->vname);\
    RETURNS(TYPE_STRING,str_dup(buf)); }

/*
 * Set integer on an actor variable function declarer.
 */
#define dSET_FUNCTIONAI(fname,vname)  \
VARIABLE *fname( void * owner, int type, VARIABLE *t, VARIABLE *v ){\
    char _v[MSL];   char _t[MSL];    PLAYER *ch;   \
    STR_PARAM(v,_v);  STR_PARAM(t,_t); if ( !t ) return NULL;\
    FOREACH(t->type,ch=ACTOR(t->value),ch=NULL,ch=NULL,ch=find_actor_here(owner,type,_t));\
    ch=t->type==TYPE_ACTOR ? ACTOR(t->value) : find_actor_here(owner,type,_t);\
    if ( !ch ) return NULL;  ch->vname = atoi(_v);    return NULL; }

/*
 * Get integer on a prop function declarer.
 */
#define dGET_FUNCTIONPI(fname,vname)  \
VARIABLE *fname( void * owner, int type, VARIABLE *t, VARIABLE *v ){\
    char _v[MSL];   char _t[MSL];   char buf[MSL]; PROP *p;   \
    STR_PARAM(v,_v);  STR_PARAM(t,_t); if ( !t ) return NULL;\
    p=t->type==TYPE_PROP ? PROP(t->value) : find_prop_here(owner,type,_t);\
    if ( !p ) return NULL;  sprintf( buf, "%d", p->vname);\
    RETURNS(TYPE_STRING,str_dup(buf)); }

/*
 * Set integer on a prop function declarer.
 */
#define dSET_FUNCTIONPI(fname,vname)  \
VARIABLE *fname( void * owner, int type, VARIABLE *t, VARIABLE *v ){\
    char _v[MSL];   char _t[MSL];    PROP *p;   \
    STR_PARAM(v,_v);  STR_PARAM(t,_t); if ( !t ) return NULL;\
    p=t->type==TYPE_PROP ? PROP(t->value) : find_prop_here(owner,type,_t);\
    if ( !p ) return NULL;  p->vname = atoi(_v);    return NULL; }


/*
 * Get integer on a scene function declarer.
 */
#define dGET_FUNCTIONSI(fname,vname)  \
VARIABLE *fname( void * owner, int type, VARIABLE *t, VARIABLE *v ){\
    char _v[MSL];   char _t[MSL];  char buf[MSL]; SCENE *s;   \
    STR_PARAM(v,_v);  STR_PARAM(t,_t); if ( !t ) return NULL;\
    s=t->type==TYPE_SCENE ? SCENE(t->value) : find_scene_here(owner,type,_t);\
    if ( !s ) return NULL;  sprintf( buf, "%d", s->vname ); \
    RETURNS(TYPE_STRING,str_dup(buf)); }

/*
 * Set integer on a scene function declarer.
 */
#define dSET_FUNCTIONSI(fname,vname)  \
VARIABLE *fname( void * owner, int type, VARIABLE *t, VARIABLE *v ){\
    char _v[MSL];   char _t[MSL];    SCENE *s;   \
    STR_PARAM(v,_v);  STR_PARAM(t,_t); if ( !t ) return NULL;\
    s=t->type==TYPE_SCENE ? SCENE(t->value) : find_scene_here(owner,type,_t);\
    if ( !s ) return NULL;  s->vname = atoi(_v);    return NULL; }


/*---*/

/*
 * Get string on an actor variable function declarer.
 */
#define dGET_FUNCTIONAS(fname,vname)  \
VARIABLE *fname( void * owner, int type, VARIABLE *t, VARIABLE *v ){\
    char _v[MSL];   char _t[MSL];    PLAYER *ch;   \
    STR_PARAM(v,_v);  STR_PARAM(t,_t); if ( !t ) return NULL;\
    ch=t->type==TYPE_ACTOR ? ACTOR(t->value) : find_actor_here(owner,type,_t);\
    if ( !ch ) return NULL;  \
    RETURNS(TYPE_STRING,str_dup(ch->vname)); }

/*
 * Set string on an actor variable function declarer.
 */
#define dSET_FUNCTIONAS(fname,vname)  \
VARIABLE *fname( void * owner, int type, VARIABLE *t, VARIABLE *v ){\
    char _v[MSL];   char _t[MSL];    PLAYER *ch;   \
    STR_PARAM(v,_v);  STR_PARAM(t,_t); if ( !t ) return NULL;\
    ch=t->type==TYPE_ACTOR ? ACTOR(t->value) : find_actor_here(owner,type,_t);\
    if ( !ch ) return NULL;  free_string(ch->vname); ch->vname = str_dup(_v);  return NULL; }


/*
 * Get string on a prop variable function declarer.
 */
#define dGET_FUNCTIONPS(fname,vname)  \
VARIABLE *fname( void * owner, int type, VARIABLE *t, VARIABLE *v ){\
    char _v[MSL];   char _t[MSL];    PROP *p;   \
    STR_PARAM(v,_v);  STR_PARAM(t,_t); if ( !t ) return NULL;\
    p=t->type==TYPE_PROP ? PROP(t->value) : find_prop_here(owner,type,_t);\
    if ( !p ) return NULL;  \
    RETURNS(TYPE_STRING,str_dup(p->vname)); }

/*
 * Set string on a prop variable function declarer.
 */
#define dSET_FUNCTIONPS(fname,vname)  \
VARIABLE *fname( void * owner, int type, VARIABLE *t, VARIABLE *v ){\
    char _v[MSL];   char _t[MSL];    PROP *p;   \
    STR_PARAM(v,_v);  STR_PARAM(t,_t); if ( !t ) return NULL;\
    p=t->type==TYPE_PROP ? PROP(t->value) : find_prop_here(owner,type,_t);\
    if ( !p ) return NULL;  free_string(p->vname); p->vname = str_dup(_v);  return NULL; }


/*
 * Get string on a scene variable function declarer.
 */
#define dGET_FUNCTIONSS(fname,vname)  \
VARIABLE *fname( void * owner, int type, VARIABLE *t, VARIABLE *v ){\
    char _v[MSL];   char _t[MSL];    SCENE *s;   \
    STR_PARAM(v,_v);  STR_PARAM(t,_t); if ( !t ) return NULL;\
    s=t->type==TYPE_SCENE ? SCENE(t->value) : find_scene_here(owner,type,_t);\
    if ( !s ) return NULL;  \
    RETURNS(TYPE_STRING,str_dup(s->vname)); }

/*
 * Set string on a scene variable function declarer.
 */
#define dSET_FUNCTIONSS(fname,vname)  \
VARIABLE *fname( void * owner, int type, VARIABLE *t, VARIABLE *v ){\
    char _v[MSL];   char _t[MSL];    SCENE *s;   \
    STR_PARAM(v,_v);  STR_PARAM(t,_t); if ( !t ) return NULL;\
    s=t->type==TYPE_SCENE ? SCENE(t->value) : find_scene_here(owner,type,_t);\
    if ( !s ) return NULL;  free_string(s->vname); s->vname = str_dup(_v);  return NULL; }


/*--*/

/*
 * Set string variable function declarer.
 */
#define dGET_FUNCTIONS(fname,vname)  \
VARIABLE *fname( void * owner, int type ){\
    char buf[MAX_STRING_LENGTH];\
    sprintf( buf, "%s", (v) ); RETURNS(TYPE_STRING,str_dup(buf));    }

/*
 * Get string variable function declarer.
 */
#define dSET_FUNCTIONS(fname,v)  \
VARIABLE *fname( void * owner, int type, VARIABLE *v ){\
    char buf[MAX_STRING_LENGTH];\
    STR_PARAM(v,buf); vname = str_dup(buf);\
    RETURNS(TYPE_STRING,str_dup(buf));}

/*
 * Set target variable function declarer.
 */
#define dGET_FUNCTIONT(fname,ttype,vname)  \
VARIABLE *fname( void * owner, int type ){\
    char buf[MAX_STRING_LENGTH];\
    sprintf( buf, "%s", (v) ); RETURNS(TYPE_STRING,str_dup(buf));    }
/*nyi*/

/*
 * Get target variable function declarer.
 */
#define dSET_FUNCTIONT(fname,ttype,vname)  \
VARIABLE *fname( void * owner, int type, VARIABLE *v ){\
    char buf[MAX_STRING_LENGTH];\
    if( vname = str_dup(buf);\
    RETURNS(TYPE_STRING,str_dup(buf));}

/*
 * Generic graphics converter macro.
 */
#define dGRAPHICS_FUNCTION(fname,OPERAND)  \
VARIABLE *fname( void * owner, int type, VARIABLE *va,\
                                              VARIABLE *vb,\
                                              VARIABLE *vc,\
                                              VARIABLE *vd,\
                                              VARIABLE *ve,\
                                              VARIABLE *vf  ){\
    char _a[MAX_STRING_LENGTH]; int a;\
    char _b[MAX_STRING_LENGTH]; int b;\
    char _c[MAX_STRING_LENGTH]; int c;\
    char _d[MAX_STRING_LENGTH]; int d;\
    char _e[MAX_STRING_LENGTH]; int e;\
    char _f[MAX_STRING_LENGTH]; int f;\
    STR_PARAM(va,_a); a=atoi(_a);\
    STR_PARAM(vb,_b); b=atoi(_b);\
    STR_PARAM(vc,_c); c=atoi(_c);\
    STR_PARAM(vd,_d); d=atoi(_d);\
    STR_PARAM(ve,_e); e=atoi(_e);\
    STR_PARAM(vf,_f); f=atoi(_f);\
    OPERAND;\
    return NULL; }



/*
 * Specialized functions for multi-typed objects to reach out to other
 * database objects.  Used in functions.c
 */

PLAYER      *find_actor_here args( ( void * owner, int type, char *exp ) );
PROP        *find_prop_here  args( ( void * owner, int type, char *exp ) );
SCENE *find_scene_here args( ( void * owner, int type, char *exp ) );

