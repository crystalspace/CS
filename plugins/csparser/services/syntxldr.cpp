/*
    Copyright (C) 2001 by Norman Krämer

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

#include <stdarg.h>
#include "cssysdef.h"
#include "syntxldr.h"
#include "csutil/ref.h"
#include "csutil/cscolor.h"
#include "csutil/scanstr.h"
#include "csutil/util.h"
#include "csutil/scfstr.h"
#include "iutil/document.h"
#include "csgeom/matrix3.h"
#include "csgeom/box.h"
#include "csgeom/vector3.h"
#include "csgeom/vector2.h"
#include "csgeom/transfrm.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/texture.h"
#include "ivideo/material.h"
#include "imesh/thing/thing.h"
#include "imesh/thing/ptextype.h"
#include "imesh/thing/polytmap.h"
#include "imesh/thing/portal.h"
#include "imesh/thing/polygon.h"
#include "imesh/object.h"
#include "iutil/object.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "imap/parser.h"
#include "imap/ldrctxt.h"

// Only used for unit-testing. The parser does not otherwise
// depend on any specific XML parser.
#include "csutil/xmltiny.h"

CS_IMPLEMENT_PLUGIN;

SCF_IMPLEMENT_IBASE (csTextSyntaxService)
  SCF_IMPLEMENTS_INTERFACE (iSyntaxService)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iDebugHelper)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTextSyntaxService::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTextSyntaxService::DebugHelper)
  SCF_IMPLEMENTS_INTERFACE (iDebugHelper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csTextSyntaxService);

SCF_EXPORT_CLASS_TABLE (cssynldr)
  SCF_EXPORT_CLASS (csTextSyntaxService,
  	"crystalspace.syntax.loader.service.text",
	"Crystal Space loader services for textual CS syntax")
SCF_EXPORT_CLASS_TABLE_END

enum
{
  XMLTOKEN_ROTX = 1,
  XMLTOKEN_ROTY,
  XMLTOKEN_ROTZ,
  XMLTOKEN_SCALE,
  XMLTOKEN_M11,
  XMLTOKEN_M12,
  XMLTOKEN_M13,
  XMLTOKEN_M21,
  XMLTOKEN_M22,
  XMLTOKEN_M23,
  XMLTOKEN_M31,
  XMLTOKEN_M32,
  XMLTOKEN_M33,
  XMLTOKEN_COPY,
  XMLTOKEN_MULTIPLY2,
  XMLTOKEN_MULTIPLY,
  XMLTOKEN_ADD,
  XMLTOKEN_ALPHA,
  XMLTOKEN_TRANSPARENT,
  XMLTOKEN_KEYCOLOR,
  XMLTOKEN_TILING,
  XMLTOKEN_NONE,
  XMLTOKEN_FLAT,
  XMLTOKEN_GOURAUD,
  XMLTOKEN_LIGHTMAP,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_LIGHTING,
  XMLTOKEN_PORTAL,
  XMLTOKEN_WARP,
  XMLTOKEN_TEXMAP,
  XMLTOKEN_SHADING,
  XMLTOKEN_UVA,
  XMLTOKEN_UV,
  XMLTOKEN_COLOR,
  XMLTOKEN_COLORS,
  XMLTOKEN_COLLDET,
  XMLTOKEN_COSFACT,
  XMLTOKEN_MAXVISIT,
  XMLTOKEN_MIXMODE,
  XMLTOKEN_LEN,
  XMLTOKEN_V,
  XMLTOKEN_ORIG,
  XMLTOKEN_ORIGREF,
  XMLTOKEN_FIRSTLEN,
  XMLTOKEN_FIRST,
  XMLTOKEN_FIRSTREF,
  XMLTOKEN_SECONDLEN,
  XMLTOKEN_SECOND,
  XMLTOKEN_SECONDREF,
  XMLTOKEN_UVEC,
  XMLTOKEN_VVEC,
  XMLTOKEN_MATRIX,
  XMLTOKEN_PLANE,
  XMLTOKEN_UVSHIFT,
  XMLTOKEN_VISCULL,
  XMLTOKEN_W,
  XMLTOKEN_MIRROR,
  XMLTOKEN_STATIC,
  XMLTOKEN_ZFILL,
  XMLTOKEN_CLIP,
  XMLTOKEN_SECTOR
};

csTextSyntaxService::csTextSyntaxService (iBase *parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiDebugHelper);
}

csTextSyntaxService::~csTextSyntaxService ()
{
}

bool csTextSyntaxService::Initialize (iObjectRegistry* object_reg)
{
  csTextSyntaxService::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  xmltokens.Register ("rotx", XMLTOKEN_ROTX);
  xmltokens.Register ("roty", XMLTOKEN_ROTY);
  xmltokens.Register ("rotz", XMLTOKEN_ROTZ);
  xmltokens.Register ("scale", XMLTOKEN_SCALE);
  xmltokens.Register ("m11", XMLTOKEN_M11);
  xmltokens.Register ("m12", XMLTOKEN_M12);
  xmltokens.Register ("m13", XMLTOKEN_M13);
  xmltokens.Register ("m21", XMLTOKEN_M21);
  xmltokens.Register ("m22", XMLTOKEN_M22);
  xmltokens.Register ("m23", XMLTOKEN_M23);
  xmltokens.Register ("m31", XMLTOKEN_M31);
  xmltokens.Register ("m32", XMLTOKEN_M32);
  xmltokens.Register ("m33", XMLTOKEN_M33);
  xmltokens.Register ("copy", XMLTOKEN_COPY);
  xmltokens.Register ("multiply2", XMLTOKEN_MULTIPLY2);
  xmltokens.Register ("multiply", XMLTOKEN_MULTIPLY);
  xmltokens.Register ("add", XMLTOKEN_ADD);
  xmltokens.Register ("alpha", XMLTOKEN_ALPHA);
  xmltokens.Register ("transparent", XMLTOKEN_TRANSPARENT);
  xmltokens.Register ("keycolor", XMLTOKEN_KEYCOLOR);
  xmltokens.Register ("tiling", XMLTOKEN_TILING);
  xmltokens.Register ("none", XMLTOKEN_NONE);
  xmltokens.Register ("flat", XMLTOKEN_FLAT);
  xmltokens.Register ("gouraud", XMLTOKEN_GOURAUD);
  xmltokens.Register ("lightmap", XMLTOKEN_LIGHTMAP);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("lighting", XMLTOKEN_LIGHTING);
  xmltokens.Register ("portal", XMLTOKEN_PORTAL);
  xmltokens.Register ("warp", XMLTOKEN_WARP);
  xmltokens.Register ("texmap", XMLTOKEN_TEXMAP);
  xmltokens.Register ("shading", XMLTOKEN_SHADING);
  xmltokens.Register ("uva", XMLTOKEN_UVA);
  xmltokens.Register ("uv", XMLTOKEN_UV);
  xmltokens.Register ("color", XMLTOKEN_COLOR);
  xmltokens.Register ("colors", XMLTOKEN_COLORS);
  xmltokens.Register ("colldet", XMLTOKEN_COLLDET);
  xmltokens.Register ("cosfact", XMLTOKEN_COSFACT);
  xmltokens.Register ("maxvisit", XMLTOKEN_MAXVISIT);
  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("len", XMLTOKEN_LEN);
  xmltokens.Register ("v", XMLTOKEN_V);
  xmltokens.Register ("orig", XMLTOKEN_ORIG);
  xmltokens.Register ("origref", XMLTOKEN_ORIGREF);
  xmltokens.Register ("firstlen", XMLTOKEN_FIRSTLEN);
  xmltokens.Register ("first", XMLTOKEN_FIRST);
  xmltokens.Register ("firstref", XMLTOKEN_FIRSTREF);
  xmltokens.Register ("secondlen", XMLTOKEN_SECONDLEN);
  xmltokens.Register ("second", XMLTOKEN_SECOND);
  xmltokens.Register ("secondref", XMLTOKEN_SECONDREF);
  xmltokens.Register ("uvec", XMLTOKEN_UVEC);
  xmltokens.Register ("vvec", XMLTOKEN_VVEC);
  xmltokens.Register ("matrix", XMLTOKEN_MATRIX);
  xmltokens.Register ("plane", XMLTOKEN_PLANE);
  xmltokens.Register ("uvshift", XMLTOKEN_UVSHIFT);
  xmltokens.Register ("viscull", XMLTOKEN_VISCULL);
  xmltokens.Register ("w", XMLTOKEN_W);
  xmltokens.Register ("mirror", XMLTOKEN_MIRROR);
  xmltokens.Register ("static", XMLTOKEN_STATIC);
  xmltokens.Register ("zfill", XMLTOKEN_ZFILL);
  xmltokens.Register ("clip", XMLTOKEN_CLIP);
  xmltokens.Register ("sector", XMLTOKEN_SECTOR);
  return true;
}

void csTextSyntaxService::OptimizePolygon (iPolygon3D *p)
{
#ifndef CS_USE_NEW_RENDERER
  if (!p->GetPortal () || p->GetAlpha ()
  	|| p->GetPolyTexType ()->GetMixMode () != 0)
    return;
#endif // CS_USE_NEW_RENDERER
  iMaterialWrapper *mat = p->GetMaterial ();
  if (mat)
  {
    iMaterial *m = mat->GetMaterial ();
    iTextureHandle *th = m ? m->GetTexture () : NULL;
    if (th && th->GetKeyColor ())
      return;
  }

  p->SetTextureType (POLYTXT_NONE);
}

class MissingSectorCallback : public iPortalCallback
{
public:
  csRef<iLoaderContext> ldr_context;
  char* sectorname;

  SCF_DECLARE_IBASE;
  MissingSectorCallback (iLoaderContext* ldr_context, const char* sector)
  {
    SCF_CONSTRUCT_IBASE (NULL);
    MissingSectorCallback::ldr_context = ldr_context;
    sectorname = csStrNew (sector);
  }
  virtual ~MissingSectorCallback ()
  {
    delete[] sectorname;
  }
  
  virtual bool Traverse (iPortal* portal, iBase* /*context*/)
  {
    iSector* sector = ldr_context->FindSector (sectorname);
    if (!sector) return false;
    portal->SetSector (sector);
    // For efficiency reasons we deallocate the name here.
    delete[] sectorname;
    sectorname = NULL;
    portal->RemoveMissingSectorCallback (this);
    return true;
  }
};

