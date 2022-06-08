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

#include "vm68k_datatypes.h"
#include "vm68k_macros.h"
#include "gfx1loader.h"
#include "linea.h"	// for the picture
#include <string.h>
#include <stdio.h>

int gfxloader_gfx1(tVM68k_ubyte* gfxbuf,tVM68k_ulong gfxsize,tVM68k_ubyte version,int picnum,tPicture* pPicture)
{
	int i;
	int retval;

	tVM68k_ulong picoffs;
	tVM68k_uword height;
	tVM68k_uword width;
	tVM68k_uword tablesize;
	tVM68k_ulong datasize;
	
	tVM68k_uword tableidx;
	tVM68k_ulong byteidx;
	tVM68k_ubyte bitidx;
	tVM68k_ubyte curbyte;
	tVM68k_ubyte curpixel;
	
	pPicture->halftones=0;	// only gfx3 offers halftone pictures
	retval=0;
	picnum&=0xffff;
	picoffs=READ_INT32BE(gfxbuf,8+4*picnum);	// the .gfx file starts with the index pointers to the actual picture data.

	// once the offset has been calculated, the actual picture data is as followed:
	// bytes 0..1: UNKNOWN
	// bytes 0x02..0x03: X1
	// bytes 0x04..0x05: X2
	// X2-X2=width
	// bytes 0x06..0x07: height
	// bytes 0x08..0x1b: UNKNOWN
	// bytes 0x1c..0x3b: palette.
	// bytes 0x3c..0x3d: size of the huffman table
	// bytes 0x3e..0x41: size of the data bit stream
	// 
	// bytes 0x42+0x42+tablesize: huffman decoding table
	// bytes 0x42+tablesize..0x42+tablesize+datasize: bitstream
	curpixel=0;
	width=READ_INT16BE(gfxbuf,picoffs+4)-READ_INT16BE(gfxbuf,picoffs+2);
	height=READ_INT16BE(gfxbuf,picoffs+6);
	for (i=0;i<16;i++)
	{
		pPicture->palette[i]=READ_INT16BE(gfxbuf,picoffs+0x1c+2*i);
	}
	tablesize=READ_INT16BE(gfxbuf,picoffs+0x3c);	// size of the huffman table
	datasize =READ_INT32BE(gfxbuf,picoffs+0x3e);	// size of the bitstream
	

	// the huffman table contains links. if a bit in the stream is set, the upper 8 bits, otherwise the lower ones.
	// terminal symbols have bit 7 set.
	bitidx=8;	// MSB first
	byteidx=picoffs+0x42+tablesize*2+2;
	tableidx=0;
	curbyte=READ_INT8BE(gfxbuf,byteidx);
	byteidx++;
	for (i=0;(i<height*width) && (byteidx<(picoffs+0x42+tablesize*2+2+datasize) );i++)
	{
		if (tableidx==0)
		{
			tableidx=tablesize;	// start at the end of the huffman table
			while (!(tableidx&0x80))	// bit 7 denotes a terminal symbol
			{
				tVM68k_uword nxt;
				bitidx--;	// MSB first.
				nxt=READ_INT16BE(gfxbuf,picoffs+0x42+2*tableidx);
				if ((curbyte>>bitidx)&1)	// bit was set. read the higher 8 bits.
				{
					tableidx=(nxt>>8)&0xff;
				} else {
					tableidx=(nxt)&0xff;	// bit was not set. read the lower 8 bits.
				}
				if (bitidx==0)
				{
					curbyte=READ_INT8BE(gfxbuf,byteidx);
					byteidx++;
					bitidx=8;	// MSB first
				}
			}
			tableidx&=0x7f;	// remove bit 7.
			if (tableidx>=0x10)	// it bits 6..4 were set, the previous pixels are being repeated
			{
				tableidx-=0x10;
			} else {	// since there are only 16 possible pixels, this is it.
				curpixel=tableidx;
				tableidx=1;	// will become 0 in the next revelation.
			}
		}
		pPicture->pixels[i]=curpixel;
		tableidx--;
	}
	pPicture->height=height;
	pPicture->width=width;

	// the finishing touch: each line has to be XORed with the previous one.
	for (i=width;i<width*height;i++)
	{
		pPicture->pixels[i]^=pPicture->pixels[i-width];
	}
	return retval;

}

