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
#include "cstool/mdltool.h"
#include "csutil/csstring.h"
#include "csutil/objiter.h"
#include "csutil/databuf.h"
#include "csgeom/math3d.h"

// A simple triangle, consisting of three vertex indices
struct csTriangle
{
  int a, b, c;

  csTriangle (int v1, int v2, int v3)
  { a=v1; b=v2; c=v3; }
};

/*
 * Extended triangle structure, consisting of three vertex, normal, color
 * and texel indices.
 */
struct csExtTriangle
{
  int va, vb, vc;
  int na, nb, nc;
  int ca, cb, cc;
  int ta, tb, tc;

  csExtTriangle (iModelDataPolygon *p, int a, int b, int c)
  {
    va = p->GetVertex (a);
    vb = p->GetVertex (b);
    vc = p->GetVertex (c);
    na = p->GetNormal (a);
    nb = p->GetNormal (b);
    nc = p->GetNormal (c);
    ca = p->GetColor (a);
    cb = p->GetColor (b);
    cc = p->GetColor (c);
    ta = p->GetTexel (a);
    tb = p->GetTexel (b);
    tc = p->GetTexel (c);
  }
};

class csModelConverterASE : iModelConverter
{
private:
  csModelConverterFormat FormatInfo;

public:
  SCF_DECLARE_IBASE;

  /// constructor
  csModelConverterASE (iBase *pBase);

  /// destructor
  virtual ~csModelConverterASE ();

  bool Initialize (iObjectRegistry *object_reg);
  virtual int GetFormatCount() const;
  virtual const csModelConverterFormat *GetFormat( int idx ) const;
  virtual iModelData *Load( UByte* Buffer, ULong size );
  virtual iDataBuffer *Save( iModelData*, const char *format );

  struct Component : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csModelConverterASE);
    virtual bool Initialize (iObjectRegistry *object_reg)
    { 
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;
};

SCF_IMPLEMENT_IBASE (csModelConverterASE)
  SCF_IMPLEMENTS_INTERFACE (iModelConverter)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csModelConverterASE::Component)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csModelConverterASE)

SCF_EXPORT_CLASS_TABLE (aseie)
  SCF_EXPORT_CLASS (csModelConverterASE, 
    "crystalspace.modelconverter.ase",
    "ASE Model Converter")
SCF_EXPORT_CLASS_TABLE_END

CS_IMPLEMENT_PLUGIN

SCF_DECLARE_FAST_INTERFACE (iModelDataPolygon);

CS_DECLARE_TYPED_VECTOR (csTriangleVector, csTriangle);
CS_DECLARE_TYPED_VECTOR (csExtTriangleVector, csExtTriangle);
CS_DECLARE_OBJECT_ITERATOR (csModelDataPolygonIterator, iModelDataPolygon);

csModelConverterASE::csModelConverterASE (iBase *pBase)
{
  SCF_CONSTRUCT_IBASE (pBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);

  FormatInfo.Name = "ase";
  FormatInfo.CanLoad = false;
  FormatInfo.CanSave = true;
}

csModelConverterASE::~csModelConverterASE ()
{
}

bool csModelConverterASE::Initialize (iObjectRegistry *objreg)
{
  return true;
}

int csModelConverterASE::GetFormatCount () const
{
  return 1;
}

const csModelConverterFormat *csModelConverterASE::GetFormat (int idx) const
{
  return (idx == 0) ? &FormatInfo : NULL;
}

iModelData *csModelConverterASE::Load (UByte * /*Buffer*/, ULong /*Size*/)
{
  return NULL;
}

