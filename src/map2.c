/************************************************************************/
/* mlkesl@stthomas.edu	=====>	Ascii Automapper utility		*/
/* Let me know if you use this. Give a newbie some _credit_,		*/
/* at least I'm not asking how to add classes...			*/
/* Also, if you fix something could ya send me mail about, thanks	*/
/* PLEASE mail me if you use this or like it, that way I will keep it up*/
/************************************************************************/
/* MapArea -> 	when given a room, ch, x, and y,...			*/
/*	   	this function will fill in values of map as it should 	*/
/* ShowMap -> 	will simply spit out the contents of map array		*/
/*		Would look much nicer if you built your own areas	*/
/*		without all of the overlapping stock Rom has		*/
/* do_map  ->	core function, takes map size as argument		*/
/************************************************************************/
/* To install:: 							*/
/*	remove all occurances of "" (unless your exits are unioned)	*/
/*	add do_map prototypes to interp.c and merc.h (or interp.h)	*/
/*	customize the first switch with your own moves (add road :)	*/
/*	remove the color codes or change to suit your own color coding	*/
/* Other stuff::							*/
/*	make a skill, call from do_move (only if ch is not in city etc) */
/*	allow players to actually make ITEM_MAP objects			*/
/* 	change your areas to make them more suited to map code! :)	*/
/* 	I WILL be making MANY enhancements as they come but a lot of	*/
/*	  people want the code, and want it now :p			*/
/*	Kermit's page also has this snippet, mail me for any q's though */
/************************************************************************/ 	
/* mlk update: map is now a 2 dimensional array of integers		*/
/*	uses MOVE_MAX for null					*/
/*	uses MOVE_MAX+1 for mazes or one ways to MOVE_ENTER		*/
/*	use the SECTOR numbers to represent move types :)		*/
/************************************************************************/ 	
#include <ctype.h>  /* for isalpha */
#include <string.h>
#include <stdlib.h>
#include <stdio.h> 
#include <time.h> 
#include "merc.h"
#include "olc.h"

#define MAX_MAP 160
#define MAX_MAP_DIR 4

int map[MAX_MAP][MAX_MAP];
int offsets[4][2] ={ {-1, 0},{ 0, 1},{ 1, 0},{ 0,-1} };

void MapArea 
(ROOM_INDEX *room, CHAR *ch, int x, int y, int min, int max)
{
ROOM_INDEX *prospect_room;
EXIT *pexit;
int door;

/* marks the room as visited */
map[x][y]=room->move;

    for ( door = 0; door < MAX_MAP_DIR; door++ ) 
    { /* cycles through for each exit */
	if (
             (pexit = room->exit[door]) != NULL
	     &&   pexit->to_room != NULL 
	     &&   can_see_room(ch,pexit->to_room)  /* optional */
	     &&   !IS_SET(pexit->exit_flags, EXIT_CLOSED)
           )
        { /* if exit there */

        if ((x<min)||(y<min)||(x>max)||(y>max)) return;

	prospect_room = pexit->to_room;

        if ( prospect_room->exit[rev_dir[door]] &&
	 prospect_room->exit[rev_dir[door]]->to_room!=room)
		{ /* if not two way */
			map[x][y]=MOVE_MAX+1; /* one way into area OR maze */	
		return;
		} /* end two way */

/*    if ( NPC(ch)
	||(!IS_SET(ch->flag,PLR_HOLYLIGHT))
	||(!IS_IMMORTAL(ch))
	) */
      if( (prospect_room->move==MOVE_ROCK_MOUNTAIN) 
	||(prospect_room->move==MOVE_SNOW_MOUNTAIN) 
	||(prospect_room->move==MOVE_HILLS) 
	||(prospect_room->move==MOVE_CITY) 
	||(prospect_room->move==MOVE_INSIDE) 
	||(prospect_room->move==MOVE_ENTER) 
        )
	{ /* players cant see past these */
		map[x+offsets[door][0]][y+offsets[door][1]]=prospect_room->move;
		/* ^--two way into area */		
	/*if (door!=0) return;*//*not needed for my mud	becuz of euclid world map*/
	/* ^--takes care of stopping when north is one of moves you cant see past */
	}

    if (map[x+offsets[door][0]][y+offsets[door][1]]==MOVE_MAX) {
                MapArea (pexit->to_room,ch,
                    x+offsets[door][0], y+offsets[door][1],min,max);
    }

	} /* end if exit there */
    }
return;
}

/* mlk :: shows a map, specified by size */
void ShowMap( CHAR *ch, int min, int max)
{
int x,y;

    for (x = min; x < max; ++x) 
    { /* every row */
         for (y = min; y < max; ++y)
         { /* every column */
if ( (y==min) || (map[x][y-1]!=map[x][y]) )
switch(map[x][y])
{
case MOVE_MAX:		send_to_char(" ",ch);		break;
case MOVE_FOREST:	send_to_char("{g@",ch);		break;
case MOVE_FIELD:	send_to_char("{G\"",ch);	break;
case MOVE_HILLS:	send_to_char("{G^",ch);		break;
case MOVE_ROAD:		send_to_char("{m+",ch);		break;
case MOVE_MOUNTAIN:	send_to_char("{y^",ch);		break;
case MOVE_WATER_SWIM:	send_to_char("{B:",ch);		break;
case MOVE_WATER_NOSWIM:	send_to_char("{b:",ch);		break;
case MOVE_UNUSED:	send_to_char("{DX",ch);		break;
case MOVE_AIR:		send_to_char("{C%",ch);		break;
case MOVE_DESERT:	send_to_char("{Y=",ch);		break;
case MOVE_ENTER:	send_to_char("{W@",ch);		break;
case MOVE_INSIDE:	send_to_char("{W%",ch);		break;
case MOVE_CITY:		send_to_char("{W#",ch);		break;
case MOVE_ROCK_MOUNTAIN:send_to_char("{D^",ch);		break;
case MOVE_SNOW_MOUNTAIN:send_to_char("{W^",ch);		break;
case MOVE_SWAMP:	send_to_char("{D&",ch);		break;
case MOVE_JUNGLE:	send_to_char("{g&",ch);		break;
case MOVE_RUINS:	send_to_char("{D#",ch);		break;
case (MOVE_MAX+1):	send_to_char("{D?",ch);		break;
default: 		send_to_char("{R*",ch);
} /* end switch1 */
else 
switch(map[x][y])
{
case MOVE_MAX:		send_to_char(" ",ch);		break;
case MOVE_FOREST:	send_to_char("@",ch);		break;
case MOVE_FIELD:	send_to_char("\"",ch);		break;
case MOVE_HILLS:	send_to_char("^",ch);		break;
case MOVE_ROAD:		send_to_char("+",ch);		break;
case MOVE_MOUNTAIN:	send_to_char("^",ch);		break;
case MOVE_WATER_SWIM:	send_to_char(":",ch);		break;
case MOVE_WATER_NOSWIM:	send_to_char(":",ch);		break;
case MOVE_UNUSED:	send_to_char("X",ch);		break;
case MOVE_AIR:		send_to_char("%",ch);		break;
case MOVE_DESERT:	send_to_char("=",ch);		break;
case MOVE_ENTER:	send_to_char("@",ch);		break;
case MOVE_INSIDE:	send_to_char("%",ch);		break;
case MOVE_CITY:		send_to_char("#",ch);		break;
case MOVE_ROCK_MOUNTAIN:send_to_char("^",ch);		break;
case MOVE_SNOW_MOUNTAIN:send_to_char("^",ch);		break;
case MOVE_SWAMP:	send_to_char("&",ch);		break;
case MOVE_JUNGLE:	send_to_char("&",ch);		break;
case MOVE_RUINS:	send_to_char("#",ch);		break;
case (MOVE_MAX+1):	send_to_char("?",ch);		break;
default: 		send_to_char("*",ch);
} /* end switch2 */
         } /* every column */

send_to_char("\n\r",ch); 
} /*every row*/   

return;
}