SCF_IMPLEMENT_IBASE (MissingSectorCallback)
  SCF_IMPLEMENTS_INTERFACE (iPortalCallback)
SCF_IMPLEMENT_IBASE_END

const char* csTextSyntaxService::MatrixToText (
	const csMatrix3 &m, int indent, bool newline)
{
  char line[100];
  csString ind = csString::PadLeft (' ', indent);

  text = ind;
  text.Append ("MATRIX (\n");

  sprintf (line, "%g, %g, %g\n", m.m11, m.m12, m.m13);
  text.Append (ind);
  text.Append (line);

  sprintf (line, "%g, %g, %g\n", m.m21, m.m22, m.m23);
  text.Append (ind);
  text.Append (line);

  sprintf (line, "%g, %g, %g\n", m.m31, m.m32, m.m33);
  text.Append (ind);
  text.Append (line);

  text.Append (ind);
  if (newline)
    text.Append (")\n");
  else
    text.Append (")");

  return text;
}

const char* csTextSyntaxService::VectorToText (
	const char *vname, float x, float y, float z,
					       int indent, bool newline)
{
  char line[100];
  csString ind = csString::PadLeft (' ', indent);

  sprintf (line, "%s (%g, %g, %g)%c", vname, x, y, z, newline ? '\n' : ' ');
  text.Append (ind);
  text.Append (line);

  return text;
}

const char* csTextSyntaxService::VectorToText (
	const char *vname, const csVector3 &v, int indent,
					       bool newline)
{
  return VectorToText (vname, v.x, v.y, v.z, indent, newline);
}

const char* csTextSyntaxService::VectorToText (
	const char *vname, float x, float y, int indent,
					       bool newline)
{
  char line[100];
  csString ind = csString::PadLeft (' ', indent);

  sprintf (line, "%s (%g, %g)%c", vname, x, y, newline ? '\n' : ' ');
  text.Append (ind);
  text.Append (line);

  return text;
}

const char* csTextSyntaxService::VectorToText (
	const char *vname, const csVector2 &v, int indent,
	bool newline)
{
  return VectorToText (vname, v.x, v.y, indent, newline, newline);
}