// the gfx2 format stored some values as little endian.
#define READ_INT16LE(ptr,idx)   (\
        (((tVM68k_ulong)((ptr)[((idx)+1)])&0xff)<< 8)   |\
        (((tVM68k_ulong)((ptr)[((idx)+0)])&0xff)<< 0)   |\
        0)
// the gfx2 format introduced MIXED ENDIAN. 
#define READ_INT32ME(ptr,idx)   (\
        (((tVM68k_ulong)((ptr)[((idx)+1)])&0xff)<<24)   |\
        (((tVM68k_ulong)((ptr)[((idx)+0)])&0xff)<<16)   |\
        (((tVM68k_ulong)((ptr)[((idx)+3)])&0xff)<< 8)   |\
        (((tVM68k_ulong)((ptr)[((idx)+2)])&0xff)<< 0)   |\
        0)

int gfxloader_gfx2(tVM68k_ubyte* gfxbuf,tVM68k_ulong gfxsize,tVM68k_ubyte version,tVM68k_ubyte* picname,tPicture* pPicture)
{
        int directorysize;
        int offset;
	//int length;
	int retval;
	int i;
	int j;
	int found;

	pPicture->width=0;
	pPicture->height=0;
	pPicture->halftones=0;	// only gfx3 offers halftone pictures
	// the gfx2 buffer starts with the magic value, and then a directory
	directorysize=READ_INT16BE(gfxbuf,4);

	retval=0;	
	// step 1: find the correct filename
	found=0;
	for (i=0;i<directorysize && !found;i+=16)
	{
		// each entry in the directory is 16 bytes long. 8 bytes "filename", 4 bytes offset, 4 bytes length. filenames are 0-terminated.
		tVM68k_ubyte c1,c2;
		found=1;
		j=0;
		do
		{

			c1=gfxbuf[6+i+j];	
			c2=picname[j];
			if ((c1&0x5f)!=(c2&0x5f)) found=0;	// compare, uppercase
			if ((c1&0x5f)==0) j=8;	// end of the entry reached.
			j++;		
		} while (j<8 && found);
		if (found)
		{
			offset=READ_INT32BE(gfxbuf,i+6+8);		// this is the offset from the beginning of the gfxbuf
//			length=READ_INT32BE(gfxbuf,i+6+12);
		}
	}
	// TODO: sanity check. is length-48==height*width/2?
	if (found)
	{
		// each picture has the following format:
		// @0: 4 bytes UNKNOWN
		// @4..36 16*2 bytes palette
		// @38:      4 bytes data size (MIXED ENDIAN)
		// @42:      2 bytes width
		// @44:      2 bytes height
		// @48:      beginning of the bitmap block.
		// after the bitmap block is a magic field.
		// if it is !=0x5ed0, there is going to be an animation

// the data format is as such: Each pixel consists of 4 bits:3210. in each line, 
// the bits are lumped together, beginning with bit 0 of the first pixel, then 
// bit 0 of the second pixel, then bit 0 of the third pixel and so on. (MSB first)
// afterwards, the bit lump for bit 1 starts.
//
// 00000000111111112222222233333333
// 00000000111111112222222233333333
// 00000000111111112222222233333333
// 00000000111111112222222233333333
//
		int x,y;
		int idx0,idx1,idx2,idx3;
		int lumpsize;
		int datasize;
		int pixidx;

		for (i=0;i<16;i++)
		{
			pPicture->palette[i]=READ_INT16LE(gfxbuf,offset+4+2*i);
		}
		datasize=READ_INT32ME(gfxbuf,offset+38);
		pPicture->width=READ_INT16LE(gfxbuf,offset+42);
		pPicture->height=READ_INT16LE(gfxbuf,offset+44);

		// animmagic=READ_INT16LE(gfx2buf,offset+48+data_size);
		lumpsize=datasize/pPicture->height/4;	// datasize=size of the picture. height: number of lines. thus: datasize/height=number of bytes per line. there are 4 lumps of bits per line.
		pixidx=0;
		for (y=0;y<pPicture->height;y++)
		{
			tVM68k_ubyte byte0,byte1,byte2,byte3;
			tVM68k_ubyte mask;
			idx0=y*4*lumpsize+offset+48;
			idx1=idx0+1*lumpsize;
			idx2=idx0+2*lumpsize;
			idx3=idx0+3*lumpsize;
			for (x=0;x<pPicture->width;x+=8)
			{
				tVM68k_ubyte p;
				byte0=gfxbuf[idx0++];
				byte1=gfxbuf[idx1++];
				byte2=gfxbuf[idx2++];
				byte3=gfxbuf[idx3++];
				mask=(1<<7);		// MSB FIRST
				for (i=0;i<8;i++)
				{
					p =(byte0&mask)?0x01:0;
					p|=(byte1&mask)?0x02:0;
					p|=(byte2&mask)?0x04:0;
					p|=(byte3&mask)?0x08:0;
					mask>>=1;
					if ((x+i)<pPicture->width)
					{
						pPicture->pixels[pixidx++]=p;
					}
				}
			}
		}
	} 
	

	return retval;
}


