
#include "cssysdef.h"
#include "imesh/crossbld.h"
#include "isys/plugin.h"
#include "imesh/mdldata.h"
#include "iutil/object.h"
#include "imesh/thing/thing.h"
#include "imesh/thing/polygon.h"

SCF_DECLARE_FAST_INTERFACE (iModelDataObject);
SCF_DECLARE_FAST_INTERFACE (iModelDataPolygon);

class csCrossBuilder : public iCrossBuilder
{
public:
  SCF_DECLARE_IBASE;

  /// constructor
  csCrossBuilder (iBase *parent);

  /// Build a thing from a model file
  virtual bool BuildThing (iModelData *Data, iThingState *tgt,
	iMaterialWrapper *defMat) const;

  /// Build a sprite factory from a model file
  virtual bool BuildSpriteFactory (iModelData *Data,
	iSprite3DFactoryState *tgt) const;

  /// Find the first object in a model data structure
  iModelDataObject *FindObject (iModelData *Data) const;
  
  class Plugin : public iPlugin
  {
  public:
    SCF_DECLARE_EMBEDDED_IBASE (csCrossBuilder);
    virtual bool Initialize (iSystem *sys)
    { return true; }
  } scfiPlugin;
};

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csCrossBuilder)
  SCF_IMPLEMENTS_INTERFACE (iCrossBuilder)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csCrossBuilder::Plugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csCrossBuilder)

SCF_EXPORT_CLASS_TABLE (crossbld)
  SCF_EXPORT_CLASS (csCrossBuilder, "crystalspace.mesh.crossbuilder",
    "Modeldata-to-Meshobject cross builder")
SCF_EXPORT_CLASS_TABLE_END

csCrossBuilder::csCrossBuilder (iBase *parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPlugin);
}

iModelDataObject *csCrossBuilder::FindObject (iModelData *Data) const
{
  iObjectIterator *it = Data->QueryObject ()->GetIterator ();
  while (!it->IsFinished ())
  {
    iModelDataObject *mdo =
      SCF_QUERY_INTERFACE_FAST (it->GetObject (), iModelDataObject);
    if (mdo)
    {
      it->DecRef ();
      return mdo;
    }
    it->Next ();
  }
  it->DecRef ();
  return NULL;
}

bool csCrossBuilder::BuildThing (iModelData *Data, iThingState *tgt,
  iMaterialWrapper *DefaultMaterial) const
{
  int i;

  // find the first object in the scene
  iModelDataObject *Object = FindObject (Data);
  if (!Object)
    return false;

  // copy the vertices
  for (i=0; i<Object->GetVertexCount (); i++)
    tgt->CreateVertex (Object->GetVertex (i));

  // copy the polygons
  iObjectIterator *it = Object->QueryObject ()->GetIterator ();
  while (!it->IsFinished ())
  {
    // test if this is a valid polygon
    iModelDataPolygon *Polygon =
      SCF_QUERY_INTERFACE_FAST (it->GetObject (), iModelDataPolygon);
    if (Polygon->GetVertexCount () < 3)
      continue;
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
      Object->GetVertex(Polygon->GetVertex(0)), Polygon->GetTextureCoords(0),
      Object->GetVertex(Polygon->GetVertex(1)), Polygon->GetTextureCoords(1),
      Object->GetVertex(Polygon->GetVertex(2)), Polygon->GetTextureCoords(2));

    it->Next ();
  }
  it->DecRef ();

  return true;
}

bool csCrossBuilder::BuildSpriteFactory (iModelData *Data,
	iSprite3DFactoryState *tgt) const
{
  iModelDataObject *Object = FindObject (Data);
  if (!Object)
    return false;

  return false;
}
