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
#include "dMagnetic_helpscreens.h"
#include "dMagnetic_pathnames.h"
#include "version.h"

#define	NUMGAMES	7
#define	NUMPLATFORMS	10

#define	PLATFORM_MAG		(1<<0)
#define	PLATFORM_GFX		(1<<1)
#define	PLATFORM_MSDOS		(1<<2)
#define	PLATFORM_TWORSC		(1<<3)
#define	PLATFORM_D64		(1<<4)
#define	PLATFORM_AMSTRADCPC	(1<<5)
#define	PLATFORM_SPECTRUM	(1<<6)
#define	PLATFORM_ARCHIMEDES	(1<<7)
#define	PLATFORM_ATARIXL	(1<<8)
#define	PLATFORM_APPLEII	(1<<9)

typedef	struct _tPlatformInfo
{
	char* name;
	char* dir;
	char* suffix;
	char uppercase;
	char special;
	int disksexpected;	// some games require komma seperated names for the images. PAWN1.D64,PAWN2.D64, for example
	int active;
	unsigned short mask;
	char *cmdline;
} tPlatformInfo;
typedef struct _tGameInfo
{
	char *name;
	char *description;
	char *maggfxname;
	int disknum;
	int special;
	char *specialPrefix;
	unsigned short ported;
} tGameInfo;

const tPlatformInfo	cdMagnetic_platformInfo[NUMPLATFORMS]={
	{"mag",		PATH_USR_LOCAL_SHARE_GAMES"magneticscrolls/",	".mag"	,0,0,1,1,	PLATFORM_MAG,		"-mag MAGFILE.mag"},
	{"gfx",		PATH_USR_LOCAL_SHARE_GAMES"magneticscrolls/",	".gfx"	,0,0,1,1,	PLATFORM_GFX,		"-gfx GFXFILE.gfx"},
	{"msdos",	"/MSDOS/C/",					""	,1,0,1,0,	PLATFORM_MSDOS,		"-msdosdir DIRECTORY/"},
	{"tworsc",	PATH_USR_LOCAL_SHARE"games/",			"TWO.RSC",0,1,1,0,	PLATFORM_TWORSC,	"-tworsc DIRECTORY/TWO.RSC"},
	{"d64",		"/8/",						".D64"	,1,0,2,0,	PLATFORM_D64,		"-d64 IMAGE1.d64,IMAGE2.d64"},
	{"amstradcpc",	"/dsk/amstradcpc/",				".DSK"	,1,0,2,0,	PLATFORM_AMSTRADCPC,	"-amstradcpc IMAGE1.DSK,IMAGE2.DSK"},
	{"spectrum",	"/dsk/spectrum/",				".DSK"	,0,0,1,0,	PLATFORM_SPECTRUM,	"-spectrum IMAGE.DSK"},
	{"archimedes",	"/adf/",					".adf"	,0,0,1,0,	PLATFORM_ARCHIMEDES,	"-archimedes IMAGE.adf"},
	{"atarixl",	"/atr/",					".ATR"	,1,0,2,0,	PLATFORM_ATARIXL,	"-atarixl IMAGE1.atr,IMAGE2.atr"},
	{"appleii",	"/appleii/",					".NIB"	,1,0,3,0,	PLATFORM_APPLEII,	"-appleii IMAGE1.NIB,IMAGE2.NIB,IMAGE3.NIB"}
};
const tGameInfo		cdMagnetic_gameInfo[NUMGAMES]={
	{"pawn",	"The Pawn",		"pawn",		2,0,""			,PLATFORM_MAG|PLATFORM_GFX|PLATFORM_MSDOS|PLATFORM_D64|PLATFORM_AMSTRADCPC|PLATFORM_SPECTRUM|PLATFORM_ARCHIMEDES|PLATFORM_ATARIXL                |PLATFORM_APPLEII},
	{"guild",	"The Guild of Thieves",	"guild",	2,1,"MSC/G"		,PLATFORM_MAG|PLATFORM_GFX|PLATFORM_MSDOS|PLATFORM_D64|PLATFORM_AMSTRADCPC|PLATFORM_SPECTRUM|PLATFORM_ARCHIMEDES|PLATFORM_ATARIXL|PLATFORM_TWORSC|PLATFORM_APPLEII},
	{"jinxter",	"Jinxter",		"jinxter",	2,0,""			,PLATFORM_MAG|PLATFORM_GFX|PLATFORM_MSDOS|PLATFORM_D64|PLATFORM_AMSTRADCPC|PLATFORM_SPECTRUM|PLATFORM_ARCHIMEDES|PLATFORM_ATARIXL                |PLATFORM_APPLEII},
	{"corruption",	"Corruption",		"corrupt",	3,1,"MSC/C"		,PLATFORM_MAG|PLATFORM_GFX|PLATFORM_MSDOS|PLATFORM_D64|PLATFORM_AMSTRADCPC|PLATFORM_SPECTRUM|PLATFORM_ARCHIMEDES                 |PLATFORM_TWORSC|PLATFORM_APPLEII},
	{"fish",	"Fish!",		"fish",		2,1,"MSC/F"		,PLATFORM_MAG|PLATFORM_GFX|PLATFORM_MSDOS|PLATFORM_D64|PLATFORM_AMSTRADCPC|PLATFORM_SPECTRUM|PLATFORM_ARCHIMEDES                 |PLATFORM_TWORSC},
	{"myth",	"Myth",			"myth",		1,0,""			,PLATFORM_MAG|PLATFORM_GFX|PLATFORM_MSDOS|PLATFORM_D64|PLATFORM_AMSTRADCPC|PLATFORM_SPECTRUM                                                     },
	{"wonderland",	"Wonderland",		"wonder",	1,1,"wonderland/"	,PLATFORM_MAG|PLATFORM_GFX                                                                                                       |PLATFORM_TWORSC},
};

