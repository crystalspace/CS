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
#include "csutil/stringarray.h"
#include "csutil/filereadhelper.h"
#include "csutil/memfile.h"
#include "csutil/hash.h"
#include "csutil/comparator.h"
#include "csutil/dirtyaccessarray.h"

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

#include "spr3md2.h"

#include <ctype.h>

struct csVertexTexel
{
  int vt;
  int texel;

  csVertexTexel ()
  	: vt (0), texel (0) { }
  csVertexTexel (int vt, int texel)
  	: vt (vt), texel (texel) { }
  csVertexTexel (const csVertexTexel& other)
  	: vt (other.vt), texel (other.texel) { }

  bool operator== (const csVertexTexel& other)
  {
    return other.vt == vt && other.texel == texel;
  }
  inline friend bool operator < (const csVertexTexel& r1, const csVertexTexel& r2)
  {
    if (r1.vt < r2.vt) return true;
    else if (r1.vt > r2.vt) return false;
    else return r1.texel < r2.texel;
  }
};

template<>
class csHashComputer<csVertexTexel>
{
public:
  /// Compute a hash value for \a key.
  static uint ComputeHash (const csVertexTexel& key)
  {
    return (uintptr_t)(key.vt + key.texel * 196613);
  }
};

using namespace CS::Plugins::Spr3dMd2;

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

SCF_IMPLEMENT_FACTORY (csSprite3DMD2FactoryLoader)

/**
 * Creates a new csSprite3DMD2FactoryLoader
 */
csSprite3DMD2FactoryLoader::csSprite3DMD2FactoryLoader (iBase* pParent) :
  scfImplementationType (this, pParent)
{
}

/**
 * Destroys a csSprite3DMD2FactoryLoader
 */
csSprite3DMD2FactoryLoader::~csSprite3DMD2FactoryLoader ()
{
}

/**
 * Initializes a csSprite3DMD2FactoryLoader
 */
bool csSprite3DMD2FactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csSprite3DMD2FactoryLoader::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);
  return true;
}

// upper bound onsize of biggest data element (vertex, polygon) in an MD2 file
static int const MAX_DATAELEMENT_SIZE = 8192;

// size of various MD2 elements
static int const SIZEOF_MD2SKINNAME = 64;
static int const SIZEOF_MD2FRAMENAME = 16;

static const float SCALE_FACTOR = 0.025f;
static const float FRAME_DELAY = 0.1f;

struct csMD2Header
{
  // width and height of skin texture in pixels
  int32 SkinWidth, SkinHeight;
  // size of each frame int the sprite, in pixels
  int32 FrameSize;
  // number of skins, vertices, texels, triangles, glcmds(?), frames
  int32 SkinCount, VertexCount, TexelCount, TriangleCount, glcmds, FrameCount;
  // offset of the skin information in the file
  int32 SkinOffset;
  // offset of the texel information in the file
  int32 TexelOffset;
  // offset of the triangle information in the file
  int32 TriangleOffset;
  // offset of the frame information in the file
  int32 FramesOffset;
  // offset of the GL commands information in the file
  int32 GLCommandsOffset;
  // total file size
  int32 FileSize;
};

static bool CheckMD2Version (csFileReadHelper& in)
{
  // Read in header and check for a correct file.  The
  // header consists of two longs, containing
  // the magic identifiter 'IDP2' as the first long,
  // followed by a version number (8)
  uint32 FileID, FileVersion;
  in.ReadUInt32 (FileID);
  in.ReadUInt32 (FileVersion);
  FileID = csLittleEndian::Convert (FileID);
  FileVersion = csLittleEndian::Convert (FileVersion);

  if (FileID != ( ('2'<<24)+('P'<<16)+('D'<<8)+'I' ) )
    return false;

  if (FileVersion != 8)
    return false;

  return true;
}

bool csSprite3DMD2FactoryLoader::TestMD2 (uint8 *Buffer, size_t Size)
{
  // Prepare input buffer
  csRef<iFile> file;
  file.AttachNew (new csMemFile ((const char*)Buffer, Size));
  csFileReadHelper in (file);
  // Check for the correct version
  if (!CheckMD2Version (in))
    return false;
  return true;
}

