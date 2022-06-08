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
#include "default_palette.h"
#include "default_render.h"
#include "picture.h"

// the following table is a bitmap, representing all the available ASCII characters.
const unsigned long long asciibitmap[95]={
	0x0000000000000000, //  
	0x00180018183c3c18, // !
	0x0000000000003636, // "
	0x0036367f367f3636, // #
	0x000c1f301e033e0c, // $
	0x0063660c18336300, // %
	0x006e333b6e1c361c, // &
	0x0000000000030606, // '
	0x00180c0606060c18, // (
	0x00060c1818180c06, // )
	0x0000663cff3c6600, // *
	0x00000c0c3f0c0c00, // +
	0x060c0c0000000000, // ,
	0x000000003f000000, // -
	0x000c0c0000000000, // .
	0x000103060c183060, // /
	0x003e676f7b73633e, // 0
	0x003f0c0c0c0c0e0c, // 1
	0x003f33061c30331e, // 2
	0x001e33301c30331e, // 3
	0x0078307f33363c38, // 4
	0x001e3330301f033f, // 5
	0x001e33331f03061c, // 6
	0x000c0c0c1830333f, // 7
	0x001e33331e33331e, // 8
	0x000e18303e33331e, // 9
	0x000c0c00000c0c00, // :
	0x060c0c00000c0c00, // ;
	0x00180c0603060c18, // <
	0x00003f00003f0000, // =
	0x00060c1830180c06, // >
	0x000c000c1830331e, // ?
	0x001e037b7b7b633e, // @
	0x0033333f33331e0c, // A
	0x003f66663e66663f, // B
	0x003c66030303663c, // C
	0x001f36666666361f, // D
	0x007f46161e16467f, // E
	0x000f06161e16467f, // F
	0x007c66730303663c, // G
	0x003333333f333333, // H
	0x001e0c0c0c0c0c1e, // I
	0x001e333330303078, // J
	0x006766361e366667, // K
	0x007f66460606060f, // L
	0x0063636b7f7f7763, // M
	0x006363737b6f6763, // N
	0x001c36636363361c, // O
	0x000f06063e66663f, // P
	0x00381e3b3333331e, // Q
	0x006766363e66663f, // R
	0x001e33380e07331e, // S
	0x001e0c0c0c0c2d3f, // T
	0x003f333333333333, // U
	0x000c1e3333333333, // V
	0x0063777f6b636363, // W
	0x0063361c1c366363, // X
	0x001e0c0c1e333333, // Y
	0x007f664c1831637f, // Z
	0x001e06060606061e, // [
	0x00406030180c0603, // BACKSLASH
	0x001e18181818181e, // ]
	0x0000000063361c08, // ^
	0xff00000000000000, // _
	0x0000000000180c0c, // `
	0x006e333e301e0000, // a
	0x003b66663e060607, // b
	0x001e3303331e0000, // c
	0x006e33333e303038, // d
	0x001e033f331e0000, // e
	0x000f06060f06361c, // f
	0x1f303e33336e0000, // g
	0x006766666e360607, // h
	0x001e0c0c0c0e000c, // i
	0x1e33333030300030, // j
	0x0067361e36660607, // k
	0x001e0c0c0c0c0c0e, // l
	0x00636b7f7f330000, // m
	0x00333333331f0000, // n
	0x001e3333331e0000, // o
	0x0f063e66663b0000, // p
	0x78303e33336e0000, // q
	0x000f06666e3b0000, // r
	0x001f301e033e0000, // s
	0x00182c0c0c3e0c08, // t
	0x006e333333330000, // u
	0x000c1e3333330000, // v
	0x00367f7f6b630000, // w
	0x0063361c36630000, // x
	0x1f303e3333330000, // y
	0x003f260c193f0000, // z
	0x00380c0c070c0c38, // {
	0x0018181800181818, // |
	0x00070c0c380c0c07, // }
      0x0000000000003b6e // ~
};

