/*

   Copyright 2022, dettus@dettus.net

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
#include "loader_msdos.h"
#include "loader_mw.h"
#include "loader_d64.h"
#include "loader_dsk.h"
#include "loader_archimedes.h"
#include "loader_atarixl.h"
#include "loader_appleii.h"


typedef	enum _eBinType
{
	BINTYPE_NONE,
	BINTYPE_MAGGFX,
	BINTYPE_MSDOS,
	BINTYPE_TWORSC,
	BINTYPE_D64,
	BINTYPE_AMSTRADCPC,
	BINTYPE_SPECTRUM,
	BINTYPE_ARCHIMEDES,
	BINTYPE_ATARIXL,
	BINTYPE_APPLEII
} eBinType;
int loader_init(int argc,char** argv,FILE *f_inifile,
		char *magbuf,int* magsize,
		char* gfxbuf,int* gfxsize)
{
	FILE *f;
	char magfilename[1024];
	char gfxfilename[1024];
	char binname[1024];
	eBinType binType;
	
	int gamenamegiven;
	int n;
	int retval;
	int nodoc;
	nodoc=0;

	binType=BINTYPE_NONE;

	retval=0;
	magfilename[0]=gfxfilename[0]=binname[0]=0;
	gamenamegiven=0;
	if ((retrievefromcommandline(argc,argv,"pawn",NULL,0))
			|| (retrievefromcommandline(argc,argv,"guild",NULL,0))
			|| (retrievefromcommandline(argc,argv,"jinxter",NULL,0))
			|| (retrievefromcommandline(argc,argv,"corruption",NULL,0))
			|| (retrievefromcommandline(argc,argv,"fish",NULL,0))
			|| (retrievefromcommandline(argc,argv,"myth",NULL,0))
			|| (retrievefromcommandline(argc,argv,"wonderland",NULL,0))
			|| (retrievefromcommandline(argc,argv,"wonder",NULL,0)))
	{
		gamenamegiven=1;
	}
	if (!f_inifile && gamenamegiven) 
	{
		fprintf(stderr,"Game name was given, but no suitable .ini file found\n");
		fprintf(stderr,"please run %s -helpini for more help\n",argv[0]);
		return 1;
	}
	{
		int i;
		char* gameprefix[]={"pawn","guild","jinxter","corruption","fish","myth","wonderland","wonder"};
		char magname[32];
		char gfxname[32];
		char tworscname[32];
		char msdosname[32];
		char d64name[32];
		char amstradcpcname[32];
		char spectrumname[32];
		char archimedesname[32];
		char atarixlname[32];
		char appleiiname[32];

		for (i=0;i<8;i++)
		{
			snprintf(magname,32,"%smag",gameprefix[i]);
			snprintf(gfxname,32,"%sgfx",gameprefix[i]);
			snprintf(tworscname,32,"%stworsc",gameprefix[i]);
			snprintf(msdosname,32,"%smsdos",gameprefix[i]);
			snprintf(d64name,32,"%sd64",gameprefix[i]);
			snprintf(amstradcpcname,32,"%samstradcpc",gameprefix[i]);
			snprintf(spectrumname,32,"%sspectrum",gameprefix[i]);
			snprintf(archimedesname,32,"%sarchimedes",gameprefix[i]);
			snprintf(atarixlname,32,"%satarixl",gameprefix[i]);
			snprintf(appleiiname,32,"%sappleii",gameprefix[i]);

			if (retrievefromcommandline(argc,argv,gameprefix[i],NULL,0))
			{
				magfilename[0]=gfxfilename[0]=0;
				if (retrievefromini(f_inifile,"[FILES]",magname,magfilename,sizeof(magfilename))&&
						retrievefromini(f_inifile,"[FILES]",gfxname,gfxfilename,sizeof(gfxfilename)))
				{
					binType=BINTYPE_MAGGFX;
				}
				else if (retrievefromini(f_inifile,"[FILES]",tworscname,binname,sizeof(binname)))
				{
					binType=BINTYPE_TWORSC;
				}
				else if (retrievefromini(f_inifile,"[FILES]",msdosname,binname,sizeof(binname)))
				{
					binType=BINTYPE_MSDOS;
				}
				else if (retrievefromini(f_inifile,"[FILES]",d64name,binname,sizeof(binname)))
				{
					binType=BINTYPE_D64;
				}
				else if (retrievefromini(f_inifile,"[FILES]",amstradcpcname,binname,sizeof(binname)))
				{
					binType=BINTYPE_AMSTRADCPC;
				}
				else if (retrievefromini(f_inifile,"[FILES]",spectrumname,binname,sizeof(binname)))
				{
					binType=BINTYPE_SPECTRUM;
				}
				else if (retrievefromini(f_inifile,"[FILES]",archimedesname,binname,sizeof(binname)))
				{
					binType=BINTYPE_ARCHIMEDES;
				}
				else if (retrievefromini(f_inifile,"[FILES]",atarixlname,binname,sizeof(binname)))
				{
					binType=BINTYPE_ATARIXL;
				}
				else if (retrievefromini(f_inifile,"[FILES]",appleiiname,binname,sizeof(binname)))
				{
					binType=BINTYPE_APPLEII;
				}
			}
		}
	}
	if (retrievefromcommandline(argc,argv,"-mag",magfilename,sizeof(magfilename)))
	{
		gfxfilename[0]=0;
		binname[0]=0;
		binType=BINTYPE_MAGGFX;
	}
	if (retrievefromcommandline(argc,argv,"-gfx",gfxfilename,sizeof(gfxfilename)))
	{
		binname[0]=0;
		binType=BINTYPE_MAGGFX;
	}
	if (retrievefromcommandline(argc,argv,"-msdosdir",binname,sizeof(binname)))
	{
		binType=BINTYPE_MSDOS;
	}
	if (retrievefromcommandline(argc,argv,"-tworsc",binname,sizeof(binname)))
	{
		binType=BINTYPE_TWORSC;
	}
	if (retrievefromcommandline(argc,argv,"-d64",binname,sizeof(binname)))
	{
		binType=BINTYPE_D64;
	}
	if (retrievefromcommandline(argc,argv,"-amstradcpc",binname,sizeof(binname)))
	{
		binType=BINTYPE_AMSTRADCPC;
	}
	if (retrievefromcommandline(argc,argv,"-spectrum",binname,sizeof(binname)))
	{
		binType=BINTYPE_SPECTRUM;
	}
	if (retrievefromcommandline(argc,argv,"-archimedes",binname,sizeof(binname)))
	{
		binType=BINTYPE_ARCHIMEDES;
	}
	if (retrievefromcommandline(argc,argv,"-atarixl",binname,sizeof(binname)))
	{
		binType=BINTYPE_ATARIXL;
	}
	if (retrievefromcommandline(argc,argv,"-appleii",binname,sizeof(binname)))
	{
		binType=BINTYPE_APPLEII;
	}
	{
		char result[64];
		nodoc=0;
		if (retrievefromini(f_inifile,"[GAMEPLAY]","nodoc",result,sizeof(result)))
		{
			nodoc=atoi(result);
		}
		if (retrievefromcommandline(argc,argv,"-nodoc",NULL,0))
		{
			nodoc=1;
		}
	}

	switch (binType)
	{
		case BINTYPE_NONE:		fprintf(stderr,"Please provide the game binaries\n");return -1;break;
		case BINTYPE_TWORSC:		retval=loader_magneticwindows(binname,magbuf,magsize,gfxbuf,gfxsize);break;
		case BINTYPE_D64:		retval=loader_d64(binname,magbuf,magsize,gfxbuf,gfxsize,nodoc);break;
		case BINTYPE_AMSTRADCPC:	retval=loader_dsk(binname,magbuf,magsize,gfxbuf,gfxsize,0,nodoc);	break;
		case BINTYPE_SPECTRUM:		retval=loader_dsk(binname,magbuf,magsize,gfxbuf,gfxsize,1,nodoc);break;
		case BINTYPE_ARCHIMEDES:	retval=loader_archimedes(binname,magbuf,magsize,gfxbuf,gfxsize,nodoc); break;
		case BINTYPE_ATARIXL:		retval=loader_atarixl(binname,magbuf,magsize,gfxbuf,gfxsize,nodoc); break;
		case BINTYPE_APPLEII:		retval=loader_appleii(binname,magbuf,magsize,gfxbuf,gfxsize,nodoc); break;
		case BINTYPE_MSDOS:		retval=loader_msdos(binname,magbuf,magsize,gfxbuf,gfxsize,nodoc);	break;
		case BINTYPE_MAGGFX:	
			retval=0;
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
				return 1;
			}
			f=fopen(magfilename,"rb");
			if (f==NULL)
			{
				fprintf(stderr,"ERROR: unable to open [%s]\n",magfilename);
				fprintf(stderr,"This interpreter needs a the game's binaries in the .mag and .gfx\n");
				fprintf(stderr,"format from the Magnetic Scrolls Memorial website. For details, \n");
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
				fprintf(stderr,"format from the Magnetic Scrolls Memorial website. For details, \n");
				fprintf(stderr,"see https://msmemorial.if-legends.org/memorial.php\n");
				return -2;
			}
			n=fread(gfxbuf,sizeof(char),*gfxsize,f);
			*gfxsize=n;
			fclose(f);
			break;
	}
	// at this point, they are stored in magbuf and gfxbuf.
	{
		FILE *f;
		int finish;
		finish=0;
		if (retrievefromcommandline(argc,argv,"-dumpmag",magfilename,sizeof(magfilename)))
		{
			finish=1;
			printf("Writing new .mag file [%s]\n",magfilename);
			f=fopen(magfilename,"wb");
			if (!f)
			{
				fprintf(stderr,"unable to open [%s]\n",magfilename);
			}
			fwrite(magbuf,sizeof(char),*magsize,f);
			fclose(f);
		}
		if (retrievefromcommandline(argc,argv,"-dumpgfx",gfxfilename,sizeof(gfxfilename)))
		{
			finish=1;
			printf("Writing new .gfx file [%s]\n",gfxfilename);
			f=fopen(gfxfilename,"wb");
			if (!f)
			{
				fprintf(stderr,"unable to open [%s]\n",gfxfilename);
			}
			fwrite(gfxbuf,sizeof(char),*gfxsize,f);
			fclose(f);
		}
		if (finish)
		{
			printf("finishing now\n");
			exit(0);
		}
	}
	return retval;


}
