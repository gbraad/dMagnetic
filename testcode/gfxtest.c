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
#include <string.h>
#include "picture.h"
#include "gfx1loader.h"

#define	MAXGFXSIZE	(1<<22)

int main(int argc,char** argv)
{
	FILE *f;
	int picnum;
	int i,j;
	void* gfxloader;
	int gfxsize;
#define	MAXXPM	(1<<20)
	char* xpm;

	tPicture picture;
	fprintf(stderr,"*** dMagnetic- magtest\n");
	fprintf(stderr,"*** Use at your own risk\n");
	fprintf(stderr,"*** (C)opyright 2019 by dettus@dettus.net\n");
	fprintf(stderr,"*****************************************\n");	
	fprintf(stderr,"\n");


	if (argc!=3)
	{
		fprintf(stderr,"please run with %s GFXFILE.gfx PICNUM >output.xpm\n",argv[0]);
		return 0;
	}
	gfxloader=malloc(MAXGFXSIZE);
	picnum=atoi(argv[2]);
	f=fopen(argv[1],"rb");
	gfxsize=fread(gfxloader,sizeof(char),MAXGFXSIZE,f);		
	fclose(f);

	gfxloader_unpackpic(gfxloader,gfxsize,0,picnum,NULL,&picture);
	xpm=malloc(MAXXPM);
	gfxloader_picture2xpm(&picture,xpm,MAXXPM);
#if 0
	printf("/* XPM */\n");
	printf("static char *xpm[] = {\n");
	printf("/* columns rows colors chars-per-pixel */\n");
	printf("\"%d %d 16 1 \",\n",picture.width,picture.height);

	for (i=0;i<16;i++)
	{
		printf("\"%c c #",i+'A');
		printf("%02X",((picture.palette[i]>>8)&0x7)*0x24);
		printf("%02X",((picture.palette[i]>>4)&0x7)*0x24);
		printf("%02X",((picture.palette[i]>>0)&0x7)*0x24);
		printf("\",\n");
	}
	printf("/* pixels */\n");
	for (i=0;i<picture.height;i++)
	{
		printf("\"");
		for (j=0;j<picture.width;j++)
		{
			printf("%c",picture.pixels[i*picture.width+j]+'A');
		}
		printf("\"");
		if (i!=(picture.height-1)) printf(",");
		printf("\n");
	}
	printf("};\n");
#endif
	printf("%s\n",xpm);
	free(xpm);
	return 0;
}
