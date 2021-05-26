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

#include <errwarn.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "config.h"

void
errwarn(File f, int isError, const char *fmt, ...)
{
	va_list ap;
	ssize_t i;

	fprintf(stderr,
#if COLOR
			"\033[0;1m"
#endif
			"%s:%d:%ld: "
#if COLOR
			"\033[1;3%cm"
#endif
			"%s: "
#if COLOR
			"\033[0m"
#endif
			,
			f.filename, f.line, (f.curtok.c.data - f.curline.data) + 1,
#if COLOR
			isError ? '1' : '3',
#endif
			isError ? "error" : "warning");

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	fprintf(stderr, "\n % 4d | %.*s\n      | ",
			f.line, (int)f.curline.len, f.curline.data);

	for (i = 0; i < f.curtok.c.data - f.curline.data; ++i) {
		fprintf(stderr, " ");
	}

	fprintf(stderr,
#if COLOR
			"\033[1;3%cm"
#endif
			"^~~~~"
#if COLOR
			"\033[0m"
#endif
			"\n"
#if COLOR
			, isError ? '1' : '3'
#endif
			);

	if (isError)
		exit(1);
}
