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
#include "csgeom/quaternion.h"
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

using namespace CS::Plugins::Spr3dBin;

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

SCF_IMPLEMENT_FACTORY (csSprite3DBinFactoryLoader)
SCF_IMPLEMENT_FACTORY (csSprite3DBinFactorySaver)

/**
 * Creates a new csSprite3DBinFactoryLoader
 */
csSprite3DBinFactoryLoader::csSprite3DBinFactoryLoader (iBase* pParent) :
  scfImplementationType (this, pParent)
{
}

/**
 * Destroys a csSprite3DBinFactoryLoader
 */
csSprite3DBinFactoryLoader::~csSprite3DBinFactoryLoader ()
{
}

/**
 * Initializes a csSprite3DBinFactoryLoader
 */
bool csSprite3DBinFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csSprite3DBinFactoryLoader::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);
  return true;
}

const char binsprMagic_oldfloat[4] = {'5','1', '5','0'};
const char binsprMagic[4] = {'6','1', '5','0'};

static int GetInt32 (const char*& p)
{
  int i = csLittleEndian::Convert (csGetFromAddress::Int32 (p));
  p += sizeof(int32);
  return i;
}

struct FloatGetter_Compat
{
  static float GetFloat (const char*& p)
  {
    int32 l = GetInt32 (p);
    // Copied from csendian.h to avoid deprecation warnings
    int exp = (l >> 24) & 0x7f;
    if (exp & 0x40) exp = exp | ~0x7f;
    float mant = float (l & 0x00ffffff) / 0x1000000;
    if (l & 0x80000000) mant = -mant;
    return (float) ldexp (mant, exp);
  }
};
struct FloatGetter_IEEE
{
  static float GetFloat (const char*& p)
  {
    int32 l = GetInt32 (p);
    return csIEEEfloat::ToNative (uint32 (l));
  }
};

/**
 * Loads a csSprite3DBinFactoryLoader
 */
csPtr<iBase> csSprite3DBinFactoryLoader::Parse (iDataBuffer* data,
				       iStreamSource*, iLoaderContext* ldr_context,
				       iBase* context, iStringArray*)
{
  csRef<iPluginManager> plugin_mgr (
    csQueryRegistry<iPluginManager> (object_reg));
  csRef<iMeshObjectType> type (
    csQueryPluginClass<iMeshObjectType> (plugin_mgr, 
    "crystalspace.mesh.object.sprite.3d"));
  if (!type)
  {
    type = csLoadPlugin<iMeshObjectType> (plugin_mgr, "crystalspace.mesh.object.sprite.3d");
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
    fact = scfQueryInterface<iMeshObjectFactory> (context);
  }

  // If there was no factory we create a new one.
  if (!fact)
    fact = type->NewFactory ();

  csRef<iSprite3DFactoryState> spr3dLook (
    scfQueryInterface<iSprite3DFactoryState> (fact));

  const char* p = data->GetData();

  // Read the magic number so we can ID the file
  if ((memcmp(binsprMagic, p, 4) != 0)
      && (memcmp(binsprMagic_oldfloat, p, 4) != 0))
  {
    ReportError (object_reg,
	"crystalspace.sprite3dbinfactoryloader.setup.objecttype",
	"Input was not binary sprite data!");
    return 0;
  }
  p += 4;
  
  bool floatCompat = memcmp(binsprMagic_oldfloat, p, 4) == 0;

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
	"Couldn't find material named %s", CS::Quote::Single (mat_name));
    return 0;
  }
  fact->SetMaterialWrapper (mat);
  p += strlen(mat_name) + 1;

  // Read the number of frames
  int frame_count = GetInt32 (p);

  // Read all the frames
  int i;
  for (i=0; i<frame_count; i++)
  {
    iSpriteFrame* fr = spr3dLook->AddFrame ();

    bool result;
    if (floatCompat)
      result = ReadFrame<FloatGetter_Compat> (spr3dLook, fr, p, has_normals);
    else
      result = ReadFrame<FloatGetter_IEEE> (spr3dLook, fr, p, has_normals);
    if (!result)
      return (iBase*)nullptr;
  }

  // Read the number of actions
  int action_count = GetInt32 (p);


  // Read each action
  for (i=0; i<action_count; i++)
  {
    iSpriteAction* act = spr3dLook->AddAction ();

    bool result;
    if (floatCompat)
      result = ReadAction<FloatGetter_Compat> (spr3dLook, act, p);
    else
      result = ReadAction<FloatGetter_IEEE> (spr3dLook, act, p);
    if (!result)
      return (iBase*)nullptr;
  }

  // Read the number of triangles
  int tri_count = GetInt32 (p);

  for(i=0; i<tri_count; i++)
  {
    int a = GetInt32 (p);
    int b = GetInt32 (p);
    int c = GetInt32 (p);

    spr3dLook->AddTriangle (a, b, c);
  }

  // Read the number of sockets
  int socket_count = GetInt32 (p);

  for(i=0; i<socket_count; i++)
  {
    char name[64];
    strcpy(name, p);
    p += strlen(name) + 1;
 
    int a = GetInt32 (p);
 
    iSpriteSocket* socket = spr3dLook->AddSocket();
    socket->SetName(name);
    socket->SetTriangleIndex(a);
  }

  /// @@@ Cannot retrieve smoothing information.
  /// SMOOTH()
  /// SMOOTH(baseframenr)
  /// or a list of SMOOTH(basenr, framenr);


  // Read the 1 byte tween value
  spr3dLook->EnableTweening ((*p++) != 0);
  return csPtr<iBase> (fact);
}

template<typename FloatGetter>
bool csSprite3DBinFactoryLoader::ReadFrame (iSprite3DFactoryState* spr3dLook,
					    iSpriteFrame* fr,
					    const char*& p,
					    bool has_normals)
{
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
  int vertex_count = GetInt32 (p);
  p += sizeof(int);

  int j;
  for (j = 0; j < vertex_count; j++)
  {
    x = FloatGetter::GetFloat (p);
    y = FloatGetter::GetFloat (p);
    z = FloatGetter::GetFloat (p);
    u = FloatGetter::GetFloat (p);
    v = FloatGetter::GetFloat (p);
    if (has_normals)
    {
      nx = FloatGetter::GetFloat (p);
      ny = FloatGetter::GetFloat (p);
      nz = FloatGetter::GetFloat (p);
    }

    // check if it's the first frame
    if (spr3dLook->GetFrameCount () == 1)
    {
      spr3dLook->AddVertices (1);
    }
    else if (j >= spr3dLook->GetVertexCount ())
    {
      ReportError (object_reg,
	  "crystalspace.sprite3dbinfactoryloader.parse.frame.vertices",
	  "Trying to add too many vertices to frame %s!",
	  CS::Quote::Single (fr->GetName ()));
      return false;
    }
    spr3dLook->SetVertex (anm_idx, j, csVector3 (x, y, z));
    spr3dLook->SetTexel  (tex_idx, j, csVector2 (u, v));
    spr3dLook->SetNormal (anm_idx, j, csVector3 (nx, ny, nz));
  }

  if (j < spr3dLook->GetVertexCount ())
  {
    ReportError (object_reg,
      "crystalspace.sprite3dbinfactoryloader.parse.frame.vertices",
      "Too few vertices in frame %s!",
      CS::Quote::Single (fr->GetName ()));
    return false;
  }
  
  return true;
}

template<typename FloatGetter>
bool csSprite3DBinFactoryLoader::ReadAction (iSprite3DFactoryState* spr3dLook, iSpriteAction* act, const char*& p)
{
  char action_name[255];
  strcpy(action_name, p);
  p += strlen(action_name) + 1;

  act->SetName (action_name);

  int as = GetInt32 (p);

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
	"Trying to add unknown frame %s to action %s!",
	CS::Quote::Single (fn), CS::Quote::Single (act->GetName ()));
      return false;
    }

    // Read the delay
    int delay = GetInt32 (p);
    float disp = 0;
    if (!delay)  // read optional displacement if no delay
    {
      disp = FloatGetter::GetFloat (p);
    }
    act->AddFrame (ff, delay,disp);
  }
  
  return true;
}

//---------------------------------------------------------------------------

/**
 * Creates a csSprite3DBinFactorySaver
 */
csSprite3DBinFactorySaver::csSprite3DBinFactorySaver (iBase* pParent) :
  scfImplementationType (this, pParent)
{
}

