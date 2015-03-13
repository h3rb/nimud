/***************************************************************************
* MapMaker originally created by
* Wendigo of Oblivionmud.static.net port 9000
*
* Adapted to NiM5 by Locke 2005 (swervez@hotpop.com)
*
* If you use this code, enjoy it, have bug fixes or simply want to
* chat, please feel free to email me at  kcaffet@hotmail.com (msn as well)
****************************************************************************/

/*
 * Installation of this snippet should be rather easy.  Simply put the file
 * in with the rest of your source code, edit your Makefile and add the
 * appropriate $.o file (where $ is the name of this file)
 * Declare a cmd_map in interp.h and add the command into interp.c
 *
 * This is a stand-alone file.  However, it is recommended that you move
 * many of these functions into their appropriate files (All the recycle
 * info into recycle.c and wnatnot.
 */

/* Commands for cmd_map are as follows:
 * (Note: Commands do not need to be issued in any order)
 * 
 * world: 	zone boundaries do not apply. It will go out as far as it
 *		can into the world.  Not specifying world leaves it at the
 *		default of zone only
 * 			
 * tight:	Do not show scene links(Not specifying will show links)
 * 
 * # > 0:	Ex: 4, 6, 30  Will show that many spaces away
 *		from you, forming a box(not specifying will go until finished)
 * 			
 * # < 0:	Ex: -1, -5, -20  Will limit the map to that depth (positive)
 *		(not specifying will go until finished)
 * 			
 * doors:	Will not go past closed doors.(Not Specified will go through
 *		all doors)
 *
 * mob:		Will highlight squares with mobs in them (colour coded) (Not
 *		specified will not show mob colours)
 *
 * terrain:	Will toggle terrain mode (scenes shown with terrain symbols)
 *		(not specified will show regular zone scenes)
 *
 * fog:		Enables fog mode (Will not go past certain terrain types)
 *		(Not specified will go through all terrain types)
 *
 * text:	Enables scene description text to be displayed to the right
 *		of the map
 *
 * border:	Adds a pretty border around the map. (not specified, no border)
 * 
 * Ex: 	map tight 8    	Will stay in the zone, not show scene links and only
 * 						go 8 spaces away from the user
 *
 * map 20 world	zone boundaries ignored, shows scene links, goes 20
 * 						spaces away from the user
 *
 * 		map world tight mob terrain fog border doors
 * 						Yah..  Look at all the above options, apply them
 *
 * 	As a side note, colour code had to be reduced to a bare minimum due
 * 	to a resource limit on how much can be displayed at one time.
 * 	One line of 80 characters can take up a lot of space if every 
 * 	spot is used and has an extra 4-8 colour code characters in it.
 * 	While the line  "bobo likes pie"  may seem small it gets quite large
 * 	when it's "{rb{ro{rb{ro{w {gl{gi{gk{ge{gs{w {rp{ri{re{w"
 * 	
 * 	If you begin to experience disconnections when using the map at it's
 * 	maximum grid size, simply reduce the grid size.
 */


#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h> 
#include <time.h>
#include <ctype.h>

#include <zlib.h>
#include "nimud.h"


#define MAX_MAP 77 // Shows 1/2 the given scenes (links) Always odd
#define MAP_MID 38 // Always 2x+1 to get MAX_MAP (makes the middle
#define MAX_MAP_HASH 16

// -1 is already taken, all positives are taken, so start lower
#define NS_2WAY		-2
#define NS_1WAYN	-3
#define NS_1WAYS	-4
#define NS_HIT		-5
#define EW_2WAY		-6
#define EW_1WAYE	-7
#define EW_1WAYW	-8
#define EW_HIT		-9
#define NS_2WAYD	-10
#define NS_1WAYND	-11
#define NS_1WAYSD	-12
#define NS_HITD		-13
#define EW_2WAYD	-14
#define EW_1WAYED	-15
#define EW_1WAYWD	-16
#define EW_HITD		-17
#define NS_UNN		-18
#define NS_UNS		-19
#define EW_UNE		-20
#define EW_UNW		-21
#define NS_UNND		-22
#define NS_UNSD		-23
#define EW_UNED		-24
#define EW_UNWD		-25

// Structures that are used.
typedef struct	map_qd		MAP_QD;
typedef struct	map_visit	MAP_VISIT;
typedef struct	map_param	MAP_PARAM;

// Memory management is internalized for this routine.
// Former external information, used by many things.

MAP_QD *map_qd_free;
MAP_QD *map_queue;
MAP_VISIT *map_visit[MAX_MAP_HASH];
MAP_VISIT *map_visit_free;

int top_map_visit;
int top_map_qd;
/*
int map[][];
 */
MAP_PARAM *mp;


// Function definitions
void make_map			args ( (PLAYER *ch, int x, int y) );
void add_to_map			args ( (PLAYER *ch, SCENE *scene, int x, int y, int depth) );
void free_map_qd		args ( (MAP_QD *map_qd) );
MAP_QD *new_map_qd		args ( (void) );
void add_map_qd			args ( (MAP_QD *map_qd) );
MAP_QD *next_map_qd		args ( (void) );
void init_map			args ( (void) );
void init_mp			args ( (PLAYER *ch) );
void show_map			args ( (PLAYER *ch, int x, int y) );
MAP_VISIT *new_map_visit	args ( (void) );
void free_map_visit		args ( (MAP_VISIT *mapv) );
void add_visited		args ( (int scene) );
bool has_visited		args ( (int scene) );
void free_all_visits	args ( (void) );
int get_y_to			args ( (void) );
int get_x_to			args ( (void) );
int get_y_start			args ( (void) );
int get_x_start			args ( (void) );
int put_link			args ( (SCENE *scene, int other, int dir) );
char *get_link			args ( (int link_type) );
char *get_linkc			args ( (int link_type) );
char *get_mscene			args ( (PLAYER *ch, int scene) );
char *get_mscenec		args ( (PLAYER *ch, int scene) );
char *get_map_text		args ( (void) );	
char *finish_map_text	args ( (void) );
void clean_map_string	args ( (PLAYER *ch) );
char *mpcolour			args ( (char *arg) );

