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
#ifndef __CS_ASSIMPLOADER_H__
#define __CS_ASSIMPLOADER_H__

#include "csgeom/quaternion.h"
#include "csgeom/vector3.h"
#include "cstool/vfsdirchange.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/hash.h"
#include "csutil/refarr.h"
#include "imap/loader.h"
#include "imap/reader.h"
#include "imap/modelload.h"
#include "imesh/skeleton2.h"

#include "assimp/assimp.hpp"      // C++ importer interface
#include "assimp/aiScene.h"       // Output data structure
#include "assimp/aiPostProcess.h" // Post processing flags
#include "assimp/IOStream.h"
#include "assimp/IOSystem.h"

#include "common.h"

struct iEngine;
struct iImageIO;
struct iMaterialWrapper;
struct iObjectRegistry;
struct iPluginManager;
struct iReporter;
struct iTextureWrapper;

namespace CS {
namespace Mesh {
  struct iAnimatedMeshFactory;
}
}

CS_PLUGIN_NAMESPACE_BEGIN(AssimpLoader)
{

/**
 * Open Asset Import Library loader for Crystal Space
 */

struct BoneData
{
  bool isBone;
  aiNode* node;
  CS::Animation::BoneID boneID;
  aiMatrix4x4 transform;

  BoneData ()
  : isBone (false), node (nullptr), boneID (0)
  {}
};

struct AnimeshData
{
  csRef<CS::Mesh::iAnimatedMeshFactory> factory;
  csRef<iMeshFactoryWrapper> factoryWrapper;

  csDirtyAccessArray<float> vertices;
  csDirtyAccessArray<float> texels;
  csDirtyAccessArray<float> normals;
  csDirtyAccessArray<float> tangents;
  csDirtyAccessArray<float> binormals;
  csDirtyAccessArray<float> colors;
  bool hasNormals;
  bool hasTexels;
  bool hasTangents;
  bool hasColors;

  AnimeshData ()
  : hasNormals (false), hasTexels (false),
    hasTangents (false), hasColors (false)
  {}

  aiNode* rootNode;
  csHash<BoneData, csString> boneNodes;

  struct AnimeshSubmesh
  {
    size_t vertexIndex;
  };
  csHash<AnimeshSubmesh, aiMesh*> submeshes;
};

class AssimpLoader : 
  public scfImplementation3<AssimpLoader,
			    iBinaryLoaderPlugin,
			    iModelLoader,
			    iComponent>
{
public:

  AssimpLoader (iBase*);
  virtual ~AssimpLoader ();

  //-- iComponent
  virtual bool Initialize (iObjectRegistry *object_reg);

  //-- iBinaryLoaderPlugin
  virtual bool IsThreadSafe () { return false; }
  virtual csPtr<iBase> Parse (iDataBuffer* buf, iStreamSource*,
    iLoaderContext* ldr_context, iBase* context, iStringArray*);

  //-- iModelLoader
  virtual iMeshFactoryWrapper* Load (const char* factname,
				     const char* filename);
  virtual iMeshFactoryWrapper* Load (const char* factname,
				     iDataBuffer* buffer);
  virtual bool IsRecognized (const char* filename);
  virtual bool IsRecognized (iDataBuffer* buffer);

 private:
  void ImportScene ();

  iTextureWrapper* FindTexture (const char* filename);

  void ImportTexture (aiTexture* texture, size_t index);
  void ImportMaterial (aiMaterial* material, size_t index);

  void ImportExtraRenderMesh (iMeshFactoryWrapper* factoryWrapper, aiMesh* mesh);

  void ImportGenmesh (aiNode* node);
  void ImportGenmeshSubMesh (iMeshFactoryWrapper* factoryWrapper,
			     iGeneralFactoryState* gmstate,
			     aiNode* node);

  void ImportAnimesh (aiNode* node);

  void InitBoneNode (AnimeshData* animeshData, aiNode* node);
  void PreProcessAnimesh (AnimeshData* animeshData,
			  aiNode* node);
  void ImportAnimeshSubMesh (AnimeshData* animeshData, aiNode* node);
  void ImportAnimeshSkeleton (AnimeshData* animeshData, aiNode* node,
			      CS::Animation::BoneID parent,
			      aiMatrix4x4 transform);
  void ImportBoneInfluences (AnimeshData* animeshData, aiNode* node);

  void ImportAnimation (aiAnimation* animation, size_t index);
  void ConvertAnimationFrames ();

private:
  iObjectRegistry* object_reg;

  csRef<iVFS> vfs;
  csRef<iEngine> engine;
  csRef<iGraphics3D> g3d;
  csRef<iTextureManager> textureManager;
  csRef<iLoader> loader;
  csRef<iLoaderContext> loaderContext;
  csRef<iImageIO> imageLoader;
  csRef<iShaderVarStringSet> shaderVariableNames;
  csRef<CS::Animation::iSkeletonManager> skeletonManager;

  csRefArray<iTextureWrapper> textures;
  csRefArray<iMaterialWrapper> materials;

  const aiScene* scene;
  unsigned int importFlags;

  csRef<iMeshFactoryWrapper> firstMesh;

  struct AnimeshNode
  {
    CS::Mesh::iAnimatedMeshFactory* factory;
    CS::Animation::BoneID boneID;
  };
  csHash<AnimeshNode, csString> nodeData;

};

/**
 * IO stream handling
 */

class csIOStream : public Assimp::IOStream
{
 public:
  csIOStream (iFile* file)
    : file (file) {}
  ~csIOStream () {}

  size_t Read (void* pvBuffer, size_t pSize, size_t pCount)
  { return file ? file->Read ((char*) pvBuffer, pSize * pCount) : 0; }

  size_t Write (const void* pvBuffer, size_t pSize, size_t pCount)
  { return file ? file->Write ((char*) pvBuffer, pSize * pCount) : 0; }

  aiReturn Seek (size_t pOffset, aiOrigin pOrigin)
  {
    if (!file)
      return aiReturn_FAILURE;

    switch (pOrigin)
      {
      case aiOrigin_SET:
	return file->SetPos (pOffset) ? aiReturn_SUCCESS : aiReturn_FAILURE;

      case aiOrigin_CUR:
	return file->SetPos (file->GetPos () + pOffset)
	  ? aiReturn_SUCCESS : aiReturn_FAILURE;

      case aiOrigin_END:
	return file->SetPos (file->GetSize () + pOffset)
	  ? aiReturn_SUCCESS : aiReturn_FAILURE;

      default:
	break;
      }

    return aiReturn_FAILURE;
  }

  size_t Tell () const
  { return file ? file->GetPos () : 0; }

  size_t FileSize () const
  { return file ? file->GetSize () : 0; }

  void Flush ()
  { if (file) file->Flush (); }

 private:
  csRef<iFile> file;
};

class csIOSystem : public Assimp::IOSystem
{
 public:
  csIOSystem (iVFS* vfs, const char* filename)
    : vfs (vfs), changer (vfs)
  {
    csString file = filename;
    if (filename && file.FindFirst ('/') != (size_t) -1)
      changer.ChangeTo (filename);
  }

  ~csIOSystem () {}

  bool Exists (const char *pFile) const
  {
    printf ("Exists [%s]: %s\n", pFile,
	    vfs->Exists (pFile) ? "true" : "false");
    return vfs->Exists (pFile);
  }

  char getOsSeparator () const
  { return '/'; }

  Assimp::IOStream* Open (const char *pFile, const char *pMode="rb")
  {
    printf ("Open [%s]\n", pFile); 
    csRef<iFile> file = vfs->Open (pFile,
				   *pMode == 'w' ? VFS_FILE_WRITE : VFS_FILE_READ);
    if (!file) printf ("failed!!\n");
    return new csIOStream (file);
  }

  void Close (Assimp::IOStream* pFile)
  { delete pFile; }

 private:
  csRef<iVFS> vfs;
  csVfsDirectoryChanger changer;
};

}
CS_PLUGIN_NAMESPACE_END(AssimpLoader)

#endif // __CS_ASSIMPLOADER_H__
