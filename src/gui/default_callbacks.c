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

// the purpose of this file is to provide a callback-fallback until proper
// user interfaces have been established. It will do ;)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "default_render.h"
#include "picture.h"
#include "configuration.h"
#include "default_callbacks.h"

#define	MAGIC	0x68654879	// = yHeh, the place where I grew up ;)
typedef	struct _tContext
{
	unsigned int magic;
	// ansi output
	int	columns;
	int	rows;
	int	mode;	// 0=none. 1=ascii art. 2=ansi art. 3=high ansi	
	// log the input
	FILE* f_logfile;
	int	echomode;

	int	capital;	// =1 if the next character should be a capital letter.
	int	lastchar;	
	int headlineflagged;
	char	low_ansi_characters[128];	// characters that are allowed for low ansi rendering
	char	monochrome_characters[128];	// characters that are allowed for monochrome rendering

// this is the line buffer.
// text is stored here, before
// being printed on the console.
	int	textalign;	// 0=left aligned. 1=block aligned
	int	textlastspace;
	int	textidx;
	char	textoutput[640];	
} tContext;


int default_flushOutputLeftAlign(tContext* pContext,int newline)
{
	if (newline || pContext->textlastspace==-1)
	{
		pContext->textoutput[pContext->textidx++]=0;
		printf("%s",pContext->textoutput);
		pContext->textidx=0;
	} else {
		int i;
		int j;
		pContext->textoutput[pContext->textlastspace]=0;
		printf("%s\n",pContext->textoutput);
		j=0;
		for (i=pContext->textlastspace+1;i<pContext->textidx;i++)
		{
			pContext->textoutput[j++]=pContext->textoutput[i];
		}	
		pContext->textidx=j;
	}
	pContext->textlastspace=-1;
	return 0;

}
int default_flushOutputRightAlign(tContext* pContext,int newline)
{
	int i;
	if (newline || pContext->textlastspace==-1)
	{
		for (i=0;i<pContext->columns-pContext->textidx;i++) printf(" ");
		pContext->textoutput[pContext->textidx++]=0;
		printf("%s",pContext->textoutput);
		pContext->textidx=0;
	} else {
		int j;
		for (i=0;i<pContext->columns-pContext->textlastspace;i++) printf(" ");
		pContext->textoutput[pContext->textlastspace]=0;
		printf("%s\n",pContext->textoutput);
		j=0;
		for (i=pContext->textlastspace+1;i<pContext->textidx;i++)
		{
			pContext->textoutput[j++]=pContext->textoutput[i];
		}	
		pContext->textidx=j;
	}
	pContext->textlastspace=-1;
	return 0;

}
int default_flushOutputBlockAlign(tContext* pContext,int newline)
{
	if (newline || pContext->textlastspace==-1)
	{
		pContext->textoutput[pContext->textidx++]=0;
		printf("%s",pContext->textoutput);
		pContext->textidx=0;
	} else {
		int i;
		int j;
		int spacecnt;
		int rightmargin;
		int accu;

		spacecnt=0;
		rightmargin=pContext->columns-pContext->textlastspace;
		pContext->textoutput[pContext->textlastspace]=0;
		for (i=0;i<pContext->textlastspace;i++)
		{
			if (pContext->textoutput[i]==' ') spacecnt++;
		}
		accu=0;	
		for (i=0;i<pContext->textlastspace;i++)
		{
			if (pContext->textoutput[i]!=' ') printf("%c",pContext->textoutput[i]);
			else {
				printf(" ");
				accu+=rightmargin;
				{
					while (accu>=spacecnt)
					{
						printf(" ");
						accu-=spacecnt;
					}
				}
			}
		}	
		printf("\n");
		j=0;
		for (i=pContext->textlastspace+1;i<pContext->textidx;i++)
		{
			pContext->textoutput[j++]=pContext->textoutput[i];
		}	
		pContext->textidx=j;
	}
	pContext->textlastspace=-1;
	return 0;

}
int default_flushOutput(tContext* pContext,int newline)
{
	switch(pContext->textalign)
	{
		case 1:
			return default_flushOutputBlockAlign(pContext,newline);
		case 2:
			return default_flushOutputRightAlign(pContext,newline);
		case 0:
		default:
			return default_flushOutputLeftAlign(pContext,newline);
			break;
	}
	return 0;
}