// the gfx3 format stored some values as little endian.
#define READ_INT32LE(ptr,idx)   (\
        (((tVM68k_ulong)((ptr)[((idx)+3)])&0xff)<<24)   |\
        (((tVM68k_ulong)((ptr)[((idx)+2)])&0xff)<<16)   |\
        (((tVM68k_ulong)((ptr)[((idx)+1)])&0xff)<< 8)   |\
        (((tVM68k_ulong)((ptr)[((idx)+0)])&0xff)<< 0)   |\
        0)
#define	MAXPICWIDTH	512
int gfxloader_gfx3(tVM68k_ubyte* gfxbuf,tVM68k_ulong gfxsize,tVM68k_ubyte version,int picnum,tPicture* pPicture)
{
	//  0.. 3: 4 bytes "MaP3"
	//  4.. 8: 4 bytes length of index
	//  8..11: 4 bytes length of disk1.pix
	// 11..15: 4 bytes length of disk2.pix
	// then the index
	// then the disk1.pix data
	// then the disk2.pix data
	int retval;
	int offs1;
	int offs2;
	int offset;
	int indexoffs,indexlen;
	int disk1offs,disk1len;
	int disk2offs;
	int i,n;

	int huffsize;
	int tableidx;
	int byteidx;
	int unhufcnt;
	int pixelcnt;
	int state;

	unsigned char mask;
	unsigned char byte;
	unsigned int unpackedsize;
	int max_stipple;
	unsigned char pl_lut[128];	// lookup table for left pixels
	unsigned char pr_lut[128];	// lookup table for right pixels
	unsigned char xorbuf[MAXPICWIDTH*2];	// ring buffer, to perform an XOR over two lines of stipples
	unsigned char rgbbuf[16];		// RGB values are 6 bits wide. 2 bits red, 2 bits green, 2 bits blue. 
	unsigned char last_stipple;
	int state_cnt;
	int height,width;

	picnum&=0xffff;
	pPicture->halftones=1;	// this format offers half tones.

	indexlen=READ_INT32BE(gfxbuf, 4);
	disk1len=READ_INT32BE(gfxbuf, 8);
//	disk2len=READ_INT32BE(gfxbuf,12);

	indexoffs=16;
	disk1offs=indexoffs+indexlen;
	disk2offs=disk1offs+disk1len;

	retval=0;

	// step 1: find the offset of the picture within the index.
	// the way it is stored is that the offsets within disk1 are stored in the first half,
	// and the offsets for disk2 are in the second half.
	// in case the offset is -1, it must be in the other one.
	offs1=(tVM68k_slong)READ_INT32LE(gfxbuf,indexoffs+picnum*4);
	offs2=(tVM68k_slong)READ_INT32LE(gfxbuf,indexoffs+indexlen/2+picnum*4);

	if (picnum==30 && offs1==-1 && offs2==-1) offs1=0;	// special case: the title screen for the GUILD of thieves is the first picture in DISK1.PIX
	if (offs1!=-1) offset=offs1+disk1offs;			// in case the index was found in the first half, use disk1
	else if (offs2!=-1) offset=offs2+disk2offs;		// in case the index was found in the second half, use disk2
	else return -1;	///  otherwise: ERROR

	
	// the picture is stored in layers.
	// the first layer is a hufman table.
	// this unpacks the second layer, which contains repitions
	// and a "stipple" table. from this, the actual pixels are being 
	// calculated.	

	huffsize=gfxbuf[offset+0];
	unpackedsize=READ_INT16BE(gfxbuf,offset+huffsize+1);
	unpackedsize*=4;
	unpackedsize+=3;
	unpackedsize&=0xffff;	// it was designed for 16 bit machines.

	pixelcnt=-1;
	unhufcnt=0;
	state=0;
	tableidx=0;
	mask=0;
	byteidx=offset+huffsize+2+1;	// the beginning of the bitstream starts after the hufman table and the unpackedsize
	byte=0;
	width=0;
	height=0;
	memset(xorbuf,0,sizeof(xorbuf));	// initialize the xor buffer with 0
	state_cnt=0;
	max_stipple=last_stipple=0;

	while (unhufcnt<unpackedsize && (pixelcnt<(2*width*height)))
	{
		// first layer: the bytes for the unhuf buf are stored as a bitstream, which are used to traverse a hufman table.
		unsigned char bleft,bright,b;
		if (mask==0)
		{
			byte=gfxbuf[byteidx];
			byteidx++;
			mask=0x80;			// MSB first
		}
		bleft =gfxbuf[offset+1+tableidx*2+0];
		bright=gfxbuf[offset+1+tableidx*2+1];
		b=(byte&mask)?bleft:bright;		// when the bit is =1, go left.
		mask>>=1;				// MSB first.
		if (b&0x80)		// leaves have the highest bit set. terminal symbols only have 7 bit.
		{
			tableidx=0;
			b&=0x7f;	// terminal symbols have 7 bit
			//
			//
			// the second layer begins here
			switch (state)
			{
				case 0:	// first state: the ID should be "0x77"
					if (b!=0x77)	return -1;	// illegal format
					state=1;
				break;
				case 1: // second byte is the number of "stipples"
					max_stipple=b;
					state=2;
				break;
				case 2:	// width, stored as 2*6 bit Big Endian
					width<<=6;	// 2*6 bit. big endian;
					width|=b&0x3f;
					state_cnt++;
					if (state_cnt==2)	state=3;
				break;
				case 3:	// height, stored as 2*6 bit Big Endian
					height<<=6;	// 2*6 bit. big endian;
					height|=b&0x3f;
					state_cnt++;
					if (state_cnt==4)	
					{
						if (height<=0 || width<=0) 	return -2;	// error in decoding the height and the width
						pixelcnt=0;
						state_cnt=0;
						state=4;
					}
				break;
				case 4:	// rgb values
					rgbbuf[state_cnt++]=b;
					if (state_cnt==16) 
					{
						state_cnt=0;
						state=5;
					}
				break;
				case 5:	// lookup-table to retrieve the left pixel value from the stipple
					pl_lut[state_cnt++]=b;
					if (state_cnt==max_stipple)
					{
						state_cnt=0;
						state=6;
					}
				break;
				case 6:	// lookup-table to retrieve the right pixel value from the stipple
					pr_lut[state_cnt++]=b;
					if (state_cnt==max_stipple)
					{
						last_stipple=0;
						state_cnt=0;
						state=7;
					}
				break;
				case 7:	
				case 8:
					// now for the stipple table
					// this is actually a third layer of encoding.
					// it contains terminal symbols [0... max_stipple)
					// 
					// if the symbol is <max_stipple, it is a terminal symbol, a stipple
					// if the symbol is =max_stipple, it means that the next byte is being escaped
					// if the symbol is >max_stipple, it means that the previous symbol is being repeated.
					n=0;
					if (state==8)	// this character has been "escaped"
					{
						state=7;
						n=1;
						last_stipple=b;
					}
					else if (b<max_stipple)
					{
						last_stipple=b;	// store this symbol for the next repeat instruction	
						n=1;
					} else {
						if (b==max_stipple)	// "escape" the NEXT symbol. use it, even though it might be >=max_stipple.
						{			// this is necessary for the XOR operation.
							state=8;
							n=0;
						} else if (b>max_stipple) 
						{
							n=b-max_stipple;	// repeat the previous stipple
							b=last_stipple;
						}
					}
					for (i=0;i<n;i++)
					{
						unsigned char x;
						xorbuf[state_cnt]^=b;			// descramble the symbols
						x=xorbuf[state_cnt];
						state_cnt=(state_cnt+1)%(2*width);
						pPicture->pixels[pixelcnt++]=pl_lut[x];
						pPicture->pixels[pixelcnt++]=pr_lut[x];
					}
				break;
			}
		} else {
			tableidx=b;	// non terminal -> traverse the tree further down
		}
	}
	pPicture->height=height;
	pPicture->width=width*2;
	// the other image formats have 9 bit wide rgb values.
	for (i=0;i<16;i++)
	{
		unsigned int red,green,blue;
		red  =(rgbbuf[i]>>4)&0x3;
		green=(rgbbuf[i]>>2)&0x3;
		blue =(rgbbuf[i]>>0)&0x3;

		red*=  0x7;
		green*=0x7;
		blue*= 0x7;
		
		red/=  0x3;
		green/=0x3;
		blue/= 0x3;
	
		red&=0x7;green&=0x7;blue&=0x7;

		pPicture->palette[i] =(  red<<8);
		pPicture->palette[i]|=(green<<4);
		pPicture->palette[i]|=( blue<<0);
	}
	

	return retval;
}


