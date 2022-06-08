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
#include "default_render.h"
#include "picture.h"
int default_render_lowansi(char* allowed,tPicture* picture,int rows,int cols)
{
	unsigned char maxplut[16]={0};	
	// the following table is a bitmap, representing all the available ASCII characters.
	unsigned long long asciibitmap[95]={
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
	const signed char greyrainbow[16][16]=	// 0
	{	
		// basecolor 0	= black/grey
		{ 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
		{ 8, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
		{ 8, 7,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// guild, pic 00
		{ 0, 8, 7,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// guild, pic 14

		{ 0, 0, 8, 7,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// pawn pic 05,pic07
		{ 0, 4, 8, 7,12,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// pawn pic01, pic13
		{ 0, 4, 3, 8, 7,12,15,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// pawn pic09, pic12
		{ 0, 4, 3, 8, 7,12,11,15,-1,-1,-1,-1,-1,-1,-1,-1},

		{ 0, 4, 3, 8, 8, 7,12,11,15,-1,-1,-1,-1,-1,-1,-1},
		{ 0, 4, 3, 8, 8, 7, 7,12,11,15,-1,-1,-1,-1,-1,-1},
		{ 0, 0, 4, 3, 8, 8, 7, 7,12,11,15,-1,-1,-1,-1,-1},
		{ 0, 0, 4, 3, 8, 8, 7, 7, 7,12,11,15,-1,-1,-1,-1},

		{ 0, 0, 4, 3, 8, 8, 7, 7, 7,12,11,15,15,-1,-1,-1},
		{ 0, 0, 4, 3, 8, 8, 8, 7, 7, 7,12,11,15,15,-1,-1},
		{ 0, 0, 4, 3, 8, 8, 8, 7, 7, 7, 7,12,11,15,15,-1},
		{ 0, 0, 4, 3, 8, 8, 8, 8, 7, 7, 7, 7,12,11,15,15}
	};
	const signed char redrainbow[16][16]=
	{
		{ 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
		{ 1, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
		{ 1, 5, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// pawn pic 06
		{ 1, 5, 9,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// pawn pic 13

		{ 8, 1, 3, 9,13,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// pawn pic 15,16
		{ 8, 1, 3, 7, 9,13,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// pawn pic 15
		{ 1, 1, 3,11, 9,13,15,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// pawn pic 07
		{ 0, 8, 3, 1, 5, 7, 9,11, 9,11,15,15,15,15,15,15},	// guild pic 05

		{ 0, 8, 1, 3, 5,13, 9,11,15,-1,-1,-1,-1,-1,-1,-1},	// guild pic 02, myth pic 00
		{ 0, 8, 1, 3, 5,13, 9,11,14,15,15,15,15,15,15,15},	// wonderland. pic36
		{ 0, 1, 1, 1, 5, 5,13, 9, 9,11,15,15,15,15,15,15},	// guild pic 01, pawn pic 03
		{ 0, 1, 1, 1, 5, 5,13, 9, 9,11,15,15,15,15,15,15},

		{ 0, 1, 1, 1, 5, 5,13, 9, 9,11,15,15,15,15,15,15},
		{ 0, 1, 1, 1, 5, 5,13, 9, 9,11,15,15,15,15,15,15},
		{ 0, 1, 1, 1, 5, 5,13, 9, 9,11,15,15,15,15,15,15},
		{ 0, 1, 1, 1, 5, 5,13, 9, 9,11,15,15,15,15,15,15}
	};
	const signed char greenrainbow[16][16]=
	{
		{ 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
		{ 2,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
		{ 2, 6,14,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// guild, pic 00
		{ 2, 6,10,14,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// guild, pic 09, pic 15

		{ 8, 6, 2,10,14,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// pawn pic 14
		{ 2, 2, 6,10,10,14,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// pawn pic 00
		{ 8, 2, 6, 7,10,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// guild pic 06
		{ 8, 2, 3, 6,14,11,10,15,-1,-1,-1,-1,-1,-1,-1,-1},	// wonderland, pic 104

		{ 3, 2, 2, 6, 6,12,14,10,10,-1,-1,-1,-1,-1,-1,-1},
		{ 3, 2, 2, 6, 6,12,14,10,10,15,15,15,15,15,15,15},
		{ 0, 8, 2, 2, 6, 6,10,12,12,14, 7,15,15,15,15,15},	// pic 4, pawn
		{ 3, 2, 2, 6, 6,12,14,10,10,15,15,15,15,15,15,15},

		{ 3, 2, 2, 6, 6,12,14,10,10,15,15,15,15,15,15,15},
		{ 3, 2, 2, 6, 6,12,14,10,10,15,15,15,15,15,15,15},
		{ 3, 2, 2, 6, 6,12,14,10,10,15,15,15,15,15,15,15},
		{ 3, 2, 2, 6, 6,12,14,10,10,15,15,15,15,15,15,15}
	};
	const signed char brownrainbow[16][16]=
	{
		{ 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
		{ 3,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
		{ 3, 7,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// guild, pic 17
		{ 8, 7, 3,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// guild, pic 15

		{ 1, 3, 9,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
		{ 8, 1, 3, 9,11, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// pawn, pic 01, guild pic 08
		{ 8, 1, 3, 3,11, 7,15,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// pawn, pic 06, guild pic 07
		{ 1, 2, 2, 6, 6,14,11,11,-1,-1,-1,-1,-1,-1,-1,-1},

		{ 1, 1, 3, 3, 7,11,11,15,15,-1,-1,-1,-1,-1,-1,-1},	// guild, pic 10
		{ 1, 2, 2, 6, 6,12,14,11,11,15,15,15,15,15,15,15},
		{ 0, 8, 2, 2, 6, 6,10,12,12,14, 7,15,15,15,15,15},
		{ 1, 2, 2, 6, 6,12,14,11,11,15,15,15,15,15,15,15},

		{ 1, 2, 2, 6, 6,12,14,10,10,15,15,15,15,15,15,15},
		{ 1, 2, 2, 6, 6,12,14,10,10,15,15,15,15,15,15,15},
		{ 1, 2, 2, 6, 6,12,14,10,10,15,15,15,15,15,15,15},
		{ 1, 2, 2, 6, 6,12,14,10,10,15,15,15,15,15,15,15}
	};
	const signed char bluerainbow[16][16]=
	{
		{ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
		{ 4,12, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
		{ 4, 6,12, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},	// pawn, pic 12
		{ 4, 6,14,12, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},	// pawn, pic 10

		{ 4, 6, 7,14,12, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},	// jinxter/pix 04
		{ 0, 4, 6, 7,14,12, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},	// jinxter/pic 00, wonderland pic52
		{ 8, 4, 6, 7,14,12,13, 4, 4, 4, 4, 4, 4, 4, 4, 4},	// jinxter/pic 10, wonderland pic46
		{ 8, 4, 6, 7,14,13,12,15, 4, 4, 4, 4, 4, 4, 4, 4},

		{ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
		{ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
		{ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
		{ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},

		{ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
		{ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
		{ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
		{ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4}
	};
	const signed char magentarainbow[16][16]=
	{
		{ 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
		{ 5,13,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
		{ 2, 5,13,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
		{ 5, 5,13,13,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},

		{ 1, 5, 9,12,13,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
		{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
		{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
		{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},

		{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
		{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
		{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
		{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},

		{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
		{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
		{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
		{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5}
	};
	const signed char cyanrainbow[16][16]=
	{
		{ 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
		{ 6,14,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// pawn, pic 09
		{ 6, 7,14,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// guild, pic 17, pic 20
		{ 8, 6, 7,14,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// GUILD, pic 21, guild pic 25

		{ 4, 6,10,12,14,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	// pawn, pic 08
		{ 4, 2, 6,10,12,14, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},	// wonderland, pic 50
		{ 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
		{ 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},

		{ 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
		{ 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
		{ 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
		{ 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},

		{ 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
		{ 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
		{ 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
		{ 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
	};
		

//	const char allowed[]={'M','d','j','7','#'};	// only use those characters to render the picture
	//const char allowed[]={ 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z', 'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','y','u','v','w','x','y','z', '0','1','2','3','4','5','6','7','8','9', '$','(',')','[',']','#','<','>', '&','*','!','{','}','%'};
//,'/','+','\\','{','}','@','!'
	int cnt0;
	int x,y,a,c,i,j,k,p;	// counting variables
	int minccnt,maxc;
	int maxpcnt,maxp;
	int accux,accuy;
	cnt0=0;

	// step one:	// find a good substitute for the palette.	
	// I did a lot of experimentation with this part! and i mean: A LOOOT!
	// the naiive approach would be to take the palette, and search for the nearest neighbour.
	// that did not produce satisfactory results. it took me a while to understand why.
	// the reason is simple:
	// think of a picture with grass. it is mostly green, but there are nuances.
	// a picture of a lake is mostly blue, but there are nuances.
	// 
	// so what i did was to figure out how many colors of the palette are "blue", how many are "red", "green" and a like.
	// this is the "basecolor" of the palette's color.
	// then i figured out how many of those are there, and ordered them according to their intensity.
	// then i defined a "rainbow" of sorts, where i substituted each palette with the ansi color, based on their hierachy within the list.
	//
	// that might result in two original colors that are vastly different being mapped onto the same ansi color. still looks good.
	{
		unsigned int paletteinfo[16];
		int basecolorcnt[7],basecoloridx[7];
		#define	BLACK	0
		#define	RED	1
		#define	GREEN	2
		#define	BROWN	3
		#define	BLUE	4
		#define	MAGENTA	5
		#define	CYAN	6
		#define	UNDEF	7
		for (i=0;i<16;i++) paletteinfo[i]=0;
		for (i=0;i< 7;i++) basecolorcnt[i]=0;
		for (i=0;i< 7;i++) basecoloridx[i]=0;
		// 4 bit primary intensity
		// 4 bit secondary intensity
		// 4 bit secondary color
		// 4 bit basecolor (0=black/grey, 1=red, 2=green, 3=brown, 4=blue, 5=magenta, 6=cyan)
		//12 bit orig rgb
		// 4 bit orig pixel

		// for each color in the palette, find the basecolor. the one with the highest intensity.
		for (i=0;i<16;i++)
		{
			int r;
			int g;
			int b;
			int basecolor;
			int secondcolor;
			int primary;
			int secondary;

			secondcolor=0;
			r=(picture->palette[i]>>8)&0x7;
			g=(picture->palette[i]>>4)&0x7;
			b=(picture->palette[i]>>0)&0x7;

			primary=secondary=0;
			basecolor=0;


			if (r==g && r==b) {basecolor=BLACK;primary=(r+g+b)/3;secondary=UNDEF;}
			if (r>g  && r>b)
			{
				basecolor=RED;primary=r;secondary=(g+b)/2;
				if (g>b) secondcolor=GREEN;
				else if (b>g) secondcolor=BLUE;
				else if (b==g) secondcolor=CYAN;
			}
			if (g>r  && g>b)
			{
				basecolor=GREEN;primary=g;secondary=(r+b)/2;
				if (r>b) secondcolor=RED;
				else if (b>r) secondcolor=BLUE;
				else if (b==r) secondcolor=MAGENTA;

			}
			if (r==g && r>b)  {basecolor=BROWN;primary=(r+g)/2;secondary=b;secondcolor=BLUE;}
			if (b>g  && b>r)
			{
				basecolor=BLUE;primary=b;secondary=(g+r)/2;
				if (r>g) secondcolor=RED;
				else if (g>r) secondcolor=GREEN;
				else if (g==r) secondcolor=BROWN;
			}
			if (r==b && r>g)  {basecolor=MAGENTA;primary=(r+b)/2;secondary=g;secondcolor=GREEN;}
			if (g==b && g>r)  {basecolor=CYAN;primary=(g+b)/2;secondary=r;secondcolor=RED;}

			paletteinfo[i]=i;
			paletteinfo[i]|=(picture->palette[i]<<4);
			paletteinfo[i]|=(basecolor<<16);
			paletteinfo[i]|=(secondcolor<<20);
			paletteinfo[i]|=(secondary<<24);
			paletteinfo[i]|=(primary  <<28);


			basecolorcnt[basecolor]++;      // at the same time, count how many of a base color are in the palette
		}
		// then: sort the list according to the intensity of the base color.
		// since the information has been combined, this is easy.
		// i am waaaay to lazy to implement quick sort, so lets do bubble sort.
		for (i=0;i<16-1;i++)
		{
			unsigned int x;
			for (j=(i+1);j<16;j++)
			{
				if (paletteinfo[i]>paletteinfo[j])
				{
					x=paletteinfo[i];
					paletteinfo[i]=paletteinfo[j];
					paletteinfo[j]=x;
				}
			}
		}

		// now, the list is ordered, with the less intense colors at the beginning
		for (i=0;i<16;i++)
		{
			int basecolor;
			int primary;
			int pixel;

			pixel=(paletteinfo[i]>>0)&0xf;
			basecolor  =(paletteinfo[i]>>16)&0xf;
			primary    =(paletteinfo[i]>>28)&0xf;


			if (basecolorcnt[basecolor]==1) // this base color has only one entry
			{
				if (primary>=4) maxplut[pixel]=basecolor+8;     // high intensity. make it bright.
				else maxplut[pixel]=basecolor;
			}
			else if (basecolorcnt[basecolor]!=0)
			{
				// depending in the base color, pick the substitute color from the rainbow.
				if (basecolor==BLACK)		maxplut[pixel]=greyrainbow[basecolorcnt[basecolor]-1][basecoloridx[basecolor]];
				else if (basecolor==RED)	maxplut[pixel]= redrainbow[basecolorcnt[basecolor]-1][basecoloridx[basecolor]];
				else if (basecolor==GREEN)	maxplut[pixel]= greenrainbow[basecolorcnt[basecolor]-1][basecoloridx[basecolor]];
				else if (basecolor==BROWN)	maxplut[pixel]= brownrainbow[basecolorcnt[basecolor]-1][basecoloridx[basecolor]];
				else if (basecolor==BLUE)	maxplut[pixel]= bluerainbow[basecolorcnt[basecolor]-1][basecoloridx[basecolor]];
				else if (basecolor==MAGENTA)	maxplut[pixel]= magentarainbow[basecolorcnt[basecolor]-1][basecoloridx[basecolor]];
				else if (basecolor==CYAN)	maxplut[pixel]= cyanrainbow[basecolorcnt[basecolor]-1][basecoloridx[basecolor]];
			}
			basecoloridx[basecolor]++;
		}



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

/*
// okay
						row=k/8;
						line=k%8;

						x2=x+4-1*line;
						y2=y+4-1*row;

*/
// okay
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
					printf("\x1b[%d;%dm",maxp/8,30+maxp%8);
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
int default_render_monochrome(char* greyscales,tPicture* picture,int rows,int cols)
{
	int i;
	int j;
	int k,l;

	int y_up,y_down,x_left,x_right;
	int accux,accuy;
	int grey;
	int mingrey,maxgrey;
	int cnt;
	int cnt2;
	int p;
	int scalenum=strlen(greyscales);
	accux=accuy=0;
	mingrey=maxgrey=0;
	// first: try to find the brightest/darkest pixels
	
	y_up=0;
	cnt=0;
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
					cnt2=0;
					for (k=y_up;k<y_down;k++)
					{
						for (l=x_left;l<x_right;l++)
						{
							p=picture->pixels[k*picture->width+l];
							grey+=(picture->palette[p]>>8)&0x7;	// red
							grey+=(picture->palette[p]>>4)&0x7;	// green
							grey+=(picture->palette[p]>>0)&0x7;	// blue
							cnt2++;
						}
					}
					grey/=cnt2;
						
					if (cnt==0 || grey>maxgrey) maxgrey=grey;
					if (cnt==0 || grey<mingrey) mingrey=grey;
					cnt++;
					x_left=x_right;
				}
			}
			y_up=y_down;
		}
	}
	// mingrey is the lowest grey scale value.
	// maxgrey is the highest one.


	y_up=0;
	accuy=0;
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
					cnt2=0;
					for (k=y_up;k<y_down;k++)
					{
						for (l=x_left;l<x_right;l++)
						{
							p=picture->pixels[k*picture->width+l];
							grey+=(picture->palette[p]>>8)&0x7;	// red
							grey+=(picture->palette[p]>>4)&0x7;	// green
							grey+=(picture->palette[p]>>0)&0x7;	// blue
							cnt2++;
						}
					}
					grey/=cnt2;
					grey-=mingrey;
					grey*=(scalenum-1);
					grey/=(maxgrey-mingrey);
					printf("%c",greyscales[grey]);
					x_left=x_right;
				}
			}
			y_up=y_down;
			printf("\n");
		}
	}
	return 0;
}

