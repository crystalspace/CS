/*
    Copyright (C) 1998 by Jorrit Tyberghein
	Written by Robert Bergkvist (FragDance)

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

#include <math.h>
#include <stdio.h>

#include "sysdef.h"
#include "csgfxldr/sgiimage.h"


//---------------------------------------------------------------------------
struct SGIHeader{
    UShort Magic;//Magic id
    UByte Storage;//Using rle or straight format
    UByte Bpc;//Bits per channel
    UShort Dimension;//1=1 channel, one scanline, 2=1 channel, many lines, 3=multiple channels
    UShort XSize;//Width
    UShort YSize;//height
    UShort ZSize;//Depth (bpp)
	ULong Pixmin;
	ULong Pixmax;
	ULong Dummy;
	UByte Imagename[80];
	ULong Colormap;
}header;

bool littleendian=false;

bool RegisterSGI ()
{
  static SGIImageLoader loader;
  return ImageLoader::Register (&loader);
}

ImageFile* SGIImageLoader::LoadImage (UByte* buf, ULong size)
{
  CHK (ImageSGIFile* i = new ImageSGIFile(buf, size));
  if (i && (i->get_status() & IFE_BadFormat))
  { CHK ( delete i );  i = NULL; }
  return i;    
}

AlphaMapFile* SGIImageLoader::LoadAlphaMap(UByte* buf,ULong size)
{
	return NULL;
}

//---------------------------------------------------------------------------

ImageSGIFile::~ImageSGIFile ()
{
}

ImageSGIFile::ImageSGIFile (UByte* ptr, long filesize) : ImageFile ()
{
	(void)filesize;
	status=IFE_BadFormat;
	RGBPixel *bufPtr=NULL;	//Where the result should go
	if(readHeader(ptr,3))
	{
		set_dimensions(header.XSize,header.YSize);
		bufPtr=get_buffer();
		if(!header.Storage)	//Indicates if it's rle encoded or not
		{
			UByte *redline=(UByte*)malloc(header.XSize);
			UByte *greenline=(UByte*)malloc(header.XSize);
			UByte *blueline=(UByte*)malloc(header.XSize);
			ptr+=512;
			for(int i=0;i<header.YSize;i++)
			{
				memcpy(redline,ptr,header.XSize);ptr+=header.XSize;
				memcpy(greenline,ptr,header.XSize);ptr+=header.XSize;
				memcpy(blueline,ptr,header.XSize);ptr+=header.XSize;
				for(int j=0;j<header.XSize;j++)
				{
					bufPtr[j+(i*header.XSize)].red=redline[j];
					bufPtr[j+(i*header.XSize)].green=greenline[j];
					bufPtr[j+(i*header.XSize)].blue=blueline[j];
				}
			}
		}
		else
		{
			loadRGBRLE(ptr,bufPtr);
		}
		status=IFE_OK;
	}
	
}

//Load a table of offsets
void ImageSGIFile::loadSGITables(UByte *in,ULong *out,int size)
{
	for(int i=0;i<size;i++)
	{
		out[i]=getLong(in);
		in+=4;
	}
}

//Decode a rle encoded line
UInt ImageSGIFile::decode_rle(UByte *buf,ULong offset,ULong length,UByte *out,UByte *tmp)
{
	UByte count=0;
	int index=0,size=0;
	
	memcpy(tmp,buf+offset,length);

	while((count=((tmp[index])&0x7f)))	//Avsluta när count är noll
	{
		if((tmp[index++]&0x80)==0)
		{
			//Just copy the next byte count times
			for(int j=0;j<count;j++)
			{
				out[size++]=tmp[index];
			}
			index++;
		}
		else
		{
			for(int j=0;j<count;j++)
			{
				out[size++]=tmp[index++];
			}
		}
	}
	return (size);
}

//Load an RLE encoded 3 bpp picture
bool ImageSGIFile::loadRGBRLE(UByte *in,RGBPixel *image)
{
	ULong *starttable=(ULong*)malloc(sizeof(ULong)*header.YSize*header.ZSize);
	ULong *lengthtable=(ULong*)malloc(sizeof(ULong)*header.YSize*header.ZSize);
	UByte *redline=(UByte*)malloc(header.XSize);
	UByte *greenline=(UByte*)malloc(header.XSize);
	UByte *blueline=(UByte*)malloc(header.XSize);
	UByte *tmpline=(UByte*)malloc(header.XSize*2);
	loadSGITables(in+512,starttable,header.YSize*header.ZSize);
	loadSGITables(in+512+(sizeof(ULong)*header.XSize*header.ZSize),lengthtable,header.YSize*header.ZSize);
	for(int i=0;i<header.XSize;i++)
	{
		if(header.XSize!=decode_rle(in,starttable[i],lengthtable[i],redline,tmpline))
			break;

		if(header.XSize!=decode_rle(in,starttable[i+header.XSize],lengthtable[i+header.XSize],
			greenline,tmpline))
			break;

		if(header.XSize!=decode_rle(in,starttable[i+(header.XSize*2)],lengthtable[i+(header.XSize*2)],
			blueline,tmpline))
			break;

		for(int j=0;j<header.XSize;j++)
		{
			image[j+(i*header.XSize)].red=redline[j];
			image[j+(i*header.XSize)].green=greenline[j];
			image[j+(i*header.XSize)].blue=blueline[j];
		}
	}
	if(starttable) free(starttable);
	if(lengthtable) free(lengthtable);
	if(redline) free(redline);
	if(greenline) free(greenline);
	if(blueline) free(blueline);
	if(tmpline) free(tmpline);
	return false;
}

//Read header and see if its what we're after
bool ImageSGIFile::readHeader(UByte *ptr,UInt numplanes)
{
	UByte *tmpPtr=ptr;

	//This could propably be done better
	header.Magic=getShort(tmpPtr);tmpPtr+=2;
		if(header.Magic!=474) return false; 
	header.Storage=(*(tmpPtr));tmpPtr++;
	header.Bpc=(*(tmpPtr));tmpPtr++;
	header.Dimension=getShort(tmpPtr);tmpPtr+=2;
	header.XSize=getShort(tmpPtr);tmpPtr+=2;
	header.YSize=getShort(tmpPtr);tmpPtr+=2;
	header.ZSize=getShort(tmpPtr);tmpPtr+=2;
	return (header.ZSize==numplanes);	//Check that it's a RGB pic
}

//Get a long from buffer
ULong ImageSGIFile::getLong(UByte *ptr)
{
	return ((*(ptr)<<24)|(*(ptr+1)<<16)|(*(ptr+2)<<8)|(*(ptr+3)));
}

//Get a short from buffer
UShort ImageSGIFile::getShort(UByte *ptr)
{
	return (((*ptr)<<8)|(*(ptr+1)));
}

