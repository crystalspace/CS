/*
    Copyright (C) 2006 by Jorrit Tyberghein

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"

#include "csgeom/math3d.h"
#include "csgeom/matrix3.h"
#include "csgeom/quaternion.h"
#include "csgeom/transfrm.h"
#include "csgeom/tri.h"
#include "csgfx/renderbuffer.h"
#include "csutil/csendian.h"
#include "csutil/csstring.h"
#include "csutil/scanstr.h"
#include "csutil/sysfunc.h"
#include "csutil/memfile.h"
#include "csutil/cscolor.h"
#include "cstool/rbuflock.h"

#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "imap/ldrctxt.h"
#include "imesh/object.h"
#include "imesh/genmesh.h"
#include "iutil/comp.h"
#include "iutil/document.h"
#include "iutil/eventh.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"

#include "gmesh3ds.h"
#include <lib3ds/camera.h>
#include <lib3ds/file.h>
#include <lib3ds/io.h>
#include <lib3ds/light.h>
#include <lib3ds/material.h>
#include <lib3ds/matrix.h>
#include <lib3ds/mesh.h>
#include <lib3ds/node.h>
#include <lib3ds/vector.h>




CS_PLUGIN_NAMESPACE_BEGIN(Genmesh3DS)
{

/**
 * Reports errors
 */
static void ReportError (iObjectRegistry* objreg, const char* id,
	const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  csReportV (objreg, CS_REPORTER_SEVERITY_ERROR, id, description, arg);
  va_end (arg);
}

SCF_IMPLEMENT_FACTORY (csGenmesh3DSFactoryLoader)

csGenmesh3DSFactoryLoader::csGenmesh3DSFactoryLoader (iBase* pParent) :
  scfImplementationType (this, pParent)
{
}

csGenmesh3DSFactoryLoader::~csGenmesh3DSFactoryLoader ()
{
}

bool csGenmesh3DSFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csGenmesh3DSFactoryLoader::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);
  return true;
}

bool csGenmesh3DSFactoryLoader::LoadMeshObjectData (
	iLoaderContext* ldr_context,
	iGeneralFactoryState* gmstate, Lib3dsMesh *p3dsMesh,
	Lib3dsMaterial* pCurMaterial)
{
  int i;

  // Get the number of vertices in the current mesh.
  int numVertices = p3dsMesh->points;
  int numTexels = p3dsMesh->texels;
  printf ("numVertices=%d numTexels=%d\n", numVertices, numTexels); fflush (stdout);

  // Current last vertex in the factory.
  int curLastVt = gmstate->GetVertexCount ();

  // Load up the vertices
  Lib3dsPoint* curPoint = p3dsMesh->pointL;
  Lib3dsTexel* curTexel = p3dsMesh->texelL;

  csColor4 black (0, 0, 0);
  csVector3 normal (0, 0, 0);
  for (i = 0 ; i < numVertices ; i++)
  {
    csVector2 uv;
    if (i < numTexels)
    {
      uv.Set ((*curTexel)[0], 1.0f - (*curTexel)[1]);
      curTexel++;
    }
    else uv.Set (0, 0);
    csVector3 v (curPoint->pos[0], curPoint->pos[1], curPoint->pos[2]);
    gmstate->AddVertex (v, uv, normal, black);
    curPoint++;
  }

  /*** Load up the triangles ***/

  // Get the trianle count and go to the first triangle
  int numTriangles = p3dsMesh->faces;
  Lib3dsFace* curFace = p3dsMesh->faceL;

  for (i = 0 ; i < numTriangles ; i++)
  {
    csTriangle tri (curLastVt + curFace->points[0],
    		    curLastVt + curFace->points[1],
		    curLastVt + curFace->points[2]);
    size_t tri_idx = gmstate->GetTriangleCount ();
    gmstate->AddTriangle (tri);
    iMaterialWrapper* mat = ldr_context->FindMaterial (curFace->material);
    if (!mat)
    {
      ReportError (object_reg,
		"crystalspace.genmesh3dsfactoryloader.load",
		"Can't find material %s!", CS::Quote::Single (curFace->material));
      return false;
    }
    size_t j;
    bool found = false;
    for (j = 0 ; j < materials_and_tris.GetSize () ; j++)
    {
      csMatAndTris& mt = materials_and_tris[j];
      if (mt.material == mat)
      {
        found = true;
        mt.tris.Push ((uint)tri_idx);
        break;
      }
    }
    if (!found)
    {
      size_t ii = materials_and_tris.Push (csMatAndTris ());
      materials_and_tris[ii].material = mat;
      materials_and_tris[ii].tris.Push ((uint)tri_idx);
    }
    curFace++;
  }

  return true;
}