bool csSprite3DMD2FactoryLoader::IsRecognized (const char* filename)
{
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);
  csRef<iDataBuffer> dbuf = vfs->ReadFile (filename);
  if (!dbuf) return false;
  return IsRecognized (dbuf);
}

bool csSprite3DMD2FactoryLoader::IsRecognized (iDataBuffer* buffer)
{
  return TestMD2 (buffer->GetUint8 (), buffer->GetSize ());
}

#define CS_MD2_READ(num,name)						\
  hdr->name = csLittleEndian::Convert (csGetFromAddress::Int32 (	\
    buf + num * sizeof (int32)));

static void ReadMD2Header (csMD2Header *hdr, csFileReadHelper* in)
{
  char buf [sizeof (csMD2Header)];
  in->GetFile()->Read (buf, sizeof (csMD2Header));

  CS_MD2_READ (0, SkinWidth);
  CS_MD2_READ (1, SkinHeight);
  CS_MD2_READ (2, FrameSize);
  CS_MD2_READ (3, SkinCount);
  CS_MD2_READ (4, VertexCount);
  CS_MD2_READ (5, TexelCount);
  CS_MD2_READ (6, TriangleCount);
  CS_MD2_READ (7, glcmds);
  CS_MD2_READ (8, FrameCount);
  CS_MD2_READ (9, SkinOffset);
  CS_MD2_READ (10, TexelOffset);
  CS_MD2_READ (11, TriangleOffset);
  CS_MD2_READ (12, FramesOffset);
  CS_MD2_READ (13, GLCommandsOffset);
  CS_MD2_READ (14, FileSize);
}

#undef CS_MD2_READ

static void extractActionName (const char * str, char * act)
{
  size_t i;
  for (i = 0; str[i] != '\0' && !isdigit(str[i]); i++)
    act[i] = str[i];
  act[i] = 0;
}

struct csFrame
{
  csString name;
  csDirtyAccessArray<csVector3> vertices;
};


struct csFrameTime
{
  float time;
  size_t frameidx;
};

struct csAction
{
  csString name;
  csArray<csFrameTime> frames;
};

