/* Copyright (c) Rene Ladan <r.c.ladan@gmail.com>, 2-claused BSD-license */

#include <errno.h>
#include <fcntl.h> /* open */
#include <limits.h> /* strtol */
#include <stdio.h>
#include <stdlib.h> /* strtol */
#include <string.h> /* strerror */
#include <unistd.h> /* close */

#include <sys/ioctl.h>

#include <biosfont.h>

int
main(int argc, char **argv)
{
	t_biosfont ch;
	int f;
	int i, j;

	/* sanity check */	
	if (argc != 2) {
		printf("usage: %s <n>, with 0 <= n <= 255\n", argv[0]);
		return(1);
	}
	ch.nr = strtol(argv[1], NULL, 0);
	printf("call = %lx\n", BIOSFONT_ASCII);
	printf("character = %i\n", ch.nr);
	
	/* obtain bitmap from BIOS */
	f = open("/dev/biosfont", O_RDONLY);
	if (f == -1 && errno != 0) {
		printf("Error: %s\n", strerror(errno));
		return(errno);
	}
	printf("Doing ioctl\n");
	if (ioctl(f, BIOSFONT_ASCII, &ch) == -1) {
		printf("Error: %s\n", strerror(errno));
		return(errno);
	}
	close(f);

	/* display the character bitmap */
        printf("\n--char-- hex\n");
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++)
			printf("%s", (ch.bitmap[i] & (0x80 >> j)) == 0 ? " " : "#");
		printf(" %02x\n", ch.bitmap[i]);	
	}

	return(0);
}