MAP_VISIT *map_visit[MAX_MAP_HASH];
MAP_VISIT *map_visit_free;
MAP_QD	*map_qd_free;
MAP_QD	*map_queue;
MAP_PARAM *mp;
int map[MAX_MAP][MAX_MAP];


// Map parameters  Saved externally to avoid many arguments being used in map function
struct map_param
{
	bool zone;
	bool tight;
	bool fog;
	int size;
	bool border;
	bool doors;
	int depth;
	bool col; // Used to add the default colour back if needed - not used as much anymore
	bool mobs;
	bool ter; // Switches to terrain mode.
	bool text;
	char *desc; 
	char *portion;
	char *colour; // Stores the previous colour code used
	bool valid;
};
	

// visited scene
struct map_visit
{
	MAP_VISIT *next;
	int scene;
	bool valid;
};

// Map queue data
struct map_qd
{
	MAP_QD *next;
	int scene;
	int x;
	int y;
	int depth;
	bool valid;
};

char *mpcolour (char *arg)
{
        return "";
	if (mp->col == TRUE)
	{
		strcpy (mp->colour, arg);
		return arg;
	}	
	else if (!strcmp(mp->colour, arg))
		return "";
	else
	{
		strcpy (mp->colour, arg);
		return arg;
	}
}


// Free's all visited scenes from the map_visit hash table
void free_all_visits (void)
{
	MAP_VISIT *m, *map_next;
	int hash;

	for (hash = 0; hash < MAX_MAP_HASH; hash++)
	{
		for (m = map_visit[hash]; m != NULL; m = map_next)
		{
			map_next = m->next;
			free_map_visit(m);
		}
		map_visit[hash] = NULL;
	}
	return;
}

// Adds a scene as visited
void add_visited (int scene)
{
	MAP_VISIT *mv;
	int hash;

	hash = scene % MAX_MAP_HASH;
	mv = new_map_visit();
	mv->scene = scene;

	mv->next = map_visit[hash];
	map_visit[hash] = mv;	
	
	return;
}

// Returns T/F if scene was visited
bool has_visited (int scene)
{
	MAP_VISIT *m;
	int hash;
	bool visited = FALSE;
	
	hash = scene % MAX_MAP_HASH;

	for (m = map_visit[hash]; m != NULL; m = m->next)	
	{
		if (m->scene == scene)
		{
			visited = TRUE;
			break;
		}	
	}
	
	return visited;
}

// Initialized the map, sets all values in grid to -1
// As well it free's all previously visited scenes
void init_map (void)
{
	int x, y;

	for (x = 0; x < MAX_MAP; x++)	
		for (y = 0; y < MAX_MAP; y++)
			map[x][y] = -1;
	
	free_all_visits();
	
	return;
}

void init_mp (PLAYER *ch)
{

	if (mp == NULL)
	{
/*		wiznet ("New map_param created", NULL, NULL, WIZ_MEM, 0, 0);*/
		mp = alloc_perm (sizeof(*mp));
		mp->portion = alloc_mem (256); // 80 chars with {w + extra
		mp->desc = alloc_mem(4608); // max string
		mp->colour = alloc_mem(16); // just for colour
	}
	
	/*VALIDATE (mp);*/

	mp->zone = FALSE;
	mp->tight = FALSE;
	mp->fog = FALSE;
	mp->size = MAP_MID;
	mp->border = TRUE;
	mp->doors = TRUE;
	mp->depth = -1;
	mp->col = FALSE;
	mp->mobs = FALSE;
	mp->ter = FALSE;
	mp->text = FALSE;
	strcat (mp->portion, "");
	strcat (mp->desc, "");
	strcat (mp->colour, "");
	if (ch != NULL)
	{ // Can call function with NULL (if used externally)
		sprintf (mp->desc, ch->in_scene->description);
		clean_map_string(ch);
	}
	return;
}

