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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "version.h"
#include "configuration.h"
#include "vm68k.h"
#include "linea.h"
#include "default_callbacks.h"
#include "maggfxloader.h"
#define	SHAREDMEMSIZE	98304	// 98304 bytes ought to be enough for everybody
#define	MAXMAGSIZE	(1<<20)
#define	MAXGFXSIZE	(1<<22)


void dMagnetic_loaderfailed_message(char* argv0)
{
	fprintf(stderr,"Please run with %s [-ini dMagnetic.ini] GAME\n",argv0);
	fprintf(stderr," where GAME is one of\n");
	fprintf(stderr,"    pawn       To play \"The Pawn\"\n");
	fprintf(stderr,"    guild      To play \"The Guild of Thieves\"\n");
	fprintf(stderr,"    jinxter    To play \"Jinxter\"\n");
	fprintf(stderr,"    corruption To play \"Corruption\"\n");
	fprintf(stderr,"    fish       To play \"Fish!\"\n");
	fprintf(stderr,"    myth       To play \"Myth\"\n");
	fprintf(stderr,"    wonderland To play \"Wonderland\"\n");
	fprintf(stderr,"\n");
	fprintf(stderr,"or one of the following\n");
	fprintf(stderr,"\n");
	fprintf(stderr,"%s -mag MAGFILE.mag\n",argv0);
	fprintf(stderr,"%s -msdosdir DIRECTORY/\n",argv0);
	fprintf(stderr,"%s -tworsc DIRECTORY/TWO.RSC\n",argv0);
	fprintf(stderr,"%s -d64 IMAGE1.d64,IMAGE2.d64\n",argv0);
	fprintf(stderr,"%s -amstradcpc IMAGE1.DSK,IMAGE2.DSK\n",argv0);
	fprintf(stderr,"%s -spectrum IMAGE.DSK\n",argv0);
	fprintf(stderr,"%s -archimedes IMAGE.adf\n",argv0);
	fprintf(stderr,"\n");
	fprintf(stderr,"You can get the .mag and .gfx files from\n");
	fprintf(stderr," https://msmemorial.if-legends.org/\n");
	fprintf(stderr,"\n");
	fprintf(stderr,"To get a more detailed help, please run\n");
	fprintf(stderr," %s --help\n",argv0);
}
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
		fprintf(stderr,"*** dMagnetic %d.%d%d\n",VERSION_MAJOR,VERSION_MINOR,VERSION_REVISION);
		fprintf(stderr,"*** Use at your own risk\n");
		fprintf(stderr,"*** (C)opyright 2020 by dettus@dettus.net\n");
		fprintf(stderr,"*****************************************\n");	
		fprintf(stderr,"\n");
