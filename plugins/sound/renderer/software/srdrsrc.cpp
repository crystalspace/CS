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

#include "csgeom/vector3.h"
#include "csutil/scopedmutexlock.h"

#include "isound/listener.h"
#include "ivaria/reporter.h"
#include "isound/data.h"

#include "srdrsrc.h"
#include "srdrcom.h"
#include "sndhdl.h"
#include "csqsqrt.h"


SCF_IMPLEMENT_IBASE(csSoundSourceSoftware)
  SCF_IMPLEMENTS_INTERFACE(iSoundSource)
SCF_IMPLEMENT_IBASE_END

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
  SampleOffset=0.0f;
  mutex_RenderLock=csMutex::Create(true);
  SetMinimumDistance(1.0f);
  SetMaximumDistance(SOUND_DISTANCE_INFINITE);

  SoundHandle->IncSourceCount();
}

csSoundSourceSoftware::~csSoundSourceSoftware()
{
  Stop();
  SoundHandle->DecSourceCount();
  SCF_DESTRUCT_IBASE();
}

void csSoundSourceSoftware::Play(unsigned long pMethod)
{

  PlayMethod=pMethod;
  if (!Active){ Active=true; SoundRender->AddSource(this); }
  if (PlayMethod & SOUND_RESTART) Restart();
}

void csSoundSourceSoftware::Stop()
{
  if (Active)
  {
    Active=false;
    SoundRender->RemoveSource (this); /* Must be last statement, as it may 
					 cause destruction of this instance! */
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

void csSoundSourceSoftware::SetMinimumDistance (float distance)
{
  MinimumDistance = distance;
  if (MinimumDistance < 0.000001f)
    MinimumDistance=0.000001f;
}

void csSoundSourceSoftware::SetMaximumDistance (float distance)
{
  MaximumDistance = distance;
  if (MaximumDistance != SOUND_DISTANCE_INFINITE && 
      MaximumDistance < MinimumDistance)
      MaximumDistance=0.0f; // Don't play at all
}

float csSoundSourceSoftware::GetMinimumDistance ()
{
  return MinimumDistance;
}

float csSoundSourceSoftware::GetMaximumDistance ()
{
  return MaximumDistance;
}


void csSoundSourceSoftware::Prepare(float BaseVolume)
{
  float LeftScale,RightScale;
  csVector3 Front,Top,Left;
  csVector3 LeftToSound,RightToSound;

  csScopedMutexLock lock(mutex_RenderLock);

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
    Left.Set(1.0f,0.0f,0.0f);
  }
  else
  {
    // calculate the 'left' vector
    Listener->GetDirection(Front,Top);
    // Use the proper coordinate system
    Left=Front%Top;
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

  LeftScale=1.0f;
  RightScale=1.0f;
  LeftToSound=Position-EarL;
  RightToSound=Position-EarR;
  DistL=LeftToSound.Norm();
  DistR=RightToSound.Norm();

  // Early out if we're beyond the maximum distance
  if (MaximumDistance != SOUND_DISTANCE_INFINITE)
  {
    if (DistL > MaximumDistance && DistR > MaximumDistance)
    {
      CalcVolL = CalcVolR = 0;
      return;
    }
  }

  // Normalize the Ear-to-sound vectors used for direction factor calculations
  LeftToSound.Normalize();
  RightToSound.Normalize();


  // Approximate the effect of direction as a 60 percent variance in volume
  LeftScale = 0.60f * (LeftToSound * Left);
  if (LeftScale < 0.0f)
    LeftScale=0.0f;
  RightScale = 0.60f * (RightToSound * (-Left));
  if (RightScale < 0.0f)
    RightScale = 0.0f;
  LeftScale+=0.40f;
  RightScale+=0.40f;


  // Turn distances into units based off minimum distance
  float minimum_distance=MinimumDistance;
  if (minimum_distance < 0.000001f)
    minimum_distance=0.000001f;
  DistL/=minimum_distance;
  DistR/=minimum_distance;

  // The minimum distance is, of course, 1 minimum distance unit
  if (DistL<1.0f) DistL=1.0f;
  if (DistR<1.0f) DistR=1.0f;

  // The rolloff factor is applied as a factor to the natural rolloff power of 2.0
  float RolloffFactor=Listener->GetRollOffFactor() * 2.0f;

  // Apply the rolloff factor to the volume
  CalcVolL/=pow(DistL,RolloffFactor);
  CalcVolR/=pow(DistR,RolloffFactor);

  // Finally apply the directional scaling to each ear
  CalcVolL*=LeftScale;
  CalcVolR*=RightScale;
}

/* helper macros */
#define READMONOSAMP        ((int)(Input[i])-NullSample)
#define READLEFTSAMP        ((int)(Input[2*i])-NullSample)
#define READRIGHTSAMP       ((int)(Input[2*i+1])-NullSample)

#define CALCMONOSAMP(x)     ((x)*(CalcVolL+CalcVolR)/2)
#define CALCLEFTSAMP(x)     ((x)*CalcVolL)
#define CALCRIGHTSAMP(x)    ((x)*CalcVolR)

#define ADDCHECK(x, y)      {int q = (int)x+int(y); \
				if(q<MINSAMPLE) x = (stype)MINSAMPLE; \
				else if(q>MAXSAMPLE) x = (stype)MAXSAMPLE; \
				else x = (stype)(q); }

#define WRITEMONOSAMP(x)    ADDCHECK(Output[i], CALCMONOSAMP(x))
#define WRITELEFTSAMP(x)    ADDCHECK(Output[2*i], CALCLEFTSAMP(x))
#define WRITERIGHTSAMP(x)   ADDCHECK(Output[2*i+1], CALCRIGHTSAMP(x))

//unclamped output macros
//#define WRITEMONOSAMP(x)    Output[i]+=(stype)((x)*(CalcVolL+CalcVolR)/2);
//#define WRITELEFTSAMP(x)    Output[2*i]+=(stype)((x)*CalcVolL);
//#define WRITERIGHTSAMP(x)   Output[2*i+1]+=(stype)((x)*CalcVolR);

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
    #define MINSAMPLE (-32700)
    #define MAXSAMPLE 32700
    ADDTOBUFFER_BITS
    #undef MINSAMPLE
    #undef MAXSAMPLE
    #undef stype
    #undef NullSample
  }
  else
  {
    #define stype unsigned char
    #define NullSample 128
    #define MINSAMPLE 0x00
    #define MAXSAMPLE 0xFF
    ADDTOBUFFER_BITS
    #undef MINSAMPLE
    #undef MAXSAMPLE
    #undef stype
    #undef NullSample
  }
}

