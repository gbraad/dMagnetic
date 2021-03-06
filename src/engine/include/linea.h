/*

Copyright 2022, dettus@dettus.net

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
#ifndef	LINEA_H
#define	LINEA_H

#define	LINEA_OK			0
#define	LINEA_OK_QUIT			28844
#define	LINEA_OK_RESTART		28816
#define	LINEA_NOK_UNKNOWN_INSTRUCTION	1
#define	LINEA_NOK_INVALID_PTR		-1
#define	LINEA_NOK_INVALID_PARAM		-2
#define	LINEA_NOK_NOT_ENOUGH_MEMORY	-3

#include "picture.h"
// callback pointers.
typedef int (*cbLineAOutputChar)(void* context,char c,unsigned char controlD2,unsigned char flag_headline);
typedef int (*cbLineAOutputString)(void* context,char* string,unsigned char controlD2,unsigned char flag_headline);
typedef int (*cbLineAInputString)(void* context,int* len,char* string);
typedef int (*cbLineADrawPicture)(void* context,tPicture* picture,int mode);
typedef int (*cbLineASaveGame)(void* context,char* filename,void* ptr,int len);
typedef int (*cbLineALoadGame)(void* context,char* filename,void* ptr,int len);

// configuration functions
int lineA_getsize(int* size);
int lineA_init(void* hLineA,void* pSharedMem,int *sharedmemsize,void* pMag,int magsize,void* pGfx,int gfxsize);
int lineA_configrandom(void* hLineA,char random_mode,unsigned int random_seed);
int lineA_setEGAMode(void* hLineA,int egamode);
int lineA_getVersion(void* hLineA,int* version);
int lineA_setCBoutputChar(void* hLineA,cbLineAOutputChar pCB,void *context);
int lineA_setCBoutputString(void* hLineA,cbLineAOutputString pCB,void* context);
int lineA_setCBinputString(void* hLineA,cbLineAInputString pCB,void* context);
int lineA_setCBDrawPicture(void* hLineA,cbLineADrawPicture pCB,void* context);
int lineA_setCBSaveGame(void* hLineA,cbLineASaveGame pCB,void* context);
int lineA_setCBLoadGame(void* hLineA,cbLineALoadGame pCB,void* context);

int lineA_showTitleScreen(void* hLineA);
// api
int lineA_substitute_aliases(void* hLineA,unsigned short* opcode);
int lineA_singlestep(void* hLineA,void* hVM68k,unsigned short opcode);


#endif
