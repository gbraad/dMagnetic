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
//#include <unistd.h>
#include "picture.h"
#include "glk.h"


// the way i am writing this is:
// callbacks do not yield any immediate results.
//
// the values are simply stored inside the context, and the text/picture is being
// displayed with the input callback is being triggered.
//
#define	MAGIC	0x4b4c4778		// = xGLK
typedef struct _tContext
{
	unsigned int magic;
	winid_t mainwin;
        winid_t statuswin;
        winid_t graphicwin;
	tPicture picture;
	char statustext[256];
	int statusidx;
	char outputwindowtext[(1<<20)];
	int outputidx;
	char lastchar;
	
	char headlineflagged;
	char capital;
} tContext;

int xglk_cbOutputChar(void* context,char c,unsigned char controlD2,unsigned char flag_headline)
{
	tContext *pContext=(tContext*)context;
	unsigned char c2;
	if (pContext->magic!=MAGIC) return -1;

	if (c==0xff)
	{
		pContext->capital=1;
	} else {
		c2=c&0x7f;
		if (c2==9) c2=' ';
		if (flag_headline && (c2==0x5f || c2==0x40)) c2=' ';
		if (controlD2 && c2==0x40) return 0;	// end marker
		if (c2==0x5e || c2==0x7e) c2=0x0a;	// ~ or ^ is actually a line feed.
		if (c2>='a' && c2<='z' && (pContext->capital||flag_headline)) c2&=0x5f;	// upper case
	
		if ((pContext->lastchar=='.' || pContext->lastchar=='!' || pContext->lastchar==':' || pContext->lastchar=='?') && c2>='a' && c2<='z') c2&=0x5f;	// after a '.': captial letter
		if ((pContext->lastchar=='.' || pContext->lastchar=='!' || pContext->lastchar==':' || pContext->lastchar=='?'|| pContext->lastchar==',' || pContext->lastchar==';') && ((c2>='A' && c2<='Z') ||(c2>='a' && c2<='z') ||(c2>='0' && c2<='9'))) 
		{
			if (flag_headline) pContext->statustext[pContext->statusidx++]=' ';
			else pContext->outputwindowtext[pContext->outputidx++]=' ';
		}
	
		if (pContext->lastchar!=' ' || c2!=' ')
		{	
			if (c2==0x0a || (c2>=32 && c2<127 && c2!='@')) 
			{
				if (flag_headline) pContext->statustext[pContext->statusidx++]=c2;
				else pContext->outputwindowtext[pContext->outputidx++]=c2;
			}
		}
		pContext->capital=0;
		pContext->lastchar=c2;
		pContext->statustext[pContext->statusidx]=0;
		pContext->outputwindowtext[pContext->outputidx]=0;	
	}
	return 0;
}
int xglk_cbOutputString(void* context,char* string,unsigned char controlD2,unsigned char flag_headline)
{
	tContext *pContext=(tContext*)context;
	unsigned char c2;
	int i;
	if (pContext->magic!=MAGIC) return -1;
	i=0;
	c2=0;
	do
	{
		c2=string[i++]&0x7f;
		//if (c2==9) c2=' ';
		//if (c2>=32 && c2<127) printf("%c",c2);
		xglk_cbOutputChar(context,c2,controlD2,flag_headline);
	} while (c2>=32);
	return 0;
}
int xglk_cbDrawPicture(void* context,tPicture* picture,int mode)
{
	tContext *pContext=(tContext*)context;
	if (pContext->magic!=MAGIC) return -1;
	memcpy(&pContext->picture,picture,sizeof(tPicture));	
	return 0;
}


int xglk_getsize(int *size)
{
	if (size==NULL) return -1;
	*size=sizeof(tContext);
	return 0;
}
int xglk_open(void* hContext)
{
	tContext *pContext=(tContext*)hContext;
	if (pContext==NULL) return -1;
	memset(pContext,0,sizeof(tContext));
	pContext->magic=MAGIC;

	pContext->mainwin=glk_window_open(0,0,0, wintype_TextBuffer,1);	
	pContext->statuswin=glk_window_open(pContext->mainwin, winmethod_Above | winmethod_Fixed, 1, wintype_TextGrid, 0);
	pContext->graphicwin=glk_window_open (pContext->mainwin,winmethod_Above|winmethod_Proportional,60,wintype_Graphics, 0);
	return 0;
}
int xglk_cbSaveGame(void* context,char* filename,void* ptr,int len)
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

int xglk_cbLoadGame(void* context,char* filename,void* ptr,int len)
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

int xglk_cbInputString(void* context,int* len,char* string)
{
	char commandbuf[256];
	event_t ev;
	int l;
	tContext *pContext=(tContext*)context;
	if (pContext==NULL) return -1;
	memset(commandbuf,0,sizeof(commandbuf));
	do
	{
		if (pContext->statusidx)
		{
			glk_set_window(pContext->statuswin);
			glk_window_clear(pContext->statuswin);
			glk_window_move_cursor(pContext->statuswin,0,0);
			glk_put_string(pContext->statustext);
		
			pContext->statusidx=0;
		}
		if (pContext->outputidx)
		{
			glk_set_window(pContext->mainwin);
			glk_put_string(pContext->outputwindowtext);
			pContext->outputidx=0;
		}
		if (pContext->picture.width && pContext->picture.height)
		{
			int i;
			int x,y;
			int xpix,ypix;
			int pixidx;
			unsigned int palette[16];
			int linecomplete;
			glui32 width,height;
			glk_window_set_background_color(pContext->graphicwin,0x00000000);
			glk_window_clear(pContext->graphicwin);
			glk_window_get_size(pContext->graphicwin, &width, &height);

			for (i=0;i<16;i++)
			{
				palette[i] =(((pContext->picture.palette[i]>>8)&0xf)&0xf)*0x18;palette[i]<<=8;	// Red
				palette[i]|=(((pContext->picture.palette[i]>>4)&0xf)&0xf)*0x18;palette[i]<<=8;	// Green
				palette[i]|=(((pContext->picture.palette[i]>>0)&0xf)&0xf)*0x18;			// Blue
			}
			pixidx=0;
			xpix=1;
			ypix=1;
			if (pContext->picture.width) xpix=width/pContext->picture.width;
			if (pContext->picture.height) ypix=height/pContext->picture.height;
			for (y=0;y<pContext->picture.height;y++)
			{
				for (x=0;x<pContext->picture.width;x++)
				{
					glk_window_fill_rect(pContext->graphicwin,palette[pContext->picture.pixels[pixidx++]],x*xpix,y*ypix,xpix,ypix);
				}
			}

		}
		glk_request_line_event(pContext->mainwin,commandbuf,255,0);
		do
		{
			glk_select(&ev);
		} while (ev.type!=evtype_LineInput);// && ev.type!=evtype_Arrange);
	//	if (ev.type==evtype_Arrange) usleep(1000000);	// wait for the window size to settle.
	} while (ev.type!=evtype_LineInput);
	l=strlen(commandbuf);
	commandbuf[l]=0x0a;	// INPUTS need to be \n terminated.
	*len=l+1;
	memcpy(string,commandbuf,l+2);
	pContext->picture.width=0;
	pContext->picture.height=0;
	pContext->statusidx=0;
	pContext->outputidx=0;
	return 0;	
}

