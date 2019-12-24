/*
Copyright (c) 2003-2019 René Ladan. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
*/

#include <biosfont.h>
#include <curses.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <stdlib.h>

/*
 * Retrieve an integer sysctl pointed to by mib
 */
int
get_int_sysctl(int mib[2], int *i)
{
	size_t len;

	len = sizeof(int);
	if (sysctl(mib, 2, i, &len, NULL, 0))
		return(errno);
	return(0);
}

/*
 * Retrieve a string sysctl pointed to by mib
 */
int
get_str_sysctl(int mib[2], char **r)
{
	size_t len;

	if (sysctl(mib, 2, NULL, &len, NULL, 0))
		return(errno);
	*r = malloc(len);
	if (sysctl(mib, 2, *r, &len, NULL, 0))
		return(errno);
	return(0);
}

/*
 * Write 8x8 character bitmap[ch] at (y, x)
 */
void
write_char(int y, int x, int ch, u_char bitmap[12][8])
{
	char outch;
	int i, j;

	/* convert ch to its character equivalent */
	outch = (ch < 10 ? '0' + ch : (ch == 10 ? ':' : '-'));

	for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++)
			mvprintw(y + i, x + 7 - j, "%c", (bitmap[ch][i] & (1 << j)) == 0 ? ' ' : outch);
	
}

int
main(int argc, char **argv)
{
	t_biosfont ch;
	u_char bitmap[12][8];
	int f;
	time_t t;
	struct tm *lt;
	int oldsec;
	int i, j;
	int colorwin;
	char *wd[7] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
	int mib[2];
	int osreldate;
	char *r, *os_ident, *os_name;

	/* get osreldate */
	mib[0] = CTL_KERN;
	mib[1] = KERN_OSRELDATE;
	if (get_int_sysctl(mib, &osreldate)) {
		printf("Error (sysctl): %s\n", strerror(errno));
		return(errno);
	}

	/* get os name, version, and identifier */
	mib[0] = CTL_KERN;
	mib[1] = KERN_VERSION;
	if (get_str_sysctl(mib, &r)) {
		printf("Error (sysctl): %s\n", strerror(errno));
		return(errno);
	}
	for (os_name = strdup(r), i = 0; os_name[i] != ':' && os_name[i] != '\n' && i < strlen(os_name); i++)
		;
	os_name[i] = '\0';
	os_ident = strrchr(r, '/');
	if (os_ident != NULL) {
		for (i = 0; os_ident[i] != '\n' && i+1 < strlen(os_ident); i++)
			os_ident[i] = os_ident[i + 1];
		os_ident[i-1] = '\0';
	}
	/* read bitmaps of 0 1 2 3 4 5 6 7 8 9 : - into bitmap */
	f = open("/dev/biosfont", O_RDONLY);
	if (f == -1 && errno != 0) {
		printf("Error: %s\n", strerror(errno));
		return(errno);
	}
	for (i = 0; i < 12; i++) {
		ch.nr = (i < 10 ? '0' + i : (i == 10 ? ':' : '-'));
		if (ioctl(f, BIOSFONT_ASCII, &ch) == -1) {
			printf("Error: %s\n", strerror(errno));
			return(errno);
		}
		for (j = 0; j < 8; j++)
			bitmap[i][j] = ch.bitmap[j];
	}
	close(f);

	/* initialize curses */
	initscr();
	cbreak();
	noecho();
	nonl();
	nodelay(stdscr, TRUE);
	curs_set(0); /* make cursor invisible */
	colorwin = (has_colors() == TRUE);

	/* initialize color table */
	if (colorwin) {
		start_color();
		init_pair(1, COLOR_BLUE, COLOR_BLACK);
		init_pair(2, COLOR_WHITE, COLOR_BLACK);
		init_pair(3, COLOR_GREEN, COLOR_BLACK);
		init_pair(4, COLOR_YELLOW, COLOR_BLACK);
		attron(COLOR_PAIR(4));
	}

	/* output something like "FreeBSD 6.1-RC #0 (600105, RENE)" */
	if (os_ident != NULL) {
		asprintf(&r, "%s (%i, %s)", os_name, osreldate, os_ident);
	} else {
		asprintf(&r, "%s (%i)", os_name, osreldate);
	}
	mvprintw(12, (80 - strlen(r)) / 2, "%s", r);

	if (colorwin)
		attron(COLOR_PAIR(1));
 	
	/* output - and : delimiters */
	write_char(2, 32, 11, bitmap);
	write_char(2, 56, 11, bitmap);
	write_char(15, 24, 10, bitmap);
	write_char(15, 48, 10, bitmap);

	/* only update screen if second has changed */
	oldsec = -1;
	/* retrieve keys non-blocking */
	while (getch() == ERR) {
		/* get current time, rewrite numbers */
		t = time(&t);
		lt = localtime(&t);
		if (oldsec != lt->tm_sec) {
			oldsec = lt->tm_sec;
			if (colorwin)
				attron(COLOR_PAIR(2));
			/* date */
			write_char(2, 0, (lt->tm_year + 1900) / 1000, bitmap);
			write_char(2, 8, (lt->tm_year + 1900) % 1000 / 100, bitmap);
			write_char(2, 16, lt->tm_year % 100 / 10, bitmap);
			write_char(2, 24, lt->tm_year % 10, bitmap);
			write_char(2, 40, (lt->tm_mon + 1) / 10, bitmap);
			write_char(2, 48, (lt->tm_mon + 1) % 10, bitmap);
			write_char(2, 64, lt->tm_mday / 10, bitmap);
			write_char(2, 72, lt->tm_mday % 10, bitmap);
			/* time */
			write_char(15, 8, lt->tm_hour / 10, bitmap);
			write_char(15, 16, lt->tm_hour % 10, bitmap);
			write_char(15, 32, lt->tm_min / 10, bitmap);
			write_char(15, 40, lt->tm_min % 10, bitmap);
			write_char(15, 56, lt->tm_sec / 10, bitmap);
			write_char(15, 64, lt->tm_sec % 10, bitmap);
			/* weekdays (only with has_colors()) */
			if (colorwin)
				for (i = 0; i < 7; i++) {
					attron(COLOR_PAIR(lt->tm_wday == i ? 3 : 2));
					mvprintw(15 + i, 75, "%s", wd[i]);
				}
		}

		refresh(); /* output diff to screen */
		usleep(100000); /* 100 ms */
	}

	/* close curses */
	endwin();

	return(0);
}
