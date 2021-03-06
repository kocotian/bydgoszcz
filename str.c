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

#include <str.h>
#include <string.h>

int
Strcmp(const String s1, const String s2)
{
	if (s1.len != s2.len)
		return (int)(s1.len - s2.len);
	return strncmp(s1.data, s2.data, s1.len);
}

int
Strccmp(const String s, const char *cs)
{
	if (s.len != strlen(cs))
		return (int)(s.len - strlen(cs));
	return strncmp(s.data, cs, s.len);
}
