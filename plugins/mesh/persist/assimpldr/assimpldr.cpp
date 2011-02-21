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
#include "csutil/cscolor.h"
#include "csutil/csstring.h"
#include "csutil/databuf.h"
#include "csutil/scfstr.h"

#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "igraphic/imageio.h"
#include "imap/ldrctxt.h"
#include "imesh/genmesh.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivaria/reporter.h"
#include "ivideo/txtmgr.h"

#include "assimp/assimp.hpp"      // C++ importer interface
#include "assimp/aiScene.h"       // Output data structure
#include "assimp/aiPostProcess.h" // Post processing flags

#include "assimpldr.h"

CS_PLUGIN_NAMESPACE_BEGIN(AssimpLoader)
{

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
  static void ReportError (iObjectRegistry* objreg, const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    csReportV (objreg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.mesh.loader.factory.assimp", description, arg);
    va_end (arg);
  }

  static void ReportWarning (iObjectRegistry* objreg, const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    csReportV (objreg, CS_REPORTER_SEVERITY_WARNING, "crystalspace.mesh.loader.factory.assimp", description, arg);
    va_end (arg);
  }

  SCF_IMPLEMENT_FACTORY (AssimpLoader);

  AssimpLoader::AssimpLoader (iBase* pParent) :
    scfImplementationType (this, pParent)
  {
  }

  AssimpLoader::~AssimpLoader ()
  {
  }

  bool AssimpLoader::Initialize (iObjectRegistry* object_reg)
  {
    AssimpLoader::object_reg = object_reg;
    return true;
  }

  bool AssimpLoader::Load (iLoaderContext* loaderContext,
			   iGeneralFactoryState* gmstate,
			   uint8* buffer, size_t size)
  {
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

  csPtr<iBase> AssimpLoader::Parse (iDataBuffer* buf, iStreamSource*,
				    iLoaderContext* ldr_context, iBase* context, iStringArray*)
  {
    // TODO
    return 0;
  }

  iMeshFactoryWrapper* AssimpLoader::Load (const char* factname,
					   const char* filename, iDataBuffer* buffer)
  {
    // TODO
    return 0;
  }

  iMeshFactoryWrapper* AssimpLoader::Load (const char* factname,
					   iDataBuffer* buffer)
  {
    // TODO
    return Load (factname, 0, buffer);
  }

  iMeshFactoryWrapper* AssimpLoader::Load (const char* factname,
					   const char* filename)
  {
    textures.DeleteAll ();
    materials.DeleteAll ();

    /// Find pointers to engine data
    engine = csQueryRegistry<iEngine> (object_reg);
    if (!engine)
    {
      ReportError (object_reg, "Could not find the engine");
      return 0;
    }

    g3d = csQueryRegistry<iGraphics3D> (object_reg);
    if (!g3d)
    {
      ReportError (object_reg, "Could not find the 3D graphics");
      return 0;
    }

    textureManager = g3d->GetTextureManager();
    if (!textureManager)
    {
      ReportError (object_reg, "Could not find the texture manager");
      return 0;
    }

    imageLoader = csQueryRegistry<iImageIO> (object_reg);
    if (!imageLoader)
    {
      ReportError (object_reg, "Failed to find an image loader!");
      return 0;
    }

    loaderContext = engine->CreateLoaderContext ();

    // TODO: use ASSIMP::IOStream
    csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);
    csRef<iDataBuffer> path = vfs->GetRealPath (filename);

    // Create an instance of the Importer class
    // And have it read the given file with some postprocessing
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile (path->GetData (), 
					      aiProcess_CalcTangentSpace
					      | aiProcess_Triangulate
					      | aiProcess_ValidateDataStructure
					      | aiProcess_GenUVCoords
					      | aiProcess_FlipUVs
					      //| aiProcess_JoinIdenticalVertices
					      | aiProcess_SortByPType);
  
    // If the import failed, report it
    if (!scene)
    {
      ReportError (object_reg, "Failed to load file %s: %s!", filename, importer.GetErrorString());
      return 0;
    }

    printf ("OK!\n");
    printf ("animations: %i\n", scene->mNumAnimations);
    printf ("meshes: %i\n", scene->mNumMeshes);
    printf ("materials: %i\n", scene->mNumMaterials);
    printf ("camera: %i\n", scene->mNumCameras);
    printf ("textures: %i\n", scene->mNumTextures);
    printf ("lights: %i\n", scene->mNumLights);

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
    for (unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
      aiMesh*& mesh = scene->mMeshes[i];

      // Check the validity of the mesh
      if (!mesh->HasFaces ()
	  || !mesh->HasPositions ())
      {
	ReportWarning (object_reg, "Skipping mesh %s for lack of vertices or triangles!",
		       CS::Quote::Single (mesh->mName.data));
	continue;
      }

      // Check the type of the mesh then import it
      // TODO: animeshes, terrains
      ImportGenmesh (mesh);
    }

    return 0;
  }

  iTextureWrapper* AssimpLoader::FindTexture (const char* filename)
  {
    // TODO: CS needs a mechanism to know where to find the external files -> current VFS path in iLoaderContext?
    csString path ("/lib/face/");
    path += filename;

    // Search in the loading context
    iTextureWrapper* texture = loaderContext->FindTexture (path.GetData (), true);    
    if (texture)
      return texture;

    // Load manually the file
    csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);
    csRef<iDataBuffer> buffer = vfs->ReadFile (path);
    if (!buffer)
    {
      ReportError (object_reg, "Could not load image file %s!", CS::Quote::Single (filename));
      return 0;
    }

    return LoadTexture (buffer, filename);
  }

  iTextureWrapper* AssimpLoader::LoadTexture (iDataBuffer* buffer, const char* filename)
  {
    // Load the image data
    int format = engine->GetTextureFormat ();
    csRef<iImage> image (imageLoader->Load (buffer, format));
    if (!image)
    {
      ReportError (object_reg, "Could not load image %s. Unknown format!",
		   CS::Quote::Single (filename));
      return 0;
    }

    // Create the texture handle
    csRef<scfString> fail_reason;
    fail_reason.AttachNew (new scfString ());
    csRef<iTextureHandle> textureHandle (textureManager->RegisterTexture (image, CS_TEXTURE_2D, fail_reason));
    if (!textureHandle)
    {
      ReportError (object_reg, "crystalspace.imagetextureloader",
		   "Error creating texture", fail_reason->GetData ());
      return 0;
    }

    // Create the texture wrapper
    csRef<iTextureWrapper> textureWrapper = engine->GetTextureList ()->CreateTexture (textureHandle);
    engine->GetTextureList ()->Add (textureWrapper);
    textureWrapper->SetImageFile(image);

    return textureWrapper;
  }

  void AssimpLoader::ImportTexture (aiTexture* texture, size_t index)
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
      buffer.AttachNew (new CS::DataBuffer<> ((char*) texture->pcData, texture->mWidth * sizeof (texture->pcData), false));

      // Load the texture
      csRef<iTextureWrapper> textureWrapper = LoadTexture (buffer, "<unknown>");
      if (textureWrapper)
	textures.Put (index, textureWrapper);
    }
  }

  void AssimpLoader::ImportMaterial (aiMaterial* material, size_t index)
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

    csRef<iMaterialWrapper> csmaterial = engine->CreateMaterial (name.data, texture);
    materials.Put (index, csmaterial);
  }

  void AssimpLoader::ImportGenmesh (aiMesh* mesh)
  {
    // Create the genmesh factory
    csRef<iMeshFactoryWrapper> factoryWrapper = engine->CreateMeshFactory
      ("crystalspace.mesh.object.genmesh", mesh->mName.data);
    csRef<iGeneralFactoryState> gmstate =
      scfQueryInterface<iGeneralFactoryState> (factoryWrapper->GetMeshObjectFactory ());

    // Add all vertices
    aiVector3D* aiuvs = mesh->mTextureCoords[0];
    aiColor4D* aicolors = mesh->mColors[0];

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
      csVector3 vertex = Assimp2CS (mesh->mVertices[i]);
      csVector2 uv = mesh->HasTextureCoords (0) ? csVector2 (aiuvs[i].x, aiuvs[i].y) : csVector2 (0.0f);
      csVector3 normal = Assimp2CS (mesh->mNormals[i]);
      csColor4 color = mesh->HasVertexColors (0) ? Assimp2CS (aicolors[i]) : csColor4 (1.0f);

      gmstate->AddVertex (vertex, uv, normal, color);
    }

    // Add all faces
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
      aiFace& face = mesh->mFaces[i];
      if (face.mNumIndices == 3)
	gmstate->AddTriangle (Assimp2CS (face.mIndices));
    }    

    // Setup the material
    if (mesh->mMaterialIndex < materials.GetSize ())
    {
      csRef<iMaterialWrapper> material = materials[mesh->mMaterialIndex];
      if (material)
	factoryWrapper->GetMeshObjectFactory ()->SetMaterialWrapper (material);
    }

    // TODO: submeshes
  }

}
CS_PLUGIN_NAMESPACE_END(AssimpLoader)
