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

#include "cssysdef.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "isound/loader.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csplugincommon/soundloader/soundraw.h"
#include "csplugincommon/soundloader/sndload.h"

CS_IMPLEMENT_PLUGIN

// MacIntosh AIFF file format loader

// big warning, I hack this format with a hex editor,
// if you have some informations about this format
// email me at noote@bigfoot.com

class csSoundLoader_AIFF : public iSoundLoader
{
public:
  SCF_DECLARE_IBASE;

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSoundLoader_AIFF);
    virtual bool Initialize (iObjectRegistry*) { return true; }
  } scfiComponent;

  csSoundLoader_AIFF(iBase *p)
  {
    SCF_CONSTRUCT_IBASE(p);
    SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  }
  virtual ~csSoundLoader_AIFF()
  {
    SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
    SCF_DESTRUCT_IBASE();
  }
  virtual csPtr<iSoundData> LoadSound(void *Buffer, size_t Size);
};

SCF_IMPLEMENT_IBASE(csSoundLoader_AIFF)
  SCF_IMPLEMENTS_INTERFACE(iSoundLoader)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSoundLoader_AIFF::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY(csSoundLoader_AIFF)


#define BIT8 0x0008
#define BIT16 0x0010
#define HZ11025 0x400C
#define HZ22050 0x400D
#define HZ44100 0x400E

#define setStream(x) {if(x>size) {goto exit_read;} else {index=x;}}
#define canAddStream(x) {if((index+x)>size) goto exit_read;}
#define addStream(x) {if((index+x)>size) {goto exit_read;} else {index+=x;}}
#define Stream buf[index]

csPtr<iSoundData>
csSoundLoader_AIFF::LoadSound(void* databuf, size_t size)
{
  uint8 *buf = (uint8*) databuf;
  unsigned long index=0;
  csSoundDataRaw *sb= 0;
  char *data=0;
  unsigned char dummy0, dummy1, dummy2, dummy3;

  unsigned long flag = 0, flag2 = 0, nchannels = 0, length_form,
    samples_size = 0, max_freq;

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

  if(memcmp(&Stream, "AIFF", 4))
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

    if(memcmp(chunk, "COMM", 4)==0)
    {
      dummy0 = Stream; addStream(1);
      dummy1 = Stream; addStream(1);
      nchannels = csSndFunc::makeWord(dummy0, dummy1);
      if(nchannels !=1 && nchannels !=2)
        goto exit_read;

      dummy0 = Stream; addStream(1);
      dummy1 = Stream; addStream(1);
      dummy2 = Stream; addStream(1);
      dummy3 = Stream; addStream(1);
      samples_size = csSndFunc::makeDWord(dummy0, dummy1, dummy2, dummy3);
      if(samples_size>size)
        goto exit_read;

      dummy0 = Stream; addStream(1);
      dummy1 = Stream; addStream(1);
      flag = csSndFunc::makeWord(dummy0, dummy1);
      if(flag!=BIT8 && flag!=BIT16)
        goto exit_read;

      dummy0 = Stream; addStream(1);
      dummy1 = Stream; addStream(1);
      flag2 = csSndFunc::makeWord(dummy0, dummy1);
      if(flag2!=HZ11025 && flag2!=HZ22050 && flag2!=HZ44100)
        goto exit_read;

      dummy0 = Stream; addStream(1);
      dummy1 = Stream; addStream(1);
      // ?
      max_freq = csSndFunc::makeWord(dummy0, dummy1);

      addStream(chunk_size-12);
    }
    else if(memcmp(chunk, "SSND", 4)==0)
    {
      if(flag==BIT8)
      {
        int i=0;
        if((uint32)chunk_size>size)
          goto exit_read;

        data = new char[chunk_size];
        if(data==0) goto exit_read;
        char *ptr=(char *)data;

        while(i<chunk_size)
        {
          dummy0 = Stream; addStream(1);
          // datas are stored in unsigned 8 bit but mixer engine only support
          // signed 8 bit
          *ptr++=dummy0-128;
          i++;
        }
      }
      else if(flag==BIT16)
      {
        int i=0;
        if((uint32)chunk_size>size)
          goto exit_read;

        data= new char[chunk_size];
        if(data==0) goto exit_read;
        unsigned short *ptr=(unsigned short *)data;

        int nbs = chunk_size/2;
        while(i<nbs)
        {
          dummy0 = Stream; addStream(1);
          dummy1 = Stream; addStream(1);
          *ptr++=csSndFunc::makeWord(dummy0, dummy1);
          i++;
        }
      }
    }
    else
    {
      canAddStream(chunk_size);
      addStream(chunk_size);
    }
  }

  if(data==0) goto exit_read;

  csSoundFormat Format;
  Format.Freq=(flag2==HZ11025)?11025:(flag2==HZ22050)?22050:44100;
  Format.Bits=(flag==BIT16)?16:8;
  Format.Channels=nchannels;
  sb=new csSoundDataRaw(0, data,
    (flag==BIT16)?samples_size/2:samples_size, Format);

  goto exit_ok;
exit_read:
  delete [] data;

exit_ok:
  return csPtr<iSoundData> (sb);
}
