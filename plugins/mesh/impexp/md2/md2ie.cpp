/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "imesh/mdlconv.h"
#include "cstool/mdldata.h"
#include "csutil/datastrm.h"
#include "csutil/csstring.h"
#include "csutil/nobjvec.h"
#include "csutil/parray.h"
#include "csutil/util.h"

// all int's in an MD2 file are little endian
#include "csutil/csendian.h"

// upper bound onsize of biggest data element (vertex, polygon) in an MD2 file
static int const MAX_DATAELEMENT_SIZE = 8192;

// size of various MD2 elements
static int const SIZEOF_MD2SHORT = 2;
static int const SIZEOF_MD2LONG = 4;
static int const SIZEOF_MD2FLOAT = 4;
static int const SIZEOF_MD2SKINNAME = 64;
static int const SIZEOF_MD2FRAMENAME = 16;
static int const SIZEOF_MD2HEADER = 15*SIZEOF_MD2LONG;

static const float SCALE_FACTOR = 0.025f;
static const float FRAME_DELAY = 0.1f;

typedef csPDelArray<csString> csStringVector;

struct csMD2Header
{
  // width and height of skin texture in pixels
  long SkinWidth, SkinHeight;
  // size of each frame int the sprite, in pixels
  long FrameSize;
  // number of skins, vertices, texels, triangles, glcmds(?), frames
  long SkinCount, VertexCount, TexelCount, TriangleCount, glcmds, FrameCount;
  // offset of the skin information in the file
  long SkinOffset;
  // offset of the texel information in the file
  long TexelOffset;
  // offset of the triangle information in the file
  long TriangleOffset;
  // offset of the frame information in the file
  long FramesOffset;
  // offset of the GL commands information in the file
  long GLCommandsOffset;
  // total file size
  long FileSize;
};

// ---------------------------------------------------------------------------

class csModelConverterMD2 : public iModelConverter
{
private:
  csModelConverterFormat FormatInfo;

public:
  SCF_DECLARE_IBASE;

  /// constructor
  csModelConverterMD2 (iBase *pBase);

  /// destructor
  virtual ~csModelConverterMD2 ();

  bool Initialize (iObjectRegistry *object_reg);
  virtual size_t GetFormatCount();
  virtual const csModelConverterFormat *GetFormat( size_t idx );
  virtual csPtr<iModelData> Load( uint8* Buffer, size_t size );
  virtual csPtr<iDataBuffer> Save( iModelData*, const char *format );

  struct Component : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csModelConverterMD2);
    virtual bool Initialize (iObjectRegistry *object_reg)
    {
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;
};

SCF_IMPLEMENT_IBASE (csModelConverterMD2)
  SCF_IMPLEMENTS_INTERFACE (iModelConverter)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csModelConverterMD2::Component)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csModelConverterMD2)


CS_IMPLEMENT_PLUGIN

csModelConverterMD2::csModelConverterMD2 (iBase *pBase)
{
  SCF_CONSTRUCT_IBASE (pBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);

  FormatInfo.Name = "md2";
  FormatInfo.CanLoad = true;
  FormatInfo.CanSave = false;
}

csModelConverterMD2::~csModelConverterMD2 ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csModelConverterMD2::Initialize (iObjectRegistry *)
{
  return true;
}

size_t csModelConverterMD2::GetFormatCount ()
{
  return 1;
}

const csModelConverterFormat *csModelConverterMD2::GetFormat (size_t idx)
{
  return (idx == 0) ? &FormatInfo : 0;
}

/*
  Purpose:

    csModelConverterMD2::Load() reads a Quake2 MD2 model file.

  Examples:

  Modified:

    13 July 1999 Gary Haussmann

  Author:

    John Burkardt

  Modified by Martin Geisse to work with the new converter system.
*/

static bool CheckMD2Version (csDataStream &in)
{
  // Read in header and check for a correct file.  The
  // header consists of two longs, containing
  // the magic identifiter 'IDP2' as the first long,
  // followed by a version number (8)
  uint32 FileID, FileVersion;
  in.ReadUInt32 (FileID);
  in.ReadUInt32 (FileVersion);
  FileID = csLittleEndianLong (FileID);
  FileVersion = csLittleEndianLong (FileVersion);

  if (FileID != ( ('2'<<24)+('P'<<16)+('D'<<8)+'I' ) )
    return false;

  if (FileVersion != 8)
    return false;

  return true;
}

#define CS_MD2_READ(num,name)						\
  hdr->name = csGetLittleEndianLong (buf + num * SIZEOF_MD2LONG);

static void ReadMD2Header (csMD2Header *hdr, csDataStream *in)
{
  char buf [SIZEOF_MD2HEADER];
  in->Read (buf, SIZEOF_MD2HEADER);

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
  unsigned i = 0;
  while (str[i] > '9')
    act[i++] = str[i];
  act[i] = 0;
}