#define	LOCNUM	14
		const char *locations[LOCNUM]={"/etc/","/usr/local/share/","/usr/local/share/games/","/usr/local/share/dMagnetic/","/usr/local/games/","/usr/local/games/dMagnetic/","/usr/share/","/usr/share/games/","/usr/share/dMagnetic/","/usr/games/","/usr/games/dMagnetic/","/usr/share/doc/dmagnetic/","/usr/pkg/share/doc/dMagnetic/",
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
		dMagnetic_loaderfailed_message(argv[0]);
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
		printf("Copyright 2020, dettus@dettus.net\n");
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
		printf("-ini dMagnetic.ini       to provide an inifile\n");
		printf("-mag MAGFILE.mag         to provide the game binary directly\n");
		printf("-gfx GFXFILE.gfx         to provide the game graphics directly\n");
		printf("-msdosdir DIR/           to provide the game binaries from MSDOS\n");
		printf("-tworsc DIR/TWO.RSC      to use resource files from Wonderland\n");
		printf("                         or The Magnetic Scrolls Collection Vol.1\n");
		printf("-d64 m1.d64,m2.d64       or use the D64 images. (Separated by ,)\n");
		printf("-amstradcpc 1.DSK,2.DSK  or use the DSK images. (Separated by ,)\n");
		printf("-spectrum image.DSK      or use DSK images from the Spectrum+3\n");
		printf("-archimedes image.adf    or use adf/adl images from the Archimedes\n");
		printf("\n");
		printf("OPTIONAL PARAMETERS\n");
		printf("-rmode RANDOMMODE  where mode is one of\n  [");
			printf("pseudo ");
			printf("real");
			printf("]");
		printf("\n");
		printf("-rseed RANDOMSEED  to set the random seed between 1 and %d\n",0x7fffffff);

		printf("-vrows ROWS        to set the number of rows for the pictures\n");
		printf("-vcols COLUMNS     to set the number of columns for the pictures\n");
		printf("-vmode MODE        where mode is one of\n  [");
			printf("none ");
			printf("monochrome ");
			printf("monochrome_inv ");
			printf("low_ansi ");
			printf("low_ansi2 ");
			printf("high_ansi\n   ");
			printf("high_ansi2 ");
			printf("sixel");
			printf("]");
		printf("\n");
		printf("-vlog LOGFILE      to write a log of the commands used\n");
		printf("-vecho             to reprint the commands (useful for | tee)\n");
		printf("\n");
	
		printf(" OTHER PARAMETERS\n");
		printf(" -bsd shows the license\n");
		printf(" -ega prefers EGA images\n");
		printf(" -help shows this help\n");
		printf(" -helpini shows an example dMagnetic.ini file\n");
		printf(" --version shows %d.%d%d\n",VERSION_MAJOR,VERSION_MINOR,VERSION_REVISION);
		return 1;
	}
	if ((retrievefromcommandline(argc,argv,"--helpini",NULL,0))
	||  (retrievefromcommandline(argc,argv,"-helpini",NULL,0)))
	{
		printf(";Maybe you need to create a file called dMagnetic.ini\n");
		printf(";Place it in your home directory, with the following content:\n");
		printf(";\n");
		printf(";-------------------------------------------------------------------------------\n");
		printf(";you can download the files from https://msmemorial.if-legends.org/magnetic.php\n");
		printf("[FILES]\n");
		printf("pawnmag=/usr/local/share/games/magneticscrolls/pawn.mag\n");
		printf("pawngfx=/usr/local/share/games/magneticscrolls/pawn.gfx\n");
		printf(";pawnmsdos=/usr/local/share/games/magneticscrolls/msdosversions/PAWN\n");
		printf(";pawnd64=/d64/PAWN1.d64,/d64/PAWN2.d64\n");
		printf(";pawnamstradcpc=/dsk/PAWN1.DSK,/dsk/PAWN2.DSK\n");
		printf(";pawnspectrum=/dsk/PAWNspectrum.DSK\n");
		printf(";pawnarchimedes=/adf/PAWNarchimedes.adf\n");
		printf("guildmag=/usr/local/share/games/magneticscrolls/guild.mag\n");
		printf("guildgfx=/usr/local/share/games/magneticscrolls/guild.gfx\n");
		printf(";guildmsdos=/usr/local/share/games/magneticscrolls/msdosversions/GUILD\n");
		printf(";guildtworsc=/usr/local/share/games/magneticscrolls/MSC/GTWO.RSC\n");
		printf(";guild64=/d64/GUILD1.d64,/d64/GUILD2.d64\n");
		printf(";guildamstradcpc=/dsk/GUILD1.DSK,/dsk/GUILD2.DSK\n");
		printf(";guildspectrum=/dsk/GUILDspectrum.DSK\n");
		printf(";guildarchimedes=/adf/GUILDarchimedes.adf\n");
		printf("jinxtermag=/usr/local/share/games/magneticscrolls/jinxter.mag\n");
		printf("jinxtergfx=/usr/local/share/games/magneticscrolls/jinxter.gfx\n");
		printf(";jinxtermsdos=/usr/local/share/games/magneticscrolls/msdosversions/JINXTER\n");
		printf(";jinxterd64=/d64/JINXTER1.d64,/d64/JINXTER2.d64\n");
		printf(";jinxteramstradcpc=/dsk/JINXTER1.DSK,/dsk/JINXTER2.DSK\n");
		printf(";jinxterspectrum=/dsk/JINXTERspectrum.DSK\n");
		printf(";jinxterarchimedes=/adf/JINXTERarchimedes.adf\n");
		printf("corruptionmag=/usr/local/share/games/magneticscrolls/ccorrupt.mag\n");
		printf("corruptiongfx=/usr/local/share/games/magneticscrolls/ccorrupt.gfx\n");
		printf(";corruptionmsdos=/usr/local/share/games/magneticscrolls/msdosversions/CORRUPT\n");
		printf(";corruptiontworsc=/usr/local/share/games/magneticscrolls/MSC/CTWO.RSC\n");
		printf(";corruptiond64=/d64/CORRUPT1.d64,/d64/CORRUPT2.d64\n");
		printf(";corruptionamstradcpc=/dsk/CORRUPTION1.DSK,/dsk/CORRUPTION2.DSK\n");
		printf(";corruptionspectrum=/dsk/CORRUPTIONspectrum.DSK\n");
		printf(";corruptionarchimedes=/adf/CORRUPTIONarchimedes.adf\n");
		printf("fishmag=/usr/local/share/games/magneticscrolls/fish.mag\n");
		printf("fishgfx=/usr/local/share/games/magneticscrolls/fish.gfx\n");
		printf(";fishmsdos=/usr/local/share/games/magneticscrolls/msdosversions/FISH\n");
		printf(";fishtworsc=/usr/local/share/games/magneticscrolls/MSC/FTWO.RSC\n");
		printf(";fishd64=/d64/FISH1.d64,/d64/FISH2.d64\n");
		printf(";fishamstradcpc=/dsk/FISH1.DSK,/dsk/FISH2.DSK\n");
		printf(";fishspectrum=/dsk/FISHspectrum.DSK\n");
		printf(";fisharchimedes=/adf/FISHarchimedes.adf\n");
		printf("mythmag=/usr/local/share/games/magneticscrolls/myth.mag\n");
		printf("mythgfx=/usr/local/share/games/magneticscrolls/myth.gfx\n");
		printf(";mythmsdos=/usr/local/share/games/magneticscrolls/msdosversions/MYTH\n");
		printf(";mythd64=/usr/local/share/games/magneticscrolls/MYTH.d64\n");
		printf(";mythamstradcpc=/dsk/MYTH1.DSK,/dsk/MYTH2.DSK\n");
		printf(";mythspectrum=/dsk/MYTHspectrum.DSK\n");
		printf(";mytharchimedes=/adf/MYTHarchimedes.adf\n");
		printf("wonderlandmag=/usr/local/share/games/magneticscrolls/wonder.mag\n");
		printf("wonderlandgfx=/usr/local/share/games/magneticscrolls/wonder.gfx\n");
		printf(";wonderlandtworsc=/usr/local/share/games/magneticscrolls/WONDER/TWO.RSC\n");
		printf("\n");
		printf("[RANDOM]\n");
		printf("mode=pseudo\n");
		printf(";mode=real\n");
		printf("seed=12345\n");
		printf("\n");
		printf("[DEFAULTGUI]\n");
		printf("rows=40\n");
		printf("columns=120\n");
		printf(";align=left\n");
		printf("align=block\n");
		printf(";align=right\n");
		printf(";mode=none\n");
		printf(";mode=monochrome\n");
		printf(";mode=monochrome_inv\n");
		printf(";mode=low_ansi\n");
		printf("mode=low_ansi2\n");
		printf(";mode=high_ansi\n");
		printf(";mode=high_ansi2\n");
		printf(";mode=sixel\n");
		printf("low_ansi_characters=\\\\/|=abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\n");
		printf("monochrome_characters= .-=+*x#@$X\n");
		printf("sixel_resolution=800x600\n");
		printf(";-------------------------------------------------------------------------------\n");
		return 1;
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
				return 0;
			}
		}
		if (retrievefromini(f_inifile,"[RANDOM]","seed",result,sizeof(result)))
		{
			random_seed=atoi(result);
			if (random_seed<1 || random_seed>0x7fffffff)
			{
				printf("illegal random seed. please use a value between %d and %d\n",1,0x7fffffff);
				return 0;
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
				return 0;
			}
		}
		if (retrievefromcommandline(argc,argv,"-rseed",result,sizeof(result)))
		{
			random_seed=atoi(result);
			if (random_seed<1 || random_seed>0x7fffffff)
			{
				printf("illegal parameter for -rseed. please use a value between %d and %d\n",1,0x7fffffff);
				return 0;
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
			dMagnetic_loaderfailed_message(argv[0]);
			return 1;
		}


		retval=dMagnetic_init(&hVM68k,&hLineA,pSharedMem,SHAREDMEMSIZE,magbuf,magsize,gfxbuf,gfxsize);
		if (retval)
		{
			return 0;
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
			return 0;
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
	return 1;
}
