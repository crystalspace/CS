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
#include "iutil/comp.h"

#include "assimp/assimp.hpp"      // C++ importer interface
#include "assimp/aiScene.h"       // Output data structure
#include "assimp/aiPostProcess.h" // Post processing flags
#include "assimp/IOStream.h"
#include "assimp/IOSystem.h"
#include "assimp/ProgressHandler.h"

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

enum ImportType
{
  IMPORT_UNDEFINED = 0,
  IMPORT_SCENE,
  IMPORT_MODEL
};

enum NodeType
{
  NODE_UNDEFINED = 0,
  NODE_GENMESH,
  NODE_ANIMESH
};

struct NodeData
{
  aiNode* node;
  NodeType type;

  NodeData (aiNode* node)
  : node (node), type (NODE_UNDEFINED) {}
};

struct ImportedMesh
{
  iMeshFactoryWrapper* factoryWrapper;
  csArray<aiMesh*> meshes;
};

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
  ImportedMesh* importedMesh;

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

struct AssimpProgressHandler : public Assimp::ProgressHandler
{
  //-- Assimp::ProgressHandler
  bool Update (float percentage);  
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

  bool HasBone (aiNode* node);
  void InitSceneNode (aiNode* node);
  void AnalyzeSceneNode (aiNode* node, aiNode* animeshNode);

  void ImportSceneNode (aiNode* node);
  bool IsInstancedMesh (aiNode* node);
  bool ContainsAllMeshes (csArray<aiMesh*>& meshes, aiNode* node);

  void ImportExtraRenderMesh (ImportedMesh* importedMesh, aiMesh* mesh);

  void ImportGenmesh (aiNode* node);
  void ImportGenmeshSubMesh (ImportedMesh* importedMesh,
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

  void ReportError (const char* description, ...);
  void ReportWarning (const char* description, ...);

  void PrintMesh (aiMesh* mesh, const char* prefix);
  void PrintNode (const aiScene* scene, aiNode* node,
		  const char* prefix);
  void PrintImportNode (aiNode* node, const char* prefix);

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

  ImportType importType;

  // The first mesh factory encountered is used as the return value when needed
  csRef<iMeshFactoryWrapper> firstMesh;

  // Assimp specific objects
  const aiScene* scene;
  unsigned int importFlags;

  // These are used to analyze the scene tree
  csHash<NodeData, aiNode*> sceneNodes;
  csHash<NodeData*, csString> sceneNodesByName;

  // This is used to find whether some meshes are instanciated
  csArray<ImportedMesh> importedMeshes;

  // This is used to find the corresponding bone data when importing animations
  struct AnimeshNode
  {
    CS::Mesh::iAnimatedMeshFactory* factory;
    CS::Animation::BoneID boneID;
  };
  csHash<AnimeshNode, csString> animeshNodes;
};

}
CS_PLUGIN_NAMESPACE_END(AssimpLoader)

#endif // __CS_ASSIMPLOADER_H__
