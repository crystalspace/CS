#include <stdio.h>
#include <soundserver.h>
#include <connect.h>
#include "csarts_impl.h"
#include <factory.h>
#include <convert.h>

using namespace Arts;
#define EPSILON 0.000001

csSoundSource_impl::csSoundSource_impl () 
{
  pos = 0;
  leftsrc = rightsrc = NULL;
  bLoop = false;
}

csSoundSource_impl::~csSoundSource_impl () 
{
  free (leftsrc);
  free (rightsrc);
}
  
void csSoundSource_impl::calculateBlock (unsigned long samples)
{
  unsigned long i, to = myMin(data.size()-2*pos, 2*samples) / 2;
  //  printf("%ld samples requested\n", samples);
  for (i=0; i < to; i++, pos++)
  {
    left [i] = leftsrc[pos];
    right [i] = rightsrc[pos];
  }
  for (;i < samples; i++)
    left [i] = right [i] = 0.0f;
  if (bLoop && pos*2 == data.size ())
    pos = 0;
}
  
std::vector<float> * csSoundSource_impl::Data()
{
  return &data;
}

void csSoundSource_impl::Play (long playtype)
{
  if (playtype == 1)
    pos = 0;

  bLoop = playtype == 2;
}

void csSoundSource_impl::Data(const std::vector<float>& newValue)
{
  data = newValue;
  short *from = (short *)malloc (data.size () * sizeof(short));
  for (unsigned long i=0; i < data.size (); i++)
    from [i] = (short) data[i];
  
  free (leftsrc);
  free (rightsrc);
  leftsrc = (float *)malloc (sizeof(float) * data.size () / 2 );
  rightsrc = (float *)malloc (sizeof(float) * data.size () / 2);
  uni_convert_stereo_2float ( data.size()/2, 
			      (unsigned char*)from, 
			      data.size () * sizeof(short),
			      2,
			      16,
			      leftsrc, rightsrc,
			      1.0,
			      0.0);
  free (from);
}

void cs3DEffect_impl::calculateBlock (unsigned long samples)
{
  recalcVolume ();
  for (unsigned long i=0; i < samples; i++)
  {
    outright[i] = volRight * inright[i];
    outleft[i] = volLeft * inleft[i];
  }
}

void cs3DEffect_impl::recalcVolume ()
{
  if (recalc < 4)
  {
    // position of the listener's ears

    if (recalc < 2)
    {
      if (mode == SOUND3D_DISABLE) 
      {
	volRight = volLeft = 1.0;
	recalc = 4;
	return;
      }
      if (mode == SOUND3D_RELATIVE) 
      {
	// position of the sound is relative to the listener, so we simply
	// place the listener at (0,0,0) with front (0,0,1) and top (0,1,0)
	EarLy = EarLz = EarRy = EarRz = 0.0f;
	EarRx = headSize;
	EarLx = -EarRx;
      } 
      else 
      {
	// calculate the 'left' vector: left = upVector x frontVector
	float leftX, leftY, leftZ, n;
	leftX = upY*frontZ-upZ*frontY;
	leftY = upZ*frontX-upX*frontZ;
	leftZ = upX*frontY-upY*frontX;

	if ( (n=Norm (leftX, leftY, leftZ))<EPSILON) 
	{
	  // user has supplied bad front and top vectors
	  volRight = volLeft = 0.0f;
	  recalc = 4;
	  return;
	} 
	else 
	{
	  leftX /=n;
	  leftY /=n;
	  leftZ /=n;
	}
    
	// calculate ear position
	EarLx=leftX*headSize;
	EarLy=leftY*headSize;
	EarLz=leftZ*headSize;
	EarRx=-EarLx;
	EarRy=-EarLy;
	EarRz=-EarLz;

	EarLx+=lposX;
	EarRx+=lposX;
	EarLy+=lposY;
	EarRy+=lposY;
	EarLz+=lposZ;
	EarRz+=lposZ;
      }
    }

    if (recalc < 3)
    {
      // calculate ear distance
      DistL=Norm (EarLx-posX, EarLy-posY, EarLz-posZ);
      DistR=Norm (EarRx-posX, EarRy-posY, EarRz-posZ);

      // prevent too near sounds
      if (DistL<1) DistL=1;
      if (DistR<1) DistR=1;
    }

    if (recalc < 4)
    {
      // calculate ear volume
      volLeft  = 1.0f/(DistL*DistFactor);
      volRight = 1.0f/(DistR*DistFactor);
    }
    recalc = 4;
  }
}

