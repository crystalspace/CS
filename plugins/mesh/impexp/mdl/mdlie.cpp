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
#include "isys/plugin.h"
#include "iutil/objreg.h"
#include "imesh/mdlconv.h"
#include "cstool/mdldata.h"
#include "csutil/datastrm.h"
#include "csutil/csstring.h"
#include "csutil/nobjvec.h"

/*
// all int's in an MDL file are little endian
#include "cssys/csendian.h"

// upper bound onsize of biggest data element (vertex, polygon) in an MDL file
static int const MAX_DATAELEMENT_SIZE = 8192;

// size of various MDL elements
static int const SIZEOF_MDLSHORT = 2;
static int const SIZEOF_MDLLONG = 4;
static int const SIZEOF_MDLFLOAT = 4;
static int const SIZEOF_MDLSKINNAME = 64;
static int const SIZEOF_MDLFRAMENAME = 16;

CS_DECLARE_TYPED_VECTOR (csStringVector, csString);
CS_DECLARE_OBJECT_VECTOR (csVertexFrameVector, iModelDataVertices);

struct csMDLHeader
{
  // translation and scale
  csVector3 Scale, Origin;
  // radius of the bounding sphere
  float BoundingSphereRadius;
  // position of the player camera for this model
  csVector3 CameraPosition;
  // number of skins, width and height of the skin texture
  long SkinCount, SkinWidth, SkinHeight;
  // number of vertices, triangles and frames
  long VertexCount, TriangleCount, FrameCount;
  // ???
  long SyncType, Flags;
  // average triangle size (?)
  float AverageTriangleSize;
};
*/

// ---------------------------------------------------------------------------

class csModelConverterMDL : iModelConverter
{
private:
  csModelConverterFormat FormatInfo;

public:
  SCF_DECLARE_IBASE;

  /// constructor
  csModelConverterMDL (iBase *pBase);

  /// destructor
  virtual ~csModelConverterMDL ();

  bool Initialize (iObjectRegistry *object_reg);
  virtual int GetFormatCount() const;
  virtual const csModelConverterFormat *GetFormat( int idx ) const;
  virtual iModelData *Load( UByte* Buffer, ULong size );
  virtual iDataBuffer *Save( iModelData*, const char *format );

  struct Plugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE (csModelConverterMDL);
    virtual bool Initialize (iObjectRegistry *object_reg)
    { 
      return scfParent->Initialize (object_reg);
    }
  } scfiPlugin;
};

SCF_IMPLEMENT_IBASE (csModelConverterMDL)
  SCF_IMPLEMENTS_INTERFACE (iModelConverter)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csModelConverterMDL::Plugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csModelConverterMDL)

SCF_EXPORT_CLASS_TABLE (mdlie)
  SCF_EXPORT_CLASS (csModelConverterMDL, 
    "crystalspace.modelconverter.mdl",
    "MDL Model Converter")
SCF_EXPORT_CLASS_TABLE_END

CS_IMPLEMENT_PLUGIN

csModelConverterMDL::csModelConverterMDL (iBase *pBase)
{
  SCF_CONSTRUCT_IBASE (pBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPlugin);

  FormatInfo.Name = "mdl";
  FormatInfo.CanLoad = true;
  FormatInfo.CanSave = false;
}

csModelConverterMDL::~csModelConverterMDL ()
{
}

bool csModelConverterMDL::Initialize (iObjectRegistry *objreg)
{
  return true;
}

int csModelConverterMDL::GetFormatCount () const
{
  return 1;
}

const csModelConverterFormat *csModelConverterMDL::GetFormat (int idx) const
{
  return (idx == 0) ? &FormatInfo : NULL;
}

/*
  Purpose:
   
    csModelConverterMDL::Load() reads a Quake2 MDL model file.

  Examples:

  Modified:

    13 July 1999 Gary Haussmann

  Author:
 
    John Burkardt
  
  Modified by Martin Geisse to work with the new converter system.
*/

