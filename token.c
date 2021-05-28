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

#include <token.h>

#include <ctype.h>
#include <stddef.h>
#include <string.h>

#include <errwarn.h>
#include <util.h>

Token *
nextToken(File *f)
#define CURCHAR (f->content.data[f->pos])
#define i (f->pos)
#define NOT_OVERFLOW (i < f->content.len)
#define TYPE(T) (f->curtok.type = T)
{
	size_t initpos;
	f->curtok.c.data += f->curtok.c.len;
	f->curtok.c.len = 0;
	f->curtok.type = TokenNULL;
	while (isspace(CURCHAR) && NOT_OVERFLOW) {
		if (CURCHAR == '\n') {
			++(f->line);
			f->curline.data = &CURCHAR + 1;
			f->curline.len = (size_t)(strchr(f->curline.data, '\n') - f->curline.data);
		}
		++(f->curtok.c.data), ++i;
	}
	initpos = i;
	if (!NOT_OVERFLOW)
		return NULL;
	/* Identifier */
	else if ((CURCHAR >= 'a' && CURCHAR <= 'z')
	     ||  (CURCHAR >= 'A' && CURCHAR <= 'Z')
	     ||  (CURCHAR == '_')) {
		TYPE(TokenIdentifier);
		++i;
		while (((CURCHAR >= 'a' && CURCHAR <= 'z')
	       ||   (CURCHAR >= 'A' && CURCHAR <= 'Z')
	       ||   (CURCHAR >= '0' && CURCHAR <= '9')
	       ||   (CURCHAR == '_'))
		   &&  NOT_OVERFLOW) ++i;
	}
	/* Number */
	else if (CURCHAR >= '0' && CURCHAR <= '9') {
		TYPE(TokenNumber);
		++i;
		while ((CURCHAR >= '0' && CURCHAR <= '9')
		   &&  NOT_OVERFLOW) ++i;
	}
	/* String */
	else if ((CURCHAR == '"')) {
		TYPE(TokenString);
		++i;
		while (CURCHAR != '"' && NOT_OVERFLOW) ++i;
		++i;
	}
	/* OpeningParenthesis */
	else if ((CURCHAR == '(')) {
		TYPE(TokenOpeningParenthesis);
		++i;
	}
	/* ClosingParenthesis */
	else if ((CURCHAR == ')')) {
		TYPE(TokenClosingParenthesis);
		++i;
	}
	/* Colon */
	else if ((CURCHAR == ':')) {
		TYPE(TokenColon);
		++i;
	}
	/* Semicolon */
	else if ((CURCHAR == ';')) {
		TYPE(TokenSemicolon);
		++i;
	}
	/* Comma */
	else if ((CURCHAR == ',')) {
		TYPE(TokenComma);
		++i;
	}
	/* Other unexpected token */
	else {
		errwarn(*f, 1, "unexpected token: '%c'", CURCHAR);
	}
	f->curtok.c.len = i - initpos;
	return &(f->curtok);
}
#undef TYPE
#undef NOT_OVERFLOW
#undef i
#undef CURCHAR

char *
stringizeTokenType(TokenType t)
{
	switch (t) {
	case TokenNULL: return "<null>"; break;
	case TokenIdentifier: return "identifier"; break;
	case TokenNumber: return "number"; break;
	case TokenString: return "string"; break;
	case TokenOpeningParenthesis: return "opening parenthesis"; break;
	case TokenClosingParenthesis: return "closing parenthesis"; break;
	case TokenColon: return "colon"; break;
	case TokenSemicolon: return "semicolon"; break;
	default: break;
	}
	return "<unknown>";
}
