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
#include "default_callbacks.h"
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
		fprintf(stderr,"This interpreter needs a the game's binaries in the .mag and .gfx\n");
		fprintf(stderr,"format from the Magnetic Scrolls Memorial webseiite. For details, \n");
		fprintf(stderr,"see https://msmemorial.if-legends.org/memorial.php\n");
		return -2;
	}
	magsize=fread(magbuf,sizeof(char),MAXMAGSIZE,f);
	fclose(f);
	f=fopen(gfxfilename,"rb");
	if (f==NULL)
	{
		fprintf(stderr,"ERROR: unable to open [%s]\n",gfxfilename);
		fprintf(stderr,"This interpreter needs a the game's binaries in the .mag and .gfx\n");
		fprintf(stderr,"format from the Magnetic Scrolls Memorial webseiite. For details, \n");
		fprintf(stderr,"see https://msmemorial.if-legends.org/memorial.php\n");
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
	
	// the relevant data from the game binaries has been copied.
	free(gfxbuf);
	free(magbuf);
	return 0;
}
int main(int argc,char** argv)
{
	char inifilename[1024];
	char magfilename[1024];
	char gfxfilename[1024];
	int retval;
	FILE *f_inifile=NULL;

	void* hVM68k;
	void* hLineA;
	void* hGUI;
	void* pSharedMem;
	int sharedmemsize;
	int sizeGUI;
	int unknownopcode;
	char *homedir;
	int gamenamegiven;
	


	// figure out the location of the inifile.
	magfilename[0]=0;
	gfxfilename[0]=0;
	if (!(retrievefromcommandline(argc,argv,"--version",NULL,0)))
	{
		fprintf(stderr,"*** dMagnetic %d.%d%d\n",VERSION_MAJOR,VERSION_MINOR,VERSION_REVISION);
		fprintf(stderr,"*** Use at your own risk\n");
		fprintf(stderr,"*** (C)opyright 2019 by dettus@dettus.net\n");
		fprintf(stderr,"*****************************************\n");	
		fprintf(stderr,"\n");

		f_inifile=NULL;
		if (f_inifile==NULL)
		{
			homedir=getenv("HOME");
			snprintf(inifilename,1023,"%s/dMagnetic.ini",homedir);
			f_inifile=fopen(inifilename,"rb");
		}
		if (f_inifile==NULL)
		{
			snprintf(inifilename,1023,"/etc/dMagnetic.ini");
			f_inifile=fopen(inifilename,"rb");
		}
		if (f_inifile==NULL)
		{
			snprintf(inifilename,1023,"/usr/local/share/dMagnetic.ini");
			f_inifile=fopen(inifilename,"rb");
		}
		if (f_inifile==NULL)
		{
			snprintf(inifilename,1023,"/usr/local/share/games/dMagnetic.ini");
			f_inifile=fopen(inifilename,"rb");
		}
		if (f_inifile==NULL)
		{
			snprintf(inifilename,1023,"/usr/local/share/dMagnetic/dMagnetic.ini");
			f_inifile=fopen(inifilename,"rb");
		}
		if (f_inifile==NULL)
		{
			snprintf(inifilename,1023,"/usr/local/games/dMagnetic.ini");
			f_inifile=fopen(inifilename,"rb");
		}
		if (f_inifile==NULL)
		{
			snprintf(inifilename,1023,"/usr/local/games/dMagnetic/dMagnetic.ini");
			f_inifile=fopen(inifilename,"rb");
		}
		if (f_inifile==NULL)
		{
			snprintf(inifilename,1023,"/usr/share/dMagnetic.ini");
			f_inifile=fopen(inifilename,"rb");
		}
		if (f_inifile==NULL)
		{
			snprintf(inifilename,1023,"/usr/share/games/dMagnetic.ini");
			f_inifile=fopen(inifilename,"rb");
		}
		if (f_inifile==NULL)
		{
			snprintf(inifilename,1023,"/usr/share/dMagnetic/dMagnetic.ini");
			f_inifile=fopen(inifilename,"rb");
		}
		if (f_inifile==NULL)
		{
			snprintf(inifilename,1023,"/usr/games/dMagnetic.ini");
			f_inifile=fopen(inifilename,"rb");
		}
		if (f_inifile==NULL)
		{
			snprintf(inifilename,1023,"/usr/games/dMagnetic/dMagnetic.ini");
			f_inifile=fopen(inifilename,"rb");
		}
		if (f_inifile==NULL)
		{
			snprintf(inifilename,1023,"./dMagnetic.ini");
			f_inifile=fopen(inifilename,"rb");
		}	

		if (f_inifile) 
		{
			fprintf(stderr,"Using %s\n",inifilename);		
			fclose(f_inifile);
		}
	}	
	if (argc<2)
	{
		fprintf(stderr,"Please run with %s [-ini dMagnetic.ini] GAME\n",argv[0]);
		fprintf(stderr," where GAME is one of\n");
		fprintf(stderr,"    pawn       To play \"The Pawn\"\n");
		fprintf(stderr,"    guild      To play \"The Guild of Thieves\"\n");
		fprintf(stderr,"    jinxter    To play \"Jinxter\"\n");
		fprintf(stderr,"    corruption To play \"Corruption\"\n");
		fprintf(stderr,"    fish       To play \"Fish!\"\n");
		fprintf(stderr,"    myth       To play \"Myth\"\n");
		fprintf(stderr,"    wonderland To play \"Wonderland\"\n");
		fprintf(stderr,"or\n");
		fprintf(stderr,"%s -mag MAGFILE.mag\n",argv[0]);
		fprintf(stderr,"\n");
		fprintf(stderr,"You can get the .mag and .gfx files from\n");
		fprintf(stderr," https://msmemorial.if-legends.org/\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"To get a more detailed help, please run\n");
		fprintf(stderr," %s --help\n",argv[0]);
		return 0;		
	}
	if ((retrievefromcommandline(argc,argv,"--version",NULL,0))
		|| (retrievefromcommandline(argc,argv,"-version",NULL,0))
		|| (retrievefromcommandline(argc,argv,"-v",NULL,0)))
	{
		printf("%d.%d%d\n",VERSION_MAJOR,VERSION_MINOR,VERSION_REVISION);
		return 1;
	}
	if (retrievefromcommandline(argc,argv,"-bsd",NULL,0))
	{
		printf("Copyright 2019, dettus@dettus.net\n");
		printf("\n");
		printf("Redistribution and use in source and binary forms, with or without modification,\n");
		printf("are permitted provided that the following conditions are met:\n");
		printf("\n");
		printf("1. Redistributions of source code must retain the above copyright notice, this \n");
		printf("   list of conditions and the following disclaimer.\n");
		printf("\n");
		printf("2. Redistributions in binary form must reproduce the above copyright notice, \n");
		printf("   this list of conditions and the following disclaimer in the documentation \n");
		printf("   and/or other materials provided with the distribution.\n");
		printf("\n");
		printf("THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND\n");
		printf("ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED \n");
		printf("WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE \n");
		printf("DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE \n");
		printf("FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL \n");
		printf("DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR \n");
		printf("SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER \n");
		printf("CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, \n");
		printf("OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE \n");
		printf("OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n");
		printf("\n");
		return 1;


	}
	if ((retrievefromcommandline(argc,argv,"--help",NULL,0))
	||  (retrievefromcommandline(argc,argv,"-help",NULL,0)))
	{
		printf("To start a game, run\n");	
		printf("%s [-ini dMagnetic.ini] GAME\n",argv[0]);
		printf("where GAME is one of \n  [");
		printf("pawn ");
		printf("guild ");
		printf("jinxter ");
		printf("corruption ");
		printf("fish ");
		printf("myth ");
		printf("wonderland]");
		printf("\n");
		printf("\n");
		printf("PARAMETERS TO RUN THE GAME\n");
		printf("-ini dMagnetic.ini   to provide an inifile\n");
		printf("-mag MAGFILE.mag     to provide the game binary directly\n");
		printf("-gfx GFXFILE.gfx     to provide the game graphics directly\n");
		printf("\n");
		printf("OPTIONAL PARAMETERS\n");
		printf("-vrows ROWS    to set the number of rows for the pictures\n");
		printf("-vcols COLUMNS to set the number of columns for the pictures\n");
		printf("-vmode MODE    where mode is one of\n  [");
			printf("none ");
			printf("monochrome ");
			printf("low_ansi ");
			printf("high_ansi");
			printf("]");
		printf("\n");
		printf("-vlog LOGFILE  to write a log of the commands used\n");
		printf("-vecho         to reprint the commands (useful for | tee)\n");
		printf("\n");
	
		printf(" OTHER PARAMETERS\n");
		printf(" -bsd shows the license\n");
		printf(" -help shows this help\n");
		printf(" -helpini shows an example dMagnetic.ini file\n");
		printf(" --version shows %d.%d%d\n",VERSION_MAJOR,VERSION_MINOR,VERSION_REVISION);
		return 1;
	}
	if ((retrievefromcommandline(argc,argv,"--helpini",NULL,0))
	||  (retrievefromcommandline(argc,argv,"-helpini",NULL,0)))
	{
		printf("Maybe you need to create a file called dMagnetic.ini\n");
		printf("Place it in your home directory, with the following content:\n");
		printf("\n");
		printf("--------------------------------------------------------------------------------\n");
		printf(";you can download the files from https://msmemorial.if-legends.org/magnetic.php\n");
		printf("[FILES]\n");
		printf("pawnmag=/usr/local/share/games/magneticscrolls/pawn.mag\n");
		printf("pawngfx=/usr/local/share/games/magneticscrolls/pawn.gfx\n");
		printf("guildmag=/usr/local/share/games/magneticscrolls/guild.mag\n");
		printf("guildgfx=/usr/local/share/games/magneticscrolls/guild.gfx\n");
		printf("jinxtermag=/usr/local/share/games/magneticscrolls/jinxter.mag\n");
		printf("jinxtergfx=/usr/local/share/games/magneticscrolls/jinxter.gfx\n");
		printf("corruptionmag=/usr/local/share/games/magneticscrolls/ccorrupt.mag\n");
		printf("corruptiongfx=/usr/local/share/games/magneticscrolls/ccorrupt.gfx\n");
		printf("fishmag=/usr/local/share/games/magneticscrolls/fish.mag\n");
		printf("fishgfx=/usr/local/share/games/magneticscrolls/fish.gfx\n");
		printf("mythmag=/usr/local/share/games/magneticscrolls/myth.mag\n");
		printf("mythgfx=/usr/local/share/games/magneticscrolls/myth.gfx\n");
		printf("wonderlandmag=/usr/local/share/games/magneticscrolls/wonder.mag\n");
		printf("wonderlandgfx=/usr/local/share/games/magneticscrolls/wonder.gfx\n");
		printf("\n");
		printf("[DEFAULTGUI]\n");
		printf("rows=40\n");
		printf("columns=120\n");
		printf(";align=left\n");
		printf("align=block\n");
		printf(";align=right\n");
		printf(";mode=none\n");
		printf(";mode=monochrome\n");
		printf(";mode=low_ansi\n");
		printf("mode=high_ansi\n");
		printf("low_ansi_characters=\\\\/|=\n");
		printf("monochrome_characters= .:-=+*x#@$X\n");
		printf("--------------------------------------------------------------------------------\n");
		return 1;
	}
	magfilename[0]=gfxfilename[0]=0;

	if (retrievefromcommandline(argc,argv,"-ini",inifilename,sizeof(inifilename))) 
	{
	}
	f_inifile=fopen(inifilename,"rb");
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
		fprintf(stderr,"error opening %s\n",inifilename);
		fprintf(stderr,"please run %s -helpini for more help\n",argv[0]);
		return 1;
	}
	if (retrievefromcommandline(argc,argv,"pawn",NULL,0))
	{
		magfilename[0]=gfxfilename[0]=0;
		retval=retrievefromini(f_inifile,"[FILES]","pawnmag",magfilename,sizeof(magfilename));
		retval=retrievefromini(f_inifile,"[FILES]","pawngfx",gfxfilename,sizeof(gfxfilename));
	}

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
	// command line parameters should overwrite the .ini parameters.
	if (retrievefromcommandline(argc,argv,"-mag",magfilename,sizeof(magfilename)))
	{
		gfxfilename[0]=0;
	}
	if (retrievefromcommandline(argc,argv,"-gfx",gfxfilename,sizeof(gfxfilename)))
	{

	}
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


///////////////////////////////////////////// init
	sharedmemsize=98304;
	pSharedMem=malloc(sharedmemsize);
	if (pSharedMem==NULL)
	{
		fprintf(stderr,"ERROR: unable to allocate shared memory\n");
		return 0;
	}
	retval=default_getsize(&sizeGUI);
	if (retval)
	{
		fprintf(stderr,"ERROR. default_getsize returned %d\n",retval);
		return 0;
	}
	hGUI=malloc(sizeGUI);
	if (hGUI==NULL)
	{
		fprintf(stderr,"ERROR: unable to locate memory for the GUI\n");
		return 0;
	}
	retval=default_open(hGUI,f_inifile,argc,argv);
	if (retval)
	{
		fprintf(stderr,"ERROR: opening the GUI failed\n");
		return 0;
	}




	if (f_inifile) fclose(f_inifile);

	// this is the main loop.
	do
	{
		retval=dMagnetic_init(&hVM68k,&hLineA,pSharedMem,sharedmemsize,magfilename,gfxfilename);
		if (retval)
		{
			return 0;
		}
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
			return 0;
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
	free(hGUI);
	free(hLineA);
	free(hVM68k);
	return 1;
}