const char* csTextSyntaxService::BoolToText (
	const char *vname, bool b, int indent, bool newline)
{
  char line[100];
  csString ind = csString::PadLeft (' ', indent);

  sprintf (line, "%s (%s)%c", vname, b ? "yes" : "no", newline ? '\n' : ' ');
  text.Append (ind);
  text.Append (line);

  return text;
}

const char* csTextSyntaxService::MixmodeToText (
	uint mixmode, int indent, bool newline)
{
  csString ind = csString::PadLeft (' ', indent);

  text = ind;
  text.Append ("MIXMODE (\n");
#ifndef CS_USE_NEW_RENDERER
  if (mixmode & CS_FX_COPY)
  {
    text.Append (ind);
    text.Append (" COPY ()\n");
  }
  if (mixmode & CS_FX_ADD)
  {
    text.Append (ind);
    text.Append (" ADD ()\n");
  }
  if(mixmode & CS_FX_MULTIPLY)
  {
    text.Append (ind);
    text.Append (" MULTIPLY ()\n");
  }
  if(mixmode & CS_FX_MULTIPLY2)
  {
    text.Append (ind);
    text.Append (" MULTIPLY2 ()\n");
  }
  if(mixmode & CS_FX_KEYCOLOR)
  {
    text.Append (ind);
    text.Append (" KEYCOLOR ()\n");
  }
  if(mixmode & CS_FX_TILING)
  {
    text.Append (ind);
    text.Append (" TILING ()\n");
  }
  if(mixmode & CS_FX_TRANSPARENT)
  {
    text.Append (ind);
    text.Append (" TRANSPARENT ()\n");
  }
  if(mixmode & CS_FX_ALPHA)
  {
    char buf[30];
    sprintf(buf, "  ALPHA (%.5g)\n", float(mixmode&CS_FX_MASK_ALPHA)/255.);;
    text.Append (ind);
    text.Append (buf);
  }
  text.Append (ind);
#endif // CS_USE_NEW_RENDERER
  if (newline)
    text.Append (")\n");
  else
    text.Append (")");

  return text;
}

bool csTextSyntaxService::ParseBool (iDocumentNode* node, bool& result,
		bool def_result)
{
  const char* v = node->GetContentsValue ();
  if (!v) { result = def_result; return true; }
  if (!strcasecmp (v, "yes"))   { result = true; return true; }
  if (!strcasecmp (v, "no"))    { result = false; return true; }
  if (!strcasecmp (v, "true"))  { result = true; return true; }
  if (!strcasecmp (v, "false")) { result = false; return true; }
  if (!strcasecmp (v, "on"))    { result = true; return true; }
  if (!strcasecmp (v, "off"))   { result = false; return true; }
  ReportError ("crystalspace.syntax.boolean", node,
    "Bad boolean value '%s'!", v);
  return false;
}

bool csTextSyntaxService::ParseMatrix (iDocumentNode* node, csMatrix3 &m)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_SCALE:
        {
	  float scale;
	  scale = child->GetAttributeValueAsFloat ("x");
	  if (ABS (scale) > SMALL_EPSILON) m *= csXScaleMatrix3 (scale);

	  scale = child->GetAttributeValueAsFloat ("y");
	  if (ABS (scale) > SMALL_EPSILON) m *= csYScaleMatrix3 (scale);

	  scale = child->GetAttributeValueAsFloat ("z");
	  if (ABS (scale) > SMALL_EPSILON) m *= csZScaleMatrix3 (scale);

	  scale = child->GetAttributeValueAsFloat ("all");
	  if (ABS (scale) > SMALL_EPSILON) m *= scale;
	}
        break;
      case XMLTOKEN_ROTX:
        {
	  float angle = child->GetContentsValueAsFloat ();
          m *= csXRotMatrix3 (angle);
	}
        break;
      case XMLTOKEN_ROTY:
        {
	  float angle = child->GetContentsValueAsFloat ();
          m *= csYRotMatrix3 (angle);
	}
        break;
      case XMLTOKEN_ROTZ:
        {
	  float angle = child->GetContentsValueAsFloat ();
          m *= csZRotMatrix3 (angle);
	}
        break;
      case XMLTOKEN_M11: m.m11 = child->GetContentsValueAsFloat (); break;
      case XMLTOKEN_M12: m.m12 = child->GetContentsValueAsFloat (); break;
      case XMLTOKEN_M13: m.m13 = child->GetContentsValueAsFloat (); break;
      case XMLTOKEN_M21: m.m21 = child->GetContentsValueAsFloat (); break;
      case XMLTOKEN_M22: m.m22 = child->GetContentsValueAsFloat (); break;
      case XMLTOKEN_M23: m.m23 = child->GetContentsValueAsFloat (); break;
      case XMLTOKEN_M31: m.m31 = child->GetContentsValueAsFloat (); break;
      case XMLTOKEN_M32: m.m32 = child->GetContentsValueAsFloat (); break;
      case XMLTOKEN_M33: m.m33 = child->GetContentsValueAsFloat (); break;
      default:
        ReportBadToken (child);
        return false;
    }
  }
  return true;
}

bool csTextSyntaxService::ParseBox (iDocumentNode* node, csBox3 &v)
{
  csVector3 miv, mav;
  csRef<iDocumentNode> minnode = node->GetNode ("min");
  if (!minnode)
  {
    ReportError ("crystalspace.syntax.box", node,
      "Expected 'min' node!");
    return false;
  }
  miv.x = minnode->GetAttributeValueAsFloat ("x");
  miv.y = minnode->GetAttributeValueAsFloat ("y");
  miv.z = minnode->GetAttributeValueAsFloat ("z");
  csRef<iDocumentNode> maxnode = node->GetNode ("max");
  if (!maxnode)
  {
    ReportError ("crystalspace.syntax.box", node,
      "Expected 'max' node!");
    return false;
  }
  mav.x = maxnode->GetAttributeValueAsFloat ("x");
  mav.y = maxnode->GetAttributeValueAsFloat ("y");
  mav.z = maxnode->GetAttributeValueAsFloat ("z");
  v.Set (miv, mav);
  return true;
}

bool csTextSyntaxService::ParseVector (iDocumentNode* node, csVector3 &v)
{
  v.x = node->GetAttributeValueAsFloat ("x");
  v.y = node->GetAttributeValueAsFloat ("y");
  v.z = node->GetAttributeValueAsFloat ("z");
  return true;
}

