#
# vcd Module Makefile
#

CFLAGS = -Wall -I../src/ -L.
CC = gcc 
MODS = resolver echomod calc

all:
	@echo -e "\nCompiling module library..."
	@make -s lib
	@echo -e "\nCompiling modules..."
	@make -s mods
	@cd ../bin

mods: resolver echomod calc

lib:
	echo "       mhp.c -> mhp.o -> libmhp.a"
	$(CC) $(CFLAGS) -o mhp.o -c mhp.c
	ar rcs libmhp.a mhp.o

.c:	
	echo "       $*.c -> $*"
	$(CC) $(CFLAGS) -o $* $*.c -lmhp

clean:
	rm -fr $(MODS) *core* *.o *.a
