/*

Copyright 2019, dettus@dettus.net

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this 
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


*/

#ifndef	GFXLOADER_H
#define	GFXLOADER_H
#include "vm68k_datatypes.h"
#include "picture.h"	// for the pPicture data type

// this function is extracting the picture from the .gfx file and converts it into something easier to handle.
int gfxloader_unpackpic(tVM68k_ubyte* gfxbuf,tVM68k_ulong gfxsize,tVM68k_ubyte version,int picnum,tVM68k_ubyte* picname,tPicture* pPicture,int egamode);


// those function can convert the picture into an xpm.
// this one calculates the space needed for the final picture
int gfxloader_picture_calcxpmsize(tPicture* pPicture,int* xpmsize);
// this one converts it.
int gfxloader_picture2xpm(tPicture* pPicture,char* xpm,int xpmspace);

#endif

