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

// the purpose of this file is to provide a callback-fallback until proper
// user interfaces have been established. It will do ;)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "default_render.h"
#include "default_palette.h"
#include "picture.h"
#include "configuration.h"
#include "default_callbacks.h"

#define	MAGIC	0x68654879	// = yHeh, the place where I grew up ;)
#define	MAXTEXTBUFFER	1024	// maximum number of buffered characters
#define	MAXHEADLINEBUFFER	256	// maximum number of buffered headline characters
#define	MAX_SRESX		4096
typedef enum _tMode
{
	eMODE_NONE,
	eMODE_MONOCHROME,
	eMODE_LOW_ANSI,
	eMODE_LOW_ANSI2,
	eMODE_HIGH_ANSI,
	eMODE_HIGH_ANSI2,
	eMODE_SIXEL,
	eMODE_UTF
} tMode;
typedef	struct _tContext
{
	unsigned int magic;
	// ansi output
	int	columns;
	int	rows;
	tMode	mode;	// 0=none. 1=ascii art. 2=ansi art. 3=high ansi. 4=high ansi2
	// log the input
	FILE* f_logfile;
	int	echomode;

	int	capital;	// =1 if the next character should be a capital letter.
	int	lastchar;	
	int	jinxterslide;	// THIS workaround makes the sliding puzzle in Jinxter solvable
	int headlineflagged;
	char	low_ansi_characters[128];	// characters that are allowed for low ansi rendering
	char	monochrome_characters[128];	// characters that are allowed for monochrome rendering
	int	monochrome_inverted;

// this is the line buffer.
// text is stored here, before
// being printed on the console.
	int	textalign;	// 0=left aligned. 1=block aligned
	int	textlastspace;
	int	textidx;
	char	textoutput[MAXTEXTBUFFER];
	int	headlineidx;
	char	headlineoutput[MAXHEADLINEBUFFER];

// sixel parameter
	int 	screenheight;
	int	screenwidth;
	int	forceres;
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
		pContext->headlineidx=0;
	}
	if (!flag_headline && pContext->headlineflagged) 	// after the headline ends, a new paragraph is beginning.
	{
		int i;
		pContext->capital=1;	// obviously, this starts with a captial letter.
		pContext->headlineoutput[pContext->headlineidx]=0;
		// highlight the headline
		//printf("\x1b[0;30;47m%s\x1b[0m\n",pContext->headlineoutput);


		// first: remove the newlines from the headline text.
		for (i=0;i<pContext->headlineidx;i++)
		{
			if (pContext->headlineoutput[i]==0x0a) 
			{
				pContext->headlineoutput[i]=0;
				pContext->headlineidx--;
			}
		}

		// second, print the ----[HEADLINE]-
		if (pContext->mode==eMODE_NONE)
		{
			printf("\n[%s]\n",pContext->headlineoutput);
		} else {
			for (i=0;i<pContext->columns-pContext->headlineidx-3;i++)
			{
				printf("-");
			}
			if (pContext->mode==eMODE_LOW_ANSI || pContext->mode==eMODE_HIGH_ANSI)	// high or low ansi -> make the headline text pop out
				printf("[\x1b[0;30;47m%s\x1b[0m]-\n",pContext->headlineoutput);
			else 
				printf("[%s]-\n",pContext->headlineoutput);
		}
			

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
		// the characters ~ and ^ are to be translated into line feeds.
		// the caracter 0xff makes the next one upper case.
		// after a second newline comes a capital letter.

		if (c2==9 || c2=='_') c2=' ';
		if (flag_headline && (c2==0x5f || c2==0x40)) c2=' ';	// in a headline, those are the control codes for a space.
		if (controlD2 && c2==0x40) return 0;	// end marker
		if (c2==0x5e || c2==0x7e) c2=0x0a;	// ~ or ^ is actually a line feed.
		if (c2==0x0a && pContext->lastchar==0x0a) 	// after two consequitive newlines comes a capital letter.
		{
			pContext->capital=1;
		}
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
				if (pContext->headlineidx<MAXHEADLINEBUFFER-1)
					pContext->headlineoutput[pContext->headlineidx++]=' '; // after those letters comes an extra space.
			}
			else {
				if (pContext->textidx<MAXTEXTBUFFER-1)
				{
					pContext->textlastspace=pContext->textidx;
					pContext->textoutput[pContext->textidx++]=' ';
				}
			}
			//pContext->lastchar=' ';
		}
		if (pContext->textidx>0 && pContext->lastchar==' ' && (c2==',' || c2==';' || c2=='.' || c2=='!'))	// there have been some glitches with extra spaces, right before a komma. which , as you can see , looks weird.
		{
			pContext->textidx--;	
		}
		if (	//allow multiple spaces in certain scenarios
			flag_headline ||
			pContext->jinxterslide ||
			pContext->lastchar!=' ' || c2!=' ')	// combine multiple spaces into a single one.
		{	
			if (c2==0x0a || (c2>=32 && c2<127 && c2!='@')) 
			{
				if (flag_headline) 
				{
					if (pContext->headlineidx<MAXHEADLINEBUFFER-1)
					pContext->headlineoutput[pContext->headlineidx++]=c2&0x7f;
				} else if (pContext->textidx<MAXTEXTBUFFER-1) {
					if (c2==':' || c2=='-' || (c2>='0'  && c2<='9')) pContext->jinxterslide=1;		// a workaround regarding the sliding puzzle in jinxter.
					else if (c2!=' ') pContext->jinxterslide=0;				// sometimes multiple spaces are bad, but in this case it is not.
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
		default_cbOutputChar(context,c2,controlD2,flag_headline);
	} while (c2>=32);
	return 0;
}
int default_cbInputString(void* context,int* len,char* string)
{
	int l;
	tContext *pContext=(tContext*)context;
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

	if (pContext->mode==eMODE_NONE) return 0;
	// flush the output buffer
	default_cbOutputChar(context,'\n',0,0);
	if (pContext->mode==eMODE_MONOCHROME)
	{
		default_render_monochrome(pContext->monochrome_characters,pContext->monochrome_inverted,picture,pContext->rows,pContext->columns);	
	}

	if (pContext->mode==eMODE_LOW_ANSI)
	{
		default_render_lowansi(pContext->low_ansi_characters,picture,pContext->rows,pContext->columns);	
	}

	if (pContext->mode==eMODE_LOW_ANSI2)
	{
		default_render_lowansi2(pContext->low_ansi_characters,picture,pContext->rows,pContext->columns);	
	}

	if (pContext->mode==eMODE_HIGH_ANSI2)
	{
		int redsum,greensum,bluesum;
		int lastx;
		int lasty;
		int pixcnt;

		redsum=0;
		greensum=0;
		bluesum=0;

		lastx=lasty=0;
		pixcnt=0;



		accux=accuy=0; 

		for (i=0;i<picture->height;i++)
		{
			accuy+=pContext->rows;
			lastrgb=-1;
			if (accuy>=picture->height || i==picture->height-1)
			{
				accuy-=picture->height;
				accux=0;
				lastx=0;
				for (j=0;j<picture->width;j++)
				{
					accux+=pContext->columns;
					if (accux>=picture->width || j==picture->width-1)
					{
						int x,y;
						redsum=0;
						greensum=0;
						bluesum=0;
						accux-=picture->width;
						pixcnt=0;
						for (y=lasty;y<=i;y++)
						{
							for (x=lastx;x<=j;x++)
							{
								unsigned int p;
								p=picture->palette[(int)(picture->pixels[y*(picture->width)+x])];
								redsum  +=(PICTURE_GET_RED(p));
								greensum+=(PICTURE_GET_GREEN(p));
								bluesum +=(PICTURE_GET_BLUE(p));
								pixcnt++;
							}
						}
						redsum*=255;greensum*=255;bluesum*=255;
						pixcnt*=PICTURE_MAX_RGB_VALUE;
						redsum/=pixcnt;greensum/=pixcnt;bluesum/=pixcnt;
						if (redsum>255) redsum=255;
						if (greensum>255) greensum=255;
						if (bluesum>255) bluesum=255;
						rgb=(redsum<<16)|(greensum<<8)|bluesum;
						if (rgb!=lastrgb)
						{
							printf("\x1b[48;2;%d;%d;%dm",
									redsum,greensum,bluesum);
							lastrgb=rgb;
						}
						printf(" ");
						lastx=j;
					}
				}
				printf("\x1b[0m\n");
				lasty=i;
			}
		}
	}
	if (pContext->mode==eMODE_HIGH_ANSI)
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
							int red,green,blue;
							red  =PICTURE_GET_RED(rgb);
							green=PICTURE_GET_GREEN(rgb);
							blue =PICTURE_GET_BLUE(rgb);
							red*=255;green*=255;blue*=255;
							red  /=PICTURE_MAX_RGB_VALUE;
							green/=PICTURE_MAX_RGB_VALUE;
							blue /=PICTURE_MAX_RGB_VALUE;
							printf("\x1b[48;2;%d;%d;%dm",
									red,green,blue);
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
	if (pContext->mode==eMODE_SIXEL) 	// sixels
	{		
		int i,j;
		int accux,accuy;
		int x,y;
		int screenheight;
		int screenwidth;
		int forceres;
		int minval;
		int minpos;
		int maxval;
		int maxpos;
		int paletteorder[16];
		x=0;y=0;
		accux=accuy=0;
		screenheight=pContext->screenheight;
		screenwidth=pContext->screenwidth;
		forceres=pContext->forceres;

		// find a good aspect ratio
		{
			int ratiox,ratioy;
			#define	MAGICFIXPOINT	1800	// magic factor was chosen when I removed the float code. simply because it worked with the check and it did not cause overflows with 32 bit machines

			ratiox=(MAGICFIXPOINT*screenwidth)/(picture->width);
			ratioy=(MAGICFIXPOINT*screenheight)/(picture->height-1);

			if (!forceres)
			{
				if (ratiox<ratioy) ratioy=ratiox; else ratiox=ratioy;
			}

			screenwidth =(int)((ratiox*picture->width)/MAGICFIXPOINT);
			screenheight=(int)((ratioy*(picture->height-1))/MAGICFIXPOINT);
			if (!forceres)
			{
				while (screenheight%6) screenheight++;	// make sure that the last line consists of a full block of sixels. 
			}
		}
		// find the darkest and the brightest color. 
		// move them to pallette position 0 or 15, to deal with an issue that james found
		{
			for (i=0;i<16;i++)
			{
				unsigned int red,green,blue;
				unsigned int rgb;
				int val;
				paletteorder[i]=i;
				rgb=picture->palette[i];
				red  =PICTURE_GET_RED(rgb);
				green=PICTURE_GET_GREEN(rgb);
				blue =PICTURE_GET_BLUE(rgb);

				val=red+green+blue;	// just add the values. luminance correction would be overkill
				if (i==0 || minval>val)
				{
					minval=val;
					minpos=i;
				}	

				if (i==0 || maxval<val)
				{
					maxval=val;
					maxpos=i;
				}	
			}
			// according to james, the background colour will be changed in some terminals
			// as well as the default foreground value. presumably, because they only had
			// memory for 16 rgb values. on those, the default foreground is being over-
			// written with the sixth colour being defined. and the default background,
			// when the 15th colour comes. ("a last ressort", as james put it)

			paletteorder[minpos]^=paletteorder[15];paletteorder[15]^=paletteorder[minpos];paletteorder[minpos]^=paletteorder[15];	// the darkest colour is now the background
			paletteorder[maxpos]^=paletteorder[ 6];paletteorder[ 6]^=paletteorder[maxpos];paletteorder[maxpos]^=paletteorder[ 6];	// the brightest one becomes the foreground
		}

		printf("\n\x1bP9;1q\"1;1;%d;%d", screenwidth, screenheight);
		for (i=0;i<16;i++)
		{
			unsigned int red,green,blue;
			unsigned int rgb;

			rgb=picture->palette[paletteorder[i]];
			red  =PICTURE_GET_RED(rgb);
			green=PICTURE_GET_GREEN(rgb);
			blue =PICTURE_GET_BLUE(rgb);
			red*=100;green*=100;blue*=100;
			red  /=PICTURE_MAX_RGB_VALUE;
			green/=PICTURE_MAX_RGB_VALUE;
			blue /=PICTURE_MAX_RGB_VALUE;

			printf("#%02d;2;%d;%d;%d",paletteorder[i],red,green,blue);
		}


		while (y<(picture->height-1))
		{
			int y0;
			int accuy0;
			int curpixel;
			accuy0=accuy;
			y0=y;
			for (i=0;i<16;i++)
			{
				printf("#%02d",i);
				x=0;
				accux=0;
				while (x<picture->width)
				{
					char bitmask;
					y=y0;
					accuy=accuy0;
					curpixel=picture->pixels[y*picture->width+x];
					bitmask=0;
					for (j=0;j<6;j++)
					{
						bitmask>>=1;
						if (curpixel==i) bitmask|=0x20;
						accuy+=(picture->height-1);
						while (accuy>=screenheight)
						{
							accuy-=screenheight;
							y++;
							if (y<(picture->height-1)) 
							{
								curpixel=picture->pixels[y*picture->width+x];
							} else {
								curpixel=0;
							}
						}
					}
					while (accux<screenwidth)
					{
						printf("%c",'?'+bitmask);
						accux+=picture->width;
					}
					accux-=screenwidth;
					x++;
				}
				printf("$");
			}
			printf("-");
		}
		printf("\x1b\\\x1b[0m\n");
	}
	if (pContext->mode==eMODE_UTF) 	// utf
	{
	// By choosing the following utf-characters:
        // 0x20,0xe29680,0xe29690,0xe2969a,0xe29696,0xe29697,0xe29698,0xe2969d
	//
	// i could transform each character field into a 2x2 bitmap:
        //  ..     ##        .#        #.     ..        ..     #.       .#
        //  ..     ..        .#        .#     #.        .#     ..       ..
	// 
	// when choosing the background and the foreground colour wisely, it
	// would create the illusion of finer grained pixels, even though,
	// technically, it is not exact. It would work best when each 2x2 bitmap
	// has only 1 or 2 colours. But with 3 or 4 it creates artifacts. 
	// they are negligable!
	// 
	// it can easily be seen that those 8 utf symbols are enough


#define	NUM_utf8chars	8
#define	CALC_RGBDELTA(rgb1,rgb2)	( \
		(PICTURE_GET_RED(rgb1)-PICTURE_GET_RED(rgb2))*(PICTURE_GET_RED(rgb1)-PICTURE_GET_RED(rgb2))+	\
		(PICTURE_GET_GREEN(rgb1)-PICTURE_GET_GREEN(rgb2))*(PICTURE_GET_GREEN(rgb1)-PICTURE_GET_GREEN(rgb2))+	\
		(PICTURE_GET_BLUE(rgb1)-PICTURE_GET_BLUE(rgb2))*(PICTURE_GET_BLUE(rgb1)-PICTURE_GET_BLUE(rgb2)))
#define	NUM_COLOURS	16
	// for each of the utf characters, define a bitmap according to the following scheme:
        //  12
        //  48
		const unsigned int default_cbDrawPicture_utf8chars[NUM_utf8chars]={0x200000,0xe29680,0xe29690,0xe2969a,0xe29696,0xe29697,0xe29698,0xe2969d};
	        const unsigned char default_cbDrawPicture_utf8map[NUM_utf8chars] ={0x0,     1|2,     2|8,     1|8,     4,       8,       1,       2};

		int x;
		int y;
		int accux;
		int accuy;

		accuy=0;
		for (y=0;y<picture->height;y++)
		{
			accuy+=pContext->rows;
			if (accuy>=picture->height)
			{
				accuy-=picture->height;
				accux=0;	
				for (x=0;x<picture->width;x++)
				{
					unsigned int lastfg,lastbg;
					lastfg=lastbg=0xffffffff;
					accux+=pContext->columns;
					if (accux>=picture->width)
					{
						unsigned int rgb1,rgb2,rgb4,rgb8;
						unsigned int mindelta;
						unsigned int bestutfsymbol;
						unsigned int bestfg;
						unsigned int bestbg;
						int fg,bg,sym;
						int fgred,fggreen,fgblue;
						int bgred,bggreen,bgblue;
						accux-=picture->width;
						// find out what colours are in the 2x2 bitmap
						rgb1=rgb2=rgb4=rgb8=picture->palette[(int)(picture->pixels[(y+0)*(picture->width)+x+0])];
						if (x<picture->width-1) rgb2=picture->palette[(int)(picture->pixels[(y+0)*(picture->width)+x+1])];
						if (y<picture->height-1) rgb4=rgb8=picture->palette[(int)(picture->pixels[(y+1)*(picture->width)+x+0])];
						if (x<picture->width-1 && y<picture->height-1) rgb8=picture->palette[(int)(picture->pixels[(y+1)*(picture->width)+x+1])];

						// now for the main event: try to find a matching foreground/background/symbol combination
						// using a neighest neighbour-approach with the RGB values helps in nuanced picture to 
						// mask some artifacts
						mindelta=bestfg=bestbg=0xffffffff;
						bestutfsymbol=0;
						for (fg=0;fg<NUM_COLOURS;fg++)
						{
							for (bg=0;bg<NUM_COLOURS;bg++)
							{
								for (sym=0;sym<NUM_utf8chars;sym++)
								{
									unsigned int delta;
									delta =CALC_RGBDELTA(rgb1,(default_cbDrawPicture_utf8map[sym]&1)?picture->palette[fg]:picture->palette[bg]);
									delta+=CALC_RGBDELTA(rgb2,(default_cbDrawPicture_utf8map[sym]&2)?picture->palette[fg]:picture->palette[bg]);
									delta+=CALC_RGBDELTA(rgb4,(default_cbDrawPicture_utf8map[sym]&4)?picture->palette[fg]:picture->palette[bg]);
									delta+=CALC_RGBDELTA(rgb8,(default_cbDrawPicture_utf8map[sym]&8)?picture->palette[fg]:picture->palette[bg]);

									if (delta<mindelta || mindelta==0xffffffff)
									{
										mindelta=delta;
										bestfg=picture->palette[fg];
										bestbg=picture->palette[bg];
										bestutfsymbol=default_cbDrawPicture_utf8chars[sym];
									}
								}
							}
						}
						// now the best fg/bg/sym combination is known. let's render it!
						// first: the colour
						if (lastfg!=bestfg)
						{
							lastfg=bestfg;
							fgred  =PICTURE_GET_RED(bestfg);
							fggreen=PICTURE_GET_GREEN(bestfg);
							fgblue =PICTURE_GET_BLUE(bestfg);
							fgred*=255;fggreen*=255;fgblue*=255;
							fgred  /=PICTURE_MAX_RGB_VALUE;
							fggreen/=PICTURE_MAX_RGB_VALUE;
							fgblue /=PICTURE_MAX_RGB_VALUE;
							printf("\x1b[38;2;%d;%d;%dm",fgred,fggreen,fgblue);
						}

						if (lastbg!=bestbg)
						{
							lastbg=bestbg;
							bgred  =PICTURE_GET_RED(bestbg);
							bggreen=PICTURE_GET_GREEN(bestbg);
							bgblue =PICTURE_GET_BLUE(bestbg);
							bgred*=255;bggreen*=255;bgblue*=255;
							bgred  /=PICTURE_MAX_RGB_VALUE;
							bggreen/=PICTURE_MAX_RGB_VALUE;
							bgblue /=PICTURE_MAX_RGB_VALUE;
							printf("\x1b[48;2;%d;%d;%dm",bgred,bggreen,bgblue);
						}


						// since the uft8symbols above are defined as BIG endian numbers, and do not have a 0x00 at the end,
						// they can be displayed one byte at a time. MSB first.
						while (bestutfsymbol)
						{
							printf("%c",(bestutfsymbol>>16)&0xff);
							bestutfsymbol<<=8;
							bestutfsymbol&=0x00ffff00;
						}
					}
				}
				// that was one line
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
	const char default_monochrome_characters[DEFAULT_MONOCHROME_CHARACTERS]=" .-=*";

	if (pContext==NULL) return DEFAULT_NOK;
	memset(pContext,0,sizeof(tContext));
	pContext->magic=MAGIC;

	// the hiearchy is: first the default values. 
	// if there is a .ini file, overwrite them with those values.
	// paramaters from the command line have the highest prioprity.

	pContext->rows=40;
	pContext->columns=120;
	pContext->mode=eMODE_LOW_ANSI;	// 0=none. 1=monochrome. 2=low_ansi. 3=high_ansi. 4=high_ansi2, 5=sixel
	pContext->f_logfile=NULL;
	pContext->echomode=0;
	pContext->textalign=1;
	pContext->textidx=0;
	pContext->textlastspace=-1;

	pContext->screenwidth=320;
	pContext->screenheight=200;
	pContext->forceres=0;

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
			if (strncmp(result,"none",4)==0) pContext->mode=eMODE_NONE;	
			else if (strncmp(result,"monochrome_inv",14)==0) {pContext->mode=eMODE_MONOCHROME;pContext->monochrome_inverted=1;}
			else if (strncmp(result,"monochrome",10)==0) pContext->mode=eMODE_MONOCHROME;
			else if (strncmp(result,"low_ansi2",9)==0) pContext->mode=eMODE_LOW_ANSI2;
			else if (strncmp(result,"low_ansi",8)==0) pContext->mode=eMODE_LOW_ANSI;
			else if (strncmp(result,"high_ansi2",10)==0) pContext->mode=eMODE_HIGH_ANSI2;
			else if (strncmp(result,"high_ansi",9)==0) pContext->mode=eMODE_HIGH_ANSI;
			else if (strncmp(result,"sixel",5)==0) pContext->mode=eMODE_SIXEL;
			else if (strncmp(result,"utf",4)==0) pContext->mode=eMODE_UTF;
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
		if (retrievefromini(f_inifile,"[DEFAULTGUI]","sixel_forceresolution",result,sizeof(result)))
		{
			if (result[0]=='y' || result[0]=='Y' || result[0]=='t' || result[0]=='T' || result[0]=='1') pContext->forceres=1;	// set it to one if the entry reads "yes"/"True"/1
		}
		if (retrievefromini(f_inifile,"[DEFAULTGUI]","sixel_resolution",result,sizeof(result)))
		{
			int i;
			int l;
			l=strlen(result);
			pContext->screenwidth=pContext->screenheight=0;
			for (i=0;i<l;i++)
			{
				if (result[i]=='x')
				{
					result[i]=0;
					pContext->screenwidth =atoi(&result[0]);
					pContext->screenheight=atoi(&result[i+1]);
				}
			}
			if (pContext->screenwidth==0 || pContext->screenwidth>MAX_SRESX || pContext->screenheight==0) 
			{
				printf("illegal parameter for sixelresultion. please use something like 1024x768\n");
				return DEFAULT_NOK;
			}

		}
	}

	if (argc)
	{
		char result[64];
		if (retrievefromcommandline(argc,argv,"-valign",result,sizeof(result)))
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
		if (retrievefromcommandline(argc,argv,"-vmode",result,sizeof(result)))
		{
			if (strncmp(result,"none",4)==0) pContext->mode=eMODE_NONE;	
			else if (strncmp(result,"monochrome_inv",14)==0) {pContext->mode=eMODE_MONOCHROME;pContext->monochrome_inverted=1;}
			else if (strncmp(result,"monochrome",10)==0) pContext->mode=eMODE_MONOCHROME;
			else if (strncmp(result,"low_ansi2",9)==0) pContext->mode=eMODE_LOW_ANSI2;
			else if (strncmp(result,"low_ansi",8)==0) pContext->mode=eMODE_LOW_ANSI;
			else if (strncmp(result,"high_ansi2",10)==0) pContext->mode=eMODE_HIGH_ANSI2;
			else if (strncmp(result,"high_ansi",9)==0) pContext->mode=eMODE_HIGH_ANSI;
			else if (strncmp(result,"sixel",5)==0) pContext->mode=eMODE_SIXEL;
			else if (strncmp(result,"utf",4)==0) pContext->mode=eMODE_UTF;
			else {
				printf("unknown parameter for -vmode. please use one of\n");
				printf("none ");
				printf("monochrome ");
				printf("monochrome_inv ");
				printf("low_ansi ");
				printf("low_ansi2 ");
				printf("high_ansi ");
				printf("high_ansi2 ");
				printf("sixel ");
				printf("utf ");
				printf("\n");	
				return DEFAULT_NOK;
			}	
		}
		if (retrievefromcommandline(argc,argv,"-vrows",result,sizeof(result)))
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
		if (retrievefromcommandline(argc,argv,"-vcols",result,sizeof(result)))
		{
			int cols;
			cols=atoi(result);
			if (cols<1 || cols>600) 
			{
				printf("illegal parameter for -vcols. please use values between 1 and 600\n");
				return DEFAULT_NOK;
			}
			pContext->columns=cols;
		}
		if (retrievefromcommandline(argc,argv,"-vecho",NULL,0))
		{
			pContext->echomode=1;
		}
		if (retrievefromcommandline(argc,argv,"-vlog",result,sizeof(result)))
		{
			fprintf(stderr,"Opening logfile [%s] for writing\n",result);
			pContext->f_logfile=fopen(result,"wb");	
			if (pContext->f_logfile==NULL)
			{
				fprintf(stderr,"Error opening logfile [%s]\n",result);
				exit(0);
			}
		}
		if (retrievefromcommandline(argc,argv,"-sres",result,sizeof(result)))
		{
			int i;
			int l;
			l=strlen(result);
			pContext->screenwidth=pContext->screenheight=0;
			for (i=0;i<l;i++)
			{
				if (result[i]=='x')
				{
					result[i]=0;
					pContext->screenwidth =atoi(&result[0]);
					pContext->screenheight=atoi(&result[i+1]);
				}
			}
			if (pContext->screenwidth==0 || pContext->screenwidth>MAX_SRESX || pContext->screenheight==0) 
			{
				printf("illegal parameter for -sres. please use something like 1024x768\n");
				return DEFAULT_NOK;
			}
		}
		if (retrievefromcommandline(argc,argv,"-sforce",result,sizeof(result)))
		{
			pContext->forceres=1;
		}
	}
	return DEFAULT_OK;
	
}
