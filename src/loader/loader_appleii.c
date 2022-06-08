/*

   Copyright 2021, dettus@dettus.net

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

// the purpose of this file is to read the D64 disk image files
// and to translate them into the .mag/.gfx format.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "loader_appleii.h"
#include "loader_common.h"
#include "configuration.h"
#include "vm68k_macros.h"

#define	MAXDISKS	3
#define	MAXTRACKS	35
#define	MAXSECTORS	16
#define	SECTORBYTES	256
#define	DSKTRACKSIZE	(MAXSECTORS*SECTORBYTES)
#define	DSKSIZE		(MAXTRACKS*MAXSECTORS*SECTORBYTES)
#define	NIBTRACKSIZE	(MAXSECTORS*416)	// TODO: why 416?


#define	GAME_PAWN	0
#define	GAME_GUILD	1
#define	GAME_JINXTER	2
#define	GAME_CORRUPTION	3


int loader_appleii_decodenibtrack(unsigned char* pTrackBuf,int track,unsigned char* pDskBuf)
{
#define	PREAMBLESIZE	3
#define	DECODEROFFS	0x96
#define	SECTORLSB	86
#define	ROL(x)	((((x)&0x80)>>7|(x)<<1)&0xff)
	const	unsigned char loader_appleii_addr_preamble[PREAMBLESIZE]={0xD5,0xAA,0x96};
	const	unsigned char loader_appleii_data_preamble[PREAMBLESIZE]={0xD5,0xAA,0xAD};
	//const	unsigned char loader_appleii_epilog[PREAMBLESIZE]={0xDE,0xAA,0xEB};

	const	unsigned char loader_appleii_translatetab[106]={
		0x00,0x01,0xFF,0xFF,0x02,0x03,0xFF,0x04,
		0x05,0x06,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0x07,0x08,0xFF,0xFF,0xFF,0x09,0x0A,0x0B,
		0x0C,0x0D,0xFF,0xFF,0x0E,0x0F,0x10,0x11,
		0x12,0x13,0xFF,0x14,0x15,0x16,0x17,0x18,
		0x19,0x1A,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0x1B,0xFF,0x1C,
		0x1D,0x1E,0xFF,0xFF,0xFF,0x1F,0xFF,0xFF,
		0x20,0x21,0xFF,0x22,0x23,0x24,0x25,0x26,
		0x27,0x28,0xFF,0xFF,0xFF,0xFF,0xFF,0x29,
		0x2A,0x2B,0xFF,0x2C,0x2D,0x2E,0x2F,0x30,
		0x31,0x32,0xFF,0xFF,0x33,0x34,0x35,0x36,
		0x37,0x38,0xFF,0x39,0x3A,0x3B,0x3C,0x3D,
		0x3E,0x3F};

	const unsigned char loader_appleii_deinterleave[16]={ 0x0,0x7,0xe,0x6,0xd,0x5,0xc,0x4,0xb,0x3,0xa,0x2,0x9,0x1,0x8,0xf };

	unsigned char addr_track=0;
	unsigned char addr_sector=0;
	unsigned char addr_volume=0;
	unsigned char addr_checksum=0;
	int volumeid;
	int foundsectors;
	int ridx;
	int state;

	volumeid=-1;
	foundsectors=0;
	ridx=0;
	state=0;
	while (ridx<(NIBTRACKSIZE+SECTORBYTES+SECTORLSB+1+9*PREAMBLESIZE))
	{
		switch(state)
		{
			case 0:		// find the ADDR preamble
				{
					if ( pTrackBuf[(ridx+0)%NIBTRACKSIZE]==loader_appleii_addr_preamble[0] && pTrackBuf[(ridx+1)%NIBTRACKSIZE]==loader_appleii_addr_preamble[1] && pTrackBuf[(ridx+2)%NIBTRACKSIZE]==loader_appleii_addr_preamble[2])
					{
						ridx+=3;
						state=1;
						foundsectors++;
					} else ridx++;
				}
				break;
			case 1:		// decode the ADDR data
				{
					unsigned char x;
					x=pTrackBuf[(ridx++)%NIBTRACKSIZE];x=ROL(x);x&=pTrackBuf[(ridx++)%NIBTRACKSIZE];addr_volume=x;
					x=pTrackBuf[(ridx++)%NIBTRACKSIZE];x=ROL(x);x&=pTrackBuf[(ridx++)%NIBTRACKSIZE];addr_track=x;
					x=pTrackBuf[(ridx++)%NIBTRACKSIZE];x=ROL(x);x&=pTrackBuf[(ridx++)%NIBTRACKSIZE];addr_sector=loader_appleii_deinterleave[x&0xf];
					x=pTrackBuf[(ridx++)%NIBTRACKSIZE];x=ROL(x);x&=pTrackBuf[(ridx++)%NIBTRACKSIZE];addr_checksum=x;
					if ((addr_volume^addr_track^addr_sector^addr_checksum)&0xf0)
					{
						printf("Warning. Checksum mismatch\n");
					}
					if (volumeid==-1 || volumeid==addr_volume)
					{
						volumeid=addr_volume;
					} else {
						printf("volumeid mismatch\n");
						return -1;
					}
					if (addr_track!=track)
					{
						printf("track mismatch %d vs %d\n",addr_track,track);
						return -1;
					}
					ridx+=PREAMBLESIZE;	// skip over the epilogue
					state=2;	// start looking for data
				}
				break;
			case 2:		// find the DATA preamble
				{
					if ( pTrackBuf[(ridx+0)%NIBTRACKSIZE]==loader_appleii_data_preamble[0] && pTrackBuf[(ridx+1)%NIBTRACKSIZE]==loader_appleii_data_preamble[1] && pTrackBuf[(ridx+2)%NIBTRACKSIZE]==loader_appleii_data_preamble[2])
					{
						ridx+=3;
						state=3;
					} else ridx++;
				}
				break;
			case 3:		// decode the DATA
				{
					unsigned char lsbbuf[SECTORLSB];
					unsigned char accu;
					int j;
					accu=0;
					for (j=0;j<SECTORLSB;j++)
					{
						accu^=loader_appleii_translatetab[pTrackBuf[ridx%NIBTRACKSIZE]-DECODEROFFS];	
						lsbbuf[j]=accu;
						ridx++;
					}
					for (j=0;j<SECTORBYTES;j++)
					{
						int widx;
						accu^=loader_appleii_translatetab[pTrackBuf[ridx%NIBTRACKSIZE]-DECODEROFFS];	
						widx=j+SECTORBYTES*addr_sector;
						pDskBuf[widx]=accu;
						pDskBuf[widx]<<=1;pDskBuf[widx]|=(1&lsbbuf[j%SECTORLSB]);lsbbuf[j%SECTORLSB]>>=1;
						pDskBuf[widx]<<=1;pDskBuf[widx]|=(1&lsbbuf[j%SECTORLSB]);lsbbuf[j%SECTORLSB]>>=1;
						ridx++;
					}
					ridx+=PREAMBLESIZE;	// skip over the epilogue
					state=0;		// search for the next ADDR preamble
				}
				break;
		}

	}	
	return volumeid;
}

int loader_appleii_mkgfx(unsigned char *gfxbuf,int* gfxsize,int gameid,int diskcnt,int *pDskOffs)
{
#define	ON_DISKA		0x100000
#define	ON_DISKB		0x200000
#define	ON_DISKC		0x400000
#define	PICTURE_HOTFIX1		0x80000000
#define	PICTURE_HOTFIX2		0x40000000
#define	MASK_OFFSET		0xC001FFFF

#define	PICTURENUM		26
	int i;

	const unsigned int loader_appleii_offsets_pictures[PICTURENUM]={
		0x0A000|ON_DISKA,
		0x00000|ON_DISKC|PICTURE_HOTFIX1,
		0x0C300|ON_DISKA|PICTURE_HOTFIX2,
		0x01D00|ON_DISKC,

		0x0E400|ON_DISKA,
		0x03E00|ON_DISKC,
		0x10500|ON_DISKA,
		0x05A00|ON_DISKC|PICTURE_HOTFIX1,

		0x07E00|ON_DISKC,
		0x0A200|ON_DISKC,
		0x12400|ON_DISKA,
		0x0C600|ON_DISKC,

		0x0EA00|ON_DISKC,
		0x10A00|ON_DISKC|PICTURE_HOTFIX2,
		0x14300|ON_DISKA,
		0x12A00|ON_DISKC,

		0x14C00|ON_DISKC,
		0x16500|ON_DISKA,
		0x17000|ON_DISKC,
		0x19600|ON_DISKC,

		0x18900|ON_DISKA,
		0x1BA00|ON_DISKC,
		0x1AC00|ON_DISKA,
		0x1CB00|ON_DISKA,

		0x1DB00|ON_DISKC,
		0x1FF00|ON_DISKC
	};


	if (gameid!=GAME_CORRUPTION)
	{
		*gfxsize=0;
		return 0;
	}
	if (diskcnt!=MAXDISKS)
	{
		fprintf(stderr,"wrong number of floppy disks\n");
		return -1;
	}
	*gfxsize=4+4*32+diskcnt*DSKSIZE;
	printf("gfxsize:%08X\n",*gfxsize);
	gfxbuf[0]='M';gfxbuf[1]='a';gfxbuf[2]='P';gfxbuf[3]='8';
	for (i=0;i<PICTURENUM;i++)
	{
		unsigned int offset;
		int disk;
		unsigned int x;
		offset=loader_appleii_offsets_pictures[i];
		disk=0;
		if (offset&ON_DISKA) disk=0;
		else if (offset&ON_DISKB) disk=1;
		else if (offset&ON_DISKC) disk=2;
		offset&=MASK_OFFSET;
		x=offset+pDskOffs[disk]+132;
		WRITE_INT32BE(gfxbuf,4+i*4,x);
	}
	return 0;
}
typedef	 struct _tSection
{
	int track;
	int sector;
	int disk;
	int len;
	int scrambled;
	int rle;
} tSection;
int loader_appleii_readsection(unsigned char* pOut,tSection section,unsigned char* pDskBuf,int diskcnt,int* pDskOffs,int pivot)
{
	int idx;
	int outidx;
	int firstsector;
	int rle;
	int rlecutoff=DSKSIZE;
	unsigned char tmp[SECTORBYTES];
	unsigned char lc;
	int i;

	rle=section.rle;
	outidx=0;
	firstsector=1;
	idx=(section.track*MAXSECTORS+section.sector)*SECTORBYTES+pDskOffs[section.disk];
	lc=0xff;

	while (outidx<section.len)
	{
		int ridx;
		int removeendmarker;
		memcpy(tmp,&pDskBuf[idx],SECTORBYTES);
		idx+=SECTORBYTES;
		ridx=0;
		removeendmarker=0;
		if (section.scrambled)
		{
			loader_common_descramble(tmp,tmp,pivot,NULL,0);
			pivot=(pivot+1)%8;
			if (firstsector && rle)
			{
				rlecutoff=READ_INT16BE(tmp,0);
				ridx=2;
				firstsector=0;		
			}
		}
		for (;ridx<SECTORBYTES;ridx++)
		{
			unsigned char c;
			int n;
			c=tmp[ridx];
			if (lc!=0 || !rle)
			{
				n=1;
				lc=c;
			} else {
				lc=c;
				n=c-1;
				c=0;
			}
			for (i=0;i<n;i++)
			{
				pOut[outidx++]=c;
			}
			rlecutoff--;
			if (rle && rlecutoff==0)
			{
				rle=0;
				removeendmarker=1;	
			}
		}
		if (removeendmarker==1)
		{
			outidx-=4;	// remove the last 4 bytes	
		}
	}
	if (outidx>section.len) outidx=section.len;

	return outidx;
}
int loader_appleii_mkmag(unsigned char* magbuf,int* magsize,int gameid,unsigned char* pDskBuf,int diskcnt,int* pDskOffs)
{
	int magidx;
	int codesize;
	int stringidx0;
	int string1size;
	int string2size;
	int dictsize;
	int huffmantreeidx;
	int i;
	typedef	 struct _tGameInfo
	{
		int name_track;
		int name_sector;
		tSection code_section;
		tSection code2_section;
		int pivot_code2;
		tSection string1_section;
		tSection string2_section;
		tSection dict_section;
		int version;
		char gamename[21];
	} tGameInfo;

	const tGameInfo loader_appleii_gameInfo[4]={
		{0x01,0x0, {0x04,0x0,0,65536,1,0},{-1,-1,-1,-1,0,0},		-1,{0x12,0x0,0,0xc000,0,0},	{0x1e,0x0,0,0xb00,0,0},	{-1,-1,-1,0,0,0},	0,"The Pawn"},	
		{0x00,0x9, {0x03,0x9,0,65536,1,1},{-1,-1,-1,-1,0,0},		-1,{0x12,0xb,0,0xf100,0,0},	{0x21,0xc,0,0xe00,0,0},	{-1,-1,-1,0,0,0},	1,"The Guild of Thieves"},	

		{0x00,0x9, {0x08,0x2,0,0x3300,1,1},{0x00,0x0,1,0xcd00,1,0},	7,{0x0c,0xc,1, 57344,0,0},	{0x1a,0xc,1, 24832,0,0},	{0x06,0x0,0,  8704,1,0},	2,"Jinxter"},
		{0x00,0x9, {0x04,0x0,0,0x4200,1,0},{0x00,0x0,1,0xbe00,1,0},	2,{0x0b,0xe,1, 57344,0,0},	{0x19,0xe,1, 37120,0,0},	{0x08,0x2,0,  7680,1,0},	3,"Corruption"}
	};
	{
		int offs;
		int i;
		unsigned char c;
		printf("Detected '%s'\n",loader_appleii_gameInfo[gameid].gamename);
		offs=(loader_appleii_gameInfo[gameid].name_track*MAXSECTORS+loader_appleii_gameInfo[gameid].name_sector)*SECTORBYTES;
		offs+=3;
		i=0;
		c=0;
		printf("[");
		while (i<0x2c && c!=0xa9)
		{
			c=pDskBuf[offs];
			i++;
			offs++;
			if (c>=' ' && c<127) printf("%c",c);
		}
		printf("]\n");
	}

	magidx=42;
	codesize=loader_appleii_readsection(&magbuf[magidx],loader_appleii_gameInfo[gameid].code_section,pDskBuf,diskcnt,pDskOffs,0);
	codesize+=loader_appleii_readsection(&magbuf[magidx+codesize],loader_appleii_gameInfo[gameid].code2_section,pDskBuf,diskcnt,pDskOffs,loader_appleii_gameInfo[gameid].pivot_code2);
	magidx+=codesize;

	stringidx0=magidx;
	string1size=loader_appleii_readsection(&magbuf[magidx],loader_appleii_gameInfo[gameid].string1_section,pDskBuf,diskcnt,pDskOffs,0);
	magidx+=string1size;
	string2size=loader_appleii_readsection(&magbuf[magidx],loader_appleii_gameInfo[gameid].string2_section,pDskBuf,diskcnt,pDskOffs,0);
	magidx+=string2size;
	dictsize=loader_appleii_readsection(&magbuf[magidx],loader_appleii_gameInfo[gameid].dict_section,pDskBuf,diskcnt,pDskOffs,0);
	magidx+=dictsize;
	

	{
		int j;
		int matchcnt;
		huffmantreeidx=0;
		for (i=string1size;i<string1size+string2size-6 && huffmantreeidx==0;i++)
		{
			matchcnt=0;
			for (j=0;j<6;j++)
			{
				if (magbuf[stringidx0+i+j]==(j+1)) matchcnt++;
			}
			if (matchcnt>=4)
			{
				huffmantreeidx=i;
			}
		}
	}

	if (gameid==GAME_CORRUPTION) for (i=0x212a;i<0x232a;i++) magbuf[i]=0;	// finishing touches on corruption

	loader_common_addmagheader(magbuf,magidx,loader_appleii_gameInfo[gameid].version,codesize,string1size,string2size,dictsize,huffmantreeidx);
	*magsize=magidx;
	return 0;

}

int loader_appleii(char *appleiiname,
			char *magbuf,int* magsize,
			char *gfxbuf,int* gfxsize)
{
	unsigned char* pDskBuf;
	char filename[1024];
	unsigned char trackbuf[NIBTRACKSIZE];
	int i,l;
	int j;
	int diskcnt;
	int volumeids[MAXDISKS];
	int dskidx;
	int gameid;
	int diskoffs[MAXDISKS];
	FILE *f;

#define	SIZE_NIBIMAGE	232960
#define	SIZE_2MGIMAGE	143424	
#define	SIZE_DSKIMAGE	143360

	pDskBuf=(unsigned char*)&gfxbuf[4+32*4];
	l=strlen(appleiiname);
	j=0;
	diskcnt=0;
	dskidx=0;
	for (i=0;i<l+1 && diskcnt<MAXDISKS;i++)
	{
		if (appleiiname[i]!=',') filename[j++]=appleiiname[i];
		filename[j]=0;

		if (appleiiname[i]==',' || appleiiname[i]==0)
		{
			int n;
			int filesize;
			f=fopen(filename,"rb");
			fseek(f,0L,SEEK_END);
			filesize=ftell(f);
			fseek(f,0L,SEEK_SET);
			n=0;
			if (filesize==SIZE_NIBIMAGE)
			{	
				for (j=0;j<MAXTRACKS;j++)
				{
					n+=fread(trackbuf,sizeof(char),NIBTRACKSIZE,f);
					volumeids[diskcnt]=loader_appleii_decodenibtrack(trackbuf,j,&pDskBuf[dskidx]);
					dskidx+=MAXSECTORS*SECTORBYTES;
				}
			}
			if (filesize==SIZE_2MGIMAGE)
			{
				n+=fread(&pDskBuf[dskidx],sizeof(char),0x40,f);	// read in the header. https://apple2.org.za/gswv/a2zine/Docs/DiskImage_2MG_Info.txt
				volumeids[diskcnt]=pDskBuf[dskidx+0x10];	// according to my observations, this is where the volume ID is
				n+=fread(&pDskBuf[dskidx],sizeof(char),DSKSIZE,f);
				dskidx+=DSKSIZE;
			}
			fclose(f);
			printf("read %d bytes from [%s]. Volume ID [%02X]\n",n,filename,volumeids[diskcnt]);
			diskcnt++;
			j=0;
		}
	}
	gameid=-1;
	
	for (i=0;i<diskcnt;i++)
	{
		int newgameid;
		int disknum;
		
		disknum=-1;
		switch(volumeids[i])
		{
			case 0x68:	newgameid=GAME_PAWN;		disknum=volumeids[i]-0x68;break;
			case 0x69:	newgameid=GAME_GUILD;		disknum=volumeids[i]-0x69;break;
			case 0x70:
			case 0x71:	newgameid=GAME_JINXTER;		disknum=volumeids[i]-0x70;break;
			case 0x72:
			case 0x73:
			case 0x74:	newgameid=GAME_CORRUPTION;	disknum=volumeids[i]-0x72;break;
			default:
					return -1;
		}
		if (gameid==-1 || gameid==newgameid) gameid=newgameid;
		else {
			fprintf(stderr,"Game detection ambigous\n");
			return -1;
		}
		if (disknum>=0 && disknum<MAXDISKS)
		{
			diskoffs[disknum]=i*DSKSIZE;
		}
	}
	if (gameid==-1)
	{
		fprintf(stderr,"Unable to detect the game\n");
		return -1;
	}


	loader_appleii_mkmag((unsigned char*)magbuf,magsize,gameid,pDskBuf,diskcnt,diskoffs);	// since the memory for the gfx buffer includes the dskbuf, this is enough	
	return loader_appleii_mkgfx((unsigned char*)gfxbuf,gfxsize,gameid,diskcnt,diskoffs);	// since the memory for the gfx buffer includes the dskbuf, this is enough	
		
}


