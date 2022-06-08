/*

Copyright 2020, dettus@dettus.net

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

#include "vm68k_datatypes.h"
#include "vm68k_macros.h"
#include "linea.h"
#include <stdio.h>

#define	READ_INT32ME(ptr,idx)	(\
	(((tVM68k_ulong)((ptr)[((idx)+1)])&0xff)<<24)	|\
	(((tVM68k_ulong)((ptr)[((idx)+0)])&0xff)<<16)	|\
	(((tVM68k_ulong)((ptr)[((idx)+3)])&0xff)<< 8)	|\
	(((tVM68k_ulong)((ptr)[((idx)+2)])&0xff)<< 0)	|\
	0)


typedef struct _tGFX2_properties
{
	tVM68k_bool is_animation;

	tVM68k_uword	width;
	tVM68k_uword	height;

	tVM68k_uword	celnum;
	tVM68k_uword	animnum;
	tVM68k_uword	commandnum;
	tVM68k_uword	framecnt;


	tVM68k_ulong offset_background;
	tVM68k_ulong offset_cels;
	tVM68k_ulong offset_animations;
	tVM68k_ulong offset_commands;

} tGFX2_properties;

int gfx2loader_findoffset(tVM68k_ubyte* gfx2buf,tVM68k_ulong gfx2bufsize,char* filename, tVM68k_slong* offset,tVM68k_slong* length)
{
	int i;
	int j;
	int idx;
	int directorysize;

	*offset=0;
	*length=0;
	directorysize=READ_INT16LE(gfx2buf,4);	// the size of the directory is always at this position.
	for (i=0;i<directorysize/16;i++)
	{
		int found;
		char c1;
		char c2;
		found=-1;
		idx=i*16+6+0;	// adressing the directory entry. 
		for (j=0;j<8 && found==-1;j++)	// the first 8 bytes are the "filename" of the picture
		{
			c1=READ_INT8BE(gfx2buf,idx+j);
			c2=filename[j];
			if (c2==0 && c1==0) found=1;
			else if (c1!=c2) found=0;	// mismatch (case sensitive)
		}
		if (found)
		{
			*offset=READ_INT32BE(gfx2buf,i*16+6+ 8);
			*length=READ_INT32BE(gfx2buf,i*16+6+12);
			return 0;	// found -> OK
		}
	}
	return -1;
	
}

int gfx2loader_loadproperties(tGFX2_properties* pGFX2,tVM68k_ubyte* gfx2buf,tVM68k_ulong gfx2bufsize,tVM68k_ubyte version,char* filename)
{
	int i;
	tVM68k_slong offset;
	tVM68k_slong length;
	tVM68k_slong idx;
	pGFX2->is_animation=0;	
	pGFX2->width=0;
	pGFX2->height=0;
	pGFX2->framecnt=0;

	// first: find out where the picture/animation is within the gfx2buf
	// and find out it is an animation or not.
	if (!gfx2loader_findoffset(gfx2buf,gfx2bufsize,filename,&offset,&length))
	{
		tVM68k_slong datasize;
		tVM68k_uword animword;

		idx=offset;
		pGFX2->offset_background=idx;
		datasize=READ_INT32ME(gfx2buf,idx+38);
		pGFX2->width=READ_INT16LE(gfx2buf,idx+42);
		pGFX2->height=READ_INT16LE(gfx2buf,idx+44);
		animword=READ_INT16LE(gfx2buf,idx+datasize+48);
		if (animword!=0x5ed0)
		{
			pGFX2->is_animation=1;
			// second: the animation cels.
			idx=offset+50+datasize;

			pGFX2->offset_cels=idx;
			pGFX2->celnum=READ_INT16LE(gfx2buf,idx);
			for (i=0;i<pGFX2->celnum;i++)
			{
				int height1,width1,datasize1;
				int height2,width2,datasize2;
	
				datasize1=READ_INT32ME(gfx2buf,idx+4);
				width1=READ_INT16LE(gfx2buf,idx+8);
				height1=READ_INT16LE(gfx2buf,idx+10);
	
				width2=READ_INT16LE(gfx2buf,idx+datasize1+14);	
				height2=READ_INT16LE(gfx2buf,idx+datasize1+16);	
				if (height1==height2 && width1==width2)	// the cel contains a transparency mask
				{
					datasize2=READ_INT16LE(gfx2buf,idx+datasize1+18);
					idx+=12+datasize1+6+datasize2;
				} else {
					// the cel does not contain a transparency mask. just width, height, size and a bitmap
					idx+=12+datasize1;
				}
			}
			idx+=2;		// UNKNOWN

			// third: the animations, or "actors"
			pGFX2->offset_animations=idx;
			pGFX2->animnum=READ_INT16LE(gfx2buf,idx);
			idx+=2;
			idx+=2;	// UNKNOWN
			for (i=0;i<pGFX2->animnum;i++)
			{
				int animsteps;
				animsteps=READ_INT16LE(gfx2buf,idx);	idx+=2;
				idx+=2;	// UNKNOWN
				idx+=2*animsteps;	// X
				idx+=2*animsteps;	// Y
				idx+=2*animsteps;	// celnum
				idx+=2*animsteps;	// UNKNOWN(*)
			}
			idx-=2;		// remove the last UNKNOWN(*)
			
			// fourth: the commands, the "script"
			pGFX2->offset_commands=idx;
			pGFX2->commandnum=READ_INT16LE(gfx2buf,idx);idx+=2;
			pGFX2->framecnt=0;
			for (i=0;i<pGFX2->commandnum;i++)
			{
				int cmd;
				cmd=READ_INT8BE(gfx2buf,idx);idx++;
				switch(cmd)
				{
					case 0x01:	idx+=3;break;
					case 0x02:	pGFX2->framecnt+=READ_INT8BE(gfx2buf,idx);idx++;break;
					case 0x03:	break;
					case 0x04:	idx+=3;break;
					case 0x05:	idx+=3;break;
					default:
						break;
				}
			}
		}
		
	}
	return 0;
}
