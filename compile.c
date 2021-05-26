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
#include <stdlib.h>
#include <string.h>

#include <errwarn.h>
#include <str.h>
#include <util.h>

#define MAX_TYPESIZE 256

static String defaultType(void);
static Token *enextToken(File *f, TokenType type);
static void g_type(File *f, String *str);
static void g_expression(File *f);
static void g_zadzwon(File *f);
static void g_obywatel(File *f);
static void g_miasto(File *f);

static String
defaultType(void)
{
	char *data = malloc(MAX_TYPESIZE); /* XXX: not freed */
	return (String){ .data = strcpy(data, "int"), .len = 3 };
}

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

static void
g_type(File *f, String *str)
{
	Token *t;
	String name, s;

	int ptrlvl, isptrconst, isvalconst, isunsigned, isshort, islong;
	/*
	** flags:
	** | long | short | unsigned | const value | const pointer |
	*/

	ptrlvl = isptrconst = isvalconst = isunsigned = isshort = islong = 0;

	while ((t = enextToken(f, TokenIdentifier))) {
		s = t->c;
		if (!Strccmp(s, "wskaznik")) {
			t = enextToken(f, TokenIdentifier);
			if (Strccmp(t->c, "na"))
				errwarn(*f, 1, "expected 'na' after 'wskaznik'");
			++ptrlvl;
		}
		else if (!Strccmp(s, "staly")) isptrconst = 1;
		else if (!Strccmp(s, "stale")) isvalconst = 1;
		else if (!Strccmp(s, "nieoznakowane")) isunsigned = 1;
		else if (!Strccmp(s, "oznakowane")) isunsigned = 2;
		else if (!Strccmp(s, "krotkie")) isshort = 1;
		else if (!Strccmp(s, "dlugie")) islong = 1;
		else {
			if (!Strccmp(t->c, "literki"))
				name.len = strlen(name.data = "char");
			else if (!Strccmp(t->c, "cyferki"))
				name.len = strlen(name.data = "int");
			else if (!Strccmp(t->c, "latajace"))
				name.len = strlen(name.data = "float");
			else if (!Strccmp(t->c, "podwojne"))
				name.len = strlen(name.data = "double");
			else if (!Strccmp(t->c, "cierpienie"))
				name.len = strlen(name.data = "int");
			else
				name = t->c;
			break;
		}
	}
	str->len = (size_t)(*(str->data) = 0);
	if (isptrconst && ptrlvl)
		strncat(str->data, "const ", MAX_TYPESIZE - str->len), str->len += 5;
	if (isunsigned == 1)
		strncat(str->data, "unsigned ", MAX_TYPESIZE - str->len), str->len += 9;
	else if (isunsigned == 2)
		strncat(str->data, "signed ", MAX_TYPESIZE - str->len), str->len += 7;
	if (isshort)
		strncat(str->data, "short ", MAX_TYPESIZE - str->len), str->len += 6;
	else if (islong)
		strncat(str->data, "long ", MAX_TYPESIZE - str->len), str->len += 5;

	strncat(str->data, name.data, UMIN(MAX_TYPESIZE - str->len, name.len));
	str->len += name.len;

	while (ptrlvl > 0 && ptrlvl--)
		strncat(str->data, "*", MAX_TYPESIZE - str->len++);
	if (isvalconst)
		strncat(str->data, " const", MAX_TYPESIZE - str->len), str->len += 5;
}

static void
g_expression(File *f)
{
	Token *t;
	t = &(f->curtok);
	if (t->type == TokenString) {
		dprintf(f->outfd, "%.*s",
				Strevalf(t->c));
	} else {
		errwarn(*f, 1, "expected expression");
	}
}

static void
g_zadzwon(File *f)
{
	Token *t;
	String name;

	t = enextToken(f, TokenIdentifier);
	name = t->c;
	dprintf(f->outfd, "%.*s(",
			Strevalf(name));
	t = enextToken(f, TokenNULL);
	if (t->type == TokenIdentifier) {
		if (!Strccmp(t->c, "daj")) {
			while ((t = enextToken(f, TokenNULL))) {
				if (t->type == TokenSemicolon)
					goto semicolon;
				else
					g_expression(f);
			}
		} else {
			errwarn(*f, 1, "unexpected identifier (expected daj)");
		}
	} else if (t->type == TokenSemicolon) {
semicolon:
		dprintf(f->outfd, ");\n");
		return;
	} else {
		errwarn(*f, 1, "unexpected token (expected identifier or semicolon)");
	}
}

static void
g_obywatel(File *f)
{
	String name;
	String type;

	Token *t;

	t = enextToken(f, TokenIdentifier);
	name = t->c;
	type = defaultType();
	while ((t = enextToken(f, TokenNULL))) {
		if (t->type == TokenIdentifier) {
			if (!Strccmp(t->c, "przechowuje")) {
				g_type(f, &type);
			} else {
				errwarn(*f, 1, "unexpected identifier (expected przechowuje)");
			}
		} else if (t->type == TokenSemicolon) {
			dprintf(f->outfd, "%.*s %.*s;\n",
					Strevalf(type), Strevalf(name));
			return;
		} else {
			errwarn(*f, 1, "unexpected token (expected identifier or semicolon)");
		}
	}
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
			} else if (!(Strccmp(t->c, "obywatel"))) {
				g_obywatel(f);
			} else if (!(Strccmp(t->c, "zadzwon"))) {
				g_zadzwon(f);
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
	type = defaultType();
	while ((t = enextToken(f, TokenNULL))) {
		if (t->type == TokenColon) {
			dprintf(f->outfd, "%.*s %.*s() {\n",
					Strevalf(type), Strevalf(name));
			g_aglomeracja(f);
			return;
		} else if (t->type == TokenIdentifier) {
			if (!(Strccmp(t->c, "oddaje"))) {
				g_type(f, &type);
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
		} else if (!(Strccmp(t->c, "obywatel"))) {
			g_obywatel(&f);
		} else if (!(Strccmp(t->c, "miasto"))) {
			g_miasto(&f);
		} else {
			errwarn(f, 1, "unexpected token");
		}
	}
}
