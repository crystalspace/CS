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
#include <ctype.h>
#include "csutil/parray.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "imesh/mdlconv.h"
#include "cstool/mdldata.h"
#include "cstool/mdltool.h"
#include "csutil/csstring.h"
#include "csutil/objiter.h"
#include "csutil/databuf.h"
#include "csutil/datastrm.h"
#include "csutil/nobjvec.h"
#include "csgeom/math3d.h"
#include "csgeom/transfrm.h"
#include "csgeom/tri.h"

typedef bool (csASEInterpreter) (class csModelConverterASE *conv,
				 csDataStream &in, const char *Token);

class csModelConverterASE : public iModelConverter
{
private:
  csModelConverterFormat FormatInfo;

public:
  SCF_DECLARE_IBASE;

  // current interpreter
  csASEInterpreter *interp;
  // current scene
  iModelData *Scene;
  // current object
  iModelDataObject *Object;
  // vertices of current object
  iModelDataVertices *Vertices;
  // list of polygons for current object
  csRefArrayObject<iModelDataPolygon> Polygons;
  // current polygon (for "normals" section; not referenced)
  iModelDataPolygon *CurrentPolygon;
  // current vertex (for "normals" section)
  int CurrentVertex;
  // object transformation
  csVector3 TransformRow1;
  csVector3 TransformRow2;
  csVector3 TransformRow3;
  csVector3 TransformOrigin;

  /// constructor
  csModelConverterASE (iBase *pBase);

  /// destructor
  virtual ~csModelConverterASE ();

