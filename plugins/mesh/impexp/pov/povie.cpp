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
#include "csutil/csstring.h"
#include "csutil/databuf.h"
#include "csutil/objiter.h"

class csModelConverterPOV : public iModelConverter
{
private:
  csModelConverterFormat FormatInfo;

public:
  SCF_DECLARE_IBASE;

  /// constructor
  csModelConverterPOV (iBase *pBase);

  /// destructor
  virtual ~csModelConverterPOV ();

  bool Initialize (iObjectRegistry *object_reg);
  virtual int GetFormatCount();
  virtual const csModelConverterFormat *GetFormat(int idx);
  virtual csPtr<iModelData> Load(uint8* Buffer, uint32 size);
  virtual csPtr<iDataBuffer> Save(iModelData*, const char *format);

  struct Component : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csModelConverterPOV);
    virtual bool Initialize (iObjectRegistry *object_reg)
    {
      return scfParent->Initialize (object_reg);
    }
  } scfiComponent;
};

SCF_IMPLEMENT_IBASE (csModelConverterPOV)
  SCF_IMPLEMENTS_INTERFACE (iModelConverter)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csModelConverterPOV::Component)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csModelConverterPOV)


CS_IMPLEMENT_PLUGIN

csModelConverterPOV::csModelConverterPOV (iBase *pBase)
{
  SCF_CONSTRUCT_IBASE (pBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);

  FormatInfo.Name = "pov";
  FormatInfo.CanLoad = false;
  FormatInfo.CanSave = true;
}

csModelConverterPOV::~csModelConverterPOV ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csModelConverterPOV::Initialize (iObjectRegistry *)
{
  return true;
}

int csModelConverterPOV::GetFormatCount ()
{
  return 1;
}

const csModelConverterFormat *csModelConverterPOV::GetFormat (int idx)
{
  return (idx == 0) ? &FormatInfo : 0;
}

static void WriteVertex (csString &out, iModelDataVertices *Vertices,
  int VertexNum, int NormalNum)
{
  csVector3 v = Vertices->GetVertex (VertexNum);
  csVector3 n = Vertices->GetNormal (NormalNum);

  csString s;
  s << "    <" << v.x << ',' << v.y << ',' << v.z << ">, <";
  s << n.x << ',' << n.y << ',' << n.z << '>';
  out << s;
}

csPtr<iModelData> csModelConverterPOV::Load (uint8 * /*Buffer*/,
					     uint32 /*Size*/)
{
  return 0;
}

/*
  Purpose:

    ::Save writes graphics information to a POV file.

  Example:

    // cone.pov created by IVCON.
    // Original data in cone.iv

    #version 3.0
    #include "colors.inc"
    #include "shapes.inc"
    global_settings { assumed_gamma 2.2 }

    camera {
     right < 4/3, 0, 0>
     up < 0, 1, 0 >
     sky < 0, 1, 0 >
     angle 20
     location < 0, 0, -300 >
     look_at < 0, 0, 0>
    }

    light_source { < 20, 50, -100 > color White }

    background { color SkyBlue }

    #declare RedText = texture {
      pigment { color rgb < 0.8, 0.2, 0.2> }
      finish { ambient 0.2 diffuse 0.5 }
    }

    #declare BlueText = texture {
      pigment { color rgb < 0.2, 0.2, 0.8> }
      finish { ambient 0.2 diffuse 0.5 }
    }
    mesh {
      smooth_triangle {
        < 0.29, -0.29, 0.0>, < 0.0, 0.0, -1.0 >,
        < 38.85, 10.03, 0.0>, < 0.0, 0.0, -1.0 >,
        < 40.21, -0.29, 0.0>, <  0.0, 0.0, -1.0 >
        texture { RedText } }
        ...
      smooth_triangle {
        <  0.29, -0.29, 70.4142 >, < 0.0,  0.0, 1.0 >,
        <  8.56,  -2.51, 70.4142 >, < 0.0,  0.0, 1.0 >,
        <  8.85, -0.29, 70.4142 >, < 0.0,  0.0, 1.0 >
        texture { BlueText } }
    }

  Modified:

    08 October 1998
    17 August 2001

  Author:

    John Burkardt

  Modified by Martin Geisse to work with the new converter system. Removed
  (IMO useless) logging.

*/

csPtr<iDataBuffer> csModelConverterPOV::Save (iModelData *Data,
					      const char *Format)
{
  if (strcasecmp (Format, "pov"))
    return 0;

  csString out;
  out << "// This file was created by Crystal Space's csModelConverterPOV.\n";

/*
  Initial declarations.
*/
  out << "\n";
  out << "#version 3.0\n";
  out << "#include \"colors.inc\"\n";
  out << "#include \"shapes.inc\"\n";
  out << "global_settings { assumed_gamma 2.2 }\n";
  out << "\n";
  out << "camera {\n";
  out << " right < 4/3, 0, 0>\n";
  out << " up < 0, 1, 0 >\n";
  out << " sky < 0, 1, 0 >\n";
  out << " angle 20\n";
  out << " location < 0, 0, -300 >\n";
  out << " look_at < 0, 0, 0>\n";
  out << "}\n";
  out << "\n";
  out << "light_source { < 20, 50, -100 > color White }\n";
  out << "\n";
  out << "background { color SkyBlue }\n";

/*
  Declare RGB textures.
*/
  out << "\n";
  out << "#declare RedText = texture {\n";
  out << "  pigment { color rgb < 0.8, 0.2, 0.2> }\n";
  out << "  finish { ambient 0.2 diffuse 0.5 }\n";
  out << "}\n";
  out << "\n";
  out << "#declare GreenText = texture {\n";
  out << "  pigment { color rgb < 0.2, 0.8, 0.2> }\n";
  out << "  finish { ambient 0.2 diffuse 0.5 }\n";
  out << "}\n";
  out << "\n";
  out << "#declare BlueText = texture {\n";
  out << "  pigment { color rgb < 0.2, 0.2, 0.8> }\n";
  out << "  finish { ambient 0.2 diffuse 0.5 }\n";
  out << "}\n\n";

  int TextureNum = 0;
  csTypedObjectIterator<iModelDataObject> it (Data->QueryObject ());
  while (it.HasNext ())
  {
    iModelDataObject *Object = it.Next ();
    out << "mesh {\n";

    csTypedObjectIterator<iModelDataPolygon> it2 (Object->QueryObject ());
    while (it2.HasNext ())
    {
      iModelDataPolygon *Polygon = it2.Next ();

      size_t i;
      for (i=2; i<Polygon->GetVertexCount (); i++)
      {
        /* @@@ is the vertex order correct ??? */
        out << "  smooth_triangle {\n";
	WriteVertex (out, Object->GetDefaultVertices (),
	  Polygon->GetVertex (0), Polygon->GetNormal (0));
	out << ",\n";
	WriteVertex (out, Object->GetDefaultVertices (),
	  Polygon->GetVertex (i-1), Polygon->GetNormal (i-1));
	out << ",\n";
	WriteVertex (out, Object->GetDefaultVertices (),
	  Polygon->GetVertex (i), Polygon->GetNormal (i));
	out << "\n";
	if (TextureNum == 0) out << "    texture { RedText }\n";
	else if (TextureNum == 1) out << "    texture { GreenText }\n";
	else if (TextureNum == 2) out << "    texture { BlueText }\n";
        out << "  }\n";
	TextureNum = (TextureNum + 1) % 3;
      }
    }
    out << "}\n";
  }

  int Length = out.Length () + 1;
  char *FileData = out.Detach ();
  return csPtr<iDataBuffer> (new csDataBuffer (FileData, Length));
}