bool csTextSyntaxService::ParseColor (iDocumentNode* node, csColor &c)
{
  c.red = node->GetAttributeValueAsFloat ("red");
  c.green = node->GetAttributeValueAsFloat ("green");
  c.blue = node->GetAttributeValueAsFloat ("blue");
  return true;
}

bool csTextSyntaxService::ParseMixmode (iDocumentNode* node, uint &mixmode)
{
#define MIXMODE_EXCLUSIVE				\
  if (mixmode & CS_FX_MASK_MIXMODE)			\
  {							\
    if (!warned)					\
    {							\
      Report ("crystalspace.syntax.mixmode",		\
        CS_REPORTER_SEVERITY_WARNING,			\
	child,						\
	"Multiple exclusive mixmodes specified! "	\
	"Only first one will be used.");		\
      warned = true;					\
    }							\
  }							\
  else
#ifndef CS_USE_NEW_RENDERER
  bool warned = false;
  mixmode = 0;
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_COPY: 
	MIXMODE_EXCLUSIVE mixmode |= CS_FX_COPY; break;
      case XMLTOKEN_MULTIPLY: 
	MIXMODE_EXCLUSIVE mixmode |= CS_FX_MULTIPLY; break;
      case XMLTOKEN_MULTIPLY2: 
	MIXMODE_EXCLUSIVE mixmode |= CS_FX_MULTIPLY2; break;
      case XMLTOKEN_ADD: 
	MIXMODE_EXCLUSIVE mixmode |= CS_FX_ADD; break;
      case XMLTOKEN_ALPHA:
        MIXMODE_EXCLUSIVE {
	  mixmode &= ~CS_FX_MASK_ALPHA;
	  float alpha = child->GetContentsValueAsFloat ();
	  mixmode |= CS_FX_SETALPHA (alpha);
	}
	break;
      case XMLTOKEN_TRANSPARENT: mixmode |= CS_FX_TRANSPARENT; break;
      case XMLTOKEN_KEYCOLOR: mixmode |= CS_FX_KEYCOLOR; break;
      case XMLTOKEN_TILING: mixmode |= CS_FX_TILING; break;
      default:
        ReportBadToken (child);
        return false;
    }
  }
#endif // CS_USE_NEW_RENDERER
  return true;

#undef MIXMODE_EXCLUSIVE
}

bool csTextSyntaxService::ParseTextureMapping (
	iDocumentNode* node, const csVector3* vref, uint &texspec,
	csVector3 &tx_orig, csVector3 &tx1, csVector3 &tx2, csVector3 &len,
	csMatrix3 &tx_m, csVector3 &tx_v,
	csVector2 &uv_shift,
	int &idx1, csVector2 &uv1,
	int &idx2, csVector2 &uv2,
	int &idx3, csVector2 &uv3,
	char *plane, const char *polyname)
{
  int cur_uvidx = 0;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_ORIGREF:
        tx_orig = vref[child->GetContentsValueAsInt ()];
        break;
      case XMLTOKEN_ORIG:
        texspec &= ~CSTEX_UV;
        texspec |= CSTEX_V1;
	tx_orig.x = child->GetAttributeValueAsFloat ("x");
	tx_orig.y = child->GetAttributeValueAsFloat ("y");
	tx_orig.z = child->GetAttributeValueAsFloat ("z");
	break;
      case XMLTOKEN_FIRSTREF:
        tx1 = vref[child->GetContentsValueAsInt ()];
	break;
      case XMLTOKEN_FIRST:
        texspec &= ~CSTEX_UV;
        texspec |= CSTEX_V1;
	tx1.x = child->GetAttributeValueAsFloat ("x");
	tx1.y = child->GetAttributeValueAsFloat ("y");
	tx1.z = child->GetAttributeValueAsFloat ("z");
        break;
      case XMLTOKEN_FIRSTLEN:
        texspec &= ~CSTEX_UV;
        texspec |= CSTEX_V1;
	len.y = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_SECONDREF:
        tx2 = vref[child->GetContentsValueAsInt ()];
	break;
      case XMLTOKEN_SECOND:
        texspec &= ~CSTEX_UV;
        texspec |= CSTEX_V2;
	tx2.x = child->GetAttributeValueAsFloat ("x");
	tx2.y = child->GetAttributeValueAsFloat ("y");
	tx2.z = child->GetAttributeValueAsFloat ("z");
        break;
      case XMLTOKEN_SECONDLEN:
        texspec &= ~CSTEX_UV;
        texspec |= CSTEX_V2;
	len.z = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_LEN:
        texspec &= ~CSTEX_UV;
	len.x = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_MATRIX:
        texspec &= ~CSTEX_UV;
        ParseMatrix (child, tx_m);
        len.x = 0;
        break;
      case XMLTOKEN_V:
        texspec &= ~CSTEX_UV;
	tx_v.x = child->GetAttributeValueAsFloat ("x");
	tx_v.y = child->GetAttributeValueAsFloat ("y");
	tx_v.z = child->GetAttributeValueAsFloat ("z");
        len.x = 0;
        break;
      case XMLTOKEN_PLANE:
        {
          texspec &= ~CSTEX_UV;
	  const char* v = child->GetContentsValue ();
	  if (v) strcpy (plane, v);
	  else plane[0] = 0;
          len.x = 0;
	}
        break;
      case XMLTOKEN_UVSHIFT:
        texspec |= CSTEX_UV_SHIFT;
	uv_shift.x = child->GetAttributeValueAsFloat ("u");
	uv_shift.y = child->GetAttributeValueAsFloat ("v");
        break;
      case XMLTOKEN_UVEC:
        texspec &= ~CSTEX_UV;
        texspec |= CSTEX_V1;
	tx1.x = child->GetAttributeValueAsFloat ("x");
	tx1.y = child->GetAttributeValueAsFloat ("y");
	tx1.z = child->GetAttributeValueAsFloat ("z");
        len.y = tx1.Norm ();
        tx1 += tx_orig;
        break;
      case XMLTOKEN_VVEC:
        texspec &= ~CSTEX_UV;
        texspec |= CSTEX_V2;
	tx2.x = child->GetAttributeValueAsFloat ("x");
	tx2.y = child->GetAttributeValueAsFloat ("y");
	tx2.z = child->GetAttributeValueAsFloat ("z");
        len.z = tx2.Norm ();
        tx2 += tx_orig;
        break;
      case XMLTOKEN_UV:
        {
          texspec |= CSTEX_UV;
	  int idx = child->GetAttributeValueAsInt ("idx");
	  float x = child->GetAttributeValueAsFloat ("u");
	  float y = child->GetAttributeValueAsFloat ("v");
	  switch (cur_uvidx)
	  {
	    case 0: idx1 = idx; uv1.x = x; uv1.y = y; break;
	    case 1: idx2 = idx; uv2.x = x; uv2.y = y; break;
	    case 2: idx3 = idx; uv3.x = x; uv3.y = y; break;
	    default:
              ReportError ("crystalspace.syntax.texture", child,
                "Too many <uv> nodes inside <texmap>! Only 3 allowed");
	      return false;
	  }
	  cur_uvidx++;
	}
        break;
      default:
        ReportBadToken (child);
        return false;
    }
  }

  if (texspec & CSTEX_V2)
  {
    if (!len.y)
    {
      ReportError ("crystalspace.syntax.texture", node,
        "Bad texture specification for polygon '%s'", polyname);
      len.y = 1;
      return false;
    }
    if (!len.z)
    {
      ReportError ("crystalspace.syntax.texture", node,
        "Bad texture specification for polygon '%s'", polyname);
      len.z = 1;
      return false;
    }
  }
  else
  {
    if (!len.y)
    {
      ReportError ("crystalspace.syntax.texture", node,
        "Bad texture specification for polygon '%s'", polyname);
      len.y = 1;
      return false;
    }
  }

  return true;
}

