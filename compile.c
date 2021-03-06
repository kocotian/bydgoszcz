/*
   bydgoszcz - simple, fast and efficient programming language
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
#include <unistd.h>

#include <errwarn.h>
#include <str.h>
#include <util.h>

#define MAX_TYPESIZE 8192
#define MAX_EXPRESSIONSIZE 8192

typedef struct {
	char ldata[MAX_TYPESIZE], rdata[MAX_TYPESIZE];
	size_t llen, rlen;
} TypeString;

typedef struct {
	char data[MAX_EXPRESSIONSIZE];
	size_t len;
} ExpressionString;

static Token *enextToken(File *f, TokenType type);
static void g_array(File *f, TypeString *str);
static void g_struct(File *f, TypeString *str);
static void g_type(File *f, TypeString *str);
static void g_expression(File *f, ExpressionString *str);
static void g_zadzwon(File *f, ExpressionString *str);
static void g_obywatel(File *f);
static void g_aglomeracja(File *f);
static void g_miasto(File *f);
static void initType(TypeString *t, int blank);

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
g_array(File *f, TypeString *str)
{
	Token *t;
	ExpressionString count;
	TypeString type;

	initType(&type, 1);
	t = enextToken(f, TokenNULL);
	if (t->type == TokenNumber) {
		g_expression(f, &count);
		t = enextToken(f, TokenIdentifier);
		if (Strccmp(t->c, "elementow"))
			errwarn(*f, 1, "unexpected identifier (expected elementow)");
		t = enextToken(f, TokenNULL);
	}
	if (t->type == TokenIdentifier) {
		if (Strccmp(t->c, "typu"))
			errwarn(*f, 1, "unexpected identifier (expected typu)");
		g_type(f, &type);
	} else {
		errwarn(*f, 1, "unexpected token (expected identifier)");
	}
	snprintf((char *)(str->ldata),
			MAX_TYPESIZE,
			"%.*s",
			(int)(type.llen), type.ldata);
	str->llen = strlen(str->ldata);
	snprintf((char *)(str->rdata),
			MAX_TYPESIZE,
			"[%.*s]",
			Strevalf(count));
	str->rlen = strlen(str->rdata);
}

static void
g_struct(File *f, TypeString *str)
{
	Token *t;
	String name, fname;
	TypeString type;

	initType(&type, 1);
	*str->ldata = 0;
	strncat(str->ldata, "struct", MAX_TYPESIZE);
	t = enextToken(f, TokenNULL);
	if (t->type == TokenIdentifier) {
		name = t->c;
		strncat(str->ldata, " ", MAX_TYPESIZE);
		++(str->llen);
		str->llen += UMIN(strlen(str->ldata) + name.len, MAX_TYPESIZE);
		strncat(str->ldata, name.data,
				UMIN(name.len, MAX_TYPESIZE - str->llen));
		str->ldata[UMIN(str->llen, MAX_TYPESIZE)] = 0;
		t = enextToken(f, TokenNULL);
	}
	if (t->type == TokenColon) {
		strncat(str->ldata, " {\n", MAX_TYPESIZE);
		while ((t = enextToken(f, TokenIdentifier))) {
			if (!Strccmp(t->c, "koniec")) {
				strncat(str->ldata, "}", MAX_TYPESIZE);
				str->llen = strlen(str->ldata);
				break;
			} else {
				fname = t->c;
				while ((t = enextToken(f, TokenNULL))) {
					if (t->type == TokenIdentifier) {
						if (!Strccmp(t->c, "przechowuje")) {
							g_type(f, &type);
						} else {
							errwarn(*f, 1, "unexpected identifier (expected przechowuje)");
						}
					} else if (t->type == TokenSemicolon) {
						snprintf((char *)(str->ldata + strlen(str->ldata)),
								MAX_TYPESIZE - strlen(str->ldata),
								"%.*s %.*s%.*s;\n",
								(int)(type.llen), type.ldata,
								Strevalf(fname),
								(int)(type.rlen), type.rdata);
						break;
					} else {
						errwarn(*f, 1, "unexpected token (expected identifier or semicolon)");
					}
				}
			}
		}
	} else if (t->type == TokenSemicolon) {
	} else {
		errwarn(*f, 1, "unexpected token (expected colon)");
	}
}

static void
g_type(File *f, TypeString *str)
{
	Token *t;
	String name, s;
	TypeString arr;

	int ptrlvl, isptrconst, isvalconst, isunsigned, isshort, islong;
	ptrlvl = isptrconst = isvalconst = isunsigned = isshort = islong = 0;

	initType(&arr, 1);

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
			/* Basic types */
			if (!Strccmp(t->c, "zadupie"))
				name.len = strlen(name.data = "void");
			else if (!Strccmp(t->c, "literki"))
				name.len = strlen(name.data = "char");
			else if (!Strccmp(t->c, "cyferki"))
				name.len = strlen(name.data = "int");
			else if (!Strccmp(t->c, "latajace"))
				name.len = strlen(name.data = "float");
			else if (!Strccmp(t->c, "podwojne"))
				name.len = strlen(name.data = "double");
			else if (!Strccmp(t->c, "cierpienie"))
				name.len = strlen(name.data = "int");
			else if (!Strccmp(t->c, "rodzine")) {
				TypeString type;
				initType(&type, 1);
				g_array(f, &type);
				name.len = type.llen;
				name.data = type.ldata;
				str->rlen = strlen(strncpy(str->rdata,
							type.rdata,
							UMIN(MAX_TYPESIZE, type.rlen)));
			}
			/* Composite types */
			else if (!Strccmp(t->c, "organizacje")) {
				TypeString type;
				initType(&type, 1);
				g_struct(f, &type);
				name.len = type.llen;
				name.data = type.ldata;
			}
			/* Other types */
			else
				name = t->c;
			break;
		}
	}
	str->llen = (size_t)(*(str->ldata) = 0);
	if (isptrconst && ptrlvl)
		str->llen = strlen(strncat(str->ldata, "const ", MAX_TYPESIZE - str->llen));
	if (isunsigned == 1)
		str->llen = strlen(strncat(str->ldata, "unsigned ", MAX_TYPESIZE - str->llen));
	else if (isunsigned == 2)
		str->llen = strlen(strncat(str->ldata, "signed ", MAX_TYPESIZE - str->llen));
	if (isshort)
		str->llen = strlen(strncat(str->ldata, "short ", MAX_TYPESIZE - str->llen));
	else if (islong)
		str->llen = strlen(strncat(str->ldata, "long ", MAX_TYPESIZE - str->llen));

	strncat(str->ldata,
			name.data,
			UMIN(MAX_TYPESIZE - str->llen, name.len));

	str->llen += name.len;

	while (ptrlvl > 0 && ptrlvl--)
		strncat(str->ldata, "*", MAX_TYPESIZE), str->llen++;
	if (isvalconst)
		strncat(str->ldata, " const", MAX_TYPESIZE), str->llen += 6;
	strncat(str->ldata,
			arr.ldata,
			UMIN(MAX_TYPESIZE - str->llen, arr.llen));
}

