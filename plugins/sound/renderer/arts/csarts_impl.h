#ifndef CS_ARTS_H_
#define CS_ARTS_H_

#include "csarts.h"
#include <stdsynthmodule.h>
#include <artsmodules.h>

namespace Arts{
class csSoundSource_impl : public csSoundSource_skel, public StdSynthModule
{
private:
  unsigned long pos;
  std::vector<float> data;
  unsigned long myMin (unsigned long x, unsigned long y)
  {return x<y?x:y;}
  float *leftsrc, *rightsrc;
  bool bLoop;
public:
  csSoundSource_impl ();
  virtual ~csSoundSource_impl ();

  void calculateBlock (unsigned long samples);
  virtual std::vector<float> * Data();
  virtual void Data(const std::vector<float>& newValue);
  void Play (long playtype);
};

class cs3DEffect_impl : public cs3DEffect_skel, public StdSynthModule
{
  float headSize;
  float upX, upY, upZ, frontX, frontY, frontZ, lposX, lposY, lposZ, posX, posY, posZ;
  float volLeft, volRight;
  float DistL,DistR, DistFactor;
  float EarLx, EarLy, EarLz, EarRx, EarRy, EarRz;
  Sound3DType mode;

  int recalc;
  void recalcVolume ();
  float Norm (float x, float y, float z);
 public:
  cs3DEffect_impl ()
  {
    headSize = 0.5f;
    upX = 0.0f;
    upY = 1.0f;
    upZ = 0.0f;
    frontX = 0.0f;
    frontY = 0.0f;
    frontZ = 1.0f;
    lposX = lposY = lposZ = 0.0f;
    posX = posY = 0.0f;
    posZ = 1.0f;
    volLeft = volRight = 1.0f;
    recalc = 1;
    mode = SOUND3D_DISABLE;
    DistFactor = 1.0f;
  }
  void calculateBlock (unsigned long samples);
  void SetHeadSize (float HeadSize)
  {
    headSize = HeadSize/2.f;
    recalc = 1;
  }
  void SetOrientation (float upX, float upY, float upZ, float frontX, float frontY, float frontZ)
  {
    this->upX = upX; this->upY = upY; this->upZ = upZ;
    this->frontX = frontX; this->frontY = frontY; this->frontZ = frontZ;
    recalc = 1;
  }
  void SetListenerPosition (float posX, float posY, float posZ)
  {
    lposX = posX; lposY = posY, lposZ = posZ;
    recalc = 1;
  }
  void SetSoundPosition (float posX, float posY, float posZ)
  {
    this->posX = posX; this->posY = posY, this->posZ = posZ;
    recalc = recalc > 2 ? 2 : recalc;
  }
  void SetDistanceFactor (float fact)
  {
    DistFactor = fact;
    recalc = recalc > 3 ? 3 : recalc;
  }
  void Set3DType (Sound3DType theMode)
  {
    mode = theMode;
    recalc = 1;
  }
  void SetVelocity (float , float , float){}
  void SetRollOffFactor (float ){}
  void SetDopplerFactor (float ){}
};

class csSoundModule_impl : public csSoundModule_skel, public StdSynthModule
{
 protected:
  csSoundSource ss;
  cs3DEffect effect;
  Synth_AMAN_PLAY play;
  Synth_FREEVERB reverb;
  StereoVolumeControl volControl;
  SimpleSoundServer server;
  bool bOK, bPlaying;

 public:
  csSoundModule_impl ();
  void SetData (const std::vector<float>& Data);
  void SetHeadSize (float headSize);
  void SetOrientation (float upX, float upY, float upZ, float frontX, float frontY, float frontZ);
  void SetListenerPosition (float posX, float posY, float posZ);
  void SetSoundPosition (float posX, float posY, float posZ);
  void SetDistanceFactor (float fact);
  void Set3DType (Sound3DType mode);
  void SetVolume (float vol);
  void SetFrequencyFactor (float freq);
  void SetVelocity (float , float , float);
  void SetRollOffFactor (float );
  void SetDopplerFactor (float );
  void Play (long playType);
  void Stop ();
  void streamStart ();
  void streamInit ();
  void streamEnd ();
};

};

#endif
