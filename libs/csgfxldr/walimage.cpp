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

/* rd_tga.c - read a TrueVision Targa file
**
** Partially based on tgatoppm.c from pbmplus (just a big hack :)
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include <math.h>
#include <stdio.h>

#include "sysdef.h"
#include "csgfxldr/walimage.h"
#include "walpal.h"

//---------------------------------------------------------------------------

bool RegisterWAL ()
{
  static WALImageLoader loader;
  return ImageLoader::Register (&loader);
}

csImageFile* WALImageLoader::LoadImage (UByte* buf, ULong size)
{
  CHK (ImageWALFile* i = new ImageWALFile(buf, size));
  if (i && (i->get_status() & IFE_BadFormat))
  { CHK ( delete i );  i = NULL; }
  return i;    
}

AlphaMapFile *WALImageLoader::LoadAlphaMap(UByte* buf,ULong size)
{
	return NULL;
}

//---------------------------------------------------------------------------
ImageWALFile::~ImageWALFile ()
{
}

ImageWALFile::ImageWALFile (UByte* ptr, long filesize) : csImageFile ()
{
  status=IFE_BadFormat;
  long chkfilesize;
  unsigned char *datPtr;
  unsigned int palindex;
  unsigned int i;
  RGBPixel *bufPtr;	//Where the result should go

  WALHeader *head=(WALHeader*)ptr;
  
  // There's no id-tag in .WAL files, so the only way I know to check
  // if it's a wal, is to use this method. Hope it works
  chkfilesize = head->offsets[3]+((head->height/8)*(head->width/8));
  if (chkfilesize != filesize) return;

  //There are 4 mipmaps in a wal-file, but we just use the first and
  //Discard the rest
  set_dimensions(head->width,head->height);
  bufPtr=get_buffer();
  datPtr=ptr+head->offsets[0];
  for(i=0;i<(head->width*head->height);i++)
  {
	  palindex=*datPtr++;
	  palindex*=3;
	  bufPtr[i].red=palette[palindex++];
	  bufPtr[i].green=palette[palindex++];
	  bufPtr[i].blue=palette[palindex];
  }
  status=IFE_OK;
}