bool csTextSyntaxService::ParsePortal (
	iDocumentNode* node, iLoaderContext* ldr_context,
	iPolygon3D* poly3d,
	csVector &flags, bool &mirror, bool &warp, int& msv,
	csMatrix3 &m, csVector3 &before, csVector3 &after)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_MAXVISIT:
	msv = child->GetContentsValueAsInt ();
	break;
      case XMLTOKEN_MATRIX:
	ParseMatrix (child, m);
	mirror = false;
	warp = true;
        break;
      case XMLTOKEN_V:
        ParseVector (child, before);
        after = before;
        mirror = false;
        warp = true;
        break;
      case XMLTOKEN_W:
        ParseVector (child, after);
        mirror = false;
        warp = true;
        break;
      case XMLTOKEN_MIRROR:
	if (!ParseBool (child, mirror, true))
	  return false;
        break;
      case XMLTOKEN_STATIC:
        flags.Push ((csSome)CS_PORTAL_STATICDEST);
        break;
      case XMLTOKEN_ZFILL:
        flags.Push ((csSome)CS_PORTAL_ZFILL);
        break;
      case XMLTOKEN_CLIP:
        flags.Push ((csSome)CS_PORTAL_CLIPDEST);
        break;
      case XMLTOKEN_SECTOR:
	{
	  iSector* sector = ldr_context->
	  	FindSector (child->GetContentsValue ());
	  if (sector)
	  {
            poly3d->CreatePortal (sector);
	  }
	  else
	  {
	    poly3d->CreateNullPortal ();
	    iPortal* portal = poly3d->GetPortal ();
	    MissingSectorCallback* mscb = new MissingSectorCallback (
	    	ldr_context, child->GetContentsValue ());
	    portal->SetMissingSectorCallback (mscb);
	    mscb->DecRef ();
	  }
	}
	break;
      default:
	ReportBadToken (child);
        return false;
    }
  }

  return true;
}