void dMagnetic_helpscreens_header()
{
	fprintf(stderr,"*** dMagnetic %d.%d%d\n",VERSION_MAJOR,VERSION_MINOR,VERSION_REVISION);
	fprintf(stderr,"*** Use at your own risk\n");
	fprintf(stderr,"*** (C)opyright 2021 by dettus@dettus.net\n");
	fprintf(stderr,"*****************************************\n");	
	fprintf(stderr,"\n");
}
void dMagnetic_helpscreens_license()
{
	printf("Copyright 2021, dettus@dettus.net\n");
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
}
void dMagnetic_helpscreens_help(char* argv0)
{
	int i;
	printf("To start a game, run\n");	
	printf("%s [-ini dMagnetic.ini] GAME\n",argv0);
	printf("where GAME is one of \n  [");
	for (i=0;i<NUMGAMES;i++)
	{
		if (i) printf(" ");
		printf("%s",cdMagnetic_gameInfo[i].name);
	}
	printf("]");
	printf("\n");
	printf("\n");
	printf("PARAMETERS TO RUN THE GAME\n");
	printf("-ini dMagnetic.ini          to provide an inifile\n");
	printf("-mag MAGFILE.mag            to provide the game binary directly\n");
	printf("-gfx GFXFILE.gfx            to provide the game graphics directly\n");
	printf("-msdosdir DIR/              to provide the game binaries from MSDOS\n");
	printf("-tworsc DIR/TWO.RSC         to use resource files from Wonderland\n");
	printf("                            or The Magnetic Scrolls Collection Vol.1\n");
	printf("-d64 m1.d64,m2.d64          or use the D64 images. (Separated by ,)\n");
	printf("-amstradcpc 1.DSK,2.DSK     or use the DSK images. (Separated by ,)\n");
	printf("-spectrum image.DSK         or use DSK images from the Spectrum+3\n");
	printf("-archimedes image.adf       or use adf/adl images from the Archimedes\n");
	printf("-atarixl 1.ATR,2.ATR        or use .atr images from the AtariXL\n");
	printf("-appleii 1.NIB,2.NIB,3.NIB  or use .nib images for the Apple ][\n");
	printf("-appleii 1.2MG,2.2MG,3.2MG  or use .2MG images for the Apple ][\n");
	printf("-appleii 1.WOZ,2.WOZ,3.WOZ  or use .woz images for the Apple ][\n");
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
	printf("none");
	printf(" monochrome");
	printf(" monochrome_inv");
	printf(" low_ansi");
	printf(" low_ansi2");
	printf(" high_ansi\n  ");
	printf(" high_ansi2");
	printf(" sixel");
	printf(" utf");
	printf("]");
	printf("\n");
	printf("-vlog LOGFILE      to write a log of the commands used\n");
	printf("-vecho             to reprint the commands (useful for | tee)\n");
	printf("\n");
	printf("The sixel output mode can be customized with the following parameters\n");
	printf("-sres 1024x768     render the pictures in this resolution\n");
	printf("-sforce            force the resolution (ignore the aspect ratio)\n");

	printf(" OTHER PARAMETERS\n");
	printf(" -bsd shows the license\n");
	printf(" -dumpmag GAME.mag -dumpgfx GAME.gfx writes the internal game data\n");
	printf(" -ega prefers EGA images\n");
	printf(" -help shows this help\n");
	printf(" -helpini shows an example dMagnetic.ini file\n");
	printf(" --version shows %d.%d%d\n",VERSION_MAJOR,VERSION_MINOR,VERSION_REVISION);

}
void dMagnetic_helpscreens_helpini()
{
	int i,j;
	printf(";Maybe you need to create a file called dMagnetic.ini\n");
	printf(";Place it in your home directory, with the following content:\n");
	printf(";\n");
	printf(";-------------------------------------------------------------------------------\n");
	printf(";you can download the files from https://msmemorial.if-legends.org/magnetic.php\n");
	printf("[FILES]\n");

	for (i=0;i<NUMGAMES;i++)
	{
		for (j=0;j<NUMPLATFORMS;j++)
		{
			if (cdMagnetic_gameInfo[i].ported&cdMagnetic_platformInfo[j].mask)
			{
				int k;
				char uppercase[16];
				for (k=0;k<strlen(cdMagnetic_gameInfo[i].name)+1;k++)
				{
					uppercase[k]=cdMagnetic_gameInfo[i].name[k]&0x5f;
				}
				if (!cdMagnetic_platformInfo[j].active) printf(";");
				printf("%s%s=",cdMagnetic_gameInfo[i].name,cdMagnetic_platformInfo[j].name);
				if (cdMagnetic_platformInfo[j].special)
				{
					printf("%s",cdMagnetic_platformInfo[j].dir);
					printf("%s",cdMagnetic_gameInfo[i].specialPrefix);
					printf("%s",cdMagnetic_platformInfo[j].suffix);
				} else {
					int l;
					int num;
					num=(cdMagnetic_platformInfo[j].disksexpected<cdMagnetic_gameInfo[i].disknum)?cdMagnetic_platformInfo[j].disksexpected:cdMagnetic_gameInfo[i].disknum;
					for (l=0;l<num;l++)
					{
						if (l) printf(",");
						printf("%s",cdMagnetic_platformInfo[j].dir);
						printf("%s",cdMagnetic_platformInfo[j].uppercase?uppercase:cdMagnetic_gameInfo[i].maggfxname);
						if (num!=1) printf("%d",(l+1));
						printf("%s",cdMagnetic_platformInfo[j].suffix);
					}
				
				}
				printf("\n");
			}
		}
	}

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
	printf(";mode=utf\n");
	printf(";low_ansi_characters=\\\\/|=abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\n");
	printf("low_ansi_characters=\\\\/|=L#TX\n");
	printf("monochrome_characters=\\ .,-=oxOX@$\n");
	printf("sixel_resolution=800x600\n");
	printf("sixel_forceresolution=No\n");
	printf(";-------------------------------------------------------------------------------\n");
}
void dMagnetic_helpscreens_loaderfailed(char* argv0)
{
	int i;
	fprintf(stderr,"Please run with %s [-ini dMagnetic.ini] GAME\n",argv0);
	fprintf(stderr," where GAME is one of\n");
	for (i=0;i<NUMGAMES;i++)
	{
		fprintf(stderr,"    %-10s to play \"%s\"\n",cdMagnetic_gameInfo[i].name,cdMagnetic_gameInfo[i].description);
	}
	fprintf(stderr,"\n");
	fprintf(stderr,"or one of the following\n");
	fprintf(stderr,"\n");
	for (i=0;i<NUMPLATFORMS;i++)
	{
		fprintf(stderr,"%s %s\n",argv0,cdMagnetic_platformInfo[i].cmdline);
	}
	fprintf(stderr,"\n");
	fprintf(stderr,"You can get the .mag and .gfx files from\n");
	fprintf(stderr," https://msmemorial.if-legends.org/\n");
	fprintf(stderr,"\n");
	fprintf(stderr,"To get a more detailed help, please run\n");
	fprintf(stderr," %s --help\n",argv0);
}
