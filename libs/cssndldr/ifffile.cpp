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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sysdef.h"
#include "csutil/csvector.h"
#include "cssndldr/common/sndbuf.h"
#include "cssndldr/funcs.h"
#include "cssndldr/ifffile.h"

// Amiga 8svx/iff file format loader
//  support 8 bits PCM

#define setStream(x) {if(x>size) {goto exit_read;} else {index=x;}}
#define canAddStream(x) {if((index+x)>size) goto exit_read;}
#define addStream(x) {if((index+x)>size) {goto exit_read;} else {index+=x;}}
#define Stream buf[index]

bool RegisterIFF ()
{
  static IFFLoader loader;
  return csSoundBufferLoader::Register (&loader);
}

csSoundBuffer* IFFLoader::loadsound(UByte* buf, ULong size)
{
  unsigned long index=0;
  csSoundBuffer *sb= NULL;
  void *data=NULL;
  unsigned char dummy0, dummy1, dummy2, dummy3;

  unsigned long freq = 0, length_form, samples_size = 0, min_freq, max_freq;

  if(memcmp(&Stream, "FORM", 4))
    goto exit_read;
  addStream(4);

  dummy0 = Stream; addStream(1);
  dummy1 = Stream; addStream(1);
  dummy2 = Stream; addStream(1);
  dummy3 = Stream; addStream(1);
  length_form = csSndFunc::makeDWord(dummy0, dummy1, dummy2, dummy3)-4;

  if(length_form>size)
    goto exit_read;

  if(memcmp(&Stream, "8SVX", 4))
    goto exit_read;
  addStream(4);

  while(index<length_form)
  {
    char chunk[4];
    int chunk_size;
    memcpy(chunk, &Stream, 4);
    addStream(4);
    dummy0 = Stream; addStream(1);
    dummy1 = Stream; addStream(1);
    dummy2 = Stream; addStream(1);
    dummy3 = Stream; addStream(1);
    chunk_size = csSndFunc::makeDWord(dummy0, dummy1, dummy2, dummy3);

    if(memcmp(chunk, "VHDR", 4)==0)
    {
      dummy0 = Stream; addStream(1);
      dummy1 = Stream; addStream(1);
      dummy2 = Stream; addStream(1);
      dummy3 = Stream; addStream(1);
      samples_size = csSndFunc::makeDWord(dummy0, dummy1, dummy2, dummy3);

      dummy0 = Stream; addStream(1);
      dummy1 = Stream; addStream(1);
      dummy2 = Stream; addStream(1);
      dummy3 = Stream; addStream(1);
      //?

      dummy0 = Stream; addStream(1);
      dummy1 = Stream; addStream(1);
      dummy2 = Stream; addStream(1);
      dummy3 = Stream; addStream(1);
      //?

      dummy0 = Stream; addStream(1);
      dummy1 = Stream; addStream(1);
      freq = csSndFunc::makeWord(dummy0, dummy1);

      dummy0 = Stream; addStream(1);
      dummy1 = Stream; addStream(1);
      // ?
      max_freq = csSndFunc::makeWord(dummy0, dummy1);

      dummy0 = Stream; addStream(1);
      dummy1 = Stream; addStream(1);
      // ?
      min_freq = csSndFunc::makeWord(dummy0, dummy1);

      addStream(chunk_size-18);
    }
    else if(memcmp(chunk, "BODY", 4)==0)
    {
      int i=0;

      if((data= new char[chunk_size])==NULL) goto exit_read;
      char *ptr=(char *)data;

      while(i<chunk_size)
      {
        dummy0 = Stream; addStream(1);
        // datas are stored in unsigned 8 bit but mixer engine only support signed 8 bit
        *ptr++=dummy0-128;
        i++;
      }
    }
    else
    {
      canAddStream(chunk_size);
      addStream(chunk_size);
    }
  }

  if(data==NULL) goto exit_read;

  sb = new csSoundBuffer(freq,
    false,
    false,
    false,
    samples_size,
    data);

  if(sb==NULL) goto exit_read;

  goto exit_ok;
exit_read:
  if(data) delete [] data;

exit_ok:
  return sb;
}