// Hack of format_string in string.c
void clean_map_string (PLAYER *ch)
{
	char xbuf[MAX_STRING_LENGTH];
	char xbuf2[MAX_STRING_LENGTH];
	char *rdesc;
	int i = 0;
	bool cap = TRUE;		
	
	sprintf (xbuf, "%c", '\0');
	sprintf (xbuf2, "%c", '\0');

	
	xbuf[0] = xbuf2[0] = 0;	
		
	for (rdesc = mp->desc; *rdesc; rdesc++)	
	{
		if (*rdesc == '\0')
			break;
		else if (*rdesc == '\n')		
		{		
			if (xbuf[i - 1] != ' ')			
			{			
				xbuf[i] = ' ';				
				i++;				
			}			
		}		
		else if (*rdesc == '\r')
		{
			if (xbuf[i - 1] != ' ')
			{
				xbuf[i] = ' ';
				i++;
			}
		}
		else if (*rdesc == ' ')		
		{		
			if (xbuf[i - 1] != ' ')			
			{			
				xbuf[i] = ' ';				
				i++;				
			}			
		}		
		else if (*rdesc == ')')		
		{		
			if (xbuf[i - 1] == ' ' && xbuf[i - 2] == ' ' &&			
					(xbuf[i - 3] == '.' || xbuf[i - 3] == '?'					
					 || xbuf[i - 3] == '!'))					
			{			
				xbuf[i - 2] = *rdesc;				
				xbuf[i - 1] = ' ';				
				xbuf[i] = ' ';				
				i++;				
			}			
			else			
			{			
				xbuf[i] = *rdesc;				
				i++;				
			}			
		}	 
		else if (*rdesc == '.' || *rdesc == '?' || *rdesc == '!')		
		{		
			if (xbuf[i - 1] == ' ' && xbuf[i - 2] == ' ' &&			
					(xbuf[i - 3] == '.' || xbuf[i - 3] == '?'					
					 || xbuf[i - 3] == '!'))					
			{			
				xbuf[i - 2] = *rdesc;				
				if (*(rdesc + 1) != '\"')				
				{				
					xbuf[i - 1] = ' ';					
					xbuf[i] = ' ';					
					i++;					
				}				
				else				
				{				
					xbuf[i - 1] = '\"';					
					xbuf[i] = ' ';					
					xbuf[i + 1] = ' ';					
					i += 2;					
					rdesc++;   
/************/		if (*rdesc == '\0')
						break;						
				}				
			}			
			else			
			{			
				xbuf[i] = *rdesc;				
				if (*(rdesc + 1) != '\"')				
				{				
					xbuf[i + 1] = ' ';					
					xbuf[i + 2] = ' ';					
					i += 3;					
				}				
				else				
				{				
					xbuf[i + 1] = '\"';					
					xbuf[i + 2] = ' ';					
					xbuf[i + 3] = ' ';					
					i += 4;					
					rdesc++;  
/****/				if (*rdesc == '\0')
						break;
				}				
			}			
			cap = TRUE;			
		}
	 	else	
		{		
			xbuf[i] = *rdesc;			
			if (cap)			
			{			
				cap = FALSE;				
				xbuf[i] = UPPER (xbuf[i]);				
			}			
			i++;			
		}	
		if (*rdesc == '\0')
			break;
	}	

	xbuf[i] = '\0';
	sprintf (mp->desc, xbuf);
	return;		
}

// In a function incase I want to mod it later
char *finish_map_text (void)
{
	return mp->desc;
}

char *get_map_text ( )
{
	char buf[MSL];
	int size, count = 3, used = 0, j = 0;
	bool word = FALSE, done = FALSE;;

	// Quick leave check
	if (mp->desc[0] == '\0')
		return "";

	// Find the size (spaces) left for text
	size = MAP_MID * 2;
	size -= mp->size * 2;
	if (mp->border)
		size -= 2;

	if (size < 4) // Don't bother if we only have 3 or less
		return "";
	
	// Initialize output
	sprintf (buf, "%s ", mpcolour("{w"));

	// Remove leading spaces
	for ( ; mp->desc[j] == ' '; j++, used++);

	// Leave if theres nothing left.
	if (mp->desc[0] == '\0')
		return "";
	
	// Put as many chars into the buf as we are allowed
	for ( ; j <= size; j++)
	{
		if (mp->desc[j] == '\0')
		{
			word = TRUE;
			done = TRUE;
			break;
		}
		
		if (mp->desc[j] == ' ')
			word = TRUE;
	
		// Colour case
		if (mp->desc[j] == '{')
		{
			buf[count] = mp->desc[j];
			count++;
			used++;
			size++;
			j++;
			// My colour codes, change as needed
			// NOTE: {{ is not a colour code, colour code
			// only cosidered to be something that does
			// not add to outputted text, but formats it
			if (strchr("rRgGbBmMcCyYwWD", mp->desc[j]))
				size++;
			else if (mp->desc[j] == '\0')
			{
				word = TRUE;
				done = TRUE;
			}
		}
			
		buf[count] = mp->desc[j];
		count++;
		used++;
		
	}
	
	if (word) // Did we get 1 full word? backup to end of last word
	{
		for ( ; buf[count - 1] != ' '; count--, used--);
		
		buf[count] = '\0';
		strcpy (mp->portion, buf);			
	}
	else // No word.  Use a -
	{
		used--;
		buf[count - 1] = '-';
		buf[count] = '\0';
		strcpy (mp->portion, buf);
	}

	// Roll everything back in our desc to remove what was used
	if (mp->desc[0] != '\0')
		for (j = 0; (mp->desc[j] = mp->desc[used + j]) != '\0'; j++);
	
	mp->desc[j] = '\0';
	
	if (done)
		mp->desc[0] = '\0';

	mp->col = TRUE;
	return mp->portion;
}


// Returns the colour of the link
char *get_linkc (int lnk)
{
	switch (lnk)
	{
		case NS_2WAYD:
		case NS_1WAYND:
		case NS_1WAYSD:
		case NS_HITD:
		case EW_2WAYD:
		case EW_1WAYED:
		case EW_1WAYWD:
		case EW_HITD:
		case NS_UNND:
		case NS_UNSD:
		case EW_UNED:
		case EW_UNWD:
			if (mp->col)
			{
				mp->col = FALSE;
				return "^2"; //mpcolour("{w{G");
			}
			else
				return ""; // mpcolour("{G");
		default:
			return mpcolour("{g");
	}
	// Somethin goofed
	return "^-"; //mpcolour("{g");
}

