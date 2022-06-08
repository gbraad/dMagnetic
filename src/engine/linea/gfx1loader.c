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
#include "gfx1loader.h"
#include "linea.h"	// for the picture
#include <string.h>
#include <stdio.h>

int gfxloader_gfx1(tVM68k_ubyte* gfxbuf,tVM68k_ulong gfxsize,tVM68k_ubyte version,int picnum,tPicture* pPicture)
{
	int i;
	int retval;

	tVM68k_ulong picoffs;
	tVM68k_uword	width;
	tVM68k_uword	height;
	tVM68k_uword tablesize;
	tVM68k_ulong datasize;
	
	tVM68k_uword tableidx;
	tVM68k_ulong byteidx;
	tVM68k_ubyte bitidx;
	tVM68k_ubyte curbyte;
	tVM68k_ubyte curpixel;
	
	pPicture->pictureType=PICTURE_DEFAULT;
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
					nxt>>=8;
				}
				tableidx=(nxt)&0xff;	// bit was not set. read the lower 8 bits.
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
	pPicture->pictureType=PICTURE_DEFAULT;	// only gfx3 offers halftone pictures
	// the gfx2 buffer starts with the magic value, and then a directory
	directorysize=READ_INT16BE(gfxbuf,4);

	retval=0;	
	// step 1: find the correct filename
	found=0;
	offset=-1;
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
		}
	}
	// TODO: sanity check. is length-48==height*width/2?
	if (found && offset!=-1)
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

	if (!gfxsize) return 0;		// there is no picture data available. nothing to do.

	picnum&=0xffff;
	pPicture->pictureType=PICTURE_HALFTONE;	// this format offers half tones.

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
	if (picnum!=30 && offs1!=-1 && offs2!=-1) offs2=-1;	// in case one picture is stored on both disks, prefer the first one.

	if (picnum==30 && offs1==-1 && offs2==-1) offs1=0;	// special case: the title screen for the GUILD of thieves is the first picture in DISK1.PIX
	if (offs1!=-1) offset=offs1+disk1offs;			// in case the index was found in the first half, use disk1
	else if (offs2!=-1) offset=offs2+disk2offs;		// in case the index was found in the second half, use disk2
	else return -1;	///  otherwise: ERROR


	if (offset>gfxsize) 	// this is MYTH: there is only a single image file.
	{
		offset=offs1;		
	}
	
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

	while (unhufcnt<unpackedsize && (pixelcnt<(2*width*height)) && byteidx<gfxsize)
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
		// the rgb palette for the halftone images has only 2 bits dynamic, whereas the other images have 3 bit.
		// the renderer will have to take care of that.
		red&=0x7;green&=0x7;blue&=0x7;

		pPicture->palette[i] =(  red<<8);
		pPicture->palette[i]|=(green<<4);
		pPicture->palette[i]|=( blue<<0);
	}
	

	return retval;
}
// the gfx4 format is used to handle the pictures from the Magnetic Windows system.
// just like the gfx2, it starts with a directory. 6 byte picname, 4 bytes (little endian) offset, 4 bytes (little endian) length.
// at the offset, the picture is being comprised of the tree (type 7) and image (type 6) from the Magnetic Windows resource files.
// the tree is ALWAYS 609 bytes long. The size of the image varies.
//
// the huffman tree uses 9 bits to encode 8 bits: the first 32 bytes are a bitmask (MSB first). Then the branches/symbols follow. 
// if the bit from the bitmask is set, it is a terminal symbol.
// 0x00...0x1f:  LEFTMASK		(0)
// 0x20...0x11f: LEFTBRANCH
// 0x120..0x13f: RIGHTMASK		(1)
// 0x140..0x23f: RIGHTBRANCH
// Byte 0x240    escape character (for run level encoding)
// Byte 0x241...0x250: EGA palette
// Byte 0x251...0x260: EGA palette pairs
//
//
// the image data looks like this:
// 0x00..0x03: magic header
// 0x04..0x23: 16x2 bytes RGB values (4 bits per channel, little endian, 0x0rgb)
// 0x24..0x25: width
// 0x26..0x27: height
// 0x28..0x29: transparency placeholder
// 0x2a..0x2b: size
// 0x2c......: bit stream. MSB first. when the bit is set (=1), follow the RIGHT branch.
//
// the run level encoding is signalled by the escape character from the tree (byte 0x240).
// in case the next character is 0xff, it actually an honest escape character.
// if any other value follows, it is the repeat num.
// the character AFTER that one is being repeated (4+repeat) number of times.
//
// each symbol is 8 bits large, 4 bits are the pixel. CAREFUL: bits 0..3 are the left, bits 4..7 are the right pixel
// in case the width is not divisible by 2, bits 4..7 are being ignored.
//
// the xor is being performed line by line.
//
// note that the end of the image comes BEFORE the end of the bitstream. (due to a bug in the encoder)
int gfxloader_gfx4(tVM68k_ubyte* gfxbuf,tVM68k_ulong gfxsize,tVM68k_ubyte version,tVM68k_ubyte* picname,tPicture* pPicture,int egamode)
{
#define	SIZEOFTREE	609
	int directorysize;
	int retval;
	int found;
	int i;
	int offset,length;
	int offset_vanilla,length_vanilla;
	int offset_ega,length_ega;
	int offset_anim,length_anim;
	int j;
	pPicture->width=0;
	pPicture->height=0;
	pPicture->pictureType=PICTURE_DEFAULT;
	// the gfx4 buffer starts with the magic value, and then a directory
	retval=0;
	found=0;
	directorysize=READ_INT16BE(gfxbuf,4);	
	offset=offset_ega=offset_anim=offset_vanilla=-1;
	length=length_ega=length_anim=length_vanilla=-1;
	for (i=0;i<directorysize && !found;i+=14)
	{
		tVM68k_ubyte c1,c2;
		found=1;
		j=0;
		do
		{
			c1=gfxbuf[6+i+j];
			c2=picname[j];
			if ((c1&0x5f)!=(c2&0x5f)) found=0;
			if ((c1&0x5f)==0) j=6;	// end of entry reached.
			j++;
		} while (j<6 && found);
		if (found)
		{
			int ega;
			int stillimage;
#define	STILLMAGIC	0x00005ed0
			const unsigned short egapalette[16]={0x000,0x005,0x050,0x055, 0x500,0x505,0x550,0x555, 0x222,0x007,0x070,0x077, 0x700,0x707,0x770,0x777};
			offset=READ_INT32LE(gfxbuf,i+6+6);
			length=READ_INT32LE(gfxbuf,i+6+10);

			// check if the image is not the ega image, and it is not an animation background.	

			// the EGA images have a specific RGB palette
			// if not, it is not a EGA image
			ega=16;
			for (j=0;j<16;j++)
			{
				if (READ_INT16LE(gfxbuf,offset+SIZEOFTREE+4+2*j)!=egapalette[j]) ega--;	
			}
			ega=(ega>=15);

			stillimage=1;
			if (READ_INT32LE(gfxbuf,offset+length-4)!=STILLMAGIC) stillimage=0;		// if they do not, it is an animation
			found=0;
			if (!ega && stillimage)
			{
				offset_vanilla=offset;
				length_vanilla=length;
			}

			if (ega) 
			{
				offset_ega=offset;
				length_ega=length;
			}
			if (!stillimage)
			{
				offset_anim=offset;
				length_anim=length;
			}
		}
	}
	// in case the image was not found
	offset=-1;
	length=-1;
	if (egamode)
	{
		offset=offset_ega;
		length=length_ega;
	}
	if (offset==-1)
	{
		offset=offset_vanilla;
		length=length_vanilla;
	}
	if (offset==-1)
	{
		offset=offset_anim;
		length=length_anim;
	}
	if (offset==-1)
	{
		offset=offset_ega;
		length=length_ega;
	}
	if (offset!=-1 && length!=-1) found=1;
	
	if (found)
	{
		int treestart;
		int picstart;
		int size;
		int treeidx;
		int repnum;
		int rlestate;	// for the run length encoding. state 0: if not the escape char, output. otherwise -> state 1. in state 1, if the symbol is 0xff, the output is the escapechar. otherwise, the repition number (sans 4) -> state 4. in state 4, repeat the symbol	
		tVM68k_ubyte escapechar;
		tVM68k_ubyte byte;
		tVM68k_ubyte mask;
		treestart=offset+0;
		picstart=offset+SIZEOFTREE;


		// byte 0x240 in the tree is the escape symbol for the run level encoding
		escapechar=gfxbuf[treestart+0x240];
		// bytes 0x04..0x23: RGB values
		for (i=0;i<16;i++)
		{
			pPicture->palette[i]=READ_INT16LE(gfxbuf,picstart+0x4+i*2);
		}
		// bytes 0x24,0x25= width
		// bytes 0x26,0x27= height
		pPicture->width=READ_INT16LE(gfxbuf,picstart+0x24);
		pPicture->height=READ_INT16LE(gfxbuf,picstart+0x26);
		// bytes 0x2a,0x2b= size of the bitstream (in bytes)
		size=READ_INT16LE(gfxbuf,picstart+0x2a);
		j=0;
		treeidx=0;
		mask=0;byte=0;
		i=0;	
		rlestate=0;
		repnum=1;

		// i is counting up the bytes in the bitstream.
		// j is counting up the pixels of the image		
		while (((i<(length-SIZEOFTREE) && i<size ) || mask ) && j<(pPicture->width*pPicture->height))
		{
			tVM68k_ubyte lterm,rterm,term;
			tVM68k_ubyte lbranch,rbranch,branch;
			// the bitmask is denoting (MSB first) if an entry is a terminal symbol (=1) or a branch (=0)
			lterm=gfxbuf[treestart+0x00 +treeidx/8]&(0x80>>(treeidx%8));
			rterm=gfxbuf[treestart+0x120+treeidx/8]&(0x80>>(treeidx%8));

			// the entry in the table could either be a branch or a terminal symbol
			lbranch=gfxbuf[treestart+0x20 +treeidx];
			rbranch=gfxbuf[treestart+0x140+treeidx];

			if (mask==0)
			{
				mask=0x80;
				byte=gfxbuf[picstart+i+0x2c];
				i++;
			}
		
			term  =(byte&mask)?  rterm:lterm;
			branch=(byte&mask)?rbranch:lbranch;
			mask>>=1;
		

			if (term)
			{
				if (rlestate==0)
				{
					if (branch==escapechar) 
					{
						rlestate=1;
					} else {
						repnum=1;	
					}
				} else if (rlestate==1) {
					if (branch==0xff)
					{
						branch=escapechar;
						repnum=1;
						rlestate=0;
					} else {
						repnum=branch+4;	// this form of RLE makes sense when the same byte was repeated 4 or more times in the source picture
						rlestate=2;
					}
				} else if (rlestate==2) {
					rlestate=0;	// the current entry is a terminal symbol, which is going to be repeated
				}
				if (rlestate==0)
				{
					while (repnum && j<(pPicture->width*pPicture->height))
					{
						// the lower 4 bits are the LEFT pixel
						pPicture->pixels[j]=branch&0xf;
						j++;
						// one byte holds two pixels. but when the width is not divisible by 2, drop the remaining nibble.
						if (j%pPicture->width) // the higher 4 bits are the RIGHT pixel; but only if it is not outside the scope of the image.
						{
							pPicture->pixels[j]=(branch>>4)&0xf;	// the higher 4 bytes are the RIGHT pixel
							j++;
						}
						repnum--;
					}
				}
				treeidx=0;	// go back to the start;
			} else {	// not a terminal symbol. keep following the branches
				treeidx=branch;
			}
		}
		// the finishing touch: XOR each line with the previous one
		for (i=pPicture->width;i<pPicture->height*pPicture->width;i++)
		{
			pPicture->pixels[i]^=pPicture->pixels[i-pPicture->width];
		}
	}
	

	return retval;
}

