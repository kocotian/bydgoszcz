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

#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L

#include <compile.h>
#include <stddef.h>
#include <stdio.h>

#include <errwarn.h>
#include <str.h>

static Token *enextToken(File *f);

static Token *
enextToken(File *f)
{
	Token *r;
	if ((r = nextToken(f)) == NULL)
		errwarn(*f, 1, "unexpected end of input");
	return r;
}

void
compile(File f)
{
	Token *t;
	while ((t = nextToken(&f)) != NULL) {
		if (!(Strccmp(t->c, "wykorzystaj"))) {
			t = enextToken(&f);
			dprintf(f.outfd, "#include <%.*s.h>\n",
					(int)t->c.len, t->c.data);
		} else {
			errwarn(f, 1, "unexpected token");
		}
	}
}