/* mlk :: shows map compacted, specified by size */
void ShowHalfMap( CHAR *ch, int min, int max)
{
int x,y;

    for (x = min; x < max; x+=2) 
    { /* every row */
         for (y = min; y < max; y+=2)
         { /* every column */

/* mlk prioritizes*/
if (map[x][y-1]==MOVE_ROAD) map[x][y]=MOVE_ROAD;
if (map[x][y-1]==MOVE_ENTER) map[x][y]=MOVE_ENTER;

if ( (y==min) || (map[x][y-2]!=map[x][y]) )
switch(map[x][y])
{
case MOVE_MAX:		send_to_char(" ",ch);		break;
case MOVE_FOREST:	send_to_char("{g@",ch);		break;
case MOVE_FIELD:	send_to_char("{G\"",ch);	break;
case MOVE_HILLS:	send_to_char("{G^",ch);		break;
case MOVE_ROAD:		send_to_char("{m+",ch);		break;
case MOVE_MOUNTAIN:	send_to_char("{y^",ch);		break;
case MOVE_WATER_SWIM:	send_to_char("{B:",ch);		break;
case MOVE_WATER_NOSWIM:	send_to_char("{b:",ch);		break;
case MOVE_UNUSED:	send_to_char("{DX",ch);		break;
case MOVE_AIR:		send_to_char("{C%",ch);		break;
case MOVE_DESERT:	send_to_char("{Y=",ch);		break;
case MOVE_ENTER:	send_to_char("{W@",ch);		break;
case MOVE_INSIDE:	send_to_char("{W%",ch);		break;
case MOVE_CITY:		send_to_char("{W#",ch);		break;
case MOVE_ROCK_MOUNTAIN:send_to_char("{D^",ch);		break;
case MOVE_SNOW_MOUNTAIN:send_to_char("{W^",ch);		break;
case MOVE_SWAMP:	send_to_char("{D&",ch);		break;
case MOVE_JUNGLE:	send_to_char("{g&",ch);		break;
case MOVE_RUINS:	send_to_char("{D#",ch);		break;
case (MOVE_MAX+1):	send_to_char("{D?",ch);		break;
default: 		send_to_char("{R*",ch);
} /* end switch1 */
else 
switch(map[x][y])
{
case MOVE_MAX:		send_to_char(" ",ch);		break;
case MOVE_FOREST:	send_to_char("@",ch);		break;
case MOVE_FIELD:	send_to_char("\"",ch);		break;
case MOVE_HILLS:	send_to_char("^",ch);		break;
case MOVE_ROAD:		send_to_char("+",ch);		break;
case MOVE_MOUNTAIN:	send_to_char("^",ch);		break;
case MOVE_WATER_SWIM:	send_to_char(":",ch);		break;
case MOVE_WATER_NOSWIM:	send_to_char(":",ch);		break;
case MOVE_UNUSED:	send_to_char("X",ch);		break;
case MOVE_AIR:		send_to_char("%",ch);		break;
case MOVE_DESERT:	send_to_char("=",ch);		break;
case MOVE_ENTER:	send_to_char("@",ch);		break;
case MOVE_INSIDE:	send_to_char("%",ch);		break;
case MOVE_CITY:		send_to_char("#",ch);		break;
case MOVE_ROCK_MOUNTAIN:send_to_char("^",ch);		break;
case MOVE_SNOW_MOUNTAIN:send_to_char("^",ch);		break;
case MOVE_SWAMP:	send_to_char("&",ch);		break;
case MOVE_JUNGLE:	send_to_char("&",ch);		break;
case MOVE_RUINS:	send_to_char("#",ch);		break;
case (MOVE_MAX+1):	send_to_char("?",ch);		break;
default: 		send_to_char("*",ch);
} /* end switch2 */
         } /* every column */

send_to_char("\n\r",ch); 
} /*every row*/   

return;
}