  bool Initialize (iObjectRegistry *object_reg);
  virtual size_t GetFormatCount();
  virtual const csModelConverterFormat *GetFormat( size_t idx );
  virtual csPtr<iModelData> Load( uint8* Buffer, size_t size );
  virtual csPtr<iDataBuffer> Save( iModelData*, const char *format );

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


CS_IMPLEMENT_PLUGIN

csModelConverterASE::csModelConverterASE (iBase *pBase)
{
  SCF_CONSTRUCT_IBASE (pBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);

  FormatInfo.Name = "ase";
  FormatInfo.CanLoad = true;
  FormatInfo.CanSave = true;
}

csModelConverterASE::~csModelConverterASE ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csModelConverterASE::Initialize (iObjectRegistry *)
{
  return true;
}

size_t csModelConverterASE::GetFormatCount ()
{
  return 1;
}

const csModelConverterFormat *csModelConverterASE::GetFormat (size_t idx)
{
  return (idx == 0) ? &FormatInfo : 0;
}

// Load a single word from the stream
static bool csASEReadWord (csDataStream &str, char *buf, int max)
{
  int count = 0;
  bool quoted = false;
  str.SkipWhitespace ();

  if (str.Finished ()) return false;

  // look if this word is enclosed in double quotes
  if (str.LookChar () == '"') {
    str.GetChar ();
    quoted = true;
  }

  while (count < max-1) {
    int c = str.GetChar ();
    bool Finished = quoted ? (c == '"') : (isspace (c));
    if (c == EOF || Finished) break;
    buf [count] = c;
    count++;
  }

  buf [count] = 0;
  return true;
}

#define CS_ASE_CHECK_TOKEN(x)						\
  (strcmp (Token, x) == 0)

#define CS_ASE_READ_IGNORE(x)						\
  if (CS_ASE_CHECK_TOKEN (x)) return true;

#define CS_ASE_ENTER_SUBSECTION(intp)					\
  {									\
    char buf [256];							\
    if (!csASEReadWord (in, buf, 256)) return false;			\
    if (strcmp (buf, "{")) return false;				\
    conv->interp = &intp;						\
  }

#define CS_ASE_LEAVE_SUBSECTION(intp)					\
  conv->interp = &intp;

#define CS_ASE_SUBSECTION(token,intp)					\
  if (CS_ASE_CHECK_TOKEN (token)) {					\
    CS_ASE_ENTER_SUBSECTION (intp);					\
    return true;							\
  }

#define CS_ASE_OUTER_SECTION(intp)					\
  if (CS_ASE_CHECK_TOKEN ("}")) {					\
    CS_ASE_LEAVE_SUBSECTION (intp);					\
    return true;							\
  }

csASEInterpreter csASEInterpreter_MAIN;
csASEInterpreter csASEInterpreter_SCENE;
csASEInterpreter csASEInterpreter_GEOMOBJECT;
csASEInterpreter csASEInterpreter_MESH;
csASEInterpreter csASEInterpreter_NODE_TM;
csASEInterpreter csASEInterpreter_MESH_VERTEX_LIST;
csASEInterpreter csASEInterpreter_MESH_FACE_LIST;
csASEInterpreter csASEInterpreter_MESH_NORMALS;
csASEInterpreter csASEInterpreter_MESH_CFACELIST;
csASEInterpreter csASEInterpreter_MESH_CVERTLIST;
csASEInterpreter csASEInterpreter_MESH_TFACELIST;
csASEInterpreter csASEInterpreter_MESH_TVERTLIST;

bool csASEInterpreter_MAIN (csModelConverterASE *conv, csDataStream &in,
  const char *Token)
{
  CS_ASE_READ_IGNORE ("*3DSMAX_ASCIIEXPORT");
  CS_ASE_READ_IGNORE ("*COMMENT");
  CS_ASE_SUBSECTION ("*SCENE", csASEInterpreter_SCENE);

  if (CS_ASE_CHECK_TOKEN ("*GEOMOBJECT")) {
    conv->Object = new csModelDataObject ();
    conv->Scene->QueryObject ()->ObjAdd (conv->Object->QueryObject ());
    conv->Vertices = new csModelDataVertices ();
    conv->Object->SetDefaultVertices (conv->Vertices);
    conv->TransformRow1 = csVector3 (0);
    conv->TransformRow2 = csVector3 (0);
    conv->TransformRow3 = csVector3 (0);
    conv->TransformOrigin = csVector3 (0);

    CS_ASE_ENTER_SUBSECTION (csASEInterpreter_GEOMOBJECT);
    return true;
  }

  return false;
}

bool csASEInterpreter_SCENE (csModelConverterASE *conv, csDataStream &/*in*/,
  const char *Token)
{
  CS_ASE_READ_IGNORE ("*SCENE_AMBIENT_STATIC");
  CS_ASE_READ_IGNORE ("*SCENE_BACKGROUND_STATIC");
  CS_ASE_READ_IGNORE ("*SCENE_FILENAME");
  CS_ASE_READ_IGNORE ("*SCENE_FIRSTFRAME");
  CS_ASE_READ_IGNORE ("*SCENE_FRAMESPEED");
  CS_ASE_READ_IGNORE ("*SCENE_LASTFRAME");
  CS_ASE_READ_IGNORE ("*SCENE_TICKSPERFRAME");

  CS_ASE_OUTER_SECTION (csASEInterpreter_MAIN);
  return false;
}

bool csASEInterpreter_GEOMOBJECT (csModelConverterASE *conv, csDataStream &in,
  const char *Token)
{
  CS_ASE_READ_IGNORE ("*NODE_NAME");
  CS_ASE_READ_IGNORE ("*PROP_CASTSHADOW");
  CS_ASE_READ_IGNORE ("*PROP_MOTIONBLUR");
  CS_ASE_READ_IGNORE ("*PROP_RECVSHADOW");
  CS_ASE_SUBSECTION ("*NODE_TM", csASEInterpreter_NODE_TM);
  CS_ASE_SUBSECTION ("*MESH", csASEInterpreter_MESH);

  if (CS_ASE_CHECK_TOKEN ("}")) {
    // @@@ transform object

    // fill missing colors or texels
    size_t vc = conv->Vertices->GetVertexCount ();
    while (conv->Vertices->GetColorCount () < vc)
      conv->Vertices->AddColor (csColor (1, 1, 1));
    while (conv->Vertices->GetTexelCount () < vc)
      conv->Vertices->AddTexel (csVector2 (0, 0));

    conv->Object->DecRef ();
    conv->Object = 0;
    conv->Vertices->DecRef ();
    conv->Vertices = 0;
    conv->Polygons.DeleteAll ();

    CS_ASE_LEAVE_SUBSECTION (csASEInterpreter_MAIN);
    return true;
  }

  return false;
}

bool csASEInterpreter_MESH (csModelConverterASE *conv, csDataStream &in,
  const char *Token)
{
  CS_ASE_SUBSECTION ("*MESH_CFACELIST", csASEInterpreter_MESH_CFACELIST);
  CS_ASE_SUBSECTION ("*MESH_CVERTLIST", csASEInterpreter_MESH_CVERTLIST);
  CS_ASE_SUBSECTION ("*MESH_FACE_LIST", csASEInterpreter_MESH_FACE_LIST);
  CS_ASE_SUBSECTION ("*MESH_NORMALS", csASEInterpreter_MESH_NORMALS);
  CS_ASE_SUBSECTION ("*MESH_TFACELIST", csASEInterpreter_MESH_TFACELIST);
  CS_ASE_SUBSECTION ("*MESH_TVERTLIST", csASEInterpreter_MESH_TVERTLIST);
  CS_ASE_SUBSECTION ("*MESH_VERTEX_LIST", csASEInterpreter_MESH_VERTEX_LIST);

  CS_ASE_READ_IGNORE ("*TIMEVALUE");
  CS_ASE_READ_IGNORE ("*MESH_NUMCVERTEX");
  CS_ASE_READ_IGNORE ("*MESH_NUMCVFACES");
  CS_ASE_READ_IGNORE ("*MESH_NUMFACES");
  CS_ASE_READ_IGNORE ("*MESH_NUMTVERTEX");
  CS_ASE_READ_IGNORE ("*MESH_NUMTVFACES");
  CS_ASE_READ_IGNORE ("*MESH_NUMVERTEX");

  CS_ASE_OUTER_SECTION (csASEInterpreter_GEOMOBJECT);
  return false;
}

bool csASEInterpreter_NODE_TM (csModelConverterASE *conv, csDataStream &in,
  const char *Token)
{
  CS_ASE_READ_IGNORE ("*INHERIT_POS");
  CS_ASE_READ_IGNORE ("*INHERIT_ROT");
  CS_ASE_READ_IGNORE ("*INHERIT_SCL");
  CS_ASE_READ_IGNORE ("*NODE_NAME");
  CS_ASE_READ_IGNORE ("*TM_POS");
  CS_ASE_READ_IGNORE ("*TM_ROTANGLE");
  CS_ASE_READ_IGNORE ("*TM_ROTAXIS");
  CS_ASE_READ_IGNORE ("*TM_SCALE");
  CS_ASE_READ_IGNORE ("*TM_SCALEAXIS");
  CS_ASE_READ_IGNORE ("*TM_SCALEAXISANG");

  if (CS_ASE_CHECK_TOKEN ("*TM_ROW0")) {
    conv->TransformRow1.x = in.ReadTextFloat ();
    conv->TransformRow1.y = in.ReadTextFloat ();
    conv->TransformRow1.z = in.ReadTextFloat ();
    return true;
  }
  if (CS_ASE_CHECK_TOKEN ("*TM_ROW1")) {
    conv->TransformRow2.x = in.ReadTextFloat ();
    conv->TransformRow2.y = in.ReadTextFloat ();
    conv->TransformRow2.z = in.ReadTextFloat ();
    return true;
  }
  if (CS_ASE_CHECK_TOKEN ("*TM_ROW2")) {
    conv->TransformRow3.x = in.ReadTextFloat ();
    conv->TransformRow3.y = in.ReadTextFloat ();
    conv->TransformRow3.z = in.ReadTextFloat ();
    return true;
  }
  if (CS_ASE_CHECK_TOKEN ("*TM_ROW3")) {
    conv->TransformOrigin.x = in.ReadTextFloat ();
    conv->TransformOrigin.y = in.ReadTextFloat ();
    conv->TransformOrigin.z = in.ReadTextFloat ();
    return true;
  }

  CS_ASE_OUTER_SECTION (csASEInterpreter_GEOMOBJECT);
  return false;
}

bool csASEInterpreter_MESH_VERTEX_LIST (csModelConverterASE *conv,
					csDataStream &in, const char *Token)
{
  if (CS_ASE_CHECK_TOKEN ("*MESH_VERTEX"))
  {
    // @@@ nbase?
    int n = in.ReadTextInt (); (void)n;
    float x = in.ReadTextFloat ();
    float y = in.ReadTextFloat ();
    float z = in.ReadTextFloat ();
    conv->Vertices->AddVertex (csVector3 (x, y, z));
    return true;
  }

  CS_ASE_OUTER_SECTION (csASEInterpreter_MESH);
  return false;
}

bool csASEInterpreter_MESH_FACE_LIST (csModelConverterASE *conv,
				      csDataStream &in, const char *Token)
{
  if (CS_ASE_CHECK_TOKEN ("*MESH_FACE"))
  {
    char Token2 [256];
    int a = -1, b = -1, c = -1, d = -1;

    // first word is the face number, which is unimportant to us
    csASEReadWord (in, Token2, 256);

    // loop through the remaining words
    while (!in.Finished ()) {
      csASEReadWord (in, Token2, 256);
      int Param = in.ReadTextInt ();

      if (!strcmp (Token2, "A:")) a = Param;
      if (!strcmp (Token2, "B:")) b = Param;
      if (!strcmp (Token2, "C:")) c = Param;
      if (!strcmp (Token2, "D:")) d = Param;
      if (!strcmp (Token2, "*MESH_MTLID")) {
        // is this the material for the face?
      }
    }

    iModelDataPolygon *poly = new csModelDataPolygon ();
    if (a == -1 || b == -1 || c == -1) return false;
    poly->AddVertex (a, 0, a, a);
    poly->AddVertex (b, 0, b, b);
    poly->AddVertex (c, 0, c, c);
    if (d != -1) poly->AddVertex (d, 0, d, d);

    conv->Object->QueryObject ()->ObjAdd (poly->QueryObject ());
    conv->Polygons.Push (poly);
    poly->DecRef ();
    return true;
  }

  CS_ASE_OUTER_SECTION (csASEInterpreter_MESH);
  return false;
}

bool csASEInterpreter_MESH_NORMALS (csModelConverterASE *conv,
				    csDataStream &in, const char *Token)
{
  if (CS_ASE_CHECK_TOKEN ("*MESH_FACENORMAL"))
  {
    int const n = in.ReadTextInt ();
    if (n < 0 || n >= conv->Polygons.Length ()) return false;
    conv->CurrentPolygon = conv->Polygons.Get ((size_t)n);
    conv->CurrentVertex = 0;
    return true;
  }

  if (CS_ASE_CHECK_TOKEN ("*MESH_VERTEXNORMAL"))
  {
    int n = in.ReadTextInt ();
    float x = in.ReadTextFloat ();
    float y = in.ReadTextFloat ();
    float z = in.ReadTextFloat ();
    csVector3 v (x, y, z);

    n = (int)conv->Vertices->FindNormal (v);
    if (n == -1) n = (int)conv->Vertices->AddNormal (v);
    conv->CurrentPolygon->SetNormal (conv->CurrentVertex, n);
    conv->CurrentVertex++;

    return true;
  }

  CS_ASE_OUTER_SECTION (csASEInterpreter_MESH);
  return false;
}

bool csASEInterpreter_MESH_CFACELIST (csModelConverterASE *conv,
  csDataStream &/*in*/, const char *Token)
{
  CS_ASE_READ_IGNORE ("*MESH_CFACE");

  CS_ASE_OUTER_SECTION (csASEInterpreter_MESH);
  return false;
}

bool csASEInterpreter_MESH_CVERTLIST (csModelConverterASE *conv,
				      csDataStream &in, const char *Token)
{
  if (CS_ASE_CHECK_TOKEN ("*MESH_VERTCOL"))
  {
    // @@@ nbase?
    int n = in.ReadTextInt (); (void)n;
    float r = in.ReadTextFloat ();
    float g = in.ReadTextFloat ();
    float b = in.ReadTextFloat ();
    conv->Vertices->AddColor (csColor (r, g, b));
    return true;
  }

  CS_ASE_OUTER_SECTION (csASEInterpreter_MESH);
  return false;
}

bool csASEInterpreter_MESH_TFACELIST (csModelConverterASE *conv,
				      csDataStream &/*in*/, const char *Token)
{
  CS_ASE_READ_IGNORE ("*MESH_TFACE");

  CS_ASE_OUTER_SECTION (csASEInterpreter_MESH);
  return false;
}

bool csASEInterpreter_MESH_TVERTLIST (csModelConverterASE *conv,
				      csDataStream &in, const char *Token)
{
  if (CS_ASE_CHECK_TOKEN ("*MESH_TVERT"))
  {
    // @@@ nbase?
    int n = in.ReadTextInt (); (void)n;
    float u = in.ReadTextFloat ();
    float v = in.ReadTextFloat ();
    conv->Vertices->AddTexel (csVector2 (u, v));
    return true;
  }

  CS_ASE_OUTER_SECTION (csASEInterpreter_MESH);
  return false;
}

csPtr<iModelData> csModelConverterASE::Load (uint8 *Buffer, size_t Size)
{
  csDataStream in (Buffer, Size, false);
  interp = &csASEInterpreter_MAIN;
  Scene = new csModelData ();
  Object = 0;
  Vertices = 0;
  CurrentPolygon = 0;

  while (!in.Finished ())
  {
    // read a line of text
    char line [2048];
    int linelen = 0;
    while (linelen < 2047) {
      int c = in.GetChar ();
      if (c == EOF || c == '\n' || c == '\r') break;
      line [linelen] = c;
      linelen++;
    }
    line [linelen] = 0;

    // create a separate stream for the line
    csDataStream Line (line, linelen, false);

    // read the first word
    char Token [256];
    if (!csASEReadWord (Line, Token, 256)) continue;

    // give the line to the current interpreter
    if (!(*interp) (this, Line, Token))
    {
      if (Scene) Scene->DecRef ();
      Scene = 0;
      if (Object) Object->DecRef ();
      Object = 0;
      if (Vertices) Vertices->DecRef ();
      Vertices = 0;
      Polygons.DeleteAll ();
      return 0;
    }
  }

  return csPtr<iModelData> (Scene);
}

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

typedef csPDelArray<csTriangle> csTriangleVector;
typedef csPDelArray<csExtTriangle> csExtTriangleVector;

csPtr<iDataBuffer> csModelConverterASE::Save (iModelData *Data,
					      const char *Format)
{
  if (strcasecmp (Format, "ase"))
    return 0;

/*
  Purpose:

  csModelConverterASE::Save() writes graphics information to an AutoCAD ASE
  file.

  Modified:

    30 September 1998
	15 April 2001 - Added texture mapping (Luca Pancallo)

  Author:

    John Burkardt

*/

  size_t i, j;

  // only the first object is saved
  csRef<iModelDataObject> obj (
  	CS_GET_CHILD_OBJECT (Data->QueryObject (), iModelDataObject));
  if (!obj) return 0;
  csString out;
  iModelDataVertices *ver = obj->GetDefaultVertices ();

  // We need to create position/texel groups for vertices since they are stored
  // together in ASE files (i.e. vertex 'n' belongs to texel 'n', and the lists
  // must be of the same size. This is not true for vertex normals.
  csSingleIndexVertexSet VertexTexelSet (true, false, false, true);

  // Build the triangle list and store the indices in VertexTexelSet. 'origtri'
  // stores a list of triangles whose indices point into the original vertex
  // list, those from 'tri' point into the vertex/texel set.
  csExtTriangleVector origtri;
  csTriangleVector tri;

  csTypedObjectIterator<iModelDataPolygon> it (obj->QueryObject ());
  while (it.HasNext ())
  {
    iModelDataPolygon *poly = it.Next ();
    int v1 = (int)VertexTexelSet.Add(poly->GetVertex(0), -1, -1, poly->GetTexel(0));
    int vprev =
      (int)VertexTexelSet.Add(poly->GetVertex(1), -1, -1, poly->GetTexel(1));

    size_t i;
    for (i=2; i<poly->GetVertexCount (); i++)
    {
      int vn =
	(int)VertexTexelSet.Add(poly->GetVertex(i), -1, -1, poly->GetTexel(i));
      tri.Push (new csTriangle (v1, vprev, vn));
      origtri.Push (new csExtTriangle (poly, 0, (int)i-1, (int)i));
      vprev = vn;
    }
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
  out << "    *MESH_NUMVERTEX " << (uint)VertexTexelSet.GetVertexCount () << "\n";
  out << "    *MESH_NUMFACES " << (uint)tri.Length () << "\n";

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

      element = (j==0) ? (t->va) : ((j==1) ? (t->vb) : (t->vc));
      out << "      *MESH_VERTEXNORMAL " << element << ' ' << n.x << ' ' << n.y
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

  int Size = (int)out.Length ();
  return csPtr<iDataBuffer> (new csDataBuffer (out.Detach (), Size));
}