static void
g_expression(File *f, ExpressionString *str)
{
	Token *t;

	t = &(f->curtok);
	if (t->type == TokenNumber || t->type == TokenString) {
		strncpy(str->data, t->c.data, UMIN(MAX_EXPRESSIONSIZE, t->c.len));
		str->len = t->c.len;
	} else if (t->type == TokenIdentifier) {
		if (!Strccmp(t->c, "zadzwon")) {
			g_zadzwon(f, str);
		} else if (!Strccmp(t->c, "powieksz")
		       ||  !Strccmp(t->c, "pomniejsz")
		       ||  !Strccmp(t->c, "popowieksz")
		       ||  !Strccmp(t->c, "popomniejsz")) {
			ExpressionString expr;
			int v = !Strccmp(t->c, "powieksz") ? 1 : !Strccmp(t->c, "pomniejsz") ? 2 :
			!Strccmp(t->c, "popowieksz") ? 3 : !Strccmp(t->c, "popomniejsz") ? 4 : 0;
			t = enextToken(f, TokenNULL);
			g_expression(f, &expr);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "%s%.*s%s",
					v == 1 ? "++" : v == 2 ? "--" : "",
					Strevalf(expr),
					v == 3 ? "++" : v == 4 ? "--" : "");
		} else if (!Strccmp(t->c, "prawda")) {
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "1");
		} else if (!Strccmp(t->c, "nieprawda")) {
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "0");
		} else if (!Strccmp(t->c, "ustaw")) {
			ExpressionString what, as;
			t = enextToken(f, TokenNULL);
			g_expression(f, &what);
			t = enextToken(f, TokenIdentifier);
			if (Strccmp(t->c, "jako"))
				errwarn(*f, 1, "unexpected identifier (expected jako)");
			t = enextToken(f, TokenNULL);
			g_expression(f, &as);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "%.*s = %.*s",
					Strevalf(what), Strevalf(as));
		} else if (!Strccmp(t->c, "dodaj")) {
			ExpressionString what, where;
			t = enextToken(f, TokenNULL);
			g_expression(f, &what);
			t = enextToken(f, TokenIdentifier);
			if (Strccmp(t->c, "do"))
				errwarn(*f, 1, "unexpected identifier (expected do)");
			t = enextToken(f, TokenNULL);
			g_expression(f, &where);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "%.*s += %.*s",
					Strevalf(where), Strevalf(what));
		} else if (!Strccmp(t->c, "odejmij")) {
			ExpressionString what, from;
			t = enextToken(f, TokenNULL);
			g_expression(f, &what);
			t = enextToken(f, TokenIdentifier);
			if (Strccmp(t->c, "od"))
				errwarn(*f, 1, "unexpected identifier (expected od)");
			t = enextToken(f, TokenNULL);
			g_expression(f, &from);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "%.*s -= %.*s",
					Strevalf(from), Strevalf(what));
		} else if (!Strccmp(t->c, "pomnoz")) {
			ExpressionString what, bywhat;
			t = enextToken(f, TokenNULL);
			g_expression(f, &what);
			t = enextToken(f, TokenIdentifier);
			if (Strccmp(t->c, "przez"))
				errwarn(*f, 1, "unexpected identifier (expected przez)");
			t = enextToken(f, TokenNULL);
			g_expression(f, &bywhat);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "%.*s *= %.*s",
					Strevalf(what), Strevalf(bywhat));
		} else if (!Strccmp(t->c, "podziel")) {
			ExpressionString what, bywhat;
			t = enextToken(f, TokenNULL);
			g_expression(f, &what);
			t = enextToken(f, TokenIdentifier);
			if (Strccmp(t->c, "przez"))
				errwarn(*f, 1, "unexpected identifier (expected przez)");
			t = enextToken(f, TokenNULL);
			g_expression(f, &bywhat);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "%.*s /= %.*s",
					Strevalf(what), Strevalf(bywhat));
		} else if (!Strccmp(t->c, "suma")) {
			ExpressionString first, second;
			t = enextToken(f, TokenNULL);
			g_expression(f, &first);
			t = enextToken(f, TokenIdentifier);
			if (Strccmp(t->c, "oraz"))
				errwarn(*f, 1, "unexpected identifier (expected oraz)");
			t = enextToken(f, TokenNULL);
			g_expression(f, &second);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "%.*s + %.*s",
					Strevalf(first), Strevalf(second));
		} else if (!Strccmp(t->c, "roznica")) {
			ExpressionString first, second;
			t = enextToken(f, TokenNULL);
			g_expression(f, &first);
			t = enextToken(f, TokenIdentifier);
			if (Strccmp(t->c, "oraz"))
				errwarn(*f, 1, "unexpected identifier (expected oraz)");
			t = enextToken(f, TokenNULL);
			g_expression(f, &second);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "%.*s - %.*s",
					Strevalf(first), Strevalf(second));
		} else if (!Strccmp(t->c, "iloczyn")) {
			ExpressionString first, second;
			t = enextToken(f, TokenNULL);
			g_expression(f, &first);
			t = enextToken(f, TokenIdentifier);
			if (Strccmp(t->c, "oraz"))
				errwarn(*f, 1, "unexpected identifier (expected oraz)");
			t = enextToken(f, TokenNULL);
			g_expression(f, &second);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "%.*s * %.*s",
					Strevalf(first), Strevalf(second));
		} else if (!Strccmp(t->c, "iloraz")) {
			ExpressionString first, second;
			t = enextToken(f, TokenNULL);
			g_expression(f, &first);
			t = enextToken(f, TokenIdentifier);
			if (Strccmp(t->c, "oraz"))
				errwarn(*f, 1, "unexpected identifier (expected oraz)");
			t = enextToken(f, TokenNULL);
			g_expression(f, &second);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "%.*s / %.*s",
					Strevalf(first), Strevalf(second));
		} else if (!Strccmp(t->c, "reszta")) {
			ExpressionString first, second;
			t = enextToken(f, TokenIdentifier);
			if (Strccmp(t->c, "z"))
				errwarn(*f, 1, "unexpected identifier (expected z)");
			t = enextToken(f, TokenNULL);
			g_expression(f, &first);
			t = enextToken(f, TokenIdentifier);
			if (Strccmp(t->c, "przez"))
				errwarn(*f, 1, "unexpected identifier (expected przez)");
			t = enextToken(f, TokenNULL);
			g_expression(f, &second);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "%.*s %% %.*s",
					Strevalf(first), Strevalf(second));
		} else if (!Strccmp(t->c, "adres")) {
			ExpressionString expr;
			t = enextToken(f, TokenNULL);
			g_expression(f, &expr);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "&(%.*s)",
					Strevalf(expr));
		} else if (!Strccmp(t->c, "zawartosc")) {
			ExpressionString expr;
			t = enextToken(f, TokenNULL);
			g_expression(f, &expr);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "*(%.*s)",
					Strevalf(expr));
		} else if (!Strccmp(t->c, "przeciwienstwo")) {
			ExpressionString expr;
			t = enextToken(f, TokenNULL);
			g_expression(f, &expr);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "-(%.*s)",
					Strevalf(expr));
		} else if (!Strccmp(t->c, "nie")) {
			ExpressionString expr;
			t = enextToken(f, TokenNULL);
			g_expression(f, &expr);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "!(%.*s)",
					Strevalf(expr));
		} else if (!Strccmp(t->c, "wielkosc")) {
			ExpressionString expr;
			t = enextToken(f, TokenNULL);
			g_expression(f, &expr);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "sizeof (%.*s)",
					Strevalf(expr));
		} else if (!Strccmp(t->c, "naprawde") || !Strccmp(t->c, "czy")) {
			ExpressionString lexpr, rexpr;
			int not; int equal; int greater; int lower;
			not = equal = greater = lower = 0;
			t = enextToken(f, TokenNULL);
			g_expression(f, &lexpr);
			t = enextToken(f, TokenIdentifier);
			if (!Strccmp(t->c, "nie")) {
				not = 1;
				t = enextToken(f, TokenIdentifier);
			}
			else if (Strccmp(t->c, "jest"))
				errwarn(*f, 1, "unexpected identifier (expected jest)");
			t = enextToken(f, TokenIdentifier);
tester:
			if (!Strccmp(t->c, "rowne")) {
				equal = 1;
			} else {
				if (!Strccmp(t->c, "wieksze")) {
					greater = 1;
				} else if (!Strccmp(t->c, "mniejsze")) {
					lower = 1;
				}
				t = enextToken(f, TokenIdentifier);
				if (!Strccmp(t->c, "albo")) {
					t = enextToken(f, TokenIdentifier);
					goto tester;
				} else if (Strccmp(t->c, "niz"))
					errwarn(*f, 1, "unexpected identifier (expected albo or niz)");
			}
			t = enextToken(f, TokenNULL);
			g_expression(f, &rexpr);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "%s((%.*s) %s%s (%.*s))",
					not ? "!" : "",
					Strevalf(lexpr),
					greater ? ">" : lower ? "<" : equal ? "=" : "",
					equal ? "=" : "",
					Strevalf(rexpr));
		} else if (!Strccmp(t->c, "czlonek")) {
			ExpressionString organization;
			String member;
			t = enextToken(f, TokenIdentifier);
			member = t->c;
			t = enextToken(f, TokenIdentifier);
			if (Strccmp(t->c, "organizacji"))
				errwarn(*f, 1, "unexpected identifier (expected organizacji)");
			t = enextToken(f, TokenNULL);
			g_expression(f, &organization);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "(%.*s).%.*s",
					Strevalf(organization), Strevalf(member));
		} else if (!Strccmp(t->c, "element")) {
			ExpressionString element, family;
			t = enextToken(f, TokenNULL);
			g_expression(f, &element);
			t = enextToken(f, TokenIdentifier);
			if (Strccmp(t->c, "rodziny"))
				errwarn(*f, 1, "unexpected identifier (expected rodziny)");
			t = enextToken(f, TokenNULL);
			g_expression(f, &family);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "(%.*s)[%.*s]",
					Strevalf(family), Strevalf(element));
		} else if (!Strccmp(t->c, "rzucaj")) {
			ExpressionString expr;
			TypeString type;
			initType(&type, 1);
			t = enextToken(f, TokenNULL);
			g_expression(f, &expr);
			t = enextToken(f, TokenIdentifier);
			if (Strccmp(t->c, "w"))
				errwarn(*f, 1, "unexpected identifier (expected organizacji)");
			g_type(f, &type);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "(%.*s)(%.*s)",
					(int)(type.llen), type.ldata, Strevalf(expr));
		} else if (!Strccmp(t->c, "nieoznakowane")) {
			ExpressionString expr;
			TypeString type;
			initType(&type, 1);
			t = enextToken(f, TokenNULL);
			g_expression(f, &expr);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "(unsigned)(%.*s)",
					Strevalf(expr));
		} else if (!Strccmp(t->c, "oznakowane")) {
			ExpressionString expr;
			TypeString type;
			initType(&type, 1);
			t = enextToken(f, TokenNULL);
			g_expression(f, &expr);
			str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE, "(signed)(%.*s)",
					Strevalf(expr));
		} else if (!Strccmp(t->c, "__c")) {
			t = enextToken(f, TokenString);
			strncpy(str->data, t->c.data + 1, t->c.len - 2);
			str->len = t->c.len - 2;
		} else {
			strncpy(str->data, t->c.data, UMIN(MAX_EXPRESSIONSIZE, t->c.len));
			str->len = t->c.len;
		}
	} else {
		errwarn(*f, 1, "expected expression");
	}
	str->data[UMIN(str->len, MAX_EXPRESSIONSIZE - 1)] = 0;
}

