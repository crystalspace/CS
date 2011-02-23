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
#include "imap/modelload.h"
#include "imesh/animesh.h"
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

csRef<iAnimatedMeshFactory> AnimatedMeshTools::LoadAnimesh
(iModelLoader* loader, const char* factoryName, const char* filename)
{
  // Load the base mesh
  csRef<iMeshFactoryWrapper> factoryWrapper =
    loader->Load (factoryName, filename);
  if (!factoryWrapper)
  {
    ReportWarning ("Could not load mesh from file %s!",
		   CS::Quote::Single (filename));
    return (iAnimatedMeshFactory*) nullptr;
  }

  // Find the animesh interface
  csRef<iAnimatedMeshFactory> factory =
    scfQueryInterface<CS::Mesh::iAnimatedMeshFactory>
    (factoryWrapper->GetMeshObjectFactory ());
  if (!factory)
  {
    ReportWarning ("The mesh loaded from file %s is not an animesh!",
		   CS::Quote::Single (filename));
    return (iAnimatedMeshFactory*) nullptr;
  }

  return factory;
}

iAnimatedMeshFactory* AnimatedMeshTools::ImportSplittedMesh
(iObjectRegistry* object_reg, const char* path, const char* baseMesh,
 const char* meshMask, const char* factoryName)
{
  // Change the current working directory to the given path
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);
  csVfsDirectoryChanger diretoryChanger (vfs);
  if (!diretoryChanger.ChangeToFull (path))
  {
    ReportError ("Could not find path %s!", CS::Quote::Single (path));
    return nullptr;
  }

  // Temporary hack: use explicitely the assimp loader
  csRef<iPluginManager> plugmgr = 
    csQueryRegistry<iPluginManager> (object_reg);
  csRef<iModelLoader> loader =
    csLoadPlugin<iModelLoader> (plugmgr, "crystalspace.mesh.loader.assimp");

  // Load the base mesh
  csRef<iAnimatedMeshFactory> meshFact =
    LoadAnimesh (loader, factoryName, baseMesh);
  if (!meshFact)
  {
    ReportError ("Error loading base mesh %s!",
		 CS::Quote::Single (baseMesh));
    return nullptr;
  }

  // Find the base name of base mesh
  csString baseName = baseMesh;
  size_t index = baseName.FindLast ('/');
  if (index != (size_t) -1)
    baseName = baseName.Slice (index + 1);
  index = baseName.FindLast ('.');
  if (index != (size_t) -1)
    baseName = baseName.Slice (0, index);
  csRef<iDataBuffer> dataBuffer = vfs->GetRealPath (baseMesh);
  csString realBaseName = dataBuffer->GetData ();

  // Load all animesh files matching the mask
  csRef<iStringArray> files = vfs->FindFiles (path);
  csRegExpMatcher matcher (meshMask);
  for (size_t i = 0; i < files->GetSize (); i++)
  {
    // Check if the file matches the mask
    if (matcher.Match (files->Get (i)) != csrxNoError) 
      continue;

    // Check that it is not the base file
    dataBuffer = vfs->GetRealPath (files->Get (i));
    csString txt = dataBuffer->GetData ();
    if (txt == realBaseName)
      continue;

    // Load the animesh
    // TODO: don't load other data than the indices
    csRef<iAnimatedMeshFactory> morphFact =
      LoadAnimesh (loader, factoryName, files->Get (i));
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

  return meshFact;
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
      ReportError ("Could not find the engine");
      return false;
    }
  }

  // Check that the two meshes have the same count of vertices
  if (baseMesh->GetVertexCount () != morphMesh->GetVertexCount ())
  {
    ReportWarning
      ("The animesh for the morph target %s has a different count of vertices!",
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
  //target->Invalidate ();

  // Delete the mesh if needed
  if (deleteMesh)
  {
    csRef<iMeshObjectFactory> factory =
      scfQueryInterface<iMeshObjectFactory> (morphMesh);
    engine->GetMeshFactories ()->Remove (factory->GetMeshFactoryWrapper ());
  }

  return true;
}

} //namespace Mesh
} //namespace CS
