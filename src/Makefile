# This is the Makefile for vcd.  This works and has been tested on the 
# following OS's: 
#
#	SunOS   4.1.3_U1
#	Linux   1.2.x
#	FreeBSD 2.1.x
#
# For any other OS's, you're on your own.
#

DESTDIR = ../bin

DEBUG = -g -Wall

#
# CFLAGS:
#	add -DNOSMARTLOG if you want to log all errors
#
CFLAGS = ${DEBUG}


CC=gcc $(GLANG) $(GWARN)

OBJ = 	talker.o log.o socket.o chat.o io.o lists.o commands.o parse.o \
	power.o channels.o db.o help.o softboot.o mail.o mods.o board.o

all:	
	@echo
	@echo "Please run make with one of the following arguments:"
	@echo "make linux   --  linux systems"
	@echo "make freebsd --  freebsd systems"
	@echo "make solaris --  solaris systems"
	@echo

linux: $(OBJ)
	$(CC) $(CFLAGS) -o $(DESTDIR)/vcd $(OBJ) $(CLIBS)
	@echo "Haven compiled to $(DESTDIR)/vcd"
	@cd ../mods ; make -s
	@cd ../bin

freebsd: $(OBJ)
	$(CC) $(CFLAGS) -o $(DESTDIR)/vcd $(OBJ) -lcrypt
	@cd ../mods ; make -s
	@cd ../bin

solaris: $(OBJ)
	$(CC) $(CFLAGS) -o $(DESTDIR)/vcd $(OBJ) -lsocket -lnsl
	@cd ../mods ; make -s
	@cd ../bin

clean:
	rm -fr *.o ../core ../logs/* $(DESTDIR)/vcd ../etc/sb.* ../etc/*.pid
	@cd ../mods ; make clean 

dist:
	make clean
	rm -fr ../data/users/*/* ../data/mail/*/* ../etc/vc.index
	@cd ../mods ; make clean

backups:
	tar -cf $(HOME)/vcdbackups.tar ../logs ../data ../etc
	gzip $(HOME)/vcdbackups.tar

restore:
	cp $(HOME)/vcdbackups.tar ..
	tar -xf ../vcdbackups.tar
	rm ../vcdbackups.tar

# dependencies
talker.o:   talker.c vcd.h
log.o:      log.c vcd.h
socket.o:   socket.c vcd.h
chat.o:     chat.c vcd.h
io.o:       io.c vcd.h
lists.o:    lists.c vcd.h  
commands.o: commands.c vcd.h
parse.o:    parse.c vcd.h
power.o:    power.c vcd.h
channels.o: channels.c vcd.h
db.o:       db.c vcd.h	
help.o:     help.c vcd.h
softboot.o: softboot.c vcd.h		
mail.o:     mail.c vcd.h
mods.o:     mods.c vcd.h
board.o:    board.c vcd.h

#.c.o:
#	@echo "     $*.c -> $*.o"
#	@$(CC) $(CFLAGS) -c $*.c 
