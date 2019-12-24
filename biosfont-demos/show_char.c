/*
Copyright (c) 2003-2012 René Ladan. All rights reserved.

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