int gfxloader_unpackpic(tVM68k_ubyte* gfxbuf,tVM68k_ulong gfxsize,tVM68k_ubyte version,int picnum,tVM68k_ubyte* picname,tPicture* pPicture)
{
	int retval;

	retval=0;

	if (gfxbuf==NULL || pPicture==NULL) return -1;
	if (gfxbuf[0]=='M' && gfxbuf[1]=='a' && gfxbuf[2]=='P' && gfxbuf[3]=='i') retval=gfxloader_gfx1(gfxbuf,gfxsize,version,picnum,pPicture);
	if (gfxbuf[0]=='M' && gfxbuf[1]=='a' && gfxbuf[2]=='P' && gfxbuf[3]=='2') retval=gfxloader_gfx2(gfxbuf,gfxsize,version,picname,pPicture);
	if (gfxbuf[0]=='M' && gfxbuf[1]=='a' && gfxbuf[2]=='P' && gfxbuf[3]=='3') retval=gfxloader_gfx3(gfxbuf,gfxsize,version,picnum,pPicture);
	return retval;
}
int gfxloader_picture_calcxpmsize(tPicture* pPicture,int* xpmsize)
{
	int calcsize;

	if (pPicture==NULL || xpmsize==NULL) return -1;
	calcsize=345;	// header of the xpm file, including the palette.
	calcsize+=pPicture->height*(pPicture->width+1+1+2)+3;	// each line with the pixels starts and ends with "". in all but 1 cases, there is a ,\n at the end. the last line consists of a };\n\0. 
	*xpmsize=calcsize;
	return 0;

}
int gfxloader_picture2xpm(tPicture* pPicture,char* xpm,int xpmspace)
{
	int row;
	int col;
	int calcsize;
	int pixelidx;
	int xpmidx;
	int i;

	if (xpm==NULL) return -1;	// invalid pointer
	
	if (gfxloader_picture_calcxpmsize(pPicture,&calcsize)) return -1;
	if (xpmspace<calcsize) return -2;// too small

	xpmidx=0;
	snprintf(&xpm[xpmidx],76,"/* XPM */\nstatic char *xpm[] = {\n/* columns rows colors chars-per-pixel */\n");	// header
	xpmidx+=75;
	snprintf(&xpm[xpmidx],18,"\"%3d %3d 16 1 \",\n",pPicture->width,pPicture->height);	// the parameters
	xpmidx+=17;
	for (i=0;i<16;i++)
	{
		int red,green,blue;
		red=(pPicture->palette[i]>>8)&0xf;
		green=(pPicture->palette[i]>>4)&0xf;
		blue=(pPicture->palette[i]>>0)&0xf;
		snprintf(&xpm[xpmidx],19,"\"%c c #%02X%02X%02X\",\n",i+'A',red*0x20,green*0x20,blue*0x20);
		xpmidx+=15;
	}
	snprintf(&xpm[xpmidx],14,"/* pixels */\n");
	xpmidx+=13;
	pixelidx=0;
	for (row=0;row<pPicture->height;row++)
	{
		xpm[xpmidx++]='"';
		for (col=0;col<pPicture->width;col++)
		{
			xpm[xpmidx++]='A'+pPicture->pixels[pixelidx++];
		}
		xpm[xpmidx++]='"';
		if (row!=pPicture->height-1) xpm[xpmidx++]=',';
		xpm[xpmidx++]='\n';
	}
	xpm[xpmidx++]='}';xpm[xpmidx++]=';';xpm[xpmidx++]='\n',xpm[xpmidx]=0;
	if (xpmidx>=xpmspace) 
	{
		fprintf(stderr,"ERROR! POSSIBLE INTERNAL MEMORY VIOLATION DETECTED in gfxloader_picture2xpm\n");
	}
	return 0;
}