csPtr<iModelData> csModelConverterMD2::Load (uint8 *Buffer, size_t Size)
{
  // prepare input buffer
  csDataStream in (Buffer, Size, false);
  uint8 readbuffer[MAX_DATAELEMENT_SIZE];
  int i,j;

  // check for the correct version
  if (!CheckMD2Version (in))
    return 0;

  // build the object framework
  iModelData *Scene = new csModelData ();
  iModelDataObject *Object = new csModelDataObject ();
  Scene->QueryObject ()->ObjAdd (Object->QueryObject ());

  // read MD2 header
  csMD2Header Header;
  ReadMD2Header (&Header, &in);

  // read in texmap (skin) names - skin names are 64 bytes long
  // Unused
  csStringVector SkinNames;
  in.SetPosition (Header.SkinOffset);
  for (i = 0; i < Header.SkinCount; i++)
  {
    char name[SIZEOF_MD2SKINNAME];
    in.Read (name, SIZEOF_MD2SKINNAME);
    csString *s = new csString (name);
    SkinNames.Push (s);
  }

  // read in skin data. This contains texture map coordinates for each
  // vertex; the spatial location of each vertex varies with each
  // frame, and is stored elsewhere, in the frame data section.
  // The only data we read here
  // are the static texture map (uv) locations for each vertex!
  csVector2 *Texels = new csVector2 [Header.TexelCount];
  in.SetPosition (Header.TexelOffset);
  for (i = 0; i < Header.TexelCount; i++)
  {
    in.Read (readbuffer, SIZEOF_MD2SHORT*2);
    Texels [i].Set (csGetLittleEndianShort(readbuffer)/(float)Header.SkinWidth,
                    csGetLittleEndianShort(readbuffer+2)/(float)Header.SkinHeight);
  }

  // next we read in the triangle connectivity data.  This data describes
  // each triangle as three indices, referring to three numbered vertices.
  // This data is, like the skin texture coords, independent of frame number.
  // There are actually two set of indices in the original quake file;
  // one indexes into the xyz coordinate table, the other indexes into
  // the uv texture coordinate table.
  in.SetPosition (Header.TriangleOffset);
  for (i = 0; i < Header.TriangleCount; i++)
  {
    iModelDataPolygon *Polygon = new csModelDataPolygon ();
    Object->QueryObject ()->ObjAdd (Polygon->QueryObject ());

    in.Read (readbuffer, SIZEOF_MD2SHORT*6);
    for (j = 0; j < 3; j++)
    {
      short xyzindex = csGetLittleEndianShort (readbuffer + j * SIZEOF_MD2SHORT);
      short texindex = csGetLittleEndianShort (readbuffer + (j+3) * SIZEOF_MD2SHORT);
      Polygon->AddVertex (xyzindex, 0, 0, texindex);
    }

    Polygon->DecRef ();
  }

  // now we read in the frames.  The number of frames is stored in 'num_object'

  iModelDataVertices *DefaultFrame = 0;

  char currAction [256];
  memset (currAction, 0, 256);
  csRef<iModelDataAction> action;
  float time = 0;

  for (i = 0; i < Header.FrameCount; i++, time += FRAME_DELAY)
  {
    csVector3 scale, translate;
    // read in scale and translate info
    in.Read (&scale, SIZEOF_MD2FLOAT*3);
    in.Read (&translate, SIZEOF_MD2FLOAT*3);
    for (j = 0; j < 3; j++)
    {
      scale[j] = csConvertEndian (scale[j]);
      translate[j] = csConvertEndian (translate[j]);
    }
    scale *= SCALE_FACTOR;
    translate *= SCALE_FACTOR;

    // name of this frame
    char FrameName [SIZEOF_MD2FRAMENAME+1];
    char ActionName [SIZEOF_MD2FRAMENAME+1];
    in.Read (FrameName, SIZEOF_MD2FRAMENAME);
    FrameName [SIZEOF_MD2FRAMENAME] = 0;
    extractActionName (FrameName, ActionName);
    if (strcmp (ActionName, currAction))
    {
      memcpy (currAction, ActionName, strlen (ActionName)+1);
      action = csPtr<iModelDataAction> (new csModelDataAction ());
      action->QueryObject ()->SetName (currAction);
      Object->QueryObject ()->ObjAdd (action->QueryObject ());
      time = FRAME_DELAY;
    }

    // read in vertex coordinate data for the frame
    in.Read (readbuffer, 4*Header.VertexCount);

    iModelDataVertices *VertexFrame = new csModelDataVertices ();
    action->AddFrame (time, VertexFrame->QueryObject ());
    VertexFrame->QueryObject ()->SetName (FrameName);
    if (!DefaultFrame)
       DefaultFrame = VertexFrame;

    for (j = 0; j < Header.TexelCount; j++)
    {
      VertexFrame->AddTexel (Texels [j]);
    }

    for (j = 0; j < Header.VertexCount; j++)
    {
      const uint8 * buf = readbuffer + j*4;
      csVector3 v (buf [0], buf [1], buf [2]);
      for (int k = 0; k < 3; k++)
        v[k] = v[k] * scale[k] + translate[k];
      // swap y and z
      float t = v.y;
      v.y = v.z;
      v.z = t;
      
      VertexFrame->AddVertex (v);
    }	
    VertexFrame->AddColor (csColor (1, 1, 1));
    VertexFrame->AddNormal (csVector3 (1, 0, 0));
    VertexFrame->DecRef ();
  }
  Object->SetDefaultVertices (DefaultFrame);
  Object->DecRef ();
  delete Texels;
  return csPtr<iModelData> (Scene);
}

csPtr<iDataBuffer> csModelConverterMD2::Save (iModelData *, const char *Format)
{
  if (strcasecmp (Format, "md2"))
    return 0;

  return 0;
}
