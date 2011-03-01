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

  SCF_IMPLEMENT_FACTORY (AssimpLoader);

  AssimpLoader::AssimpLoader (iBase* pParent) :
    scfImplementationType (this, pParent)
  {
    importFlags = aiProcess_CalcTangentSpace
      | aiProcess_Triangulate
      | aiProcess_ValidateDataStructure
      | aiProcess_GenUVCoords
      | aiProcess_SortByPType
      | aiProcess_LimitBoneWeights
      | aiProcess_ConvertToLeftHanded
      //| aiProcess_GenNormals 
      | aiProcess_GenSmoothNormals 
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
    printf ("lights: %i\n", scene->mNumLights);

    printf ("\nScene tree:\n");
    PrintNode (scene, scene->mRootNode, "");
    printf ("\n");

    // Clear previous import data
    textures.DeleteAll ();
    materials.DeleteAll ();
    firstMesh = nullptr;
    nodeData.DeleteAll ();

    // Find pointers to engine data
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

    // Find the skeleton manager
    skeletonManager = csQueryRegistryOrLoad<CS::Animation::iSkeletonManager>
      (object_reg, "crystalspace.skeletalanimation");
    if (!skeletonManager)
      ReportWarning (object_reg,
		     "Could not find the skeleton manager. Importing animesh skeletons and animations won't be possible");

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
      // TODO: Check the type of the mesh then import it
      // TODO: terrains, other primitives than triangles, whole scene (lights, cameras)
      //ImportGenmesh (scene->mRootNode);
      ImportAnimesh (scene->mRootNode);

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

}
CS_PLUGIN_NAMESPACE_END(AssimpLoader)
