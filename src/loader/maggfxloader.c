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

// the purpose of this file is to figure out what kind of binaries the 
// user has. is it the .mag/.gfx one? or is it the original MS-DOS version?
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "configuration.h"
#include "vm68k_macros.h"

typedef struct _tGameInfo
{
	char prefix[8];	// the prefix for the game's binaries.
	int disk1size;	// the size of the DISK1.PIX file is in an indicator for the game being used.
	int version;	// the interpreter version.
} tGameInfo;
// TODO: so far, i have understood the formats for PAWN, THE GUILD OF THIEVES and JINXTER
// The others have some opcodes which I can not decode (yet)
#define	KNOWN_GAMES	5	
const tGameInfo gameInfo[KNOWN_GAMES]={
	{"PAWN",		209529,0},
	{"GUILD",		185296,1},
	{"JINX",		159027,2},
	{"CORR",		160678,3},
	{"FILE",		162541,3}
};

int loader_huffman_unpack(char* magbuf,int* decodedbytes,FILE *f)
{
	unsigned char huffsize;
	unsigned char hufftab[256];
	int huffidx;
	unsigned short todo;
	unsigned char byte;
	unsigned char mask;
	unsigned char tmp[3];
	int threebytes;
	int n;	// this variable is getting rid of some compiler warnings.
	int magidx;

	magidx=0;

	n=fread(&huffsize,sizeof(char),1,f);
	n+=fread(hufftab,sizeof(char),huffsize,f);
	n+=fread(&todo,sizeof(short),1,f);	// what are those two bytes?
	mask=0;
	huffidx=0;
	threebytes=0;
	while (!feof(f) || mask)
	{
		unsigned char branchr,branchl;
		unsigned char branch;
		if (mask==0)
		{
			n+=fread(&byte,sizeof(char),1,f);
			mask=0x80;
		}
		branchl=hufftab[2*huffidx+0];
		branchr=hufftab[2*huffidx+1];
		branch=(byte&mask)?branchl:branchr;
		if (branch&0x80)	// the highest bit signals a terminal symbol.
		{
			huffidx=0;
			branch&=0x7f;
			// the terminal symbols from the tree are only 6 bits wide.  
			// to extend this to 8 bits, the fourth symbol contains the
			// two MSB for the previous three:
			// 00AAAAAA 00BBBBBB 00CCCCCC 00aabbcc
			if (threebytes==3)
			{
				int i;
				threebytes=0;
				for (i=0;i<3;i++)
				{
					// add the 2 MSB to the existing 6.
					tmp[i]|=((branch<<2)&0xc0);	// MSB first
					magbuf[magidx]|=(char)tmp[i];
					magidx++;
					branch<<=2;	// MSB first
				}
			} else {
				tmp[threebytes]=branch;
				threebytes++;
			}
		} else {
			huffidx=branch;
		}
		mask>>=1;
	}
	*decodedbytes=magidx;
	return (n==0);
}
int loader_msdos(char* msdosdir,
		char *magbuf,int* magsize,
		char* gfxbuf,int* gfxsize)
{
#define	OPENFILE(filename)	\
	f=fopen((filename),"rb");	\
	if (!f)		\
	{	\
		fprintf(stderr,"ERROR. Unable to open %s. Sorry.\n",(filename));	\
		return -1;	\
	}

	FILE *f;
	char filename[1024];
	int i;
	int gameid;
	int magsize0;
	int gfxsize0;

	magsize0=*magsize;
	gfxsize0=*gfxsize;
	*magsize=0;
	*gfxsize=0;

	// clear the headers.
	memset(gfxbuf,0,16);
	memset(magbuf,0,42);	

	{
		int sizedisk1,sizedisk2,sizeindex;
		/////////////////////// GFX packing
		// the header of the GFX is always 16 bytes.
		// values are stored as BigEndians
		//  0.. 3 are the magic word 'MaP3'
		//  4.. 7 are the size of the GAME4 (index) file (always 256)
		//  8..11 are the size of the DISK1.PIX file
		// 12..15 are the size of the DISK2.PIX file
		// then the INDEX file (beginning at 16)
		// then the DISK1.PIX file
		// then the DISK2.PIX file
		// step 1: find out which game it is.
		snprintf(filename,1024,"%s/DISK1.PIX",msdosdir);
		OPENFILE(filename);

		sizedisk1=fread(&gfxbuf[16+256],sizeof(char),gfxsize0-16-256,f);
		fclose(f);
		gameid=-1;
		for (i=0;i<KNOWN_GAMES;i++)
		{
			if (sizedisk1==gameInfo[i].disk1size) gameid=i;
		}
		if (gameid<0 || gameid>=KNOWN_GAMES)
		{
			fprintf(stderr,"ERROR: Unable to recognize game\n");
			return -2;
		}
		// step 2: read the binary files, and store them in the gfxbuffer
		snprintf(filename,1024,"%s/DISK2.PIX",msdosdir);
		OPENFILE(filename);
		sizedisk2=fread(&gfxbuf[16+256+sizedisk1],sizeof(char),gfxsize0-sizedisk1-16-256,f);
		fclose(f);
		snprintf(filename,1024,"%s/%s4",msdosdir,gameInfo[gameid].prefix);
		OPENFILE(filename);
		sizeindex=fread(&gfxbuf[16],sizeof(char),256,f);
		fclose(f);
		// step 3: add the header to the gfx buffer
		gfxbuf[0]='M';gfxbuf[1]='a';gfxbuf[2]='P';gfxbuf[3]='3';
		WRITE_INT32BE(gfxbuf, 4,sizeindex);
		WRITE_INT32BE(gfxbuf, 8,sizedisk1);
		WRITE_INT32BE(gfxbuf,12,sizedisk2);
		*gfxsize=16+sizeindex+sizedisk1+sizedisk2;
	}
	////////////////////////// done with GFX packing

	////////////////////////// MAG packing
	{
		int codesize=0;
		int dictsize=0;
		int string0size=0;
		int string1size=0;
		int string2size=0;
		int magidx=0;

		magidx=42;
		magbuf[0]='M';magbuf[1]='a';magbuf[2]='S';magbuf[3]='c';
		WRITE_INT32BE(magbuf,8,magidx);
		// the program for the 68000 machine is stored in the file ending with 1.
		snprintf(filename,1024,"%s/%s1",msdosdir,gameInfo[gameid].prefix);
		OPENFILE(filename);
		if (gameInfo[gameid].version>=3)	// beginning with version 3, it is huffman-encoded.
		{
			if (loader_huffman_unpack(&magbuf[magidx],&codesize,f)) return -1;
		} else {
			codesize=fread(&magbuf[magidx],sizeof(char),magsize0-magidx,f);
		}
		fclose(f);
		magidx+=codesize;
		// the strings for the game are stored in the files ending with 3 and 2
		snprintf(filename,1024,"%s/%s3",msdosdir,gameInfo[gameid].prefix);
		OPENFILE(filename);
		string1size=fread(&magbuf[magidx],sizeof(char),magsize0-magidx,f);
		fclose(f);
		magidx+=string1size;
		snprintf(filename,1024,"%s/%s2",msdosdir,gameInfo[gameid].prefix);
		OPENFILE(filename);
		string2size=fread(&magbuf[magidx],sizeof(char),magsize0-magidx,f);
		fclose(f);
		magidx+=string2size;


		if (gameInfo[gameid].version>=2)
		{
			// dictionaries are packed. 

			dictsize=0;
			snprintf(filename,1024,"%s/%s0",msdosdir,gameInfo[gameid].prefix);
			OPENFILE(filename);
			if (loader_huffman_unpack(&magbuf[magidx],&dictsize,f)) return -1;
			fclose(f);
			magidx+=dictsize;
		}
		// TODO: what about the 5?

		WRITE_INT32BE(magbuf, 4,codesize+dictsize+string1size+string2size+42);
		WRITE_INT32BE(magbuf,14,codesize);
		string0size=string1size;
		if (string1size>=0x10000) 
		{
			string2size+=(string1size-0x10000);
			string1size=0x10000;
		}
		WRITE_INT32BE(magbuf,18,string1size);
		WRITE_INT32BE(magbuf,22,string2size);
		WRITE_INT32BE(magbuf,26,dictsize);
		WRITE_INT32BE(magbuf,30,string0size);
		WRITE_INT32BE(magbuf,34,0);	// undosize
		WRITE_INT32BE(magbuf,38,0);	// undopc

		magbuf[13]=gameInfo[gameid].version;

		*magsize=magidx;
	}	
	return 0;
}
// the purpose of this function is to use the naming of the "???one.rsc" file as a blueprint.
int loader_mw_substituteOne(char* two_rsc,int num,char* output)
{
	int i;
	int l;
	int onestart;
	int uppercase;
	char *names[10]={"zero","one","two","three","four","five","six","seven","eight","nine"};

	l=strlen(two_rsc);
	onestart=-1;
	uppercase=0;
	for (i=0;i<l-4;i++)
	{
//		if (two_rsc[i+0]=='o'  && two_rsc[i+1]=='n' && two_rsc[i+2]=='e' && two_rsc[i+3]=='.') {onestart=i;uppercase=0;}
//		if (two_rsc[i+0]=='O'  && two_rsc[i+1]=='N' && two_rsc[i+2]=='E' && two_rsc[i+3]=='.') {onestart=i;uppercase=1;}
		if (two_rsc[i+0]=='t'  && two_rsc[i+1]=='w' && two_rsc[i+2]=='o' && two_rsc[i+3]=='.') {onestart=i;uppercase=0;}
		if (two_rsc[i+0]=='T'  && two_rsc[i+1]=='W' && two_rsc[i+2]=='O' && two_rsc[i+3]=='.') {onestart=i;uppercase=1;}
	}
	if (onestart==-1 || num>=10 || num<0) return 0;
	memcpy(output,two_rsc,strlen(two_rsc));
	memcpy(&output[onestart],&names[num][0],strlen(names[num]));
	if (uppercase)
	{
		for (i=0;i<strlen(names[num]);i++) output[onestart+i]&=0x5f;
	}
	memcpy(&output[onestart+strlen(names[num])],&two_rsc[onestart+3],strlen(two_rsc)-onestart+4);

	return 1;
}
// the purpose of this function is to determine the file sizes of each individual .rsc file.
int loader_mw_collectSizes(char* two_rsc,int* sizes,char* gfxbuf)
{
	int i;
	FILE *f;
	char filename[1024];
	int sum;
	sum=0;
	for (i=0;i<10;i++)
	{
		loader_mw_substituteOne(two_rsc,i,filename);
		f=fopen(filename,"rb");
		sizes[i]=0;
		if (f)
		{
			while (!feof(f))
			{
				sizes[i]+=fread(gfxbuf,sizeof(char),1024,f);
				sum+=sizes[i];
			}
			fclose(f);
		}
	}
	return sum;
}
// the purpose of this function is to read n bytes from the resource files.
int loader_mw_readresource(char* two_rsc,int* sizes,int offset,char* buf,int n)
{
	int i;
	int sum;
	int firstresource;
	int resoffs;
	int bidx;
	char filename[1024];
	sum=0;
	resoffs=0;
	firstresource=-1;
	for (i=0;i<10;i++)
	{
		if (sum<=offset)
		{
			resoffs=offset-sum;
			firstresource=i;
		}
		sum+=sizes[i];
	}
	if (firstresource==-1) return 0;
	bidx=0;
	// in case the resource is spread out over multiple .rsc files, loop
	while (bidx<n && firstresource<10)
	{
		FILE *f;
		loader_mw_substituteOne(two_rsc,firstresource,filename);

		f=fopen(filename,"rb");
		if (f)
		{
			fseek(f,resoffs,SEEK_SET);
			bidx+=fread(&buf[bidx],sizeof(char),n-bidx,f);
			fclose(f);
		}
		firstresource++;	// next iteration: the next file
		resoffs=0;	// in the next file, the continuation of the resource starts at the beginning.
	}
	return 1;
}
int loader_mw_mkgfx(char* two_rsc,int* sizes,char* gfxbuf,int* bytes)
{
	int diroffset;
	int direntries;
	int imagecnt;
	int outidx;
	int diridx;
	int i;
	int j;
	char tmpbuf[18];
#define	NAMELENGTH	6	// length of the "file" names in the resource file
#define	DIRENTRYSIZE	(NAMELENGTH+4+4)	// name=6 bytes. offset=4 bytes. length=4 bytes
#define	HEADERSIZE	(4+2)	// "MaP4"+2 bytes for the number of entries
#define	MARGIN		4	// a little bit of extra space
#define	MAXIMAGES	256	// actually 226, but who's counting..
	typedef struct _tImageEntry
	{
		char name[NAMELENGTH+1];
		int offset6;
		int offset7;
		int length6;
		int length7;
	} tImageEntry;
	int entrynum;
	tImageEntry	imageEntries[MAXIMAGES];
	//tImageEntry*  imageEntries=(tImageEntry*)&gfxbuf[16];	// if the memory needs to be preserved
	
	memset(imageEntries,0,sizeof(imageEntries));
	entrynum=0;
	// memset(imageEntries,0,MAXIMAGES*sizeof(tImageEntry));

	// step one: find the directory. it is stored in the very first 4 bytes.
	loader_mw_readresource(two_rsc,sizes,0,gfxbuf,4);
	diroffset=READ_INT32LE(gfxbuf,0);
	loader_mw_readresource(two_rsc,sizes,diroffset,gfxbuf,2);diroffset+=2;
	direntries=READ_INT16LE(gfxbuf,0);
	imagecnt=0;
	// step two: count the image files
	gfxbuf[0]='M';gfxbuf[1]='a';gfxbuf[2]='P';gfxbuf[3]='4';

	// go through all the directory entries.
	// collect the images.
	// 
	// images are spread out over two "files": type 7, which is the huffman tree. and type 6, which is the bitstream. both of them have the same name.
	for (i=0;i<direntries;i++)
	{
		//int unknown;
		char name[NAMELENGTH+1];
		int type;
		int offset;
		int length;
		
		loader_mw_readresource(two_rsc,sizes,diroffset,tmpbuf,18);diroffset+=18;
		//unknown=READ_INT16LE(tmpbuf,0);	// intentionally left in
		offset=READ_INT32LE(tmpbuf,2);
		length=READ_INT32LE(tmpbuf,6);
		for (j=0;j<NAMELENGTH;j++) name[j]=tmpbuf[10+j];
		name[NAMELENGTH]=0;
		type=READ_INT16LE(tmpbuf,16);
		if (type==6 || type==7)	// either the tree or an image
		{
			int found;
			// see if there is an entry with the same name, but the other half of the data
			found=-1;
			for (j=0;j<entrynum && found==-1;j++)
			{
				if (strncmp(name,imageEntries[j].name,NAMELENGTH+1)==0) found=j;	
			}
			if (found==-1) found=entrynum++;	// the name has not been found before. create a new entry
			if (entrynum==(MAXIMAGES+1))		// too many image files found
			{
				fprintf(stderr,"Too many image files found.\n");
				return 0;
			}
			memcpy(imageEntries[found].name,name,NAMELENGTH+1);	// the name is the same. so it does not matter if it is being overwritten
			if (type==6)	// store the information regarding the image
			{
				imageEntries[found].offset6=offset;	
				imageEntries[found].length6=length;	
				imagecnt++;
			}
			if (type==7)	// store the tree
			{
				imageEntries[found].offset7=offset;	
				imageEntries[found].length7=length;	// always 609 bytes
			}
		}
		//printf("    %08X  %5d  %s  %d\n",offset,length,name,type);    
	}
	diridx=4;
	WRITE_INT16LE(gfxbuf,diridx,imagecnt);
	diridx+=2;

	// copy the information from the resource file into the gfxbuf container
	outidx=(imagecnt+1)*(DIRENTRYSIZE)+HEADERSIZE+MARGIN;
	for (i=0;i<entrynum;i++)
	{
		//		int unknown;
		if (imageEntries[i].length7!=609 || imageEntries[i].length6==0) 
		{
			fprintf(stderr,"illegal resource file found.\n");
			return 0;
		}
		for (j=0;j<NAMELENGTH;j++) gfxbuf[diridx++]=imageEntries[i].name[j];
		WRITE_INT32LE(gfxbuf,diridx,outidx);diridx+=4;  // offset within the new mag file
		WRITE_INT32LE(gfxbuf,diridx,imageEntries[i].length7+imageEntries[i].length6);diridx+=4; // length within the new mag file
		// the size of the tree is fixed. Thus,it can be stored together with the bit stream 
		loader_mw_readresource(two_rsc,sizes,imageEntries[i].offset7,&gfxbuf[outidx],imageEntries[i].length7);outidx+=imageEntries[i].length7;      // first the tree (ALWAYS 609 bytes)
		loader_mw_readresource(two_rsc,sizes,imageEntries[i].offset6,&gfxbuf[outidx],imageEntries[i].length6);outidx+=imageEntries[i].length6;      // then the palette/size/width/height and bitstream
	}
	// and the finishing touch
	WRITE_INT32LE(gfxbuf,diridx,0x23232323);diridx+=MARGIN;
	WRITE_INT32LE(gfxbuf,outidx,0x42424242);outidx+=4;

	*bytes=outidx;
	return 1;
}
int loader_mw_mkmag(char* two_rsc,int* sizes,char* magbuf,int* bytes)
{
	int diroffset;
	int direntries;
	int codesize=0;
	int text1size=0;
	int text2size=0;
	int dictsize=0;
	int undosize=0;
	int undopc=0;
	int wtabsize=0;

	int codeoffs=0;
	int textoffs=0;
	int dictoffs=0;
	int wtaboffs=0;
	int total;
	int magidx;
	int wonderland=0;

	int i;

	undosize=undopc=0;

	// step one: find the directory. it is stored in the very first 4 bytes.
	loader_mw_readresource(two_rsc,sizes,0,magbuf,4);
	diroffset=READ_INT32LE(magbuf,0);
	loader_mw_readresource(two_rsc,sizes,diroffset,magbuf,2);diroffset+=2;
	direntries=READ_INT16LE(magbuf,0);
	// step two: count the image files
	codeoffs=textoffs=dictoffs=wtaboffs=-1;
	for (i=0;i<direntries;i++)
	{
//		int unknown;
		int offset;
		int length;
		char name[7];
		int type;
		int j;
		char tmpbuf[18];
		loader_mw_readresource(two_rsc,sizes,diroffset,tmpbuf,18);diroffset+=18;
//		unknown=READ_INT16LE(tmpbuf,0);
		offset=READ_INT32LE(tmpbuf,2);
		length=READ_INT32LE(tmpbuf,6);
		for (j=0;j<6;j++) name[j]=tmpbuf[10+j];
		name[6]=0;
		type=READ_INT16LE(tmpbuf,16);

		if (type==4)
		{
			if (strncmp(name,"code",4)==0) wonderland=1;
			if (strncmp(name,"code",4)==0  || strncmp(name,"ccode",5)==0  ||strncmp(name,"fcode",5)==0  ||strncmp(name,"gcode",4)==0)  {codeoffs=offset;codesize=length;}
			if (strncmp(name,"text",4)==0  || strncmp(name,"ctext",5)==0  ||strncmp(name,"ftext",5)==0  ||strncmp(name,"gtext",5)==0)  {textoffs=offset;text1size=length;}
			if (strncmp(name,"index",5)==0 || strncmp(name,"cindex",6)==0 ||strncmp(name,"findex",6)==0 ||strncmp(name,"gindex",6)==0) {dictoffs=offset;dictsize=length;}
			if (strncmp(name,"wtab",4)==0  || strncmp(name,"cwtab",5)==0  ||strncmp(name,"fwtab",5)==0  ||strncmp(name,"gwtab",5)==0)  {wtaboffs=offset;wtabsize=length;}
		}
	}
	if (codeoffs==-1) return 0;
	if (textoffs==-1) return 0;
	if (dictoffs==-1) return 0;
	if (wtaboffs==-1) return 0;
	magidx=0;
	magbuf[0]='M';magbuf[1]='a';magbuf[2]='S';magbuf[3]='c';magidx+=4;

	total=42+codesize+text1size+dictsize+wtabsize;
	WRITE_INT32BE(magbuf,4,total);
	WRITE_INT32BE(magbuf,8,42);
	magbuf[13]=4;   // version
	WRITE_INT32BE(magbuf,14,codesize);
	WRITE_INT32BE(magbuf,18,(text1size>=0x10000)?0x10000:0xe000);
	if (text1size>0x10000) text2size=text1size+dictsize-0x10000; else text2size=text1size+dictsize-0xe000;
	WRITE_INT32BE(magbuf,22,text2size);
	WRITE_INT32BE(magbuf,26,wtabsize);
	WRITE_INT32BE(magbuf,30,text1size);
	WRITE_INT32BE(magbuf,34,undosize);     // undosize
	WRITE_INT32BE(magbuf,38,undopc);     // undopc

	magidx=42;
	loader_mw_readresource(two_rsc,sizes,codeoffs,&magbuf[magidx],codesize);    
	magidx+=codesize;
	loader_mw_readresource(two_rsc,sizes,textoffs,&magbuf[magidx],text1size);   magidx+=text1size;
	loader_mw_readresource(two_rsc,sizes,dictoffs,&magbuf[magidx],dictsize);    magidx+=dictsize;
	loader_mw_readresource(two_rsc,sizes,wtaboffs,&magbuf[magidx],wtabsize);    magidx+=wtabsize;

	*bytes=magidx;
	// finishing patch
	if (wonderland)
	{
		if (READ_INT16BE(magbuf,0x67a2)==0xa62c)
		{
			magbuf[0x67a2]=0x4e;
			magbuf[0x67a3]=0x75;
		}
	}

	return 1;
}


