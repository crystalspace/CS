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
#include "csutil/databuf.h"
#include "csutil/csstring.h"
#include "csutil/objiter.h"
#include <ctype.h>

class csModelConverterOBJ : public iModelConverter
{
private:
  csModelConverterFormat FormatInfo;

public:
  SCF_DECLARE_IBASE;

  /// constructor
  csModelConverterOBJ (iBase *pBase);

  /// destructor
  virtual ~csModelConverterOBJ ();

  bool Initialize (iObjectRegistry *object_reg);
  virtual int GetFormatCount();
  virtual const csModelConverterFormat *GetFormat(int idx);
  virtual csPtr<iModelData> Load(uint8* Buffer, uint32 size);
  virtual csPtr<iDataBuffer> Save(iModelData*, const char *format);

  struct Component : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csModelConverterOBJ);
    virtual bool Initialize (iObjectRegistry *object_reg)
    {
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;
};

SCF_IMPLEMENT_IBASE (csModelConverterOBJ)
  SCF_IMPLEMENTS_INTERFACE (iModelConverter)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csModelConverterOBJ::Component)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csModelConverterOBJ)

CS_IMPLEMENT_PLUGIN

csModelConverterOBJ::csModelConverterOBJ (iBase *pBase)
{
  SCF_CONSTRUCT_IBASE (pBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);

  FormatInfo.Name = "obj";
  FormatInfo.CanLoad = true;
  FormatInfo.CanSave = true;
}

csModelConverterOBJ::~csModelConverterOBJ ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csModelConverterOBJ::Initialize (iObjectRegistry *)
{
  return true;
}

int csModelConverterOBJ::GetFormatCount ()
{
  return 1;
}

const csModelConverterFormat *csModelConverterOBJ::GetFormat (int idx)
{
  return (idx == 0) ? &FormatInfo : 0;
}