// Returns the character of the link
char *get_link (int lnk)
{
	switch (lnk)
	{
		case NS_2WAY:
		case NS_2WAYD:
			return "|";
		case NS_1WAYN:
		case NS_1WAYND:
			return "^^";
		case NS_1WAYS:
		case NS_1WAYSD:
			return "v";
		case NS_HIT:
		case NS_HITD:
			return "/";
		case EW_2WAY:
		case EW_2WAYD:
			return "-";
		case EW_1WAYE:
		case EW_1WAYED:
			return ">";
		case EW_1WAYW:
		case EW_1WAYWD:
			return "<";
		case EW_HIT:
		case EW_HITD:
			return "=";
		case NS_UNN:
		case NS_UNND:
			return "]";
		case NS_UNS:
		case NS_UNSD:
			return "[";
		case EW_UNE:
		case EW_UNED:
			return ")";
		case EW_UNW:
		case EW_UNWD:
			return "(";
			
		default:
			return "?";
	}
	return "?";
}
// Returns the character of the scene.
// Change as you like
char *get_mscene (PLAYER *ch, int scene)
{
	SCENE *rm;
	PLAYER *keeper;
	SHOP *shop;

	rm = get_scene(scene);
	shop = NULL;
	
	// Check for a shop. 
	for (keeper = rm->people; keeper; keeper = keeper->next_in_scene)
	{
		if (NPC(keeper) && (shop = keeper->pIndexData->pShop) != NULL)
			break;
	}

	if (mp->ter) // Terrain mode
	{
		switch (rm->move)
		{
			case MOVE_INSIDE:
				return "*";
			case MOVE_CITY:
				return "+";
			case MOVE_FIELD:
				return "o";
			case MOVE_FOREST:
				return "f";
			case MOVE_HILLS:
				return "z";
			case MOVE_MOUNTAIN:
				return "x";
			case MOVE_WATER_SWIM:
				return "~";
			case MOVE_WATER_NOSWIM:
				return "~";
//			case MOVE_UNUSED:
//				return "*";
			case MOVE_AIR:
				return ".";
			case MOVE_DESERT:
				return "!";
			default:
				return "*";
		}
	}
	// End Terrain mode
	
	if (shop != NULL) // Do we have a shop??
		return "S";
	else if (IS_SET (rm->scene_flags, SCENE_PET_SHOP)) // How about a pet shop?
		return "P";

	// Default.
	return "*";
}
/* Returns the colour code of the scene */
char *get_mscenec (PLAYER *ch, int scene)
{
	SCENE *rm;
	PLAYER *mob;
	int mtype = 0;

	rm = get_scene(scene);

	for (mob = rm->people; mob; mob = mob->next_in_scene)
	{
		if (NPC(mob))
		{
			if (IS_SET(mob->flag, ACTOR_AGGRESSIVE))
			{
				mtype = 2;
				break;
			}
			else
				mtype = 1;
		}
	}

	if (mp->ter) /* Terrain mode colour, if needed.  This will override exit and mob colours.  Warning:  May cause disconnect on large display due to extra colour codes - hopefully the new colour control code has fixed that */
	{
//		mp->col = TRUE;
 		// Set as desired
 		switch (rm->move)
		{		
			case MOVE_INSIDE:
                                if ( HAS_ANSI(ch) ) 
                                 return "^N^+^4";
                                else return "";						
			case MOVE_CITY:
                                if ( HAS_ANSI(ch) ) 
                                 return "^-^6^B";
                                else return "";						
			case MOVE_FIELD:
                                if ( HAS_ANSI(ch) ) 
                                 return "^#^2^B";
                                else return "";				
			case MOVE_FOREST:
                                if ( HAS_ANSI(ch) ) 
                                 return "^#^B^0";
                                else return "";
			case MOVE_HILLS:
                                if ( HAS_ANSI(ch) ) return "^4"; else return "";
			case MOVE_MOUNTAIN:
                                if ( HAS_ANSI(ch) ) return "^=^5"; else return "";
			case MOVE_WATER_SWIM:
                                if ( HAS_ANSI(ch) ) 
                                 return "^B^1"; else return "";
			case MOVE_WATER_NOSWIM:
                                if ( HAS_ANSI(ch) ) 
                                 return "^:^0"; else return "";
//			case MOVE_UNUSED:
//				return mpcolour("{g");
			case MOVE_AIR:
                                if ( HAS_ANSI(ch) ) 
                                 return "^:^3"; else return "";
			case MOVE_DESERT:
                                if ( HAS_ANSI(ch) ) 
                                 return "^-^6^B"; else return "";
			default:
                                if ( HAS_ANSI(ch) ) 
                                 return "^N^-"; else return "";
		}
                return "";
	} // End terrain mode

	
	// Does it have an up and a down? Can they get there?
	if (	(rm->exit[DIR_UP] != NULL
              && rm->exit[DIR_UP]->to_scene
              && !IS_SET(rm->exit[DIR_UP]->exit_flags, EXIT_CONCEALED) 
              && !IS_SET(rm->exit[DIR_UP]->exit_flags, EXIT_SECRET) )
             && (rm->exit[DIR_DOWN] != NULL
              && rm->exit[DIR_DOWN]->to_scene 
              && !IS_SET(rm->exit[DIR_DOWN]->exit_flags, EXIT_CONCEALED)
              && !IS_SET(rm->exit[DIR_DOWN]->exit_flags, EXIT_SECRET) 
	&& (mp->doors || (!IS_SET(rm->exit[DIR_UP]->exit_flags, EXIT_CLOSED)
|| !IS_SET(rm->exit[DIR_DOWN]->exit_flags, EXIT_CLOSED)))) ) 
	{
		if (mp->mobs && mtype == 1)
		{
			mp->col = TRUE;
                if ( HAS_ANSI(ch) ) return CYAN; else return "";
		}
		else if (mp->mobs && mtype == 2)
		{
			mp->col = TRUE;
                if ( HAS_ANSI(ch) ) return B_RED; else return "";
		}
		else
                if ( HAS_ANSI(ch) ) return RED; else return "";
	}
	else if (rm->exit[DIR_UP] != NULL &&
!IS_SET(rm->exit[DIR_UP]->exit_flags, EXIT_CONCEALED) &&
!IS_SET(rm->exit[DIR_UP]->exit_flags, EXIT_SECRET)
	&& (mp->doors || !IS_SET(rm->exit[DIR_UP]->exit_flags, EXIT_CLOSED) )
                ) // Going up?
	{
		if (mp->mobs && mtype == 1)
		{
			mp->col = TRUE;
			return ""; //"^B^4";
		}
		else if(mp->mobs && mtype == 2)
		{
			mp->col = TRUE;
			return ""; //"^B^1";
		}
		else
			return ""; //^N^-"; //mpcolour("{M");
	}
	else if (rm->exit[DIR_DOWN] != NULL
				&& (mp->doors || !IS_SET(rm->exit[DIR_DOWN]->exit_flags, EXIT_CLOSED))) // Going down..
	{
		if (mp->mobs && mtype == 1)
		{
			mp->col = TRUE;
			return mpcolour("\e[0;44m{B");
		}
		else if (mp->mobs && mtype == 2)
		{
			mp->col = TRUE;
			return mpcolour("\e[0;41m{B");
		}
		else
			return mpcolour("{B");
	}	
	// Default.
	if (mp->col == TRUE)
	{
		if (mp->mobs && mtype == 1)
		{
			mp->col = TRUE;
                if ( HAS_ANSI(ch) ) return CYAN; else return "";
		}
		else if (mp->mobs && mtype == 2)
		{
			mp->col = TRUE;
                if ( HAS_ANSI(ch) ) return B_RED; else return "";
		}
		else
		{
			mp->col = FALSE;
                        return "";
		}
	}
	else if (mp->mobs && mtype == 1)
	{
		mp->col = TRUE;
		return mpcolour("\e[0;44;32m");
	}
	else if (mp->mobs && mtype == 2)
	{
		mp->col = TRUE;
		return mpcolour("\e[0;41;32m");
	}
	else
		return ""; //"^N^-"; //mpcolour("{g");
}

	