iDataBuffer *csModelConverterASE::Save (iModelData *Data, const char *Format)
{
  if (strcasecmp (Format, "ase"))
    return NULL;

/*
  Purpose:

    csModelConverterASE::Save () writes graphics information to an AutoCAD ASE file.

  Modified:

    30 September 1998
	15 April 2001 - Added texture mapping (Luca Pancallo)

  Author:

    John Burkardt

*/

  int i, j;

  // only the first object is saved
  iModelDataObject *obj = CS_GET_CHILD_OBJECT (Data->QueryObject (), iModelDataObject);
  if (!obj) return NULL;
  csString out;
  iModelDataVertices *ver = obj->GetDefaultVertices ();

  // We need to create position/texel groups for vertices since they are stored
  // together in ASE files (i.e. vertex 'n' belongs to texel 'n', and the lists
  // must be of the same size. This is not true for vertex normals.
  csSingleIndexVertexSet VertexTexelSet (true, false, false, true);

  // build the triangle list and store the indices in VertexTexelSet. 'origtri'
  // stores a list of triangles whose indices point into the original vertex list,
  // those from 'tri' point into the vertex/texel set.
  csExtTriangleVector origtri;
  csTriangleVector tri;

  csModelDataPolygonIterator it (obj->QueryObject ());
  while (!it.IsFinished ()) {
    iModelDataPolygon *poly = it.Get ();
    int v1 = VertexTexelSet.Add (poly->GetVertex (0), -1, -1, poly->GetTexel (0));
    int vprev = VertexTexelSet.Add (poly->GetVertex (1), -1, -1, poly->GetTexel (1));

    for (int i=2; i<poly->GetVertexCount (); i++)
    {
      int vn = VertexTexelSet.Add (poly->GetVertex (i), -1, -1, poly->GetTexel (i));
      tri.Push (new csTriangle (v1, vprev, vn));
      origtri.Push (new csExtTriangle (poly, 0, i-1, i));
      vprev = vn;
    }
    it.Next ();
  }

  // Write the header.
  out << "*3DSMAX_ASCIIEXPORT 200\n";
  out << "*COMMENT \"Exported by Crystal Space.\"\n";

  // Write the scene block.
  out << "*SCENE {\n";
  out << "  *SCENE_FILENAME \"\"\n";
  out << "  *SCENE_FIRSTFRAME 0\n";
  out << "  *SCENE_LASTFRAME 100\n";
  out << "  *SCENE_FRAMESPEED 30\n";
  out << "  *SCENE_TICKSPERFRAME 160\n";
  out << "  *SCENE_BACKGROUND_STATIC 0.0000 0.0000 0.0000\n";
  out << "  *SCENE_AMBIENT_STATIC 0.0431 0.0431 0.0431\n";
  out << "}\n";

  // Begin the big geometry block.
  out << "*GEOMOBJECT {\n";
  out << "  *NODE_NAME \"" << obj->QueryObject ()->GetName () << "\"\n";

  // Sub block NODE_TM:
  out << "  *NODE_TM {\n";
  out << "    *NODE_NAME \"Object01\"\n";
  out << "    *INHERIT_POS 0 0 0\n";
  out << "    *INHERIT_ROT 0 0 0\n";
  out << "    *INHERIT_SCL 0 0 0\n";
  out << "    *TM_ROW0 1.0000 0.0000 0.0000\n";
  out << "    *TM_ROW1 0.0000 1.0000 0.0000\n";
  out << "    *TM_ROW2 0.0000 0.0000 1.0000\n";
  out << "    *TM_ROW3 0.0000 0.0000 0.0000\n";
  out << "    *TM_POS 0.0000 0.0000 0.0000\n";
  out << "    *TM_ROTAXIS 0.0000 0.0000 0.0000\n";
  out << "    *TM_ROTANGLE 0.0000\n";
  out << "    *TM_SCALE 1.0000 1.0000 1.0000\n";
  out << "    *TM_SCALEAXIS 0.0000 0.0000 0.0000\n";
  out << "    *TM_SCALEAXISANG 0.0000\n";
  out << "  }\n";

  // Sub block MESH:
  //   Items
  out << "  *MESH {\n";
  out << "    *TIMEVALUE 0\n";
  out << "    *MESH_NUMVERTEX " << VertexTexelSet.GetVertexCount () << "\n";
  out << "    *MESH_NUMFACES " << tri.Length () << "\n";

  // Sub sub block MESH_VERTEX_LIST
  out << "    *MESH_VERTEX_LIST {\n";
  for (i=0; i<VertexTexelSet.GetVertexCount(); i++)
  {
    csVector3 v = ver->GetVertex (VertexTexelSet.GetVertex (i));
    out << "      *MESH_VERTEX " << i << ' ' << v.x << ' ' << v.y << ' '
      << v.z << '\n';
  }
  out << "    }\n";

  // Sub sub block MESH_FACE_LIST
  //   Items MESH_FACE
  out << "    *MESH_FACE_LIST {\n";
  for (i=0; i<tri.Length (); i++)
  {
    csTriangle *t = tri.Get (i);
      out << "      *MESH_FACE " << i << ": A: " << t->a << " B: " << t->b
        << " C: " << t->c;
      out << " AB: 1 BC: 1 CA: 1 *MESH_SMOOTHING *MESH_MTLID 1\n";
  }
  out << "    }\n";

  // Item MESH_NUMTVERTEX.
  out << "    *MESH_NUMTVERTEX 0\n";

  // Item NUMCVERTEX.
  out << "    *MESH_NUMCVERTEX 0\n";

  // Sub block MESH_NORMALS
  //   Items MESH_FACENORMAL, MESH_VERTEXNORMAL (repeated)
  out << "    *MESH_NORMALS {\n";
  for (i=0; i<origtri.Length (); i++)
  {
    // write the face normal
    csExtTriangle *t = origtri.Get (i);
    csVector3 n;
    csMath3::CalcNormal (n, ver->GetVertex (t->va), ver->GetVertex (t->vb),
      ver->GetVertex (t->vc));
    n.Normalize ();

    out << "      *MESH_FACENORMAL " << i << ' ' << n.x << ' ' << n.y << ' '
      << n.z <<'\n';

    // write vertex normals
    for (j=0; j<3; j++)
    {
      int element = (j==0) ? (t->na) : ((j==1) ? (t->nb) : (t->nc));
      n = ver->GetNormal (element);

      out << "      *MESH_VERTEXNORMAL " << j << ' ' << n.x << ' ' << n.y
        << ' ' << n.z << '\n';
    }
  }
  out << "    }\n";

  // Sub block MESH_TVERTLIST
  //   Items MESH_TVERT (repeated)
  out << "    *MESH_TVERTLIST {\n";

  for (i=0; i<VertexTexelSet.GetVertexCount(); i++)
  {
    csVector2 v = ver->GetTexel (VertexTexelSet.GetTexel (i));
    out << "      *MESH_TVERT " << i << ' ' << v.x << ' ' << v.y << '\n';
  }

  // Close the MESH object.
  out << "  }\n";

  // A few closing parameters.
  out << "  *PROP_MOTIONBLUR 0\n";
  out << "  *PROP_CASTSHADOW 1\n";
  out << "  *PROP_RECVSHADOW 1\n";

  // Close the GEOM object.
  out << "}\n";

  int Size = out.Length ();
  return new csDataBuffer (out.Detach (), Size);
}
