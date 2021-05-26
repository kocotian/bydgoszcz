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
#include <string.h>

#include <errwarn.h>
#include <str.h>

static Token *enextToken(File *f, TokenType type);
static String evalType(String s);
static void g_miasto(File *f);

static Token *
enextToken(File *f, TokenType type)
{
	Token *r;
	if ((r = nextToken(f)) == NULL)
		errwarn(*f, 1, "unexpected end of input");
	if (type != TokenNULL && r->type != type)
		errwarn(*f, 1, "token type mismatch (expected %s, have %s)",
				stringizeTokenType(type), stringizeTokenType(r->type));
	return r;
}

static String
evalType(String s)
{
	if (!Strccmp(s, "literki"))
		s.len = strlen(s.data = "char");
	if (!Strccmp(s, "cyferki"))
		s.len = strlen(s.data = "int");
	if (!Strccmp(s, "latajace"))
		s.len = strlen(s.data = "float");
	if (!Strccmp(s, "podwojne"))
		s.len = strlen(s.data = "double");
	return s;
}

static void
g_aglomeracja(File *f)
{
	Token *t;

	while ((t = enextToken(f, TokenNULL))) {
		if (t->type == TokenIdentifier) {
			if (!(Strccmp(t->c, "koniec"))) {
				dprintf(f->outfd, "}\n");
				return;
			} else {
				errwarn(*f, 1, "unexpected identifier (expected a keyword)");
			}
		} else {
			errwarn(*f, 1, "unexpected token (expected an expression)");
		}
	}

	return;
}

static void
g_miasto(File *f)
{
	String name;
	String type;

	Token *t;

	t = enextToken(f, TokenIdentifier);
	name = t->c;
	type.data = "int"; type.len = 3;
	while ((t = enextToken(f, TokenNULL))) {
		if (t->type == TokenColon) {
			dprintf(f->outfd, "%.*s %.*s() {\n",
					Strevalf(type), Strevalf(name));
			g_aglomeracja(f);
			return;
		} else if (t->type == TokenIdentifier) {
			if (!(Strccmp(t->c, "oddaje"))) {
				t = enextToken(f, TokenIdentifier);
				type = evalType(t->c);
			} else {
				errwarn(*f, 1, "unexpected identifier (expected przyjmuje or oddaje)");
			}
		} else {
			errwarn(*f, 1, "unexpected token (expected colon, przyjmuje or oddaje)");
		}
	}
}

void
compile(File f)
{
	Token *t;
	while ((t = nextToken(&f)) != NULL) {
		if (!(Strccmp(t->c, "wykorzystaj"))) {
			t = enextToken(&f, TokenString);
			dprintf(f.outfd, "#include <%.*s.h>\n",
					(int)t->c.len - 2, t->c.data + 1);
		} else if (!(Strccmp(t->c, "poczatek"))) {
			t = enextToken(&f, TokenIdentifier);
			if (!(Strccmp(t->c, "miasta"))) {
				g_miasto(&f);
			}
		} else {
			errwarn(f, 1, "unexpected token");
		}
	}
}