/*printing function*/
void do_printmap(CHAR *ch, char *argument)
{
int x,y,min=-1,max=MAX_MAP-1;
FILE *fp;

if (strcmp(ch->name,"Kroudar"))
{
send_to_char("Curiousity is a disease...",ch);
return;
}

for (x = 0; x < MAX_MAP; ++x)
        for (y = 0; y < MAX_MAP; ++y)
                  map[x][y]=MOVE_MAX;

MapArea(ch->in_room, ch, 80, 80, min, max); 

fclose (fpReserve);                        
                                                
fp = fopen("WILDERNESS_MAP","w");
    for (x = min; x < max; ++x) 
    { /* every row */
         for (y = min; y < max; ++y)
         { /* every column */
switch(map[x][y])
{
case MOVE_MAX:		fprintf(fp," ");		break;
case MOVE_FOREST:	fprintf(fp,"@");		break;
case MOVE_FIELD:	fprintf(fp,"\"");		break;
case MOVE_HILLS:	fprintf(fp,"^");		break;
case MOVE_ROAD:		fprintf(fp,"+");		break;
case MOVE_MOUNTAIN:	fprintf(fp,"^");		break;
case MOVE_WATER_SWIM:	fprintf(fp,":");		break;
case MOVE_WATER_NOSWIM:	fprintf(fp,":");		break;
case MOVE_UNUSED:	fprintf(fp,"X");		break;
case MOVE_AIR:		fprintf(fp,"~");		break;
case MOVE_DESERT:	fprintf(fp,"=");		break;
case MOVE_ENTER:	fprintf(fp,"$");		break;
case MOVE_INSIDE:	fprintf(fp,"I");		break;
case MOVE_CITY:		fprintf(fp,"#");		break;
case MOVE_ROCK_MOUNTAIN:fprintf(fp,"^");		break;
case MOVE_SNOW_MOUNTAIN:fprintf(fp,"^");		break;
case MOVE_SWAMP:	fprintf(fp,"&");		break;
case MOVE_JUNGLE:	fprintf(fp,"&");		break;
case MOVE_RUINS:	fprintf(fp,"#");		break;
case (MOVE_MAX+1):	fprintf(fp,"?");		break;
default: 		fprintf(fp,"*");
} /* end switch2 */
         } /* every column */

fprintf(fp,"\n"); 
} /*every row*/   

fclose(fp);
fpReserve = fopen( NULL_FILE, "r" );

return;
}

/* will put a small map with current room desc and title */
void ShowRoom( CHAR *ch, int min, int max)
{
int x,y,str_pos=0,desc_pos=0,start;
char buf[500];
char desc[500];
char line[100];

strcpy(desc,ch->in_room->description);

map[min][min]=MOVE_MAX;map[max-1][max-1]=MOVE_MAX; /* mlk :: rounds edges */
map[min][max-1]=MOVE_MAX;map[max-1][min]=MOVE_MAX;

    for (x = min; x < max; ++x) 
    { /* every row */
         for (y = min; y < max; ++y)
         { /* every column */
if ( (y==min) || (map[x][y-1]!=map[x][y]) )
switch(map[x][y])
{
case MOVE_MAX:		send_to_char(" ",ch);		break;
case MOVE_FOREST:	send_to_char("{g@",ch);		break;
case MOVE_FIELD:	send_to_char("{G\"",ch);	break;
case MOVE_HILLS:	send_to_char("{G^",ch);		break;
case MOVE_ROAD:		send_to_char("{m+",ch);		break;
case MOVE_MOUNTAIN:	send_to_char("{y^",ch);		break;
case MOVE_WATER_SWIM:	send_to_char("{B:",ch);		break;
case MOVE_WATER_NOSWIM:	send_to_char("{b:",ch);		break;
case MOVE_UNUSED:	send_to_char("{DX",ch);		break;
case MOVE_AIR:		send_to_char("{C%",ch);		break;
case MOVE_DESERT:	send_to_char("{Y=",ch);		break;
case MOVE_ENTER:	send_to_char("{W@",ch);		break;
case MOVE_INSIDE:	send_to_char("{W%",ch);		break;
case MOVE_CITY:		send_to_char("{W#",ch);		break;
case MOVE_ROCK_MOUNTAIN:send_to_char("{D^",ch);		break;
case MOVE_SNOW_MOUNTAIN:send_to_char("{W^",ch);		break;
case MOVE_SWAMP:	send_to_char("{D&",ch);		break;
case MOVE_JUNGLE:	send_to_char("{g&",ch);		break;
case MOVE_RUINS:	send_to_char("{D#",ch);		break;
case (MOVE_MAX+1):	send_to_char("{D?",ch);		break;
default: 		send_to_char("{R*",ch);
} /* end switch1 */
else 
switch(map[x][y])
{
case MOVE_MAX:		send_to_char(" ",ch);		break;
case MOVE_FOREST:	send_to_char("@",ch);		break;
case MOVE_FIELD:	send_to_char("\"",ch);		break;
case MOVE_HILLS:	send_to_char("^",ch);		break;
case MOVE_ROAD:		send_to_char("+",ch);		break;
case MOVE_MOUNTAIN:	send_to_char("^",ch);		break;
case MOVE_WATER_SWIM:	send_to_char(":",ch);		break;
case MOVE_WATER_NOSWIM:	send_to_char(":",ch);		break;
case MOVE_UNUSED:	send_to_char("X",ch);		break;
case MOVE_AIR:		send_to_char("%",ch);		break;
case MOVE_DESERT:	send_to_char("=",ch);		break;
case MOVE_ENTER:	send_to_char("@",ch);		break;
case MOVE_INSIDE:	send_to_char("%",ch);		break;
case MOVE_CITY:		send_to_char("#",ch);		break;
case MOVE_ROCK_MOUNTAIN:send_to_char("^",ch);		break;
case MOVE_SNOW_MOUNTAIN:send_to_char("^",ch);		break;
case MOVE_SWAMP:	send_to_char("&",ch);		break;
case MOVE_JUNGLE:	send_to_char("&",ch);		break;
case MOVE_RUINS:	send_to_char("#",ch);		break;
case (MOVE_MAX+1):	send_to_char("?",ch);		break;
default: 		send_to_char("*",ch);
} /* end switch2 */
         } /* every column */

if (x==min) {
	sprintf(buf,"{x   {C%s{x",ch->in_room->name);
	send_to_char(buf,ch);

/* mlk :: no brief in wilderness, ascii map is automatic, autoexits off too
        if (  !NPC(ch) && IS_SET(ch->flag, PLR_AUTOEXIT)  ) { 
        send_to_char( " {b",ch);do_exits( ch, "auto" );send_to_char( "{x",ch);
        } */

       		if (IS_IMMORTAL(ch) && (NPC(ch) || IS_SET(ch->flag,PLR_HOLYLIGHT)))
       		{ /* for showing certain people room dbkey */
           	sprintf(buf," {c[Room %d]{x",ch->in_room->dbkey);
           	send_to_char(buf,ch);
       		}
}
else
{
start=str_pos;
for (desc_pos = desc_pos ; desc[desc_pos]!='\0' ; desc_pos++)
{ 
   if (desc[desc_pos]=='\n') {
	line[str_pos-start]='\0';str_pos+=3;
	desc_pos += 2;
	break;
	}
   else if (desc[desc_pos]=='\r') {
	line[str_pos-start]='\0';str_pos+=2;
	break;
	}
   else {
	line[str_pos-start]=desc[desc_pos];
	str_pos+=1;
	}
}
/* set the final character to \0 for descs that are longer than 5 lines? */
line[str_pos-start]='\0'; /* best way to clear string? */
/* maybe do strcpy(line,"  ");  instead? */
/* not needed here because the for loops stop at \0 ?? */
if (x==min+1) send_to_char("  ",ch);
send_to_char("   {c",ch);
send_to_char(line,ch);
send_to_char("{x",ch);

} /*else*/

send_to_char("\n\r",ch); 
} /*every row*/   

send_to_char("\n\r",ch);  /* puts a line between contents/people */
return;
}

