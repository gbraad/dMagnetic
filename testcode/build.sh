#!/bin/sh
#Copyright 2019, dettus@dettus.net
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



export CFLAGS="-g"
export CFLAGS_EXTRA=""

export PROJ_HOME="../"
export INCFLAGS="-I"$PROJ_HOME"src/gui/ -I"$PROJ_HOME"src/toplevel -I"$PROJ_HOME"src/engine/vm68k -I"$PROJ_HOME"src/engine/linea -I"$PROJ_HOME"src/engine/include"
export OBJDIR=$PROJ_HOME"/obj"
export LDFLAGS="-L"$OBJDIR
(
 cd ../
 sh build.sh
)
mkdir -p $OBJDIR

gcc -g -c -o instmatcher.o instmatcher.c $INCFLAGS
gcc -g -o instmatcher.app instmatcher.o $OBJDIR/vm68k_decode.o $LDFLAGS -l_vm68k
gcc -g -c -o magtest.o magtest.c $INCFLAGS
gcc -g -o magtest.app magtest.o $LDFLAGS -l_vm68k -l_linea -l_gui -l_configuration
gcc -g -c -o gfxtest.o gfxtest.c $INCFLAGS
gcc -g -o gfxtest.app gfxtest.o $LDFLAGS -l_vm68k -l_linea -l_gui -l_configuration

md5sum *.app

