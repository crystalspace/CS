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
#include "csutil/scf.h"
#include "srdrsrc.h"
#include "srdrcom.h"
#include "isystem.h"
#include "isndlstn.h"
#include "isnddata.h"

IMPLEMENT_IBASE(csSoundSourceSoftware)
  IMPLEMENTS_INTERFACE(iSoundSource)
IMPLEMENT_IBASE_END;

csSoundSourceSoftware::csSoundSourceSoftware(iBase *piBase, bool snd3d,
    csSoundRenderSoftware *srdr, iSoundData *sndData)
{
  CONSTRUCT_IBASE(piBase);

  SoundRender=srdr;
  FrequencyFactor=1;
  Volume=1.0;
  Sound3d=snd3d;
  Position=csVector3(0,0,0);
  Velocity=csVector3(0,0,0);
  Active=false;
  SoundStream=sndData->CreateStream();
}

csSoundSourceSoftware::~csSoundSourceSoftware()
{
  Stop();
  SoundStream->DecRef();
}

void csSoundSourceSoftware::Play(unsigned long pMethod)
{
  PlayMethod=pMethod;
  if (!Active) SoundRender->AddSource(this);
  Active=true;
  if (PlayMethod & SOUND_RESTART) SoundStream->Reset();
}

void csSoundSourceSoftware::Stop() {
  if (Active) {
    Active=false;
    SoundRender->RemoveSource(this);
  }
}

bool csSoundSourceSoftware::IsActive() {
  return Active;
}

void csSoundSourceSoftware::SetVolume(float vol) {
  Volume = vol;
}

float csSoundSourceSoftware::GetVolume() {
  return Volume;
}

void csSoundSourceSoftware::SetFrequencyFactor(float factor) {
  FrequencyFactor = factor;
}

float csSoundSourceSoftware::GetFrequencyFactor() {
  return FrequencyFactor;
}

bool csSoundSourceSoftware::Is3d() {
  return Sound3d;
}

void csSoundSourceSoftware::SetPosition(csVector3 pos) {
  Position=pos;
}

csVector3 csSoundSourceSoftware::GetPosition() {
  return Position;
}

void csSoundSourceSoftware::SetVelocity(csVector3 v) {
  Velocity=v;
}

csVector3 csSoundSourceSoftware::GetVelocity() {
  return Velocity;
}

void csSoundSourceSoftware::Prepare(unsigned long VolDiv) {
  // frequency
  CalcFreqFactor=FrequencyFactor;

  // volume
  CalcVolL=CalcVolR=Volume/VolDiv;

  // for non-3d sources we don't calculate anything else
  if (!Is3d()) return;

  // get the global listener object
  iSoundListener *Listener=SoundRender->GetListener();
  float DistFactor=Listener->GetDistanceFactor();
    
  // calculate the 'left' vector
  csVector3 Front,Top,Left;
  Listener->GetDirection(Front,Top);
  Left=Top%Front;

  // calculate ear position
  csVector3 EarL,EarR;
  EarL=Listener->GetPosition()+Left*Listener->GetHeadSize()/2;
  EarR=Listener->GetPosition()-Left*Listener->GetHeadSize()/2;

  // calculate ear distance
  float DistL,DistR;
  DistL=(EarL-Position).Norm();
  DistR=(EarR-Position).Norm();

  // the following tests should not be deleted until we see (or hear)
  // what gives the best results. Just try it!
/*
  // @@@ test: amplify difference between ears.
  float d=DistL-DistR;
  DistL+=d;
  DistR-=d;
*/
  // @@@ test: take square root of distance. Otherwise the missile sound
  // in walktest seems to become distant too fast.
  DistL=sqrt(DistL);
  DistR=sqrt(DistR);

  // prevent too near sounds
  if (DistL<1) DistL=1;
  if (DistR<1) DistR=1;

  // calculate ear volume
  CalcVolL/=DistL*DistFactor;
  CalcVolR/=DistR*DistFactor;
}

