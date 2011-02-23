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

#include <iutil/stringarray.h>

#include "assimpldr.h"

CS_PLUGIN_NAMESPACE_BEGIN(AssimpLoader)
{

  template<typename T>
  static csRef<iRenderBuffer> FillBuffer
    (csDirtyAccessArray<T>& buf, csRenderBufferComponentType compType,
     int componentNum, bool indexBuf)
  {
    csRef<iRenderBuffer> buffer;
    size_t bufElems = buf.GetSize() / componentNum;
    if (indexBuf)
    {
      T min;
      T max;
      size_t i = 0;
      size_t n = buf.GetSize(); 
      if (n & 1)
      {
	min = max = csMax (buf[0], T (0));
	i++;
      }
      else
      {
	min = T (INT_MAX);
	max = 0;
      }
      for (; i < n; i += 2)
      {
	T a = buf[i]; T b = buf[i+1];
	if (a < b)
	{
	  min = csMin (min, a);
	  max = csMax (max, b);
	}
	else
	{
	  min = csMin (min, b);
	  max = csMax (max, a);
	}
      }
      buffer = csRenderBuffer::CreateIndexRenderBuffer
	(bufElems, CS_BUF_STATIC, compType,
	 size_t (min), size_t (max));
    }
    else
    {
      buffer = csRenderBuffer::CreateRenderBuffer
	(bufElems, CS_BUF_STATIC, compType, (uint)componentNum);
    }
    buffer->CopyInto (buf.GetArray(), bufElems);

    return buffer;
  }
 
  void PrintMesh (aiMesh* mesh, const char* prefix)
  {
    printf ("%s  mesh [%s]: %i vertices %i triangles %i bones\n",
	    prefix, mesh->mName.data, mesh->mNumVertices,
	    mesh->mNumFaces, mesh->mNumBones);
  }

  void PrintNode (const aiScene* scene, aiNode* node,
		  const char* prefix)
  {
    printf ("%s+ node [%s] [%s]\n", prefix, node->mName.data,
	    node->mTransformation.IsIdentity ()
	    ? "unmoved" : "moved");

    for (unsigned int i = 0; i < node->mNumMeshes; i++)
      PrintMesh (scene->mMeshes[node->mMeshes[i]], prefix);

    csString pref = prefix;
    pref += " ";

    for (unsigned int i = 0; i < node->mNumChildren; i++)
      PrintNode (scene, node->mChildren[i], pref.GetData ());
  }

  inline csVector3 Assimp2CS (aiVector3D v)
  {
    return csVector3 (v.x, v.y, v.z);
  }

  inline csColor4 Assimp2CS (aiColor4D c)
  {
    return csColor4 (c.r, c.g, c.b, c.a);
  }

  inline csTriangle Assimp2CS (unsigned int* t)
  {
    return csTriangle (t[0], t[1], t[2]);
  }

  /**
   * Error reporting
   */
  static void ReportError (iObjectRegistry* objreg,
			   const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    csReportV (objreg, CS_REPORTER_SEVERITY_ERROR,
	       "crystalspace.mesh.loader.factory.assimp",
	       description, arg);
    va_end (arg);
  }

  static void ReportWarning (iObjectRegistry* objreg,
			     const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    csReportV (objreg, CS_REPORTER_SEVERITY_WARNING,
	       "crystalspace.mesh.loader.factory.assimp",
	       description, arg);
    va_end (arg);
  }

  SCF_IMPLEMENT_FACTORY (AssimpLoader);

  AssimpLoader::AssimpLoader (iBase* pParent) :
    scfImplementationType (this, pParent)
  {
    importFlags = aiProcess_CalcTangentSpace
      | aiProcess_Triangulate
      | aiProcess_ValidateDataStructure
      | aiProcess_GenUVCoords
      | aiProcess_FlipUVs
      | aiProcess_SortByPType
      | aiProcess_LimitBoneWeights
      //| aiProcess_OptimizeGraph
      //| aiProcess_OptimizeMeshes
      | aiProcess_SplitLargeMeshes
      | aiProcess_SortByPType;
  }

  AssimpLoader::~AssimpLoader ()
  {
  }

  bool AssimpLoader::Initialize (iObjectRegistry* object_reg)
  {
    AssimpLoader::object_reg = object_reg;
    return true;
  }

  bool AssimpLoader::IsRecognized (const char* filename)
  {
    // TODO
    return true;
  }

  bool AssimpLoader::IsRecognized (iDataBuffer* buffer)
  {
    // TODO
    return true;
  }

  csPtr<iBase> AssimpLoader::Parse
    (iDataBuffer* buffer, iStreamSource*,
     iLoaderContext* ldr_context, iBase* context,
     iStringArray* failed)
  {
    loaderContext = ldr_context;

    // Create an Assimp importer and parse the file
    Assimp::Importer importer;
    scene = importer.ReadFileFromMemory
      (**buffer, buffer->GetSize (), importFlags, "");

    // If the import failed, report it
    if (!scene)
    {
      ReportError (object_reg, "Failed to load binary file: %s!",
		   importer.GetErrorString());
      return (iBase*) nullptr;
    }

    // Import the scene into CS
    ImportScene ();

    // TODO: list of failed factories

    firstMesh->IncRef ();
    return csPtr<iBase> (firstMesh);
  }

  iMeshFactoryWrapper* AssimpLoader::Load (const char* factname,
					   iDataBuffer* buffer)
  {
    // Create an Assimp importer and parse the file
    Assimp::Importer importer;
    scene = importer.ReadFileFromMemory
      (**buffer, buffer->GetSize (), importFlags, "");

    // If the import failed, report it
    if (!scene)
    {
      ReportError (object_reg, "Failed to load factory %s: %s!",
		   CS::Quote::Single (factname),
		   importer.GetErrorString());
      return nullptr;
    }

    // Import the scene into CS
    ImportScene ();

    return firstMesh;
  }

  iMeshFactoryWrapper* AssimpLoader::Load (const char* factname,
					   const char* filename)
  {
    // TODO: custom options: scale, genmesh/animesh/scene/factories,
    //   find duplicates/optimize
    // TODO: if forced to be a genmesh then don't read animations,
    //   weights, etc

    // TODO: use ASSIMP::IOStream
    csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);
    csRef<iDataBuffer> path = vfs->GetRealPath (filename);

    // Create an Assimp importer and parse the file
    Assimp::Importer importer;
    scene = importer.ReadFile (path->GetData (), importFlags);

    // If the import failed, report it
    if (!scene)
    {
      ReportError (object_reg,
		   "Failed to load factory %s from file %s: %s!",
		   CS::Quote::Single (factname),
		   CS::Quote::Single (filename),
		   importer.GetErrorString());
      return nullptr;
    }

    // Import the scene into CS
    ImportScene ();

    return firstMesh;
  }

  void AssimpLoader::ImportScene ()
  {
    printf ("Loading OK!\n");
    printf ("animations: %i\n", scene->mNumAnimations);
    printf ("meshes: %i\n", scene->mNumMeshes);
    printf ("materials: %i\n", scene->mNumMaterials);
    printf ("camera: %i\n", scene->mNumCameras);
    printf ("textures: %i\n", scene->mNumTextures);
    printf ("lights: %i\n\n", scene->mNumLights);

    PrintNode (scene, scene->mRootNode, "");

    textures.DeleteAll ();
    materials.DeleteAll ();
    firstMesh = nullptr;

    /// Find pointers to engine data
    engine = csQueryRegistry<iEngine> (object_reg);
    if (!engine)
    {
      ReportError (object_reg, "Could not find the engine");
      return;
    }

    g3d = csQueryRegistry<iGraphics3D> (object_reg);
    if (!g3d)
    {
      ReportError (object_reg, "Could not find the 3D graphics");
      return;
    }

    textureManager = g3d->GetTextureManager();
    if (!textureManager)
    {
      ReportError (object_reg,
		   "Could not find the texture manager");
      return;
    }

    imageLoader = csQueryRegistry<iImageIO> (object_reg);
    if (!imageLoader)
    {
      ReportError (object_reg,
		   "Failed to find an image loader!");
      return;
    }

    // Create the loader context if needed
    if (!loaderContext)
      loaderContext = engine->CreateLoaderContext ();

    // Import all textures
    textures.SetSize (scene->mNumTextures);
    for (unsigned int i = 0; i < scene->mNumTextures; i++)
    {
      aiTexture*& texture = scene->mTextures[i];
      ImportTexture (texture, i);
    }

    // Import all materials
    materials.SetSize (scene->mNumMaterials);
    for (unsigned int i = 0; i < scene->mNumMaterials; i++)
    {
      aiMaterial*& material = scene->mMaterials[i];
      ImportMaterial (material, i);
    }

    // Import all meshes
    if (scene->mRootNode)
      //ImportGenmesh (scene->mRootNode);
      ImportAnimesh (scene->mRootNode);

    /*
    for (unsigned int i = 0; i < scene->mRootNode->mNumChildren; i++)
    {
      // Check the type of the mesh then import it
      // TODO: animeshes, terrains, whole scene (lights, cameras)
      aiNode*& node = scene->mRootNode->mChildren[i];
      //ImportGenmesh (node);
      ImportAnimesh (node);
    }
    */

    // Import all animations
    for (unsigned int i = 0; i < scene->mNumAnimations; i++)
    {
      aiAnimation*& animation = scene->mAnimations[i];
      ImportAnimation (animation);
    }
  }

  iTextureWrapper* AssimpLoader::FindTexture (const char* filename)
  {
    csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);

    // Search in the loading context
    iTextureWrapper* texture =
      loaderContext->FindTexture (filename, true);    
    if (texture)
      return texture;

    // Load manually the file
    csRef<iDataBuffer> buffer = vfs->ReadFile (filename, false);
    if (!buffer)
    {
      ReportWarning (object_reg, "Could not find image file %s!",
		     CS::Quote::Single (filename));
      return nullptr;
    }

    return LoadTexture (buffer, filename);
  }

  iTextureWrapper* AssimpLoader::LoadTexture (iDataBuffer* buffer,
					      const char* filename)
  {
    // Load the image data
    int format = engine->GetTextureFormat ();
    csRef<iImage> image (imageLoader->Load (buffer, format));
    if (!image)
    {
      ReportWarning (object_reg,
		     "Could not load image %s. Unknown format!",
		     CS::Quote::Single (filename));
      return nullptr;
    }

    // Create the texture handle
    csRef<scfString> fail_reason;
    fail_reason.AttachNew (new scfString ());
    csRef<iTextureHandle> textureHandle
      (textureManager->RegisterTexture
       (image, CS_TEXTURE_2D, fail_reason));
    if (!textureHandle)
    {
      ReportError (object_reg, "crystalspace.imagetextureloader",
		   "Error creating texture",
		   fail_reason->GetData ());
      return nullptr;
    }

    // Create the texture wrapper
    csRef<iTextureWrapper> textureWrapper =
      engine->GetTextureList ()->CreateTexture (textureHandle);
    engine->GetTextureList ()->Add (textureWrapper);
    textureWrapper->SetImageFile(image);
    loaderContext->AddToCollection (textureWrapper->QueryObject ());

    // TODO: default texture in case of problem

    return textureWrapper;
  }

  void AssimpLoader::ImportTexture (aiTexture* texture,
				    size_t index)
  {
    // Check the type of the image
    if (texture->mHeight != 0)
    {
      // This is a raw image
      // TODO
    }

    else
    {
      // This is a compressed image

      // Create the data buffer
      csRef<iDataBuffer> buffer;
      buffer.AttachNew (new CS::DataBuffer<>
			((char*) texture->pcData,
			 texture->mWidth * sizeof (aiTexel),
			 false));

      // Load the texture
      csRef<iTextureWrapper> textureWrapper =
	LoadTexture (buffer, "<unknown>");
      if (textureWrapper)
	textures.Put (index, textureWrapper);
    }
  }

  void AssimpLoader::ImportMaterial (aiMaterial* material,
				     size_t index)
  {
    // TODO: import all other material properties

    aiString name;
    material->Get (AI_MATKEY_NAME, name);

    //csRef<iTextureWrapper> texture;
    iTextureWrapper* texture = nullptr;
    if (material->GetTextureCount (aiTextureType_DIFFUSE) > 0)
    {
      aiString path;
      material->GetTexture (aiTextureType_DIFFUSE, 0, &path);

      if ('*' == *path.data)
      {
	// This is an imported texture
	int iIndex = atoi (path.data + 1);
	if (iIndex > 0 && iIndex < (int) textures.GetSize ())
	  texture = textures[iIndex];
      }

      else
      {
	// This is an external file
	texture = FindTexture (path.data);
      }
    }

    csRef<iMaterialWrapper> materialWrapper =
      engine->CreateMaterial (name.data, texture);
    materials.Put (index, materialWrapper);
    loaderContext->AddToCollection
      (materialWrapper->QueryObject ());
  }

  void AssimpLoader::ImportGenmesh (aiNode* node)
  {
    // Create the genmesh factory
    csRef<iMeshFactoryWrapper> factoryWrapper =
      engine->CreateMeshFactory
      ("crystalspace.mesh.object.genmesh", node->mName.data);
    loaderContext->AddToCollection (factoryWrapper->QueryObject ());
    csRef<iGeneralFactoryState> gmstate =
      scfQueryInterface<iGeneralFactoryState>
      (factoryWrapper->GetMeshObjectFactory ());

    if (!firstMesh)
      firstMesh = factoryWrapper;

    // Import all submeshes
    ImportGenmeshSubMesh (gmstate, node);
  }

  void AssimpLoader::ImportGenmeshSubMesh
    (iGeneralFactoryState* gmstate, aiNode* node)
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

      // Save the current count of vertices of the genmesh
      size_t currentVertices = gmstate->GetVertexCount ();

      // Add all vertices
      aiVector3D* aiuvs = mesh->mTextureCoords[0];
      aiColor4D* aicolors = mesh->mColors[0];

      for (unsigned int i = 0; i < mesh->mNumVertices; i++)
      {
	csVector3 vertex = Assimp2CS (mesh->mVertices[i]);
	csVector2 uv = mesh->HasTextureCoords (0)
	  ? csVector2 (aiuvs[i].x, aiuvs[i].y) : csVector2 (0.0f);
	csVector3 normal = mesh->HasNormals ()
	  ? Assimp2CS (mesh->mNormals[i])
	  : csVector3 (1.0f, 0.0f, 0.0f);
	csColor4 color = mesh->HasVertexColors (0)
	  ? Assimp2CS (aicolors[i]) : csColor4 (1.0f);

	gmstate->AddVertex (vertex, uv, normal, color);
      }

      // Create a render buffer for all faces
      csRef<csRenderBuffer> buffer =
	csRenderBuffer::CreateIndexRenderBuffer
	(mesh->mNumFaces * 3, CS_BUF_STATIC,
	 CS_BUFCOMP_UNSIGNED_INT, currentVertices,
	 gmstate->GetVertexCount () - 1);

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

      // Setup the material
      csRef<iMaterialWrapper> material;
      if (mesh->mMaterialIndex < materials.GetSize ())
	   material = materials[mesh->mMaterialIndex];

      // Create the submesh
      gmstate->AddSubMesh (buffer, material, mesh->mName.data);
    }

    // Import all subnodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      aiNode*& subnode = node->mChildren[i];
      ImportGenmeshSubMesh (gmstate, subnode);
    }
  }

  void AssimpLoader::ImportAnimesh (aiNode* node)
  {
    // Create the animesh factory
    AnimeshData animeshData;
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

    // Import all submeshes
    PreProcessAnimeshSubMesh (&animeshData, node);
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

    // Invalidate the data of the factory
    animeshData.factory->Invalidate ();
  }

  void AssimpLoader::PreProcessAnimeshSubMesh
    (AnimeshData* animeshData, aiNode* node)
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
    }

    // Pre-process the child nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      aiNode*& subnode = node->mChildren[i];
      PreProcessAnimeshSubMesh (animeshData, subnode);
    }
  }

  void AssimpLoader::ImportAnimeshSubMesh (AnimeshData* animeshData,
					   aiNode* node)
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

  void AssimpLoader::ImportAnimation (aiAnimation* animation)
  {
    printf ("Animation [%s]:\n", animation->mName.data);
    for (unsigned int i = 0; i < animation->mNumChannels; i++)
      printf (" anim [%s]\n", animation->mChannels[i]->mNodeName.data);
  }

}
CS_PLUGIN_NAMESPACE_END(AssimpLoader)
