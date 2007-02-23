/*
    Copyright (C) 2003 by Boyan Hristov

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
#include "csgeom/math3d.h"
#include "csgeom/poly2d.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "iengine/camera.h"
#include "igeom/clip2d.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "csqsqrt.h"

#include "lghtng.h"

CS_IMPLEMENT_PLUGIN

//------------ csLightningMeshObject -------------------------------

csLightningMeshObject::csLightningMeshObject (
  csLightningMeshObjectFactory* factory) :
  scfImplementationType(this)
{
  csLightningMeshObject::factory = factory;
  logparent = 0;
  ifactory = SCF_QUERY_INTERFACE (factory, iMeshObjectFactory);

  initialized = false;
  vis_cb = 0;
  origin.Set(0,0,0);  

  CS_ASSERT(factory);
  /// copy the factory settings
  material = factory->GetMaterialWrapper ();
  MixMode = factory->GetMixMode ();
  points = factory->GetPointCount();
  origin = factory->GetOrigin ();
  directional = factory->GetDirectional ();
  wildness = factory->GetWildness ();
  vibration = factory->GetVibration ();
  bandwidth = factory->GetBandWidth ();
  
  GenMesh = factory->GetMeshFactory ()->NewInstance ();
  if (GenMesh)
  {
    GenState = SCF_QUERY_INTERFACE (GenMesh, iGeneralMeshState);
    GenState->SetLighting (false);
    GenState->SetManualColors (true);
    GenMesh->SetMaterialWrapper (material);
    GenMesh->SetColor (csColor (1.f, 1.f, 1.f));
    GenMesh->SetMixMode (MixMode);    
  }
}

csLightningMeshObject::~csLightningMeshObject ()
{
  if (vis_cb) vis_cb->DecRef ();
}

void csLightningMeshObject::SetupObject ()
{
  if (!initialized)
  {    
    //csVector3 pos; //@@@ Unused
    //int l, i; //@@@ Unused
    initialized = true;
  }
}

csRenderMesh** csLightningMeshObject::GetRenderMeshes (int &n, 
						       iRenderView* rview, 
						       iMovable* movable, 
						       uint32 frust_mask)
{ 
  csReversibleTransform &camtr = rview->GetCamera ()->GetTransform ();
  csReversibleTransform &sprtr = movable->GetTransform ();
  sprtr.LookAt(directional, camtr.GetOrigin () - sprtr.GetOrigin ());  
  
  movable->UpdateMove ();

  return GenMesh->GetRenderMeshes (n, rview, movable, frust_mask); 
}


void csLightningMeshObject::GetObjectBoundingBox (csBox3& retbbox)
{
  GenMesh->GetObjectModel ()->GetObjectBoundingBox (retbbox);
}

void csLightningMeshObject::SetObjectBoundingBox (const csBox3& inbbox)
{
  GenMesh->GetObjectModel ()->SetObjectBoundingBox (inbbox);
  this->ShapeChanged ();
}

void csLightningMeshObject::GetRadius (float& rad, csVector3& cent)
{
  GenMesh->GetObjectModel ()->GetRadius (rad, cent);
}

void csLightningMeshObject::HardTransform (const csReversibleTransform& t)
{
  (void)t;
}

void csLightningMeshObject::NextFrame (csTicks current_time,
	const csVector3& /*pos*/, uint /*currentFrame*/)
{
  factory->NextFrame(current_time);
}

//----------------------------------------------------------------------

