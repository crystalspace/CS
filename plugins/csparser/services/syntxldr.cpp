/*
    Copyright (C) 2001 by Norman Kraemer

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
#include "csgfx/gradient.h"
#include "csgfx/shadervar.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/texture.h"
#include "iengine/portal.h"
#include "ivideo/material.h"
#include "imesh/thing/thing.h"
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
  XMLTOKEN_CLIPSTRADDLING,
  XMLTOKEN_COPY,
  XMLTOKEN_MULTIPLY2,
  XMLTOKEN_MULTIPLY,
  XMLTOKEN_ADD,
  XMLTOKEN_DESTALPHAADD,
  XMLTOKEN_SRCALPHAADD,
  XMLTOKEN_ALPHA,
  XMLTOKEN_TRANSPARENT,
  XMLTOKEN_KEYCOLOR,
  XMLTOKEN_TILING,
  XMLTOKEN_NONE,
  XMLTOKEN_LIGHTMAP,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_LIGHTING,
  XMLTOKEN_PORTAL,
  XMLTOKEN_WARP,
  XMLTOKEN_COLOR,
  XMLTOKEN_COLORS,
  XMLTOKEN_COLLDET,
  XMLTOKEN_INTEGER,
  XMLTOKEN_MAXVISIT,
  XMLTOKEN_MIXMODE,
  XMLTOKEN_WV,
  XMLTOKEN_MATRIX,
  XMLTOKEN_VISCULL,
  XMLTOKEN_WW,
  XMLTOKEN_MIRROR,
  XMLTOKEN_STATIC,
  XMLTOKEN_STRING,
  XMLTOKEN_ZFILL,
  XMLTOKEN_FLOAT,
  XMLTOKEN_CLIP,
  XMLTOKEN_VECTOR2,
  XMLTOKEN_VECTOR3,
  XMLTOKEN_VECTOR4,
  XMLTOKEN_SECTOR,
  XMLTOKEN_TEXTURE,
  // gradients
  XMLTOKEN_SHADE,
  XMLTOKEN_LEFT,
  XMLTOKEN_RIGHT,
  //XMLTOKEN_COLOR,
  XMLTOKEN_POS,
};

csTextSyntaxService::csTextSyntaxService (iBase *parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiDebugHelper);
}

csTextSyntaxService::~csTextSyntaxService ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiDebugHelper);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
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
  xmltokens.Register ("clipstraddling", XMLTOKEN_CLIPSTRADDLING);
  xmltokens.Register ("copy", XMLTOKEN_COPY);
  xmltokens.Register ("multiply2", XMLTOKEN_MULTIPLY2);
  xmltokens.Register ("multiply", XMLTOKEN_MULTIPLY);
  xmltokens.Register ("add", XMLTOKEN_ADD);
  xmltokens.Register ("destalphaadd", XMLTOKEN_DESTALPHAADD);
  xmltokens.Register ("srcalphaadd", XMLTOKEN_SRCALPHAADD);
  xmltokens.Register ("alpha", XMLTOKEN_ALPHA);
  xmltokens.Register ("transparent", XMLTOKEN_TRANSPARENT);
  xmltokens.Register ("keycolor", XMLTOKEN_KEYCOLOR);
  xmltokens.Register ("tiling", XMLTOKEN_TILING);
  xmltokens.Register ("none", XMLTOKEN_NONE);
  xmltokens.Register ("lightmap", XMLTOKEN_LIGHTMAP);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("lighting", XMLTOKEN_LIGHTING);
  xmltokens.Register ("portal", XMLTOKEN_PORTAL);
  xmltokens.Register ("warp", XMLTOKEN_WARP);
  xmltokens.Register ("color", XMLTOKEN_COLOR);
  xmltokens.Register ("colors", XMLTOKEN_COLORS);
  xmltokens.Register ("colldet", XMLTOKEN_COLLDET);
  xmltokens.Register ("maxvisit", XMLTOKEN_MAXVISIT);
  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("wv", XMLTOKEN_WV);
  xmltokens.Register ("matrix", XMLTOKEN_MATRIX);
  xmltokens.Register ("viscull", XMLTOKEN_VISCULL);
  xmltokens.Register ("ww", XMLTOKEN_WW);
  xmltokens.Register ("mirror", XMLTOKEN_MIRROR);
  xmltokens.Register ("static", XMLTOKEN_STATIC);
  xmltokens.Register ("zfill", XMLTOKEN_ZFILL);
  xmltokens.Register ("float", XMLTOKEN_FLOAT);
  xmltokens.Register ("clip", XMLTOKEN_CLIP);
  xmltokens.Register ("sector", XMLTOKEN_SECTOR);
  
  xmltokens.Register ("shade", XMLTOKEN_SHADE);
  xmltokens.Register ("left", XMLTOKEN_LEFT);
  xmltokens.Register ("right", XMLTOKEN_RIGHT);
  xmltokens.Register ("pos", XMLTOKEN_POS);

  xmltokens.Register ("integer", XMLTOKEN_INTEGER);
  xmltokens.Register ("string", XMLTOKEN_STRING);
  xmltokens.Register ("vector2", XMLTOKEN_VECTOR2);
  xmltokens.Register ("vector3", XMLTOKEN_VECTOR3);
  xmltokens.Register ("vector4", XMLTOKEN_VECTOR4);
  xmltokens.Register ("texture", XMLTOKEN_TEXTURE);
  return true;
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
      case XMLTOKEN_DESTALPHAADD:
	MIXMODE_EXCLUSIVE mixmode |= CS_FX_DESTALPHAADD; break;
      case XMLTOKEN_SRCALPHAADD:
	MIXMODE_EXCLUSIVE mixmode |= CS_FX_SRCALPHAADD; break;
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
  return true;

#undef MIXMODE_EXCLUSIVE
}

bool csTextSyntaxService::HandlePortalParameter (
	iDocumentNode* child, iLoaderContext* ldr_context,
	uint32 &flags, bool &mirror, bool &warp, int& msv,
	csMatrix3 &m, csVector3 &before, csVector3 &after,
	iString* destSector, bool& handled)
{
  handled = true;
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
    case XMLTOKEN_WV:
      ParseVector (child, before);
      after = before;
      mirror = false;
      warp = true;
      break;
    case XMLTOKEN_WW:
      ParseVector (child, after);
      mirror = false;
      warp = true;
      break;
    case XMLTOKEN_MIRROR:
      if (!ParseBool (child, mirror, true))
        return false;
      break;
    case XMLTOKEN_CLIPSTRADDLING:
      flags |= CS_PORTAL_CLIPSTRADDLING;
      break;
    case XMLTOKEN_COLLDET:
      flags |= CS_PORTAL_COLLDET;
      break;
    case XMLTOKEN_VISCULL:
      flags |= CS_PORTAL_VISCULL;
      break;
    case XMLTOKEN_STATIC:
      flags |= CS_PORTAL_STATICDEST;
      break;
    case XMLTOKEN_FLOAT:
      flags |= CS_PORTAL_FLOAT;
      break;
    case XMLTOKEN_ZFILL:
      flags |= CS_PORTAL_ZFILL;
      break;
    case XMLTOKEN_CLIP:
      flags |= CS_PORTAL_CLIPDEST;
      break;
    case XMLTOKEN_SECTOR:
      destSector->Append (child->GetContentsValue ());
      break;
    default:
      handled = false;
      return true;
  }

  return true;
}

bool csTextSyntaxService::ParseGradientShade (iDocumentNode* node, 
					      csGradientShade& shade)
{
  bool has_left = false;
  bool has_right = false;
  bool has_color = false;
  bool has_position = false;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_COLOR:
	{
	  if (has_left)
	  {
	    Report (
	      "crystalspace.syntax.gradient.shade",
	      CS_REPORTER_SEVERITY_WARNING,
	      child,
	      "'color' overrides previously specified 'left'.");
	  }
	  else if (has_right)
	  {
	    Report (
	      "crystalspace.syntax.gradient.shade",
	      CS_REPORTER_SEVERITY_WARNING,
	      child,
	      "'color' overrides previously specified 'right'.");
	  }
	  else if (has_color)
	  {
	    Report (
	      "crystalspace.syntax.gradient.shade",
	      CS_REPORTER_SEVERITY_WARNING,
	      child,
	      "'color' overrides previously specified 'color'.");
	  }
	  csColor c;
	  if (!ParseColor (child, c))
	  {
	    return false;
	  }
	  else
	  {
	    shade.left = c;
	    shade.right = c;
	    has_color = true;
	  }
	}
	break;
      case XMLTOKEN_LEFT:
	{
	  if (has_color)
	  {
	    Report (
	      "crystalspace.syntax.gradient.shade",
	      CS_REPORTER_SEVERITY_WARNING,
	      child,
	      "'left' overrides previously specified 'color'.");
	  }
	  if (!ParseColor (child, shade.left))
	  {
	    return false;
	  }
	  else
	  {
	    has_left = true;
	  }
	}
	break;
      case XMLTOKEN_RIGHT:
	{
	  if (has_color)
	  {
	    Report (
	      "crystalspace.syntax.gradient.shade",
	      CS_REPORTER_SEVERITY_WARNING,
	      child,
	      "'right' overrides previously specified 'color'.");
	  }
	  if (!ParseColor (child, shade.right))
	  {
	    return false;
	  }
	  else
	  {
	    has_right = true;
	  }
	}
	break;
      case XMLTOKEN_POS:
	shade.position = child->GetContentsValueAsFloat ();
	has_position = true;
	break;
      default:
        ReportBadToken (child);
        return false;
    }
  }

  if (!has_color && ((!has_left && has_right) || (has_left && !has_right)))
  {
    Report (
      "crystalspace.syntax.gradient.shade",
      CS_REPORTER_SEVERITY_WARNING,
      node,
      "Only one of 'left' or 'right' specified.");
  }
  if (!has_color && !has_left && !has_right)
  {
    Report (
      "crystalspace.syntax.gradient.shade",
      CS_REPORTER_SEVERITY_WARNING,
      node,
      "No color at all specified.");
  }
  if (!has_position)
  {
    Report (
      "crystalspace.syntax.gradient.shade",
      CS_REPORTER_SEVERITY_WARNING,
      node,
      "No position specified.");
  }

  return true;
}

bool csTextSyntaxService::ParseGradient (iDocumentNode* node,
					 csGradient& gradient)
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
      case XMLTOKEN_SHADE:
	{
	  csGradientShade shade;
	  if (!ParseGradientShade (child, shade))
	  {
	    return false;
	  }
	  else
	  {
	    gradient.AddShade (shade);
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

bool csTextSyntaxService::ParseShaderParam (iDocumentNode* node,
                       csShaderVariable* var)
{
  const char *type = node->GetAttributeValue("type");
  if (!type)
    return false;
  csStringID idtype = xmltokens.Request (type);
  switch (idtype)
  {
    case XMLTOKEN_INTEGER:
      var->SetType (csShaderVariable::INT);
      var->SetValue (node->GetContentsValueAsInt ());
      break;
    case XMLTOKEN_FLOAT:
      var->SetType (csShaderVariable::FLOAT);
      var->SetValue (node->GetContentsValueAsFloat ());
      break;
    case XMLTOKEN_STRING:
      var->SetType (csShaderVariable::STRING);
      var->SetValue (new scfString(node->GetContentsValue ()));
      break;
    case XMLTOKEN_VECTOR2:
      var->SetType (csShaderVariable::VECTOR2);
      {
        const char* def = node->GetContentsValue ();
        csVector2 v;
        sscanf (def, "%f,%f", &v.x, &v.y);
        var->SetValue (v);
      }
      break;
    case XMLTOKEN_VECTOR3:
      var->SetType (csShaderVariable::VECTOR3);
      {
        const char* def = node->GetContentsValue ();
        csVector3 v;
        sscanf (def, "%f,%f,%f", &v.x, &v.y, &v.z);
        var->SetValue (v);
      }
      break;
    case XMLTOKEN_VECTOR4:
      var->SetType (csShaderVariable::VECTOR4);
      {
        const char* def = node->GetContentsValue ();
        csVector4 v;
        sscanf (def, "%f,%f,%f,%f", &v.x, &v.y, &v.z, &v.w);
        var->SetValue (v);
      }
      break;
    case XMLTOKEN_TEXTURE:
      var->SetType (csShaderVariable::TEXTURE);
      {
        // @@@ This should be done in a better way...
        csRef<iEngine> eng = CS_QUERY_REGISTRY (object_reg, iEngine);
        if (eng)
        {
          const char* texname = node->GetContentsValue ();
          csRef<iTextureWrapper> tex = eng->FindTexture (texname);
          if (tex)
          {
            var->SetValue (tex);
          }
	  else
	  {
            Report (
              "crystalspace.syntax.shadervariable",
              CS_REPORTER_SEVERITY_WARNING,
              node,
              "Texture '%s' not found.", texname);
          }
        }
        else
        {
          Report (
            "crystalspace.syntax.shadervariable",
            CS_REPORTER_SEVERITY_WARNING,
            node,
            "Engine not found.");
        }
      }
      break;
  }

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
  SYN_ASSERT (error == 0, error);

  csRef<iDocumentNode> root = doc->GetRoot ()->GetNode ("root");
  csRef<iDocumentNode> vector_node = root->GetNode ("v");
  SYN_ASSERT (vector_node != 0, "vector_node");
  csVector3 v;
  SYN_ASSERT (ParseVector (vector_node, v) == true, "");
  SYN_ASSERT (v.x == 1, "x");
  SYN_ASSERT (v.y == 2, "y");
  SYN_ASSERT (v.z == 3, "z");

  csRef<iDocumentNode> matrix_node = root->GetNode ("matrix");
  SYN_ASSERT (matrix_node != 0, "matrix_node");
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

  csRef<iDocumentNode> mixmode_node = root->GetNode ("mixmode");
  SYN_ASSERT (mixmode_node != 0, "mixmode_node");
  uint mixmode;
  SYN_ASSERT (ParseMixmode (mixmode_node, mixmode) == true, "");
  uint desired_mixmode = CS_FX_TILING;
  desired_mixmode &= ~CS_FX_MASK_ALPHA;
  desired_mixmode |= CS_FX_SETALPHA (.5);
  SYN_ASSERT (mixmode == desired_mixmode, "mixmode");

  return 0;
}