void csSoundSourceSoftware::AddToBufferStatic(void *mem, long size)
{
  csScopedMutexLock lock(mutex_RenderLock);

  iSoundData *snd = SoundHandle->Data;
  if (!snd) return;

  // Process any stream reset that may have been queued
  SoundHandle->ProcessReset();

  long InBPS = (snd->GetFormat()->Bits/8) * (snd->GetFormat()->Channels);
  long OutBPS = (SoundRender->is16Bits() ? 2 : 1) *
                (SoundRender->isStereo() ? 2 : 1);
  long NumSamples = size / OutBPS;

  if (snd->IsStatic ())
    if (CalcFreqFactor == 1.0 || CalcFreqFactor <= 0.0)
    {
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
      if (!(PlayMethod & SOUND_LOOP))
      {
        Active = (SoundPos  < snd->GetStaticSampleCount()); 
        break;
      }
      Restart();
      }
    }
    else
    {
      /* Frequency shifted sound.
       *  
       *  The concept here is that a normal step size is 1 sample.
       *  The Frequency Factor can be thought of as a multiplier to
       *  the step size, so that the real step size is 1*FreqFactor
       *  or just FreqFactor.
       *  Any given step will either fall directly on a source sample
       *  or part way between two samples.
       *  In the first case, we simply use the source sample.
       *  In the other case, we read the two samples surrounding this
       *  step and calculate what value the step would have if the
       *  sample values were linearly connected.
       * 
       *         B=1.0
       *        /
       *       /
       *      Step=.7
       *     /
       *    A=.5
       */
      int channels=snd->GetFormat()->Channels;
      int bits=snd->GetFormat()->Bits;
      long WriteSample=0;
      long SampleCount=snd->GetStaticSampleCount();

      if (bits==8)
      {
        unsigned char InterpolatedData[2];
        unsigned char *Input = (unsigned char*)snd->GetStaticData();
        /* This loop has to move 1 sample at a time to perform 
        *  linear interpolation between actual data points. 
        */
        while (WriteSample<NumSamples)
        {
          // Each channel must be calculated separately
          for (int channel_num=0;channel_num<channels;channel_num++)
          {
            if (SampleOffset==0.0f || SoundPos>=SampleCount)
            {
              /* If this sample falls exactly on a source sample
              *  or if it falls after the last sample, use only
              *  one sample.
              */
              InterpolatedData[channel_num]=Input[SoundPos*channels+channel_num];
            }
            else
            {
              /* If this sample point falls betwee two source points
              *  perform a linear interpolation between the two points 
              *  to estimate this value.
              */
              InterpolatedData[channel_num]=(unsigned char)((float)(Input[(SoundPos+1)*channels+channel_num]-Input[SoundPos*channels+channel_num])*SampleOffset)+Input[SoundPos*channels+channel_num];
            }


          }

          WriteBuffer(InterpolatedData,mem,1);
          mem = ((unsigned char *)mem) + OutBPS;

          // SampleOffset may be >= 1.0 after this addition
          SampleOffset+=CalcFreqFactor;
          /* SoundPos is an integer value, so only the integer portion
          *  of SampleOffset is added here.
          */
          SoundPos+=(int)SampleOffset;
          /* By subtracting the integer portion of SampleOffset
          *  from the floating point number, we are left with
          *  0.0 <= SampleOffset < 1.0 which we can use in the
          *  linear calculation in the next write cycle.
          */
          SampleOffset=SampleOffset -(int)SampleOffset;

          // Advance the write sample as well
          WriteSample++;

          /* If we are past the end of the static buffer
          *  we are either done, or need to restart at
          *  the begining.
          */
          if (SoundPos>SampleCount)
          {
            if (!(PlayMethod & SOUND_LOOP))
            {
              Active = (SoundPos  < snd->GetStaticSampleCount()); 
              break;
            }
            else
            {
              SoundPos%=SampleCount;
              Restart();
            }
          } 
        }
      }
      else
      {
        short InterpolatedData[2];
        short *Input = (short *)snd->GetStaticData();
        /* This loop has to move 1 sample at a time to perform 
        *  linear interpolation between actual data points. 
        */
        while (WriteSample<NumSamples)
        {
          // Each channel must be calculated separately
          for (int channel_num=0;channel_num<channels;channel_num++)
          {
            if (SampleOffset==0.0f || SoundPos>=SampleCount)
            {
              /* If this sample falls exactly on a source sample
              *  or if it falls after the last sample, use only
              *  one sample.
              */
              InterpolatedData[channel_num]=Input[SoundPos*channels+channel_num];
            }
            else
            {
              /* If this sample point falls betwee two source points
              *  perform a linear interpolation between the two points 
              *  to estimate this value.
              */
              InterpolatedData[channel_num]=(short)((float)(Input[(SoundPos+1)*channels+channel_num]-Input[SoundPos*channels+channel_num])*SampleOffset)+Input[SoundPos*channels+channel_num];
            }
          }
          WriteBuffer(InterpolatedData,mem,1);
          mem = ((unsigned char *)mem) + OutBPS;

          // SampleOffset may be >= 1.0 after this addition
          SampleOffset+=CalcFreqFactor;
          /* SoundPos is an integer value, so only the integer portion
          *  of SampleOffset is added here.
          */
          SoundPos+=(int)SampleOffset;
          /* By subtracting the integer portion of SampleOffset
          *  from the floating point number, we are left with
          *  0.0 <= SampleOffset < 1.0 which we can use in the
          *  linear calculation in the next write cycle.
          */
          SampleOffset=SampleOffset -(int)SampleOffset;

          // Advance the write sample as well
          WriteSample++;

          /* If we are past the end of the static buffer
          *  we are either done, or need to restart at
          *  the begining.
          */
          if (SoundPos>SampleCount)
          {
            if (!(PlayMethod & SOUND_LOOP))
            {
              Active = (SoundPos  < snd->GetStaticSampleCount()); 
              break;
            }
            else
            {
              SoundPos%=SampleCount;
              Restart();
            }
          } 
        }
      }
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
        if (!SoundHandle->LoopStream)
        {
          Active = false;
          break;
        }
        if (wait == 0)
        {
          Restart();
	  SoundHandle->ProcessReset(); 
          wait++;
        }
        else
          break;
      }
    }
  }
}