bool csSprite3DMD2FactoryLoader::Load (iSprite3DFactoryState* state,
	uint8 *Buffer, size_t Size)
{
  // Prepare input buffer
  csRef<iFile> file;
  file.AttachNew (new csMemFile ((const char*)Buffer, Size));
  csFileReadHelper in (file);
  uint8 readbuffer[MAX_DATAELEMENT_SIZE];
  int i,j;

  // Check for the correct version
  if (!CheckMD2Version (in))
    return false;

  // Read MD2 header
  csMD2Header Header;
  ReadMD2Header (&Header, &in);

  // The data we read in.
  csDirtyAccessArray<csTriangle> triangles;
  csDirtyAccessArray<csVector2> Texels;
  csArray<csFrame> frames;
  csArray<csAction> actions;
  csStringArray SkinNames;
  // Map between the vertex/texel combination and the final vertex index.
  csHash<size_t, csVertexTexel> vertex_mapping;
  csArray<csVertexTexel> mapped_vertices;

  // Read in texmap (skin) names - skin names are 64 bytes long
  // Unused
  in.GetFile()->SetPos (Header.SkinOffset);
  for (i = 0; i < Header.SkinCount; i++)
  {
    char name[SIZEOF_MD2SKINNAME];
    in.GetFile()->Read (name, SIZEOF_MD2SKINNAME);
    SkinNames.Push (name);
  }

  // Next we read in the triangle connectivity data.  This data describes
  // each triangle as three indices, referring to three numbered vertices.
  // This data is, like the skin texture coords, independent of frame number.
  // There are actually two set of indices in the original quake file;
  // one indexes into the xyz coordinate table, the other indexes into
  // the uv texture coordinate table.
  in.GetFile()->SetPos (Header.TriangleOffset);
  triangles.SetCapacity (Header.TriangleCount);
  for (i = 0; i < Header.TriangleCount; i++)
  {
    in.GetFile()->Read ((char*)readbuffer, sizeof (uint16)*6);
    csVertexTexel vt;
    size_t idx[3];
    for (int j = 0 ; j < 3 ; j++)
    {
      vt.vt = csLittleEndian::Convert (csGetFromAddress::UInt16 (
	readbuffer + j * sizeof (uint16)));
      vt.texel = csLittleEndian::Convert (csGetFromAddress::UInt16 (
	readbuffer + (j+3) * sizeof (uint16)));
      idx[j] = vertex_mapping.Get (vt, csArrayItemNotFound);
      if (idx[j] == csArrayItemNotFound)
      {
        idx[j] = mapped_vertices.Push (vt);
        vertex_mapping.Put (vt, idx[j]);
      }
    }
    triangles.Push (csTriangle (int (idx[0]), int (idx[1]), int (idx[2])));
  }

  // Read in skin data. This contains texture map coordinates for each
  // vertex; the spatial location of each vertex varies with each
  // frame, and is stored elsewhere, in the frame data section.
  // The only data we read here
  // are the static texture map (uv) locations for each vertex!
  in.GetFile()->SetPos (Header.TexelOffset);
  csArray<csVector2> intexels;
  intexels.SetCapacity (Header.TexelCount);
  for (i = 0; i < Header.TexelCount; i++)
  {
    int16 x, y;
    in.ReadInt16 (x);
    in.ReadInt16 (y);
    intexels.Push (
    	csVector2 (csLittleEndian::Convert (x)/(float)Header.SkinWidth,
                   csLittleEndian::Convert (y)/(float)Header.SkinHeight));
  }
  // Now map the read texels to the real data.
  Texels.SetSize (mapped_vertices.GetSize ());
  size_t fi;
  for (fi = 0 ; fi < mapped_vertices.GetSize () ; fi++)
    Texels[fi] = intexels[mapped_vertices[fi].texel];

  // Now we read in the frames.  The number of frames is stored in 'num_object'
  csString currAction;
  size_t currActionIdx = csArrayItemNotFound;
  float time = 0;

  in.GetFile()->SetPos (Header.FramesOffset);
  frames.SetCapacity (Header.FrameCount);
  for (i = 0; i < Header.FrameCount; i++, time += FRAME_DELAY)
  {
    size_t idx = frames.Push (csFrame ());
    csFrame& frame = frames[idx];

    csVector3 scale, translate;
    // read in scale and translate info
    {
      uint32 u;
      for (j = 0; j < 3; j++)
      {
	in.ReadUInt32 (u);
	scale[j] = csIEEEfloat::ToNative (csLittleEndian::Convert (u));
      }
      for (j = 0; j < 3; j++)
      {
	in.ReadUInt32 (u);
	translate[j] = csIEEEfloat::ToNative (csLittleEndian::Convert (u));
      }
    }
    scale *= SCALE_FACTOR;
    translate *= SCALE_FACTOR;

    // name of this frame
    char FrameName [SIZEOF_MD2FRAMENAME+1];
    char ActionName [SIZEOF_MD2FRAMENAME+1];
    in.GetFile()->Read (FrameName, SIZEOF_MD2FRAMENAME);
    FrameName [SIZEOF_MD2FRAMENAME] = 0;
    frame.name = FrameName;
    extractActionName (FrameName, ActionName);
    if (currActionIdx == csArrayItemNotFound ||
    	strcmp (ActionName, actions[currActionIdx].name))
    {
      currActionIdx = actions.Push (csAction ());
      actions[currActionIdx].name = ActionName;
      time = FRAME_DELAY;
    }

    // read in vertex coordinate data for the frame
    in.GetFile()->Read ((char*)readbuffer, 4*Header.VertexCount);

    csFrameTime ft;
    ft.time = time;
    ft.frameidx = i;
    actions[currActionIdx].frames.Push (ft);

    csArray<csVector3> inframe;
    inframe.SetCapacity (Header.VertexCount);
    for (j = 0; j < Header.VertexCount; j++)
    {
      const uint8 * buf = readbuffer + j*4;
      csVector3 v (buf [0], buf [1], buf [2]);
      v.x = v.x * scale.x + translate.x;
      v.y = v.y * scale.y + translate.y;
      v.z = v.z * scale.z + translate.z;
      // swap y and z
      float t = v.y;
      v.y = v.z;
      v.z = t;
 
      inframe.Push (v);
    }
    // Now map the read vertices to the real data.
    frame.vertices.SetSize (mapped_vertices.GetSize ());
    for (fi = 0 ; fi < mapped_vertices.GetSize () ; fi++)
      frame.vertices[fi] = inframe[mapped_vertices[fi].vt];
  }

  // Now fill the sprite.
  state->SetTriangles (triangles.GetArray (), int (triangles.GetSize ()));
  for (j = 0 ; j < int (frames.GetSize ()) ; j++)
  {
    csFrame& f = frames[j];
    iSpriteFrame* fr = state->AddFrame ();
    fr->SetName (f.name);
    if (j == 0) state->AddVertices (int (mapped_vertices.GetSize ()));
    state->SetVertices (f.vertices.GetArray (), j);
    state->SetTexels (Texels.GetArray (), j);
  }
  for (j = 0 ; j < int (actions.GetSize ()) ; j++)
  {
    csAction& a = actions[j];
    iSpriteAction* action = state->AddAction ();
    action->SetName (a.name);
    size_t k;
    for (k = 0 ; k < a.frames.GetSize () ; k++)
    {
      csFrameTime& ft = a.frames[k];
      action->AddFrame (state->GetFrame (int (ft.frameidx)), 
        csTicks (100.0f * ft.time), 0.0f);
    }
  }
  state->MergeNormals ();

  return true;
}

