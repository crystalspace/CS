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

// Sun AU file loader
//  support 8 and 16 bits PCM
//  and 8 bit ULAW (no compressed)

class csSoundLoader_AU : public iSoundLoader
{
public:
  SCF_DECLARE_IBASE;

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSoundLoader_AU);
    virtual bool Initialize (iObjectRegistry*) { return true; }
  } scfiComponent;

  csSoundLoader_AU(iBase *p)
  {
    SCF_CONSTRUCT_IBASE(p);
    SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  }

  virtual ~csSoundLoader_AU()
  {
    SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
    SCF_DESTRUCT_IBASE();
  }

  virtual csPtr<iSoundData> LoadSound(void *Buffer, size_t Size);
};

SCF_IMPLEMENT_IBASE(csSoundLoader_AU)
  SCF_IMPLEMENTS_INTERFACE(iSoundLoader)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (csSoundLoader_AU::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY(csSoundLoader_AU);


#define BIT16 0x03
#define BIT8 0x02
#define BIT8ULAW 0x01

#define setStream(x) {if(x>size) {goto exit_read;} else {index=x;}}
#define canAddStream(x) {if((index+x)>size) goto exit_read;}
#define addStream(x) {if((index+x)>size) {goto exit_read;} else {index+=x;}}
#define Stream buf[index]

csPtr<iSoundData>
csSoundLoader_AU::LoadSound(void *databuf, size_t size)
{
  uint8 *buf = (uint8*) databuf;
  unsigned long index=0;
  csSoundDataRaw *sb= 0;
  char *data=0;
  unsigned char dummy0, dummy1, dummy2, dummy3;

  unsigned long offset, nbytes, flag, freq, nchannels;

  if(memcmp(&Stream, ".snd", 4))
    goto exit_read;
  addStream(4);

  dummy0 = Stream; addStream(1);
  dummy1 = Stream; addStream(1);
  dummy2 = Stream; addStream(1);
  dummy3 = Stream; addStream(1);
  offset = csSndFunc::makeDWord(dummy0, dummy1, dummy2, dummy3);

  dummy0 = Stream; addStream(1);
  dummy1 = Stream; addStream(1);
  dummy2 = Stream; addStream(1);
  dummy3 = Stream; addStream(1);
  nbytes = csSndFunc::makeDWord(dummy0, dummy1, dummy2, dummy3);

  dummy0 = Stream; addStream(1);
  dummy1 = Stream; addStream(1);
  dummy2 = Stream; addStream(1);
  dummy3 = Stream; addStream(1);
  flag = csSndFunc::makeDWord(dummy0, dummy1, dummy2, dummy3);
  if(flag!=BIT16 && flag!=BIT8 && flag!=BIT8ULAW)
    goto exit_read;

  dummy0 = Stream; addStream(1);
  dummy1 = Stream; addStream(1);
  dummy2 = Stream; addStream(1);
  dummy3 = Stream; addStream(1);
  freq = csSndFunc::makeDWord(dummy0, dummy1, dummy2, dummy3);

  dummy0 = Stream; addStream(1);
  dummy1 = Stream; addStream(1);
  dummy2 = Stream; addStream(1);
  dummy3 = Stream; addStream(1);
  nchannels = csSndFunc::makeDWord(dummy0, dummy1, dummy2, dummy3);
  if(nchannels>2 || nchannels<1)
    goto exit_read;

  canAddStream(nbytes);
  if(flag==BIT8)
  {
    data=new char[nbytes];
    if (data==0)
      goto exit_read;

    unsigned long i=0;
    char *ptr=(char *)data;
    while(i<nbytes)
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
    data=new char[nbytes];
    if(data==0)
      goto exit_read;

    int i=0;
    int nbs = nbytes/2;
    unsigned short *ptr=(unsigned short *)data;
    while(i<nbs)
    {
      dummy0 = Stream; addStream(1);
      dummy1 = Stream; addStream(1);
      *ptr++=csSndFunc::makeWord(dummy0, dummy1);
      i++;
    }
  }
  else if(flag==BIT8ULAW)
  {
    data=new char[nbytes*2];
    if(data==0)
      goto exit_read;

    int i=0;
    int nbs = nbytes;
    unsigned short *ptr=(unsigned short *)data;
    while(i<nbs)
    {
      dummy0 = Stream; addStream(1);
      *ptr++=csSndFunc::ulaw2linear(dummy0);
      i++;
    }
  }

  csSoundFormat Format;
  Format.Freq=freq;
  Format.Bits=(flag==BIT16 || flag==BIT8ULAW)?16:8;
  Format.Channels=nchannels;
  sb=new csSoundDataRaw(0, data,
    (flag==BIT16)?(nbytes/2)-1:nbytes-1, Format);

  goto exit_ok;
exit_read:
  delete [] data;

exit_ok:
  return csPtr<iSoundData> (sb);
}
