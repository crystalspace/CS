/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
   $Id$

    This file is part of Ter'Angreal, a 3D VR application
    using VOS and CrystalSpace from interreality.org

    Copyright (C) 2002 Peter Amstutz

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "cssysdef.h"

// Hack: Work around problems caused by #defining 'new'.
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# undef new
#endif
#include <new>

#include "voslight.h"
#include "iengine/engine.h"
#include "iengine/light.h"

using namespace VUtil;
using namespace VOS;

class ConstructLightTask : public Task
{
public:
  iObjectRegistry *object_reg;
  csRef<iSector> sector;
  vRef<csMetaLight> metalight;
  std::string name;
  csVector3 pos;
  csColor color;
  float radius;
  bool isStatic;
  csVosA3DL *vosa3dl;


  ConstructLightTask(iObjectRegistry *objreg, iSector *sector,
                     const std::string &n, csMetaLight *ml);
  virtual ~ConstructLightTask();
  virtual void doTask();
};

ConstructLightTask::ConstructLightTask(iObjectRegistry *objreg, iSector *s,
                                       const std::string &n, csMetaLight *ml)
  : object_reg(objreg), sector(s), metalight(ml, true), name(n)
{
}

ConstructLightTask::~ConstructLightTask()
{
}

void ConstructLightTask::doTask()
{
  LOG("ConstructLightTask", 3, "creating light");

  csRef<iEngine> engine = CS_QUERY_REGISTRY (object_reg, iEngine);

  csRef<iLight> light = engine->CreateLight(name.c_str(), pos,
                                            radius, color,
                                            (isStatic
                                             ? CS_LIGHT_DYNAMICTYPE_STATIC
                                             : CS_LIGHT_DYNAMICTYPE_DYNAMIC));

  sector->GetLights()->Add(light);

  vosa3dl->decrementRelightCounter();

  metalight->light = light;
}


csMetaLight::csMetaLight(VobjectBase* superobject)
  : A3DL::Light(superobject), sector(0), alreadyLoaded(false), isStatic(false)
{
}

csMetaLight::~csMetaLight()
{
}

void csMetaLight::notifyPropertyChange(const PropertyEvent& event)
{
#if 0
  try {
    rREF(ParentChildRelation&, pcr, event.getProperty()->findParent(*this),
         if(pcr.contextual_name == "a3dl:position")
         {
           csVector3 position;
           sscanf(event.getValue().c_str(), "%f %f %f",
                  &position.x, &position.y, &position.z);
           light->SetCenter(position);
           gotpos = true;
         }
         else if(pcr.contextual_name == "a3dl:radius")
         {
           float radius;
           sscanf(event.getValue().c_str(), "%f", &radius);
           light->SetInfluenceRadius(radius);
           gotradius = true;
         }
         else if(pcr.contextual_name == "a3dl:color")
         {
           csColor color;
           sscanf(event.getValue().c_str(), "%f %f %f", &color.red,
                  &color.green,
                  &color.blue);
           light->SetColor(color);
           gotcolor = true;
         }
         else if(pcr.contextual_name == "a3dl:static")
         {
           isStatic = (event.getValue() == "yes");
           gotstatic = true;
         }
      );
  } catch(NoSuchObjectError) {
  } catch(AccessControlError) {
  } catch(RemoteError) { }

  LOG("light", 3, "pos " << gotpos << "  rad " << gotradius
      << "  color " << gotcolor);
  if(!lightAdded && gotpos && gotradius && gotcolor)
  {
    sector->GetLights()->Add(light);
    //sector->ShineLights();
    //engine->Prepare();
    lightAdded = true;
  }

  if(dynLight && alreadyLoaded)
    dynLight->Setup();
#endif
}

void csMetaLight::Setup(csVosA3DL* vosa3dl, csVosSector* sector)
{
  if(alreadyLoaded) return;
  else alreadyLoaded = true;

  ConstructLightTask* clt = new ConstructLightTask(
    vosa3dl->GetObjectRegistry(), sector->GetSector(), getURLstr(), this);
  double x, y, z;
  getPosition(x, y, z);
  clt->pos.Set((float)x, (float)y, (float)z);
  getColor(clt->color.red, clt->color.green, clt->color.blue);
  clt->radius = getRadius();
  try
  {
    isStatic = getStatic();
  }
  catch(NoSuchObjectError)
  {
    isStatic = true;
  }
  clt->isStatic = isStatic;
  clt->vosa3dl = vosa3dl;

  vosa3dl->mainThreadTasks.push(clt);

  try
  {
    getPositionObj()->addPropertyListener(this);
  }
  catch(NoSuchObjectError) { }

  try
  {
    getRadiusObj()->addPropertyListener(this);
  }
  catch(NoSuchObjectError) { }

  try
  {
    getColorObj()->addPropertyListener(this);
  }
  catch(NoSuchObjectError) { }

  alreadyLoaded = true;
}

void csMetaLight::doExcise() {
#if 0
  if(dynLight.IsValid())
    engine->RemoveObject(dynLight);
  if(staticLight.IsValid()) {
    engine->RemoveLight(staticLight);
    engine->ForceRelight();
  }
#endif
}

MetaObject* csMetaLight::new_csMetaLight(VobjectBase* superobject,
                                         const std::string& /*type*/)
{
  return new csMetaLight(superobject);
}