int loader_magneticwindows(char* two_rsc,
		char *magbuf,int* magsize,
		char* gfxbuf,int* gfxsize)
{
	int bytes;
	int sizes[10]={0};
	*magsize=0;
	*gfxsize=0;
	printf("Pondering...\n");
	if (!loader_mw_collectSizes(two_rsc,sizes,gfxbuf))
	{
		fprintf(stderr,"unable to find resource files\n");
		fprintf(stderr," please make sure that they are named TWO.RSC\n");
		fprintf(stderr," CTWO.RSC or something similar.\n");
		return 0;
	}
	if (loader_mw_mkmag(two_rsc,sizes,magbuf,&bytes))
	{
		*magsize=bytes;
	}
	loader_mw_mkgfx(two_rsc,sizes,gfxbuf,&bytes);
	*gfxsize=bytes;
	return 1;
}

int loader_init(int argc,char** argv,FILE *f_inifile,
		char *magbuf,int* magsize,
		char* gfxbuf,int* gfxsize)
{
	FILE *f;
	char magfilename[1024];
	char gfxfilename[1024];
	char msdosdirname[1024];
	char tworscname[1024];
	int gamenamegiven;
	int n;

	magfilename[0]=gfxfilename[0]=msdosdirname[0]=tworscname[0]=0;
	gamenamegiven=0;
	if ((retrievefromcommandline(argc,argv,"pawn",NULL,0))
			|| (retrievefromcommandline(argc,argv,"guild",NULL,0))
			|| (retrievefromcommandline(argc,argv,"jinxter",NULL,0))
			|| (retrievefromcommandline(argc,argv,"corruption",NULL,0))
			|| (retrievefromcommandline(argc,argv,"fish",NULL,0))
			|| (retrievefromcommandline(argc,argv,"myth",NULL,0))
			|| (retrievefromcommandline(argc,argv,"wonderland",NULL,0)))
	{
		gamenamegiven=1;
	}
	if (!f_inifile && gamenamegiven) 
	{
		fprintf(stderr,"Game name was given, but no suitable .ini file found\n");
		fprintf(stderr,"please run %s -helpini for more help\n",argv[0]);
		return 1;
	}

	if (retrievefromcommandline(argc,argv,"pawn",NULL,0))
	{
		magfilename[0]=gfxfilename[0]=0;
		retrievefromini(f_inifile,"[FILES]","pawnmag",magfilename,sizeof(magfilename));
		retrievefromini(f_inifile,"[FILES]","pawngfx",gfxfilename,sizeof(gfxfilename));
		retrievefromini(f_inifile,"[FILES]","pawnmsdos",msdosdirname,sizeof(msdosdirname));
	}

	if (retrievefromcommandline(argc,argv,"guild",NULL,0))
	{
		magfilename[0]=gfxfilename[0]=0;
		retrievefromini(f_inifile,"[FILES]","guildmag",magfilename,sizeof(magfilename));
		retrievefromini(f_inifile,"[FILES]","guildgfx",gfxfilename,sizeof(gfxfilename));
		retrievefromini(f_inifile,"[FILES]","guildmsdos",msdosdirname,sizeof(msdosdirname));
		retrievefromini(f_inifile,"[FILES]","guildtworsc",tworscname,sizeof(tworscname));
	}

	if (retrievefromcommandline(argc,argv,"jinxter",NULL,0))
	{
		magfilename[0]=gfxfilename[0]=0;
		retrievefromini(f_inifile,"[FILES]","jinxtermag",magfilename,sizeof(magfilename));
		retrievefromini(f_inifile,"[FILES]","jinxtergfx",gfxfilename,sizeof(gfxfilename));
		retrievefromini(f_inifile,"[FILES]","jinxtermsdos",msdosdirname,sizeof(msdosdirname));
	}

	if (retrievefromcommandline(argc,argv,"corruption",NULL,0))
	{
		magfilename[0]=gfxfilename[0]=0;
		retrievefromini(f_inifile,"[FILES]","corruptionmag",magfilename,sizeof(magfilename));
		retrievefromini(f_inifile,"[FILES]","corruptiongfx",gfxfilename,sizeof(gfxfilename));
		retrievefromini(f_inifile,"[FILES]","corruptionmsdos",msdosdirname,sizeof(msdosdirname));
		retrievefromini(f_inifile,"[FILES]","corruptiontworsc",tworscname,sizeof(tworscname));
	}

	if (retrievefromcommandline(argc,argv,"fish",NULL,0))
	{
		magfilename[0]=gfxfilename[0]=0;
		retrievefromini(f_inifile,"[FILES]","fishmag",magfilename,sizeof(magfilename));
		retrievefromini(f_inifile,"[FILES]","fishgfx",gfxfilename,sizeof(gfxfilename));
		retrievefromini(f_inifile,"[FILES]","fishmsdos",msdosdirname,sizeof(msdosdirname));
		retrievefromini(f_inifile,"[FILES]","fishtworsc",tworscname,sizeof(tworscname));
	}

	if (retrievefromcommandline(argc,argv,"myth",NULL,0))
	{
		magfilename[0]=gfxfilename[0]=0;
		retrievefromini(f_inifile,"[FILES]","mythmag",magfilename,sizeof(magfilename));
		retrievefromini(f_inifile,"[FILES]","mythgfx",gfxfilename,sizeof(gfxfilename));
		retrievefromini(f_inifile,"[FILES]","mythmsdos",msdosdirname,sizeof(msdosdirname));
	}


	if (retrievefromcommandline(argc,argv,"wonderland",NULL,0))
	{
		magfilename[0]=gfxfilename[0]=0;
		retrievefromini(f_inifile,"[FILES]","wonderlandmag",magfilename,sizeof(magfilename));
		retrievefromini(f_inifile,"[FILES]","wonderlandgfx",gfxfilename,sizeof(gfxfilename));
		retrievefromini(f_inifile,"[FILES]","wonderlandmsdos",msdosdirname,sizeof(msdosdirname));
		retrievefromini(f_inifile,"[FILES]","wonderlandtworsc",tworscname,sizeof(tworscname));
	}
	// command line parameters should overwrite the .ini parameters.
	if (retrievefromcommandline(argc,argv,"-mag",magfilename,sizeof(magfilename)))
	{
		gfxfilename[0]=0;
		msdosdirname[0]=0;
		tworscname[0]=0;
	}
	if (retrievefromcommandline(argc,argv,"-gfx",gfxfilename,sizeof(gfxfilename)))
	{
		msdosdirname[0]=0;
		tworscname[0]=0;
	}
	if (retrievefromcommandline(argc,argv,"-msdosdir",msdosdirname,sizeof(msdosdirname)))
	{
		gfxfilename[0]=magfilename[0]=0;
		tworscname[0]=0;
	}
	if (retrievefromcommandline(argc,argv,"-tworsc",tworscname,sizeof(tworscname)))
	{
		gfxfilename[0]=magfilename[0]=0;
		msdosdirname[0]=0;
	}
	if (tworscname[0])
	{
		loader_magneticwindows(tworscname,magbuf,magsize,gfxbuf,gfxsize);
	}
	else if (msdosdirname[0])
	{
		loader_msdos(msdosdirname,magbuf,magsize,gfxbuf,gfxsize);	
	} else {

		if (magfilename[0] && !gfxfilename[0])
		{
			// deducing the name of the gfx file from the mag file
			int l;
			int found;
			found=0;
			l=strlen(magfilename);
			fprintf(stderr,"Warning! -mag given, but not -gfx. Deducing filename\n");
			if (l>=4)
			{
				if (strncmp(&magfilename[l-4],".mag",4)==0)
				{
					memcpy(gfxfilename,magfilename,l+1);
					found=1;
					gfxfilename[l-4]='.';
					gfxfilename[l-3]='g';
					gfxfilename[l-2]='f';
					gfxfilename[l-1]='x';
				}
			}
			if (!found)
			{
				fprintf(stderr,"filename did not end in .mag (lower case)\n");
				return 0;
			}
		}
		if (!magfilename[0] && gfxfilename[0])
		{
			// deducing the name of the mag from the gfx
			int l;
			int found;
			found=0;
			l=strlen(gfxfilename);
			fprintf(stderr,"warning! -gfx given, but not -mag. Deducing filename\n");
			if (l>=4)
			{
				if (strncmp(&gfxfilename[l-4],".gfx",4)==0)
				{
					memcpy(magfilename,gfxfilename,l+1);
					found=1;
					magfilename[l-4]='.';
					magfilename[l-3]='m';
					magfilename[l-2]='a';
					magfilename[l-1]='g';
				}
			}
			if (!found)
			{
				fprintf(stderr,"filename did not end in .gfx (lower case)\n");
				return 0;
			}
		}
		if ((!magfilename[0] || !gfxfilename[0]))
		{
			fprintf(stderr,"Please provide a game via commandline. Please use either\n");
			fprintf(stderr," %s -mag MAGFILE.mag or %s -ini dMagnetic.ini pawn\n",argv[0],argv[0]);
			fprintf(stderr,"\n");
			fprintf(stderr,"you need to provide a working dMagnetic.ini file\n");
			fprintf(stderr,"please run %s -helpini for more help\n",argv[0]);
			return 1;
		}
		f=fopen(magfilename,"rb");
		if (f==NULL)
		{
			fprintf(stderr,"ERROR: unable to open [%s]\n",magfilename);
			fprintf(stderr,"This interpreter needs a the game's binaries in the .mag and .gfx\n");
			fprintf(stderr,"format from the Magnetic Scrolls Memorial webseiite. For details, \n");
			fprintf(stderr,"see https://msmemorial.if-legends.org/memorial.php\n");
			return -2;
		}
		n=fread(magbuf,sizeof(char),*magsize,f);
		fclose(f);
		*magsize=n;
		f=fopen(gfxfilename,"rb");
		if (f==NULL)
		{
			fprintf(stderr,"ERROR: unable to open [%s]\n",gfxfilename);
			fprintf(stderr,"This interpreter needs a the game's binaries in the .mag and .gfx\n");
			fprintf(stderr,"format from the Magnetic Scrolls Memorial webseiite. For details, \n");
			fprintf(stderr,"see https://msmemorial.if-legends.org/memorial.php\n");
			return -2;
		}
		n=fread(gfxbuf,sizeof(char),*gfxsize,f);
		*gfxsize=n;
		fclose(f);
		// at this point, they are stored in magbuf and gfxbuf.
	}		

	return 0;


}
