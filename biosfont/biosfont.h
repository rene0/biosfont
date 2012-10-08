/* Copyright (c) Rene Ladan <r.c.ladan@gmail.com>, 2-claused BSD license */

#include <sys/types.h> /* u_char */
#include <sys/ioccom.h>

typedef struct s_biosfont {
	u_char bitmap[8];
	u_char nr;
} t_biosfont;

/* 'b' (group) and 1 (len) do not seem to matter much */
#define BIOSFONT_ASCII	_IOWR('b', 1, t_biosfont)
