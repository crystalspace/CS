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
#ifndef __CS_ASSIMPLOADER_COMMON_H__
#define __CS_ASSIMPLOADER_COMMON_H__

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
//#include "imesh/skeleton2.h"
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

  /**
   * Type conversion
   */
  inline csVector3 Assimp2CS (aiVector3D& v)
  {
    return csVector3 (v.x, v.y, v.z);
  }

  inline csColor4 Assimp2CS (aiColor4D& c)
  {
    return csColor4 (c.r, c.g, c.b, c.a);
  }

  inline csTriangle Assimp2CS (unsigned int* t)
  {
    return csTriangle (t[0], t[1], t[2]);
  }

  inline csQuaternion Assimp2CS (aiQuaternion& q)
  {
    return csQuaternion (q.x, q.y, q.z, q.w);
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

  /**
   * Printing of scene tree
   */
  static void PrintMesh (aiMesh* mesh, const char* prefix)
  {
    printf ("%s  mesh [%s]: %i vertices %i faces %i bones %i anims %s%s%s%s\n",
	    prefix, mesh->mName.data, mesh->mNumVertices,
	    mesh->mNumFaces, mesh->mNumBones, mesh->mNumAnimMeshes,
	    mesh->mPrimitiveTypes & aiPrimitiveType_POINT ? "p" : "-",
	    mesh->mPrimitiveTypes & aiPrimitiveType_LINE ? "l" : "-",
	    mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE ? "t" : "-",
	    mesh->mPrimitiveTypes & aiPrimitiveType_POLYGON ? "p" : "-");
  }

  static void PrintNode (const aiScene* scene, aiNode* node,
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

  /**
   * Render buffer creation
   */
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
 
}
CS_PLUGIN_NAMESPACE_END(AssimpLoader)

#endif // __CS_ASSIMPLOADER_COMMON_H__
