/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/object.h"
#include "imesh/crossbld.h"
#include "imesh/mdldata.h"
#include "imesh/thing/thing.h"
#include "imesh/thing/polygon.h"
#include "imesh/sprite3d.h"
#include "imesh/object.h"
#include "iengine/engine.h"
#include "iengine/mesh.h"
#include "cstool/sprbuild.h"
#include "csgeom/transfrm.h"
#include "csutil/objiter.h"

SCF_DECLARE_FAST_INTERFACE (iModelDataPolygon);
SCF_DECLARE_FAST_INTERFACE (iSprite3DFactoryState);
SCF_DECLARE_FAST_INTERFACE (iModelDataObject);

CS_DECLARE_OBJECT_ITERATOR (csModelDataObjectIterator, iModelDataObject);

//----------------------------------------------------------------------------

class csCrossBuilder : public iCrossBuilder
{
public:
  SCF_DECLARE_IBASE;

  /// constructor
  csCrossBuilder (iBase *parent);

  /// Build a thing from a model data object
  virtual bool BuildThing (iModelDataObject *Data, iThingState *tgt,
	iMaterialWrapper *defMat) const;

  /// Build a sprite factory from a model data object
  virtual bool BuildSpriteFactory (iModelDataObject *Data,
	iSprite3DFactoryState *tgt) const;

  /// Build a hierarchical sprite factory from all objects in a scene
  virtual iMeshFactoryWrapper *BuildSpriteFactoryHierarchy (iModelData *Scene,
	iEngine *Engine, iMaterialWrapper *DefaultMaterial) const;

  class Component : public iComponent
  {
  public:
    SCF_DECLARE_EMBEDDED_IBASE (csCrossBuilder);
    virtual bool Initialize (iObjectRegistry *)
    { return true; }
  } scfiComponent;
};

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csCrossBuilder)
  SCF_IMPLEMENTS_INTERFACE (iCrossBuilder)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csCrossBuilder::Component)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csCrossBuilder)

SCF_EXPORT_CLASS_TABLE (crossbld)
  SCF_EXPORT_CLASS (csCrossBuilder, "crystalspace.mesh.crossbuilder",
    "Modeldata-to-Meshobject cross builder")
SCF_EXPORT_CLASS_TABLE_END

csCrossBuilder::csCrossBuilder (iBase *parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
}

bool csCrossBuilder::BuildThing (iModelDataObject *Object, iThingState *tgt,
  iMaterialWrapper *DefaultMaterial) const
{
  int i;

  // copy the vertices
  iModelDataVertices *Vertices = Object->GetDefaultVertices ();
  if (!Vertices) return false;
  for (i=0; i<Vertices->GetVertexCount (); i++)
    tgt->CreateVertex (Vertices->GetVertex (i));

  // copy the polygons
  iObjectIterator *it = Object->QueryObject ()->GetIterator ();
  while (!it->IsFinished ())
  {
    // test if this is a valid polygon
    iModelDataPolygon *Polygon =
      SCF_QUERY_INTERFACE_FAST (it->GetObject (), iModelDataPolygon);
    if (!Polygon || Polygon->GetVertexCount () < 3)
    {
      it->Next ();
      continue;
    }
    iPolygon3D *ThingPoly = tgt->CreatePolygon ();

    // copy vertices
    for (i=0; i<Polygon->GetVertexCount (); i++)
      ThingPoly->CreateVertex (Polygon->GetVertex (i));

    // copy material
    iModelDataMaterial *mat = Polygon->GetMaterial ();
    if (mat && mat->GetMaterialWrapper ())
      ThingPoly->SetMaterial (mat->GetMaterialWrapper ());
    else
      ThingPoly->SetMaterial (DefaultMaterial);

    // copy texture transformation
    ThingPoly->SetTextureSpace (
      Vertices->GetVertex (Polygon->GetVertex (0)),
      Vertices->GetTexel (Polygon->GetTexel(0)),
      Vertices->GetVertex (Polygon->GetVertex (1)),
      Vertices->GetTexel (Polygon->GetTexel(1)),
      Vertices->GetVertex (Polygon->GetVertex (2)),
      Vertices->GetTexel (Polygon->GetTexel(2)));

    it->Next ();
  }
  it->DecRef ();

  return true;
}

bool csCrossBuilder::BuildSpriteFactory (iModelDataObject *Object,
	iSprite3DFactoryState *tgt) const
{
  csSpriteBuilderMesh SpriteBuilder;
  return SpriteBuilder.Build (Object, tgt);
}

iMeshFactoryWrapper *csCrossBuilder::BuildSpriteFactoryHierarchy (
	iModelData *Scene, iEngine *Engine, iMaterialWrapper *DefaultMaterial) const
{
  iMeshFactoryWrapper *MainWrapper = NULL;

  csModelDataObjectIterator it (Scene->QueryObject ());
  while (!it.IsFinished ())
  {
    iMeshFactoryWrapper *SubWrapper = Engine->CreateMeshFactory (
      "crystalspace.mesh.object.sprite.3d", NULL);
    if (!SubWrapper)
    {
      // seems like building 3d sprites is impossible
      return NULL;
    }

    iSprite3DFactoryState *sfState = SCF_QUERY_INTERFACE_FAST (
      SubWrapper->GetMeshObjectFactory (), iSprite3DFactoryState);
    if (!sfState)
    {
      // impossible to query the correct interface, maybe because
      // of a version conflict
      Engine->GetMeshFactories ()->Remove (SubWrapper);
      return NULL;
    }

    sfState->SetMaterialWrapper (DefaultMaterial);
    BuildSpriteFactory (it.Get (), sfState);
    sfState->DecRef ();

    if (MainWrapper)
    {
      MainWrapper->GetChildren ()->Add (SubWrapper);
      /* @@@ remove the sub-wrapper from the iEngine again? */
    }
    else
      MainWrapper = SubWrapper;

    it.Next ();
  }

  return MainWrapper;
}
