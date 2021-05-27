/*
   bydgoszczscript - simple, fast and efficient programming language
   Copyright (C) 2021  Kacper Kocot <kocotian@kocotian.pl>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

*/

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <arg.h>
#include <str.h>
#include <util.h>

#include <compile.h>

/* function prototypes */
static ssize_t readfile(const char *filename, char *data, size_t len);
static void usage(void);

/* global variables */
char *argv0;
#include "config.h"

/* function declarations */
static ssize_t
readfile(const char *filename, char *data, size_t len)
{
	int fd;
	ssize_t rb;

	if ((fd = open(filename, O_RDONLY)) < 0)
		return (ssize_t)fd;
	if ((rb = read(fd, data, len)) < 0)
		return rb;
	if ((fd = close(fd)) < 0)
		return fd;
	return rb;
}

static void
usage(void)
{
	die("usage: %s [-h]", argv0);
}

/* main */
int
main(int argc, char *argv[])
{
	int i;
	char data[640 * 1024];
	ssize_t rb;
	File f;

	ARGBEGIN {
	/* fallthrough */
	case 'h': default:
		usage();
		break;
	} ARGEND

	if (!argc) {
#if STDIN_IF_NO_ARGS
		f.line = 0;
		strncpy(f.filename, "<stdin>", PATH_MAX);
		while ((rb = read(STDIN_FILENO, data, LEN(data))) > 1) {
			++(f.line);
			f.content.data = data; f.content.len = (unsigned)rb;
			f.pos = 0;
			f.curtok.type = TokenNULL;
			f.curtok.c.data = data;
			f.curtok.c.len = 0;
			f.curline.data = data;
			f.curline.len = (size_t)(strchr(data, '\n') - data);
			f.outfd = STDOUT_FILENO;
			compile(f);
		}
#else
		usage();
#endif
	} else for (i = 0; i < argc; ++i) {
		if ((rb = readfile(argv[i], data, LEN(data))) < 0)
			die("readfile:");
		f.content.data = data; f.content.len = (unsigned)rb;
		f.pos = 0;
		f.line = 1;
		strncpy(f.filename, argv[i], PATH_MAX);
		f.curtok.type = TokenNULL;
		f.curtok.c.data = data;
		f.curtok.c.len = 0;
		f.curline.data = data;
		f.curline.len = (size_t)(strchr(data, '\n') - data);
		f.outfd = STDOUT_FILENO;
		compile(f);
	}
}

/*
** vim: number relativenumber ts=4 sw=4
*/
