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

#include "cssysdef.h"
#include "util.h"
#include "isound/loader.h"
#include "soundraw.h"
#include "sndload.h"

// Microsoft Wav file loader
//  support 8 and 16 bits PCM (RIFF)

class csSoundLoader_WAV : public iSoundLoader
{
public:
  DECLARE_IBASE;

  csSoundLoader_WAV(iBase *p) {
    CONSTRUCT_IBASE(p);
  }
  virtual bool Initialize(iSystem *) {
    return true;
  }
  virtual iSoundData *LoadSound(void *Buffer, unsigned long Size) const;
};

IMPLEMENT_IBASE(csSoundLoader_WAV)
  IMPLEMENTS_INTERFACE(iSoundLoader)
  IMPLEMENTS_INTERFACE(iPlugIn)
IMPLEMENT_IBASE_END;

IMPLEMENT_FACTORY(csSoundLoader_WAV);


struct _WAVchk
{
  char chunk[4];
  unsigned long len;
} wavchk;

struct _WAVhdr
{
  char format[4];
  unsigned long f_len;
  char fmt[8];
  unsigned long fmt_len;
  unsigned short fmt_tag;
  unsigned short channel;
  unsigned long samples_per_sec;
  unsigned long avg_bytes_per_sec;
  unsigned short blk_align;
  unsigned short bits_per_sample;
} wavhdr;

#define setStream(x) {if(x>size) {goto exit_read;} else {index=x;}}
#define canAddStream(x) {if((index+x)>size) goto exit_read;}
#define addStream(x) {if((index+x)>size) {goto exit_read;} else {index+=x;}}
#define Stream buf[index]

iSoundData *csSoundLoader_WAV::LoadSound(void *databuf, ULong size) const {
  UByte *buf = (UByte*) databuf;
  int index=0;

  csSoundDataRaw *sb= NULL;
  char *data=NULL;
  UByte *ptr;
  UByte *ptr_end;

  memset(&wavhdr, 0, sizeof(wavhdr));
  if(size<sizeof(wavhdr)) goto exit_read;
    
  if(memcpy(&wavhdr, &Stream, sizeof(wavhdr))==NULL)
    goto exit_read;
  canAddStream(sizeof(wavhdr));
  addStream(sizeof(wavhdr));

  if(memcmp(wavhdr.format, "RIFF", 4))
    goto exit_read;

  if(memcmp(wavhdr.fmt, "WAVEfmt ", 8))
    goto exit_read;
  
#ifdef CS_BIG_ENDIAN
  wavhdr.f_len = ByteSwap32bit( wavhdr.f_len );
  wavhdr.fmt_len = ByteSwap32bit( wavhdr.fmt_len );
  wavhdr.fmt_tag = ByteSwap16bit( wavhdr.fmt_tag );
  wavhdr.channel = ByteSwap16bit( wavhdr.channel );
  wavhdr.samples_per_sec = ByteSwap32bit( wavhdr.samples_per_sec );
  wavhdr.avg_bytes_per_sec = ByteSwap32bit( wavhdr.avg_bytes_per_sec );
  wavhdr.blk_align = ByteSwap16bit( wavhdr.blk_align );
  wavhdr.bits_per_sample = ByteSwap16bit( wavhdr.bits_per_sample );
#endif // CS_BIG_ENDIAN

  if(!((wavhdr.channel == 1) || (wavhdr.channel == 2)))
    goto exit_read;
  

  ptr=&Stream;
  ptr_end=ptr+size;
  while(ptr<ptr_end)
  {
    memcpy(&wavchk, ptr, sizeof(wavchk));
#ifdef CS_BIG_ENDIAN
    wavchk.len = ByteSwap32bit( wavchk.len );
#endif // CS_BIG_ENDIAN

    ptr+=sizeof(wavchk);
    if(memcmp(wavchk.chunk, "data", 4)==0)
      break;
    ptr+=wavchk.len;
  }
  
  data=new char[wavchk.len];
  if(data==NULL)
    goto exit_read;
  
  if(memcpy(data, ptr, wavchk.len)==NULL)
    goto exit_read;  

#ifdef CS_BIG_ENDIAN
  if ( wavhdr.bits_per_sample==16 ) {
    ByteSwap16bitBuffer( (unsigned short *)data, wavchk.len / 2 );
  }
#endif // CS_BIG_ENDIAN


  csSoundFormat Format;
  Format.Freq=wavhdr.samples_per_sec;
  Format.Bits=wavhdr.bits_per_sample;
  Format.Channels=wavhdr.channel;
  sb=new csSoundDataRaw(NULL, data,
    (wavhdr.bits_per_sample==16)?(wavchk.len/2)-1:wavchk.len-1, Format);

  goto exit_ok;
exit_read:
  if(data) delete [] data;

exit_ok:
  return sb;
}