bool csTextSyntaxService::ParsePoly3d (
        iDocumentNode* node,
	iLoaderContext* ldr_context,
	iEngine* , iPolygon3D* poly3d,
	float default_texlen,
	iThingState* thing_state, int vt_offset)
{
  iMaterialWrapper* mat = NULL;

  if (!thing_type)
  {
    csRef<iPluginManager> plugin_mgr (
    	CS_QUERY_REGISTRY (object_reg, iPluginManager));
    CS_ASSERT (plugin_mgr != NULL);
    thing_type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	  "crystalspace.mesh.object.thing", iMeshObjectType);
    if (!thing_type)
      thing_type = CS_LOAD_PLUGIN (plugin_mgr,
    	  "crystalspace.mesh.object.thing", iMeshObjectType);
  }

  CS_ASSERT (thing_type != NULL);
  csRef<iThingEnvironment> te (SCF_QUERY_INTERFACE (
  	thing_type, iThingEnvironment));

  uint texspec = 0;
  int tx_uv_i1 = 0;
  int tx_uv_i2 = 0;
  int tx_uv_i3 = 0;
  csVector2 tx_uv1;
  csVector2 tx_uv2;
  csVector2 tx_uv3;

  csVector3 tx_orig (0, 0, 0), tx1 (0, 0, 0), tx2 (0, 0, 0);
  csVector3 tx_len (default_texlen, default_texlen, default_texlen);

  csMatrix3 tx_matrix;
  csVector3 tx_vector (0, 0, 0);
  char plane_name[100];
  plane_name[0] = 0;
  csVector2 uv_shift (0, 0);

  bool do_mirror = false;
  int set_colldet = 0; // If 1 then set, if -1 then reset, else default.
  int set_viscull = 0; // If 1 then set, if -1 then reset, else default.

  bool init_gouraud_poly = false;
  csRef<iPolyTexFlat> fs;
  csRef<iPolyTexGouraud> gs;
  int num_uv = 0;
  int num_col = 0;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_MATERIAL:
        mat = ldr_context->FindMaterial (child->GetContentsValue ());
        if (mat == NULL)
        {
          ReportError ("crystalspace.syntax.polygon", child,
            "Couldn't find material named '%s'!", child->GetContentsValue ());
          return false;
        }
        poly3d->SetMaterial (mat);
        break;
      case XMLTOKEN_LIGHTING:
        {
          bool do_lighting;
	  if (!ParseBool (child, do_lighting, true))
	    return false;
          poly3d->GetFlags ().Set (CS_POLY_LIGHTING,
	  	do_lighting ? CS_POLY_LIGHTING : 0);
        }
        break;
      case XMLTOKEN_COSFACT:
        poly3d->SetCosinusFactor (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_ALPHA:
        poly3d->SetAlpha (child->GetContentsValueAsInt () * 655 / 256);
	// Disable vis culling for alpha objects.
	if (!set_viscull) set_viscull = -1;
        break;
      case XMLTOKEN_VISCULL:
        {
          bool do_viscull;
	  if (!ParseBool (child, do_viscull, true))
	    return false;
	  if (do_viscull) set_viscull = 1;
	  else set_viscull = -1;
        }
        break;
      case XMLTOKEN_COLLDET:
        {
          bool do_colldet;
	  if (!ParseBool (child, do_colldet, true))
	    return false;
	  if (do_colldet) set_colldet = 1;
	  else set_colldet = -1;
        }
        break;
      case XMLTOKEN_PORTAL:
        {
          csMatrix3 m_w; m_w.Identity ();
          csVector3 v_w_before (0, 0, 0);
          csVector3 v_w_after (0, 0, 0);
	  csVector flags;
	  bool do_warp = false;
	  int msv = -1;

	  if (ParsePortal (child, ldr_context,
	      poly3d, flags, do_mirror, do_warp, msv,
	      m_w, v_w_before, v_w_after))
	  {
	    for (int i = 0; i < flags.Length (); i++)
	      poly3d->GetPortal ()->GetFlags ().Set ((uint)flags.Get (i));

	    if (do_mirror)
	    {
	      if (!set_colldet) set_colldet = 1;
	    }
	    else if (do_warp)
	      poly3d->GetPortal ()->SetWarp (m_w, v_w_before, v_w_after);

	    if (msv != -1)
	    {
	      poly3d->GetPortal ()->SetMaximumSectorVisit (msv);
	    }
	  }
        }
        break;
      case XMLTOKEN_TEXMAP:
	if (!ParseTextureMapping (child, thing_state->GetVertices (), texspec,
			   tx_orig, tx1, tx2, tx_len,
			   tx_matrix, tx_vector,
			   uv_shift,
			   tx_uv_i1, tx_uv1,
			   tx_uv_i2, tx_uv2,
			   tx_uv_i3, tx_uv3,
			   plane_name, poly3d->QueryObject ()->GetName ()))
	{
	  return false;
	}
        break;
      case XMLTOKEN_V:
        {
	  int* vts = poly3d->GetVertexIndices ();
	  int vt_idx = child->GetContentsValueAsInt ();
	  bool ignore = false;
	  for (int i = 0 ; i < poly3d->GetVertexCount () ; i++)
	  {
	    if (vts[i] == vt_idx+vt_offset)
	    {
	      csPrintf ("Duplicate vertex-index found! "
			"(polygon '%s') ignored ...\n",
			poly3d->QueryObject ()->GetName ());
	      ignore = true;
	    }
	  }
	  if (!ignore)
	    poly3d->CreateVertex (vt_idx+vt_offset);
        }
        break;
      case XMLTOKEN_SHADING:
	{
	  int shading;
	  const char* shad = child->GetContentsValue ();
          if (!strcasecmp (shad, "none"))
	    shading = POLYTXT_NONE;
	  else if (!strcasecmp (shad, "flat"))
	    shading = POLYTXT_FLAT;
	  else if (!strcasecmp (shad, "gouraud"))
	    shading = POLYTXT_GOURAUD;
	  else if (!strcasecmp (shad, "lightmap"))
	    shading = POLYTXT_LIGHTMAP;
	  else
	  {
	    ReportError ("crystalspace.syntax.polygon", child,
	      "Bad 'shading' specification '%s'!", shad);
            return false;
	  }
          poly3d->SetTextureType (shading);
	}
        break;
      case XMLTOKEN_MIXMODE:
        {
          uint mixmode;
	  if (ParseMixmode (child, mixmode))
	  {
	    iPolyTexType* ptt = poly3d->GetPolyTexType ();
#ifndef CS_USE_NEW_RENDERER
            ptt->SetMixMode (mixmode);
	    if (mixmode & CS_FX_MASK_ALPHA)
	      poly3d->SetAlpha (mixmode & CS_FX_MASK_ALPHA);
#endif // CS_USE_NEW_RENDERER
	  }
	}
        break;
      case XMLTOKEN_UV:
        {
	  float u = child->GetAttributeValueAsFloat ("u");
	  float v = child->GetAttributeValueAsFloat ("v");
	  if (!init_gouraud_poly)
	  {
            poly3d->SetTextureType (POLYTXT_GOURAUD);
	    init_gouraud_poly = true;
	  }
	  if (!fs)
	  {
	    iPolyTexType* ptt = poly3d->GetPolyTexType ();
	    fs = SCF_QUERY_INTERFACE (ptt, iPolyTexFlat);
	    fs->Setup (poly3d);
	  }
	  if (num_uv >= poly3d->GetVertexCount ())
	  {
	    ReportError ("crystalspace.syntax.polygon", child,
	      "Too many <uv> statements in polygon!");
	    return false;
	  }
	  fs->SetUV (num_uv, u, v);
	  num_uv++;
	}
	break;
      case XMLTOKEN_UVA:
        {
	  float angle = child->GetAttributeValueAsFloat ("angle");
	  float scale = child->GetAttributeValueAsFloat ("scale");
	  float shift = child->GetAttributeValueAsFloat ("shift");
	  if (!init_gouraud_poly)
	  {
            poly3d->SetTextureType (POLYTXT_GOURAUD);
	    init_gouraud_poly = true;
	  }
	  if (!fs)
	  {
	    iPolyTexType* ptt = poly3d->GetPolyTexType ();
	    fs = SCF_QUERY_INTERFACE (ptt, iPolyTexFlat);
	    fs->Setup (poly3d);
	  }
	  if (num_uv >= poly3d->GetVertexCount ())
	  {
	    ReportError ("crystalspace.syntax.polygon", child,
	      "Too many <uva> statements in polygon!");
	    return false;
	  }
          float a = angle * TWO_PI / 360.;
          fs->SetUV (num_uv, cos (a) * scale + shift, sin (a) * scale + shift);
	  num_uv++;
	}
	break;
      case XMLTOKEN_COLOR:
        {
	  float r = child->GetAttributeValueAsFloat ("red");
	  float g = child->GetAttributeValueAsFloat ("green");
	  float b = child->GetAttributeValueAsFloat ("blue");
	  if (!init_gouraud_poly)
	  {
            poly3d->SetTextureType (POLYTXT_GOURAUD);
	    init_gouraud_poly = true;
	  }
	  if (!gs)
	  {
	    iPolyTexType* ptt = poly3d->GetPolyTexType ();
	    gs = SCF_QUERY_INTERFACE (ptt, iPolyTexGouraud);
	    gs->Setup (poly3d);
	  }
	  if (num_col >= poly3d->GetVertexCount ())
	  {
	    ReportError ("crystalspace.syntax.polygon", child,
	      "Too many <color> statements in polygon!");
	    return false;
	  }
	  gs->SetColor (num_col, csColor (r, g, b));
	  num_col++;
	}
	break;
      default:
        ReportBadToken (child);
        return false;
    }
  }

  if (poly3d->GetVertexCount () < 3)
  {
    ReportError ("crystalspace.syntax.polygon", node,
      "Polygon '%s' contains just %d vertices!",
      poly3d->QueryObject()->GetName(),
      poly3d->GetVertexCount ());
    return false;
  }

  if (set_colldet == 1)
    poly3d->GetFlags ().Set (CS_POLY_COLLDET);
  else if (set_colldet == -1)
    poly3d->GetFlags ().Reset (CS_POLY_COLLDET);

  if (!set_viscull)
  {
    mat = poly3d->GetMaterial ();
    csRef<iMaterialEngine> mateng = SCF_QUERY_INTERFACE (mat, iMaterialEngine);
    if (mateng)
    {
      iTextureWrapper* tw = mateng->GetTextureWrapper ();
      if (tw)
      {
        iImage* im = tw->GetImageFile ();
        if (im)
        {
          if (im->HasKeycolor ()) set_viscull = -1;
	  else if (im->GetFormat () & CS_IMGFMT_ALPHA)
	    set_viscull = -1;
        }
      }
    }
  }

  if (set_viscull == 1)
    poly3d->GetFlags ().Set (CS_POLY_VISCULL);
  else if (set_viscull == -1)
    poly3d->GetFlags ().Reset (CS_POLY_VISCULL);

  if (texspec & CSTEX_UV)
  {
    poly3d->SetTextureSpace (
			     poly3d->GetVertex (tx_uv_i1), tx_uv1,
			     poly3d->GetVertex (tx_uv_i2), tx_uv2,
			     poly3d->GetVertex (tx_uv_i3), tx_uv3);
  }
  else if (texspec & CSTEX_V1)
  {
    if (texspec & CSTEX_V2)
    {
      if ((tx1-tx_orig) < SMALL_EPSILON)
      {
        ReportError ("crystalspace.syntax.polygon", node,
          "Bad texture specification!");
	return false;
      }
      else if ((tx2-tx_orig) < SMALL_EPSILON)
      {
        ReportError ("crystalspace.syntax.polygon", node,
          "Bad texture specification!");
	return false;
      }
      else poly3d->SetTextureSpace (tx_orig, tx1, tx_len.y, tx2, tx_len.z);
    }
    else
    {
      if ((tx1-tx_orig) < SMALL_EPSILON)
      {
        ReportError ("crystalspace.syntax.polygon", node,
          "Bad texture specification!");
	return false;
      }
      else poly3d->SetTextureSpace (tx_orig, tx1, tx_len.x);
    }
  }
  else if (plane_name[0])
  {
    iPolyTxtPlane* pl = te->FindPolyTxtPlane (plane_name);
    if (!pl)
    {
      ReportError ("crystalspace.syntax.polygon", node,
        "Can't find plane '%s' for polygon '%s'",
      	plane_name, poly3d->QueryObject ()->GetName ());
      return false;
    }
    poly3d->SetTextureSpace (pl);
  }
  else if (tx_len.x)
  {
    // If a length is given (with 'LEN') we will first see if the polygon
    // is coplanar with the X, Y, or Z plane. In that case we will use
    // a standard plane. Otherwise we will just create a plane specific
    // for this case given the first two vertices.
    bool same_x = true, same_y = true, same_z = true;
    const csVector3& v = poly3d->GetVertex (0);
    for (int i = 1 ; i < poly3d->GetVertexCount () ; i++)
    {
      const csVector3& v2 = poly3d->GetVertex (i);
      if (same_x && ABS (v.x-v2.x) >= SMALL_EPSILON) same_x = false;
      if (same_y && ABS (v.y-v2.y) >= SMALL_EPSILON) same_y = false;
      if (same_z && ABS (v.z-v2.z) >= SMALL_EPSILON) same_z = false;
    }
    if (same_x)
    {
      char buf[200];
      sprintf (buf, "__X_%g,%g__", v.x, tx_len.x);
      csRef<iPolyTxtPlane> pl (te->FindPolyTxtPlane (buf));
      if (!pl)
      {
        pl = te->CreatePolyTxtPlane ();
        pl->QueryObject()->SetName (buf);
        pl->SetTextureSpace (csVector3 (v.x, 0, 0), csVector3 (v.x, 0, 1),
			     tx_len.x, csVector3 (v.x, 1, 0), tx_len.x);
      }
      poly3d->SetTextureSpace (pl);
    }
    else if (same_y)
    {
      char buf[200];
      sprintf (buf, "__Y_%g,%g__", v.y, tx_len.x);
      csRef<iPolyTxtPlane> pl (te->FindPolyTxtPlane (buf));
      if (!pl)
      {
        pl = te->CreatePolyTxtPlane ();
        pl->QueryObject()->SetName (buf);
        pl->SetTextureSpace (csVector3 (0, v.y, 0), csVector3 (1, v.y, 0),
			     tx_len.x, csVector3 (0, v.y, 1), tx_len.x);
      }
      poly3d->SetTextureSpace (pl);
    }
    else if (same_z)
    {
      char buf[200];
      sprintf (buf, "__Z_%g,%g__", v.z, tx_len.x);
      csRef<iPolyTxtPlane> pl (te->FindPolyTxtPlane (buf));
      if (!pl)
      {
        pl = te->CreatePolyTxtPlane ();
        pl->QueryObject()->SetName (buf);
        pl->SetTextureSpace (csVector3 (0, 0, v.z), csVector3 (1, 0, v.z),
			     tx_len.x, csVector3 (0, 1, v.z), tx_len.x);
      }
      poly3d->SetTextureSpace (pl);
    }
    else
      poly3d->SetTextureSpace (poly3d->GetVertex (0), poly3d->GetVertex (1),
			       tx_len.x);
  }
  else
    poly3d->SetTextureSpace (tx_matrix, tx_vector);

  if (texspec & CSTEX_UV_SHIFT)
  {
    iPolyTexType* ptt = poly3d->GetPolyTexType ();
    csRef<iPolyTexLightMap> plm (SCF_QUERY_INTERFACE (ptt, iPolyTexLightMap));
    if (plm)
    {
      plm->GetPolyTxtPlane ()->GetTextureSpace (tx_matrix, tx_vector);
      // T = Mot * (O - Vot)
      // T = Mot * (O - Vot) + Vuv      ; Add shift Vuv to final texture map
      // T = Mot * (O - Vot) + Mot * Mot-1 * Vuv
      // T = Mot * (O - Vot + Mot-1 * Vuv)
      csVector3 shift (uv_shift.x, uv_shift.y, 0);
      tx_vector -= tx_matrix.GetInverse () * shift;
      poly3d->SetTextureSpace (tx_matrix, tx_vector);
    }
  }

  if (do_mirror)
    poly3d->GetPortal ()->SetWarp (csTransform::GetReflect (
    	poly3d->GetWorldPlane () ));

  OptimizePolygon (poly3d);

  return true;
}

