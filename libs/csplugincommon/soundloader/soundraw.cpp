/*
    Copyright (C) 1998, 1999 by Jorrit Tyberghein

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
#include "csplugincommon/soundloader/soundraw.h"

SCF_IMPLEMENT_IBASE(csSoundDataRaw);
  SCF_IMPLEMENTS_INTERFACE(iSoundData);
SCF_IMPLEMENT_IBASE_END

csSoundDataRaw::csSoundDataRaw(iBase *p, void *d, long n, csSoundFormat f)
{
  SCF_CONSTRUCT_IBASE(p);
  Data = d;
  NumSamples = n;
  Format = f;
}

csSoundDataRaw::~csSoundDataRaw()
{
  unsigned char* const p = (unsigned char*)Data;
  delete[] p;
  SCF_DESTRUCT_IBASE();
}


const csSoundFormat *csSoundDataRaw::GetFormat()
{
  return &Format;
}


bool csSoundDataRaw::IsStatic()
{
  return true;
}

long csSoundDataRaw::GetStaticSampleCount()
{
  return NumSamples;
}

void *csSoundDataRaw::GetStaticData()
{
  return Data;
}

void csSoundDataRaw::ResetStreamed()
{
}

void *csSoundDataRaw::ReadStreamed(long &)
{
  return 0;
}

/*** format conversion functions follow ***/

#define REPLACE_DATA(x)				 \
{                        			 \
  unsigned char* const p = (unsigned char*)Data; \
  Data = x;                                      \
  delete[] p;                                    \
}

void *ConvertBuffer8To16Bit(void *buf, unsigned long Num)
{
  unsigned char *in=(unsigned char *)buf;
  short *out=new short[Num];
  for (unsigned long i=0;i<Num;i++)
  {
    out[i]=((short)in[i]-128)*256;
  }
  return out;
}

void *ConvertBuffer16To8Bit(void *buf, unsigned long Num)
{
  short *in=(short *)buf;
  unsigned char *out=new unsigned char[Num];
  for (unsigned long i=0;i<Num;i++)
  {
    out[i]=(in[i]/256)+128;
  }
  return out;
}

#define CONVERT_CHANNELS_TYPE(Type)		    \
{               				    \
  Type *OldData=(Type*)d;                           \
  if (newfmt->Channels==1)			    \
  {                        			    \
    Type *NewData=new Type[NumSamples];             \
    for (long i=0;i<NumSamples;i++)		    \
    {      					    \
      NewData[i]=(OldData[2*i]+OldData[2*i+1])/2;   \
    }                                               \
    return NewData;                                 \
  }						    \
  else						    \
  {                                          	    \
    Type *NewData=new Type[NumSamples*2];           \
    for (long i=0;i<NumSamples;i++)		    \
    {      					    \
      NewData[2*i]=NewData[2*i+1]=OldData[i];       \
    }                                               \
    return NewData;                                 \
  }                                                 \
}

void *ConvertChannels(void *d, const csSoundFormat *oldfmt,
  const csSoundFormat *newfmt, long NumSamples)
{
  if (oldfmt->Bits == 8)
  {
    CONVERT_CHANNELS_TYPE(unsigned char);
  }
  else
  {
    CONVERT_CHANNELS_TYPE(short);
  }
}

// @@@ ConvertFreq() : quality loss! Need to use a filter.

#define CONVERT_FREQ_TYPE(Type,Channels)			\
{                      						\
  Type *NewData=new Type[NewNumSamples*Channels];               \
  Type *OldData=(Type*)d;                                       \
  for (unsigned long i=0;i<NewNumSamples;i++)			\
  {                 						\
    int samppos = (int)(i/Factor);                              \
    if (Channels==1)						\
    {                                          			\
      NewData[i]=OldData[samppos];                              \
    }								\
    else							\
    {                                                    	\
      NewData[2*i]=OldData[2*samppos];                          \
      NewData[2*i+1]=OldData[2*samppos+1];                      \
    }                                                           \
  }                                                             \
  NumSamples = NewNumSamples;                                   \
  return NewData;                                               \
}

void *ConvertFreq(void *d, const csSoundFormat *oldfmt,
  const csSoundFormat *newfmt, long &NumSamples)
  {
  float Factor=newfmt->Freq/(float)oldfmt->Freq;
  unsigned long NewNumSamples=(unsigned long)(NumSamples*Factor);

  if (oldfmt->Bits==16)
  {
    CONVERT_FREQ_TYPE(short,oldfmt->Channels);
  }
  else
  {
    CONVERT_FREQ_TYPE(unsigned char,oldfmt->Channels);
  }
}

bool csSoundDataRaw::Initialize(const csSoundFormat *RequestFormat)
{
  if (Format.Bits==16 && RequestFormat->Bits==8)
  {
    REPLACE_DATA(ConvertBuffer16To8Bit(Data, NumSamples * Format.Channels));
    Format.Bits = 8;
  }
  else if (Format.Bits==8 && RequestFormat->Bits==16)
  {
    REPLACE_DATA(ConvertBuffer8To16Bit(Data, NumSamples * Format.Channels));
    Format.Bits = 16;
  }

  if (Format.Channels != RequestFormat->Channels &&
      RequestFormat->Channels != -1)
  {
    REPLACE_DATA(ConvertChannels(Data, &Format, RequestFormat, NumSamples));
    Format.Channels = RequestFormat->Channels;
  }

  if (RequestFormat->Freq != Format.Freq && RequestFormat->Freq != -1)
  {
    REPLACE_DATA(ConvertFreq(Data, &Format, RequestFormat, NumSamples));
    Format.Freq = RequestFormat->Freq;
  }

  return true;
}
