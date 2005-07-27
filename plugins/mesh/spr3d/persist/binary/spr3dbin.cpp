/*
    Copyright (C) 2002 by David M. Asbell

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

/**
 *
 * These classes Save and Load Sprites with a binary representation
 *
 */

#include "cssysdef.h"

#include "csgeom/math3d.h"
#include "csgeom/matrix3.h"
#include "csgeom/quaterni.h"
#include "csgeom/transfrm.h"
#include "csgeom/tri.h"
#include "csutil/csendian.h"
#include "csutil/csstring.h"
#include "csutil/scanstr.h"
#include "csutil/sysfunc.h"

#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "imap/ldrctxt.h"
#include "imesh/object.h"
#include "imesh/sprite3d.h"
#include "iutil/comp.h"
#include "iutil/document.h"
#include "iutil/eventh.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"

#include "spr3dbin.h"

CS_IMPLEMENT_PLUGIN

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

SCF_IMPLEMENT_IBASE (csSprite3DBinFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iBinaryLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSprite3DBinFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSprite3DBinFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iBinarySaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSprite3DBinFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csSprite3DBinFactoryLoader)
SCF_IMPLEMENT_FACTORY (csSprite3DBinFactorySaver)


/**
 * Creates a new csSprite3DBinFactoryLoader
 */
csSprite3DBinFactoryLoader::csSprite3DBinFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

/**
 * Destroys a csSprite3DBinFactoryLoader
 */
csSprite3DBinFactoryLoader::~csSprite3DBinFactoryLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

/**
 * Initializes a csSprite3DBinFactoryLoader
 */
bool csSprite3DBinFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csSprite3DBinFactoryLoader::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

const char binsprMagic[4] = {'5','1', '5','0'};


/**
 * Loads a csSprite3DBinFactoryLoader
 */
