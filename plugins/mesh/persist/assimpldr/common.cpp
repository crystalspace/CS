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

#include "iengine/mesh.h"
#include "imesh/animesh.h"
#include "imesh/genmesh.h"

#include "common.h"

CS_PLUGIN_NAMESPACE_BEGIN(AssimpLoader)
{

  /**
   * Progress reporting
   */
  bool AssimpProgressHandler::Update (float percentage)
  {
    printf (".");
    // TODO: let the user abort the loading
    return true;
  }

  /**
   * Error reporting
   */
  void AssimpLoader::ReportError (const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    csReportV (object_reg, CS_REPORTER_SEVERITY_ERROR,
	       "crystalspace.mesh.loader.factory.assimp",
	       description, arg);
    va_end (arg);
  }

  void AssimpLoader::ReportWarning (const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    csReportV (object_reg, CS_REPORTER_SEVERITY_WARNING,
	       "crystalspace.mesh.loader.factory.assimp",
	       description, arg);
    va_end (arg);
  }

  /**
   * Printing of scene tree
   */
  void AssimpLoader::PrintMesh (aiMesh* mesh, const char* prefix)
  {
    printf ("%s  mesh [%s]: %i vertices %i faces %i bones %i anims %s%s%s%s\n",
	    prefix, mesh->mName.data, mesh->mNumVertices,
	    mesh->mNumFaces, mesh->mNumBones, mesh->mNumAnimMeshes,
	    mesh->mPrimitiveTypes & aiPrimitiveType_POINT ? "p" : "-",
	    mesh->mPrimitiveTypes & aiPrimitiveType_LINE ? "l" : "-",
	    mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE ? "t" : "-",
	    mesh->mPrimitiveTypes & aiPrimitiveType_POLYGON ? "p" : "-");
  }

  void AssimpLoader::PrintNode (const aiScene* scene, aiNode* node,
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

  void AssimpLoader::PrintImportNode (aiNode* node, const char* prefix)
  {
    NodeData* nodeData = sceneNodes[node];
    printf ("%s+ node [%s] ", prefix, node->mName.data);

    if (nodeData->type == NODE_GENMESH)
      printf ("GENMESH!\n");

    else if (nodeData->type == NODE_ANIMESH)
      printf ("ANIMESH!\n");

    else
    {
      printf ("\n");

      csString pref = prefix;
      pref += " ";

      for (unsigned int i = 0; i < node->mNumChildren; i++)
	PrintImportNode (node->mChildren[i], pref.GetData ());
    }
  }

}
CS_PLUGIN_NAMESPACE_END(AssimpLoader)