float cs3DEffect_impl::Norm (float x, float y, float z)
{
  return sqrt (x*x + y*y + z*z);
}

//////////////////////// csSoundModule /////////////////////////////////////

csSoundModule_impl::csSoundModule_impl ()
{
  bOK = true;
  bPlaying = false;

  // create the object we'll need
  server = Arts::Reference ("global:Arts_SimpleSoundServer");
  if (server.isNull ())
  {
    fprintf (stderr, "arts server null\n");
    bOK = false;
  }
  else
  {
    // ok, since this soundmodule instance is created on the server we actually could use the
    // usual way to create instances, but, oh well, since we are so dynamic, we'll do that dynamically too
    play = Arts::DynamicCast (server.createObject ("Arts::Synth_AMAN_PLAY"));
    bOK = !play.isNull();
    if (!bOK)
      fprintf (stderr, "aman_play null\n");
    else
    {
      play.title ("play");
      play.autoRestoreID ("play");

      ss = Arts::DynamicCast (server.createObject ("Arts::csSoundSource"));
      bOK = !ss.isNull();
      if (!bOK)
	fprintf (stderr, "sound source null\n");
      else
      {
	effect = Arts::DynamicCast (server.createObject ("Arts::cs3DEffect"));
	bOK = !effect.isNull();
	if (!bOK)
	  fprintf (stderr, "3d effect null\n");
	else
	{
	  volControl = Arts::DynamicCast (server.createObject ("Arts::StereoVolumeControl"));
	  bOK = !volControl.isNull();
	  if (!bOK)
	    fprintf (stderr, "volume control null\n");
	}
      }
    }
  }
}

void csSoundModule_impl::SetData (const std::vector<float>&  Data)
{
  if (bOK)
    ss.Data (Data);
}

void csSoundModule_impl::SetHeadSize (float headSize)
{
  if (bOK)
    effect.SetHeadSize (headSize);
}

void csSoundModule_impl::SetOrientation (float upX, float upY, float upZ, 
					 float frontX, float frontY, float frontZ)
{
  if (bOK)
    effect.SetOrientation (upX, upY, upZ, frontX, frontY, frontZ);
}

void csSoundModule_impl::SetListenerPosition (float posX, float posY, float posZ)
{
  if (bOK)
    effect.SetListenerPosition (posX, posY, posZ);
}

void csSoundModule_impl::SetSoundPosition (float posX, float posY, float posZ)
{
  if (bOK)
    effect.SetSoundPosition (posX, posY, posZ);
}

void csSoundModule_impl::SetDistanceFactor (float fact)
{
  if (bOK)
    effect.SetDistanceFactor (fact);
}

void csSoundModule_impl::Set3DType (Sound3DType mode)
{
  if (bOK)
    effect.Set3DType (mode);
}

void csSoundModule_impl::SetVelocity (float x, float y, float z)
{
  if (bOK)
    effect.SetVelocity (x, y, z);
}

void csSoundModule_impl::SetRollOffFactor (float factor)
{
  if (bOK)
    effect.SetRollOffFactor (factor);
}

void csSoundModule_impl::SetDopplerFactor (float factor)
{
  if (bOK)
    effect.SetDopplerFactor (factor);
}

void csSoundModule_impl::SetVolume (float vol)
{
  if (bOK)
    volControl.scaleFactor (vol);
}

void csSoundModule_impl::SetFrequencyFactor (float freq)
{
  (void)freq;
}

void csSoundModule_impl::Play (long playType)
{
  if (bOK)
  {
    ss.Play (playType);
    if (!bPlaying)
      start ();
  }
}

void csSoundModule_impl::Stop ()
{
  if (bOK && bPlaying)
    stop ();
}

void csSoundModule_impl::streamStart ()
{
  if (bOK)
  {
    bPlaying = true;
    ss.start ();
    effect.start ();
    volControl.start ();
    play.start();
  }
}

void csSoundModule_impl::streamInit ()
{
  if (bOK)
  {
    connect (ss, effect);
    connect (effect, volControl);
    connect (volControl, play);
  }
}

void csSoundModule_impl::streamEnd ()
{
  if (bOK)
  {
    bPlaying = false;
    play.stop ();
    volControl.stop ();
    effect.stop ();
    ss.stop ();
  }
}


REGISTER_IMPLEMENTATION (csSoundSource_impl);
REGISTER_IMPLEMENTATION (cs3DEffect_impl);
REGISTER_IMPLEMENTATION (csSoundModule_impl);
