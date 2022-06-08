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
#define	KNOWN_GAMES	3	
const tGameInfo gameInfo[KNOWN_GAMES]={
	{"PAWN",		209529,0},
	{"GUILD",		185296,1},
	{"JINX",		159027,2},
//	{"CORR",		160678,3},
//	{"FILE",		162541,3}
};

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
		codesize=fread(&magbuf[magidx],sizeof(char),magsize0-magidx,f);
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
			unsigned char huffsize;
			unsigned char hufftab[256];
			int huffidx;
			unsigned short todo;
			unsigned char byte;
			unsigned char mask;
			int threebytes;
			int n;
			
			snprintf(filename,1024,"%s/%s0",msdosdir,gameInfo[gameid].prefix);
			OPENFILE(filename);
			n=fread(&huffsize,sizeof(char),1,f);
			n+=fread(hufftab,sizeof(char),huffsize,f);
			n+=fread(&todo,sizeof(short),1,f);	// what are those two bytes?
			dictsize=0;
			mask=0;
			huffidx=0;
			threebytes=0;
			while (!feof(f))
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
					// to extend this to 8 bits, each third symbol contains the
					// two MSB for the previous three:
					// 00AAAAAA 00BBBBBB 00CCCCCC 00aabbcc
					if (threebytes==3)
					{
						int i;
						threebytes=0;
						for (i=0;i<3;i++)
						{
							// add the 2 MSB to the existing 6.
							magbuf[magidx-3+i]|=((branch<<2)&0xc0);
							branch<<=2;	// MSB first
						}
					} else {
						magbuf[magidx]=branch;
						magidx++;
						dictsize++;
						threebytes++;
					}
				} else {
					huffidx=branch;
				}
				mask>>=1;
			}
			fclose(f);
			if (n==0) return -1;
			// &magbuf[magidx];
//			magidx+=dictsize;
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
		{
			FILE *g;
			g=fopen("debug.mag","wb");
			fwrite(magbuf,sizeof(char),magidx,g);
			fclose(g);
		}
		
		*magsize=magidx;
	}	
	return 0;
}

int loader_init(int argc,char** argv,FILE *f_inifile,
	char *magbuf,int* magsize,
	char* gfxbuf,int* gfxsize)
{
	FILE *f;
	char magfilename[1024];
	char gfxfilename[1024];
	char msdosdirname[1024];
	int gamenamegiven;
	int n;

	magfilename[0]=gfxfilename[0]=msdosdirname[0]=0;
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
	}

	if (retrievefromcommandline(argc,argv,"fish",NULL,0))
	{
		magfilename[0]=gfxfilename[0]=0;
		retrievefromini(f_inifile,"[FILES]","fishmag",magfilename,sizeof(magfilename));
		retrievefromini(f_inifile,"[FILES]","fishgfx",gfxfilename,sizeof(gfxfilename));
		retrievefromini(f_inifile,"[FILES]","fishmsdos",msdosdirname,sizeof(msdosdirname));
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
	}
	// command line parameters should overwrite the .ini parameters.
	if (retrievefromcommandline(argc,argv,"-mag",magfilename,sizeof(magfilename)))
	{
		gfxfilename[0]=0;
		msdosdirname[0]=0;
	}
	if (retrievefromcommandline(argc,argv,"-gfx",gfxfilename,sizeof(gfxfilename)))
	{
		msdosdirname[0]=0;
	}
	if (retrievefromcommandline(argc,argv,"-msdosdir",msdosdirname,sizeof(msdosdirname)))
	{
		gfxfilename[0]=magfilename[0]=0;
	}

	if (msdosdirname[0])
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