csPtr<iBase> csSprite3DBinFactoryLoader::Parse (void* data,
				       iLoaderContext* ldr_context,
				       iBase* context)
{
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS(plugin_mgr,
	"crystalspace.mesh.object.sprite.3d", iMeshObjectType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.sprite.3d",
    	iMeshObjectType);
  }
  if (!type)
  {
    ReportError (object_reg,
		"crystalspace.sprite3dbinfactoryloader.setup.objecttype",
		"Could not load the sprite.3d mesh object plugin!");
    return 0;
  }

  // @@@ Temporary fix to allow to set actions for objects loaded
  // with impexp. Once those loaders move to another plugin this code
  // below should be removed.
  csRef<iMeshObjectFactory> fact;
  if (context)
  {
    fact = SCF_QUERY_INTERFACE (context, iMeshObjectFactory);
    // DecRef of fact will be handled later.
  }

  // If there was no factory we create a new one.
  if (!fact)
    fact = type->NewFactory ();

  csRef<iSprite3DFactoryState> spr3dLook (SCF_QUERY_INTERFACE (fact,
  	iSprite3DFactoryState));

  char* p = (char*)data;

  // Read the magic number so we can ID the file
  if (memcmp(binsprMagic, p, 4) != 0)
  {
    ReportError (object_reg,
	"crystalspace.sprite3dbinfactoryloader.setup.objecttype",
	"Input was not binary sprite data!");
    return 0;
  }
  p += 4;

  // Read the version number so we can ID the file
  bool has_normals = false;
  if ( ((uint8)*p != 0x01) || ((uint8)*(p+1) > 0x01) )
  {
    ReportError (object_reg,
	"crystalspace.sprite3dbinfactoryloader.setup.objecttype",
	"Unexpected format version %" PRIu8 ".%" PRIu8 "!", 
	(uint8)*p, (uint8)*(p+1));
    return 0;
  }
  has_normals = (uint8)*(p+1) >= 0x01;
  p += 2;


  // Read and set the (one for now) material
  char mat_name[255];
  strcpy(mat_name, p);
  iMaterialWrapper* mat = ldr_context->FindMaterial (mat_name);
  if (!mat)
  {
    ReportError (object_reg,
	"crystalspace.sprite3dbinfactoryloader.parse.unknownmaterial",
	"Couldn't find material named '%s'", mat_name);
    return 0;
  }
  spr3dLook->SetMaterialWrapper (mat);
  p += strlen(mat_name) + 1;

  // Read the number of frames
  int frame_count = csConvertEndian(*((int32 *)p)); p += sizeof(int);

  // Read all the frames
  int i;
  for (i=0; i<frame_count; i++)
  {
    iSpriteFrame* fr = spr3dLook->AddFrame ();

    char frame_name[255];
    strcpy(frame_name, p);
    p += strlen(frame_name) + 1;

    fr->SetName (frame_name);

    int anm_idx = fr->GetAnmIndex ();
    int tex_idx = fr->GetTexIndex ();
    float x, y, z, u, v, nx, ny, nz;
    x = y = z = u = v = nx = ny = nz = 0.0f;

    if (!has_normals)
    {
      nx = ny = nz = 0.0f;
    }

    // Read the number of vertecies
    int vertex_count = csConvertEndian(*((int32 *)p));
    p += sizeof(int);

    int j;
    for (j = 0; j < vertex_count; j++)
    {
      x = csConvertEndian(csLongToFloat(*((long *)p))); p += sizeof(float);
      y = csConvertEndian(csLongToFloat(*((long *)p))); p += sizeof(float);
      z = csConvertEndian(csLongToFloat(*((long *)p))); p += sizeof(float);
      u = csConvertEndian(csLongToFloat(*((long *)p))); p += sizeof(float);
      v = csConvertEndian(csLongToFloat(*((long *)p))); p += sizeof(float);
      if (has_normals)
      {
	nx = csConvertEndian(csLongToFloat(*((long *)p))); p += sizeof(float);
	ny = csConvertEndian(csLongToFloat(*((long *)p))); p += sizeof(float);
	nz = csConvertEndian(csLongToFloat(*((long *)p))); p += sizeof(float);
      }

      // check if it's the first frame
      if (spr3dLook->GetFrameCount () == 1)
      {
	spr3dLook->AddVertices (1);
      }
      else if (i >= spr3dLook->GetVertexCount ())
      {
	ReportError (object_reg,
	    "crystalspace.sprite3dbinfactoryloader.parse.frame.vertices",
	    "Trying to add too many vertices to frame '%s'!",
	    fr->GetName ());
	return 0;
      }
      spr3dLook->SetVertex (anm_idx, j, csVector3 (x, y, z));
      spr3dLook->SetTexel  (tex_idx, j, csVector2 (u, v));
      spr3dLook->SetNormal (anm_idx, j, csVector3 (nx, ny, nz));
    }

    if (j < spr3dLook->GetVertexCount ())
    {
      ReportError (object_reg,
	"crystalspace.sprite3dbinfactoryloader.parse.frame.vertices",
	"Too few vertices in frame '%s'!",
	fr->GetName ());
      return 0;
    }
  }

  // Read the number of actions
  int action_count = csConvertEndian(*((int32 *)p)); p += sizeof(int);


  // Read each action
  for (i=0; i<action_count; i++)
  {
    iSpriteAction* act = spr3dLook->AddAction ();

    char action_name[255];
    strcpy(action_name, p);
    p += strlen(action_name) + 1;

    act->SetName (action_name);

    int as = csConvertEndian(*((int32 *)p)); p += sizeof(int);

    int j;
    for (j = 0; j < as; j++)
    {
      char fn[64];
      strcpy(fn, p);
      p += strlen(fn) + 1;

      iSpriteFrame* ff = spr3dLook->FindFrame (fn);
      if (!ff)
      {
	ReportError (object_reg,
	  "crystalspace.sprite3dbinfactoryloader.parse.action.badframe",
	  "Trying to add unknown frame '%s' to action '%s'!",
	  fn, act->GetName ());
	return 0;
      }

      // Read the delay
      int delay = csConvertEndian(*((int32 *)p)); p += sizeof(int);
      float disp = 0;
      if (!delay)  // read optional displacement if no delay
      {
        disp = csConvertEndian(csLongToFloat(*((long *)p))); p += sizeof(float);
      }
      act->AddFrame (ff, delay,disp);
    }
  }

  // Read the number of triangles
  int tri_count = csConvertEndian(*((int32 *)p)); p += sizeof(int);

  for(i=0; i<tri_count; i++)
  {
    int a = csConvertEndian(*((int32 *)p)); p += sizeof(int);
    int b = csConvertEndian(*((int32 *)p)); p += sizeof(int);
    int c = csConvertEndian(*((int32 *)p)); p += sizeof(int);

    spr3dLook->AddTriangle (a, b, c);
  }

  // Read the number of sockets
  int socket_count = csConvertEndian(*((int32 *)p)); p += sizeof(int);

  for(i=0; i<socket_count; i++)
  {
    char name[64];
    strcpy(name, p);
    p += strlen(name) + 1;
 
    int a = csConvertEndian(*((int32 *)p)); p += sizeof(int);
 
    iSpriteSocket* socket = spr3dLook->AddSocket();
    socket->SetName(name);
    socket->SetTriangleIndex(a);
  }

  /// @@@ Cannot retrieve smoothing information.
  /// SMOOTH()
  /// SMOOTH(baseframenr)
  /// or a list of SMOOTH(basenr, framenr);


  // Read the 1 byte tween value
  spr3dLook->EnableTweening (*p++);
  return csPtr<iBase> (fact);
}

