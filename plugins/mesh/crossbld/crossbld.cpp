
#include "cssysdef.h"
#include "csutil/csobject.h"
#include "csutil/typedvec.h"
#include "csutil/objiter.h"
#include "csgeom/transfrm.h"
#include "isys/plugin.h"
#include "imesh/crossbld.h"
#include "imesh/mdldata.h"
#include "imesh/thing/thing.h"
#include "imesh/thing/polygon.h"
#include "imesh/sprite3d.h"
#include "imesh/object.h"
#include "iengine/engine.h"
#include "iengine/mesh.h"

SCF_DECLARE_FAST_INTERFACE (iModelDataObject);
SCF_DECLARE_FAST_INTERFACE (iModelDataPolygon);
SCF_DECLARE_FAST_INTERFACE (iModelDataAction);
SCF_DECLARE_FAST_INTERFACE (iModelDataVertices);
SCF_DECLARE_FAST_INTERFACE (iSprite3DFactoryState);

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

  class Plugin : public iPlugin
  {
  public:
    SCF_DECLARE_EMBEDDED_IBASE (csCrossBuilder);
    virtual bool Initialize (iObjectRegistry *)
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

CS_DECLARE_TYPED_VECTOR_NODELETE (csModelFrameVector, iModelDataVertices);
CS_DECLARE_TYPED_VECTOR_NODELETE (csSpriteFrameVector, iSpriteFrame);
CS_TYPEDEF_GROWING_ARRAY (csIntArray, int);

static void BuildVertexArray (iModelDataPolygon* poly,
	csIntArray& SpriteVertices,
	csIntArray& SpriteNormals,
	csIntArray& SpriteTexels,
	csIntArray& PolyVertices)
{
  // build the vertex array
  PolyVertices.SetLength (0);
  int i, j;
  for (i=0; i<poly->GetVertexCount (); i++)
  {
    int SpriteVertexIndex = -1;
    int PolyVertex = poly->GetVertex (i);
    int PolyNormal = poly->GetNormal (i);
    int PolyTexel = poly->GetTexel (i);

    for (j=0; j<SpriteVertices.Length (); j++)
    {
      int SpriteVertex = SpriteVertices [j];
      int SpriteNormal = SpriteNormals [j];
      int SpriteTexel = SpriteTexels [j];

      if (SpriteVertex == PolyVertex &&
	  SpriteNormal == PolyNormal &&
	  SpriteTexel == PolyTexel)
      {
	SpriteVertexIndex = i;
	break;
      }
    }
    if (SpriteVertexIndex == -1)
    {
      SpriteVertexIndex = SpriteVertices.Length ();
      SpriteTexels.Push (PolyTexel);
      SpriteNormals.Push (PolyNormal);
      SpriteVertices.Push (PolyVertex);
    }
    PolyVertices.Push (SpriteVertexIndex);
  }
}

bool csCrossBuilder::BuildSpriteFactory (iModelDataObject *Object,
	iSprite3DFactoryState *tgt) const
{
  int i,j;
  iObjectIterator *it1;
  iMaterialWrapper *Material = NULL;

  //--- preparation stage: arrange and validate incoming data locally --------

  // build a list of all frames and merge duplicate frames
  csModelFrameVector Frames;

  it1 = Object->QueryObject ()->GetIterator ();
  while (!it1->IsFinished ())
  {
    iModelDataAction *ac = SCF_QUERY_INTERFACE_FAST (it1->GetObject (),
    	iModelDataAction);
    if (ac)
    {
      for (i=0; i<ac->GetFrameCount (); i++)
      {
        iModelDataVertices *ver =
	  SCF_QUERY_INTERFACE_FAST (ac->GetState (i), iModelDataVertices);
        if (ver)
	{
	  if (Frames.Find (ver) == -1)
	    Frames.Push (ver);
	  ver->DecRef ();
	}
      }
      ac->DecRef ();
    }
    it1->Next ();
  }
  it1->DecRef ();

  // we need at least one vertex frame
  if (Frames.Length () == 0)
  {
    iModelDataVertices *BaseVertices = Object->GetDefaultVertices ();
    if (!BaseVertices) return false;
    Frames.Push (BaseVertices);
  }

  //--- building stage -------------------------------------------------------

  /* These lists are filled up with (sprite vertex) to (model data vertex),
   * (model data normal) and (model data texel) mappings. This means that
   * they are indexed by the sprite vertex and must be of the same size.
   */
  csIntArray SpriteVertices;
  csIntArray SpriteNormals;
  csIntArray SpriteTexels;

  // copy polygon data (split polygons into triangles)
  it1 = Object->QueryObject ()->GetIterator ();
  while (!it1->IsFinished ())
  {
    iModelDataPolygon *poly =
      SCF_QUERY_INTERFACE_FAST (it1->GetObject (), iModelDataPolygon);
    if (poly)
    {
      // build the vertex array
      csIntArray PolyVertices;
      BuildVertexArray (poly,
	SpriteVertices, SpriteNormals, SpriteTexels, PolyVertices);

      // split the polygon into triangles and copy them
      for (i=2; i<PolyVertices.Length (); i++)
        tgt->AddTriangle (PolyVertices[0], PolyVertices[i-1], PolyVertices[i]);
      
      // store the material if we don't have any yet
      if (!Material && poly->GetMaterial ())
        Material = poly->GetMaterial ()->GetMaterialWrapper ();

      poly->DecRef ();
    }
    it1->Next ();
  }
  it1->DecRef ();

  // copy the first valid material
  if (Material) tgt->SetMaterialWrapper (Material);

  /* create all frames in the target factory. This is done separately because
   * adding the vertices fails if no frames exist.
   */
  int NumPreviousFrames = tgt->GetFrameCount ();
  for (i=0; i<Frames.Length (); i++)
    tgt->AddFrame ();

  // create room for the vertices in the sprite
  tgt->AddVertices (SpriteVertices.Length ());

  // create all frames in the target factory
  bool NeedTiling = false;
  for (i=0; i<Frames.Length (); i++)
  {
    int FrameIndex = NumPreviousFrames + i;
    iSpriteFrame *SpriteFrame = tgt->GetFrame (FrameIndex);
    iModelDataVertices *Vertices = Frames.Get (i);
    SpriteFrame->SetName (Vertices->QueryObject ()->GetName ());

    for (j=0; j<SpriteVertices.Length (); j++)
    {
      tgt->SetVertex (FrameIndex, j, Vertices->GetVertex (SpriteVertices [j]));
      tgt->SetNormal (FrameIndex, j, Vertices->GetNormal (SpriteNormals [j]));

      csVector2 t = Vertices->GetTexel (SpriteTexels [j]);
      tgt->SetTexel (FrameIndex, j, t);
      if (t.x < 0 || t.y < 0 || t.x > 1 || t.y > 1)
        NeedTiling = true;
    }
  }

  // enable texture tiling if required
  if (NeedTiling)
    tgt->SetMixMode (tgt->GetMixMode () | CS_FX_TILING);

  /* Create all actions in the target factory. We also build a default
   * action (named 'default') if no action with this name exists. The
   * default action shows the first frame all the time. 
   */
  bool FoundDefault = false;

  it1 = Object->QueryObject ()->GetIterator ();
  while (!it1->IsFinished ())
  {
    iModelDataAction *ac = SCF_QUERY_INTERFACE_FAST (it1->GetObject (), iModelDataAction);
    if (ac)
    {
      iSpriteAction *spract = tgt->AddAction ();
      const char *name = ac->QueryObject ()->GetName ();
      if (name)
      {
        spract->SetName (name);
        if (!strcmp (name, "default"))
	  FoundDefault = true;
      }

      float LastTime = 0;
      for (i=0; i<ac->GetFrameCount (); i++)
      {
        /* It might seem strange to store the nth frame time value with the
	 * (n-1)th frame state. This difference is due to the different
	 * meaning of the time values in the model data structures and
	 * in 3d sprites.
	 */
        int FrameIndex = (i == 0) ? (ac->GetFrameCount ()-1) : (i-1);
        iModelDataVertices *ver = SCF_QUERY_INTERFACE_FAST (
		ac->GetState (FrameIndex), iModelDataVertices);
	if (ver)
	{
	  float ThisTime = ac->GetTime (i);
	  float Delay = ThisTime - LastTime;
  	  LastTime = ThisTime;

	  int FrameIndex = Frames.Find (ver);
	  CS_ASSERT (FrameIndex != -1);
	  spract->AddFrame (tgt->GetFrame (FrameIndex), int(Delay * 1000));
	}
      }
    }
    it1->Next ();
  }
  it1->DecRef ();

  if (!FoundDefault)
  {
    iSpriteAction *spract = tgt->AddAction ();
    spract->SetName ("default");
    spract->AddFrame (tgt->GetFrame (0), 1000);
  }

  return true;
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
    if (!SubWrapper) {
      // seems like building 3d sprites is impossible
      Engine->GetMeshFactories ()->RemoveMeshFactory (MainWrapper);
      return NULL;
    }

    iSprite3DFactoryState *sfState = SCF_QUERY_INTERFACE_FAST (
      SubWrapper->GetMeshObjectFactory (), iSprite3DFactoryState);
    if (!sfState) {
      // impossible to query the correct interface, maybe because
      // of a version conflict
      Engine->GetMeshFactories ()->RemoveMeshFactory (MainWrapper);
      return NULL;
    }
    
    sfState->SetMaterialWrapper (DefaultMaterial);
    BuildSpriteFactory (it.Get (), sfState);
    sfState->DecRef ();

    if (MainWrapper) {
      MainWrapper->AddChild (SubWrapper, csReversibleTransform ());
      /* @@@ remove the sub-wrapper from the iEngine again? */
    } else MainWrapper = SubWrapper;

    it.Next ();
  }
  
  return MainWrapper;
}
