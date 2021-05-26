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

#ifndef _TOKEN_H
#define _TOKEN_H

#include <linux/limits.h>
#include <sys/types.h>

#include <str.h>

typedef enum {
	TokenNULL,
	TokenIdentifier, TokenNumber, TokenString,
	TokenOpeningParenthesis, TokenClosingParenthesis,
	TokenColon, TokenSemicolon,
} TokenType;

typedef struct Token {
	TokenType type;
	String c;
} Token;

typedef struct File {
	String content;
	char filename[PATH_MAX];
	size_t pos;
	int line;
	Token curtok;
	String curline;
	int outfd;
} File;

Token *nextToken(File *f);
char *stringizeTokenType(TokenType t);

#endif
