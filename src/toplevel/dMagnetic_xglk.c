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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "version.h"
#include "configuration.h"
#include "vm68k.h"
#include "linea.h"
#include "glk.h"
#include "glkstart.h" 
#include "xglk_callbacks.h"


glkunix_argumentlist_t glkunix_arguments[] = {                                                                  
	{ NULL, glkunix_arg_End, NULL }                                                                             
};                                                                                                              

#define	MAXMAGSIZE	(1<<20)
#define	MAXGFXSIZE	(1<<22)

int dMagnetic_init(void** hVM68k,void** hLineA,void* pSharedMem,int memsize,char* magfilename,char* gfxfilename)
{
	int sizevm68k;
	int sizelineA;
	int retval;
	int version;
	char* magbuf;
	char* gfxbuf;
	int magsize;
	int gfxsize;
	FILE *f;

	// step 1: load the game binaries
	magbuf=malloc(MAXMAGSIZE);
	gfxbuf=malloc(MAXGFXSIZE);
	if (magbuf==NULL || gfxbuf==NULL) 
	{
		fprintf(stderr,"ERROR: unable to allocate memory for the data files\n");
		return -1;
	}
	f=fopen(magfilename,"rb");
	if (f==NULL)
	{
		fprintf(stderr,"ERROR: unable to open [%s]\n",magfilename);
		return -2;
	}
	magsize=fread(magbuf,sizeof(char),MAXMAGSIZE,f);
	fclose(f);
	f=fopen(gfxfilename,"rb");
	if (f==NULL)
	{
		fprintf(stderr,"ERROR: unable to open [%s]\n",gfxfilename);
		return -2;
	}
	gfxsize=fread(gfxbuf,sizeof(char),MAXGFXSIZE,f);
	fclose(f);

	// at this point, they are stored in magbuf and gfxbuf.


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
	retval=vm68k_init(*hVM68k,pSharedMem,memsize,version);
	if (retval)
	{
		fprintf(stderr,"ERROR: vm68k_init returned %d\n",retval);
		return retval;
	}

	// the relevant data from the game binaries has been copied.
	free(gfxbuf);
	free(magbuf);
	return 0;
}

int glkunix_startup_code(glkunix_startup_t *data)                                                               
{                                                                                                               
	return TRUE;                                                                                                
}                                                                                                               