void cmd_map( CHAR *ch, char *argument )
{
int size,center,x,y,min,max;
char arg1[10];

   if ( !IS_SET(ch->flag2,PLR_NAGE) ) 
  { return; send_to_char( "Ascii renderings are disabled.\n\r", ch ); }

   one_argument( argument, arg1 );
   size = atoi (arg1);

size=URANGE(6,size,MAX_MAP);
center=MAX_MAP/2;

min = MAX_MAP/2-size/2;
max = MAX_MAP/2+size/2;

for (x = 0; x < MAX_MAP; ++x)
        for (y = 0; y < MAX_MAP; ++y)
                  map[x][y]=MOVE_MAX;

/* starts the mapping with the center room */
MapArea(ch->in_room, ch, center, center, min-1, max-1); 

/* marks the center, where ch is */
map[center][center]=MOVE_MAX+2; /* can be any number above MOVE_MAX+1 	*/
				/* switch default will print out the *	*/

if (   (!IS_IMMORTAL(ch))||(NPC(ch))   )
{
  if (  !IS_SET(ch->in_room->room_flags, ROOM_WILDERNESS)   )
     {send_to_char("{CYou can not do that here{x.\n\r",ch);return;}
  if ( room_is_dark( ch->in_room ,ch) ) 
     {send_to_char( "{bThe wilderness is pitch black at night... {x\n\r", ch );return;}
  else   
     {ShowRoom(ch,MAX_MAP/2-3,MAX_MAP/2+3);return;}
}
/* mortals not in city, enter or inside will always get a ShowRoom */
if (IS_IMMORTAL(ch)) {
   if (arg1[0]=='\0') ShowRoom (ch, min, max); 
   else ShowMap (ch, min, max); 
   return;
   }

send_to_char("{CHuh?{x\n\r",ch);
return;
}

void cmd_smallmap( CHAR *ch, char *argument )
{
int size,center,x,y,min,max;
char arg1[10];

   one_argument( argument, arg1 );
   size = atoi (arg1);

size=URANGE(6,size,MAX_MAP);
center=MAX_MAP/2;

min = MAX_MAP/2-size/2;
max = MAX_MAP/2+size/2;

for (x = 0; x < MAX_MAP; ++x)
        for (y = 0; y < MAX_MAP; ++y)
                  map[x][y]=MOVE_MAX;

/* starts the mapping with the center room */
MapArea(ch->in_room, ch, center, center, min-1, max-1); 

/* marks the center, where ch is */
map[center][center]=MOVE_MAX+2; /* can be any number above MOVE_MAX+1 	*/
				/* switch default will print out the *	*/

if (IS_IMMORTAL(ch)) {
   ShowHalfMap (ch, min, max); 
   return;
   }

send_to_char("{CHuh?{x\n\r",ch);
return;
}

/* mlk :: pass it (SECTOR_XXX,"") and get back the move ascii 
 	  in a roleplaying format of course, not mountain_snow etc */
char *get_move_name(int move)
{
char *move_name;

move_name="movement";

switch (move)
{
case MOVE_FOREST:	move_name="some trees";break; 
case MOVE_FIELD:	move_name="a field";break;
case MOVE_HILLS:	move_name="some rolling hills";break;
case MOVE_ROAD:		move_name="a road";break;
case MOVE_WATER_SWIM:	move_name="shallow water";break;
case MOVE_WATER_NOSWIM:	move_name="deep water";break;
case MOVE_AIR:		move_name="the sky";break;
case MOVE_DESERT:	move_name="a lot of sand";break;
case MOVE_MOUNTAIN:	move_name="some mountainous terrain";break;
case MOVE_ROCK_MOUNTAIN:move_name="a rocky mountain";break;
case MOVE_SNOW_MOUNTAIN:move_name="snow covered mountains";break;
case MOVE_SWAMP:	move_name="bleak swampland";break;
case MOVE_JUNGLE:	move_name="thick jungle";break;
case MOVE_RUINS:	move_name="ruins of some sort";break;
case MOVE_INSIDE:	move_name="movement";break; 
case MOVE_CITY:		move_name="movement";break; 
case MOVE_ENTER:	move_name="movement";break; 
} /*switch1*/

return(strdup(move_name));
}

/* mlk ::
	when given (int array[5]) with dbkey in [0], it will return
1	north move, 
2	east move, 
3	south move,
4	east_move &
5	number of exits leading to rooms of the same move
 */
int *get_exit_moves(int *exit_moves)
{
ROOM_INDEX *room;
EXIT *pexit;
int door;

room=get_room_index(exit_moves[0]);
exit_moves[4]=0;

    for ( door = 0; door < MAX_MAP_DIR; door++ ) 
    { /* cycles through for each exit */
	if (
             (pexit = room->exit[door]) != NULL
	     &&   pexit->to_room != NULL
	   )
	{   
	   exit_moves[door]=pexit->to_room->move;	
	   if ((pexit->to_room->move)==(room->move))
		exit_moves[4]+=1;
	} 
	else
	   exit_moves[door]=-1;	
    }	   

return(exit_moves);
}

