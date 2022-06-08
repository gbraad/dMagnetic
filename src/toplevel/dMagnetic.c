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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "version.h"
#include "configuration.h"
#include "vm68k.h"
#include "linea.h"
#include "default_callbacks.h"
#include "maggfxloader.h"
#include "dMagnetic_helpscreens.h"
#include "dMagnetic_pathnames.h"
#define	SHAREDMEMSIZE	98304	// 98304 bytes ought to be enough for everybody
#define	MAXMAGSIZE	(1<<20)
#define	MAXGFXSIZE	(1<<22)


int dMagnetic_init(void** hVM68k,void** hLineA,void* pSharedMem,int memsize,char* magbuf,int magsize,char* gfxbuf,int gfxsize)
{
	int sizevm68k;
	int sizelineA;
	int retval;
	int version;




	// step 2: start the engine.
	retval=vm68k_getsize(&sizevm68k);
	if (retval) 
	{
		fprintf(stderr,"ERROR: vm68k_getsize returned %d\n",retval);
		return retval;
	}
	retval=lineA_getsize(&sizelineA);
	if (retval) 
	{
		fprintf(stderr,"ERROR: lineA_getsize returned %d\n",retval);
		return retval;
	}
	*hVM68k=malloc(sizevm68k);
	*hLineA=malloc(sizelineA);
	if (*hVM68k==NULL || *hLineA==NULL) 
	{
		fprintf(stderr,"ERROR: unable to allocate memory for the engine\n");
		return -1;
	}
	
	retval=lineA_init(*hLineA,pSharedMem,&memsize,magbuf,magsize,gfxbuf,gfxsize);
	if (retval)
	{
		fprintf(stderr,"ERROR: lineA_init returned %d\n",retval);
		return retval;
	}
	retval=lineA_getVersion(*hLineA,&version);
	if (retval)
	{
		fprintf(stderr,"ERROR: lineA_getversion returned %d\n",retval);
		return retval;
	}
	fprintf(stderr,"Initializing the 68K version %d VM\n",version);	
	if (version==4)
	{
		fprintf(stderr,"\n");
		fprintf(stderr,"---------------------------------------\n");
		fprintf(stderr,"- Version 4 of the VM requires you to -\n");
		fprintf(stderr,"- activate the graphics manually. Use -\n");
		fprintf(stderr,"- the command       GRAPHICS      now -\n");
		fprintf(stderr,"---------------------------------------\n");
		fprintf(stderr,"\n");
	}
	retval=vm68k_init(*hVM68k,pSharedMem,memsize,version);
	if (retval)
	{
		fprintf(stderr,"ERROR: vm68k_init returned %d\n",retval);
		return retval;
	}
	
	return 0;
}
int main(int argc,char** argv)
{
	int i;
	char inifilename[1024];
	int retval;
	FILE *f_inifile=NULL;

	void* hVM68k;
	void* hLineA;
	void* hGUI;
	void* pSharedMem;
	int sizeGUI;
	int unknownopcode;
	char *homedir;
	char random_mode;
	unsigned int random_seed;
	int egamode;
	char* magbuf;	
	char* gfxbuf;
	


	// figure out the location of the inifile.
	if (!(retrievefromcommandline(argc,argv,"--version",NULL,0)))
	{
		dMagnetic_helpscreens_header();
#define	LOCNUM	14
		const char *locations[LOCNUM]={
			PATH_ETC,
			PATH_USR_LOCAL_SHARE,
			PATH_USR_LOCAL_SHARE_GAMES,
			PATH_USR_LOCAL_SHARE"dMagnetic/",
			PATH_USR_LOCAL_GAMES,
			PATH_USR_LOCAL_GAMES"dMagnetic/",
			PATH_USR_SHARE,
			PATH_USR_SHARE_GAMES,
			PATH_USR_SHARE"dMagnetic/",
			PATH_USR_GAMES,
			PATH_USR_GAMES"dMagnetic/",
			PATH_USR_SHARE"doc/dmagnetic/",
			PATH_USR_PKG_SHARE"doc/dMagnetic/",
			"./"};	// this should always be the last one.

		f_inifile=NULL;
		if (f_inifile==NULL)
		{
			homedir=getenv("HOME");
			snprintf(inifilename,1023,"%s/dMagnetic.ini",homedir);
			f_inifile=fopen(inifilename,"rb");
		}
		for (i=0;i<LOCNUM;i++)
		{
			if (f_inifile==NULL)
			{
				snprintf(inifilename,1023,"%sdMagnetic.ini",locations[i]);
				f_inifile=fopen(inifilename,"rb");
			}
		}

		if (f_inifile) 
		{
			fclose(f_inifile);
		}
	}	
	if (argc<2)
	{
		dMagnetic_helpscreens_loaderfailed(argv[0]);
		return 1;
	}
	if ((retrievefromcommandline(argc,argv,"--version",NULL,0))
		|| (retrievefromcommandline(argc,argv,"-version",NULL,0))
		|| (retrievefromcommandline(argc,argv,"-v",NULL,0)))
	{
		printf("%d.%d%d\n",VERSION_MAJOR,VERSION_MINOR,VERSION_REVISION);
		return 0;
	}
	if (retrievefromcommandline(argc,argv,"-bsd",NULL,0))
	{
		dMagnetic_helpscreens_license();
		return 0;


	}
	if ((retrievefromcommandline(argc,argv,"--help",NULL,0))
	||  (retrievefromcommandline(argc,argv,"-help",NULL,0)))
	{
		dMagnetic_helpscreens_help(argv[0]);
		return 0;
	}
	if ((retrievefromcommandline(argc,argv,"--helpini",NULL,0))
	||  (retrievefromcommandline(argc,argv,"-helpini",NULL,0)))
	{
		dMagnetic_helpscreens_helpini();
		return 0;
	}
	if (retrievefromcommandline(argc,argv,"-ini",inifilename,sizeof(inifilename))) 
	{
	}
	fprintf(stderr,"Using .ini file: %s\n",inifilename);		
	f_inifile=fopen(inifilename,"rb");
	pSharedMem=malloc(SHAREDMEMSIZE);
	if (pSharedMem==NULL)
	{
		fprintf(stderr,"ERROR: unable to allocate shared memory\n");
		return 1;
	}
	retval=default_getsize(&sizeGUI);
	if (retval)
	{
		fprintf(stderr,"ERROR. default_getsize returned %d\n",retval);
		return 1;
	}
	hGUI=malloc(sizeGUI);
	if (hGUI==NULL)
	{
		fprintf(stderr,"ERROR: unable to locate memory for the GUI\n");
		return 1;
	}
	retval=default_open(hGUI,f_inifile,argc,argv);
	if (retval)
	{
		fprintf(stderr,"ERROR: opening the GUI failed\n");
		return 1;
	}

//////////////////////////////////////////////// random
	random_mode=0;
	random_seed=12345;
	if (f_inifile)
	{
		char result[64];

		if (retrievefromini(f_inifile,"[RANDOM]","mode",result,sizeof(result)))
		{
			if (result[0]=='p') random_mode=0;
			else if (result[0]=='r') random_mode=1;
			else {
				printf("illegal random mode in inifile. please use one of ");
				printf("pseudo ");
				printf("real ");
				return 1;
			}
		}
		if (retrievefromini(f_inifile,"[RANDOM]","seed",result,sizeof(result)))
		{
			random_seed=atoi(result);
			if (random_seed<1 || random_seed>0x7fffffff)
			{
				printf("illegal random seed. please use a value between %d and %d\n",1,0x7fffffff);
				return 1;
			}
		}
	}
		

	if (argc)
	{
		char result[64];
		if (retrievefromcommandline(argc,argv,"-rmode",result,sizeof(result)))
		{
			if (result[0]=='p') random_mode=0;
			else if (result[0]=='r') random_mode=1;
			else {
				printf("illegal parameter for -rmode. please use one of ");
				printf("pseudo ");
				printf("real ");
				printf("\n");
				return 1;
			}
		}
		if (retrievefromcommandline(argc,argv,"-rseed",result,sizeof(result)))
		{
			random_seed=atoi(result);
			if (random_seed<1 || random_seed>0x7fffffff)
			{
				printf("illegal parameter for -rseed. please use a value between %d and %d\n",1,0x7fffffff);
				return 1;
			}
		}
	}	
	egamode=0;
	if (f_inifile)
	{
		char result[64];
		if (retrievefromcommandline(argc,argv,"-ega",result,sizeof(result)))
		{
			egamode=1;
		}
	}


	if (f_inifile) fclose(f_inifile);
	magbuf=malloc(MAXMAGSIZE);
	gfxbuf=malloc(MAXGFXSIZE);

	// this is the main loop.
	do
	{
		int magsize;
		int gfxsize;

		magsize=MAXMAGSIZE;
		gfxsize=MAXGFXSIZE;
		if (magbuf==NULL || gfxbuf==NULL) 
		{
			fprintf(stderr,"ERROR: unable to allocate memory for the data files\n");
			return -1;
		}
		f_inifile=fopen(inifilename,"rb");
		if (loader_init(argc,argv,f_inifile, magbuf,&magsize,gfxbuf,&gfxsize))
		{
			dMagnetic_helpscreens_loaderfailed(argv[0]);
			return 1;
		}


		retval=dMagnetic_init(&hVM68k,&hLineA,pSharedMem,SHAREDMEMSIZE,magbuf,magsize,gfxbuf,gfxsize);
		if (retval)
		{
			return 1;
		}
		retval|=lineA_configrandom(hLineA,random_mode,random_seed);
		retval|=lineA_setEGAMode(hLineA,egamode);
		// set the call back hooks for this GUI
		retval|=lineA_setCBoutputChar(hLineA,default_cbOutputChar,	hGUI);
		retval|=lineA_setCBoutputString(hLineA,default_cbOutputString,	hGUI);
		retval|=lineA_setCBinputString(hLineA,default_cbInputString,	hGUI);
		retval|=lineA_setCBDrawPicture(hLineA,default_cbDrawPicture,	hGUI);
		retval|=lineA_setCBLoadGame(hLineA,default_cbLoadGame,		hGUI);
		retval|=lineA_setCBSaveGame(hLineA,default_cbSaveGame,		hGUI);
		if (retval)
		{
			fprintf(stderr,"ERROR: setting the API hooks failed\n");
			return 1;
		}
		/////////////////////////////////////////////		

		lineA_showTitleScreen(hLineA);	// some versions of the game have a title screen.
		// everything is good. have fun playing!
		do
		{
			unsigned short opcode;
			unknownopcode=0;
			retval=vm68k_getNextOpcode(hVM68k,&opcode);
			if (retval==VM68K_OK) retval=lineA_substitute_aliases(hLineA,&opcode);
			if (retval==LINEA_OK) retval=vm68k_singlestep(hVM68k,opcode);
			if (retval!=VM68K_OK) retval=lineA_singlestep(hLineA,hVM68k,opcode);
			if (retval!=LINEA_OK && retval!=LINEA_OK_QUIT && retval!=LINEA_OK_RESTART) 
			{
				fprintf(stderr,"\x1b[0m\n\x1b[0;37;44m Sorry. Unknown opcode %04X\x1b[0m\n",opcode);
				unknownopcode=1;
			}
			//		if (retval==LINEA_OK_QUIT) printf("\x1b[0m\n\x1b[0;37;44mGoodbye!\x1b[0m\n");
		} while (!unknownopcode && !retval);	
	} while (retval==LINEA_OK_RESTART);
	free(gfxbuf);
	free(magbuf);
	// this concludes the main loop

	free(pSharedMem);
	free(hGUI);
	free(hLineA);
	free(hVM68k);
	return 0;
}