void csTextSyntaxService::ReportError (const char* msgid,
	iDocumentNode* errornode, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  ReportV (msgid, CS_REPORTER_SEVERITY_ERROR, errornode, msg, arg);
  va_end (arg);
}

void csTextSyntaxService::Report (const char* msgid, int severity, 
	iDocumentNode* errornode, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  ReportV (msgid, severity, errornode, msg, arg);
  va_end (arg);
}

void csTextSyntaxService::ReportV (const char* msgid, int severity, 
	iDocumentNode* errornode, const char* msg, va_list arg)
{
  csString errmsg;
  errmsg.FormatV (msg, arg);
  bool first = true;

  csString nodepath;
  csRef<iDocumentNode> n (errornode);
  while (n)
  {
    const char* v = n->GetValue ();
    const char* name = n->GetAttributeValue ("name");
    if (name || (v && *v))
    {
      if (first) { nodepath = nodepath; first = false; }
      else nodepath = "," + nodepath;
      if (name)
      {
        nodepath = ")" + nodepath;
        nodepath = name + nodepath;
        nodepath = "(" + nodepath;
      }
      if (v && *v)
      {
        nodepath = v + nodepath;
      }
    }
    n = n->GetParent ();
  }
  if (nodepath != "")
    errmsg.Format ("%s\n[node: %s]", 
      (const char*)errmsg,
      (const char*)nodepath);

  if (reporter)
  {
    reporter->Report (severity, msgid, "%s",
    	(const char*)errmsg);
  }
  else
  {
    csPrintf ("Error ID: %s\n", msgid);
    csPrintf ("Description: %s\n", (const char*)errmsg);
  }
}

