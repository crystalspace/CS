/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
  
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sysdef.h"

#ifndef OS_WIN32
#include <unistd.h>
#endif

#include "types.h"

typedef struct
{
	char	manufacturer;
	char	version;
	char	encoding;
	char	bits_per_pixel;
        SShort	xmin;
	SShort	ymin;
	SShort	xmax;
	SShort	ymax;
	SShort	hres;
	SShort	vres;
	UByte	palette[48];
	char	reserved;
	char	color_planes;
	SShort	bytes_per_line;
	SShort	palette_type;
	char	filler[58];
	UByte	data;
} pcx_header;

void WritePCX (char *name, unsigned char *data, UByte *pal, int width, int height)
{
	int i, j, len;
	pcx_header *pcx;
	UByte *pack;
	FILE *fp;

	pcx = (pcx_header*)malloc (width*height*2+1000);
	pcx->manufacturer = 10;         // Some programs complains if this is 0
	pcx->version = 5;
	pcx->encoding = 1;
	pcx->bits_per_pixel = 8;
	pcx->xmin = 0;
	pcx->ymin = 0;
	pcx->xmax = width - 1;
	pcx->ymax = height - 1;
	pcx->hres = width;
	pcx->vres = height;
	memset (pcx->palette, 0, sizeof(pcx->palette));
	pcx->color_planes = 1;
	pcx->bytes_per_line = width;
	pcx->palette_type = 2;
	memset (pcx->filler, 0, sizeof(pcx->filler));
	pack = &(pcx->data);

	for (i=0; i<height; i++)
	{
		for (j=0; j<width; j++)
		{
			if  ((*data & 0xc0) != 0xc0)
				*pack++ = *data++;
			else
			{
				*pack++ = 0xc1;
				*pack++ = *data++;
			}
		}
//		data += width;
	}
	*pack++ = 0x0c;
	for (i=0; i<768; i++)
		*pack++ = *pal++;
	len = pack - (UByte *)pcx;
	fp = fopen (name, "wb");
	if (!fp) return;
	fwrite (pcx, len, 1, fp);
	fclose (fp);
	free (pcx);
}
