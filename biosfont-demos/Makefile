.PHONY: all clean install install-strip uninstall

PREFIX?=.
INSTALL?=install
INSTALL_PROGRAM?=$(INSTALL)

all: show_char showdati

show_char: $(PREFIX)/include/biosfont.h
	$(CC) $(CFLAGS) show_char.c -o show_char -I$(PREFIX)/include

showdati: $(PREFIX)/include/biosfont.h
	$(CC) $(CFLAGS) showdati.c -o showdati -I$(PREFIX)/include -lncurses

install: show_char showdati
	$(INSTALL_PROGRAM) show_char $(DESTDIR)$(PREFIX)/bin
	$(INSTALL_PROGRAM) showdati $(DESTDIR)$(PREFIX)/bin

install-strip:
	$(MAKE) INSTALL_PROGRAM='$(INSTALL_PROGRAM) -s' install

uninstall:
	rm $(DESTDIR)$(PREFIX)/bin/show_char
	rm $(DESTDIR)$(PREFIX)/bin/showdati

clean:
	rm -f show_char showdati