int default_cbOutputChar(void* context,char c,unsigned char controlD2,unsigned char flag_headline)
{
	tContext *pContext=(tContext*)context;
	int newline;
	unsigned char c2;
	if (flag_headline && !pContext->headlineflagged) 
	{
		// highlight the headline
		printf("\x1b[0;30;47m");
	}
	if (!flag_headline && pContext->headlineflagged) 	// after the headline ends, a new paragraph is beginning.
	{
		pContext->capital=1;	// obviously, this starts with a captial letter.
		printf("\x1b[0m\n");	// stop highlighting.
	}
	pContext->headlineflagged=flag_headline;

	newline=0;
	if ((unsigned char)c==0xff) 
	{
		pContext->capital=1;
	} else {
		c2=c&0x7f;	// the highest bit was an end marker for the hufman tree in the dictionary.

		// THE RULES FOR THE OUTPUT ARE:
		// replace tabs and _ with space.
		// the headline is printed in upper case letters.
		// after a . there has to be a space.
		// and after a . The next letter has to be uppercase.
		// multiple spaces are to be reduced to a single one.
		// the characters ~ and ^ are to be transplated into line feeds.
		// the caracter 0xff makes the next one upper case.

		if (c2==9 || c2=='_') c2=' ';
		if (flag_headline && (c2==0x5f || c2==0x40)) c2=' ';	// in a headline, those are the control codes for a space.
		if (controlD2 && c2==0x40) return 0;	// end marker
		if (c2==0x5e || c2==0x7e) c2=0x0a;	// ~ or ^ is actually a line feed.
		if (c2=='.' || c2=='!' || c2==':' || c2=='?')	// a sentence is ending.
		{
			pContext->capital=1;	
		}
		if (((c2>='a' && c2<='z') || (c2>='A' && c2<='Z')) && (pContext->capital||flag_headline)) 
		{
			pContext->capital=0;
			c2&=0x5f;	// upper case
		}
		newline=0;
		if (
				(pContext->lastchar=='.' || pContext->lastchar=='!' || pContext->lastchar==':' || pContext->lastchar=='?'|| pContext->lastchar==',' || pContext->lastchar==';') 	// a sentence as ended
				&&  ((c2>='A' && c2<='Z') ||(c2>='a' && c2<='z') ||(c2>='0' && c2<='9'))) 	// and a new one is beginning.
		{
			if (flag_headline) 
			{
				printf(" ");	// after those letters comes an extra space.
			}
			else {
				pContext->textlastspace=pContext->textidx;
				pContext->textoutput[pContext->textidx++]=' ';
			}
			pContext->lastchar=' ';
		}
		if (pContext->textidx>0 && pContext->lastchar==' ' && (c2==',' || c2==':' || c2==';' || c2=='.' || c2=='!'))	// there have been some glitches with extra spaces, right before a komma. which , as you can see , looks weird.
		{
			pContext->textidx--;	
		}
		if (pContext->lastchar!=' ' || c2!=' ')	// combine multiple spaces into a single one.
		{	
			if (c2==0x0a || (c2>=32 && c2<127 && c2!='@')) 
			{
				if (flag_headline) 
				{
					printf("%c",c2&0x7f);
				} else {
					if (c2==' ') pContext->textlastspace=pContext->textidx;
					pContext->textoutput[pContext->textidx++]=c2;

					if (c2=='\n') newline=1;
				}
				pContext->lastchar=c2;
			}
		}
		if (newline || pContext->textidx>=pContext->columns || pContext->textidx>=511)
		{
			default_flushOutput(pContext,newline);
		}
	}
	return 0;
}
int default_cbOutputString(void* context,char* string,unsigned char controlD2,unsigned char flag_headline)
{
	unsigned char c2;
	int i;
	i=0;
	c2=0;
	do
	{
		c2=string[i++]&0x7f;
		//if (c2==9) c2=' ';
		//if (c2>=32 && c2<127) printf("%c",c2);
		default_cbOutputChar(context,c2,controlD2,flag_headline);
	} while (c2>=32);
	return 0;
}
int default_cbInputString(void* context,int* len,char* string)
{
	int l;
	tContext *pContext=(tContext*)context;
	//printf("? ");
	default_flushOutput(pContext,1);

	fflush(stdout);
	if (feof(stdin)) exit(0);
	if (fgets(string,256,stdin)==NULL) exit(0);
	if (pContext->echomode)
	{
		for (l=0;l<strlen(string);l++) 
		{
			if (string[l]>='a' && string[l]<='z') 
			{
				printf("%c",string[l]&0x5f);
			} else {
				printf("%c",string[l]);	// print upper case letters.
			}
		}
	}
	if (pContext->f_logfile)
	{
		fprintf(pContext->f_logfile,"%s",string);
		fflush(pContext->f_logfile);
	}
	l=strlen(string);
	*len=l;
	pContext->capital=1;	// the next line after the input is most definiately the beginning of a sentence! 
	return 0;
}
int default_cbDrawPicture(void* context,tPicture* picture,int mode)
{
	int i;
	int j;
	int rgb;
	int accux,accuy;
	tContext *pContext=(tContext*)context;
	int lastrgb;

	if (pContext->mode==0) return 0;
	// flush the output buffer
	default_cbOutputChar(context,'\n',0,0);
	if (pContext->mode==1)
	{
		default_render_monochrome(pContext->monochrome_characters,picture,pContext->rows,pContext->columns);	
	}

	if (pContext->mode==2)
	{
		default_render_lowansi(pContext->low_ansi_characters,picture,pContext->rows,pContext->columns);	
	}

	if (pContext->mode==3)
	{
		accux=accuy=0;
		for (i=0;i<picture->height;i++)
		{
			accuy+=pContext->rows;
			lastrgb=-1;
			if (accuy>=picture->height || i==picture->height-1)
			{
				accux=0;
				for (j=0;j<picture->width;j++)
				{
					accux+=pContext->columns;
					if (accux>=picture->width || j==picture->width-1)
					{
						rgb=picture->palette[(int)(picture->pixels[i*(picture->width)+j])];
						if (rgb!=lastrgb)
						{
							printf("\x1b[48;2;%d;%d;%dm",
									((rgb>>8)&0xf)*0x18,
									((rgb>>4)&0xf)*0x18,
									((rgb>>0)&0xf)*0x18);
						}
						printf(" ");
						lastrgb=rgb;
						accux-=picture->width;
					}
				}		

				accuy-=picture->height;	
				printf("\x1b[0m\n");
			}		
		}
	}

	return 0;
}
int default_cbSaveGame(void* context,char* filename,void* ptr,int len)
{
	FILE *f;
	int n;
	f=fopen(filename,"wb");
	if (!f)
	{
		printf("Unable to open file [%s]\n",(char*)ptr);
		return 0;
	}
	n=fwrite(ptr,sizeof(char),len,f);	
	fclose(f);
	if (n==len) return 0;
	return -1;
}

