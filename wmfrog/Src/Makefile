CC     = gcc
CFLAGS = -O2 -Wall
INCDIR = -I/usr/X11R6/include/X11 -I/usr/X11R6/include
LIBDIR = -L/usr/X11R6/lib
# for linux
LIBS   = -lXpm -lX11 -lXext
# for Solaris
# LIBS   = -lXpm -lX11 -lXext -lsocket
PREFIX=/usr

OBJS   = wmFrog.o \
         xutils.o


.c.o:
	$(CC) $(CFLAGS) -D$(shell echo `uname -s`) -c $< -o $*.o $(INCDIR)


all:	wmFrog.o wmFrog 

wmFrog.o: frog.xpm frog.xbm
wmFrog:	$(OBJS) 
	$(CC) $(CFLAGS) $(SYSTEM) -o wmfrog $^ $(INCDIR) $(LIBDIR) $(LIBS)
install:
		mkdir -p "$(DESTDIR)$(PREFIX)/bin"
		mkdir -p "$(DESTDIR)$(PREFIX)/lib/wmfrog"
		cp wmfrog "$(DESTDIR)$(PREFIX)/bin"
		cp weather.pl "$(DESTDIR)$(PREFIX)/lib/wmfrog"
clean:
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmfrog


