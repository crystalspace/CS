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

#include "csgeom/tri.h"
#include "csgfx/renderbuffer.h"
#include "cstool/rbuflock.h"
#include "csutil/cscolor.h"
#include "csutil/csstring.h"
#include "csutil/databuf.h"
#include "csutil/scfstr.h"

#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "igraphic/imageio.h"
#include "imap/ldrctxt.h"
#include "imesh/animesh.h"
#include "imesh/genmesh.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivaria/reporter.h"
#include "ivideo/txtmgr.h"

#include "assimpldr.h"

CS_PLUGIN_NAMESPACE_BEGIN(AssimpLoader)
{

  void AssimpLoader::ImportAnimesh (aiNode* node)
  {
    // Create the animesh data
    AnimeshData animeshData;
    animeshData.rootNode = node;

    // Create the animesh factory
    csRef<iMeshFactoryWrapper> factoryWrapper =
      engine->CreateMeshFactory
      ("crystalspace.mesh.object.animesh", node->mName.data);
    loaderContext->AddToCollection
      (factoryWrapper->QueryObject ());
    animeshData.factory =
      scfQueryInterface<CS::Mesh::iAnimatedMeshFactory>
      (factoryWrapper->GetMeshObjectFactory ());

    if (!firstMesh)
      firstMesh = factoryWrapper;

    // Register the bone nodes
    InitBoneNode (&animeshData, node);

    // Pre-process the mesh tree
    PreProcessAnimesh (&animeshData, node);

    // Import the animesh and all its submeshes
    ImportAnimeshSubMesh (&animeshData, node);

    // Create a render buffer for all types of parameters
    csRef<iRenderBuffer> buffer;
    buffer = FillBuffer<float> (animeshData.vertices,
				CS_BUFCOMP_FLOAT, 3, false);
    animeshData.factory->SetVertices (buffer);

    if (animeshData.hasColors)
    {
      buffer = FillBuffer<float> (animeshData.colors,
				  CS_BUFCOMP_FLOAT, 4, false);
      animeshData.factory->SetColors (buffer);
    }

    if (animeshData.hasNormals)
    {
      buffer = FillBuffer<float> (animeshData.normals,
				  CS_BUFCOMP_FLOAT, 3, false);
      animeshData.factory->SetNormals (buffer);
    }

    if (animeshData.hasTangents)
    {
      buffer = FillBuffer<float> (animeshData.tangents,
				  CS_BUFCOMP_FLOAT, 3, false);
      animeshData.factory->SetTangents (buffer);

      buffer = FillBuffer<float> (animeshData.binormals,
				  CS_BUFCOMP_FLOAT, 3, false);
      animeshData.factory->SetBinormals (buffer);
    }

    if (animeshData.hasTexels)
    {
      buffer = FillBuffer<float> (animeshData.texels,
				  CS_BUFCOMP_FLOAT, 2, false);
      animeshData.factory->SetTexCoords (buffer);
    }

    // Setup the material of the animesh as the first material
    // encountered in the submeshes
    for (size_t i = 0;
	 i < animeshData.factory->GetSubMeshCount (); i++)
    {
      iMaterialWrapper* material =
	animeshData.factory->GetSubMesh (i)->GetMaterial ();
      if (material)
      {
	factoryWrapper->GetMeshObjectFactory ()
	  ->SetMaterialWrapper (material);
	break;
      }
    }

    // Check if there is any bone in the tree
    bool hasBones = false;
    for (csHash<BoneData, csString>::GlobalIterator it = animeshData.boneNodes.GetIterator ();
	 it.HasNext (); )
    {
      csTuple2<BoneData, csString> tuple = it.NextTuple ();
      if (tuple.first.isBone)
      {
	hasBones = true;
	break;
      }
    }

    // Create the skeleton if needed
    if (hasBones)
    {
      // Find the skeleton manager
      csRef<CS::Animation::iSkeletonManager> skeletonManager =
	csQueryRegistryOrLoad<CS::Animation::iSkeletonManager>
	(object_reg, "crystalspace.skeletalanimation");
      if (!skeletonManager)
	ReportWarning (object_reg, "Could not find the skeleton manager");

      else
      {
	// Create the skeleton
	CS::Animation::iSkeletonFactory* skeletonFactory =
	  skeletonManager->CreateSkeletonFactory (animeshData.rootNode->mName.data);
	animeshData.factory->SetSkeletonFactory (skeletonFactory);
      }
    }

    // Import the skeleton
    if (hasBones)
    {
      // Initialize the bone influences
      CS::Mesh::csAnimatedMeshBoneInfluence* influences =
	animeshData.factory->GetBoneInfluences ();
      for (size_t i = 0; i < animeshData.factory->GetVertexCount ()
	     * animeshData.factory->GetBoneInfluencesPerVertex (); i++)
      {
	influences[i].bone = 0;
	influences[i].influenceWeight = 0.0f;
      }

      // Import the skeleton
      csQuaternion rotation;
      csVector3 offset (0.0f);
      ImportAnimeshSkeleton
	(&animeshData, node, CS::Animation::InvalidBoneID, rotation, offset);

      printf ("Skeleton: %i bones:\n%s\n",
	      animeshData.factory->GetSkeletonFactory()->GetTopBoneID (),
	      animeshData.factory->GetSkeletonFactory()->Description().GetData());
    }

    // Invalidate the data of the factory
    animeshData.factory->Invalidate ();
  }

  void AssimpLoader::InitBoneNode (AnimeshData* animeshData, aiNode* node)
  {
    // Add an entry in the registered bone nodes
    BoneData boneData;
    boneData.isBone = false;
    boneData.node = node;
    animeshData->boneNodes.PutUnique (node->mName.data, boneData);

    // Init the child nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      aiNode*& subnode = node->mChildren[i];
      InitBoneNode (animeshData, subnode);
    }
  }

  void AssimpLoader::PreProcessAnimesh (AnimeshData* animeshData,
					aiNode* node)
  {
    // Check for the data of all meshes
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
      aiMesh*& mesh = scene->mMeshes[node->mMeshes[i]];

      // Check the validity of the mesh
      if (!mesh->HasFaces ()
	  || !mesh->HasPositions ())
	continue;

      // Process the mesh
      if (mesh->HasTextureCoords (0))
	animeshData->hasTexels = true;
      if (mesh->HasNormals ())
	animeshData->hasNormals = true;
      if (mesh->HasTangentsAndBitangents ())
	animeshData->hasTangents = true;
      if (mesh->HasVertexColors (0))
	animeshData->hasColors = true;

      // Check for the bones of this submesh
      for (unsigned int i = 0; i < mesh->mNumBones; i++)
      {
	aiBone*& bone = mesh->mBones[i];

	// Find the node corresponding to this bone and mark it as a bone
	BoneData* boneData = animeshData->boneNodes[bone->mName.data];
	if (!boneData)
	  continue;
	boneData->isBone = true;

	// Mark the parents of this node as bones
	aiNode* nodeIterator = boneData->node->mParent;
	while (nodeIterator)
	{
	  // Find the node corresponding to this bone and mark it as a bone
	  BoneData* boneData = animeshData->boneNodes[nodeIterator->mName.data];
	  if (!boneData)
	    break;
	  boneData->isBone = true;

	  // Check for the end of the iteration
	  nodeIterator = nodeIterator->mParent;
	  if (nodeIterator == animeshData->rootNode
	      || nodeIterator == animeshData->rootNode->mParent)
	    break;
	}
      }
    }

    // Pre-process the child nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      aiNode*& subnode = node->mChildren[i];
      PreProcessAnimesh (animeshData, subnode);
    }
  }

  void AssimpLoader::ImportAnimeshSubMesh (AnimeshData* animeshData, aiNode* node)
  {
    // Import all meshes of this node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
      aiMesh*& mesh = scene->mMeshes[node->mMeshes[i]];
      
      // Check the validity of the mesh
      if (!mesh->HasFaces ()
	  || !mesh->HasPositions ())
      {
	ReportWarning (object_reg,
		       "Skipping mesh %s for lack of vertices or triangles!",
		       CS::Quote::Single (mesh->mName.data));
	continue;
      }

      // Save the current count of vertices of the animesh
      size_t currentVertices = animeshData->vertices.GetSize () / 3;
      for (unsigned int i = 0; i < mesh->mNumBones; i++)
      {
	aiBone*& bone = mesh->mBones[i];

	BoneData* boneData = animeshData->boneNodes[bone->mName.data];
	if (boneData)
	  boneData->vertexIndex = currentVertices;
      }

      // Add all vertices
      aiVector3D* aiuvs = mesh->mTextureCoords[0];
      aiColor4D* aicolors = mesh->mColors[0];

      for (unsigned int i = 0; i < mesh->mNumVertices; i++)
      {
	animeshData->vertices.Push (mesh->mVertices[i][0]);
	animeshData->vertices.Push (mesh->mVertices[i][1]);
	animeshData->vertices.Push (mesh->mVertices[i][2]);

	if (mesh->HasTextureCoords (0))
	{
	  animeshData->texels.Push (aiuvs[i].x);
	  animeshData->texels.Push (aiuvs[i].y);
	}
	else if (animeshData->hasTexels)
	{
	  animeshData->texels.Push (0.0f);
	  animeshData->texels.Push (0.0f);
	}

	if (mesh->HasNormals ())
	{
	  animeshData->normals.Push (mesh->mNormals[i][0]);
	  animeshData->normals.Push (mesh->mNormals[i][1]);
	  animeshData->normals.Push (mesh->mNormals[i][2]);
	}
	else if (animeshData->hasNormals)
	{
	  animeshData->normals.Push (1.0f);
	  animeshData->normals.Push (0.0f);
	  animeshData->normals.Push (0.0f);
	}

	if (mesh->HasTangentsAndBitangents ())
	{
	  animeshData->tangents.Push (mesh->mTangents[i][0]);
	  animeshData->tangents.Push (mesh->mTangents[i][1]);
	  animeshData->tangents.Push (mesh->mTangents[i][2]);
	  animeshData->binormals.Push (mesh->mBitangents[i][0]);
	  animeshData->binormals.Push (mesh->mBitangents[i][1]);
	  animeshData->binormals.Push (mesh->mBitangents[i][2]);
	}
	else if (animeshData->hasTangents)
	{
	  animeshData->tangents.Push (1.0f);
	  animeshData->tangents.Push (0.0f);
	  animeshData->tangents.Push (0.0f);
	  animeshData->binormals.Push (1.0f);
	  animeshData->binormals.Push (0.0f);
	  animeshData->binormals.Push (0.0f);
	}

	if (mesh->HasVertexColors (0))
	{
	  animeshData->colors.Push (aicolors[i].r);
	  animeshData->colors.Push (aicolors[i].g);
	  animeshData->colors.Push (aicolors[i].b);
	  animeshData->colors.Push (aicolors[i].a);
	}
	else if (animeshData->hasColors)
	{
	  animeshData->colors.Push (1.0f);
	  animeshData->colors.Push (0.0f);
	  animeshData->colors.Push (0.0f);
	  animeshData->colors.Push (1.0f);
	}
      }

      // Create a render buffer for all faces
      csRef<csRenderBuffer> buffer =
	csRenderBuffer::CreateIndexRenderBuffer
	(mesh->mNumFaces * 3, CS_BUF_STATIC,
	 CS_BUFCOMP_UNSIGNED_INT, currentVertices,
	 animeshData->vertices.GetSize () - 1);

      csTriangle* triangleData =
	(csTriangle*) buffer->Lock (CS_BUF_LOCK_NORMAL);
      for (unsigned int i = 0; i < mesh->mNumFaces; i++)
      {
	aiFace& face = mesh->mFaces[i];
	triangleData[i] =
	  csTriangle (currentVertices + face.mIndices[0],
		      currentVertices + face.mIndices[1],
		      currentVertices + face.mIndices[2]);
      }
      buffer->Release ();

      // Create the submesh
      CS::Mesh::iAnimatedMeshSubMeshFactory* submesh =
	animeshData->factory->CreateSubMesh
	(buffer, mesh->mName.data, true);

      // Setup the material
      if (mesh->mMaterialIndex < materials.GetSize ())
      {
	csRef<iMaterialWrapper> material;
	material = materials[mesh->mMaterialIndex];
	submesh->SetMaterial (material);
      }
    }

    // Import all subnodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      aiNode*& subnode = node->mChildren[i];
      ImportAnimeshSubMesh (animeshData, subnode);
    }
  }

  void AssimpLoader::ImportAnimeshSkeleton (AnimeshData* animeshData, aiNode* node,
					    CS::Animation::BoneID parent,
					    csQuaternion rotation, csVector3 offset)
  {
    // Accumulate the transform from the parent bone
    aiQuaternion aiquaternion;
    aiVector3D aivector;
    node->mTransformation.DecomposeNoScaling (aiquaternion, aivector);
    offset = offset + rotation.Rotate (Assimp2CS (aivector));
    rotation = rotation * Assimp2CS (aiquaternion);

    // Create a bone if needed
    BoneData* boneData = animeshData->boneNodes[node->mName.data];
    if (boneData && boneData->isBone)
    {
      // Create the bone
      CS::Animation::iSkeletonFactory* skeletonFactory =
	animeshData->factory->GetSkeletonFactory ();
      boneData->boneID = skeletonFactory->CreateBone (parent);
      parent = boneData->boneID;
      skeletonFactory->SetBoneName (boneData->boneID, node->mName.data);
      skeletonFactory->SetTransformBoneSpace (boneData->boneID, rotation, offset);

      // Reset the transform from the parent bone
      rotation.SetIdentity ();
      offset.Set (0.0f);
    }


    // Import the bone influences fo all submeshes
    if (animeshData->vertices.GetSize () > 0)
    {
      CS::Mesh::csAnimatedMeshBoneInfluence* influences =
	animeshData->factory->GetBoneInfluences ();

      // Iterate on all meshes
      for (unsigned int i = 0; i < node->mNumMeshes; i++)
      {
	aiMesh*& mesh = scene->mMeshes[node->mMeshes[i]];

	// Iterate on all bones
	for (unsigned int i = 0; i < mesh->mNumBones; i++)
	{
	  aiBone*& bone = mesh->mBones[i];

	  BoneData* boneData = animeshData->boneNodes[bone->mName.data];
	  CS_ASSERT (boneData);

	  // Iterate on all bone weight
	  for (unsigned int i = 0; i < bone->mNumWeights; i++)
	  {
	    aiVertexWeight& weight = bone->mWeights[i];
	    size_t vertexIndex = boneData->vertexIndex + weight.mVertexId;

	    // Check for the first bone influence not yet used
	    for (uint i = 0; i < animeshData->factory->GetBoneInfluencesPerVertex ();
		 i++)
	    {
	      if (influences[vertexIndex].influenceWeight > SMALL_EPSILON)
	      {
		influences[vertexIndex].bone = boneData->boneID;
		influences[vertexIndex].influenceWeight = weight.mWeight;
		break;
	      }
	    }
	  }
	}
      }
    }

    // Import all subnodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      aiNode*& subnode = node->mChildren[i];
      ImportAnimeshSkeleton (animeshData, subnode, parent, rotation, offset);
    }
  }

  void AssimpLoader::ImportAnimation (aiAnimation* animation)
  {
    printf ("Animation [%s]:\n", animation->mName.data);
    for (unsigned int i = 0; i < animation->mNumChannels; i++)
      printf (" anim [%s]\n", animation->mChannels[i]->mNodeName.data);
  }

}
CS_PLUGIN_NAMESPACE_END(AssimpLoader)
