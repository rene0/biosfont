/*
 * Copyright (c) Rene Ladan <r.c.ladan@gmail.com>, 2-claused BSD license
 *
 * biosfont character device
 *
 * Based on 5.X version of echo device, FreeBSD Architecture Handbook
 */

#include "biosfont.h"

#include <sys/systm.h> /* printf, bootverbose */
#include <sys/module.h>
#include <sys/param.h> /* defines used in kernel.h */
#include <sys/kernel.h> /* types used in module initialization */
#include <sys/conf.h> /* cdevsw struct */
#include <sys/stat.h> /* S_Ixxxx */

#include <vm/vm.h> /* required for vm/pmap.h */
#include <vm/pmap.h> /* required for machine/pmap.h */
#include <machine/pmap.h> /* required for vmparam.h */
#include <machine/vmparam.h> /* KERNBASE */

/* Function prototypes */
static d_ioctl_t biosfont_ioctl;

/* Character device entry points */
static struct cdevsw biosfont_cdevsw = {
	.d_version = D_VERSION,
	.d_flags = D_MEM, /* prevent kernel panic */
	.d_name = "biosfont",
	.d_ioctl = biosfont_ioctl,
};

/* Character device */
static struct cdev *biosfont_dev;

/*
 * Addresses are stored as segment:offset, in little endian, thus
 * 0x000ffa6e aka f000:fa6e is stored as 0x6e 0xfa 0x00 0xf0
 * Absolute address is segment * 16 + offset.
 *
 * XXX This define is i386/amd64-specific
 */
#define SEGOFS(i) (((i & 0xffff0000) >> 12) + ((i & 0x0000ffff)))

static u_long fontbase;

/*
 * This function is called by the kld[un]load(2) system calls to
 * determine what actions to take when a module is loaded or unloaded.
 */
static int
biosfont_loader(struct module *m __unused, int what, void *arg __unused)
{
	int err = 0;
	u_long *p; /* "read-only" variable to prevent garbling memory */

	switch (what) {
	case MOD_LOAD: /* kldload */
		p = (u_long *) (KERNBASE + 0x0000007CL);
		/* correct byte order */
		fontbase = SEGOFS(*p);
		biosfont_dev = make_dev(&biosfont_cdevsw,
			0, /* auto-minor-device */
			UID_ROOT,
			GID_WHEEL,
			S_IRUSR | S_IRGRP | S_IROTH,
			biosfont_cdevsw.d_name);
		if (bootverbose)
			printf("<biosfont>: upper=%08lx\n", fontbase);
		break;
	case MOD_UNLOAD:
		destroy_dev(biosfont_dev);
		break;
	default:
		err = EINVAL;
		break;
	}
	return (err);
}

static int
biosfont_ioctl(struct cdev *dev __unused, u_long cmd, caddr_t data, int fflag __unused, struct thread *td __unused)
{
	u_char i;
	u_long start;
	u_char *p;

	t_biosfont *request = (t_biosfont *) data;
	switch (cmd) {
	case BIOSFONT_ASCII:
		/* determine start of bitmap table */
		start = (u_long) (KERNBASE + request->nr * 8);
		if (request->nr < 0x80)
			start += 0x000FFA6EL;
		else
			start += (u_long) (fontbase - 1024);
			/* 1024 == 8*128, char 128 starts at fontbase */
		/* copy bitmap to userland */
		for (i = 0; i < 8; i++) {
			p = (u_char *) (start + i);
			request->bitmap[i] = *p;
		}
		return (0);
	default:
		/* should not happen */
		printf("<biosfont>: cmd, nr = %08lx, %02x\n", cmd, request->nr);
		return (EINVAL);
	}
}

DEV_MODULE(biosfont, biosfont_loader, NULL);