bool csGenmesh3DSFactoryLoader::Load (iLoaderContext* ldr_context,
	iGeneralFactoryState* gmstate,
	uint8* buffer, size_t size)
{
  Lib3dsFile *p3dsFile;

  /***  send the buffer in to be translated  ***/
  p3dsFile = LoadFileData (buffer, size);
  if (!p3dsFile)
  {
    ReportError (object_reg,
		"crystalspace.genmesh3dsfactoryloader.load",
		"Error reading the 3DS file!");
    return false;
  }

  /***  go through all of the mesh objects and convert them  ***/
  Lib3dsMesh *pCurMesh;

  // RDS NOTE: add support for frames

  // set the current mesh to the first in the file
  pCurMesh = p3dsFile->meshes;

  // As long as we have a valid mesh...
  while (pCurMesh)
  {
    // Now process that mesh
    if (!LoadMeshObjectData (ldr_context, gmstate, pCurMesh, p3dsFile->materials))
    {
      ReportError (object_reg,
		"crystalspace.genmesh3dsfactoryloader.load",
		"Error parsing the 3DS file!");
      return false;
    }
    pCurMesh = pCurMesh->next;
  }

  lib3ds_file_free (p3dsFile);

  return true;
}

bool csGenmesh3DSFactoryLoader::Test3DS (uint8* buffer, size_t size)
{
  Lib3dsFile *p3dsFile = LoadFileData (buffer, size);
  if (!p3dsFile) return false;
  lib3ds_file_free (p3dsFile);
  return true;
}

bool csGenmesh3DSFactoryLoader::IsRecognized (const char* filename)
{
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);
  csRef<iDataBuffer> dbuf = vfs->ReadFile (filename);
  if (!dbuf) return false;
  return IsRecognized (dbuf);
}

bool csGenmesh3DSFactoryLoader::IsRecognized (iDataBuffer* buffer)
{
  return Test3DS (buffer->GetUint8 (), buffer->GetSize ());
}

// these are wrappers for csDataStream to interface with Lib3dsIO
static Lib3dsBool DataErrorFunc (void *)
{
  // does nothing for now
  return LIB3DS_FALSE;
}


static long DataSeekFunc (void *self, long offset, Lib3dsIoSeek origin)
{
  iFile *pData = (iFile*)self;

  size_t newOffs = offset;
  switch (origin)
  {
    case LIB3DS_SEEK_SET:
      break;
    case LIB3DS_SEEK_CUR:
      newOffs += pData->GetPos();
      break;
    case LIB3DS_SEEK_END:
      newOffs = pData->GetSize();
      break;
    default:
      return 1;
  }
  pData->SetPos (newOffs);
  return 0;
}


static long DataTellFunc (void *self)
{
  iFile* pData = (iFile*)self;
  return (long)pData->GetPos();
}


#ifdef LIB3DS_GENERIC_DATA_IO_CALLBACKS
static size_t DataReadFunc (void *self, void *buffer, size_t size)
#else
static int DataReadFunc (void *self, Lib3dsByte *buffer, int size)
#endif
{
  iFile* pData = (iFile*)self;
  return (int)pData->Read ((char*)buffer, size );
}


#ifdef LIB3DS_GENERIC_DATA_IO_CALLBACKS
static size_t DataWriteFunc (void* /*self*/, const void* /*buffer*/, 
 	size_t /*size*/)
#else
static int DataWriteFunc (void* /*self*/, const Lib3dsByte* /*buffer*/,
	int /*size*/)
#endif
{
  // not yet implemented
  return 0;
}


Lib3dsFile* csGenmesh3DSFactoryLoader::LoadFileData (uint8* pBuffer, size_t size)
{
  // This code is pulled from lib3ds
  Lib3dsFile *pFile;
  Lib3dsIo *pLibIO;
  csRef<iFile> pData;

  pFile = lib3ds_file_new ();
  if (!pFile) return 0;

  // create a data stream from the buffer and don't delete it
  pData.AttachNew (new csMemFile ((const char*)pBuffer, size));

  pLibIO = lib3ds_io_new (
    pData,
    DataErrorFunc,
    DataSeekFunc,
    DataTellFunc,
    DataReadFunc,
    DataWriteFunc
  );

  if (!pLibIO)
  {
    lib3ds_file_free (pFile);
    return 0;
  }

  if (!lib3ds_file_read (pFile, pLibIO))
  {
    lib3ds_file_free (pFile);
    return 0;
  }

  if (!pFile) return 0;

  lib3ds_io_free (pLibIO);
  return pFile;
}

