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
#include "isound/listener.h"
#include "isound/data.h"
#include "srdrsrc.h"
#include "srdrcom.h"
#include "sndhdl.h"
#include "qsqrt.h"

SCF_IMPLEMENT_IBASE(csSoundSourceSoftware)
  SCF_IMPLEMENTS_INTERFACE(iSoundSource)
SCF_IMPLEMENT_IBASE_END;

csSoundSourceSoftware::csSoundSourceSoftware(csSoundRenderSoftware *srdr,
    csSoundHandleSoftware *hdl, int m3d)
{
  SCF_CONSTRUCT_IBASE(hdl);

  SoundRender = srdr;
  FrequencyFactor = 1;
  Volume = 1.0;
  Mode3d = m3d;
  Position = csVector3(0,0,0);
  Velocity = csVector3(0,0,0);
  Active = false;
  SoundPos = 0;
  SoundHandle = hdl;
}

csSoundSourceSoftware::~csSoundSourceSoftware()
{
}

void csSoundSourceSoftware::Play(unsigned long pMethod)
{
  PlayMethod=pMethod;
  if (!Active) SoundRender->AddSource(this);
  Active=true;
  if (PlayMethod & SOUND_RESTART) Restart();
}

void csSoundSourceSoftware::Stop()
{
  if (Active)
  {
    Active=false;
    SoundRender->RemoveSource(this);
  }
}

void csSoundSourceSoftware::Restart()
{
  if (!SoundHandle->Data) return;
  if (SoundHandle->Data->IsStatic())
    SoundPos = 0;
  else
    SoundHandle->Data->ResetStreamed();
}

bool csSoundSourceSoftware::IsActive()
{
  return Active;
}

void csSoundSourceSoftware::SetVolume(float vol)
{
  Volume = vol;
}

float csSoundSourceSoftware::GetVolume()
{
  return Volume;
}

void csSoundSourceSoftware::SetFrequencyFactor(float factor)
{
  FrequencyFactor = factor;
}

float csSoundSourceSoftware::GetFrequencyFactor()
{
  return FrequencyFactor;
}

int csSoundSourceSoftware::GetMode3D()
{
  return Mode3d;
}

void csSoundSourceSoftware::SetMode3D(int m3d)
{
  Mode3d = m3d;
}

void csSoundSourceSoftware::SetPosition(csVector3 pos)
{
  Position=pos;
}

csVector3 csSoundSourceSoftware::GetPosition()
{
  return Position;
}

void csSoundSourceSoftware::SetVelocity(csVector3 v)
{
  Velocity=v;
}

csVector3 csSoundSourceSoftware::GetVelocity()
{
  return Velocity;
}

void csSoundSourceSoftware::Prepare(float BaseVolume)
{
  // frequency
  CalcFreqFactor=FrequencyFactor;

  // volume
  CalcVolL = CalcVolR = Volume*BaseVolume;

  // for non-3d sources we don't calculate anything else
  if (Mode3d == SOUND3D_DISABLE) return;

  // get the global listener object
  iSoundListener *Listener=SoundRender->GetListener();

  // position of the listener's ears
  csVector3 EarL,EarR;

  if (Mode3d == SOUND3D_RELATIVE)
  {
    // position of the sound is relative to the listener, so we simply
    // place the listener at (0,0,0) with front (0,0,1) and top (0,1,0)
    EarL = csVector3(-Listener->GetHeadSize()/2, 0, 0);
    EarR = csVector3(+Listener->GetHeadSize()/2, 0, 0);
  }
  else
  {
    // calculate the 'left' vector
    csVector3 Front,Top,Left;
    Listener->GetDirection(Front,Top);
    Left=Top%Front;
    if (Left.Norm()<EPSILON)
    {
      // user has supplied bad front and top vectors
      CalcVolL = CalcVolR = 0;
      return;
    } else Left.Normalize();

    // calculate ear position
    EarL=Listener->GetPosition()+Left*Listener->GetHeadSize()/2;
    EarR=Listener->GetPosition()-Left*Listener->GetHeadSize()/2;
  }

  // calculate ear distance
  float DistL,DistR;
  DistL=(EarL-Position).Norm();
  DistR=(EarR-Position).Norm();

  // the following tests should not be deleted until we see (or hear)
  // what gives the best results. Just try it!

  // @@@ test: amplify difference between ears.
//  float d=DistL-DistR;
//  DistL+=2*d;
//  DistR-=2*d;

  // @@@ test: take square root of distance. Otherwise the missile sound
  // in walktest seems to become distant too fast.
//  DistL=sqrt(DistL);
//  DistR=sqrt(DistR);

  // prevent too near sounds
  if (DistL<1) DistL=1;
  if (DistR<1) DistR=1;

  // calculate ear volume
  float DistFactor=Listener->GetDistanceFactor();
  CalcVolL/=DistL*DistFactor;
  CalcVolR/=DistR*DistFactor;
}

/* helper macros */
#define READMONOSAMP        ((int)(Input[i])-NullSample)
#define READLEFTSAMP        ((int)(Input[2*i])-NullSample)
#define READRIGHTSAMP       ((int)(Input[2*i+1])-NullSample)