int gfxloader_gfx5(unsigned char* gfxbuf,int gfxsize,int version,int picnum,tPicture* pPicture)
{
	unsigned char tmpbuf[6080+760+760];	// maximum size for a picture. plus room for the threebuf
	unsigned char colour[4]={0};
	int format;
	int i;
	int offs;


	unsigned char rgbvalues[16][3]={
		{  0,  0,  0},
		{255,255,255},
		{129, 51, 56},
		{117,206,200},

		{142, 60,151},
		{ 86,172,77},
		{ 46, 44,155},
		{237,241,113},

		{142, 80, 41},
		{ 85, 56,  0},
		{196,108,113},
		{ 74, 74, 74},

		{123,123,123},
		{169,255,159},
		{112,109,235},
		{178,178,178}
	};


	offs=READ_INT32BE(gfxbuf,4+4*picnum);
	pPicture->width=160*2;	// make it twice as wide as it actually is.
	pPicture->height=152;
	pPicture->pictureType=PICTURE_C64;	// only gfx3 offers halftone pictures
	format=0;

	for (i=0;i<16;i++)
	{
		unsigned int red,green,blue;
		red=rgbvalues[i][0];
		green=rgbvalues[i][1];
		blue=rgbvalues[i][2];

		red*=8;green*=8;blue*=8;
		red/=255;green/=255;blue/=255;
		red&=0xf;green&=0xf;blue&=0xf;
		pPicture->palette[i]=(red<<8)|(green<<4)|(blue);
	}


	///////////// dehuff /////////////
	{
		int outcnt;
		int expected;
		int treeidx;
		int bitcnt;
		int byteidx;
		int threecnt;
		int rlenum;
		int rlecnt;
		unsigned char rlebuf[256];
		unsigned char threebuf[3]={0};
		unsigned char* ptr;
		unsigned char byte=0;
		unsigned char rlechar;

		treeidx=0;
		expected=6080;
		outcnt=-1;
		bitcnt=0;
		byteidx=127;
		ptr=&gfxbuf[offs];
		threecnt=0;
		rlechar=0;

		rlenum=0;
		rlecnt=0;

		while (outcnt<expected)
		{
			unsigned char branchl,branchr,branch;
			if (bitcnt==0)
			{
				bitcnt=8;
				byte=ptr[byteidx++];
			}
			branchl=ptr[2*treeidx+1];
			branchr=ptr[2*treeidx+2];
			branch=(byte&0x80)?branchl:branchr;
			byte<<=1;bitcnt--;
			if (branch&0x80)
			{
				treeidx=branch&0x7f;
			} else {
				treeidx=0;
				if (threecnt==3)
				{
					int i;
					threecnt=0;
					for (i=0;i<3;i++) 
					{
						threebuf[i]|=((branch<<2)&0xc0);
						branch<<=2;
					}
					if (outcnt==-1)
					{
						outcnt=0;
						if (version==0) 	// PAWN specific
						{
							colour[0]=threebuf[0]&0xf;	// for when the bitmask is 00
							colour[3]=threebuf[1]&0xf;	// for when the bitmask is 11
							rlenum=threebuf[2];
							expected=6144+760;
						} else {
							format=threebuf[0];
							if (threebuf[0]==0x00)
							{
								expected=6080+380+760;	// after the bitmask comes the colourmap
								rlenum=0;
								rlecnt=0;
								rlechar=tmpbuf[outcnt++]=threebuf[2];
							} else {
								colour[0]=threebuf[1]&0xf;	// for when the bitmask is 00
								colour[3]=threebuf[1]&0xf;	// for when the bitmask is 11
								expected=6080+760+760;	// after the bitmask comes the colourmap
								rlecnt=0;
								rlenum=threebuf[2];
							}
						}
					} else {
						for (i=0;i<3;i++)
						{
							if (rlecnt<rlenum) 
							{
								rlebuf[rlecnt++]=threebuf[i];
							} else {
								int j;
								int rle;
								rle=0;
								for (j=0;j<rlecnt;j++)
								{
									if (rlebuf[j]==threebuf[i]) rle=(j+1);
								}
								if (rle)
								{
									for (j=0;j<rle;j++)
									{
										if (outcnt<expected) tmpbuf[outcnt++]=rlechar;
									}
								} else {
									if (outcnt<expected) rlechar=tmpbuf[outcnt++]=threebuf[i];
								}
							}
						}
					}
				} else {
					threebuf[threecnt++]=branch;
				}
			}
		}	
	}

	///////////// render the picture ///////////////

	{
		int colidx;
		int maskidx;
		int x,y;
		int i,j;

		x=0;
		y=0;
		for (maskidx=0,colidx=0;maskidx<6080;maskidx+=8,colidx++)
		{
			// prepare everything for rendering a 4x8 block
			
			if (version==0)
			{
				colour[1]=(tmpbuf[6144+colidx]>>4)&0xf;
				colour[2]=(tmpbuf[6144+colidx]>>0)&0xf;
			} else if (format==0x00) {
				colour[1]=(tmpbuf[6080+380+colidx]>>4)&0xf;
				colour[2]=(tmpbuf[6080+380+colidx]>>0)&0xf;
				if ((colidx%2)==0)
				{
					colour[3]=(tmpbuf[6080+colidx/2]>>4)&0xf;
				} else {
					colour[3]=(tmpbuf[6080+colidx/2]>>0)&0xf;
				}
			} else {
				colour[1]=(tmpbuf[6080+760+colidx]>>4)&0xf;
				colour[2]=(tmpbuf[6080+760+colidx]>>0)&0xf;
				colour[3]=(tmpbuf[6080+colidx]&0xf);
			}
			for (i=0;i<8;i++)
			{
				int y2;
				unsigned char mask;
				y2=y+i;
				mask=tmpbuf[maskidx+i];
				for (j=0;j<4;j++)
				{
					int x2;
					unsigned char col;
					x2=x+j;
					col=colour[(mask>>6)&0x3];
					pPicture->pixels[0+2*x2+y2*(pPicture->width)]=col;	// render the picture T W I C E as wide as it is.
					pPicture->pixels[1+2*x2+y2*(pPicture->width)]=col;
					mask<<=2;	
				}
			}
			x+=4;
			if (x==160) 
			{
				x=0;
				y+=8;
			}
		}	
	}
	return 0;
}




int gfxloader_unpackpic(tVM68k_ubyte* gfxbuf,tVM68k_ulong gfxsize,tVM68k_ubyte version,int picnum,tVM68k_ubyte* picname,tPicture* pPicture,int egamode)
{
	int retval;

	retval=0;
	picnum&=0x3f;	// there are no more than 30 pictures in each game. except Wonderland. 

	if (gfxbuf==NULL || pPicture==NULL) return -1;
	if (gfxbuf[0]=='M' && gfxbuf[1]=='a' && gfxbuf[2]=='P')
	{
		if (gfxbuf[3]=='i') retval=gfxloader_gfx1(gfxbuf,gfxsize,version,picnum,pPicture);
		if (gfxbuf[3]=='2') retval=gfxloader_gfx2(gfxbuf,gfxsize,version,picname,pPicture);
		if (gfxbuf[3]=='3') retval=gfxloader_gfx3(gfxbuf,gfxsize,version,picnum,pPicture);
		if (gfxbuf[3]=='4') retval=gfxloader_gfx4(gfxbuf,gfxsize,version,picname,pPicture,egamode);
		if (gfxbuf[3]=='5') retval=gfxloader_gfx5(gfxbuf,gfxsize,version,picnum,pPicture);
	}
	return retval;
}