/* helper macros */
#define MONOSAMP        ((int)(Data[i]))
#define LEFTSAMP        ((int)(Data[2*i]))
#define RIGHTSAMP       ((int)(Data[2*i+1]))
#define READMONO3D      int samp=MONOSAMP;
#define READSTEREO3D    int samp=(LEFTSAMP+RIGHTSAMP)/2;
#define WRITEMONO3D     Buffer[i]+=(stype)((samp AddModify)*(CalcVolL+CalcVolR)/2);
#define WRITESTEREO3D   {                   \
  Buffer[2*i]+=(stype)((samp AddModify)*CalcVolL);   \
  Buffer[2*i+1]=(stype)((samp AddModify)*CalcVolR);  \
}

#define LOOP        for (unsigned long i=0;i<NumSamples;i++)

#define ADDTOBUFFER_BITS {                                                  \
  stype *Buffer=(stype*)Memory;                                             \
  unsigned long RemainingSamples=MemSize/(sizeof(stype));                   \
  if (SoundRender->isStereo()) RemainingSamples/=2;                         \
                                                                            \
  while (1) {                                                               \
    unsigned long NumSamples=RemainingSamples;                              \
    stype *Data=(stype*)(SoundStream->Read(NumSamples));                    \
    if (Is3d()) {                                                           \
      /* handle 3d sound using the above macros */                          \
      if (Format->Channels==2) {                                            \
        if (SoundRender->isStereo()) {                                      \
          LOOP {READSTEREO3D; WRITESTEREO3D;}                               \
        } else {                                                            \
          LOOP {READSTEREO3D; WRITEMONO3D;}                                 \
        }                                                                   \
      } else {                                                              \
        if (SoundRender->isStereo()) {                                      \
          LOOP {READMONO3D; WRITESTEREO3D;}                                 \
        } else {                                                            \
          LOOP {READMONO3D; WRITEMONO3D;}                                   \
        }                                                                   \
      }                                                                     \
    } else {                                                                \
      /* handle non-3d sound (use CalcVolL for volume, NOT 'Volume', */     \
      /* to use any precalculation done in Prepare()                 */     \
      if (Format->Channels==2) {                                            \
        if (SoundRender->isStereo()) {                                      \
          /* stereo -> stereo */                                            \
          LOOP {                                                            \
            Buffer[2*i]+= (stype)CalcVolL*(LEFTSAMP AddModify);             \
            Buffer[2*i+1]+= (stype)CalcVolL*(RIGHTSAMP AddModify);          \
          }                                                                 \
        } else {                                                            \
          /* stereo -> mono */                                              \
          LOOP {                                                            \
            Buffer[i]+=(stype)CalcVolL*((LEFTSAMP + RIGHTSAMP)/2 AddModify);       \
          }                                                                 \
        }                                                                   \
      } else {                                                              \
        if (SoundRender->isStereo()) {                                      \
          /* mono -> stereo */                                              \
          LOOP {                                                            \
            Buffer[2*i]+=(stype)CalcVolL*(MONOSAMP AddModify);                     \
            Buffer[2*i+1]+=(stype)CalcVolL*(MONOSAMP AddModify);                   \
          }                                                                 \
        } else {                                                            \
          /* mono -> mono */                                                \
          LOOP {Buffer[i]+=(stype)CalcVolL*(MONOSAMP AddModify);}                  \
        }                                                                   \
      }                                                                     \
    }                                                                       \
    SoundStream->DiscardBuffer(Data);                                       \
    RemainingSamples-=NumSamples;                                           \
    if (RemainingSamples==0) break;                                         \
    else {                                                                  \
      if (PlayMethod & SOUND_LOOP) SoundStream->Reset();                    \
      else {Active=false;return;}                                           \
    }                                                                       \
  }                                                                         \
}

void csSoundSourceSoftware::AddToBuffer(void *Memory, unsigned long MemSize) {
  if (!Active) return;

  const csSoundFormat *Format=SoundStream->GetSoundData()->GetFormat();

  if (SoundRender->is16Bits()) {
    #define stype short
    #define AddModify
    ADDTOBUFFER_BITS
    #undef stype
    #undef AddModify
  } else {
    #define stype unsigned char
    #define AddModify -128
    ADDTOBUFFER_BITS
    #undef stype
    #undef AddModify
  }
  
}