csPtr<iModelData> csModelConverterOBJ::Load (uint8 *Buffer, uint32 Size)
{
  // prepare input buffer
  csDataStream in (Buffer, Size, false);
  char line [2048], token [64], *params;
  int tokenlen;

  // build the object framework
  iModelData *Scene = new csModelData ();
  iModelDataObject *Object = new csModelDataObject ();
  Scene->QueryObject ()->ObjAdd (Object->QueryObject ());
  iModelDataVertices *Vertices = new csModelDataVertices ();
  Object->SetDefaultVertices (Vertices);

  while (!in.Finished ())
  {
    // read a line
    in.GetString (line, 2048);

    // ignore blank lines and comments
    if (line[0] == 0 || isspace (line[0]) || line[0] == '$' || line[0] == '#')
      continue;

    // extract the keyword of this line
    sscanf (line, "%s%n", token, &tokenlen);
    params = line + tokenlen;
    while (isspace (*params)) params++;

/*
  BEVEL
  Bevel interpolation.
*/
    if (!strcasecmp (token, "BEVEL")) {
      continue;
    }
/*
  BMAT
  Basis matrix.
*/
    else if (!strcasecmp (token, "BMAT")) {
      continue;
    }
/*
  C_INTERP
  Color interpolation.
*/
    else if (!strcasecmp (token, "C_INTERP")) {
      continue;
    }
/*
  CON
  Connectivity between free form surfaces.
*/
    else if (!strcasecmp (token, "CON")) {
      continue;
    }
/*
  CSTYPE
  Curve or surface type.
*/
    else if (!strcasecmp (token, "CSTYPE")) {
      continue;
    }
/*
  CTECH
  Curve approximation technique.
*/
    else if (!strcasecmp (token, "CTECH")) {
      continue;
    }
/*
  CURV
  Curve.
*/
    else if (!strcasecmp (token, "CURV")) {
      continue;
    }
/*
  CURV2
  2D curve.
*/
    else if (!strcasecmp (token, "CURV2")) {
      continue;
    }
/*
  D_INTERP
  Dissolve interpolation.
*/
    else if (!strcasecmp (token, "D_INTERP")) {
      continue;
    }
/*
  DEG
  Degree.
*/
    else if (!strcasecmp (token, "DEG")) {
      continue;
    }
/*
  END
  End statement.
*/
    else if (!strcasecmp (token, "END")) {
      continue;
    }
/*
  F V1 V2 V3
    or
  F V1/VT1/VN1 V2/VT2/VN2 ...
    or
  F V1//VN1 V2//VN2 ...

  Face.
  A face is defined by the vertices.
  Optionally, slashes may be used to include the texture vertex
  and vertex normal indices.

  OBJ line node indices are 1 based rather than 0 based.
  So we have to decrement them before loading them into FACE.

  If negative indices are used, they are interpreted as moving
  backwards from the last stored vertex.
*/

    else if (!strcasecmp (token, "F"))
    {
      iModelDataPolygon *poly = new csModelDataPolygon ();
      csDataStream Params (params, strlen (params), false);

      while (!Params.Finished ())
      {
	char sep;
        int vidx = Params.ReadTextInt (), nidx = 0, tidx = 0;
        sep = Params.GetChar ();

	// look if extended information is given for this vertex
	if (sep == '/' && !isspace (Params.LookChar ())) {

	  // if the next character after the slash is not a slash
	  // then texel information is given
	  if (Params.LookChar () != '/')
	    tidx = Params.ReadTextInt ();

          // look if normal information is given
	  sep = Params.GetChar ();
	  if (sep == '/' && !isspace (Params.LookChar ())) {
            nidx = Params.ReadTextInt ();
	  }
	}

	// look for EOF or errors
	if (sep == (char)EOF)
	{
	  break;
	}
	else if (!isspace (sep))
	{
	  // @@@ this is an error!
	}

	Params.SkipWhitespace ();
	// @@@ better values should be used here
	if (!nidx) nidx = 1;
	if (!tidx) tidx = 1;

	// handle negative indices
	if (vidx < 0) vidx += Vertices->GetVertexCount () + 1;
	if (nidx < 0) nidx += Vertices->GetNormalCount () + 1;
	if (tidx < 0) tidx += Vertices->GetTexelCount () + 1;

	poly->AddVertex (vidx-1, nidx-1, 0, tidx-1);
      }

      Object->QueryObject ()->ObjAdd (poly->QueryObject ());
      poly->DecRef ();
    }

/*
  G
  Group name.
*/

    else if (!strcasecmp (token, "G")) {
      continue;
    }
/*
  HOLE
  Inconverternew trimming hole.
*/
    else if (!strcasecmp (token, "HOLE")) {
      continue;
    }
/*
  L
  I believe OBJ line node indices are 1 based rather than 0 based.
  So we have to decrement them before loading them into LINE_DEX.

  Note that lines are currently ignored since they cannot be stored
  in the model data structures.
*/

    else if (!strcasecmp (token, "L")) {
      continue;
/*
      for ( ;; ) {

        count = sscanf ( next, "%d%n", &node, &width );
        next = next + width;

        if ( count != 1 ) {
          break;
        }

        if ( num_line < MAX_LINE  ) {
          line_dex[num_line] = node-1;
          line_mat[num_line] = 0;
        }
        num_line = num_line + 1;

      }

      if ( num_line < MAX_LINE ) {
        line_dex[num_line] = -1;
        line_mat[num_line] = -1;
      }
      num_line = num_line + 1;
*/
    }

/*
  LOD
  Level of detail.
*/
    else if (!strcasecmp (token, "LOD")) {
      continue;
    }
/*
  MG
  Merging group.
*/
    else if (!strcasecmp (token, "MG")) {
      continue;
    }
/*
  MTLLIB
  Material library.
*/

    else if (!strcasecmp (token, "MTLLIB")) {
      continue;
    }
/*
  O
  Object name.
*/
    else if (!strcasecmp (token, "O")) {
      continue;
    }
/*
  P
  Point.
*/
    else if (!strcasecmp (token, "P")) {
      continue;
    }
/*
  PARM
  Parameter values.
*/
    else if (!strcasecmp (token, "PARM")) {
      continue;
    }
/*
  S
  Smoothing group
*/
    else if (!strcasecmp (token, "S")) {
      continue;
    }
/*
  SCRV
  Special curve.
*/
    else if (!strcasecmp (token, "SCRV")) {
      continue;
    }
/*
  SHADOW_OBJ
  Shadow casting.
*/
    else if (!strcasecmp (token, "SHADOW_OBJ")) {
      continue;
    }
/*
  SP
  Special point.
*/
    else if (!strcasecmp (token, "SP")) {
      continue;
    }
/*
  STECH
  Surface approximation technique.
*/
    else if (!strcasecmp (token, "STECH")) {
      continue;
    }
/*
  STEP
  Stepsize.
*/
    else if (!strcasecmp (token, "CURV")) {
      continue;
    }
/*
  SURF
  Surface.
*/
    else if (!strcasecmp (token, "SURF")) {
      continue;
    }
/*
  TRACE_OBJ
  Ray tracing.
*/
    else if (!strcasecmp (token, "TRACE_OBJ")) {
      continue;
    }
/*
  TRIM
  Outer trimming loop.
*/
    else if (!strcasecmp (token, "TRIM")) {
      continue;
    }
/*
  USEMTL
  Material name.
*/
    else if (!strcasecmp (token, "USEMTL")) {
      continue;
    }

/*
  V X Y Z W
  Geometric vertex.
  W is optional, a weight for rational curves and surfaces.
  The default for W is 1.
*/
    else if (!strcasecmp (token, "V")) {
      csVector3 v;
      sscanf (params, "%f %f %f", &v.x, &v.y, &v.z);
      Vertices->AddVertex (v);
    }

/*
  VN
  Vertex normals.
*/
    else if (!strcasecmp (token, "VN")) {
      csVector3 v;
      sscanf (params, "%f %f %f", &v.x, &v.y, &v.z);
      Vertices->AddNormal (v);
    }

/*
  VT
  Vertex texture.
*/
    else if (!strcasecmp (token, "VT")) {
      csVector2 v;
      sscanf (params, "%f %f", &v.x, &v.y);
      Vertices->AddTexel (v);
    }

/*
  VP
  Parameter space vertices.
*/
    else if (!strcasecmp (token, "VP")) {
      continue;
    }

    else {
      Object->DecRef ();
      Vertices->DecRef ();
      Scene->DecRef ();
      return 0;
    }
  }

  if (Vertices->GetNormalCount () == 0)
    Vertices->AddNormal (csVector3 (1, 0, 0));
  if (Vertices->GetTexelCount () == 0)
    Vertices->AddTexel (csVector2 (0, 0));
  Vertices->AddColor (csColor (1, 1, 1));

  Object->DecRef ();
  Vertices->DecRef ();
  return csPtr<iModelData> (Scene);
}