void csTextSyntaxService::ReportBadToken (iDocumentNode* badtokennode)
{
  Report ("crystalspace.syntax.badtoken",
        CS_REPORTER_SEVERITY_ERROR,
  	badtokennode, "Unexpected token '%s'!",
	badtokennode->GetValue ());
}

//======== Debugging =======================================================

#define SYN_ASSERT(test,msg) \
  if (!(test)) \
  { \
    csString ss; \
    ss.Format ("Syntax services failure (%d,%s): %s\n", int(__LINE__), \
    	#msg, #test); \
    str.Append (ss); \
    return csPtr<iString> (rc); \
  }

csPtr<iString> csTextSyntaxService::Debug_UnitTest ()
{
  csRef<scfString> rc (csPtr<scfString> (new scfString ()));
  csString& str = rc->GetCsString ();

  //==========================================================================
  // Tests for XML parsing.
  //==========================================================================
  csRef<iDocumentSystem> xml (csPtr<iDocumentSystem> (
  	new csTinyDocumentSystem ()));
  csRef<iDocument> doc = xml->CreateDocument ();
  const char* error = doc->Parse ("\
    <root>\
      <v x='1' y='2' z='3'/>\
      <matrix>\
        <scale all='3'/>\
	<m13>1.5</m13>\
      </matrix>\
      <mixmode>\
        <tiling/> <alpha>.5</alpha>\
      </mixmode>\
    </root>\
  ");
  SYN_ASSERT (error == NULL, error);

  csRef<iDocumentNode> root = doc->GetRoot ()->GetNode ("root");
  csRef<iDocumentNode> vector_node = root->GetNode ("v");
  SYN_ASSERT (vector_node != NULL, "vector_node");
  csVector3 v;
  SYN_ASSERT (ParseVector (vector_node, v) == true, "");
  SYN_ASSERT (v.x == 1, "x");
  SYN_ASSERT (v.y == 2, "y");
  SYN_ASSERT (v.z == 3, "z");

  csRef<iDocumentNode> matrix_node = root->GetNode ("matrix");
  SYN_ASSERT (matrix_node != NULL, "matrix_node");
  csMatrix3 m;
  SYN_ASSERT (ParseMatrix (matrix_node, m) == true, "");
  SYN_ASSERT (m.m11 == 3, "m");
  SYN_ASSERT (m.m12 == 0, "m");
  SYN_ASSERT (m.m13 == 1.5, "m");
  SYN_ASSERT (m.m21 == 0, "m");
  SYN_ASSERT (m.m22 == 3, "m");
  SYN_ASSERT (m.m23 == 0, "m");
  SYN_ASSERT (m.m31 == 0, "m");
  SYN_ASSERT (m.m32 == 0, "m");
  SYN_ASSERT (m.m33 == 3, "m");

#ifndef CS_USE_NEW_RENDERER
  csRef<iDocumentNode> mixmode_node = root->GetNode ("mixmode");
  SYN_ASSERT (mixmode_node != NULL, "mixmode_node");
  uint mixmode;
  SYN_ASSERT (ParseMixmode (mixmode_node, mixmode) == true, "");
  uint desired_mixmode = CS_FX_TILING;
  desired_mixmode &= ~CS_FX_MASK_ALPHA;
  desired_mixmode |= CS_FX_SETALPHA (.5);
  SYN_ASSERT (mixmode == desired_mixmode, "mixmode");
#endif // CS_USE_NEW_RENDERER

  return NULL;
}