/**
 * Destroys a csSprite3DBinFactorySaver
 */
csSprite3DBinFactorySaver::~csSprite3DBinFactorySaver ()
{
}

/**
 * Initializes a csSprite3DBinFactorySaver
 */
bool csSprite3DBinFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csSprite3DBinFactorySaver::object_reg = object_reg;
  return true;
}

static void WriteInt32 (iFile* file, int i)
{
  int j = csLittleEndian::Convert (int32 (i));
  file->Write ((char*)&j, sizeof (int32));
}

static void WriteFloat (iFile* file, float f)
{
  WriteInt32 (file, csIEEEfloat::FromNative (f));
}

/**
 * Saves a csSprite3DBinFactorySaver
 */
bool csSprite3DBinFactorySaver::WriteDown (iBase* obj, iFile* file,
	iStreamSource*)
{
  const char * name = 0;

  if (!obj) return false;

  csRef<iMeshObjectFactory> fact = scfQueryInterface<iMeshObjectFactory> (
  	obj);
  csRef<iSprite3DFactoryState> state =
    scfQueryInterface<iSprite3DFactoryState> (obj);

  // Write a magic number so we can ID the file
  file->Write (binsprMagic, 4);

  // Write a version
  char ver[2] = {0x01, 0x01}; // Major, Minor
  file->Write (ver, 2);

  // Write out the material... This can easily expanded to multiple
  // materials later.
  name = fact->GetMaterialWrapper()->QueryObject ()->GetName();
  file->Write (name, strlen(name) + 1);

  // Write the number of frames
  WriteInt32 (file, state->GetFrameCount());

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
    WriteInt32 (file, vertex_count);

    // Get the animation and textures indicies for the frame
    int anm_idx = frame->GetAnmIndex ();
    int tex_idx = frame->GetTexIndex ();

    // Write out each vertex and texcel coord
    for (j=0; j<vertex_count; j++)
    {
      WriteFloat (file, state->GetVertex(anm_idx, j).x);
      WriteFloat (file, state->GetVertex(anm_idx, j).y);
      WriteFloat (file, state->GetVertex(anm_idx, j).z);
      WriteFloat (file, state->GetTexel(tex_idx, j).x);
      WriteFloat (file, state->GetTexel(tex_idx, j).y);
      WriteFloat (file, state->GetNormal(anm_idx, j).x);
      WriteFloat (file, state->GetNormal(anm_idx, j).y);
      WriteFloat (file, state->GetNormal(anm_idx, j).z);
    }
  }

  // Write out the number of actions
  int action_count = state->GetActionCount();
  WriteInt32 (file, action_count);

  // Write out each action
  for(i=0; i<action_count; i++)
  {
    iSpriteAction* action = state->GetAction(i);

    // Write the name of the action
    name = action->GetName();
    file->Write (name, strlen(name) + 1);

    // Write out the number of frames
    int action_frame_count = action->GetFrameCount();
    WriteInt32 (file, action_frame_count);

    // Write the frame name, delay tuples
    for (j=0; j<action_frame_count; j++)
    {
      name = action->GetFrame(j)->GetName();
      file->Write (name, strlen(name) + 1);

      int frame_delay = action->GetFrameDelay(j);
      WriteInt32 (file, frame_delay);
      float disp = 0;
      if (!frame_delay)  // write optional displacement if no delay
      {
        disp = action->GetFrameDisplacement(j);
	WriteFloat (file, disp);
      }
    }
  }

  // Write out the number of triangles
  int tri_count = state->GetTriangleCount();
  WriteInt32 (file, tri_count);

  for (i=0; i<tri_count; i++)
  {
    WriteInt32 (file, state->GetTriangle(i).a);
    WriteInt32 (file, state->GetTriangle(i).b);
    WriteInt32 (file, state->GetTriangle(i).c);
  }

  // Write out the number of sockets
  int socket_count = state->GetSocketCount();
  WriteInt32 (file, socket_count);

  for (i=0; i<socket_count; i++)
  {
    name = state->GetSocket(i)->GetName();
    file->Write (name, strlen(name) + 1);

    WriteInt32 (file, state->GetSocket(i)->GetTriangleIndex());
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