// Finds out what type of link to put between scenes
// returns the link number
int put_link (SCENE *scene, int next, int dir)
{
	SCENE *other;
	EXIT  *pexit, *org;
	int dir2;

	// Get the reverse dir
	dir2 = dir == 0 ? 2 : dir == 1 ? 3 : dir == 2 ? 0 : 1;

	if (next > 0) // Do we have a scene there already?
	{
		other = get_scene(next);
		pexit = other->exit[dir2];
	}
	else
	{
		pexit = scene->exit[dir]->to_scene->exit[dir2];
		other = scene->exit[dir]->to_scene;
	}
	
	org = scene->exit[dir];

	if  (dir == DIR_NORTH) // I'm going north
	{
		if (pexit == NULL) // 1 way?
		{
			if (other != NULL && org->to_scene == other) // Is the link to that scene?
				return IS_SET(org->exit_flags, EXIT_ISDOOR) ? NS_1WAYND : NS_1WAYN;
			else // Goes that way but not to that scene
				return IS_SET(org->exit_flags, EXIT_ISDOOR) ? NS_UNND : NS_UNN;
		}		
		else if (pexit->to_scene == scene) // 2 way?		
			return (IS_SET(pexit->exit_flags, EXIT_ISDOOR) || IS_SET(org->exit_flags, EXIT_ISDOOR)) ? NS_2WAYD : NS_2WAY;
		else if (pexit->to_scene != scene) // 2 way collide?
			return (IS_SET(pexit->exit_flags, EXIT_ISDOOR) || IS_SET(org->exit_flags, EXIT_ISDOOR)) ? NS_HITD : NS_HIT;
		else
			return -1;
	}
	else if (dir == DIR_SOUTH) // I'm going south
	{
		if (pexit == NULL) // 1 way?
		{
			if (org->to_scene == other) // Is the link to that scene?
				return IS_SET(org->exit_flags, EXIT_ISDOOR) ? NS_1WAYSD : NS_1WAYS;
			else // Goes that way but not to that scene
				return IS_SET(org->exit_flags, EXIT_ISDOOR) ? NS_UNSD : NS_UNS;
		}
		else if (pexit->to_scene == scene) // 2 way?
			return (IS_SET(pexit->exit_flags, EXIT_ISDOOR) || IS_SET(org->exit_flags, EXIT_ISDOOR)) ? NS_2WAYD : NS_2WAY;
		else if (pexit->to_scene != scene) // 2 way collide?
			return (IS_SET(pexit->exit_flags, EXIT_ISDOOR) || IS_SET(org->exit_flags, EXIT_ISDOOR)) ? NS_HITD : NS_HIT;
		else
			return -1;
	}
	else if (dir == DIR_EAST) // I'm going east
	{
		if (pexit == NULL) // 1 way?
		{
			if (org->to_scene == other) // Is the link to that scene?
				return IS_SET(org->exit_flags, EXIT_ISDOOR) ? EW_1WAYED : EW_1WAYE;
			else // Goes that way but no to that scene
				return IS_SET(org->exit_flags, EXIT_ISDOOR) ? EW_UNED : EW_UNE;
		}
		else if (pexit->to_scene == scene) // 2 way?
			return (IS_SET(pexit->exit_flags, EXIT_ISDOOR) || IS_SET(org->exit_flags, EXIT_ISDOOR)) ? EW_2WAYD : EW_2WAY;
		else if (pexit->to_scene != scene) // 2 way collide?
			return (IS_SET(pexit->exit_flags, EXIT_ISDOOR) || IS_SET(org->exit_flags, EXIT_ISDOOR)) ? EW_HITD : EW_HIT;
		else
			return -1;
	}
	else if(dir == DIR_WEST) // I'm going west
	{
		if (pexit == NULL) // 1 way?
		{
			if (org->to_scene == other) // Is the link to that scene?
				return IS_SET(org->exit_flags, EXIT_ISDOOR) ? EW_1WAYWD : EW_1WAYW;
			else
				return IS_SET(org->exit_flags, EXIT_ISDOOR) ? EW_UNWD : EW_UNW;
		}
		else if (pexit->to_scene == scene)
			return (IS_SET(pexit->exit_flags, EXIT_ISDOOR) || IS_SET(org->exit_flags, EXIT_ISDOOR)) ? EW_2WAYD : EW_2WAY;
		else if (pexit->to_scene != scene)
			return (IS_SET(pexit->exit_flags, EXIT_ISDOOR) || IS_SET(org->exit_flags, EXIT_ISDOOR)) ? EW_HITD : EW_HIT;
		else
			return -1;
	}
	else
		return -1;	
}

