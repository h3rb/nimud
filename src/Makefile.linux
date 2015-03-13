CC      =gcc
PROF    =-O -g3#      -pg -DUSE_MCHECK
L_FLAGS =$(PROF) -lcrypt#     -lmcheck -static -DMALLOC_DEBUG
E_FLAGS =#            -Lefence/ -lefence -DMALLOC_DEBUG
O_FLAGS =$(PROF)
C_FLAGS =-Wall -Wstrict-prototypes -Wpointer-arith -Wno-char-subscripts \
         -Winline $(O_FLAGS)

.SUFFIXES:
.SUFFIXES: .c .o
# -L/usr/local/lib 


O_FILES = admin.o admin_edit.o admin_info.o bit.o board.o client.o combat.o config.o def.o door.o edit.o events.o file_io.o function.o furn.o grammar.o graphics.o handler.o language.o lists.o magic.o mem.o money.o mount.o nanny.o net.o imc.o info.o move.o props.o parser.o save.o script.o shop.o skills.o social.o spells.o string.o track.o update.o worldgen.o xrand.o

nimud.new: $(O_FILES)
	$(CC) $(L_FLAGS) -o nimud *.o

.c.o: nimud.h
	$(CC) -c $(C_FLAGS) $<

clean:
	rm -f *.o nimud
