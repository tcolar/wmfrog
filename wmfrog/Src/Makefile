CC     = gcc
CFLAGS = -g #-O2 -Wall
INCDIR = -I/usr/X11R6/include/X11 -I/usr/X11R6/include
LIBDIR = -L/usr/X11R6/lib
# for linux
LIBS   = -lXpm -lX11 -lXext
# for Solaris
# LIBS   = -lXpm -lX11 -lXext -lsocket
PREFIX=/usr

OBJS   = wmfrog.o \
         xutils.o


.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -D$(shell echo `uname -s`) -c $< -o $*.o $(INCDIR)


all:	wmfrog.o wmfrog 

wmfrog.o: frog.xpm frog.xbm
wmfrog:	$(OBJS) 
	$(CC) $(LDFLAGS) $(SYSTEM) -o wmfrog $^ $(INCDIR) $(LIBDIR) $(LIBS)
install:
		mkdir -p "$(DESTDIR)$(PREFIX)/bin"
		mkdir -p "$(DESTDIR)$(PREFIX)/lib/wmfrog"
		mkdir -p "$(DESTDIR)$(PREFIX)/share/man/man1"
		cp wmfrog "$(DESTDIR)$(PREFIX)/bin"
		cp weather.pl "$(DESTDIR)$(PREFIX)/lib/wmfrog"
		cp wmfrog.1 "$(DESTDIR)$(PREFIX)/share/man/man1"
clean:
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmfrog


