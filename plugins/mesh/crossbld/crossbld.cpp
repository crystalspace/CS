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
#include "imesh/thing.h"
#include "imesh/sprite3d.h"
#include "imesh/object.h"
#include "iengine/engine.h"
#include "iengine/mesh.h"
#include "cstool/sprbuild.h"
#include "csgeom/transfrm.h"
#include "csutil/objiter.h"

class csCrossBuilder : public iCrossBuilder
{
public:
  SCF_DECLARE_IBASE;

  /// constructor
  csCrossBuilder (iBase *parent);

  /// destructor
  virtual ~csCrossBuilder ();

  /// Build a thing from a model data object
  virtual bool BuildThing (iModelDataObject *Data, iThingFactoryState *tgt,
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


csCrossBuilder::csCrossBuilder (iBase *parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
}

csCrossBuilder::~csCrossBuilder ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csCrossBuilder::BuildThing (iModelDataObject *Object,
	iThingFactoryState *tgt, iMaterialWrapper *DefaultMaterial) const
{
  size_t i;

  // copy the vertices
  iModelDataVertices *Vertices = Object->GetDefaultVertices ();
  if (!Vertices) return false;
  for (i=0; i<Vertices->GetVertexCount (); i++)
    tgt->CreateVertex (Vertices->GetVertex (i));

  // copy the polygons
  csRef<iObjectIterator> it (Object->QueryObject ()->GetIterator ());
  while (it->HasNext ())
  {
    // test if this is a valid polygon
    // @@@ MEMORY LEAK???
    csRef<iModelDataPolygon> Polygon (
      SCF_QUERY_INTERFACE (it->Next (), iModelDataPolygon));
    if (!Polygon || Polygon->GetVertexCount () < 3)
    {
      continue;
    }
    tgt->AddEmptyPolygon ();

    // copy vertices
    for (i=0; i<Polygon->GetVertexCount (); i++)
      tgt->AddPolygonVertex (CS_POLYRANGE_LAST, Polygon->GetVertex (i));

    // copy material
    iModelDataMaterial *mat = Polygon->GetMaterial ();
    if (mat && mat->GetMaterialWrapper ())
      tgt->SetPolygonMaterial (CS_POLYRANGE_LAST, mat->GetMaterialWrapper ());
    else
      tgt->SetPolygonMaterial (CS_POLYRANGE_LAST, DefaultMaterial);

    // copy texture transformation
    tgt->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      Vertices->GetVertex (Polygon->GetVertex (0)),
      Vertices->GetTexel (Polygon->GetTexel(0)),
      Vertices->GetVertex (Polygon->GetVertex (1)),
      Vertices->GetTexel (Polygon->GetTexel(1)),
      Vertices->GetVertex (Polygon->GetVertex (2)),
      Vertices->GetTexel (Polygon->GetTexel(2)));
  }

  return true;
}

bool csCrossBuilder::BuildSpriteFactory (iModelDataObject *Object,
	iSprite3DFactoryState *tgt) const
{
  csSpriteBuilderMesh SpriteBuilder;
  return SpriteBuilder.Build (Object, tgt);
}

iMeshFactoryWrapper *csCrossBuilder::BuildSpriteFactoryHierarchy (
	iModelData *Scene, iEngine *Engine,
	iMaterialWrapper *DefaultMaterial) const
{
  csRef<iMeshFactoryWrapper> MainWrapper;

  csTypedObjectIterator<iModelDataObject> it (Scene->QueryObject ());
  while (it.HasNext ())
  {
    csRef<iMeshFactoryWrapper> SubWrapper (Engine->CreateMeshFactory (
      "crystalspace.mesh.object.sprite.3d", 0));
    if (!SubWrapper)
    {
      // seems like building 3d sprites is impossible
      return 0;
    }

    csRef<iSprite3DFactoryState> sfState (SCF_QUERY_INTERFACE (
      SubWrapper->GetMeshObjectFactory (), iSprite3DFactoryState));
    if (!sfState)
    {
      // impossible to query the correct interface, maybe because
      // of a version conflict
      Engine->GetMeshFactories ()->Remove (SubWrapper);
      return 0;
    }

    sfState->SetMaterialWrapper (DefaultMaterial);
    BuildSpriteFactory (it.Next (), sfState);

    if (MainWrapper)
    {
      MainWrapper->GetChildren ()->Add (SubWrapper);
      /* @@@ remove the sub-wrapper from the iEngine again? */
    }
    else
      MainWrapper = SubWrapper;
  }

  MainWrapper->IncRef ();	// IncRef() to avoid smart pointer release.
  return MainWrapper;
}
