/*

Copyright 2022, dettus@dettus.net

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

#ifndef	PICTURE_H
#define	PICTURE_H

#define	PICTURE_BITS_PER_RGB_CHANNEL	10
#define	PICTURE_MAX_RGB_VALUE		((1<<PICTURE_BITS_PER_RGB_CHANNEL)-1)
#define	PICTURE_GET_RED(p)		(((p)>>(2*PICTURE_BITS_PER_RGB_CHANNEL))&PICTURE_MAX_RGB_VALUE)
#define	PICTURE_GET_GREEN(p)		(((p)>>(1*PICTURE_BITS_PER_RGB_CHANNEL))&PICTURE_MAX_RGB_VALUE)
#define	PICTURE_GET_BLUE(p)		(((p)>>(0*PICTURE_BITS_PER_RGB_CHANNEL))&PICTURE_MAX_RGB_VALUE)

typedef	enum _ePictureType
{
	PICTURE_DEFAULT,
	PICTURE_HALFTONE,
	PICTURE_C64
} ePictureType;
// the purpose of this file is to provide the data structure for the pictures.
typedef	struct _tPicture
{
	unsigned int palette[16];
	int height;
	int width;
	char pixels[262144];
	ePictureType pictureType;
} tPicture;
#endif

