CC      =gcc
PROF    =-O0 -g3#      -pg -DUSE_MCHECK
LIBS    = -lcrypt -lz #-lgd
L_FLAGS =$(PROF) -ggdb3 -o0  -D_GNU_SOURCE
E_FLAGS =#            -Lefence/ -lefence -DMALLOC_DEBUG
O_FLAGS =$(PROF)
C_FLAGS =-Wall -Wstrict-prototypes -Wpointer-arith -Wno-char-subscripts -Wmissing-declarations -Wmissing-prototypes -Wshadow -Wformat=2 -Waggregate-return -DLARGEFILE_SOURCE=1 -O0 \
         -Winline $(O_FLAGS)

#-Wmissing-prototypes -Wmissing-declarations

.SUFFIXES:
.SUFFIXES: .c .o
# -L/usr/local/lib 


O_FILES = alias.o admin.o admin_edit.o admin_info.o bit.o board.o client.o combat.o \
	config.o def.o door.o edit.o events.o load.o functions.o furn.o \
	grammar.o graphics.o handler.o html.o disease.o interpreter.o \
	language.o lists.o magic.o map.o mem.o money.o mount.o newbie.o net.o \
	info.o move.o props.o parser.o save.o shop.o skills.o social.o \
	string.o tips.o translate.o track.o update.o worldgen.o xrand.o \
        mssp_client.o mssp_tables.o telopt.o

nimud.new: $(O_FILES)
	$(CC) $(L_FLAGS) -o nimud $(O_FILES) $(LIBS)
#	$(CC) $(L_FLAGS) -o nimud *.o

.c.o: nimud.h
	$(CC) -c $(C_FLAGS) $<

clean:
	rm -f *.o nimud
