/*
  Copyright (C) 2011 Christian Van Brussel, Institute of Information
      and Communication Technologies, Electronics and Applied Mathematics
      at Universite catholique de Louvain, Belgium
      http://www.uclouvain.be/en-icteam.html

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
#include "imesh/animnode/debug.h"
#include "imesh/animnode/skeleton2anim.h"
#include "imesh/genmesh.h"
#include "iutil/document.h"
#include "iutil/object.h"
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
    csRef<iMeshFactoryWrapper> factoryWrapper = engine->CreateMeshFactory
      ("crystalspace.mesh.object.animesh", node->mName.data);
    loaderContext->AddToCollection (factoryWrapper->QueryObject ());
    animeshData.factory = scfQueryInterface<CS::Mesh::iAnimatedMeshFactory>
      (factoryWrapper->GetMeshObjectFactory ());

    // Create an entry in the list of imported meshes
    ImportedMesh importedMesh;
    importedMesh.factoryWrapper = factoryWrapper;
    importedMeshes.Push (importedMesh);
    animeshData.importedMesh = &importedMeshes[importedMeshes.GetSize () - 1];

    // Check if there is not yet any 'first mesh' defined
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
	animeshData.importedMesh->factoryWrapper->GetMeshObjectFactory ()
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

    // Import the skeleton and the bone influences
    if (hasBones && skeletonManager)
    {
      // Create the skeleton then import it
      CS::Animation::iSkeletonFactory* skeletonFactory =
	skeletonManager->CreateSkeletonFactory (animeshData.rootNode->mName.data);
      animeshData.factory->SetSkeletonFactory (skeletonFactory);

      ImportAnimeshSkeleton
	(&animeshData, node, CS::Animation::InvalidBoneID, aiMatrix4x4 ());

      printf ("Skeleton: %i bones:\n%s\n",
	      (int) animeshData.factory->GetSkeletonFactory()->GetTopBoneID (),
	      animeshData.factory->GetSkeletonFactory()->Description().GetData());
      
      if (animeshData.factory->GetVertexCount () > 0)
      {
	// Initialize the bone influences
	CS::Mesh::AnimatedMeshBoneInfluence* influences =
	  animeshData.factory->GetBoneInfluences ();
	for (size_t i = 0; i < animeshData.factory->GetVertexCount ()
	       * animeshData.factory->GetBoneInfluencesPerVertex (); i++)
	{
	  influences[i].bone = 0;
	  influences[i].influenceWeight = 0.0f;
	}

	// Import the bone influences
	ImportBoneInfluences (&animeshData, node);
      }
    }

    // Invalidate the data of the factory
    animeshData.factory->Invalidate ();
  }

  void AssimpLoader::InitBoneNode (AnimeshData* animeshData, aiNode* node)
  {
    // Add an entry in the registered bone nodes
    BoneData boneData;
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

	// Save the bone transform
	boneData->transform = bone->mOffsetMatrix;

	// Mark the parents of this node as bones
	aiNode* nodeIterator = boneData->node->mParent;
	while (nodeIterator)
	{
	  // TODO: only nodes marked as bones (but then need to update the animations)
	  // Find the node corresponding to this bone and mark it as a bone
	  BoneData* boneData = animeshData->boneNodes[nodeIterator->mName.data];
	  if (!boneData)
	    break;

	  // Check for the end of the iteration
	  if (nodeIterator->mParent == animeshData->rootNode
	      || nodeIterator->mParent == animeshData->rootNode->mParent)
	    break;

	  boneData->isBone = true;
	  nodeIterator = nodeIterator->mParent;
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
	ReportWarning ("Skipping mesh %s for lack of vertices or faces!",
		       CS::Quote::Single (mesh->mName.data));
	continue;
      }

      // Check the type of the primitives
      if (!(mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE))
      {
	// If these are not triangles then export them in an extra render mesh
	if (mesh->mPrimitiveTypes & aiPrimitiveType_POINT
	    || mesh->mPrimitiveTypes & aiPrimitiveType_LINE)
	  ImportExtraRenderMesh (animeshData->importedMesh, mesh);

	else ReportWarning ("Skipping mesh %s for lack of points, lines or triangles!",
			    CS::Quote::Single (mesh->mName.data));

	continue;
      }

      // Save the current count of vertices of the animesh
      size_t currentVertices = animeshData->vertices.GetSize () / 3;

      // Save the submesh state
      AnimeshData::AnimeshSubmesh animeshSubmesh;
      animeshSubmesh.vertexIndex = currentVertices;
      animeshData->submeshes.Put (mesh, animeshSubmesh);

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
      animeshData->importedMesh->meshes.Push (mesh);

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
					    aiMatrix4x4 transform)
  {
    // Accumulate the transform from the parent bone
    transform = transform * node->mTransformation;

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

      // Compute the bone transform
      // TODO: missing inverse mesh transform
      aiMatrix4x4 boneTransform = boneData->transform.Inverse ();

      // Convert it to a CS transform
      aiQuaternion aiquaternion;
      aiVector3D aivector;
      aiVector3D aiscale;
      boneTransform.Decompose (aiscale, aiquaternion, aivector);

      csQuaternion rotation = Assimp2CS (aiquaternion);
      csVector3 offset = Assimp2CS (aivector);
      offset[0] /= aiscale.x;
      offset[1] /= aiscale.y;
      offset[2] /= aiscale.z;

      // Apply the transform
      skeletonFactory->SetTransformAbsSpace (boneData->boneID, rotation, offset);

      // Register the node information
      AnimeshNode animeshNode;
      animeshNode.factory = animeshData->factory;
      animeshNode.boneID = boneData->boneID;
      animeshNodes.Put (node->mName.data, animeshNode);
    }

    // TODO: else skip branch

    // Import all subnodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      aiNode*& subnode = node->mChildren[i];
      ImportAnimeshSkeleton (animeshData, subnode, parent, transform);
    }
  }

  void AssimpLoader::ImportBoneInfluences (AnimeshData* animeshData, aiNode* node)
  {
    // Import the bone influences fo all submeshes
    CS::Mesh::AnimatedMeshBoneInfluence* influences =
      animeshData->factory->GetBoneInfluences ();

    // Iterate on all meshes
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
      aiMesh*& mesh = scene->mMeshes[node->mMeshes[i]];
      size_t submeshVertexIndex = animeshData->submeshes[mesh]->vertexIndex;

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
	  size_t vertexIndex = (submeshVertexIndex + weight.mVertexId)
	    * animeshData->factory->GetBoneInfluencesPerVertex ();

	  // Check for the first bone influence not yet used
	  for (uint i = 0; i < animeshData->factory->GetBoneInfluencesPerVertex ();
	       i++)
	  {
	    if (influences[vertexIndex + i].influenceWeight < SMALL_EPSILON)
	    {
	      influences[vertexIndex + i].bone = boneData->boneID;
	      influences[vertexIndex + i].influenceWeight = weight.mWeight;
	      break;
	    }
	  }
	}
      }
    }

    // Import all subnodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      aiNode*& subnode = node->mChildren[i];
      ImportBoneInfluences (animeshData, subnode);
    }
  }

  void AssimpLoader::ImportAnimation (aiAnimation* animation, size_t index)
  {
    if (!skeletonManager)
      return;

    // Give a unique name (let's hope non empty names are unique)
    csString animationName = animation->mName.data;
    if (!strlen (animation->mName.data))
    {
      animationName = "animation ";
      animationName += index + 1;
    }

    for (unsigned int i = 0; i < animation->mNumChannels; i++)
    {
      aiNodeAnim* channel = animation->mChannels[i];

      // Search the animesh and bone ID for this animation channel
      AnimeshNode* animeshNode = animeshNodes[channel->mNodeName.data];
      if (!animeshNode)
	continue;

      // Create the animation packet if not yet made
      CS::Animation::iSkeletonFactory* skeletonFactory =
	animeshNode->factory->GetSkeletonFactory ();
      if (!skeletonFactory)
	continue;

      CS::Animation::iSkeletonAnimPacketFactory* animationPacket =
	skeletonFactory->GetAnimationPacket ();

      if (!animationPacket)
      {
	// Get the name of the animesh
	csRef<iMeshObjectFactory> meshFactory =
	  scfQueryInterface<iMeshObjectFactory> (animeshNode->factory);
	const char* name = meshFactory->GetMeshFactoryWrapper ()->QueryObject ()->GetName ();

	animationPacket = skeletonManager->CreateAnimPacketFactory (name);
	skeletonFactory->SetAnimationPacket (animationPacket);
      }

      // Create the animation if not yet made
      CS::Animation::iSkeletonAnimation* skeletonAnimation =
	animationPacket->FindAnimation (animationName);

      if (!skeletonAnimation)
      {
	skeletonAnimation = animationPacket->CreateAnimation (animationName);

	// create an animation node for this animation
	csRef<CS::Animation::iSkeletonAnimationNodeFactory> animationNode =
	  animationPacket->CreateAnimationNode (animationName);
	animationNode->SetAnimation (skeletonAnimation);
	animationNode->SetCyclic (true);

	// Temporary debug: set a debug node as the root of the animation tree
	csRef<CS::Animation::iSkeletonDebugNodeManager> debugNodeManager =
	  csQueryRegistryOrLoad<CS::Animation::iSkeletonDebugNodeManager>
	  (object_reg, "crystalspace.mesh.animesh.animnode.debug");
	if (!debugNodeManager)
	{
	  ReportWarning ("Failed to locate debug node plugin!");
	  animationPacket->SetAnimationRoot (animationNode);
	}

	else
	{
	  csRef<CS::Animation::iSkeletonDebugNodeFactory> debugNode =
	    debugNodeManager->CreateAnimNodeFactory ("debug");
	  debugNode->SetDebugModes
	    ((CS::Animation::SkeletonDebugMode) (CS::Animation::DEBUG_2DLINES
						 | CS::Animation::DEBUG_SQUARES));
	  debugNode->SetChildNode (animationNode);
	  animationPacket->SetAnimationRoot (debugNode);
	}
      }

      // Create the animation channel
      CS::Animation::ChannelID channelID = skeletonAnimation->AddChannel (animeshNode->boneID);

      // Add all rotation keyframes
      for (unsigned int i = 0; i < channel->mNumRotationKeys; i++)
      {
	aiQuatKey& key = channel->mRotationKeys[i];

	// Don't accept negative time values
	if (key.mTime < 0.0f)
	  continue;

	// Check ticks are not null
	float time = animation->mTicksPerSecond > SMALL_EPSILON ?
	  key.mTime / animation->mTicksPerSecond : key.mTime;

	// Add the keyframe
	skeletonAnimation->AddOrSetKeyFrame (channelID, time, Assimp2CS (key.mValue));
	// TODO: the frames without position component are invalid
      }

      // Add all position keyframes
      for (unsigned int i = 0; i < channel->mNumPositionKeys; i++)
      {
	aiVectorKey& key = channel->mPositionKeys[i];

	// Don't accept negative time values
	if (key.mTime < 0.0f)
	  continue;

	// Check ticks are not null
	float time = animation->mTicksPerSecond > SMALL_EPSILON ?
	  key.mTime / animation->mTicksPerSecond : key.mTime;

	// Add the keyframe
	// TODO: really need to scale the offset?
	skeletonAnimation->AddOrSetKeyFrame (channelID, time, Assimp2CS (key.mValue));
      }
    }
  }

  void AssimpLoader::ConvertAnimationFrames ()
  {
    for (csHash<AnimeshNode, csString>::GlobalIterator it = animeshNodes.GetIterator ();
	 it.HasNext (); )
    {
      AnimeshNode& animeshNode = it.Next ();

      CS::Animation::iSkeletonFactory* skeletonFactory =
	animeshNode.factory->GetSkeletonFactory ();
      if (!skeletonFactory)
	continue;

      CS::Animation::iSkeletonAnimPacketFactory* animationPacket =
	skeletonFactory->GetAnimationPacket ();
      if (!animationPacket)
	continue;

      for (size_t i = 0; i < animationPacket->GetAnimationCount (); i++)
	animationPacket->GetAnimation (i)->ConvertFrameSpace (skeletonFactory);
    }
  }
}
CS_PLUGIN_NAMESPACE_END(AssimpLoader)