// Returns a new, clean map_visit
MAP_VISIT *new_map_visit (void)
{
	static MAP_VISIT mapv_zero;
	MAP_VISIT *mapv;

	if (map_visit_free == NULL)
	{
/*		wiznet ("New map_visit created", NULL, NULL, WIZ_MEM, 0, 0);*/
		mapv = alloc_perm (sizeof (*mapv));
		top_map_visit++;
	}
	else
	{
		mapv = map_visit_free;
		map_visit_free = map_visit_free->next;
	}

	*mapv = mapv_zero;

	mapv->next = NULL;
	mapv->scene = -1;

	/*VALIDATE (mapv);*/
	return mapv;
}

// free's the map visit for future use
void free_map_visit (MAP_VISIT *mapv)
{
	if (!mapv)
		return;
/*	INVALIDATE (mapv);*/
	mapv->next = map_visit_free;
	map_visit_free = mapv;
	return;
}

// Returns a new map queue data
MAP_QD *new_map_qd (void)
{
	static MAP_QD map_zero;
	MAP_QD *m;

	if (map_qd_free == NULL)
	{
/*		wiznet ("New map_qd created", NULL, NULL, WIZ_MEM, 0, 0);*/
		m = alloc_perm (sizeof (*m));	
		top_map_qd++;
	}
	else
	{
		m = map_qd_free;
		map_qd_free = map_qd_free->next;
	}

	*m = map_zero;

	m->next = NULL;
	m->scene = -1;
	m->x = -1;
	m->y = -1;

	/*VALIDATE (m);*/
	return m;
}
	
// Free map queue data
void free_map_qd (MAP_QD *mqd)
{
	if (!mqd) return;
	/*INVALIDATE (mqd);*/
	mqd->next = map_qd_free;
	map_qd_free = mqd;
	return;
}

// Add an MQD to the queue
void add_map_qd (MAP_QD *mqd)
{
	MAP_QD *m;

	// Mark the scene as visited
	add_visited(mqd->scene);

	// Is there a mqd in the queue?  If not lets start it off
	if (map_queue == NULL)
	{
		map_queue = mqd;
		return;
	}

	// Lets add it at the end.  It has to wait in line..
	for (m = map_queue; m != NULL; m = m->next)
	{
		if (m->next == NULL)
		{
			m->next = mqd;
			break;
		}
	}
	return;	
}

// Returns the next queue waiting in line
MAP_QD *next_map_qd (void)
{
	MAP_QD *m;

	if (map_queue == NULL)
		return NULL;
	
	m = map_queue;
	map_queue = map_queue->next;
	return m;
}

// Command to start it all.
void cmd_map (PLAYER *ch, char *argument)
{
	int x = MAP_MID, y = MAP_MID, size = -1;
	char arg[MSL];

        //clrscr(ch);
	init_mp(ch);
	
	for (argument = one_argument(argument, arg); arg[0] != '\0'; )
	{ // For every argument given..
		if (is_number(arg))
		{
			size = atoi(arg);
			if (size >= MAP_MID)
				mp->size = MAP_MID;
			else if (size > 0)
				mp->size = size;
			else if (size < 0)
				mp->depth = size * -1;
		}
		else if(!str_prefix(arg, "world"))
			mp->zone = TRUE;
		else if(!str_prefix(arg, "tight"))
			mp->tight = TRUE;
		else if (!str_prefix(arg, "fog"))
			mp->fog = TRUE;
		else if (!str_prefix(arg, "doors"))
			mp->doors = FALSE;
		else if (!str_prefix(arg, "mobs"))
			mp->mobs = TRUE;
		else if (!str_prefix(arg, "terrain"))
			mp->ter = TRUE;
		else if (!str_prefix(arg, "text"))
			mp->text = TRUE;
		
		arg[0] = '\0';
		argument = one_argument (argument, arg);
	} 
	if (!mp->tight && mp->size > 0) // It's not a tight map, so we double it's size to make up for the extra links
		mp->size *= 2;
	
	make_map (ch, x, y); // Create your map - most important part
	show_map (ch, x, y); // Use the show_map routine to display it - can create different routines to suit taste or need
        to_actor( "\n\r", ch );
	return;
}

// make the map.  Meat of the whole thing
void make_map (PLAYER *ch, int x, int y)
{
	// Lets start out with a fresh grid and hash table
	init_map();

	// Add your startin point
	map[x][y] = ch->in_scene->dbkey;
	// Say you've visited your startin point
	add_visited(ch->in_scene->dbkey);
	
	// Use your starting point to begin the graphing process, with you in the middle
	add_to_map (ch, ch->in_scene, x, y, 0);
	return;	
}