/**
 * Loads a csSprite3DMD2FactoryLoader
 */
csPtr<iBase> csSprite3DMD2FactoryLoader::Parse (iDataBuffer* data,
				       iStreamSource*, iLoaderContext*,
				       iBase* context, iStringArray*)
{
  csRef<iPluginManager> plugin_mgr (
    csQueryRegistry<iPluginManager> (object_reg));
  csRef<iMeshObjectType> type (
    csQueryPluginClass<iMeshObjectType> (plugin_mgr, 
    "crystalspace.mesh.object.sprite.3d"));
  if (!type)
  {
    type = csLoadPlugin<iMeshObjectType> (plugin_mgr,
    	"crystalspace.mesh.object.sprite.3d");
  }
  if (!type)
  {
    ReportError (object_reg,
		"crystalspace.sprite3dmd2factoryloader.setup.objecttype",
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

  csRef<iSprite3DFactoryState> state = scfQueryInterface<iSprite3DFactoryState>
    (fact);

  bool rc = Load (state, data->GetUint8 (), data->GetSize ());
  if (!rc) return 0;

  return csPtr<iBase> (fact);
}

iMeshFactoryWrapper* csSprite3DMD2FactoryLoader::Load (const char* factname,
	const char* filename, iDataBuffer* buffer)
{
  csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
  csRef<iMeshFactoryWrapper> ff = engine->CreateMeshFactory (
  	"crystalspace.mesh.object.sprite.3d", factname);
  csRef<iLoaderContext> ldr_context = engine->CreateLoaderContext ();
  csRef<iBase> b = Parse (buffer, 0, ldr_context, ff->GetMeshObjectFactory (), 0);
  if (!b)
  {
    ReportError (object_reg,
		"crystalspace.sprite3dmd2factoryloader.load",
		filename
		    ? "Error loading MD2 file %s!"
		    : "Error loading MD2 file!", CS::Quote::Single (filename));
    return 0;
  }
  return ff;
}

iMeshFactoryWrapper* csSprite3DMD2FactoryLoader::Load (const char* factname,
	iDataBuffer* buffer)
{
  return Load (factname, 0, buffer);
}

iMeshFactoryWrapper* csSprite3DMD2FactoryLoader::Load (const char* factname,
	const char* filename)
{
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);
  csRef<iDataBuffer> dbuf = vfs->ReadFile (filename);
  if (!dbuf)
  {
    ReportError (object_reg,
		"crystalspace.sprite3dmd2factoryloader.load",
		"Can't load file %s!", CS::Quote::Single (filename));
    return 0;
  }
  return Load (factname, filename, dbuf);
}