csPtr<iDataBuffer> csModelConverterOBJ::Save (iModelData *Data,
					      const char *Format)
{
  if (strcasecmp (Format, "obj"))
    return 0;

  // only the first object is saved
  csRef<iModelDataObject> obj (
  	CS_GET_CHILD_OBJECT (Data->QueryObject (), iModelDataObject));
  if (!obj) return 0;
  iModelDataVertices *ver = obj->GetDefaultVertices ();
  size_t i;

  csString out;
  out << "# Created by Crystal Space.\n\n";
  out << "g " << obj->QueryObject ()->GetName () << "\n\n";

  // store vertex coordinates
  for (i=0; i<ver->GetVertexCount (); i++)
  {
    csVector3 v = ver->GetVertex (i);
    out << "v " << v.x << ' ' << v.y << ' ' << v.z << '\n';
  }

  // store vertex normals
  for (i=0; i<ver->GetNormalCount (); i++)
  {
    csVector3 v = ver->GetNormal (i);
    out << "vn " << v.x << ' ' << v.y << ' ' << v.z << '\n';
  }

  // store texel coordinates
  for (i=0; i<ver->GetTexelCount (); i++)
  {
    csVector2 v = ver->GetTexel (i);
    out << "vt " << v.x << ' ' << v.y << '\n';
  }

  // store polygons
  csTypedObjectIterator<iModelDataPolygon> it (obj->QueryObject ());
  while (it.HasNext ())
  {
    iModelDataPolygon *poly = it.Next ();
    out << "f";

    for (i=0; i<poly->GetVertexCount (); i++)
      out << ' ' << (poly->GetVertex (i)+1) << '/' << (poly->GetTexel (i)+1) <<
        '/' << (poly->GetNormal (i)+1);

    out << '\n';
  }

  int Size = out.Length ();
  return csPtr<iDataBuffer> (new csDataBuffer (out.Detach (), Size));
}