csPtr<iBase> csGenmesh3DSFactoryLoader::Parse (iDataBuffer* buf,
				       iStreamSource*, iLoaderContext* ldr_context,
				       iBase* context, iStringArray*)
{
  materials_and_tris.Empty ();

  csRef<iPluginManager> plugin_mgr (
    csQueryRegistry<iPluginManager> (object_reg));
  csRef<iMeshObjectType> type (
    csQueryPluginClass<iMeshObjectType> (plugin_mgr, 
    "crystalspace.mesh.object.genmesh"));
  if (!type)
  {
    type = csLoadPlugin<iMeshObjectType> (plugin_mgr,
    	"crystalspace.mesh.object.genmesh");
  }
  if (!type)
  {
    ReportError (object_reg,
		"crystalspace.genmesh3dsfactoryloader.setup.objecttype",
		"Could not load the genmesh mesh object plugin!");
    return 0;
  }

  // @@@ Temporary fix to allow to set actions for objects loaded
  // with impexp. Once those loaders move to another plugin this code
  // below should be removed.
  csRef<iMeshObjectFactory> fact;
  if (context)
  {
    fact = scfQueryInterface<iMeshObjectFactory> (context);
  }

  // If there was no factory we create a new one.
  if (!fact)
    fact = type->NewFactory ();

  csRef<iGeneralFactoryState> gmstate = scfQueryInterface<iGeneralFactoryState>
  	(fact);

  uint8* p = buf->GetUint8 ();
  bool rc = Load (ldr_context, gmstate, p, buf->GetSize ());
  if (!rc) return 0;

  size_t j;
  fact->SetMaterialWrapper (materials_and_tris[0].material);
  gmstate->Compress ();
  gmstate->CalculateNormals ();
  if (materials_and_tris.GetSize () > 1)
    for (j = 0 ; j < materials_and_tris.GetSize () ; j++)
    {
      csRef<iRenderBuffer> indexBuffer = csRenderBuffer::CreateIndexRenderBuffer (
        materials_and_tris[j].tris.GetSize()*3, CS_BUF_STATIC, 
        CS_BUFCOMP_UNSIGNED_INT, 0, gmstate->GetVertexCount() - 1);
      {
        csRenderBufferLock<uint> indices (indexBuffer);
        csTriangle* tris = gmstate->GetTriangles();
        for (size_t i = 0; i < materials_and_tris[j].tris.GetSize(); i++)
        {
          uint tri = materials_and_tris[j].tris[i];
          *indices++ = tris[tri].a;
          *indices++ = tris[tri].b;
          *indices++ = tris[tri].c;
        }
      }
      gmstate->AddSubMesh (indexBuffer,
	materials_and_tris[j].material, 0);
    }
  materials_and_tris.Empty ();

  return csPtr<iBase> (fact);
}

iMeshFactoryWrapper* csGenmesh3DSFactoryLoader::Load (const char* factname,
	const char* filename, iDataBuffer* buffer)
{
  csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
  csRef<iMeshFactoryWrapper> ff = engine->CreateMeshFactory (
  	"crystalspace.mesh.object.genmesh", factname);
  csRef<iLoaderContext> ldr_context = engine->CreateLoaderContext ();
  csRef<iBase> b = Parse (buffer, 0, ldr_context, ff->GetMeshObjectFactory (), 0);
  if (!b)
  {
    ReportError (object_reg,
		"crystalspace.genmesh3dsfactoryloader.load",
		filename
			? "Error loading 3DS file %s!"
			: "Error loading 3DS file!", CS::Quote::Single (filename));
    return 0;
  }
  return ff;
}

iMeshFactoryWrapper* csGenmesh3DSFactoryLoader::Load (const char* factname,
	iDataBuffer* buffer)
{
  return Load (factname, 0, buffer);
}

iMeshFactoryWrapper* csGenmesh3DSFactoryLoader::Load (const char* factname,
	const char* filename)
{
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);
  csRef<iDataBuffer> dbuf = vfs->ReadFile (filename);
  if (!dbuf)
  {
    ReportError (object_reg,
		"crystalspace.genmesh3dsfactoryloader.load",
		"Can't load file %s!", CS::Quote::Single (filename));
    return 0;
  }
  return Load (factname, filename, dbuf);
}

}
CS_PLUGIN_NAMESPACE_END(Genmesh3DS)