// get_?_start and get_?_to  functions are used to trim the map down so only the smallet, visible map is displayed
// returns their appropriate boundaries
int get_x_start (void)
{
	int y, x, size = mp->size;
	
	if (size < MAP_MID)
		return MAP_MID - size;
	else	
	{
		for (x = 0; x < MAX_MAP; x++)
		{
			for (y = 0; y < MAX_MAP; y++)
			{
				if (map[x][y] != -1)
					return x;
			}
		}
	}
	return MAX_MAP - 1;
}
// See above
int get_y_start (void)
{
	int y, x, size = mp->size;

	if (size < MAP_MID)
		return MAP_MID - size;
	else
	{
		for (y = 0; y < MAX_MAP; y++)
		{
			for (x = 0; x < MAX_MAP; x++)
			{
				if (map[x][y] != -1)
					return y;
			}
		}	
	}

	return MAX_MAP - 1;
}
// See above
int get_x_to (void)
{
	int y, x, size = mp->size;

        return 70;

	if (size < MAP_MID)
		return MAX_MAP - MAP_MID + size - 1;
	else
	{
		for (x = MAX_MAP - 1; x >= 0; x--)
		{
			for (y = 0; y < MAX_MAP; y++)
			{
				if (map[x][y] != -1)
					return x;
			}
		}
	}
	return 0;	
}
// See above
int get_y_to (void)
{
	int y, x, size = mp->size;

	if (size < MAP_MID)
		return MAX_MAP - MAP_MID + size - 1;
	else
	{
		for (y = MAX_MAP - 1; y >= 0; y--)
		{
			for (x = 0; x < MAX_MAP; x++)
			{
				if (map[x][y] != -1)
					return y;
			}
		}
	}
	
	return 0;
}

// The map display function
void show_map (PLAYER *ch, int xx, int yy)
{
	int x, x_to, x_from, y, y_to, y_from;
	char buf[MSL];
	
	// Get our trimming threashholds
	y_from = get_y_start();
	y_to = get_y_to();
	x_from = get_x_start();
	x_to = get_x_to();
	
	// Start out our buffer
	sprintf (buf, "%c", '\0');
	
	mp->col = TRUE;
	if (mp->border)  // draw top
	{
                to_actor( "     ", ch ); 
                                display_interp( ch, "^4" );
                                display_interp( ch, "^-" );
                to_actor( "+", ch );
                                display_interp( ch, "^0" );
		for (x = x_from; x <= x_to; x++)
			to_actor ("-", ch);
                                display_interp( ch, "^4" );
		to_actor ("+", ch);
                                display_interp( ch, "^N" );

		if (mp->text) {
                        to_actor( get_map_text(), ch );
                        to_actor( "\n\r", ch );
                    }
		else
			to_actor ("\n\r", ch);
	}

	for (y = y_from; y <= y_to; y++)		
	{
		for (x = x_from; x <= x_to; x++)
		{
			if (map[x][y] != -1) // Is there something here?
			{
				if (map[x][y] < -1) // ? < -1 means a link type.  Lets buffer it
                                      {
                                        if ( HAS_ANSI(ch) )
					sprintf (buf, "%s^-^0%s", buf, get_link(map[x][y]));
                                        else
					sprintf (buf, "%s%s", buf, get_link(map[x][y]));
                                      }
				else if (xx == x && yy == y) // This is where we started.  Make sure to mark it with a special character
				{
					if (mp->col == TRUE) {
                                        if ( HAS_ANSI(ch) ) 
						sprintf (buf, "%s^B^3@^N", buf);
                                             else
						sprintf (buf, "%s@", buf );
                                        } 
					else
					{
						mp->col = TRUE;
                                        if ( HAS_ANSI(ch) ) 
						sprintf (buf, "%s^B^3@^N", buf);
                                             else
						sprintf (buf, "%s@", buf );
					}
				}
				else // Must be a scene
				{
					sprintf (buf, "%s%s%s", buf, get_mscenec(ch, map[x][y]), get_mscene(ch, map[x][y]));
				}
			}
			else // Nothing's here..  Make a blank space
			{
				if (mp->col == TRUE)
				{
					mp->col = FALSE;
                                        if ( HAS_ANSI(ch) ) 
					sprintf (buf, "%s^N^- ", buf);
				}
				else
					sprintf (buf, "%s^N^- ", buf);
			}
		}
		
		if (mp->border) // draw left
		{
			if (mp->col == TRUE)
                             {
                                to_actor( "     ", ch );
                                display_interp( ch, "^-" );
                                display_interp( ch, "^0" );
                                to_actor( ">", ch );
                                to_actor( buf, ch );
                                display_interp( ch, "^N" );
                             }
			else
                             {
                                to_actor( "     ", ch );
                                display_interp( ch, "^-" );
                                display_interp( ch, "^0" );                              
                                to_actor( ">", ch );
                                to_actor( buf, ch );
                                display_interp( ch, "^N" );
                             }

			mp->col = TRUE;
		}
		else         {  to_actor( buf, ch );
                             }
		// draw right
		if (mp->text) { to_actor( get_map_text(), ch );
                                display_interp( ch, "^-" );
                                display_interp( ch, "^0" );
			to_actor ( mp->border ? "<" : "", ch );
                                display_interp( ch, "^N" );
                        to_actor( "\n\r", ch );
                             }
		else {
                                display_interp( ch, "^-" );
                                display_interp( ch, "^0" );
                        if ( !HAS_ANSI(ch) ) to_actor( " ", ch ); 
			to_actor ( mp->border ? "<" : "", ch );
                                display_interp( ch, "^N" );
                        to_actor( "\n\r", ch );
                     }
		
		sprintf (buf, "%c", '\0');
	}
	
	if (mp->border)  // draw bottom
        {
                to_actor ( "     ", ch );
                                display_interp( ch, "^-" );
                                display_interp( ch, "^4" );
                to_actor( "+", ch );
                                display_interp( ch, "^0" );
		for (x = x_from; x <= x_to; x++)
			to_actor ("-", ch);

                                display_interp( ch, "^N" );
		if (mp->text)
			{ to_actor ( get_map_text(), ch);
                          //to_actor ( "     ", ch );
                                display_interp( ch, "^-" );
                                display_interp( ch, "^4" );
                          to_actor( "+", ch );
                                display_interp( ch, "^N" );
                          to_actor( "\n\r", ch ); 
                        }
		else {
                                display_interp( ch, "^-" );
                                display_interp( ch, "^4" );
			to_actor ("+", ch);
                                display_interp( ch, "^N" );
                          to_actor( "\n\r", ch ); 
                     }

	}
	
	if (mp->text)
	{
                                display_interp( ch, "^N" );
           to_actor( finish_map_text(), ch );
           to_actor( "\n\r", ch );
	}
                                display_interp( ch, "^N" );
	// We're done, so free the visits ( I know, it's done twice, but it's safer this way )
	free_all_visits();
	return;			
}

