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


int loader_init(int argc,char** argv,FILE *f_inifile,
		char *magbuf,int* magsize,
		char* gfxbuf,int* gfxsize)
{
	FILE *f;
	char magfilename[1024];
	char gfxfilename[1024];
	char msdosdirname[1024];
	char tworscname[1024];
	char d64name[1024];
	int gamenamegiven;
	int n;
	int retval;

	retval=0;
	magfilename[0]=gfxfilename[0]=msdosdirname[0]=tworscname[0]=d64name[0]=0;
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
		retrievefromini(f_inifile,"[FILES]","pawnd64",d64name,sizeof(d64name));
	}

	if (retrievefromcommandline(argc,argv,"guild",NULL,0))
	{
		magfilename[0]=gfxfilename[0]=0;
		retrievefromini(f_inifile,"[FILES]","guildmag",magfilename,sizeof(magfilename));
		retrievefromini(f_inifile,"[FILES]","guildgfx",gfxfilename,sizeof(gfxfilename));
		retrievefromini(f_inifile,"[FILES]","guildmsdos",msdosdirname,sizeof(msdosdirname));
		retrievefromini(f_inifile,"[FILES]","guildtworsc",tworscname,sizeof(tworscname));
		retrievefromini(f_inifile,"[FILES]","guildd64",d64name,sizeof(d64name));
	}

	if (retrievefromcommandline(argc,argv,"jinxter",NULL,0))
	{
		magfilename[0]=gfxfilename[0]=0;
		retrievefromini(f_inifile,"[FILES]","jinxtermag",magfilename,sizeof(magfilename));
		retrievefromini(f_inifile,"[FILES]","jinxtergfx",gfxfilename,sizeof(gfxfilename));
		retrievefromini(f_inifile,"[FILES]","jinxtermsdos",msdosdirname,sizeof(msdosdirname));
		retrievefromini(f_inifile,"[FILES]","jinxterd64",d64name,sizeof(d64name));
	}

	if (retrievefromcommandline(argc,argv,"corruption",NULL,0))
	{
		magfilename[0]=gfxfilename[0]=0;
		retrievefromini(f_inifile,"[FILES]","corruptionmag",magfilename,sizeof(magfilename));
		retrievefromini(f_inifile,"[FILES]","corruptiongfx",gfxfilename,sizeof(gfxfilename));
		retrievefromini(f_inifile,"[FILES]","corruptionmsdos",msdosdirname,sizeof(msdosdirname));
		retrievefromini(f_inifile,"[FILES]","corruptiontworsc",tworscname,sizeof(tworscname));
		retrievefromini(f_inifile,"[FILES]","corruptiond64",d64name,sizeof(d64name));
	}

	if (retrievefromcommandline(argc,argv,"fish",NULL,0))
	{
		magfilename[0]=gfxfilename[0]=0;
		retrievefromini(f_inifile,"[FILES]","fishmag",magfilename,sizeof(magfilename));
		retrievefromini(f_inifile,"[FILES]","fishgfx",gfxfilename,sizeof(gfxfilename));
		retrievefromini(f_inifile,"[FILES]","fishmsdos",msdosdirname,sizeof(msdosdirname));
		retrievefromini(f_inifile,"[FILES]","fishtworsc",tworscname,sizeof(tworscname));
		retrievefromini(f_inifile,"[FILES]","fishd64",d64name,sizeof(d64name));
	}

	if (retrievefromcommandline(argc,argv,"myth",NULL,0))
	{
		magfilename[0]=gfxfilename[0]=0;
		retrievefromini(f_inifile,"[FILES]","mythmag",magfilename,sizeof(magfilename));
		retrievefromini(f_inifile,"[FILES]","mythgfx",gfxfilename,sizeof(gfxfilename));
		retrievefromini(f_inifile,"[FILES]","mythmsdos",msdosdirname,sizeof(msdosdirname));
		retrievefromini(f_inifile,"[FILES]","mythd64",d64name,sizeof(d64name));
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
		d64name[0]=0;
	}
	if (retrievefromcommandline(argc,argv,"-gfx",gfxfilename,sizeof(gfxfilename)))
	{
		msdosdirname[0]=0;
		tworscname[0]=0;
		d64name[0]=0;
	}
	if (retrievefromcommandline(argc,argv,"-msdosdir",msdosdirname,sizeof(msdosdirname)))
	{
		gfxfilename[0]=magfilename[0]=0;
		tworscname[0]=0;
		d64name[0]=0;
	}
	if (retrievefromcommandline(argc,argv,"-tworsc",tworscname,sizeof(tworscname)))
	{
		gfxfilename[0]=magfilename[0]=0;
		msdosdirname[0]=0;
		d64name[0]=0;
	}
	if (retrievefromcommandline(argc,argv,"-d64",d64name,sizeof(tworscname)))
	{
		gfxfilename[0]=magfilename[0]=0;
		msdosdirname[0]=0;
	}
	if (tworscname[0])
	{
		retval=loader_magneticwindows(tworscname,magbuf,magsize,gfxbuf,gfxsize);
	}
	else if (d64name[0])
	{
		retval=loader_d64(d64name,magbuf,magsize,gfxbuf,gfxsize);	
	}
	else if (msdosdirname[0])
	{
		retval=loader_msdos(msdosdirname,magbuf,magsize,gfxbuf,gfxsize);	
	} else {
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
		// at this point, they are stored in magbuf and gfxbuf.

	}		

	return retval;


}
