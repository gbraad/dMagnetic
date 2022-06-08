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

// the purpose of this file is to read the MS DOS Game binaries,
// and to translate them into the .mag/.gfx format.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "loader_common.h"
#include "vm68k_macros.h"

#define	READ_INT24LE(ptr,idx)	(\
	(((unsigned int)((ptr)[((idx)+2)])&0xff)<<16)	|\
	(((unsigned int)((ptr)[((idx)+1)])&0xff)<< 8)	|\
	(((unsigned int)((ptr)[((idx)+0)])&0xff)<< 0)	|\
	0)


// most of the time, a huffman tree encoding was employed.
// It would start with 1 byte treesize, then the branches in the order LRLRLRLR... (Left Right)
// and then the bitstream.
// within the tree, there are branches and terminal symbols.
// the terminal symbols have bit 7 set, and are thus smaller than 8 bit.
// to be able to encode full 8 bit bytes, 4 symbols are being combined into 3 bytes.
int loader_common_unhuffer(unsigned char* input,int length,unsigned char* output)
{
	int outputidx;
	unsigned char byte;
	unsigned char mask;
	int bitidx;
	int treesize;
	int treeidx;
	int threecnt;

	treeidx=0;
	treesize=input[0];
	bitidx=3+treesize;	// start decoding the bitstream directly after the tree
	threecnt=0;	
	outputidx=0;
	mask=0;
	byte=0;
	while (bitidx<length || mask)
	{
		unsigned char branchl,branchr;
		unsigned char branch;

		branchl=input[1+2*treeidx];
		branchr=input[2+2*treeidx];
		if (mask==0)
		{
			mask=0x80;
			byte=input[bitidx++];
		}
		branch=(byte&mask)?branchl:branchr;
		mask>>=1;
		
		if (branch&0x80)	// the branch was a terminal symbol.
		{
			branch&=0x7f;
			if (threecnt==3)	// this is the fourth symbol. distribute the bits within this symbol to the previous three.
			{
				output[outputidx-3]|=((branch>>4)&0x3)<<6;
				output[outputidx-2]|=((branch>>2)&0x3)<<6;
				output[outputidx-1]|=((branch>>0)&0x3)<<6;
				threecnt=0;
			} else {
				output[outputidx++]=branch;
				threecnt++;
			}
			treeidx=0;
		} else {
			treeidx=branch;	// follow the branch
		}
	}
	return outputidx;
}
int loader_common_addmagheader(unsigned char* magbuf,int magsize,int version,int codesize,int string1size,int string2size,int dictsize,int huffmantreeidx)
{
	if (huffmantreeidx==-1)
	{
		huffmantreeidx=string1size;
	}
	if (string1size>0x10000)
	{
		int x;
		x=string1size+string2size;
		string1size=0x10000;
		string2size=x-string1size;
	}
	magbuf[0]='M';magbuf[1]='a';magbuf[2]='S';magbuf[3]='c';        //  0.. 3: the magic word
	WRITE_INT32BE(magbuf, 4 ,magsize);                               //  4.. 7: the total size
	WRITE_INT32BE(magbuf, 8 ,42);                           //  8..11: the size of the header
	WRITE_INT16BE(magbuf,12 ,version);  			// 12..13: the version for the virtual machine
	WRITE_INT32BE(magbuf,14 ,codesize);          // 14..17 the size of the game code
	WRITE_INT32BE(magbuf,18 ,string1size);                  // 18..21 the size of the string1
	WRITE_INT32BE(magbuf,22 ,string2size);                  // 22..25 the size of the string2
	WRITE_INT32BE(magbuf,26 ,dictsize);                     // 26..29 the size of the dictionary
	WRITE_INT32BE(magbuf,30 ,huffmantreeidx)                // 30..33 the beginning of the huffman tree within the string buffer
	WRITE_INT32BE(magbuf,34 ,0);                            //  34..37: undo size
	WRITE_INT32BE(magbuf,38 ,0);                            //  38..41: undo pc
	return LOADER_OK;
}