/*
static bool CheckMDLVersion (csDataStream &in)
{
  // Read in header and check for a correct file.  The
  // header consists of two longs, containing
  // the magic identifiter 'IDP2' as the first long,
  // followed by a version number (8)
  uint32 FileID, FileVersion;
  in.ReadUInt32 (FileID);
  in.ReadUInt32 (FileVersion);
  FileID = little_endian_long (FileID);
  FileVersion = little_endian_long (FileVersion);

  if (FileID != ( ('O'<<24)+('P'<<16)+('D'<<8)+'I' ) )
    return false;

  if (FileVersion != 8)
    return false;

  return true;
}

#define CS_MDL_READ_LONG(name)						\
  hdr->name = get_le_long (buf + Position);				\
  Position += SIZEOF_MDL_LONG;

#define CS_MDL_READ_FLOAT(name)						\
  hrd->name = convert_endian(*((float*)(buf + Position)));		\
  Position += SIZEOF_MDL_FLOAT;

static void ReadMDLHeader (csMDLHeader *hdr, csDataStream *in)
{
  char buf [SIZEOF_MD2_HEADER];
  in.Read (buf, SIZEOF_MD2_HEADER);
  int Position = 0;

  @@@
}
#undef CS_MDL_READ_LONG
#undef CS_MDL_READ_FLOAT
*/

iModelData *csModelConverterMDL::Load (UByte *Buffer, ULong Size)
{
  return NULL;
/*
  // prepare input buffer
  csDataStream in (Buffer, Size, false);
  unsigned char readbuffer[MAX_DATAELEMENT_SIZE];
  int i,j;

  // check for the correct version
  if (!CheckMDLVersion (in))
    return NULL;

  // build the object framework
  iModelData *Scene = new csModelData ();
  iModelDataObject *Object = new csModelDataObject ();
  Scene->QueryObject ()->ObjAdd (Object->QueryObject ());

  // read MDL header
  csMDLHeader Header;
  ReadMDLHeader (&Header, &in);

  // read in texmap (skin) names - skin names are 64 bytes long
  csStringVector SkinNames;
  in.SetPosition (Header.SkinOffset);
  for (i = 0; i < Header.SkinCount; i++)
  {
    csString *s = new csString (SIZEOF_MDLSKINNAME);
    in.Read (s->GetData (), SIZEOF_MDLSKINNAME);
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
    in.Read (readbuffer, SIZEOF_MDLSHORT*2);
    Texels [i].Set (get_le_short(readbuffer)/(float)Header.SkinWidth,
                    get_le_short(readbuffer+2)/(float)Header.SkinHeight);
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

    in.Read (readbuffer, SIZEOF_MDLSHORT*6);
    for (j = 2; j>=0; j--)
    {
      short xyzindex = get_le_short(readbuffer + j * SIZEOF_MDLSHORT);
      short texindex = get_le_short(readbuffer + (j+3) * SIZEOF_MDLSHORT);
      Polygon->AddVertex (xyzindex, 0, 0, texindex);
    }

    Polygon->DecRef ();
  }

  // now we read in the frames.  The number of frames is stored in 'num_object'
  float scale[3],translate[3];
  csVertexFrameVector Frames;
  iModelDataVertices *DefaultFrame = NULL;

  for (i = 0; i < Header.FrameCount; i++)
  {
    // read in scale and translate info
    in.Read (scale, SIZEOF_MDLFLOAT*3);
    in.Read (translate, SIZEOF_MDLFLOAT*3);
    for (j = 0; j<3; j++)
    {
      scale[j] = convert_endian(scale[j]);
      translate[j] = convert_endian(translate[j]);
    }

    // name of this frame
    char FrameName [SIZEOF_MDLFRAMENAME];
    in.Read (FrameName, SIZEOF_MDLFRAMENAME);

    // read in vertex coordinate data for the frame
    float *raw_vertexcoords = new float[3*Header.VertexCount];
    in.Read (readbuffer, 4*Header.VertexCount);

    iModelDataVertices *VertexFrame = new csModelDataVertices ();
    Frames.Push (VertexFrame);
    if (!DefaultFrame)
       DefaultFrame = VertexFrame;

    for (j = 0; j < Header.TexelCount; j++)
    {
      VertexFrame->AddTexel (Texels [j]);
    }

    for (j = 0; j < Header.VertexCount; j++)
    {
      csVector3 Vertex (readbuffer[j*4] * scale[0] + translate[0],
                        readbuffer[j*4+1] * scale[1] + translate[1],
			readbuffer[j*4+2] * scale[2] + translate[2]);
      VertexFrame->AddVertex (Vertex);
    }
    
    VertexFrame->AddColor (csColor (1, 1, 1));
    VertexFrame->AddNormal (csVector3 (1, 0, 0));
    VertexFrame->DecRef ();
  }

  Object->SetDefaultVertices (DefaultFrame);  
  Object->DecRef ();
  return Scene;
*/
}

iDataBuffer *csModelConverterMDL::Save (iModelData *Data, const char *Format)
{
  if (strcasecmp (Format, "mdl"))
    return NULL;

  return NULL;
}