//---------------------------------------------------------------------------

/**
 * Creates a csSprite3DBinFactorySaver
 */
csSprite3DBinFactorySaver::csSprite3DBinFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

/**
 * Destroys a csSprite3DBinFactorySaver
 */
csSprite3DBinFactorySaver::~csSprite3DBinFactorySaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

/**
 * Initializes a csSprite3DBinFactorySaver
 */
bool csSprite3DBinFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csSprite3DBinFactorySaver::object_reg = object_reg;
  return true;
}


/**
 * Saves a csSprite3DBinFactorySaver
 */
bool csSprite3DBinFactorySaver::WriteDown (iBase* obj, iFile* file)
{
  const char * name = 0;

  if (!obj) return false;

  csRef<iSprite3DFactoryState> state (
    SCF_QUERY_INTERFACE (obj, iSprite3DFactoryState));

  // Write a magic number so we can ID the file
  file->Write (binsprMagic, 4);

  // Write a version
  char ver[2] = {0x01, 0x01}; // Major, Minor
  file->Write (ver, 2);

  // Write out the material... This can easily expanded to multiple
  // materials later.
  name = state->GetMaterialWrapper()->QueryObject ()->GetName();
  file->Write (name, strlen(name) + 1);

  // Write the number of frames
  int frame_count = state->GetFrameCount();
  int fc = csConvertEndian((int32)frame_count);
  file->Write ((char *)&fc, 4);

  // Write all the frames
  int i,j;
  for(i=0; i<state->GetFrameCount(); i++)
  {
    iSpriteFrame* frame = state->GetFrame(i);

    // Write out the frame name
    name = frame->GetName();
    file->Write (name, strlen(name) + 1);

    // Write the number of verts
    int vertex_count = state->GetVertexCount();
    int vc = csConvertEndian((int32)vertex_count);
    file->Write ((char *)&vc, 4);

    // Get the animation and textures indicies for the frame
    int anm_idx = frame->GetAnmIndex ();
    int tex_idx = frame->GetTexIndex ();

    // Write out each vertex and texcel coord
    for (j=0; j<vertex_count; j++)
    {
      long v;
      v = csConvertEndian(csFloatToLong(state->GetVertex(anm_idx, j).x));
      file->Write ((char *)&v, 4);
      v = csConvertEndian(csFloatToLong(state->GetVertex(anm_idx, j).y));
      file->Write ((char *)&v, 4);
      v = csConvertEndian(csFloatToLong(state->GetVertex(anm_idx, j).z));
      file->Write ((char *)&v, 4);
      v = csConvertEndian(csFloatToLong(state->GetTexel(tex_idx, j).x));
      file->Write ((char *)&v, 4);
      v = csConvertEndian(csFloatToLong(state->GetTexel(tex_idx, j).y));
      file->Write ((char *)&v, 4);
      v = csConvertEndian(csFloatToLong(state->GetNormal(anm_idx, j).x));
      file->Write ((char *)&v, 4);
      v = csConvertEndian(csFloatToLong(state->GetNormal(anm_idx, j).y));
      file->Write ((char *)&v, 4);
      v = csConvertEndian(csFloatToLong(state->GetNormal(anm_idx, j).z));
      file->Write ((char *)&v, 4);
    }
  }

  // Write out the number of actions
  int action_count = state->GetActionCount();
  int ac = csConvertEndian((int32)action_count);
  file->Write ((char *)&ac, 4);

  // Write out each action
  for(i=0; i<action_count; i++)
  {
    iSpriteAction* action = state->GetAction(i);

    // Write the name of the action
    name = action->GetName();
    file->Write (name, strlen(name) + 1);

    // Write out the number of frames
    int action_frame_count = action->GetFrameCount();
    int afc = csConvertEndian((int32)action_frame_count);
    file->Write ((char *)&afc, 4);

    // Write the frame name, delay tuples
    for (j=0; j<action_frame_count; j++)
    {
      name = action->GetFrame(j)->GetName();
      file->Write (name, strlen(name) + 1);

      int frame_delay = action->GetFrameDelay(j);
      int fd = csConvertEndian((int32)frame_delay);
      file->Write ((char *)&fd, 4);
      float disp = 0;
      if (!frame_delay)  // write optional displacement if no delay
      {
        disp = action->GetFrameDisplacement(j);
	long ce_disp = csConvertEndian(csFloatToLong(disp));
        file->Write ((char *)&ce_disp, 4);
      }
    }
  }

  // Write out the number of triangles
  int tri_count = state->GetTriangleCount();
  int tc = csConvertEndian((int32)tri_count);
  file->Write ((char *)&tc, 4);

  for (i=0; i<tri_count; i++)
  {
    int idx;
    idx = csConvertEndian((int32)state->GetTriangle(i).a);
    file->Write ((char *)&idx, 4);
    idx = csConvertEndian((int32)state->GetTriangle(i).b);
    file->Write ((char *)&idx, 4);
    idx = csConvertEndian((int32)state->GetTriangle(i).c);
    file->Write ((char *)&idx, 4);
  }

  // Write out the number of sockets
  int socket_count = state->GetSocketCount();
  int sc = csConvertEndian((int32)socket_count);
  file->Write ((char *)&sc, 4);

  for (i=0; i<socket_count; i++)
  {
    name = state->GetSocket(i)->GetName();
    file->Write (name, strlen(name) + 1);

    int idx;
    idx = csConvertEndian((int32)state->GetSocket(i)->GetTriangleIndex());
    file->Write ((char *)&idx, 4);
  }
  // [res] the following doesn't matter as the normals are saved:
  /// @@@ Cannot retrieve smoothing information.
  /// SMOOTH()
  /// SMOOTH(baseframenr)
  /// or a list of SMOOTH(basenr, framenr);

  // Write out TWEEN state
  char buf[1];
  buf[0] = state->IsTweeningEnabled() ? 0x01 : 0x00;
  file->Write(buf, 1);

  return true;
}