#define WRITEMONOSAMP(x)    Output[i]+=(stype)((x)*(CalcVolL+CalcVolR)/2);
#define WRITELEFTSAMP(x)    Output[2*i]+=(stype)((x)*CalcVolL);
#define WRITERIGHTSAMP(x)   Output[2*i+1]+=(stype)((x)*CalcVolR);

#define READMONO3D          int samp=READMONOSAMP;
#define READSTEREO3D        int samp=(READLEFTSAMP+READRIGHTSAMP)/2;

#define WRITEMONO3D         WRITEMONOSAMP(samp);
#define WRITESTEREO3D       {WRITELEFTSAMP(samp); WRITERIGHTSAMP(samp);}

#define LOOP                for (long i=0;i<NumSamples;i++)

#define ADDTOBUFFER_BITS {                                              \
  stype *Input = (stype*)Source;                                        \
  stype *Output = (stype*)Dest;                                         \
                                                                        \
  if (Mode3d != SOUND3D_DISABLE) {                                      \
    /* is 3d effect is enabled, stereo input channels are mixed */      \
    if (InputFormat->Channels==2) {                                     \
      if (OutputFormat->Channels==2) {                                  \
        LOOP {READSTEREO3D; WRITESTEREO3D;}                             \
      } else {                                                          \
        LOOP {READSTEREO3D; WRITEMONO3D;}                               \
      }                                                                 \
    } else {                                                            \
      if (OutputFormat->Channels==2) {                                  \
        LOOP {READMONO3D; WRITESTEREO3D;}                               \
      } else {                                                          \
        LOOP {READMONO3D; WRITEMONO3D;}                                 \
      }                                                                 \
    }                                                                   \
  } else {                                                              \
    /* handle non-3d sound (use CalcVolL for volume, NOT 'Volume', */   \
    /* to use any precalculation done in Prepare()                 */   \
    if (InputFormat->Channels==2) {                                     \
      if (OutputFormat->Channels==2) {                                  \
        /* stereo -> stereo */                                          \
        LOOP {                                                          \
          WRITELEFTSAMP(READLEFTSAMP);                                  \
          WRITERIGHTSAMP(READRIGHTSAMP);                                \
        }                                                               \
      } else {                                                          \
        /* stereo -> mono */                                            \
        LOOP {                                                          \
          WRITEMONOSAMP((READLEFTSAMP + READRIGHTSAMP)/2);              \
        }                                                               \
      }                                                                 \
    } else {                                                            \
      if (OutputFormat->Channels==2) {                                  \
        /* mono -> stereo */                                            \
        LOOP {                                                          \
          WRITELEFTSAMP(READMONOSAMP);                                  \
          WRITERIGHTSAMP(READMONOSAMP);                                 \
        }                                                               \
      } else {                                                          \
        LOOP {                                                          \
        /* mono -> mono */                                              \
          WRITEMONOSAMP(READMONOSAMP);                                  \
        }                                                               \
      }                                                                 \
    }                                                                   \
  }                                                                     \
}

void csSoundSourceSoftware::WriteBuffer(const void *Source, void *Dest,
        long NumSamples)
{
  csSoundFormat outfmt;
  outfmt.Freq = SoundRender->getFrequency();
  outfmt.Bits = SoundRender->is16Bits() ? 16 : 8;
  outfmt.Channels = SoundRender->isStereo() ? 2 : 1;

  const csSoundFormat *InputFormat = SoundHandle->Data->GetFormat();
  const csSoundFormat *OutputFormat = &outfmt;

  if (OutputFormat->Bits == 16)
  {
    #define stype short
    #define NullSample 0
    ADDTOBUFFER_BITS
    #undef stype
    #undef NullSample
  }
  else
  {
    #define stype unsigned char
    #define NullSample 128
    ADDTOBUFFER_BITS
    #undef stype
    #undef NullSample
  }
}

void csSoundSourceSoftware::AddToBufferStatic(void *mem, long size)
{
  iSoundData *snd = SoundHandle->Data;
  if (!snd) return;

  long InBPS = (snd->GetFormat()->Bits/8) * (snd->GetFormat()->Channels);
  long OutBPS = (SoundRender->is16Bits() ? 2 : 1) *
                (SoundRender->isStereo() ? 2 : 1);
  long NumSamples = size / OutBPS;

  if (snd->IsStatic ())
    while (1)
    {
      long Num = NumSamples;

      if (SoundPos + Num > snd->GetStaticSampleCount())
	Num = snd->GetStaticSampleCount() - SoundPos;
      unsigned char *Input = (unsigned char*)snd->GetStaticData();

      WriteBuffer(Input + SoundPos * InBPS, mem, Num);
      SoundPos += Num;

      NumSamples -= Num;
      mem = ((unsigned char *)mem) + Num * OutBPS;

      if (NumSamples == 0) break;
      if (!(PlayMethod & SOUND_LOOP)) break;
      Restart();
    }
  else // streamed sound
  {
    long wait = 0;
    while (1)
    {
      long Num = NumSamples;

      void *Buffer = snd->ReadStreamed (Num);
      if (Num)
      {
	WriteBuffer(Buffer, mem, Num);
	NumSamples -= Num;
	mem = ((unsigned char *)mem) + Num * OutBPS;
      }
      if (NumSamples == 0) break;

      if (Num == 0)
      {
	if (wait == 0)
	{
	  Restart();
	  wait++;
	}
	else
	  break;
      }
    }
  }
}

