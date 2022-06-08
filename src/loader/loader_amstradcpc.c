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

// the purpose of this file is to read the Amstrad .DSK image files
// and to translate them into the .mag/.gfx format.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm68k_macros.h"
#include "loader_amstradcpc.h"

#include <string.h>
#include <stdlib.h>

#define	DSK_IMAGESIZE	194816

#define	MAXFILENAMELEN	(8+3)	// filenames in CPM are 8 bytes long. with a 3 byte extension
#define	EXTENDLEN	4	// this part is 4 bytes long
#define	MAXBLOCKS	16	// 16 pointers per directory entry

#define	MAXBLOCKSIZE	1024	// one pointer is pointing towards 1 kByte
#define	MINSECTORSIZE	128	// the smallest number of bytes in a sector
#define	MAXOFFSETSPERENTRY	((MAXBLOCKSIZE/MINSECTORSIZE)*MAXBLOCKS)

#define	SIZE_FILEHEADER	256
#define	SIZE_TRACKHEADER	256
#define	SIZE_SECTORHEADER	8
#define	SIZE_DIRENTRY		32
#define	MAX_DIRENTRIES		64	// TODO???
#define	MAX_SECTORNUMPERTRACK	64
#define	MAX_SECTORNUMPERDISK	(DSK_IMAGESIZE/MINSECTORSIZE)
#define	MAX_DISKS		2
#define	NUM_GAMES		6

#define	FILESUFFIX1		1
#define	FILESUFFIX2		2
#define	FILESUFFIX3		3
#define	FILESUFFIX4		4
#define	FILESUFFIX5		5
#define	FILESUFFIX6		6
#define	FILESUFFIX7		7
#define	FILESUFFIX8		8

typedef struct _tGames
{
	char gamename[32];
	char gamefilename[MAXFILENAMELEN+1];
	int version;	
	unsigned short expectedsuffixes;
} tGames;
const tGames loader_amstradcpc_knownGames[NUM_GAMES]={
	// name,game file names, version number
	{"The Pawn",			"PAWN\0       ",0,(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)},
	{"The Guild of Thieves",	"GUILD\0      ",1,(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)|(1<<FILESUFFIX6)|(1<<FILESUFFIX7)},
	{"Jinxter",			"JINX\0       ",2,(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)|(1<<FILESUFFIX6)|(1<<FILESUFFIX7)|(1<<FILESUFFIX8)},
	{"Corruption",			"CORR\0       ",3,(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)|(1<<FILESUFFIX6)|(1<<FILESUFFIX7)|(1<<FILESUFFIX8)},
	{"Fish!",			"????\0	      ",3,(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)|(1<<FILESUFFIX6)|(1<<FILESUFFIX7)|(1<<FILESUFFIX8)},
	{"Myth",			"????\0       ",3,(1<<FILESUFFIX1)|(1<<FILESUFFIX2)|(1<<FILESUFFIX3)|(1<<FILESUFFIX4)|(1<<FILESUFFIX5)|(1<<FILESUFFIX6)|(1<<FILESUFFIX7)|(1<<FILESUFFIX8)}
};


