#!/bin/sh
#Copyright 2020, dettus@dettus.net
#
#Redistribution and use in source and binary forms, with or without modification,
#are permitted provided that the following conditions are met:
#
#1. Redistributions of source code must retain the above copyright notice, this 
#   list of conditions and the following disclaimer.
#
#2. Redistributions in binary form must reproduce the above copyright notice, 
#   this list of conditions and the following disclaimer in the documentation 
#   and/or other materials provided with the distribution.
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
#ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
#WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
#DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
#FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
#DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
#SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
#CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
#OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
#OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

PREFIX?=/usr/local
INSTALLBIN=$(PREFIX)/bin
INSTALLSHARE=$(PREFIX)/share/games
INSTALLMAN=$(PREFIX)/share/man

CC?=gcc
AR?=ar
#CFLAGS=-g -O0
CFLAGS?=-O3
CFLAGS+=-Wall
#CFLAGS+=-Werror
PROJ_HOME=./
INCFLAGS=-I$(PROJ_HOME)src/gui -I$(PROJ_HOME)src/toplevel -I$(PROJ_HOME)src/loader -I$(PROJ_HOME)src/engine/vm68k -I$(PROJ_HOME)src/engine/linea -I$(PROJ_HOME)src/engine/include 
OBJDIR=$(PROJ_HOME)obj/
LINK=$(CC)
LDFLAGS+="-L"$(OBJDIR)
ECHO_CMD?=echo
SHA256_CMD?=sha256
AWK_CMD?=awk

SOURCES_LOADER=	\
	src/loader/loader_msdos.c	\
	src/loader/loader_mw.c		\
	src/loader/loader_d64.c		\
	src/loader/maggfxloader.c

SOURCES_LINEA=	\
	src/engine/linea/gfx1loader.c	\
	src/engine/linea/gfx2loader.c	\
	src/engine/linea/linea.c

SOURCES_VM68K=	\
	src/engine/vm68k/vm68k.c	\
	src/engine/vm68k/vm68k_decode.c	\
	src/engine/vm68k/vm68k_loadstore.c

SOURCES_GUI=	\
	src/gui/default_callbacks.c	\
	src/gui/default_palette.c	\
	src/gui/default_render.c

SOURCES_TOPLEVEL=	\
	src/toplevel/configuration.c	\
	src/toplevel/dMagnetic.c

OBJ_LOADER=${SOURCES_LOADER:.c=.o}
OBJ_LINEA=${SOURCES_LINEA:.c=.o}
OBJ_VM68K=${SOURCES_VM68K:.c=.o}
OBJ_GUI=${SOURCES_GUI:.c=.o}
OBJ_TOPLEVEL=${SOURCES_TOPLEVEL:.c=.o}

all:	dMagnetic
	strip dMagnetic

clean:
	rm -rf dMagnetic
	rm -rf $(OBJ_LOADER)
	rm -rf $(OBJ_LINEA)
	rm -rf $(OBJ_VM68K)
	rm -rf $(OBJ_GUI)
	rm -rf $(OBJ_TOPLEVEL)

install: all dMagnetic.1 dMagneticini.5
	mkdir -p $(INSTALLBIN)
	mkdir -p $(INSTALLMAN)/man1
	mkdir -p $(INSTALLMAN)/man5
	mkdir -p $(INSTALLSHARE)/dMagnetic

	cp dMagnetic $(INSTALLBIN)
	cp dMagnetic.1 $(INSTALLMAN)/man1
	cp dMagneticini.5 $(INSTALLMAN)/man5
	cp README.txt $(INSTALLSHARE)/dMagnetic
	cp LICENSE.txt $(INSTALLSHARE)/dMagnetic
	cp dMagnetic.ini $(INSTALLSHARE)/dMagnetic


dMagnetic:	$(OBJ_LOADER) $(OBJ_LINEA) $(OBJ_VM68K) $(OBJ_GUI) $(OBJ_TOPLEVEL)
	$(LINK) $(LDFLAGS) -o $@ $(OBJ_LOADER) $(OBJ_LINEA) $(OBJ_VM68K) $(OBJ_GUI) $(OBJ_TOPLEVEL)

.c.o:
	$(CC) $(CFLAGS) $(CFLAGS_EXTRA) $(INCFLAGS) -c -o $@ $<

do-test:	dMagnetic
	if [ true \
		-a "`${ECHO_CMD} Hello | ./dMagnetic -ini dMagnetic.ini -vmode none -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $1; }' - `" = "70af45367a6ad1b612ceabd36fef309d4258abac275281a5541342ccf0a765cd" \
		-a "`${ECHO_CMD} Hello | ./dMagnetic -ini dMagnetic.ini -vmode monochrome -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $1; }' - `" = "aae8070b0ca69ec099e3407859256765723c9daa9d18cc0e17916897febfa2f5" \
		-a "`${ECHO_CMD} Hello | ./dMagnetic -ini dMagnetic.ini -vmode monochrome_inv -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $1; }' - `" = "5802834232c4bee199c0318b69a781c722a75b15486652bbcf3bb907c31eef8c" \
		-a "`${ECHO_CMD} Hello | ./dMagnetic -ini dMagnetic.ini -vmode low_ansi -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $1; }' - `" = "b9795498b0ac1d458e39e6817142faafa99fec5e97b1dc450462f425d734d075" \
		-a "`${ECHO_CMD} Hello | ./dMagnetic -ini dMagnetic.ini -vmode low_ansi2 -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $1; }' - `" = "aba97f906b979cb277211ee2f8bfe6a7b647c7b1fa5faaac5bdb242e9f179354" \
		-a "`${ECHO_CMD} Hello | ./dMagnetic -ini dMagnetic.ini -vmode high_ansi -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $1; }' - `" = "91a443b46e40caa3a8f229144adafe8220f445f452cae6632e0f7dcd95e03b9f" \
		-a "`${ECHO_CMD} Hello | ./dMagnetic -ini dMagnetic.ini -vmode high_ansi2 -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $1; }' - `" = "f322e2b9c1e3683e444e8174bc16329014d5b2fde33916f8b7f799010e83b5dd" \
		-a "`${ECHO_CMD} Hello | ./dMagnetic -ini dMagnetic.ini -vmode sixel -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $1; }' - `" = "66c74ef9cf29839cec26639345e546705c87bacd95a7e47c1343cc9e8ffa8ccb" \
	] ; then echo OK ; else echo "expected output not seen"; exit 1; fi