static void
g_zadzwon(File *f, ExpressionString *str)
{
	Token *t;
	String name;
	ExpressionString expr;

	t = enextToken(f, TokenIdentifier);
	name = t->c;
	str->len = (size_t)snprintf(str->data, MAX_EXPRESSIONSIZE,
			"%.*s(", Strevalf(name));
	t = enextToken(f, TokenNULL);
	if (t->type == TokenIdentifier) {
		if (!Strccmp(t->c, "daj")) {
			t = enextToken(f, TokenColon);
			while ((t = enextToken(f, TokenNULL))) {
				g_expression(f, &expr);
				strncat(str->data, expr.data, MAX_EXPRESSIONSIZE);
				str->len += expr.len;
				t = enextToken(f, TokenNULL);
				if (t->type == TokenSemicolon)
					break;
				else if (t->type == TokenComma) {
					strncat(str->data, ", ", MAX_EXPRESSIONSIZE);
					str->len += 2;
				} else {
					errwarn(*f, 1, "unexpected %s (expected comma or semicolon)",
							stringizeTokenType(t->type));
				}
			}
		} else {
			errwarn(*f, 1, "unexpected identifier (expected daj)");
		}
	}
	strncat(str->data, ")", MAX_EXPRESSIONSIZE);
	str->len += 1;
}