typedef struct _tDirEntry
{
	unsigned char userID;
	char name[MAXFILENAMELEN+1];
	unsigned char attrs;
	unsigned char extend[EXTENDLEN];
	unsigned char blocks[MAXBLOCKS];// block identifier

	int fileID;		// the "filename" without the prefix.
	int offsets[MAXOFFSETSPERENTRY];	// translated offsets
} tDirEntry;
void loader_amstradcpc_descrambler(unsigned char* outputbuf,int len,unsigned short startvalue)
{
	unsigned short value;
	unsigned char key;
	int i;

	value=startvalue;
	for (i=0;i<len;i++)
	{
		value=(value+((value<<8)+0x29))&0xffff;
		key=(value^(value>>8))&0xff;
		outputbuf[i]^=key;
	}
}
int loader_amstradcpc_unhuffer(unsigned char* inputbuf,int len,unsigned char* outputbuf)
{
	int treesize;
	int treeidx;
	int bitidx;
	int maxtree;
	unsigned char threebuf[3];
	int threecnt;
	int outidx;

	unsigned char byte;
	unsigned char mask;

	treesize=inputbuf[0];
	treeidx=0;
	outidx=0;
	bitidx=treesize+3;
	mask=0;
	threecnt=0;
	maxtree=0;
	while (bitidx<len|| mask)
	{
		unsigned char branchl,branchr;
		unsigned char branch;

		if (mask==0)
		{
			mask=0x80;
			byte=inputbuf[bitidx++];
		}
		branchl=inputbuf[1+2*treeidx];
		branchr=inputbuf[2+2*treeidx];
		branch=(mask&byte)?branchl:branchr;
		mask>>=1;

		if (branch&0x80)
		{
			if (maxtree<treeidx) maxtree=treeidx;
			treeidx=0;
			branch&=0x7f;

			if (threecnt==3)
			{
				threebuf[0]|=((branch>>4)&0x3)<<6;
				threebuf[1]|=((branch>>2)&0x3)<<6;
				threebuf[2]|=((branch>>0)&0x3)<<6;

				outputbuf[outidx++]=threebuf[0];
				outputbuf[outidx++]=threebuf[1];
				outputbuf[outidx++]=threebuf[2];
				threecnt=0;
			} else {
				threebuf[threecnt++]=branch;
			}
		} else {
			treeidx=branch;
		}
	}
	return outidx;
}
int loader_amstradcpc_readfile(unsigned char* inputbuf,unsigned char* outputbuf,int fileID,tDirEntry* pDirEntries,int entrycnt,int sectorsize)
{
	int i;
	int outputidx;
	outputidx=0;
	for (i=0;i<entrycnt;i++)
	{
		int j;
		if (pDirEntries[i].fileID==fileID)
		{
			for (j=0;j<MAXOFFSETSPERENTRY;j++)
			{
				if (pDirEntries[i].offsets[j]!=-1)
				{
					memcpy(&outputbuf[outputidx],&inputbuf[pDirEntries[i].offsets[j]],sectorsize);
					outputidx+=sectorsize;
				}
			}
		}	
	}
	return outputidx;

}
int loader_amstradcpc_mag(unsigned char* magbuf,int* magsize,
		unsigned char* diskimage,int diskcnt,
		unsigned char* tmpbuf,int tmpbufsize,
		int gamedetected,
		tDirEntry* pDirEntries,int entrycnt,int sectorsize)
{
	int outputidx;
	int version;
	int code1size;
	int code2size;
	int string1size;
	int string2size;
	int dictsize;
	int huffmantreeidx;

	outputidx=42;
	version=loader_amstradcpc_knownGames[gamedetected].version;
	if (version==0)
	{
		int tmpsize;
		// in THE PAWN, the code section is packed.
		tmpsize=loader_amstradcpc_readfile(diskimage,tmpbuf,FILESUFFIX1,pDirEntries,entrycnt,sectorsize);
		code1size=loader_amstradcpc_unhuffer(tmpbuf,tmpsize,&magbuf[outputidx]);
		outputidx+=code1size;
		code2size=0;
	} else {
		int i;
		// in all the other games, it is spread out over two files: FILE1 and FILE6.
		code1size=loader_amstradcpc_readfile(diskimage,&magbuf[outputidx],FILESUFFIX1,pDirEntries,entrycnt,sectorsize);
		loader_amstradcpc_descrambler(&magbuf[outputidx],code1size,0x1803);	// the first part is scrambled different than the second one.
		outputidx+=code1size;
		code2size=loader_amstradcpc_readfile(diskimage,&magbuf[outputidx],FILESUFFIX6,pDirEntries,entrycnt,sectorsize);
		for (i=0;i<code2size;i+=0x80)
		{
			loader_amstradcpc_descrambler(&magbuf[outputidx],0x80,code1size+i);	// each 128 block has its own PRBS.
			outputidx+=0x80;
		}
	}
	string1size=loader_amstradcpc_readfile(diskimage,&magbuf[outputidx],FILESUFFIX3,pDirEntries,entrycnt,sectorsize);
	outputidx+=string1size;
	if (version==0)
	{
		int tmpsize;
		// in THE PAWN, the string2 section is packed.
		tmpsize=loader_amstradcpc_readfile(diskimage,tmpbuf,FILESUFFIX2,pDirEntries,entrycnt,sectorsize);
		string2size=loader_amstradcpc_unhuffer(tmpbuf,tmpsize,&magbuf[outputidx]);
		outputidx+=string2size;
	} else {
		string2size=loader_amstradcpc_readfile(diskimage,&magbuf[outputidx],FILESUFFIX2,pDirEntries,entrycnt,sectorsize);
		outputidx+=string2size;
	}
	// some games have a file with the suffix 8, and this is for the dict.
	dictsize=loader_amstradcpc_readfile(diskimage,&magbuf[outputidx],FILESUFFIX8,pDirEntries,entrycnt,sectorsize);
	loader_amstradcpc_descrambler(&magbuf[outputidx],dictsize,0x1803);	// the dictionary is scrambled the same way the code is
	outputidx+=dictsize;
	huffmantreeidx=string1size;
	if (string1size>0x10000)
	{
		int x;
		x=string1size+string2size;
		string1size=0x10000;
		string2size=x-string1size;
	}

	magbuf[0]='M';magbuf[1]='a';magbuf[2]='S';magbuf[3]='c';        //  0.. 3: the magic word
	WRITE_INT32BE(magbuf, 4 ,outputidx);                               //  4.. 7: the total size
	WRITE_INT32BE(magbuf, 8 ,42);                           //  8..11: the size of the header
	WRITE_INT16BE(magbuf,12 ,version);  // 12..13: the version for the virtual machine
	WRITE_INT32BE(magbuf,14 ,code1size+code2size);          // 14..17 the size of the game code
	WRITE_INT32BE(magbuf,18 ,string1size);                  // 18..21 the size of the string1
	WRITE_INT32BE(magbuf,22 ,string2size);                  // 22..25 the size of the string2
	WRITE_INT32BE(magbuf,26 ,dictsize);                     // 26..29 the size of the dictionary
	WRITE_INT32BE(magbuf,30 ,huffmantreeidx)                // 30..33 the beginning of the huffman tree within the string buffer
		WRITE_INT32BE(magbuf,34 ,0);                            //  34..37: undo size
	WRITE_INT32BE(magbuf,38 ,0);                            //  38..41: undo pc

	*magsize=outputidx;

	return 0;
}
int loader_amstradcpc_gfx(
		unsigned char* gfxbuf,int* gfxsize,
		unsigned char* diskimage,int diskcnt,
		int gamedetected,
		tDirEntry* pDirEntries,int entrycnt,int sectorsize)
{
	int outputidx;
	int outputidx0;
	int outputidx1;
	int i;
	int version;

	version=loader_amstradcpc_knownGames[gamedetected].version;

	outputidx=0;
	gfxbuf[outputidx++]='M';	
	gfxbuf[outputidx++]='a';	
	gfxbuf[outputidx++]='P';	
	gfxbuf[outputidx++]='6';
	outputidx0=outputidx;
	if (version==0)
	{
		// THE PAWN uses a single file for the images and the index.
		gfxbuf[outputidx++]=0;
		gfxbuf[outputidx++]=0;
		outputidx+=loader_amstradcpc_readfile(diskimage,&gfxbuf[outputidx],FILESUFFIX4,pDirEntries,entrycnt,sectorsize);
		// it is necessary to convert the image a little bit. the header has to be added, for example.
		// and, just for fun, change the index to BIG endian as well
		for (i=0;i<29;i++)	// no more than 29 images
		{
			unsigned int x;
			x=READ_INT32LE(gfxbuf,outputidx0+2+i*4);
			x+=6;
			x&=0xffffff;
			WRITE_INT32BE(gfxbuf,outputidx0+i*4,x);
		}


	} else {
		int idxoffs;
		outputidx+=loader_amstradcpc_readfile(diskimage,&gfxbuf[outputidx],FILESUFFIX4,pDirEntries,entrycnt,sectorsize);
		outputidx=4+4*32;	// due to limitations of the readfile() function, the data being read is too much	
		outputidx0=outputidx;
		outputidx+=loader_amstradcpc_readfile(diskimage,&gfxbuf[outputidx],FILESUFFIX5,pDirEntries,entrycnt,sectorsize);
		outputidx1=outputidx;
		outputidx+=loader_amstradcpc_readfile(diskimage,&gfxbuf[outputidx],FILESUFFIX7,pDirEntries,entrycnt,sectorsize);
		idxoffs=4;
		for (i=0;i<32;i++)
		{
			unsigned int x;
			x=READ_INT32LE(gfxbuf,idxoffs);
			if (x&0xff000000)	// MSB set, so the picture is in FILE5
			{
				x&=0xffffff;
				x+=outputidx0;
			} else {
				x+=outputidx1;	// MSB not set. so the picture is in FILE7
			}
			if (x<outputidx)
			{
				WRITE_INT32BE(gfxbuf,idxoffs,x);
			} else {
				WRITE_INT32BE(gfxbuf,idxoffs,0);
			}
			idxoffs+=4;
		}

	}
	*gfxsize=outputidx;

	return 0;
}
int loader_amstradcpc(char* amstradcpcname,
		char *magbuf,int* magsize,
		char* gfxbuf,int* gfxsize)
{
	char* filename[MAX_DISKS];
	unsigned char *dskimage;
	int gfxsize0;
	int diskcnt;
	int offsets[MAX_DISKS][MAX_SECTORNUMPERDISK];
	int retval;
	int i,l;
	int entrycnt;
	int gamedetected;
	int sectorsize;
	unsigned short foundsuffixes;
	tDirEntry dirEntries[MAX_DIRENTRIES];
	gfxsize0=*gfxsize;	
	foundsuffixes=0;
	sectorsize=0;
	if (gfxsize0<4*DSK_IMAGESIZE)
	{
		fprintf(stderr,"not enough memory to load DSK images. sorry.\n");
		return -1;
	}
	dskimage=(unsigned char*)&gfxbuf[2*DSK_IMAGESIZE];
	filename[0]=&amstradcpcname[0];
	filename[1]=&amstradcpcname[0];
	diskcnt=1;
	l=strlen(amstradcpcname);
	for (i=0;i<l;i++)
	{
		if (amstradcpcname[i]==',') 
		{
			amstradcpcname[i]=0;
			filename[1]=&amstradcpcname[i+1];
			diskcnt++;
			if (diskcnt>2) 
			{
				fprintf(stderr,"Please provide no more than 2 filenames, separated by ,\n");
				return -1;
			}
		}
	}
	// load the game
	for (i=0;i<diskcnt;i++)
	{
		int n;
		FILE *f;
		f=fopen(filename[i],"rb");
		if (!f)
		{
			fprintf(stderr,"unable to open [%s]. Sorry.\n",filename[i]);
			return -1;
		}
		n=fread(&dskimage[i*DSK_IMAGESIZE],sizeof(char),DSK_IMAGESIZE,f);
		fclose(f);
		if (n!=DSK_IMAGESIZE)
		{
			fprintf(stderr,"[%s] does not look like a DSK image\n",filename[i]);
			return -1;
		}
	}


	// start by finding the offsets to the sectors
	for (i=0;i<diskcnt;i++)
	{
		int tracknum;
		int sidenum;
		int tracksize;
		int sectorcnt;
		int idx;
		int j;
		// TODO...
		// 0x30: number of tracks
		// 0x31: number of sides
		// 0x32..33: size of the tracks

		tracknum=dskimage[i*DSK_IMAGESIZE+0x30];
		sidenum=dskimage[i*DSK_IMAGESIZE+0x31];
		tracksize=READ_INT16LE(dskimage,i*DSK_IMAGESIZE+0x32);

		sectorcnt=0;
		idx=i*DSK_IMAGESIZE+SIZE_FILEHEADER;

		for (j=0;j<tracknum*sidenum;j++)
		{
			int sectorids[MAX_SECTORNUMPERTRACK]={0};
			int order[MAX_SECTORNUMPERTRACK]={0};
			int idx0;
			int track0,side0;
			int k;
			int sectornum;

			idx0=idx;
			// TODO: 0..0x0b: Magic Word
			// 0x0c..0x0f: unused
			idx+=0x0c;		// skip the magic word	
			idx+=0x04;		// skip the header
			track0=dskimage[idx++];
			side0=dskimage[idx++];
			idx+=2;			// skip over the unused

			if ((track0*sidenum+side0)!=j) 
			{
				fprintf(stderr,"sanity check 1 for the DSK file failed.\n");
				return -1;

			}
			sectorsize=128<<((dskimage[idx++])&0xf);	// 128, 256 512, 1024 bytes
			sectornum=dskimage[idx++];
			idx+=2;		// skip over the gap3 length and the filler byte
			if (sectornum>=MAX_SECTORNUMPERTRACK) 
			{
				fprintf(stderr,"this file has too many sectors\n");
				return -1;
			}
			// after the track header comes the sector header
			for (k=0;k<sectornum;k++)
			{
				int track1;
				int side1;
				// 0x00: track number
				// 0x01: side number
				// 0x02: sector ID
				// 0x03: sector size
				// 0x04..0x05: fdx status
				// 0x06..0x07: unused

				order[k]=k;
				track1=dskimage[idx++];
				side1=dskimage[idx++];
				if (track1!=track0 || side1!=side0) 
				{
					fprintf(stderr,"sanity check 2 for the DSK file failed.\n");
					return -1;
				}
				sectorids[k]=dskimage[idx++];
				idx+=3;		// skip sector size and fdc status
				idx+=2;		// skip over unused bytes
			}
			// sort them. I am too lazy to implement a faster sorting algorithm
			for (k=0;k<sectornum-1;k++)
			{
				int l;
				for (l=k+1;l<sectornum;l++)
				{
					if (sectorids[order[k]]>sectorids[order[l]])
					{
						// swap them
						order[k]^=order[l];
						order[l]^=order[k];
						order[k]^=order[l];
					}
				}
			}
			for (k=0;k<sectornum;k++)
			{
				offsets[i][sectorcnt++]=idx0+SIZE_TRACKHEADER+order[k]*sectorsize;
			}
			idx=idx0+tracksize;
		}
	}


	////// step 2: read the directory. now we known where it is.
	entrycnt=0;
	gamedetected=-1;
	for (i=0;i<MAX_DISKS;i++)
	{
		int j,k,l;
		unsigned char *ptr;
		for (j=0;j<(MAXBLOCKSIZE/sectorsize)*2;j++)
		{
			for (k=0;k<sectorsize;k+=SIZE_DIRENTRY)
			{
				ptr=&dskimage[offsets[i][j]+k];
				if (ptr[0]==0)
				{
					dirEntries[entrycnt].userID=ptr[0];
					for (l=0;l<MAXFILENAMELEN;l++)
					{
						dirEntries[entrycnt].name[l]=ptr[1+l]&0x7f;
					}
					dirEntries[entrycnt].name[MAXFILENAMELEN]=0;
					dirEntries[entrycnt].fileID=-1;
					if (gamedetected==-1)
					{
						int m;
						for (m=0;m<NUM_GAMES;m++)
						{
							if (strncmp(dirEntries[entrycnt].name,loader_amstradcpc_knownGames[m].gamefilename,strlen(loader_amstradcpc_knownGames[m].gamefilename))==0)
							{
								gamedetected=m;
							}	
						}
					}
					if (gamedetected!=-1)
					{
						int m;
						m=strlen(loader_amstradcpc_knownGames[gamedetected].gamefilename);
						if (strncmp(dirEntries[entrycnt].name,loader_amstradcpc_knownGames[gamedetected].gamefilename,m)==0)
						{
							dirEntries[entrycnt].fileID=dirEntries[entrycnt].name[m]-'0';
							if (dirEntries[entrycnt].fileID>=0 && dirEntries[entrycnt].fileID<=8)
							{
								foundsuffixes|=(1<<dirEntries[entrycnt].fileID);
							}
						}	
						dirEntries[entrycnt].attrs=(ptr[ 9]>>7)&1;
						dirEntries[entrycnt].attrs<<=1;
						dirEntries[entrycnt].attrs|=(ptr[10]>>7)&1;
						dirEntries[entrycnt].attrs<<=1;
						dirEntries[entrycnt].attrs|=(ptr[11]>>7)&1;
						for (l=0;l<EXTENDLEN;l++) dirEntries[entrycnt].extend[l]=ptr[12+l];
						for (l=0;l<MAXOFFSETSPERENTRY;l++) dirEntries[entrycnt].offsets[l]=-1;
						for (l=0;l<MAXBLOCKS;l++) 
						{
							int m;
							int n;

							n=MAXBLOCKSIZE/sectorsize;
							dirEntries[entrycnt].blocks[l]=ptr[16+l];
							if (dirEntries[entrycnt].blocks[l]==0)
							{
								for (m=0;m<n;m++)
								{
									dirEntries[entrycnt].offsets[l*n+m]=-1;
								}
							} else {
								
								for (m=0;m<n;m++)
								{
									dirEntries[entrycnt].offsets[l*n+m]=offsets[i][dirEntries[entrycnt].blocks[l]*n+m];
								}
							}
						}
						entrycnt++;
					}
				}
			}
		}
	}

	if (gamedetected==-1)
	{
		fprintf(stderr,"!! Unable to detect the game. Sorry\n\n");
		return 1;
	}
	printf("Amstrad CPC loader detected '%s'\n",loader_amstradcpc_knownGames[gamedetected].gamename);
	if ((foundsuffixes&0x1fe)!=(0x1fe&loader_amstradcpc_knownGames[gamedetected].expectedsuffixes))
	{
		unsigned short s;
		s=loader_amstradcpc_knownGames[gamedetected].expectedsuffixes&0x1fe;
		fprintf(stderr,"expected ");
		for (i=0;i<9;i++)
		{
			if (s&1) fprintf(stderr,"%s%d ",loader_amstradcpc_knownGames[gamedetected].gamefilename,i);
			s>>=1;
		}
		fprintf(stderr,"\n");
		s=foundsuffixes&0x1fe;
		fprintf(stderr,"found    ");
		for (i=0;i<9;i++)
		{
			if (s&1) fprintf(stderr,"%s%d ",loader_amstradcpc_knownGames[gamedetected].gamefilename,i);
			s>>=1;
		}
		fprintf(stderr,"\n");

		fprintf(stderr,"!! Some files missing. Sorry\n\n");
		return 1;
	}
	// start with the magbuf. use the first half of the empty gfx buf as tmpbuf for the unhuff	
	retval=loader_amstradcpc_mag((unsigned char*)magbuf,magsize,
			dskimage,diskcnt,
			(unsigned char*)gfxbuf,2*DSK_IMAGESIZE,
			gamedetected,
			dirEntries,entrycnt,sectorsize);
	if (retval) return retval;

	retval=loader_amstradcpc_gfx((unsigned char*)gfxbuf,gfxsize,
			dskimage,diskcnt,
			gamedetected,
			dirEntries,entrycnt,sectorsize);

	return retval;


}

