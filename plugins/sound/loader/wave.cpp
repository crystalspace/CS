/*
    Copyright (C) 1998, 1999 by Nathaniel 'NooTe' Saint Martin
    Copyright (C) 1998, 1999 by Jorrit Tyberghein
    Written by Nathaniel 'NooTe' Saint Martin

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
#include "qint.h"
#include "wave.h"

IMPLEMENT_IBASE(csSoundDataWave)
  IMPLEMENTS_INTERFACE(iSoundData)
IMPLEMENT_IBASE_END

csSoundDataWave::csSoundDataWave(iBase *iParent) {
  CONSTRUCT_IBASE(iParent);
  Data=NULL;
}

csSoundDataWave::~csSoundDataWave() {
  Cleanup();
}

void csSoundDataWave::Cleanup() {
  if (Data) delete [] Data;
  Data=NULL;
}

void csSoundDataWave::Initialize(int frequency, int bits, int channels, long numsmp, unsigned char *data)
{
  Cleanup();

  NumSamples = numsmp;
  Format.Freq = frequency;
  Format.Bits = bits;
  Format.Channels = channels;
  Data=data;
}

void csSoundDataWave::Prepare(const csSoundFormat *fmt) {
  if (fmt->Channels>0) ConvertChannels(fmt->Channels);
  if (fmt->Bits>0) ConvertBits(fmt->Bits);
  if (fmt->Freq>0) ConvertFreq(fmt->Freq);
}

const csSoundFormat *csSoundDataWave::GetFormat() {
  return &Format;
}

iSoundStream *csSoundDataWave::CreateStream() {
  return new csSoundStreamWave(this);
}

unsigned long csSoundDataWave::GetNumSamples() {
  return NumSamples;
}

#define CONVERT_CHANNELS_TYPE(Type) {               \
  Type *OldData=(Type*)Data;                        \
  if (Channels==1) {                                \
    Type *NewData=new Type[NumSamples];             \
    for (unsigned long i=0;i<NumSamples;i++) {      \
      NewData[i]=(OldData[2*i]+OldData[2*i+1])/2;   \
    }                                               \
    delete[] OldData;                               \
    Data=(unsigned char*)NewData;                   \
  } else {                                          \
    Type *NewData=new Type[NumSamples*2];           \
    for (unsigned long i=0;i<NumSamples;i++) {      \
      NewData[2*i]=NewData[2*i+1]=OldData[i];       \
    }                                               \
    delete[] OldData;                               \
    Data=(unsigned char*)NewData;                   \
  }                                                 \
}

void csSoundDataWave::ConvertChannels(int Channels) {
  if (Format.Channels==Channels) return;
  if (Format.Bits==16) CONVERT_CHANNELS_TYPE(short)
  else CONVERT_CHANNELS_TYPE(char)
  Format.Channels=Channels;
}

// @@@ ConvertFreq() : quality loss! Need to use a filter.

#define CONVERT_FREQ_TYPE(Type,Channels) {                      \
  Type *NewData=new Type[NewNumSamples*Channels];               \
  Type *OldData=(Type*)Data;                                    \
  for (unsigned long i=0;i<NewNumSamples;i++) {                 \
    int samppos = (int)(i/Factor);                              \
    if (Channels==1) {                                          \
      NewData[i]=OldData[samppos];                              \
    } else {                                                    \
      NewData[2*i]=OldData[2*samppos];                          \
      NewData[2*i+1]=OldData[2*samppos+1];                      \
    }                                                           \
  }                                                             \
  delete[] OldData;                                             \
  Data=(unsigned char*)NewData;                                 \
  NumSamples=NewNumSamples;                                     \
}

void csSoundDataWave::ConvertFreq(int NewFreq) {
  if (NewFreq==Format.Freq) return;
  float Factor=NewFreq/Format.Freq;
  unsigned long NewNumSamples=(unsigned long)(NumSamples*Factor);
  if (Format.Bits==16) {
    CONVERT_FREQ_TYPE(short,Format.Channels);
  } else {
    CONVERT_FREQ_TYPE(unsigned char,Format.Channels);
  }
  Format.Freq=NewFreq;
}

void *ConvertBuffer8To16Bit(void *buf, unsigned long Num) {
  unsigned char *in=(unsigned char *)buf;
  short *out=new short[Num];
  for (unsigned long i=0;i<Num;i++) {
    out[i]=((short)in[i]-128)*256;
  }
  return out;
}

void *ConvertBuffer16To8Bit(void *buf, unsigned long Num) {
  short *in=(short *)buf;
  unsigned char *out=new unsigned char[Num];
  for (unsigned long i=0;i<Num;i++) {
    out[i]=(in[i]/256)+128;
  }
  return out;
}

void csSoundDataWave::ConvertBits(int Bits) {
  if (Format.Bits==Bits) return;
  void *(*Proc)(void*,unsigned long);
  if (Bits==16) Proc=ConvertBuffer8To16Bit;
  else Proc=ConvertBuffer16To8Bit;
  void *d=Proc(Data,NumSamples*Format.Channels);
  delete[] Data;
  Data=(unsigned char*)d;
  Format.Bits=Bits;
}

iSoundData *csSoundDataWave::Decode() {
  // the user expects to have two separate objects now, which means we have
  // to increase RefCount.
  IncRef();
  return this;
}

IMPLEMENT_IBASE(csSoundStreamWave)
  IMPLEMENTS_INTERFACE(iSoundStream)
IMPLEMENT_IBASE_END

csSoundStreamWave::csSoundStreamWave(csSoundDataWave *snd) {
  CONSTRUCT_IBASE(snd);
  Data=snd;
  Position=0;
}

iSoundData *csSoundStreamWave::GetSoundData() {
  return Data;
}

void *csSoundStreamWave::Read(unsigned long &NumSamples) {
  // decrease size of data if we reached the end of the sound
  if (Position+NumSamples>Data->NumSamples) {
    NumSamples=Data->NumSamples-Position;
  }
  // set up return data
  void *d;
  if (Data->Format.Bits==16) {
    d=(short*)(Data->Data)+Position*Data->Format.Channels;
  } else {
    d=(unsigned char *)(Data->Data)+Position*Data->Format.Channels;
  }
  Position+=NumSamples;
  return d;
}

void csSoundStreamWave::DiscardBuffer(void *buf) {
  // our buffers are not allocated, so don't delete them
  (void)buf;
}

void csSoundStreamWave::Reset() {
  Position=0;
}
