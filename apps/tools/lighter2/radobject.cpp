/*
  Copyright (C) 2005 by Marten Svanfeldt

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

#include "crystalspace.h"

#include "lighter.h"
#include "lightmap.h"
#include "lightmapuv.h"
#include "radobject.h"

namespace lighter
{

  RadObjectFactory::RadObjectFactory ()
  {
  }

  RadObject* RadObjectFactory::CreateObject ()
  {
    return new RadObject (this);
  }

  bool RadObjectFactory::ComputeLightmapUV (LightmapUVLayouter* layoutEngine)
  {
    // Default implementation
    bool res = layoutEngine->LayoutUVOnPrimitives (allPrimitives, 
      lightmapTemplates);
    if (!res) return false;

    return true;
  }

  void RadObjectFactory::ParseFactory (iMeshFactoryWrapper *factory)
  {
    this->factoryWrapper = factory;
    // Get the name
    this->factoryName = factoryWrapper->QueryObject ()->GetName ();
  }

  void RadObjectFactory::SaveFactory (iDocumentNode *node)
  {
    // Save out the factory to the node
    csRef<iSaverPlugin> saver = 
      csQueryPluginClass<iSaverPlugin> (globalLighter->pluginManager,
      saverPluginName);      
    if (!saver) 
      saver = csLoadPlugin<iSaverPlugin> (globalLighter->pluginManager,
      saverPluginName);
    if (saver) 
    {
      // Make sure to remove old params node
      csRef<iDocumentNode> paramChild = node->GetNode ("params");
      if (paramChild) node->RemoveNode (paramChild);
      saver->WriteDown(factoryWrapper->GetMeshObjectFactory (), node);
    }
  }

  RadObject::RadObject (RadObjectFactory* fact)
    : factory (fact)
  {
  }
  
  void RadObject::Initialize ()
  {
    if (!factory || !meshWrapper) return;
    const csReversibleTransform transform = meshWrapper->GetMovable ()->
      GetFullTransform ();

    //Copy over data, transform the radprimitives..
    allPrimitives = factory->allPrimitives;
    unsigned int i = 0;
    for (i = 0; i < allPrimitives.GetSize (); i++)
    {
      allPrimitives[i].Transform (transform);
      //recompute the factors
      allPrimitives[i].ComputeUVTransform ();
    }

    // Create and init lightmaps
    for (i = 0; i < factory->lightmapTemplates.GetSize (); i++)
    {
      Lightmap *lm = new Lightmap (*(factory->lightmapTemplates.Get (i)));
      lm->Initialize ();
      lightmaps.Push (lm);
    }
  }

  void RadObject::ParseMesh (iMeshWrapper *wrapper)
  {
    this->meshWrapper = wrapper;
    this->meshName = wrapper->QueryObject ()->GetName ();
  }

  void RadObject::SaveMesh (iDocumentNode *node)
  {

  }

  //////////////////////////////////////////////////////////////////////////
  // SPECIFIC VERISONS
  //////////////////////////////////////////////////////////////////////////
  
  //////////////   Genmesh     /////////////////////////////////////////////
  
  RadObjectFactory_Genmesh::RadObjectFactory_Genmesh()
    : normals (0)
  {
    saverPluginName = "crystalspace.mesh.saver.factory.genmesh";
  }

  void RadObjectFactory_Genmesh::ParseFactory (iMeshFactoryWrapper *factory)
  {
    RadObjectFactory::ParseFactory (factory);

    // Very dumb parser, just disconnect all triangles etc
    csRef<iGeneralFactoryState> genFact = 
      scfQueryInterface<iGeneralFactoryState> (factory->GetMeshObjectFactory ());
    
    if (!genFact) return; // bail

    csTriangle *tris = genFact->GetTriangles ();
    csVector3 *verts = genFact->GetVertices ();
    csVector3 *factNormals = genFact->GetNormals ();

    int i = 0;

    // Here we should save extra per-vertex stuff!
    normals = new csVector3[genFact->GetVertexCount ()];
    for (i = 0; i < genFact->GetVertexCount (); i++)
    {
      normals[i] = factNormals[i];
    }

    for (i=0; i<genFact->GetTriangleCount ();i++)
    {
      RadPrimitive newPrim;
      Vector3DArray &arr = newPrim.GetVertices ();
      IntArray &extra = newPrim.GetExtraData ();

      arr.Push (verts[tris[i].a]);
      arr.Push (verts[tris[i].b]);
      arr.Push (verts[tris[i].c]);

      extra.Push (tris[i].a);
      extra.Push (tris[i].b);
      extra.Push (tris[i].c);

      newPrim.GetUVs ().SetSize (3);
      newPrim.ComputePlane ();
      
      allPrimitives.Push (newPrim);
    }
  }

  void RadObjectFactory_Genmesh::SaveFactory (iDocumentNode *node)
  {
    csRef<iGeneralFactoryState> genFact = 
      scfQueryInterface<iGeneralFactoryState> (
      factoryWrapper->GetMeshObjectFactory ());
    
    if (!genFact) return; // bail

    // For now, just dump.. later we should preserve extra attributes etc :)

    genFact->SetVertexCount ((int)allPrimitives.GetSize () * 3);
    genFact->SetTriangleCount ((int)allPrimitives.GetSize ());
    
    csTriangle *tris = genFact->GetTriangles ();
    csVector3 *verts = genFact->GetVertices ();
    csVector2 *tc = genFact->GetTexels ();
    csVector3 *factNormals = genFact->GetNormals ();

    int j = 0;

    for (int i=0; i< genFact->GetTriangleCount (); i++)
    {
      const Lightmap* lm = lightmapTemplates[allPrimitives[i].GetLightmapID ()];
      
      allPrimitives[i].RenormalizeUVs (lm->GetWidth (),
                                       lm->GetHeight ());

      const Vector3DArray& v = allPrimitives[i].GetVertices ();
      const Vector2DArray& uv = allPrimitives[i].GetUVs ();
      const IntArray& extra = allPrimitives[i].GetExtraData ();

      verts[j] = v[0];
      tc[j] = uv[0];
      factNormals[j] = normals[extra[0]];
      tris[i].a = j++;

      verts[j] = v[1];
      tc[j] = uv[1];
      factNormals[j] = normals[extra[1]];
      tris[i].b = j++;

      verts[j] = v[2];
      tc[j] = uv[2];
      factNormals[j] = normals[extra[2]];
      tris[i].c = j++;
    }

    RadObjectFactory::SaveFactory (node);
  }
}