// Uses a combination of a queue and recursion.  Takes you, does all the scenes around you
// After that, it does all those scenes, then the scenes those generated..  until it stops
void add_to_map (PLAYER *ch, SCENE *scene, int x, int y, int depth)
{
	EXIT *pexit;
	MAP_QD *qd, *next_qd;
	int dir, num;
	bool fog, tight, azone;
	int qscene, qx, qy, qdepth; // better memory management
	
	// Just to saved a few keystrokes
	fog = mp->fog;
	tight = mp->tight;
	azone = mp->zone;
	
	
	// Go in all 4 directions NESW, add the lines then add to map the next scene
	for (dir = 0; dir < 4; dir++)
	{
		if ((pexit = scene->exit[dir]) != NULL)
		{// We have an exit scene to search!

			// Add your conditions for not mapping that scene here.
			if (!pexit->to_scene // Can they get there normally?
				|| (mp->depth >= 0 && depth == mp->depth)
				|| ( 
                                     mp->doors == FALSE && (
                                                          IS_SET(pexit->exit_flags, EXIT_CLOSED) 
                                                       || IS_SET(pexit->exit_flags, EXIT_SECRET)
                                                       || IS_SET(pexit->exit_flags, EXIT_CONCEALED)
                                                       || ( scene_is_dark(pexit->to_scene) && !IS_AFFECTED(ch, BONUS_INFRARED) )
                                                           ) 
                                   ) 
                           ) 
				continue;
			
			switch (dir)
			{
				case DIR_NORTH: // y - 1
					if (!tight) // Do we want our scene links?
					{
						// Num is used to control if we do every second point as a scene, or every point
						num = 2;
						if (y - 2 < 0) // Will we oversteap our bounds if we put try to put in this link?
							break;
						else
							map[x][y-1] = put_link (scene, map[x][y-2], dir);
					}
					else
						num = 1;
					
					// Cases where we WON'T add this scene, and thereby search it
					if (y - num < 0
						|| map[x][y-num] != -1
						|| (scene->zone != pexit->to_scene->zone && !azone)
						|| has_visited(pexit->to_scene->dbkey))
						break;
					// It's passed the test, lets add it
					map[x][y-num] = pexit->to_scene->dbkey;
					qd = new_map_qd();
					qd->x = x;
					qd->y = y - num;
					qd->scene = pexit->to_scene->dbkey;
					qd->depth = depth + 1;
					add_map_qd(qd);					
					break;
					
				case DIR_EAST: // x + 1
					if (!tight) // See all comments above.
					{
						num = 2;
						if (x + 2 > MAX_MAP - 1)
							break;
						else
							map[x+1][y] = put_link (scene, map[x+2][y],  dir);
					}
					else
						num = 1;

					if (x + num > MAX_MAP - 1
						|| map[x+num][y] != -1
						|| (scene->zone != pexit->to_scene->zone && !azone)
						|| has_visited(pexit->to_scene->dbkey))
						break;
					map[x+num][y] = pexit->to_scene->dbkey;
					qd = new_map_qd();
					qd->x = x + num;
					qd->y = y;
					qd->scene = pexit->to_scene->dbkey;
					qd->depth = depth + 1;
					add_map_qd(qd);					
					break;
					
				case DIR_SOUTH: // y + 1
					if (!tight)
					{
						num = 2;
						if (y + 2 > MAX_MAP - 1)
							break;
						else
							map[x][y+1] = put_link (scene, map[x][y+2], dir);
					}
					else
						num = 1;
					
					if (y + num > MAX_MAP - 1
						|| map[x][y+num] != -1
						|| (scene->zone != pexit->to_scene->zone && !azone)
						|| has_visited(pexit->to_scene->dbkey))
						break;
					map[x][y+num] = pexit->to_scene->dbkey;
					qd = new_map_qd();
					qd->x = x;
					qd->y = y + num;
					qd->scene = pexit->to_scene->dbkey;
					qd->depth = depth + 1;
					add_map_qd(qd);					
					break;
					
				case DIR_WEST: // x - 1
					if (!tight)
					{
						num = 2;
						if (x - 2 < 0)
							break;
						else
							map[x-1][y] = put_link (scene, map[x-2][y], dir);
					}
					else
						num = 1;
					
					if (x - num < 0
						|| map[x-num][y] != -1
						|| (scene->zone != pexit->to_scene->zone && !azone)
						|| has_visited(pexit->to_scene->dbkey))
						break;
					map[x-num][y] = pexit->to_scene->dbkey;
					qd = new_map_qd();
					qd->x = x - num;
					qd->y = y;
					qd->scene = pexit->to_scene->dbkey;
					qd->depth = depth + 1;
					add_map_qd(qd);
					break;
				default:
					break;
			} // End of dir switch
		} // End of exit existing
	} // end of searching NESW from our point
	
	next_qd = next_map_qd(); // But wait!  Is there another scene in the queue to search?

	if (next_qd != NULL)
	{ // Looks like it.  Lets search this point just like we did when we first started this whole thing
		qscene = next_qd->scene;
		qx = next_qd->x;
		qy = next_qd->y;
		qdepth = next_qd->depth;
		// In this way I cn free the qdata before the recursive call, meaning it's available right away instead of after all the recursion is done.
		
		free_map_qd (next_qd);
		// Is it a valid scene?  Lets double check
		if (get_scene(qscene) != NULL)
			add_to_map (ch, get_scene(qscene), qx, qy, qdepth);
	}
	return;
}