csLightningMeshObjectFactory::csLightningMeshObjectFactory (
  iMeshObjectType *pParent, iObjectRegistry *object_registry) :
  scfImplementationType(this, pParent)
{
  MaxPoints = 20;
  wildness = 0.02f;
  vibrate = 0.02f;
  glowsize = 0.02f;
  length = 5;
  bandwidth = 0.3f;
  update_interval = 60;
  update_counter = 0;
  
  material = 0;
  MixMode = 0;
  origin.Set (0, 0, 0);
  directional.Set (0, 0, 1);  
  logparent = 0;
  lghtng_type = pParent;
  
  csRef<iPluginManager> PlugMgr (CS_QUERY_REGISTRY (object_registry, iPluginManager));
  CS_ASSERT (PlugMgr);
  csRef<iMeshObjectType> MeshType (CS_LOAD_PLUGIN(PlugMgr,
      "crystalspace.mesh.object.genmesh", iMeshObjectType));
  if (MeshType)
  {
    GenMeshFact = MeshType->NewFactory ();
    Invalidate ();
  }
}

csLightningMeshObjectFactory::~csLightningMeshObjectFactory ()
{
}

void csLightningMeshObjectFactory::CalculateFractal (int left, int right,
    float lh, float rh, int xyz, csVector3 *Vertices)
{
  int mid = (left + right) / 2;
  float fracScale = ((float)(right - left)) / (float)(MaxPoints);
  float midh = (lh + rh) / 2 + (fracScale * wildness * (int)(rand.Get(20)-10))
      - (fracScale * wildness) / 2;

  const int mid2 = mid * 2;
  switch (xyz)
  {
    case 0:    
      Vertices[mid2].x = origin.x + midh + (vibrate * (int)(rand.Get(10)-5) - (vibrate / 2));      
      break;    
    case 1:
      Vertices[mid2].y = origin.y + midh + (vibrate * (int)(rand.Get(10)-5) - (vibrate / 2));
      break;    
  }

  if ((mid - left) > 1)
    CalculateFractal(left, mid, lh, midh, xyz, Vertices);
  if ((right - mid) > 1)
    CalculateFractal(mid, right, midh, rh, xyz, Vertices);
}

void csLightningMeshObjectFactory::CalculateFractal()
{
  int i;
  const int m2 = MaxPoints * 2;  

  csVector3 *Vertices = GenFactState->GetVertices();

  Vertices[0] = origin;    

  CalculateFractal(0, MaxPoints - 1, 0, 0, 0, GenFactState->GetVertices());
  CalculateFractal(0, MaxPoints - 1, 0, 0, 1, GenFactState->GetVertices());  
  
  
  float CurrZ = 0;
  float ZStep = length / (float)MaxPoints;
    
  for ( i = 0; i < m2; i += 2 )
  {
    Vertices[i + 1].x = Vertices[i].x + bandwidth;
    Vertices[i + 1].y = Vertices[i].y;
    Vertices[i].z = CurrZ + origin.z;
    Vertices[i + 1].z = Vertices[i].z;
    CurrZ += ZStep;
  }    

  Vertices[m2 - 2].x = origin.x;
  Vertices[m2 - 2].y = origin.y;

  GenFactState->Invalidate();
}

void csLightningMeshObjectFactory::NextFrame (csTicks CurrentTime)
{
  if (update_counter == (csTicks)-1
  	|| CurrentTime - update_counter > update_interval)
  {
    update_counter = CurrentTime;      
    CalculateFractal();
  }
}

csPtr<iMeshObject> csLightningMeshObjectFactory::NewInstance ()
{
  csLightningMeshObject* cm = new csLightningMeshObject (this);
  csRef<iMeshObject> im (SCF_QUERY_INTERFACE (cm, iMeshObject));
  cm->DecRef ();
  return csPtr<iMeshObject> (im);
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY (csLightningMeshObjectType)

csLightningMeshObjectType::csLightningMeshObjectType (iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

csLightningMeshObjectType::~csLightningMeshObjectType ()
{
}

csPtr<iMeshObjectFactory> csLightningMeshObjectType::NewFactory ()
{
  csLightningMeshObjectFactory* cm = new csLightningMeshObjectFactory (this, Registry);
  csRef<iMeshObjectFactory> ifact (
  	SCF_QUERY_INTERFACE (cm, iMeshObjectFactory));
  cm->DecRef ();
  return csPtr<iMeshObjectFactory> (ifact);
}