/* will assign default values for NAME and DESC of wilderness */
void set_wilderness(CHAR *ch, char *argument)
{
char arg1[10],name[50],desc[300],buf[MAX_STRING_LENGTH];
ROOM_INDEX *room;
int dbkey,exit[5],exitsum;
int CHANCE=10;


/*if (strcmp(ch->name, "Kroudar") ) {
   send_to_char("There are few beings who can do that.\n\r",ch);return;
}*/

   one_argument( argument, arg1 );
   dbkey = atoi (arg1);

if ( argument[0] == '\0' ) { /* for immortal command */
dbkey=(ch->in_room->dbkey);
}    

room=get_room_index(dbkey); 

if (!IS_BUILDER(ch,room->area))
{
send_to_char("Wset: Insufficient Security",ch);
return;
}	

    if (  !IS_SET(room->room_flags, ROOM_WILDERNESS)   )
    	return; /* for NON wilderness */

exit[0]=dbkey;
get_exit_moves(exit);
exitsum=exit[4];
switch (room->move)
{
case MOVE_FOREST:	

strcpy(name,"In a Forest");
strcpy(desc,"You are in a forest teeming with life.  All around are the sure signs of nature.");
  if (exitsum==4) { 
    strcpy(name,"Within a Forest");
    strcpy(desc,"You are deep within in a forest, surrounded on all sides by trees. Dead leaves blanket the ground and the lush canopy prevents much light from getting though.");
  } 
  if
(((exit[0]!=(room->move))&&(number_percent()<CHANCE))||(exit[0]==MOVE_ROAD)||(exit[0]==MOVE_RUINS)){
    sprintf(buf,"You see %s to the north.",get_move_name(exit[0]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if
(((exit[1]!=(room->move))&&(number_percent()<CHANCE))||(exit[1]==MOVE_ROAD)||(exit[1]==MOVE_RUINS)){
    sprintf(buf,"To the east you can see %s.",get_move_name(exit[1]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[2]!=(room->move))&&(number_percent()<CHANCE))||(exit[2]==MOVE_ROAD)||(exit[2]==MOVE_RUINS)){
    sprintf(buf,"South of you, you can see %s.",get_move_name(exit[2]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[3]!=(room->move))&&(number_percent()<CHANCE))||(exit[3]==MOVE_ROAD)||(exit[3]==MOVE_RUINS)){
    sprintf(buf,"%s can be seen to the west.",get_move_name(exit[3]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
break;

case MOVE_FIELD:
	
strcpy(name,"In a Field");
strcpy(desc,"You are in a field of grass. Prairie flora and shrubs are \
scattered throughout adding to the inviting aroma.");
  if (exitsum==4) { 
    strcpy(name,"On the Plains");
    strcpy(desc,"You are on the plains. All around is grass, but not just \
any grass. Out here it comes in many shapes, sizes, and even colors. \
Insects fly through the air in occasional swarms.");
  }
  if (((exit[0]!=(room->move))&&(number_percent()<CHANCE))||(exit[0]==MOVE_ROAD)||(exit[0]==MOVE_RUINS)){
    sprintf(buf,"You see %s to the north.",get_move_name(exit[0]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[1]!=(room->move))&&(number_percent()<CHANCE))||(exit[1]==MOVE_ROAD)||(exit[1]==MOVE_RUINS)){
    sprintf(buf,"To the east you can see %s.",get_move_name(exit[1]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[2]!=(room->move))&&(number_percent()<CHANCE))||(exit[2]==MOVE_ROAD)||(exit[2]==MOVE_RUINS)){
    sprintf(buf,"South of you, you can see %s.",get_move_name(exit[2]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[3]!=(room->move))&&(number_percent()<CHANCE))||(exit[3]==MOVE_ROAD)||(exit[3]==MOVE_RUINS)){
    sprintf(buf,"%s can be seen to the west.",get_move_name(exit[3]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
break;

case MOVE_HILLS:	
strcpy(name,"On a Hill");
strcpy(desc,"You are on a large and rather high hill. You can see \
much of the surrounding area in full now more clearly.");
  if (exitsum==4) { 
    strcpy(name,"In the Foothills");
    strcpy(desc,"You are within the highlands. \
All that surrounds is much of the same. Hills and more hills, rolling \
across the land. Definitely not the best route of travel. \
");
  }
  if
(((exit[0]!=(room->move))&&(number_percent()<CHANCE+20))||(exit[0]==MOVE_ROAD)||(exit[0]==MOVE_RUINS)){
    sprintf(buf,"You see %s to the north.",get_move_name(exit[0]));
    strcat(desc,buf);strcpy(buf,"  ");
  }

if(((exit[1]!=(room->move))&&(number_percent()<CHANCE+20))||(exit[1]==MOVE_ROAD)||(exit[1]==MOVE_RUINS)){
    sprintf(buf,"To the east you can see %s.",get_move_name(exit[1]));
    strcat(desc,buf);strcpy(buf,"  ");
  }

if(((exit[2]!=(room->move))&&(number_percent()<CHANCE+20))||(exit[2]==MOVE_ROAD)||(exit[2]==MOVE_RUINS)){
    sprintf(buf,"South of you, you can see %s.",get_move_name(exit[2]));
    strcat(desc,buf);strcpy(buf,"  ");
  }

if(((exit[3]!=(room->move))&&(number_percent()<CHANCE+20))||(exit[3]==MOVE_ROAD)||(exit[3]==MOVE_RUINS)){
    sprintf(buf,"%s can be seen to the west.",get_move_name(exit[3]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
break;

case MOVE_ROAD:		
strcpy(name,"A Road");
strcpy(desc,"You are on a dirt road. Being an easier and much safer \
way to travel, many would not think of leaving the safety of the road. \
Hence it is somewhat highly trafficked. \
");
  if (exitsum==4) { 
    strcpy(name,"At a Four Way");
    strcpy(desc,"\
You are at a four way on the road. There are roads in every direction. \
The dirt road is trod down pretty well here and equally so in each \
direction. \
");
  }
  else if (exitsum==3) {
    strcpy(name,"At a Fork in the Road");
    strcpy(desc,"You are at a fork in the road, there are two paths to \
choose from and no indication whatsoever which would be best. \
"); 
}
  else if (exitsum==2) {
strcpy(name,"A Road");
strcpy(desc,"You are on a dirt road. Being an easier and much safer \
way to travel, many would not think of leaving the safety of the road. \
");
if (exit[0]==MOVE_ROAD) {
    sprintf(buf,"The road continues north and ");
    strcat(desc,buf);strcpy(buf,"  "); 
	if (exit[1]==MOVE_ROAD) {
	    sprintf(buf,"east.");
	    strcat(desc,buf);strcpy(buf,"  ");break; 	
	}
	if (exit[2]==MOVE_ROAD) {
	    sprintf(buf,"south.");
	    strcat(desc,buf);strcpy(buf,"  ");break;
	}
	if (exit[3]==MOVE_ROAD) {
	    sprintf(buf,"west.");
	    strcat(desc,buf);strcpy(buf,"  ");break;
	}
}
if (exit[1]==MOVE_ROAD) {
    sprintf(buf,"The road continues east and ");
    strcat(desc,buf);strcpy(buf,"  "); 
	if (exit[0]==MOVE_ROAD) {
	    sprintf(buf,"north.");
	    strcat(desc,buf);strcpy(buf,"  ");break; 	
	}
	if (exit[2]==MOVE_ROAD) {
	    sprintf(buf,"south.");

	    strcat(desc,buf);strcpy(buf,"  ");break;
	}
	if (exit[3]==MOVE_ROAD) {
	    sprintf(buf,"west.");
	    strcat(desc,buf);strcpy(buf,"  ");break;
	}
}
if (exit[2]==MOVE_ROAD) {
    sprintf(buf,"The road continues south and ");
    strcat(desc,buf);strcpy(buf,"  "); 
	if (exit[0]==MOVE_ROAD) {
	    sprintf(buf,"north.");
	    strcat(desc,buf);strcpy(buf,"  ");break; 	
	}
	if (exit[1]==MOVE_ROAD) {
	    sprintf(buf,"east.");
	    strcat(desc,buf);strcpy(buf,"  ");break;
	}
	if (exit[3]==MOVE_ROAD) {
	    sprintf(buf,"west.");
	    strcat(desc,buf);strcpy(buf,"  ");break;
	}
}
if (exit[3]==MOVE_ROAD) {
    sprintf(buf,"The road continues west and ");
    strcat(desc,buf);strcpy(buf,"  "); 
	if (exit[1]==MOVE_ROAD) {
	    sprintf(buf,"east.");
	    strcat(desc,buf);strcpy(buf,"  ");break; 	
	}
	if (exit[2]==MOVE_ROAD) {
	    sprintf(buf,"south.");
	    strcat(desc,buf);strcpy(buf,"  ");break;
	}
	if (exit[0]==MOVE_ROAD) {
	    sprintf(buf,"north.");
	    strcat(desc,buf);strcpy(buf,"  ");break;
	}
} /* if exit[3]=MOVE_ROAD */
} /* if exitsum==2 */

  if
(((exit[0]!=(room->move))&&(number_percent()<CHANCE-20))||(exit[0]==MOVE_RUINS)||(exit[0]!=MOVE_ROAD)) {
    sprintf(buf,"You see %s to the north.",get_move_name(exit[0]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if
(((exit[1]!=(room->move))&&(number_percent()<CHANCE-20))||(exit[1]==MOVE_RUINS)||(exit[1]!=MOVE_ROAD)){
    sprintf(buf,"To the east you can see %s.",get_move_name(exit[1]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if
(((exit[2]!=(room->move))&&(number_percent()<CHANCE-20))||(exit[2]==MOVE_RUINS)||(exit[2]!=MOVE_ROAD)){
    sprintf(buf,"South of you, you can see %s.",get_move_name(exit[2]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if
(((exit[3]!=(room->move))&&(number_percent()<CHANCE-20))||(exit[3]==MOVE_RUINS)||(exit[3]!=MOVE_ROAD)){
    sprintf(buf,"%s can be seen to the west.",get_move_name(exit[3]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
break;

case MOVE_WATER_SWIM:	
strcpy(name,"Shallow Water");
strcpy(desc,"The water here is rather shallow. You can see many little fish in the water, speeding to and fro in schools.");
  if (exitsum==4) { 
    strcpy(name,"Shallow Basin");
    strcpy(desc,"You are surrounded by shallow water. All around in the water affluent marine life comes through, giving quite a show with the occasional leap above the waters surface. "); 
  }
  if (exitsum==2) { 
    strcpy(name,"Shallow Water");
    strcpy(desc,"You are on shallow water. All around in the water affluent marine life comes through, giving quite a show with the occasional leap above the waters surface. ");
  }
  if (((exit[0]!=(room->move))&&(number_percent()<CHANCE))||(exit[0]==MOVE_ROAD)||(exit[0]==MOVE_RUINS)){
    sprintf(buf,"You see %s to the north.",get_move_name(exit[0]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[1]!=(room->move))&&(number_percent()<CHANCE))||(exit[1]==MOVE_ROAD)||(exit[1]==MOVE_RUINS)){
    sprintf(buf,"To the east you can see %s.",get_move_name(exit[1]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[2]!=(room->move))&&(number_percent()<CHANCE))||(exit[2]==MOVE_ROAD)||(exit[2]==MOVE_RUINS)){
    sprintf(buf,"South of you, you can see %s.",get_move_name(exit[2]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[3]!=(room->move))&&(number_percent()<CHANCE))||(exit[3]==MOVE_ROAD)||(exit[3]==MOVE_RUINS)){
    sprintf(buf,"%s can be seen to the west.",get_move_name(exit[3]));
    strcat(desc,buf);strcpy(buf,"  ");
  }

break;

case MOVE_WATER_NOSWIM:	
strcpy(name,"Deep Water");
strcpy(desc,"The water here is of unknown depth and fairly dark.\
The currents here are strong and the waves equally striking.\
");
  if (exitsum==4) { 
    strcpy(name,"On a Sea");
    strcpy(desc,"You are on a dark sea, surrounded completely by more of the same. \
Denizens of this dark water occasionaly make themselves seen \
out of curiousity, or perhaps necessity. \
");
  }
  if (exitsum==2) { 
    strcpy(name,"On Deep Water");
    strcpy(desc,"The water is is rather dark and \
not terribly temperate. However, it seems very clean.  \
");
  }
  if (((exit[0]!=(room->move))&&(number_percent()<CHANCE))||(exit[0]==MOVE_ROAD)||(exit[0]==MOVE_RUINS)){
    sprintf(buf,"You see %s to the north.",get_move_name(exit[0]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[1]!=(room->move))&&(number_percent()<CHANCE))||(exit[1]==MOVE_ROAD)||(exit[1]==MOVE_RUINS)){
    sprintf(buf,"To the east you can see %s.",get_move_name(exit[1]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[2]!=(room->move))&&(number_percent()<CHANCE))||(exit[2]==MOVE_ROAD)||(exit[2]==MOVE_RUINS)){
    sprintf(buf,"South of you, you can see %s.",get_move_name(exit[2]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[3]!=(room->move))&&(number_percent()<CHANCE))||(exit[3]==MOVE_ROAD)||(exit[3]==MOVE_RUINS)){
    sprintf(buf,"%s can be seen to the west.",get_move_name(exit[3]));
    strcat(desc,buf);strcpy(buf,"  ");
  }

break;

case MOVE_AIR:		
strcpy(name,"In the Air");
strcpy(desc,"You are in the air.");
  if (exitsum==4) { 
    strcpy(name,"In the Air");
    strcpy(desc,"\
You are in the air, completed surrounded by more air. Wisps of cloud-like \
moisture blows by and droplets of water condense in fatal collision with \
one another.\
");
  }
  if (((exit[0]!=(room->move))&&(number_percent()<CHANCE))||(exit[0]==MOVE_ROAD)||(exit[0]==MOVE_RUINS)){
    sprintf(buf,"You see %s to the north.",get_move_name(exit[0]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[1]!=(room->move))&&(number_percent()<CHANCE))||(exit[1]==MOVE_ROAD)||(exit[1]==MOVE_RUINS)){
    sprintf(buf,"To the east you can see %s.",get_move_name(exit[1]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[2]!=(room->move))&&(number_percent()<CHANCE))||(exit[2]==MOVE_ROAD)||(exit[2]==MOVE_RUINS)){
    sprintf(buf,"South of you, you can see %s.",get_move_name(exit[2]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[3]!=(room->move))&&(number_percent()<CHANCE))||(exit[3]==MOVE_ROAD)||(exit[3]==MOVE_RUINS)){
    sprintf(buf,"%s can be seen to the west.",get_move_name(exit[3]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
break;

case MOVE_DESERT:	
strcpy(name,"Desert Wasteland");
strcpy(desc,"You are surrounded by sand dunes. The occasional blooming \
cactus and sandstone formation give the land a simple beauty.\
");
  if (exitsum==4) { 
    strcpy(name,"Deep Within the Desert");
    strcpy(desc,"\
You are deep within a barren desert. Sand swept by the chaotic winds forms \
sand dunes that stretch on seemingly forever. The shadow of overhead \
vultures indirectly warn of the danger in this place.\
");
  }
  if (((exit[0]!=(room->move))&&(number_percent()<CHANCE))||(exit[0]==MOVE_ROAD)||(exit[0]==MOVE_RUINS)){
    sprintf(buf,"You see %s to the north.",get_move_name(exit[0]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[1]!=(room->move))&&(number_percent()<CHANCE))||(exit[1]==MOVE_ROAD)||(exit[1]==MOVE_RUINS)){
    sprintf(buf,"To the east you can see %s.",get_move_name(exit[1]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[2]!=(room->move))&&(number_percent()<CHANCE))||(exit[2]==MOVE_ROAD)||(exit[2]==MOVE_RUINS)){
    sprintf(buf,"South of you, you can see %s.",get_move_name(exit[2]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[3]!=(room->move))&&(number_percent()<CHANCE))||(exit[3]==MOVE_ROAD)||(exit[3]==MOVE_RUINS)){
    sprintf(buf,"%s can be seen to the west.",get_move_name(exit[3]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
break;

case MOVE_MOUNTAIN:	
strcpy(name,"On a Mountain");
strcpy(desc,"You are on a tranquil mountain. Though not even remotely \
dangerous, this land has its share of cliffs and gorges. \
");
  if (exitsum==4) { 
    strcpy(name,"In the Mountains");
    strcpy(desc,"You are deep within the mountains. An occasional cliff \
add to the illusory nature of this once buried rock.  Yet, nature still \
makes every use of the land here. Trees, shrubs and hardy animal all \
make their lives here. \
");
  }
  if (((exit[0]!=(room->move))&&(number_percent()<CHANCE))||(exit[0]==MOVE_ROAD)||(exit[0]==MOVE_RUINS)){
    sprintf(buf,"You see %s to the north.",get_move_name(exit[0]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[1]!=(room->move))&&(number_percent()<CHANCE))||(exit[1]==MOVE_ROAD)||(exit[1]==MOVE_RUINS)){
    sprintf(buf,"To the east you can see %s.",get_move_name(exit[1]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[2]!=(room->move))&&(number_percent()<CHANCE))||(exit[2]==MOVE_ROAD)||(exit[2]==MOVE_RUINS)){
    sprintf(buf,"South of you, you can see %s.",get_move_name(exit[2]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[3]!=(room->move))&&(number_percent()<CHANCE))||(exit[3]==MOVE_ROAD)||(exit[3]==MOVE_RUINS)){
    sprintf(buf,"%s can be seen to the west.",get_move_name(exit[3]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
break;

case MOVE_ROCK_MOUNTAIN:
strcpy(name,"On a Mountain of Rock");
strcpy(desc,"You are on a dangerous mountain. \
The rock juts from the ground like a broken bone from skin, \
and is just as jagged and dangerous. \
");
  if (exitsum==4) { 
    strcpy(name,"Within Dangerous Mountains");
    strcpy(desc,"You are withing a dangerous rocky mountain range. There \
is no way to see a way out from here. Travel in any direction would either \
have you climbing a rocky cliff or falling down one. \
");
  }
  if (((exit[0]!=(room->move))&&(number_percent()<CHANCE))||(exit[0]==MOVE_ROAD)||(exit[0]==MOVE_RUINS)){
    sprintf(buf,"You see %s to the north.",get_move_name(exit[0]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[1]!=(room->move))&&(number_percent()<CHANCE))||(exit[1]==MOVE_ROAD)||(exit[1]==MOVE_RUINS)){
    sprintf(buf,"To the east you can see %s.",get_move_name(exit[1]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[2]!=(room->move))&&(number_percent()<CHANCE))||(exit[2]==MOVE_ROAD)||(exit[2]==MOVE_RUINS)){
    sprintf(buf,"South of you, you can see %s.",get_move_name(exit[2]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[3]!=(room->move))&&(number_percent()<CHANCE))||(exit[3]==MOVE_ROAD)||(exit[3]==MOVE_RUINS)){
    sprintf(buf,"%s can be seen to the west.",get_move_name(exit[3]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
break;

case MOVE_SNOW_MOUNTAIN:
strcpy(name,"On a Snowy Peak");
strcpy(desc,"You are on a desolate snowy mountain.  It is freezing cold \
here and devoid of all life, except your own. However, even that could \
change in the blink of an eye here. If not by the gold then by a snowy \
crevasse or avalanche. \
");
  if (exitsum==4) { 
    strcpy(name,"Within Snowy Mountains");
strcpy(desc,"You are on a desolate snowy mountain.  It is freezing cold \
here and devoid of all life, except your own. However, even that could \
be changed. If not by the cold then by a icy \
crevasse or avalanche. \
");
  }
  if (((exit[0]!=(room->move))&&(number_percent()<CHANCE))||(exit[0]==MOVE_ROAD)||(exit[0]==MOVE_RUINS)){
    sprintf(buf,"You see %s to the north.",get_move_name(exit[0]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[1]!=(room->move))&&(number_percent()<CHANCE))||(exit[1]==MOVE_ROAD)||(exit[1]==MOVE_RUINS)){
    sprintf(buf,"To the east you can see %s.",get_move_name(exit[1]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[2]!=(room->move))&&(number_percent()<CHANCE))||(exit[2]==MOVE_ROAD)||(exit[2]==MOVE_RUINS)){
    sprintf(buf,"South of you, you can see %s.",get_move_name(exit[2]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[3]!=(room->move))&&(number_percent()<CHANCE))||(exit[3]==MOVE_ROAD)||(exit[3]==MOVE_RUINS)){
    sprintf(buf,"%s can be seen to the west.",get_move_name(exit[3]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
break;

case MOVE_SWAMP:	
strcpy(name,"Swamps Edge");
strcpy(desc,"You are in a dank swamps edge. The smell and humidity make
this a repelling place to say the least. 
");
  if (exitsum==4) { 
    strcpy(name,"Swampland");
    strcpy(desc,"You are in a dank swampland. All around is murky water \
and decadent mud. Occasional trees and grass mar this sickeningly beautiful \
work of nature. \
");
  }
  if (((exit[0]!=(room->move))&&(number_percent()<CHANCE))||(exit[0]==MOVE_ROAD)||(exit[0]==MOVE_RUINS)){
    sprintf(buf,"You see %s to the north.",get_move_name(exit[0]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[1]!=(room->move))&&(number_percent()<CHANCE))||(exit[1]==MOVE_ROAD)||(exit[1]==MOVE_RUINS)){
    sprintf(buf,"To the east you can see %s.",get_move_name(exit[1]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[2]!=(room->move))&&(number_percent()<CHANCE))||(exit[2]==MOVE_ROAD)||(exit[2]==MOVE_RUINS)){
    sprintf(buf,"South of you, you can see %s.",get_move_name(exit[2]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[3]!=(room->move))&&(number_percent()<CHANCE))||(exit[3]==MOVE_ROAD)||(exit[3]==MOVE_RUINS)){
    sprintf(buf,"%s can be seen to the west.",get_move_name(exit[3]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
break;

case MOVE_JUNGLE:	
strcpy(name,"At a Jungle");
strcpy(desc,"You are at the edge of a jungle. An assortment of rare and
unique plant life grow in abundance here, making it nearly impassable. \
");
  if (exitsum==4) { 
    strcpy(name,"Within a Jungle");
    strcpy(desc,"You are deep within in a jungle, \
surrounded by extravagant tropical fauna. It seems there is no way out \
and no end to the wall of plant life in every direction. \
");
  }
  if (((exit[0]!=(room->move))&&(number_percent()<CHANCE))||(exit[0]==MOVE_ROAD)||(exit[0]==MOVE_RUINS)){
    sprintf(buf,"You see %s to the north.",get_move_name(exit[0]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[1]!=(room->move))&&(number_percent()<CHANCE))||(exit[1]==MOVE_ROAD)||(exit[1]==MOVE_RUINS)){
    sprintf(buf,"To the east you can see %s.",get_move_name(exit[1]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[2]!=(room->move))&&(number_percent()<CHANCE))||(exit[2]==MOVE_ROAD)||(exit[2]==MOVE_RUINS)){
    sprintf(buf,"South of you, you can see %s.",get_move_name(exit[2]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[3]!=(room->move))&&(number_percent()<CHANCE))||(exit[3]==MOVE_ROAD)||(exit[3]==MOVE_RUINS)){
    sprintf(buf,"%s can be seen to the west.",get_move_name(exit[3]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
break;

case MOVE_RUINS:	
strcpy(name,"A Ruin");
strcpy(desc,"You have stumbled upon something of yore.");
  if (exitsum==4) { 
    strcpy(name,"Some Ruins");
    strcpy(desc,"You have managed to find some ruins.");
  }
  if
(((exit[0]!=(room->move))&&(number_percent()<CHANCE))||(exit[0]==MOVE_ROAD)){
    sprintf(buf,"You see %s to the north.",get_move_name(exit[0]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[1]!=(room->move))&&(number_percent()<CHANCE))||(exit[1]==MOVE_ROAD)){
    sprintf(buf,"To the east you can see %s.",get_move_name(exit[1]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[2]!=(room->move))&&(number_percent()<CHANCE))||(exit[2]==MOVE_ROAD)){
    sprintf(buf,"South of you, you can see %s.",get_move_name(exit[2]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
  if (((exit[3]!=(room->move))&&(number_percent()<CHANCE))||(exit[3]==MOVE_ROAD)){
    sprintf(buf,"%s can be seen to the west.",get_move_name(exit[3]));
    strcat(desc,buf);strcpy(buf,"  ");
  }
break;
} /*switch1*/

SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );

free_string( room->name );
room->name = str_dup( name );

free_string( room->description );
room->description= str_dup ( desc );

format_string(room->description);

return;
}

/* mlk this sets the whole wilderness default descs and names*/
void do_set_wilderness_all(CHAR *ch, char *argument)
{
ROOM_INDEX *room;
int start,end,current;
start=20001;
end  =29600;

room=get_room_index(start);
/* initialize room for this check here */
if (!IS_BUILDER(ch,room->area))
{
send_to_char("Wset_all: Insufficient Security",ch);
return;
}	

for (current=start ; current <= end ; current++)
{

	room=get_room_index(current);
        char_to_room( ch, room );
	set_wilderness(ch,"");
        char_from_room( ch );
	 
}

char_to_room( ch, room );

return;
}
