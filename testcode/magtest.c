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

#include <stdio.h>
#include <stdlib.h>

#include "vm68k.h"
#include "linea.h"
#include "default_callbacks.h"

#define	MAXMAGSIZE	(1<<20)
#define	MAXGFXSIZE	(1<<22)
int main(int argc,char** argv)
{
	FILE *f;
	int sizevm68k;
	int sizelineA;
	int sizeGUI;
	int retval=0;
	unsigned char* magloader;
	int magsize;
	unsigned char* gfxloader;
	int gfxsize;
	void *hVM68k;
	void *hLineA;
	void *hGUI;
	unsigned char* sharedMem;
	int unknownopcode;
	
	fprintf(stderr,"*** dMagnetic- magtest\n");
	fprintf(stderr,"*** Use at your own risk\n");
	fprintf(stderr,"*** (C)opyright 2019 by dettus@dettus.net\n");
	fprintf(stderr,"*****************************************\n");	
	fprintf(stderr,"\n");

	if (argc!=3)
	{
		fprintf(stderr,"Please run with %s MAGFILE.mag GFXFILE.gfx\n",argv[0]);
		fprintf(stderr," where MAGFILE.mag/GFXFILE.gfx is one of\n");
		fprintf(stderr,"    pawn.mag  pawn.gfx\n");
		exit(0);
	}
	magloader=malloc((MAXMAGSIZE));fprintf(stderr,"allocating %7d bytes for loading mag files\n",(MAXMAGSIZE));
	f=fopen(argv[1],"rb");
	if (f==NULL)
	{
		fprintf(stderr,"error opening file %s\n",argv[1]);
		exit(0);
	}
	magsize=fread(magloader,sizeof(char),(MAXMAGSIZE),f);
	fclose(f);
	gfxloader=malloc((MAXGFXSIZE));fprintf(stderr,"allocating %7d bytes for loading gfx files\n",(MAXGFXSIZE));
	f=fopen(argv[2],"rb");
	if (f==NULL)
	{
		fprintf(stderr,"error opening file %s\n",argv[2]);
		exit(0);
	}
	gfxsize=fread(gfxloader,sizeof(char),(MAXGFXSIZE),f);
	fclose(f);
	retval=vm68k_getsize(&sizevm68k);
	retval=lineA_getsize(&sizelineA);
	retval=default_getsize(&sizeGUI);
	if (retval) {fprintf(stderr,"getsize returned %d\n",retval);exit(0);}

	hVM68k=malloc(sizevm68k);fprintf(stderr,"allocating %7d bytes for 68000 core\n",sizevm68k);
	hLineA=malloc(sizelineA);fprintf(stderr,"allocating %7d bytes for lineA core\n",sizelineA);
	hGUI=malloc(sizeGUI);fprintf(stderr,"allocating %7d bytes for the GUI\n",sizeGUI);
	sharedMem=malloc(65536);fprintf(stderr,"allocating %7d bytes for shared mem\n",65536);
	

		
	retval=vm68k_init(hVM68k,sharedMem,65536);
	retval=lineA_init(hLineA,sharedMem,65536,magloader,magsize,gfxloader,gfxsize);
	retval=default_open(hGUI,NULL,argc,argv);

	retval=lineA_setCBoutputChar(hLineA,default_cbOutputChar,	hGUI);
	retval=lineA_setCBoutputString(hLineA,default_cbOutputString,	hGUI);
	retval=lineA_setCBinputString(hLineA,default_cbInputString,	hGUI);
	retval=lineA_setCBDrawPicture(hLineA,default_cbDrawPicture,	hGUI);
	retval=lineA_setCBLoadGame(hLineA,default_cbLoadGame,		hGUI);
	retval=lineA_setCBSaveGame(hLineA,default_cbSaveGame,		hGUI);

	// the mag and gfx buffers are no longer necessary
	free(gfxloader);
	free(magloader);

	do
	{
		unsigned short opcode;
		unknownopcode=0;
		retval=vm68k_getNextOpcode(hVM68k,&opcode);
		if (retval==VM68K_OK) retval=lineA_substitute_aliases(hLineA,&opcode);
		if (retval==LINEA_OK) retval=vm68k_singlestep(hVM68k,opcode);
		if (retval!=VM68K_OK) retval=lineA_singlestep(hLineA,hVM68k,opcode);
		if (retval!=LINEA_OK && retval!=LINEA_OK_QUIT) 
		{
			printf("\x1b[0m\n\x1b[0;37;44m unknown opcode %04X\x1b[0m\n",opcode);
			unknownopcode=1;
		}
		if (retval==LINEA_OK_QUIT) printf("\x1b[0m\n\x1b[0;37;44mGoodbye!\x1b[0m\n");
	} while (!unknownopcode && !retval);	
	
}