static void
g_obywatel(File *f)
{
	String name;
	TypeString type;
	ExpressionString expr;

	Token *t;

	initType(&type, 0);
	t = enextToken(f, TokenIdentifier);
	name = t->c;
	expr.len = strlen(strcpy(expr.data, ""));
	while ((t = enextToken(f, TokenNULL))) {
		if (t->type == TokenIdentifier) {
			if (!Strccmp(t->c, "przechowuje")) {
				g_type(f, &type);
			} else if (!Strccmp(t->c, "rowne")) {
				t = enextToken(f, TokenNULL);
				g_expression(f, &expr);
			} else {
				errwarn(*f, 1, "unexpected identifier (expected przechowuje)");
			}
		} else if (t->type == TokenSemicolon) {
			dprintf(f->outfd, "%.*s %.*s%.*s%s%.*s;\n",
					(int)(type.llen), type.ldata,
					Strevalf(name),
					(int)(type.rlen), type.rdata,
					expr.len ? " = " : "",
					(int)(expr.len), expr.data);
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
	ExpressionString expr;

	while ((t = enextToken(f, TokenNULL))) {
		if (t->type == TokenIdentifier) {
			if (!Strccmp(t->c, "koniec")) {
				dprintf(f->outfd, "}\n");
				return;
			} else if (!Strccmp(t->c, "obywatel")) {
				g_obywatel(f);
			} else if (!Strccmp(t->c, "typ")) {
				dprintf(f->outfd, "typedef ");
				g_obywatel(f);
			} else if (!Strccmp(t->c, "oddaj")) {
				dprintf(f->outfd, "return ");
				t = enextToken(f, TokenNULL);
				goto expr;
			} else if (!Strccmp(t->c, "jesli") || !Strccmp(t->c, "dopoki")) {
				int isif; isif = !Strccmp(t->c, "jesli");
				dprintf(f->outfd, isif ? "if (" : "while (");
				while ((t = enextToken(f, TokenNULL))) {
					g_expression(f, &expr);
					t = enextToken(f, TokenIdentifier);
					dprintf(f->outfd, "(%.*s)", Strevalf(expr));
					if ((isif && !Strccmp(t->c, "wtedy")) || (!isif && !Strccmp(t->c, "rob")))
						break;
					else if (!Strccmp(t->c, "albo")) {
						dprintf(f->outfd, " || ");
					} else if (!Strccmp(t->c, "oraz")) {
						dprintf(f->outfd, " && ");
					} else {
						errwarn(*f, 1, "unexpected identifier (expected albo, oraz or %s)",
								isif ? "wtedy" : "rob");
					}
				}
				dprintf(f->outfd, ") {\n");
				t = enextToken(f, TokenColon);
				g_aglomeracja(f);
			} else if (!Strccmp(t->c, "inaczej")) {
				dprintf(f->outfd, "} else {\n");
			} else {
				goto expr;
			}
		} else if (t->type == TokenSemicolon) {
		} else {
expr:
			g_expression(f, &expr);
			dprintf(f->outfd, "%.*s;\n", Strevalf(expr));
		}
	}

	return;
}

static void
g_miasto(File *f)
{
	String name;
	TypeString type;
	ExpressionString args;

	Token *t;

	initType(&type, 0);
	t = enextToken(f, TokenIdentifier);
	name = t->c;
	args.len = (unsigned)(*(args.data) = 0);
	while ((t = enextToken(f, TokenNULL))) {
		if (t->type == TokenColon) {
			dprintf(f->outfd, "%.*s %.*s(%.*s) {\n",
					(int)(type.llen), type.ldata, Strevalf(name), Strevalf(args));
			g_aglomeracja(f);
			return;
		} else if (t->type == TokenIdentifier) {
			if (!Strccmp(t->c, "oddaje")) {
				g_type(f, &type);
			} else if (!Strccmp(t->c, "przyjmuje")) {
				t = enextToken(f, TokenColon);
				while ((t = enextToken(f, TokenIdentifier))) {
					if (!Strccmp(t->c, "cdn")) {
						args.len += (size_t)snprintf(
								(char *)(args.data + strlen(args.data)),
								MAX_TYPESIZE - strlen(args.data),
								"...");
						t = enextToken(f, TokenSemicolon);
						break;
					} else {
						String argname;
						TypeString argtype;

						initType(&type, 0);
						argname = t->c;
						t = enextToken(f, TokenNULL);
						if (t->type == TokenIdentifier) {
							if (!Strccmp(t->c, "przechowuje")) {
								g_type(f, &argtype);
							} else {
								errwarn(*f, 1, "unexpected identifier (expected przechowuje)");
							}
							t = enextToken(f, TokenNULL);
						}
						if (t->type == TokenComma || t->type == TokenSemicolon) {
							args.len += (size_t)snprintf(
									(char *)(args.data + strlen(args.data)),
									MAX_TYPESIZE - strlen(args.data),
									"%.*s %.*s%.*s%s",
									(int)(argtype.llen), argtype.ldata,
									Strevalf(argname),
									(int)(argtype.rlen), argtype.rdata,
									t->type == TokenComma ? ", " : "");
							if (t->type == TokenSemicolon) break;
						} else {
							errwarn(*f, 1, "unexpected token (expected identifier or semicolon)");
						}
					}
				}
			} else {
				errwarn(*f, 1, "unexpected identifier (expected przyjmuje or oddaje)");
			}
		} else {
			errwarn(*f, 1, "unexpected token (expected colon, przyjmuje or oddaje)");
		}
	}
}

static void
initType(TypeString *t, int blank)
{
	t->llen = strlen(strcpy(t->ldata, blank ? "" : "int"));
	t->rlen = strlen(strcpy(t->rdata, ""));
}

void
compile(File f)
{
	Token *t;

	while ((t = nextToken(&f)) != NULL) {
		if (!Strccmp(t->c, "wykorzystaj")) {
			t = enextToken(&f, TokenString);
			dprintf(f.outfd, "#include <%.*s.h>\n",
					(int)t->c.len - 2, t->c.data + 1);
		} else if (!Strccmp(t->c, "miasto")) {
			g_miasto(&f);
		} else if (!Strccmp(t->c, "obywatel")) {
			g_obywatel(&f);
		} else if (!Strccmp(t->c, "typ")) {
			dprintf(f.outfd, "typedef ");
			g_obywatel(&f);
		} else {
			errwarn(f, 1, "unexpected token");
		}
	}
}
