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
    importFlags =
      aiProcess_CalcTangentSpace // needed only for advanced shaders
      //| aiProcess_ConvertToLeftHanded // seems deprecated
      //| aiProcess_FindDegenerates // optimization
      //| aiProcess_FindInvalidData // in all cases? Not currently since it invalidates the animation data...
      | aiProcess_FlipUVs // always needed for CS
      | aiProcess_FlipWindingOrder // always needed for CS
      //| aiProcess_GenNormals // for fast
      | aiProcess_GenSmoothNormals // for optimized
      | aiProcess_GenUVCoords // needed only for texture based materials
      //| aiProcess_ImproveCacheLocality // optimization
      //| aiProcess_JoinIdenticalVertices // almost always needed. Causes problems when wanting to merge morph targets
      | aiProcess_LimitBoneWeights // Needed due to limitation of animeshes
      | aiProcess_MakeLeftHanded // always needed for CS
      | aiProcess_OptimizeGraph // only for models, not for scenes
      //| aiProcess_OptimizeMeshes // in all cases? Causes problems when wanting to merge morph targets
      //| aiProcess_PreTransformVertices // can be useful for genmeshes
      //| aiProcess_RemoveRedundantMaterials // in all cases?
      | aiProcess_SortByPType // always needed
      | aiProcess_SplitLargeMeshes // optimization
      | aiProcess_Triangulate // always needed unless other primitives available in CS
      | aiProcess_ValidateDataStructure; // always needed
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
    csString format = filename;
    format = format.Slice (format.FindLast ('.'));
    Assimp::Importer importer;
    return importer.IsExtensionSupported (format.GetData ());
  }

  bool AssimpLoader::IsRecognized (iDataBuffer* buffer)
  {
    // TODO: this is dangerous...
    return true;
  }

  csPtr<iBase> AssimpLoader::Parse
    (iDataBuffer* buffer, iStreamSource*,
     iLoaderContext* ldr_context, iBase* context,
     iStringArray* failed)
  {
    loaderContext = ldr_context;

    // Find the VFS
    vfs = csQueryRegistry<iVFS> (object_reg);
    if (!vfs)
    {
      ReportError ("Could not load VFS system!");
      return (iBase*) nullptr;
    }

    // Create an Assimp importer and parse the file
    Assimp::Importer importer;
    importer.SetIOHandler (new csIOSystem (vfs, nullptr));
    importer.SetProgressHandler (new AssimpProgressHandler ());
    printf ("Loading...");
    scene = importer.ReadFileFromMemory
      (**buffer, buffer->GetSize (), importFlags, "");

    // If the import failed, report it
    if (!scene)
    {
      printf ("FAILED!\n");
      ReportError ("Failed to load binary file: %s",
		   importer.GetErrorString());
      return (iBase*) nullptr;
    }

    // Import the scene into CS
    importType = IMPORT_SCENE;
    ImportScene ();

    // TODO: list of failed factories

    firstMesh->IncRef ();
    return csPtr<iBase> (firstMesh);
  }

  iMeshFactoryWrapper* AssimpLoader::Load (const char* factname,
					   iDataBuffer* buffer)
  {
    // Find the VFS
    vfs = csQueryRegistry<iVFS> (object_reg);
    if (!vfs)
    {
      ReportError ("Could not load VFS system!");
      return nullptr;
    }

    // Create an Assimp importer and parse the file
    Assimp::Importer importer;
    importer.SetIOHandler (new csIOSystem (vfs, nullptr));
    importer.SetProgressHandler (new AssimpProgressHandler ());
    printf ("Loading...");
    scene = importer.ReadFileFromMemory
      (**buffer, buffer->GetSize (), importFlags, "");

    // If the import failed, report it
    if (!scene)
    {
      printf ("FAILED!\n");
      ReportError ("Failed to load factory %s: %s",
		   CS::Quote::Single (factname),
		   importer.GetErrorString());
      return nullptr;
    }

    // Import the model into CS
    importType = IMPORT_MODEL;
    ImportScene ();

    return firstMesh;
  }

  iMeshFactoryWrapper* AssimpLoader::Load (const char* factname,
					   const char* filename)
  {
    // TODO: implement iPluginConfig, use iVerbosityManager, iProgressMeter
    // TODO: custom options: scale, genmesh/animesh/scene/factories,
    //   find duplicates/optimize, save default animesh pose, fast/optimized
    // TODO: if forced to be a genmesh then don't read animations,
    //   weights, etc
    // TODO: if forced to be a mesh then load only the textures/materials needed for it

    // Find the VFS
    vfs = csQueryRegistry<iVFS> (object_reg);
    if (!vfs)
    {
      ReportError ("Could not load VFS system!");
      return nullptr;
    }

    // Create an Assimp importer and parse the file
    Assimp::Importer importer;
    importer.SetIOHandler (new csIOSystem (vfs, filename));
    importer.SetProgressHandler (new AssimpProgressHandler ());
    printf ("Loading...");
    scene = importer.ReadFile (filename, importFlags);

    // If the import failed, report it
    if (!scene)
    {
      printf ("FAILED!\n");
      ReportError ("Failed to load factory %s from file %s: %s",
		   CS::Quote::Single (factname),
		   CS::Quote::Single (filename),
		   importer.GetErrorString());
      return nullptr;
    }

    // Import the model into CS
    importType = IMPORT_MODEL;
    ImportScene ();

    return firstMesh;
  }

  void AssimpLoader::ImportScene ()
  {
    printf ("SUCCESS!\n");
    printf ("animations: %i\n", scene->mNumAnimations);
    printf ("meshes: %i\n", scene->mNumMeshes);
    printf ("materials: %i\n", scene->mNumMaterials);
    printf ("camera: %i\n", scene->mNumCameras);
    printf ("textures: %i\n", scene->mNumTextures);
    printf ("lights: %i\n", scene->mNumLights);

    printf ("\nScene tree:\n");
    PrintNode (scene, scene->mRootNode, "");
    printf ("\n");

    // Clear any previous imported first mesh
    firstMesh = nullptr;

    // Find pointers to engine data
    engine = csQueryRegistry<iEngine> (object_reg);
    if (!engine)
    {
      ReportError ("Could not find the engine");
      return;
    }

    g3d = csQueryRegistry<iGraphics3D> (object_reg);
    if (!g3d)
    {
      ReportError ("Could not find the 3D graphics");
      return;
    }

    textureManager = g3d->GetTextureManager();
    if (!textureManager)
    {
      ReportError ("Could not find the texture manager");
      return;
    }

    loader = csQueryRegistry<iLoader> (object_reg);
    if (!loader)
    {
      ReportError ("Could not find the main loader!");
      return;
    }

    imageLoader = csQueryRegistry<iImageIO> (object_reg);
    if (!imageLoader)
    {
      ReportError ("Failed to find an image loader!");
      return;
    }

    shaderVariableNames = csQueryRegistryTagInterface<iShaderVarStringSet>
      (object_reg, "crystalspace.shader.variablenameset");
    if (!shaderVariableNames)
      ReportWarning ("Could not find the shader variable set. Import of materials will be limited");

    // Find the skeleton manager
    skeletonManager = csQueryRegistryOrLoad<CS::Animation::iSkeletonManager>
      (object_reg, "crystalspace.skeletalanimation");
    if (!skeletonManager)
      ReportWarning ("Could not find the skeleton manager. Importing animesh skeletons and animations won't be possible");

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

    if (!scene->mRootNode)
      return;

    // Import either the model or the whole scene
    if (importType == IMPORT_MODEL)
    {
      // If there is any bone then this an animesh
      if (HasBone (scene->mRootNode))
	ImportAnimesh (scene->mRootNode);

      // Otherwise this is a genmesh
      else ImportGenmesh (scene->mRootNode);
    }

    else
    {
      // Analyze the scene tree
      InitSceneNode (scene->mRootNode);
      AnalyzeSceneNode (scene->mRootNode, nullptr);

      printf ("\nImported scene tree:\n");
      PrintImportNode (scene->mRootNode, "");
      printf ("\n");

      // TODO: terrains, lights, cameras, null meshes with extra render meshes
      ImportSceneNode (scene->mRootNode);
    }

    // Import all animations
    for (unsigned int i = 0; i < scene->mNumAnimations; i++)
    {
      aiAnimation*& animation = scene->mAnimations[i];
      ImportAnimation (animation, i);
    }

    // Convert the animations in bind space
    ConvertAnimationFrames ();

    // Clear the internal import data
    textures.DeleteAll ();
    materials.DeleteAll ();
    sceneNodes.DeleteAll ();
    sceneNodesByName.DeleteAll ();
    importedMeshes.DeleteAll ();
    animeshNodes.DeleteAll ();
  }

  iTextureWrapper* AssimpLoader::FindTexture (const char* filename)
  {
    // Check if it is a texture which has been imported from Assimp
    if ('*' == *filename)
    {
      int iIndex = atoi (filename + 1);
      if (iIndex > 0 && iIndex < (int) textures.GetSize ())
	return textures[iIndex];
    }

    // This is an external file, check at first in the loading context if it was
    // already loaded
    csRef<iTextureWrapper> texture =
      loaderContext->FindTexture (filename, true);    
    if (texture)
      return texture;

    // Reformat the file name
    csString file = filename;
    if (file.StartsWith ("//"))
      file = file.Slice (2);
    file.FindReplace ("\\", "/");

    // Load manually the file
    csRef<iTextureHandle> textureHandle =
      loader->LoadTexture (file.GetData (), CS_TEXTURE_2D);
    texture = engine->GetTextureList ()->CreateTexture (textureHandle);
    engine->GetTextureList ()->Add (texture);
    loaderContext->AddToCollection (texture->QueryObject ());

    return texture;
  }

  void AssimpLoader::ImportTexture (aiTexture* texture,
				    size_t index)
  {
    csRef<iDataBuffer> buffer;

    // Check the type of the image
    if (texture->mHeight != 0)
    {
      // This is a raw image
      // TODO: this is still untested
      buffer.AttachNew
	(new CS::DataBuffer<>
	 ((char*) texture->pcData,
	  texture->mWidth * texture->mHeight * sizeof (aiTexel),
	  false));
    }

    else
    {
      // This is a compressed image
      buffer.AttachNew (new CS::DataBuffer<>
			((char*) texture->pcData,
			 texture->mWidth * sizeof (aiTexel),
			 false));
    }

    // Load the texture
    // TODO: need to specify the context to the loader
    csRef<iTextureWrapper> textureWrapper =
      loader->LoadTexture ("<unknown>", buffer, CS_TEXTURE_2D);
    if (textureWrapper)
      textures.Put (index, textureWrapper);
  }

  void AssimpLoader::ImportMaterial (aiMaterial* material,
				     size_t index)
  {
    // Find the name of the material
    aiString name;
    material->Get (AI_MATKEY_NAME, name);

    // Find the base texture
    iTextureWrapper* texture = nullptr;
    aiString path;
    if (material->GetTextureCount (aiTextureType_DIFFUSE) > 0)
    {
      material->GetTexture (aiTextureType_DIFFUSE, 0, &path);
      texture = FindTexture (path.data);
    }

    // Create the material
    csRef<iMaterialWrapper> materialWrapper = engine->CreateMaterial (name.data, texture);
    materials.Put (index, materialWrapper);
    loaderContext->AddToCollection (materialWrapper->QueryObject ());

    if (!shaderVariableNames)
      return;

    // Check for a specular texture
    csRefArray<csShaderVariable> shaderVariables;
    if (material->GetTextureCount (aiTextureType_SPECULAR) > 0)
    {
      material->GetTexture (aiTextureType_SPECULAR, 0, &path);
      texture = FindTexture (path.data);
      if (texture)
      {
	csRef<csShaderVariable> variable;
	variable.AttachNew (new csShaderVariable);
	variable->SetName (shaderVariableNames->Request ("tex specular"));
	variable->SetValue (texture);
	shaderVariables.Push (variable);
      }
    }

    // Check for a normals texture
    if (material->GetTextureCount (aiTextureType_NORMALS) > 0)
    {
      material->GetTexture (aiTextureType_NORMALS, 0, &path);
      texture = FindTexture (path.data);
      if (texture)
      {
	csRef<csShaderVariable> variable;
	variable.AttachNew (new csShaderVariable);
	variable->SetName (shaderVariableNames->Request ("tex normal"));
	variable->SetValue (texture);
	shaderVariables.Push (variable);
      }
    }

    // Check for an ambient texture
    if (material->GetTextureCount (aiTextureType_AMBIENT) > 0)
    {
      material->GetTexture (aiTextureType_AMBIENT, 0, &path);
      texture = FindTexture (path.data);
      if (texture)
      {
	csRef<csShaderVariable> variable;
	variable.AttachNew (new csShaderVariable);
	variable->SetName (shaderVariableNames->Request ("tex dynamic ambient"));
	variable->SetValue (texture);
	shaderVariables.Push (variable);
      }
    }

    for (size_t i = 0; i < shaderVariables.GetSize (); i++)
      materialWrapper->GetMaterial ()->AddVariable (shaderVariables[i]);

    // TODO: import all other material properties
  }

  bool AssimpLoader::HasBone (aiNode* node)
  {
    // Check if any mesh of this node has any bone
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
      aiMesh*& mesh = scene->mMeshes[node->mMeshes[i]];

      // Check the validity of the mesh
      if (!mesh->HasFaces ()
	  || !mesh->HasPositions ())
	continue;

      // Check if there are any bones
      if (mesh->mNumBones > 0)
	return true;
    }

    // Check the child nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      aiNode*& subnode = node->mChildren[i];
      if (HasBone (subnode))
	return true;
    }

    return false;
  }

  void AssimpLoader::InitSceneNode (aiNode* node)
  {
    // Add an entry in the registered scene nodes
    NodeData nodeData (node);
    if (node->mNumMeshes > 0)
      nodeData.type = NODE_GENMESH;

    NodeData& nodeRef = sceneNodes.PutUnique (node, nodeData);
    if (strlen (node->mName.data))
      sceneNodesByName.PutUnique (node->mName.data, &nodeRef);

    // Init the child nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      aiNode*& subnode = node->mChildren[i];
      InitSceneNode (subnode);
    }
  }

  void AssimpLoader::AnalyzeSceneNode (aiNode* node, aiNode* animeshNode)
  {
    // Check for the data of all meshes
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
      aiMesh*& mesh = scene->mMeshes[node->mMeshes[i]];

      // Check the validity of the mesh
      if (!mesh->HasFaces ()
	  || !mesh->HasPositions ())
	continue;

      // Check if we found an animesh
      if (!animeshNode && mesh->mNumBones > 0)
      {
	// Find the data of this node and mark it as an animesh
	NodeData* nodeData = sceneNodes[node];
	nodeData->type = NODE_ANIMESH;
	animeshNode = node;
      }

      // Check for the bones of this submesh
      for (unsigned int i = 0; i < mesh->mNumBones; i++)
      {
	aiBone*& bone = mesh->mBones[i];

	// Find the node corresponding to this bone and mark it as a bone
	NodeData* nodeData = *sceneNodesByName[bone->mName.data];

	// Mark the parents of this node as bones
	aiNode* nodeIterator = nodeData->node->mParent;
	while (nodeIterator)
	{
	  // Find the node corresponding to this bone and mark it as a bone
	  NodeData* nodeData = sceneNodes[nodeIterator];
	  if (!nodeData)
	    break;

	  // Check for the root of the tree reached
	  if (!nodeIterator->mParent)
	  {
	    // We found a new animesh node
	    nodeData->type = NODE_ANIMESH;
	    animeshNode = nodeIterator;
	    break;
	  }

	  // Check for the parent reached
	  if (nodeIterator->mParent == animeshNode)
	    break;

	  // Check for the grand parent reached
	  if (animeshNode && nodeIterator->mParent == animeshNode->mParent)
	  {
	    NodeData* nodeData = sceneNodes[animeshNode->mParent];
	    if (!nodeData)
	      break;

	    // We found a new animesh node
	    nodeData->type = NODE_ANIMESH;
	    animeshNode = animeshNode->mParent;
	    break;
	  }

	  nodeIterator = nodeIterator->mParent;
	}
      }
    }

    // Analyze the child nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      aiNode*& subnode = node->mChildren[i];
      AnalyzeSceneNode (subnode, animeshNode);
    }
  }

  void AssimpLoader::ImportSceneNode (aiNode* node)
  {
    // Check if we found a genmesh or an animesh
    NodeData* nodeData = sceneNodes[node];
    if (nodeData->type == NODE_GENMESH)
    {
      if (!IsInstancedMesh (node))
	ImportGenmesh (node);
    }

    else if (nodeData->type == NODE_ANIMESH)
      ImportAnimesh (node);

    // Otherwise import the children nodes
    else for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      aiNode*& subnode = node->mChildren[i];
      ImportSceneNode (subnode);
    }
  }

  bool AssimpLoader::IsInstancedMesh (aiNode* node)
  {
    // Check in the list of imported meshes if this same list of Assimp meshes has already
    // been used
    for (size_t i = 0; i < importedMeshes.GetSize (); i++)
      if (ContainsAllMeshes (importedMeshes[i].meshes, node))
	return true;

    return false;
  }

  bool AssimpLoader::ContainsAllMeshes (csArray<aiMesh*>& meshes, aiNode* node)
  {
    // Check if all the meshes of this node are in the mesh list
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
      aiMesh*& mesh = scene->mMeshes[node->mMeshes[i]];
      //if (!meshes.Contains (mesh))
      //return false;
      bool found = false;
      for (size_t i = 0; i < meshes.GetSize (); i++)
	if (meshes[i] == mesh)
	{
	  found = true;
	  break;
	}

      if (!found) return false;
    }

    // Check all subnodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      aiNode*& subnode = node->mChildren[i];
      if (!ContainsAllMeshes (meshes, subnode))
	return false;
    }

    return true;
  }

  void AssimpLoader::ImportExtraRenderMesh (ImportedMesh* importedMesh, aiMesh* mesh)
  {
    bool isPoint = mesh->mPrimitiveTypes & aiPrimitiveType_POINT;
    // TODO: is_mirror

    // Create the extra render mesh and the render buffer holder
    CS::Graphics::RenderMesh* renderMesh = new CS::Graphics::RenderMesh ();
    renderMesh->buffers.AttachNew (new csRenderBufferHolder);
    renderMesh->meshtype = isPoint ? CS_MESHTYPE_POINTS : CS_MESHTYPE_LINES;
    renderMesh->indexstart = 0;
    renderMesh->indexend = mesh->mNumFaces;
    renderMesh->bbox.StartBoundingBox ();

    // Setup the vertex buffer
    csDirtyAccessArray<float> vertices (mesh->mNumVertices * 3);
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
      vertices.Push (mesh->mVertices[i][0]);
      vertices.Push (mesh->mVertices[i][1]);
      vertices.Push (mesh->mVertices[i][2]);
      renderMesh->bbox.AddBoundingVertex (Assimp2CS (mesh->mVertices[i]));
    }

    csRef<iRenderBuffer> buffer;
    buffer = FillBuffer<float> (vertices, CS_BUFCOMP_FLOAT, 3, false);
    renderMesh->buffers->SetRenderBuffer (CS_BUFFER_POSITION, buffer);

    // Setup the index buffer
    // TODO: no need of an index buffer for points...
    csDirtyAccessArray<uint> indices (mesh->mNumFaces * isPoint ? 1 : 1);
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
      aiFace& face = mesh->mFaces[i];

      indices.Push (face.mIndices[0]);
      if (!isPoint)
	indices.Push (face.mIndices[1]);
    }

    buffer = FillBuffer<uint> (indices, CS_BUFCOMP_UNSIGNED_INT,
			       isPoint ? 1 : 1, true);
    renderMesh->buffers->SetRenderBuffer (CS_BUFFER_INDEX, buffer);

    // Setup the color buffer
    if (mesh->HasVertexColors (0))
    {
      csDirtyAccessArray<float> colors (mesh->mNumVertices * 4);
      aiColor4D* aicolors = mesh->mColors[0];
      for (unsigned int i = 0; i < mesh->mNumVertices; i++)
      {
	colors.Push (aicolors[i].r);
	colors.Push (aicolors[i].g);
	colors.Push (aicolors[i].b);
	colors.Push (aicolors[i].a);
      }

      buffer = FillBuffer<float> (colors, CS_BUFCOMP_FLOAT, 4, false);
      renderMesh->buffers->SetRenderBuffer (CS_BUFFER_COLOR, buffer);
    }

    // Setup the material
    if (mesh->mMaterialIndex < materials.GetSize ())
      renderMesh->material = materials[mesh->mMaterialIndex];

    importedMesh->factoryWrapper->AddExtraRenderMesh (renderMesh);
    importedMesh->meshes.Push (mesh);
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

    // Create an entry in the list of imported meshes
    ImportedMesh importedMesh;
    importedMesh.factoryWrapper = factoryWrapper;
    importedMeshes.Push (importedMesh);

    // Check if there is not yet any 'first mesh' defined
    if (!firstMesh)
      firstMesh = factoryWrapper;

    // Import all submeshes
    ImportGenmeshSubMesh (&importedMeshes[importedMeshes.GetSize () - 1], gmstate, node);
  }

  void AssimpLoader::ImportGenmeshSubMesh
    (ImportedMesh* importedMesh, iGeneralFactoryState* gmstate, aiNode* node)
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
	  ImportExtraRenderMesh (importedMesh, mesh);

	else ReportWarning ("Skipping mesh %s for lack of points, lines or triangles!",
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
	(mesh->mNumFaces * 3, CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT,
	 currentVertices, gmstate->GetVertexCount () - 1);

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
      importedMesh->meshes.Push (mesh);
    }

    // Import all subnodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      aiNode*& subnode = node->mChildren[i];
      ImportGenmeshSubMesh (importedMesh, gmstate, subnode);
    }
  }

}
CS_PLUGIN_NAMESPACE_END(AssimpLoader)
