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
#include "cssfxldr/common/snddata.h"
#include "cssfxldr/funcs.h"
#include "cssfxldr/aufile.h"

// Sun AU file loader
//  support 8 and 16 bits PCM
//  and 8 bit ULAW (no compressed)

#define BIT16 0x03
#define BIT8 0x02
#define BIT8ULAW 0x01

#define setStream(x) {if(x>size) {goto exit_read;} else {index=x;}}
#define canAddStream(x) {if((index+x)>size) goto exit_read;}
#define addStream(x) {if((index+x)>size) {goto exit_read;} else {index+=x;}}
#define Stream buf[index]

bool RegisterAU ()
{
  static AULoader loader;
  return csSoundLoader::Register (&loader);
}

csSoundData* AULoader::loadsound(UByte* buf, ULong size)
{
  unsigned long index=0;
  csSoundData *sb= NULL;
  void *data=NULL;
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
    CHK (data=new char[nbytes]);
    if (data==NULL)
      goto exit_read;

    unsigned long i=0;
    char *ptr=(char *)data;
    while(i<nbytes)
    {
      dummy0 = Stream; addStream(1);
       // datas are stored in unsigned 8 bit but mixer engine only support signed 8 bit
      *ptr++=dummy0-128;
      i++;
    }
  }
  else if(flag==BIT16)
  {
    CHK (data=new char[nbytes]);
    if(data==NULL)
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
    CHK (data=new char[nbytes*2]);
    if(data==NULL)
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

  CHK (sb = new csSoundData(freq,
    (flag==BIT16 || flag==BIT8ULAW)?true:false,
    (nchannels==2)?true:false,
    (flag==BIT16)?true:false,
    (flag==BIT16)?(nbytes/2)-1:nbytes-1,
    data));
  
  if(sb==NULL) goto exit_read;

  goto exit_ok;
exit_read:
  CHK (delete [] data);

exit_ok:
  return sb;
}
