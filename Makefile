# bydgoszcz - simple, fast and efficient programming language
# Copyright (C) 2021  Kacper Kocot <kocotian@kocotian.pl>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

PREFIX = /usr/local
CC = gcc

all: by

by: by.c util.c str.c compile.c token.c errwarn.c
	${CC} -std=c99 -pedantic -Wall -Wextra -Wconversion -Iinclude -o $@ $^

install: by
	mkdir -p ${DESTDIR}${PREFIX}/bin
	install -Dm755 by ${DESTDIR}${PREFIX}/bin/by-core
	install -Dm755 by-wrapper ${DESTDIR}${PREFIX}/bin/by

clean:
	rm -f by

.PHONY: all clean
