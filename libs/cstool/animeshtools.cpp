/*
  Copyright (C) 2011 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include "cssysdef.h"

#include "iengine/engine.h"
#include "iengine/mesh.h"
#include "imap/loader.h"
#include "imesh/animesh.h"
#include "imesh/genmesh.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/stringarray.h"
#include "iutil/vfs.h"

#include "csgfx/renderbuffer.h"
#include "cstool/vfsdirchange.h"
#include "csutil/regexp.h"
#include "csutil/stringquote.h"

#include "cstool/animeshtools.h"

namespace CS {
namespace Mesh {

csPtr<iAnimatedMeshFactory> AnimatedMeshTools::LoadAnimesh
(iObjectRegistry* object_reg, iLoader* loader, const char* factoryName,
 const char* filename)
{
  // Load the base mesh
  csLoadResult result = loader->Load (filename);
  if (!result.success)
  {
    ReportWarning ("Could not load mesh from file %s!",
		   CS::Quote::Single (filename));
    return csPtr<iAnimatedMeshFactory> (nullptr);
  }

  // Search for the factory wrapper
  csRef<iMeshFactoryWrapper> factoryWrapper =
    scfQueryInterface<iMeshFactoryWrapper> (result.result);
  if (!factoryWrapper)
  {
    ReportWarning ("The object loaded from file %s is not a mesh factory!",
		   CS::Quote::Single (filename));
    return csPtr<iAnimatedMeshFactory> (nullptr);
  }

  // Find the animesh interface
  csRef<iAnimatedMeshFactory> factory =
    scfQueryInterface<CS::Mesh::iAnimatedMeshFactory>
    (factoryWrapper->GetMeshObjectFactory ());
  if (!factory)
  {
    // If it is a genmesh then convert it to an animesh
    csRef<iGeneralFactoryState> gmstate =
      scfQueryInterface<iGeneralFactoryState> (factoryWrapper->GetMeshObjectFactory ());

    if (gmstate)
      factory = ImportGeneralMesh (object_reg, gmstate, true);

    else
    {
      ReportWarning ("The mesh loaded from file %s is neither a genmesh nor an animesh!",
		     CS::Quote::Single (filename));
      return csPtr<iAnimatedMeshFactory> (nullptr);
    }
  }

  return csPtr<iAnimatedMeshFactory> (factory);
}

csPtr<iAnimatedMeshFactory> AnimatedMeshTools::ImportSplitMorphMesh
(iObjectRegistry* object_reg, const char* path, const char* baseMesh,
 const char* factoryName, const char* meshMask)
{

  // Change the current working directory to the given path
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);
  csVfsDirectoryChanger diretoryChanger (vfs);
  if (path)
  {
    if (!diretoryChanger.ChangeToFull (path))
    {
      ReportError ("Could not find path %s!", CS::Quote::Single (path));
      return csPtr<iAnimatedMeshFactory> (nullptr);
    }
  }

  // Find the main loader
  csRef<iLoader> loader = csQueryRegistry<iLoader> (object_reg);
  if (!loader)
  {
    ReportError ("Could not find the main loader!");
    return csPtr<iAnimatedMeshFactory> (nullptr);
  }

  // Load the base mesh
  csRef<iAnimatedMeshFactory> meshFact =
    LoadAnimesh (object_reg, loader, factoryName, baseMesh);
  if (!meshFact)
  {
    ReportError ("Error loading base mesh %s!",
		 CS::Quote::Single (baseMesh));
    return csPtr<iAnimatedMeshFactory> (nullptr);
  }

  // Find the base name of the base mesh
  csString baseName = baseMesh;
  csString suffix;
  size_t index = baseName.FindLast ('/');
  if (index != (size_t) -1)
    baseName = baseName.Slice (index + 1);
  index = baseName.FindLast ('.');
  if (index != (size_t) -1)
  {
    suffix = baseName.Slice (index + 1);
    baseName = baseName.Slice (0, index);
  }
  csRef<iDataBuffer> dataBuffer = vfs->GetRealPath (baseMesh);
  csString realBaseFile = dataBuffer->GetData ();

  // If the mask is empty then build a new one based the base name
  csString mask = meshMask;
  if (mask == "")
    mask = baseName + ".*[.]" + suffix;

  // Load all animesh files matching the mask
  csRef<iStringArray> files = vfs->FindFiles (path);
  csRegExpMatcher matcher (mask);
  for (size_t i = 0; i < files->GetSize (); i++)
  {
    // Check if the file matches the mask
    if (matcher.Match (files->Get (i)) != csrxNoError) 
      continue;

    // Check that it is not the base file
    dataBuffer = vfs->GetRealPath (files->Get (i));
    csString txt = dataBuffer->GetData ();
    if (txt == realBaseFile)
      continue;

    // Load the animesh
    // TODO: don't load other data than the indices
    csRef<iAnimatedMeshFactory> morphFact =
      LoadAnimesh (object_reg, loader, factoryName, files->Get (i));
    if (!morphFact)
      continue;

    // Find the name of the morph target
    csString name = files->Get (i);
    size_t index = name.FindLast ('/');
    if (index != (size_t) -1)
      name = name.Slice (index + 1);
    name = name.ReplaceAll (baseName, "");
    index = name.FindLast ('.');
    if (index != (size_t) -1)
      name = name.Slice (0, index);
    if (name.StartsWith ("_"))
      name = name.Slice (1);

    // Merge it with the base mesh
    if (!ImportMorphMesh (object_reg, meshFact, morphFact, name, true))
      continue;
  }

  return csPtr<iAnimatedMeshFactory> (meshFact);
}

bool AnimatedMeshTools::ImportMorphMesh
(iObjectRegistry* object_reg, iAnimatedMeshFactory* baseMesh,
 iAnimatedMeshFactory* morphMesh, const char* morphName, bool deleteMesh)
{
  // Find a pointer to the engine
  csRef<iEngine> engine;
  if (deleteMesh)
  {
    engine = csQueryRegistry<iEngine> (object_reg);
    if (!engine)
    {
      ReportError ("Could not find the engine in order to delete the imported genmesh");
      deleteMesh = false;
    }
  }

  // Check that the base mesh has some vertices
  if (!baseMesh->GetVertexCount ())
  {
    ReportWarning
      ("The base animesh has no vertices!",
       CS::Quote::Single (morphName));

    // Delete the mesh if needed
    if (deleteMesh)
    {
      csRef<iMeshObjectFactory> factory =
	scfQueryInterface<iMeshObjectFactory> (morphMesh);
      engine->GetMeshFactories ()->Remove (factory->GetMeshFactoryWrapper ());
    }

    return false;
  }

  // Check that the two meshes have the same count of vertices
  if (baseMesh->GetVertexCount () != morphMesh->GetVertexCount ())
  {
    ReportWarning
      ("The animesh for the morph target %s has a different count of vertices (%i VS %i)!",
       CS::Quote::Single (morphName), baseMesh->GetVertexCount (), morphMesh->GetVertexCount ());

    // Delete the mesh if needed
    if (deleteMesh)
    {
      csRef<iMeshObjectFactory> factory =
	scfQueryInterface<iMeshObjectFactory> (morphMesh);
      engine->GetMeshFactories ()->Remove (factory->GetMeshFactoryWrapper ());
    }

    return false;
  }

  // Setup the offsets of the morph target
  iRenderBuffer* baseBuffer = baseMesh->GetVertices ();
  csVector3* baseIndices = (csVector3*) baseBuffer->Lock (CS_BUF_LOCK_READ);

  csRef<iRenderBuffer> morphBuffer;
  csRef<iRenderBuffer> initialMorphBuffer;
  csVector3* initialMorphIndices;

  if (deleteMesh)
    morphBuffer = morphMesh->GetVertices ();
  else
  {
    initialMorphBuffer = morphMesh->GetVertices ();
    initialMorphIndices = (csVector3*) initialMorphBuffer->Lock (CS_BUF_LOCK_READ);
    morphBuffer = csRenderBuffer::CreateRenderBuffer
      (morphMesh->GetVertexCount (), CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
  }
  csVector3* morphIndices = (csVector3*) morphBuffer->Lock (CS_BUF_LOCK_NORMAL);

  for (size_t i = 0; i < baseMesh->GetVertexCount (); i++)
    if (deleteMesh)
      morphIndices[i] = morphIndices[i] - baseIndices[i];
    else
      morphIndices[i] = initialMorphIndices[i] - baseIndices[i];

  baseBuffer->Release ();
  morphBuffer->Release ();
  if (!deleteMesh)
    initialMorphBuffer->Release ();

  // Create the morph target
  iAnimatedMeshMorphTarget* target =
    baseMesh->CreateMorphTarget (morphName);
  target->SetVertexOffsets (morphBuffer);
  target->Invalidate ();

  // Delete the mesh if needed
  if (deleteMesh)
  {
    csRef<iMeshObjectFactory> factory =
      scfQueryInterface<iMeshObjectFactory> (morphMesh);
    engine->GetMeshFactories ()->Remove (factory->GetMeshFactoryWrapper ());
  }

  return true;
}

csPtr<iAnimatedMeshFactory> AnimatedMeshTools::ImportGeneralMesh
(iObjectRegistry* object_reg, iGeneralFactoryState* genmesh, bool deleteMesh)
{
  // Find a pointer to the engine
  csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
  if (!engine)
  {
    ReportError ("Could not find the engine");
    return csPtr<iAnimatedMeshFactory> (nullptr);
  }

  // Create the animesh factory
  csRef<iMeshObjectFactory> genmeshFactory =
    scfQueryInterface<iMeshObjectFactory> (genmesh);
  csRef<iMeshFactoryWrapper> factoryWrapper = engine->CreateMeshFactory
    ("crystalspace.mesh.object.animesh",
     genmeshFactory->GetMeshFactoryWrapper ()->QueryObject ()->GetName ());

  // Find the animesh interface
  csRef<iAnimatedMeshFactory> factory =
    scfQueryInterface<CS::Mesh::iAnimatedMeshFactory>
    (factoryWrapper->GetMeshObjectFactory ());

  // Copy the render buffers
  csRef<iRenderBuffer> buffer;
  if (genmesh->GetVertexCount ())
  {
    buffer = csRenderBuffer::CreateRenderBuffer
      (genmesh->GetVertexCount (), CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
    buffer->CopyInto (genmesh->GetVertices (), genmesh->GetVertexCount ());
    factory->SetVertices (buffer);
  }

  if (genmesh->GetNormals ())
  {
    buffer = csRenderBuffer::CreateRenderBuffer
      (genmesh->GetVertexCount (), CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
    buffer->CopyInto (genmesh->GetNormals (), genmesh->GetVertexCount ());
    factory->SetNormals (buffer);
  }

  if (genmesh->GetTexels ())
  {
    buffer = csRenderBuffer::CreateRenderBuffer
      (genmesh->GetVertexCount (), CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
    buffer->CopyInto (genmesh->GetTexels (), genmesh->GetVertexCount ());
    factory->SetTexCoords (buffer);
  }

  // Add all submeshes
  for (size_t i = 0; i < genmesh->GetSubMeshCount (); i++)
  {
    iGeneralMeshSubMesh* subMesh = genmesh->GetSubMesh (i);
    CS::Mesh::iAnimatedMeshSubMeshFactory* submeshFactory;

    if (deleteMesh)
      submeshFactory =
	factory->CreateSubMesh (subMesh->GetIndices (), subMesh->GetName (), true);

    else
    {
      // Copy the index buffer of the submesh
      csRef<iRenderBuffer> indices = subMesh->GetIndices ();
      buffer = csRenderBuffer::CreateIndexRenderBuffer
	(indices->GetElementCount (), CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT,
	 indices->GetRangeStart (), indices->GetRangeEnd ());

      csTriangle* triangleData = (csTriangle*) indices->Lock (CS_BUF_LOCK_NORMAL);
      buffer->CopyInto (triangleData, indices->GetElementCount ());
      indices->Release ();

      // Create the submesh
      submeshFactory = factory->CreateSubMesh (buffer, subMesh->GetName (), true);
    }

    // Setup the material of the submesh
    submeshFactory->SetMaterial (subMesh->GetMaterial ());
    if (!i)
      factoryWrapper->GetMeshObjectFactory ()->SetMaterialWrapper (subMesh->GetMaterial ());
  }

  factory->Invalidate ();

  // Delete the genmesh if needed
  if (deleteMesh)
  {
    csRef<iMeshObjectFactory> factory =
      scfQueryInterface<iMeshObjectFactory> (genmesh);
    engine->GetMeshFactories ()->Remove (factory->GetMeshFactoryWrapper ());
  }

  return csPtr<iAnimatedMeshFactory> (factory);
}

} //namespace Mesh
} //namespace CS