int default_cbLoadGame(void* context,char* filename,void* ptr,int len)
{
	FILE *f;
	int n;
	f=fopen(filename,"rb");
	if (!f)
	{
		printf("Unable to open file [%s]\n",(char*)ptr);
		return 0;
	}
	n=fread(ptr,sizeof(char),len,f);	
	fclose(f);
	if (n==len) return 0;
	return -1;
}

int default_getsize(int* size)
{
	if (size==NULL) return DEFAULT_NOK;
	*size=sizeof(tContext);
	return DEFAULT_OK;
}
int default_open(void* hContext,FILE *f_inifile,int argc,char** argv)
{
	tContext *pContext=(tContext*)hContext;
	char result[1024];
	#define	DEFAULT_LOW_ANSI_CHARACTERS 8
	const char default_low_ansi_characters[DEFAULT_LOW_ANSI_CHARACTERS]="\\/|=L#T";
	#define	DEFAULT_MONOCHROME_CHARACTERS 14
	const char default_monochrome_characters[DEFAULT_MONOCHROME_CHARACTERS]=" .:-=+*x#/@$X";

	if (pContext==NULL) return DEFAULT_NOK;
	memset(pContext,0,sizeof(tContext));
	pContext->magic=MAGIC;

	// the hiearchy is: first the default values. 
	// if there is a .ini file, overwrite them with those values.
	// paramaters from the command line have the highest prioprity.

	pContext->rows=40;
	pContext->columns=120;
	pContext->mode=2;	// 0=none. 1=monochrome. 2=low_ansi. 3=high_ansi
	pContext->f_logfile=NULL;
	pContext->echomode=0;
	pContext->textalign=1;
	pContext->textidx=0;
	pContext->textlastspace=-1;
	
	memcpy(pContext->low_ansi_characters,default_low_ansi_characters,sizeof(default_low_ansi_characters));
	memcpy(pContext->monochrome_characters,default_monochrome_characters,sizeof(default_monochrome_characters));


	if (f_inifile)
	{
		if (retrievefromini(f_inifile,"[DEFAULTGUI]","rows",result,sizeof(result))) 
		{
			pContext->rows=atoi(result);
		}
		if (retrievefromini(f_inifile,"[DEFAULTGUI]","columns",result,sizeof(result))) 
		{
			pContext->columns=atoi(result);
		}
		if (retrievefromini(f_inifile,"[DEFAULTGUI]","mode",result,sizeof(result)))
		{
			if (strncmp(result,"none",4)==0) pContext->mode=0;	
			if (strncmp(result,"monochrome",10)==0) pContext->mode=1;
			if (strncmp(result,"low_ansi",8)==0) pContext->mode=2;
			if (strncmp(result,"high_ansi",9)==0) pContext->mode=3;
		}
		if (retrievefromini(f_inifile,"[DEFAULTGUI]","align",result,sizeof(result)))
		{
			if (strncmp(result,"left",4)==0) pContext->textalign=0;
			if (strncmp(result,"block",5)==0) pContext->textalign=1;
			if (strncmp(result,"right",5)==0) pContext->textalign=2;
		}
		if (retrievefromini(f_inifile,"[DEFAULTGUI]","low_ansi_characters",pContext->low_ansi_characters,sizeof(pContext->low_ansi_characters)))
		{
		
		} else {
			memcpy(pContext->low_ansi_characters,default_low_ansi_characters,DEFAULT_LOW_ANSI_CHARACTERS);
		}
		if (retrievefromini(f_inifile,"[DEFAULTGUI]","monochrome_characters",pContext->monochrome_characters,sizeof(pContext->monochrome_characters)))
		{

		} else {
			memcpy(pContext->monochrome_characters,default_monochrome_characters,DEFAULT_MONOCHROME_CHARACTERS);
		}
	}

	if (argc)
	{
		char result[64];
		if (retrievefromcommandline(argc,argv,"-valign",result,64))
		{
			if (strncmp(result,"left",4)==0) pContext->textalign=0;	
			else if (strncmp(result,"block",5)==0) pContext->textalign=1;
			else if (strncmp(result,"right",5)==0) pContext->textalign=2;
			else {
				printf("unknown parameter for -valign. please use one of\n");
				printf("left ");
				printf("block ");
				printf("right ");
				printf("\n");	
				return DEFAULT_NOK;
			}	
		}
		if (retrievefromcommandline(argc,argv,"-vmode",result,64))
		{
			if (strncmp(result,"none",4)==0) pContext->mode=0;	
			else if (strncmp(result,"monochrome",10)==0) pContext->mode=1;
			else if (strncmp(result,"low_ansi",8)==0) pContext->mode=2;
			else if (strncmp(result,"high_ansi",9)==0) pContext->mode=3;
			else {
				printf("unknown parameter for -vmode. please use one of\n");
				printf("none ");
				printf("monochrome ");
				printf("low_ansi ");
				printf("high_ansi ");
				printf("\n");	
				return DEFAULT_NOK;
			}	
		}
		if (retrievefromcommandline(argc,argv,"-vrows",result,64))
		{
			int rows;
			rows=atoi(result);
			if (rows<1 || rows>500) 
			{
				printf("illegal parameter for -vrows. please use values between 1 and 500\n");
				return DEFAULT_NOK;
			}
			pContext->rows=rows;
		}
		if (retrievefromcommandline(argc,argv,"-vcols",result,64))
		{
			int cols;
			cols=atoi(result);
			if (cols<1 || cols>500) 
			{
				printf("illegal parameter for -vcols. please use values between 1 and 500\n");
				return DEFAULT_NOK;
			}
			pContext->columns=cols;
		}
		if (retrievefromcommandline(argc,argv,"-vecho",NULL,0))
		{
			pContext->echomode=1;
		}
		if (retrievefromcommandline(argc,argv,"-vlog",result,64))
		{
			fprintf(stderr,"Opening logfile [%s] for writing\n",result);
			pContext->f_logfile=fopen(result,"wb");	
			if (pContext->f_logfile==NULL)
			{
				fprintf(stderr,"Error opening logfile [%s]\n",result);
				exit(0);
			}
		}
	}
	return DEFAULT_OK;
	
}
