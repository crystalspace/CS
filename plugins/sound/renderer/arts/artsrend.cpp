/*
    Copyright (C) 2001 by Norman Krämer
  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.
  
    You should have received a copy of the GNU General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <soundserver.h>
#include "cssysdef.h"
#include "artsrend.h"
#include "isys/system.h"

#define ARTS_SIMPLESOUNDSERVER "global:Arts_SimpleSoundServer"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csArtsRenderer)
  SCF_IMPLEMENTS_INTERFACE (iSoundRender)
  SCF_IMPLEMENTS_INTERFACE (iPlugIn)
  SCF_IMPLEMENTS_INTERFACE (iSoundListener)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csArtsRenderer);

SCF_EXPORT_CLASS_TABLE (csarts)
  SCF_EXPORT_CLASS (csArtsRenderer, "crystalspace.sound.render.arts", 
		"aRts renderer plugin for Crystal Space")
SCF_EXPORT_CLASS_TABLE_END

csArtsRenderer::csArtsRenderer (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  dispatcher = NULL;
  bInit = false;
  SetVolume (1.0f);
  SetEnvironment (ENVIRONMENT_GENERIC);
  SetPosition (csVector3 (0.0f, 0.0f, 1.0f));
  SetDirection (csVector3 (0.0f, 1.0f, 0.0f), csVector3 (0.0f, 0.0f, 1.0f));
  SetVelocity (csVector3 (1.0f, 1.0f, 1.0f));
  SetHeadSize (1.0f);
}

csArtsRenderer::~csArtsRenderer ()
{
  vObject.DeleteAll ();
  if (dispatcher) delete dispatcher;
}

bool csArtsRenderer::Initialize (iSystem *iSys)
{
  // ok, we try to get a remote soundserver object
  dispatcher = new Arts::Dispatcher;
  server = Arts::Reference (ARTS_SIMPLESOUNDSERVER);
  if (server.isNull ())
  {
    iSys->Printf (CS_MSG_WARNING, "Couldn't get a reference to the soundserver !\n");
    iSys->Printf (CS_MSG_WARNING, 
		  "Check whether you have the aRts server running (usually called \"artsd\")\n");
  }
  else
  {
    bInit = true;
  }
  return bInit;
}

Arts::csSoundModule *csArtsRenderer::CreateArtsModule ()
{
  Arts::csSoundModule *sm = new Arts::csSoundModule;
  *sm = Arts::DynamicCast (server.createObject ("Arts::csSoundModule"));
  if (sm->isNull ())
  {
    delete sm;
    return NULL;
  }
  return sm;
}

void csArtsRenderer::SetVolume (float vol)
{
  if (bInit)
  {
    /// Skip through all the soundobjects we handed out and set the volume for each.
    /// Note that we could just insert a stereovolume control on the servers effect stack.
    /// The upside would be easier handling here, the downside is we would modify the 
    /// volume of other sounds too, like the napstered mp3 playing in the background.

    for (int i=0; i<vObject.Length (); i++)
      vObject.Get(i)->SetVolume (vol);

  }
  volume = vol;
}

float csArtsRenderer::GetVolume ()
{
  return volume;
}

iSoundHandle *csArtsRenderer::RegisterSound(iSoundData *sd)
{
  csArtsHandle *sh = new csArtsHandle (this);
  if (sh->UseData (sd))
  {
    vObject.InsertSorted (sh);
    sh->SetVolume (volume);
    return sh;
  }
  else
    delete sh;
  return NULL;
}

void csArtsRenderer::UnregisterSound(iSoundHandle *sh)
{
  int idx = vObject.FindSortedKey ((csConstSome)sh);
  if (idx != -1)
    vObject.Delete (idx);

}

iSoundSource *csArtsRenderer::CreateSource (csArtsHandle *pHandle, int Mode3D)
{
  csArtsHandle *sh = new csArtsHandle (this);
  if (sh->UseData (pHandle->sd))
  {
    vObject.InsertSorted (sh);
    sh->SetVolume (volume);
    sh->SetMode3D (Mode3D);
    return sh;
  }
  else
    delete sh;
  return NULL;
}

iSoundListener *csArtsRenderer::GetListener ()
{
  return this;
}

void csArtsRenderer::SetDirection (const csVector3 &Front, const csVector3 &Top)
{
  if (bInit)
  {
    for (int i=0; i<vObject.Length (); i++)
      vObject.Get(i)->SetDirection (Front, Top);

  }
  front = Front;
  top = Top;
}

void csArtsRenderer::SetPosition (const csVector3 &pos)
{
  if (bInit)
  {
    for (int i=0; i<vObject.Length (); i++)
      vObject.Get(i)->SetPosition (pos);

  }
  this->pos = pos;
}

void csArtsRenderer::SetVelocity (const csVector3 &v)
{
  if (bInit)
  {
    for (int i=0; i<vObject.Length (); i++)
      vObject.Get(i)->SetVelocity (v);

  }
  velocity = v;
}

void csArtsRenderer::SetDistanceFactor (float factor)
{
  if (bInit)
  {
    for (int i=0; i<vObject.Length (); i++)
      vObject.Get(i)->SetDistanceFactor (factor);
  }
  distanceFactor = factor;
}

void csArtsRenderer::SetRollOffFactor (float factor)
{
  if (bInit)
  {
    for (int i=0; i<vObject.Length (); i++)
      vObject.Get(i)->SetRollOffFactor (factor);
  }
  rolloffFactor = factor;
}

void csArtsRenderer::SetDopplerFactor (float factor)
{
  if (bInit)
  {
    for (int i=0; i<vObject.Length (); i++)
      vObject.Get(i)->SetDopplerFactor (factor);
  }
  dopplerFactor = factor;
}

void csArtsRenderer::SetHeadSize (float size)
{
  if (bInit)
  {
    for (int i=0; i<vObject.Length (); i++)
      vObject.Get(i)->SetHeadSize (size);
  }
  headSize = size;
}

void csArtsRenderer::SetEnvironment (csSoundEnvironment env)
{
  int n = (int)env;
  static struct {
    float distFact;
    float roffFact;
    float doppFact;
  } enviro[] = {
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_GENERIC
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_PADDEDCELL
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_ROOM
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_BATHROOM
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_LIVINGROOM
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_STONEROOM
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_AUDITORIUM
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_CONCERTHALL
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_CAVE
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_ARENA
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_CARPETEDHALLWAY
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_HALLWAY
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_STONECORRIDOR
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_ALLEY
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_FOREST
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_CITY
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_MOUNTAINS
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_QUARRY
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_PLAIN
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_PARKINGLOT
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_SEWERPIPE
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_UNDERWATER
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_DRUGGED
    {1.0f, 1.0f, 1.0f}, // ENVIRONMENT_DIZZY
    {1.0f, 1.0f, 1.0f} // ENVIRONMENT_PSYCHOTIC
  };

  SetDistanceFactor (enviro[n].distFact);
  SetRollOffFactor (enviro[n].roffFact);
  SetDopplerFactor (enviro[n].doppFact);
  environment = env;
}