int default_render_lowansi(char* allowed,tPicture* picture,int rows,int cols)
{
	unsigned char maxplut[16]={0};	


	int cnt0;
	int x,y,a,c,i,j,k,p;	// counting variables
	int minccnt,maxc;
	int maxpcnt,maxp;
	int accux,accuy;
	cnt0=0;
	// step one:	// find a good substitute for the palette.
	if (picture->pictureType==PICTURE_C64)
	{
		// the c64 had a fixed palette.
	
		maxplut[ 0]=0x0;maxplut[ 1]=0xe;maxplut[ 2]=0x1;maxplut[ 3]=0x6;	// black, white, red, cyan
		maxplut[ 4]=0x5;maxplut[ 5]=0x2;maxplut[ 6]=0x4;maxplut[ 7]=0xb;	// purple, green, blue, yellow
		maxplut[ 8]=0x9;maxplut[ 9]=0x3;maxplut[10]=0xd;maxplut[11]=0x8;	// orange, brown, pink, dark grey
		maxplut[12]=0x7;maxplut[13]=0xa;maxplut[14]=0xc;maxplut[15]=0xf;	// medium grey, light green, light blue, light grey
		
	} else {
		default_palette(picture,maxplut);
	}

	// step 2: render the picture. use the color and the character that best represents a 8x8 block.
	y=0;
	accux=accuy=0;
	for (i=0;i<picture->height;i++)
	{
		accuy+=rows;	// since the output is smaller than the actual picture, count a few lines.
				// at some point, there are enough to print one character.
		if (accuy>=picture->height)
		{
			int lastmaxp;
			lastmaxp=-1;
			accux=0;
			x=0;
			for (j=0;j<picture->width;j++)
			{
				accux+=cols;
				if (accux>=picture->width)
				{	
					unsigned long long pb;
					int scalex,scaley;

					scalex=(j-x)/8;
					scaley=(i-y)/8;
					if (scalex==0) scalex=1;
					if (scaley==0) scaley=1;

					// at this point, a rectangle from X:x..j, Y:y..i is being rendered.
					// first: find the most common characters and the most common color.
					// find within the rectangle the largest amount of pixels with the same color
					maxp=0;
					maxpcnt=0;
					for (p=0;p<16;p++)
					{
						int x2,y2;
						cnt0=0;
						for (x2=x;x2<j;x2++)
						{
							for (y2=y;y2<i;y2++)
							{
								if (x2>=0 && x2<picture->width && y2>=0 && y2<picture->height)
								if (picture->pixels[y2*picture->width+x2]==p) cnt0++;
							}
						}
						if (cnt0>maxpcnt)
						{
							maxpcnt=cnt0;
							maxp=p;
						}
					}
					// create a bitmap of the next 8x8 pixel block
					pb=0;
					for (k=0;k<64;k++)
					{
						int x2,y2;
						int line,row;

						row=k/8;
						line=k%8;

						x2=x+scalex*4-scalex*line;
						y2=y+scaley*4-scaley*row;

						pb<<=1;
						if (x2>=0 && x2<picture->width && y2>=0 && y2<picture->height)
						if (picture->pixels[y2*picture->width+x2]==maxp) pb|=1;

					}

					// compare the bitmap to the bitmap for a character.
					// find the one with the lowest number of mismatches
					minccnt=64;
					maxc=0;
					for (a=0;a<strlen(allowed);a++)
					{
						unsigned long long cb;
						c=allowed[a];	// character
						cb=pb^asciibitmap[c-32];	// cb is the comparision of the bitmap for this character with the one for the 8x8 pixel block.
						cnt0=0;
						for (k=0;k<64;k++)	// count the differences.
						{
							cnt0+=(cb&1);
							cb>>=1;
						}
						if ((cnt0)<=minccnt)
						{
							minccnt=cnt0;
							maxc=c;
						}
					}
	
					maxp=maxplut[maxp];
					if (maxp!=lastmaxp) printf("\x1b[%d;%dm",maxp/8,30+maxp%8);
					lastmaxp=maxp;
					if (maxc<32 || maxc>=127) printf(" ");
					else printf("%c",maxc);
					accux-=picture->width;
					x=j;	// the next rectangle will begin at the edge of this one.
				}
			}
			accuy-=picture->height;
			y=i;	// the next rectangle will begin in this line
			printf("\x1b[0m\n");
		}
	}
	return 0;
}
int default_render_lowansi2(char* allowed,tPicture* picture,int rows,int cols)
{
	int cnt0;
	int x,y,a,c,i,j,k,p;	// counting variables
	int minccnt,maxc;
	int maxpcnt,maxp;
	int accux,accuy;
	int redsum,greensum,bluesum,pixcnt;
	int lastx,lasty;
	cnt0=0;
	// render the picture. use the color and the character that best represents a 8x8 block.

	y=0;
	accux=accuy=0;
	lasty=0;
	for (i=0;i<picture->height;i++)
	{
		accuy+=rows;	// since the output is smaller than the actual picture, count a few lines.
				// at some point, there are enough to print one character.
		if (accuy>=picture->height)
		{
			int lastmaxp;
			lastmaxp=-1;
			accux=0;
			x=0;
			lastx=0;
			for (j=0;j<picture->width;j++)
			{
				accux+=cols;
				if (accux>=picture->width)
				{	
					unsigned long long pb;
					int scalex,scaley;
					int l;
					int maxp2;
					int mul;

					redsum=greensum=bluesum=pixcnt=0;
					for (k=lastx;k<=j;k++)
					{
						for (l=lasty;l<=i;l++)
						{
							unsigned int p;
							p=picture->palette[(int)picture->pixels[k+l*picture->width]];
							redsum  +=(PICTURE_GET_RED(p));
							greensum+=(PICTURE_GET_GREEN(p));
							bluesum +=(PICTURE_GET_BLUE(p));
							pixcnt++;
						}
					}
					
					lastx=j;

					maxp2=default_findrgbcluster(redsum/pixcnt,greensum/pixcnt,bluesum/pixcnt);

					

					if (picture->pictureType==PICTURE_HALFTONE) mul=2;
					else mul=1;
					scalex=(j-x)/(8*mul);
					scaley=(i-y)/(8*mul);
					if (scalex==0) scalex=1;
					if (scaley==0) scaley=1;

					// at this point, a rectangle from X:x..j, Y:y..i is being rendered.
					// first: find the most common characters and the most common color.
					// find within the rectangle the largest amount of pixels with the same color
					maxp=0;
					maxpcnt=0;
					for (p=0;p<16;p++)
					{
						int x2,y2;
						cnt0=0;
						for (x2=x;x2<j;x2+=mul)
						{
							for (y2=y;y2<i;y2+=mul)
							{
								if (x2>=0 && x2<picture->width && y2>=0 && y2<picture->height)
								if (picture->pixels[y2*picture->width+x2]==p) cnt0++;
							}
						}
						if (cnt0>maxpcnt)
						{
							maxpcnt=cnt0;
							maxp=p;
						}
					}
					// create a bitmap of the next 8x8 pixel block
					pb=0;
					for (k=0;k<64;k++)
					{
						int x2,y2;
						int line,row;

						row=k/(8*mul);
						line=k%(8*mul);

						x2=x+scalex*(4*mul)-scalex*line;
						y2=y+scaley*(1*mul)-scaley*row;

						pb<<=1;
						if (x2>=0 && x2<picture->width && y2>=0 && y2<picture->height)
						if (picture->pixels[y2*picture->width+x2]==maxp) pb|=1;

					}

					// compare the bitmap to the bitmap for a character.
					// find the one with the lowest number of mismatches
					minccnt=64;
					maxc=0;
					for (a=0;a<strlen(allowed);a++)
					{
						unsigned long long cb;
						c=allowed[a];	// character
						cb=pb^asciibitmap[c-32];	// cb is the comparision of the bitmap for this character with the one for the 8x8 pixel block.
						cnt0=0;
						for (k=0;k<64;k++)	// count the differences.
						{
							cnt0+=(cb&1);
							cb>>=1;
						}
						if ((cnt0)<=minccnt)
						{
							minccnt=cnt0;
							maxc=c;
						}
					}
	
					if (maxp2!=lastmaxp) printf("\x1b[%d;%dm",maxp2/8,30+maxp2%8);
					lastmaxp=maxp2;
					if (maxc<32 || maxc>=127) printf(" ");
					else printf("%c",maxc);
					accux-=picture->width;
					x=j;	// the next rectangle will begin at the edge of this one.
				}
			}
			lasty=i;
			accuy-=picture->height;
			y=i;	// the next rectangle will begin in this line
			printf("\x1b[0m\n");
		}
	}
	return 0;
}
int default_render_monochrome(char* greyscales,int inverted,tPicture* picture,int rows,int cols)
{
	int i;
	int j;
	int k,l;

	int y_up,y_down,x_left,x_right;
	int accux,accuy;
	int grey;
	int mingrey,maxgrey;
	int cnt;
	int pixcnt;
	int p;
	int scalenum=strlen(greyscales);
	int pass;
	// first: try to find the brightest/darkest pixels
	// second pass: the position of the character within the monochrome ramp should be proportional to the brightness of a "pixel".

	cnt=0;
	mingrey=maxgrey=0;
	for (pass=0;pass<2;pass++)
	{	
		if (pass==1 && mingrey==maxgrey)
		{
			maxgrey++;
		}
		y_up=0;
		accux=accuy=0;
		for (i=0;i<picture->height;i++)
		{
			accuy+=rows;
			if (accuy>=picture->height || i==picture->height-1)
			{
				accuy-=picture->height;
				y_down=i+1;
				x_left=0;
				for (j=0;j<picture->width;j++)
				{
					accux+=cols;
					if (accux>=picture->width || j==picture->width-1)
					{
						x_right=j+1;
						accux-=picture->width;		
						// at this point, a rectangle between y_up,y_down, x_left,x_right contains the pixels that need to be greyscaled.
						grey=0;
						pixcnt=0;
						for (k=y_up;k<y_down;k++)
						{
							for (l=x_left;l<x_right;l++)
							{
								p=picture->pixels[k*picture->width+l];
								// compensate for luminance (0.2126*R + 0.7152*G + 0.0722*B)
								grey+=2126*(PICTURE_GET_RED(picture->palette[p]));
								grey+=7152*(PICTURE_GET_GREEN(picture->palette[p]));
								grey+= 722*(PICTURE_GET_BLUE(picture->palette[p]));

								pixcnt++;
							}
						}
						grey/=pixcnt;
						if (pass==0)
						{
							// first pass: find the dimmest and the brightest "pixels"
							if (cnt==0 || grey>maxgrey) maxgrey=grey;
							if (cnt==0 || grey<mingrey) mingrey=grey;
							cnt++;
						} else {
							// second pass: render the "pixel" with the appropriate character
							grey-=mingrey;
							grey*=(scalenum-1);
							grey/=(maxgrey-mingrey);
							if (inverted) grey=scalenum-1-grey;
							printf("%c",greyscales[grey]);

						}
						x_left=x_right;
					}
				}
				y_up=y_down;
				if (pass!=0) printf("\n");
			}
		}
	}

	return 0;
}

