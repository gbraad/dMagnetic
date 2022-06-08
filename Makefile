#!/usr/bin/make -f 
#
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

SOURCES_LOADER=	\
	src/loader/loader_common.c	\
	src/loader/loader_msdos.c	\
	src/loader/loader_mw.c		\
	src/loader/loader_d64.c		\
	src/loader/loader_dsk.c		\
	src/loader/loader_archimedes.c	\
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
#	strip dMagnetic

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
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CFLAGS_EXTRA) $(INCFLAGS) -c -o $@ $<


## in case some post-compilation checks are needed
## if not, please comment out this line
include ./checks.mk
