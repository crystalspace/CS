/*
    Copyright (C) 2001 by Jorrit Tyberghein

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
#include "stars.h"
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
#include "qsqrt.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csStarsMeshObject)
  SCF_IMPLEMENTS_INTERFACE (iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iStarsState)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csStarsMeshObject::StarsState)
  SCF_IMPLEMENTS_INTERFACE (iStarsState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csStarsMeshObject::csStarsMeshObject (iMeshObjectFactory* factory)
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiStarsState);
  csStarsMeshObject::factory = factory;
  initialized = false;
  box.Set (csVector3 (-10, -10, -10), csVector3 (10, 10, 10));
  vis_cb = NULL;
  shapenr = 0;
  color.red = 1;
  color.green = 1;
  color.blue = 1;
  use_max_color = false;
  max_dist = 20;
  density = .1;
  seed = 3939394;
  current_lod = 1;
  current_features = ALL_FEATURES;
}

csStarsMeshObject::~csStarsMeshObject ()
{
  if (vis_cb) vis_cb->DecRef ();
}

void csStarsMeshObject::SetupObject ()
{
  if (!initialized)
  {
    initialized = true;
  }
}

bool csStarsMeshObject::DrawTest (iRenderView* rview, iMovable* movable)
{
  SetupObject ();
  return true;
}

void csStarsMeshObject::UpdateLighting (iLight**, int,
    iMovable*)
{
  SetupObject ();
  return;
}

/*
 * Commented this out because it was not used -- mgeisse
 *
static void Perspective (const csVector3& v, csVector2& p, float fov,
    	float sx, float sy)
{
  float iz = fov / v.z;
  p.x = v.x * iz + sx;
  p.y = v.y * iz + sy;
}
*/

bool csStarsMeshObject::Draw (iRenderView* rview, iMovable* /*movable*/,
	csZBufMode mode)
{
  if (vis_cb) if (!vis_cb->BeforeDrawing (this, rview)) return false;

#if 0
  iGraphics2D* g2d; g2d = rview->GetGraphics2D ();
  iCamera* camera = rview->GetCamera ();
  const csReversibleTransform& camtrans = camera->GetTransform ();
  //float fov = camera->GetFOV ();
  //float shiftx = camera->GetShiftX ();
  //float shifty = camera->GetShiftY ();

  srand (seed);
  int i;
  for (i = 0 ; i < 100 ; i++)
  {
  }
#endif

  return true;
}

void csStarsMeshObject::GetObjectBoundingBox (csBox3& b, int /*type*/)
{
  SetupObject ();
  b = box;
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csStarsMeshObjectFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
SCF_IMPLEMENT_IBASE_END

csStarsMeshObjectFactory::csStarsMeshObjectFactory (iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
}

csStarsMeshObjectFactory::~csStarsMeshObjectFactory ()
{
}

iMeshObject* csStarsMeshObjectFactory::NewInstance ()
{
  csStarsMeshObject* cm = new csStarsMeshObject ((iMeshObjectFactory*)this);
  iMeshObject* im = SCF_QUERY_INTERFACE (cm, iMeshObject);
  im->DecRef ();
  return im;
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csStarsMeshObjectType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_INTERFACE (iPlugIn)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csStarsMeshObjectType)

SCF_EXPORT_CLASS_TABLE (stars)
  SCF_EXPORT_CLASS (csStarsMeshObjectType, "crystalspace.mesh.object.stars",
    "Crystal Space Stars Mesh Type")
SCF_EXPORT_CLASS_TABLE_END

csStarsMeshObjectType::csStarsMeshObjectType (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
}

csStarsMeshObjectType::~csStarsMeshObjectType ()
{
}

bool csStarsMeshObjectType::Initialize (iSystem*)
{
  return true;
}

iMeshObjectFactory* csStarsMeshObjectType::NewFactory ()
{
  csStarsMeshObjectFactory* cm = new csStarsMeshObjectFactory (this);
  iMeshObjectFactory* ifact = SCF_QUERY_INTERFACE (cm, iMeshObjectFactory);
  ifact->DecRef ();
  return ifact;
}