void glk_main(void)
{
	char inifilename[1024];
	char magfilename[1024];
	char gfxfilename[1024];
	int retval;
	FILE *f_inifile=NULL;

	void* hVM68k;
	void* hLineA;
	void* hGlk;
	void* pSharedMem;
	int sharedmemsize;
	int sizeGUI;
	int unknownopcode;


	inifilename[0]=0;
	magfilename[0]=0;
	gfxfilename[0]=0;

	fprintf(stderr,"*** dMagnetic_xglk\n");
	fprintf(stderr,"*** Use at your own risk\n");
	fprintf(stderr,"*** (C)opyright 2019 by dettus@dettus.net\n");
	fprintf(stderr,"*****************************************\n");	
	fprintf(stderr,"\n");

	f_inifile=fopen("dMagnetic.ini","rb");
	if (!f_inifile) 
	{
		fprintf(stderr,"error opening %s\n",inifilename);
		return;
	}
	//if (retrievefromcommandline(argc,argv,"pawn",NULL,0))
	if (0)
	{
		magfilename[0]=gfxfilename[0]=0;
		retval=retrievefromini(f_inifile,"[FILES]","pawnmag",magfilename,sizeof(magfilename));
		retval=retrievefromini(f_inifile,"[FILES]","pawngfx",gfxfilename,sizeof(gfxfilename));
	}
	if (1)//retrievefromcommandline(argc,argv,"guild",NULL,0))
	{
		magfilename[0]=gfxfilename[0]=0;
		retval=retrievefromini(f_inifile,"[FILES]","guildmag",magfilename,sizeof(magfilename));
		retval=retrievefromini(f_inifile,"[FILES]","guildgfx",gfxfilename,sizeof(gfxfilename));
	}
	/*
	   if (retrievefromcommandline(argc,argv,"guild",NULL,0))
	   {
	   magfilename[0]=gfxfilename[0]=0;
	   retval=retrievefromini(f_inifile,"[FILES]","guildmag",magfilename,sizeof(magfilename));
	   retval=retrievefromini(f_inifile,"[FILES]","guildgfx",gfxfilename,sizeof(gfxfilename));
	   }

	   if (retrievefromcommandline(argc,argv,"jinxter",NULL,0))
	   {
	   magfilename[0]=gfxfilename[0]=0;
	   retval=retrievefromini(f_inifile,"[FILES]","jinxtermag",magfilename,sizeof(magfilename));
	   retval=retrievefromini(f_inifile,"[FILES]","jinxtergfx",gfxfilename,sizeof(gfxfilename));
	   }

	   if (retrievefromcommandline(argc,argv,"corruption",NULL,0))
	   {
	   magfilename[0]=gfxfilename[0]=0;
	   retval=retrievefromini(f_inifile,"[FILES]","corruptionmag",magfilename,sizeof(magfilename));
	   retval=retrievefromini(f_inifile,"[FILES]","corruptiongfx",gfxfilename,sizeof(gfxfilename));
	   }

	   if (retrievefromcommandline(argc,argv,"fish",NULL,0))
	   {
	   magfilename[0]=gfxfilename[0]=0;
	   retval=retrievefromini(f_inifile,"[FILES]","fishmag",magfilename,sizeof(magfilename));
	   retval=retrievefromini(f_inifile,"[FILES]","fishgfx",gfxfilename,sizeof(gfxfilename));
	   }

	   if (retrievefromcommandline(argc,argv,"myth",NULL,0))
	   {
	   magfilename[0]=gfxfilename[0]=0;
	   retval=retrievefromini(f_inifile,"[FILES]","mythmag",magfilename,sizeof(magfilename));
	   retval=retrievefromini(f_inifile,"[FILES]","mythgfx",gfxfilename,sizeof(gfxfilename));
	   }


	   if (retrievefromcommandline(argc,argv,"wonderland",NULL,0))
	   {
	   magfilename[0]=gfxfilename[0]=0;
	   retval=retrievefromini(f_inifile,"[FILES]","wonderlandmag",magfilename,sizeof(magfilename));
	   retval=retrievefromini(f_inifile,"[FILES]","wonderlandgfx",gfxfilename,sizeof(gfxfilename));
	   }
	 */

	///////////////////////////////////////////// init
	sharedmemsize=98304;
	pSharedMem=malloc(sharedmemsize);
	if (pSharedMem==NULL)
	{
		fprintf(stderr,"ERROR: unable to allocate shared memory\n");
		return;
	}
	retval=xglk_getsize(&sizeGUI);
	if (retval)
	{
		fprintf(stderr,"ERROR. default_getsize returned %d\n",retval);
		return;
	}
	hGlk=malloc(sizeGUI);
	if (hGlk==NULL)
	{
		fprintf(stderr,"ERROR: unable to locate memory for the GLK\n");
		return;
	}
	retval=xglk_open(hGlk);
	if (retval)
	{
		fprintf(stderr,"ERROR: opening the GUI failed\n");
		return;
	}




	if (f_inifile) fclose(f_inifile);

	// this is the main loop.
	do
	{
		retval=dMagnetic_init(&hVM68k,&hLineA,pSharedMem,sharedmemsize,magfilename,gfxfilename);
		if (retval)
		{
			return;
		}
		// set the call back hooks for this GUI
		retval|=lineA_setCBoutputChar(hLineA,xglk_cbOutputChar,		hGlk);
		retval|=lineA_setCBoutputString(hLineA,xglk_cbOutputString,	hGlk);
		retval|=lineA_setCBinputString(hLineA,xglk_cbInputString,	hGlk);
		retval|=lineA_setCBDrawPicture(hLineA,xglk_cbDrawPicture,	hGlk);
		retval|=lineA_setCBLoadGame(hLineA,xglk_cbLoadGame,		hGlk);
		retval|=lineA_setCBSaveGame(hLineA,xglk_cbSaveGame,		hGlk);
		if (retval)
		{
			fprintf(stderr,"ERROR: setting the API hooks failed\n");
			return;
		}
		/////////////////////////////////////////////		

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
	// this concludes the main loop

	free(pSharedMem);
	free(hGlk);
	free(hLineA);
	free(hVM68k);
	return;
}
