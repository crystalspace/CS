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

#include "cssysdef.h"
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>

#include "csgeom/box.h"
#include "csgeom/math.h"
#include "csgeom/matrix3.h"
#include "csgeom/plane3.h"
#include "csgeom/transfrm.h"
#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "csgfx/gradient.h"
#include "csgfx/renderbuffer.h"
#include "csgfx/shaderexp.h"
#include "csgfx/shaderexpaccessor.h"
#include "cstool/keyval.h"
#include "cstool/rbuflock.h"
#include "cstool/vfsdirchange.h"
#include "csutil/cscolor.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/ref.h"
#include "csutil/scanstr.h"
#include "csutil/scfstr.h"
#include "csutil/util.h"

#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/portal.h"
#include "iengine/texture.h"
#include "imap/ldrctxt.h"
#include "imap/loader.h"
#include "imesh/object.h"
#include "imesh/thing.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/plugin.h"
#include "iutil/stringarray.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/texture.h"

#include "syntxldr.h"

// Only used for unit-testing. The parser does not otherwise
// depend on any specific XML parser.
#include "csutil/xmltiny.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(SyntaxService)
{

SCF_IMPLEMENT_FACTORY (csTextSyntaxService)

csTextSyntaxService::csTextSyntaxService (iBase *parent) : 
  scfImplementationType (this, parent)
{
}

csTextSyntaxService::~csTextSyntaxService ()
{
}

bool csTextSyntaxService::Initialize (iObjectRegistry* object_reg)
{
  csTextSyntaxService::object_reg = object_reg;
  reporter = csQueryRegistry<iReporter> (object_reg);

  InitTokenTable (xmltokens);

  return true;
}

bool csTextSyntaxService::ParseBoolAttribute (iDocumentNode* node,
	const char* attrname, bool& result, bool def_result, bool required)
{
  csRef<iDocumentAttribute> attr = node->GetAttribute (attrname);
  if (!attr)
    if (required)
    {
      ReportError ("crystalspace.syntax.boolean", node,
        "Boolean attribute '%s' is missing!", attrname);
      return false;
    }
    else
    {
      result = def_result;
      return true;
    }
  const char* v = attr->GetValue ();
  if (!v) { result = def_result; return true; }
  if (!strcasecmp (v, "1"))     { result = true; return true; }
  if (!strcasecmp (v, "0"))     { result = false; return true; }
  if (!strcasecmp (v, "yes"))   { result = true; return true; }
  if (!strcasecmp (v, "no"))    { result = false; return true; }
  if (!strcasecmp (v, "true"))  { result = true; return true; }
  if (!strcasecmp (v, "false")) { result = false; return true; }
  if (!strcasecmp (v, "on"))    { result = true; return true; }
  if (!strcasecmp (v, "off"))   { result = false; return true; }
  ReportError ("crystalspace.syntax.boolean", node,
    "Bad boolean value '%s' for attribute '%s'!", v, attrname);
  return false;
}

bool csTextSyntaxService::ParseBool (iDocumentNode* node, bool& result,
		bool def_result)
{
  const char* v = node->GetContentsValue ();
  if (!v) { result = def_result; return true; }
  if (!strcasecmp (v, "1"))     { result = true; return true; }
  if (!strcasecmp (v, "0"))     { result = false; return true; }
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

bool csTextSyntaxService::WriteBool (iDocumentNode* node, const char* name,
                                     bool value)
{
  if (value)
  {
    csRef<iDocumentNode> valueNode = node->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    if (!valueNode.IsValid()) return false;
    valueNode->SetValue(name);
    return true;
  }
  else
  {
    csRef<iDocumentNode> valueNode = node->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    if (!valueNode.IsValid()) return false;
    valueNode->SetValue(name);
    valueNode = valueNode->CreateNodeBefore(CS_NODE_TEXT, 0);
    if (!valueNode.IsValid()) return false;
    valueNode->SetValue("no");
    return false;
  }
}
bool csTextSyntaxService::ParsePlane (iDocumentNode* node, csPlane3 &p)
{
  p.A () = node->GetAttributeValueAsFloat ("a");
  p.B () = node->GetAttributeValueAsFloat ("b");
  p.C () = node->GetAttributeValueAsFloat ("c");
  p.D () = node->GetAttributeValueAsFloat ("d");
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_A: p.A () = child->GetContentsValueAsFloat (); break;
      case XMLTOKEN_B: p.B () = child->GetContentsValueAsFloat (); break;
      case XMLTOKEN_C: p.C () = child->GetContentsValueAsFloat (); break;
      case XMLTOKEN_D: p.D () = child->GetContentsValueAsFloat (); break;
      case XMLTOKEN_THREEPOINTS:
      {
        csVector3 v1;
        ParseVector (child->GetNode ("v1"), v1);
        
        csVector3 v2;
        ParseVector (child->GetNode ("v2"), v2);
        
        csVector3 v3;
        ParseVector (child->GetNode ("v3"), v3);

        p.Set (v1, v2, v3);
      }
      break;
      default: break;
    }
  }
  return true;
}

bool csTextSyntaxService::WritePlane (iDocumentNode* node, const csPlane3 &p)
{
  csRef<iDocumentNode> ANode = node->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  ANode->SetValue("A");
  ANode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(p.A ());

  csRef<iDocumentNode> BNode = node->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  BNode->SetValue("B");
  BNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(p.B ());

  csRef<iDocumentNode> CNode = node->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  CNode->SetValue("C");
  CNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(p.C ());

  csRef<iDocumentNode> DNode = node->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  DNode->SetValue("D");
  DNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(p.D ());

  return true;
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
      case XMLTOKEN_LOOKAT:
        {
	  csVector3 to (0, 0, 1);
	  csVector3 up (0, 1, 0);
	  
	  csRef<iDocumentNode> to_node = child->GetNode ("to");
	  if (to_node)
	  {
	    if (!ParseVector (to_node, to))
	      return false;
	  }
	  csRef<iDocumentNode> up_node = child->GetNode ("up");
	  if (up_node)
	  {
	    if (!ParseVector (up_node, up))
	      return false;
	  }
	  csReversibleTransform tr;
	  tr.LookAt (to, up);
	  m = tr.GetO2T ();
	}
	break;
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

bool csTextSyntaxService::WriteMatrix (iDocumentNode* node, const csMatrix3& m)
{
  csRef<iDocumentNode> m11Node = node->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  m11Node->SetValue("m11");
  m11Node->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(m.m11);

  csRef<iDocumentNode> m12Node = node->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  m12Node->SetValue("m12");
  m12Node->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(m.m12);

  csRef<iDocumentNode> m13Node = node->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  m13Node->SetValue("m13");
  m13Node->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(m.m13);

  csRef<iDocumentNode> m21Node = node->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  m21Node->SetValue("m21");
  m21Node->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(m.m21);

  csRef<iDocumentNode> m22Node = node->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  m22Node->SetValue("m22");
  m22Node->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(m.m22);

  csRef<iDocumentNode> m23Node = node->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  m23Node->SetValue("m23");
  m23Node->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(m.m23);

  csRef<iDocumentNode> m31Node = node->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  m31Node->SetValue("m31");
  m31Node->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(m.m31);

  csRef<iDocumentNode> m32Node = node->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  m32Node->SetValue("m32");
  m32Node->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(m.m32);

  csRef<iDocumentNode> m33Node = node->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  m33Node->SetValue("m33");
  m33Node->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(m.m33);

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

bool csTextSyntaxService::WriteBox (iDocumentNode* node, const csBox3& v)
{
  csRef<iDocumentNode> minNode = node->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  minNode->SetValue("min");
  minNode->SetAttributeAsFloat("x", v.MinX());
  minNode->SetAttributeAsFloat("y", v.MinY());
  minNode->SetAttributeAsFloat("z", v.MinZ());
  csRef<iDocumentNode> maxNode = node->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  maxNode->SetValue("max");
  maxNode->SetAttributeAsFloat("x", v.MaxX());
  maxNode->SetAttributeAsFloat("y", v.MaxY());
  maxNode->SetAttributeAsFloat("z", v.MaxZ());

  return true;
}

bool csTextSyntaxService::ParseVector (iDocumentNode* node, csVector3 &v)
{
  v.x = node->GetAttributeValueAsFloat ("x");
  v.y = node->GetAttributeValueAsFloat ("y");
  v.z = node->GetAttributeValueAsFloat ("z");
  return true;
}

bool csTextSyntaxService::WriteVector (iDocumentNode* node, const csVector3& v)
{
  node->SetAttributeAsFloat("x", v.x);
  node->SetAttributeAsFloat("y", v.y);
  node->SetAttributeAsFloat("z", v.z);
  return true;
}


bool csTextSyntaxService::ParseVector (iDocumentNode* node, csVector2 &v)
{
  v.x = node->GetAttributeValueAsFloat ("x");
  v.y = node->GetAttributeValueAsFloat ("y");
  return true;
}

bool csTextSyntaxService::WriteVector (iDocumentNode* node, const csVector2& v)
{
  node->SetAttributeAsFloat("x", v.x);
  node->SetAttributeAsFloat("y", v.y);
  return true;
}

bool csTextSyntaxService::ParseColor (iDocumentNode* node, csColor &c)
{				      
  c.red = node->GetAttributeValueAsFloat ("red");
  c.green = node->GetAttributeValueAsFloat ("green");
  c.blue = node->GetAttributeValueAsFloat ("blue");
  return true;
}

bool csTextSyntaxService::WriteColor (iDocumentNode* node, const csColor& col)
{				      
  node->SetAttributeAsFloat("red", col.red);
  node->SetAttributeAsFloat("green", col.green);
  node->SetAttributeAsFloat("blue", col.blue);
  return true;
}

bool csTextSyntaxService::ParseColor (iDocumentNode* node, csColor4 &c)
{				      
  c.red = node->GetAttributeValueAsFloat ("red");
  c.green = node->GetAttributeValueAsFloat ("green");
  c.blue = node->GetAttributeValueAsFloat ("blue");
  csRef<iDocumentAttribute> attr = node->GetAttribute ("alpha");
  if (attr.IsValid())
    c.alpha = attr->GetValueAsFloat ();
  else
    c.alpha = 1.0f;
  return true;
}

bool csTextSyntaxService::WriteColor (iDocumentNode* node, const csColor4& col)
{				      
  node->SetAttributeAsFloat("red", col.red);
  node->SetAttributeAsFloat("green", col.green);
  node->SetAttributeAsFloat("blue", col.blue);
  node->SetAttributeAsFloat("alpha", col.alpha);
  return true;
}

static bool StringToBlendFactor (const char* str, uint& blendFactor)
{
  if (!str) return false;

  struct BlendFactorStr
  {
    const char* str;
    uint blendFactor;
  };
  static const BlendFactorStr blendFactors[] =
  {
    {"dstalpha",     CS_MIXMODE_FACT_DSTALPHA},
    {"dstalpha_inv", CS_MIXMODE_FACT_DSTALPHA_INV},
    {"dstcolor",     CS_MIXMODE_FACT_DSTCOLOR},
    {"dstcolor_inv", CS_MIXMODE_FACT_DSTCOLOR_INV},
    {"one",          CS_MIXMODE_FACT_ONE},
    {"srcalpha",     CS_MIXMODE_FACT_SRCALPHA},
    {"srcalpha_inv", CS_MIXMODE_FACT_SRCALPHA_INV},
    {"srccolor",     CS_MIXMODE_FACT_SRCCOLOR},
    {"srccolor_inv", CS_MIXMODE_FACT_SRCCOLOR_INV},
    {"zero",         CS_MIXMODE_FACT_ZERO}
  };

  size_t l = 0, 
    r = sizeof (blendFactors) / sizeof (BlendFactorStr);
  while (l < r)
  {
    size_t m = (l+r) / 2;
    int i = strcmp (blendFactors[m].str, str);
    if (i == 0) 
    {
      blendFactor = blendFactors[m].blendFactor;
      return true;
    }
    if (i < 0)
      l = m + 1;
    else
      r = m;
  }
  return false;
}

bool csTextSyntaxService::ParseMixmode (iDocumentNode* node, uint &mixmode,
					bool allowFxMesh)
{
#define MIXMODE_EXCLUSIVE				\
  if (mixmodeSpecified)					\
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
    break;						\
  }                                                     \
  mixmodeSpecified = true;

  bool warned = false;
  bool mixmodeSpecified = false;
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
	MIXMODE_EXCLUSIVE mixmode = CS_FX_COPY; break;
      case XMLTOKEN_MULTIPLY: 
	MIXMODE_EXCLUSIVE mixmode = CS_FX_MULTIPLY; break;
      case XMLTOKEN_MULTIPLY2: 
	MIXMODE_EXCLUSIVE mixmode = CS_FX_MULTIPLY2; break;
      case XMLTOKEN_ADD: 
	MIXMODE_EXCLUSIVE mixmode = CS_FX_ADD; break;
      case XMLTOKEN_DESTALPHAADD:
	MIXMODE_EXCLUSIVE mixmode = CS_FX_DESTALPHAADD; break;
      case XMLTOKEN_SRCALPHAADD:
	MIXMODE_EXCLUSIVE mixmode = CS_FX_SRCALPHAADD; break;
      case XMLTOKEN_PREMULTALPHA:
	MIXMODE_EXCLUSIVE mixmode = CS_FX_PREMULTALPHA; break;
      case XMLTOKEN_ALPHA:
        {
	  mixmode &= ~CS_FX_MASK_ALPHA;
	  float alpha = child->GetContentsValueAsFloat ();
          if (mixmodeSpecified)
            mixmode |= uint (alpha * CS_FX_MASK_ALPHA);
          else
	    mixmode = CS_FX_SETALPHA (alpha);
	}
	break;
      case XMLTOKEN_TRANSPARENT: mixmode = CS_FX_TRANSPARENT; break;
      case XMLTOKEN_MESH:
	if (allowFxMesh)
	{
	  MIXMODE_EXCLUSIVE mixmode = CS_MIXMODE_TYPE_MESH; 
	  break;
	}
      case XMLTOKEN_ALPHATEST:
        {
          mixmodeSpecified = true;
          mixmode &= ~CS_MIXMODE_ALPHATEST_MASK;
          const char* mode = child->GetContentsValue();
          if (strcmp (mode, "auto") == 0)
            mixmode |= CS_MIXMODE_ALPHATEST_AUTO;
          else if (strcmp (mode, "enable") == 0)
            mixmode |= CS_MIXMODE_ALPHATEST_ENABLE;
          else if (strcmp (mode, "disable") == 0)
            mixmode |= CS_MIXMODE_ALPHATEST_DISABLE;
          else
          {
            Report ("crystalspace.syntax.mixmode",
              CS_REPORTER_SEVERITY_WARNING,
	      child, "Invalid alphatest mode %s", mode);
          }
        }
        break;
      case XMLTOKEN_BLENDOP:
        {
          mixmodeSpecified = true;
          const char* srcFactorStr = child->GetAttributeValue ("src");
          const char* dstFactorStr = child->GetAttributeValue ("dst");
          uint srcFactor = 0, dstFactor = 0;
          if (!StringToBlendFactor (srcFactorStr, srcFactor))
          {
            Report ("crystalspace.syntax.mixmode",
              CS_REPORTER_SEVERITY_WARNING,
	      child, "Invalid blend factor %s", srcFactorStr);
          }
          if (!StringToBlendFactor (dstFactorStr, dstFactor))
          {
            Report ("crystalspace.syntax.mixmode",
              CS_REPORTER_SEVERITY_WARNING,
	      child, "Invalid blend factor %s", srcFactorStr);
          }
          mixmode &= ~((CS_MIXMODE_FACT_MASK << 20) | (CS_MIXMODE_FACT_MASK << 16));
          mixmode |= ((srcFactor << 20) | (dstFactor << 16));
        }
        break;
      default:
        ReportBadToken (child);
        return false;
    }
  }
  return true;

#undef MIXMODE_EXCLUSIVE
}

static const char* BlendFactorToString (uint factor)
{
  if (factor >= CS_MIXMODE_FACT_COUNT) return 0;

  static const char* factorStrings[CS_MIXMODE_FACT_COUNT] =
  {
    "zero", "one", 
    "srccolor", "srccolor_inv", "dstcolor", "dstcolor_inv", 
    "srcalpha", "srcalpha_inv", "dstalpha", "dstalpha_inv"
  };

  return factorStrings[factor];
}

bool csTextSyntaxService::WriteMixmode (iDocumentNode* node, uint mixmode,
					bool /*allowFxMesh*/)
{
  if ((mixmode & CS_MIXMODE_TYPE_MASK) == CS_MIXMODE_TYPE_AUTO)
    node->CreateNodeBefore(CS_NODE_ELEMENT)->SetValue ("copy");
  else if ((mixmode & CS_MIXMODE_TYPE_MASK) == CS_MIXMODE_TYPE_MESH)
    node->CreateNodeBefore(CS_NODE_ELEMENT)->SetValue ("mesh");
  else
  {
    // mixmode type is "blendop"
    switch (mixmode & CS_FX_MASK_MIXMODE)
    {
      case CS_FX_TRANSPARENT:
        node->CreateNodeBefore(CS_NODE_ELEMENT)->SetValue ("transparent");
      case CS_FX_MULTIPLY: 
        node->CreateNodeBefore(CS_NODE_ELEMENT)->SetValue ("multiply");
        break;
      case CS_FX_MULTIPLY2: 
        node->CreateNodeBefore(CS_NODE_ELEMENT)->SetValue ("multipy2");
        break;
      case CS_FX_ADD: 
        node->CreateNodeBefore(CS_NODE_ELEMENT)->SetValue ("add");
        break;
      case CS_FX_DESTALPHAADD:
        node->CreateNodeBefore(CS_NODE_ELEMENT)->SetValue ("destalphaadd");
        break;
      case CS_FX_SRCALPHAADD:
        node->CreateNodeBefore(CS_NODE_ELEMENT)->SetValue ("destalphaadd");
        break;
      case CS_FX_PREMULTALPHA:
        node->CreateNodeBefore(CS_NODE_ELEMENT)->SetValue ("premultalpha");
        break;
      case CS_FX_ALPHA:
        {
          csRef<iDocumentNode> alpha = node->CreateNodeBefore(CS_NODE_ELEMENT);
          alpha->SetValue ("alpha");
          alpha->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat (
            (float) ((mixmode & CS_FX_MASK_ALPHA) / 255.0f));
        }
        break;
      default:
        {
          csRef<iDocumentNode> blendOp = node->CreateNodeBefore(CS_NODE_ELEMENT);
          blendOp->SetValue ("blendop");
          blendOp->SetAttribute ("src", 
            BlendFactorToString (CS_MIXMODE_BLENDOP_SRC (mixmode)));
          blendOp->SetAttribute ("dst", 
            BlendFactorToString (CS_MIXMODE_BLENDOP_DST (mixmode)));
        }
    }        
  }

  // Write out alphatest flag
  if ((mixmode & CS_MIXMODE_ALPHATEST_MASK) == CS_MIXMODE_ALPHATEST_ENABLE)
  {
    csRef<iDocumentNode> alphatest = node->CreateNodeBefore(CS_NODE_ELEMENT);
    alphatest->SetValue ("alphatest");
    alphatest->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValue ("enable");
  }
  else if ((mixmode & CS_MIXMODE_ALPHATEST_MASK) == CS_MIXMODE_ALPHATEST_DISABLE)
  {
    csRef<iDocumentNode> alphatest = node->CreateNodeBefore(CS_NODE_ELEMENT);
    alphatest->SetValue ("alphatest");
    alphatest->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValue ("disable");
  }

  // Write out alpha value, but only if not written above
  if (((mixmode & CS_FX_MASK_ALPHA) != 0)
    && ((mixmode & CS_FX_MASK_MIXMODE) != CS_FX_ALPHA)
    && ((mixmode & CS_MIXMODE_TYPE_MASK) == CS_MIXMODE_TYPE_BLENDOP))
  {
    csRef<iDocumentNode> alpha = node->CreateNodeBefore(CS_NODE_ELEMENT);
    alpha->SetValue ("alpha");
    alpha->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat (
      (float) ((mixmode & CS_FX_MASK_ALPHA) / 255.0f));
  }
  return true;
}

bool csTextSyntaxService::HandlePortalParameter (
	iDocumentNode* child, iLoaderContext* /*ldr_context*/,
	uint32 &flags, bool &mirror, bool &warp, int& msv,
	csMatrix3 &m, csVector3 &before, csVector3 &after,
	iString* destSector, bool& handled, bool& autoresolve)
{
  handled = true;
  const char* value = child->GetValue ();
  bool ww_given = false;
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
      if (!ww_given) after = before;
      mirror = false;
      warp = true;
      break;
    case XMLTOKEN_WW:
      ParseVector (child, after);
      ww_given = true;
      mirror = false;
      warp = true;
      break;
    case XMLTOKEN_AUTORESOLVE:
      if (!ParseBool (child, autoresolve, true))
        return false;
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

bool csTextSyntaxService::WriteGradientShade (iDocumentNode* node, 
					      const csGradientShade& shade)
{
  if (shade.left == shade.right)
  {
    csRef<iDocumentNode> color = node->CreateNodeBefore (CS_NODE_ELEMENT, 0);
    color->SetValue ("color");
    WriteColor (color, shade.left);
  }
  else
  {
    csRef<iDocumentNode> left = node->CreateNodeBefore (CS_NODE_ELEMENT, 0);
    left->SetValue ("left");
    WriteColor (left, shade.left);
    csRef<iDocumentNode> right = node->CreateNodeBefore (CS_NODE_ELEMENT, 0);
    right->SetValue ("right");
    WriteColor (right, shade.right);
  }
  csRef<iDocumentNode> pos = node->CreateNodeBefore (CS_NODE_ELEMENT, 0);
  pos->SetValue ("pos");
  pos->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValueAsFloat (shade.position);

  return true;
}

bool csTextSyntaxService::ParseGradient (iDocumentNode* node,
					 iGradient* gradient)
{
  gradient->Clear();
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
	    gradient->AddShade (shade);
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

bool csTextSyntaxService::WriteGradient (iDocumentNode* node,
					 iGradient* gradient)
{
  csRef<iGradientShades> shades = gradient->GetShades ();
  for (size_t i = 0; i < shades->GetSize(); i++)
  {
    const csGradientShade& shade = shades->Get (i);
    csRef<iDocumentNode> child = node->CreateNodeBefore (CS_NODE_ELEMENT, 0);
    child->SetValue ("shade");
    WriteGradientShade (child, shade);
  }

  return true;
}

bool csTextSyntaxService::ParseShaderVar (iDocumentNode* node,
					  csShaderVariable& var)
{
  const char *type = node->GetAttributeValue("type");
  if (!type)
  {
    Report (
      "crystalspace.syntax.shadervariable",
      CS_REPORTER_SEVERITY_WARNING,
      node,
      "Invalid shadervar type specified.");
    return false;
  }
  csStringID idtype = xmltokens.Request (type);
  switch (idtype)
  {
    case XMLTOKEN_INTEGER:
      var.SetValue (node->GetContentsValueAsInt ());
      break;
    case XMLTOKEN_FLOAT:
      var.SetValue (node->GetContentsValueAsFloat ());
      break;
    case XMLTOKEN_VECTOR2:
      {
        const char* def = node->GetContentsValue ();
        csVector2 v (0.0f, 0.0f);
        sscanf (def, "%f,%f", &v.x, &v.y);
        var.SetValue (v);
      }
      break;
    case XMLTOKEN_VECTOR3:
      {
        const char* def = node->GetContentsValue ();
        csVector3 v (0);
        sscanf (def, "%f,%f,%f", &v.x, &v.y, &v.z);
        var.SetValue (v);
      }
      break;
    case XMLTOKEN_VECTOR4:
      {
        const char* def = node->GetContentsValue ();
        csVector4 v (0);
        sscanf (def, "%f,%f,%f,%f", &v.x, &v.y, &v.z, &v.w);
        var.SetValue (v);
      }
      break;
    case XMLTOKEN_TEXTURE:
      {
	csRef<iTextureWrapper> tex;
        // @@@ This should be done in a better way...
	//  @@@ E.g. lazy retrieval of the texture with an accessor?
        csRef<iEngine> eng = CS_QUERY_REGISTRY (object_reg, iEngine);
        if (eng)
        {
          const char* texname = node->GetContentsValue ();
          tex = eng->FindTexture (texname);
          if (!tex)
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
        var.SetValue (tex);
      }
      break;
    case XMLTOKEN_EXPR:
    case XMLTOKEN_EXPRESSION:
      {
	csRef<iShaderVariableAccessor> acc = ParseShaderVarExpr (node);
	if (!acc.IsValid())
	  return false;
	var.SetType (csShaderVariable::VECTOR4);
	var.SetAccessor (acc);
      }
      break;
    case XMLTOKEN_ARRAY:
      {
        csRef<iDocumentNodeIterator> varNodes = node->GetNodes ("shadervar");

        int varCount = 0;
        while (varNodes->HasNext ()) 
        {
          varCount++;
          varNodes->Next ();
        }

        var.SetType (csShaderVariable::ARRAY);
        var.SetArraySize (varCount);

        varCount = 0;
        varNodes = node->GetNodes ("shadervar");
        while (varNodes->HasNext ()) 
        {
          csRef<iDocumentNode> varNode = varNodes->Next ();
          csRef<csShaderVariable> elementVar = 
            csPtr<csShaderVariable> (new csShaderVariable (csInvalidStringID));
          var.SetArrayElement (varCount, elementVar);
          ParseShaderVar (varNode, *elementVar);
          varCount++;
        }
      }
      break;
    default:
      Report (
        "crystalspace.syntax.shadervariable",
        CS_REPORTER_SEVERITY_WARNING,
        node,
	"Invalid shadervar type '%s'.", type);
      return false;
  }

  return true;
}

csRef<iShaderVariableAccessor> csTextSyntaxService::ParseShaderVarExpr (
  iDocumentNode* node)
{
  csRef<iDocumentNode> exprNode;
  csRef<iDocumentNodeIterator> nodeIt = node->GetNodes();
  while (nodeIt->HasNext())
  {
    csRef<iDocumentNode> child = nodeIt->Next();
    if (child->GetType() != CS_NODE_ELEMENT) continue;
    exprNode = child;
    break;
  }

  if (!exprNode)
  {
    Report ("crystalspace.syntax.shadervariable.expression",
      CS_REPORTER_SEVERITY_WARNING,
      node, "Can't find expression node");
    return 0;
  }

  csShaderExpression* expression = new csShaderExpression (object_reg);
  if (!expression->Parse (exprNode))
  {
    Report ("crystalspace.syntax.shadervariable.expression",
      CS_REPORTER_SEVERITY_WARNING,
      node, "Error parsing expression: %s", expression->GetError());
    delete expression;
    return 0;
  }
  csRef<csShaderVariable> var;
  var.AttachNew (new csShaderVariable (csInvalidStringID));
  csRef<csShaderExpressionAccessor> acc;
  acc.AttachNew (new csShaderExpressionAccessor (object_reg, expression));
  return acc;
}

bool csTextSyntaxService::WriteShaderVar (iDocumentNode* node,
					  csShaderVariable& var)
{
  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.shared.stringset", iStringSet);
  node->SetAttribute ("name", strings->Request (var.GetName ()));
  switch (var.GetType ())
  {
    case csShaderVariable::INT:
      {
        node->SetAttribute ("type", "integer");
        int val;
        var.GetValue (val);
        node->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValueAsInt (val);
      }
      break;
    case csShaderVariable::FLOAT:
      {
        node->SetAttribute ("type", "float");
        float val;
        var.GetValue (val);
        node->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValueAsFloat (val);
      }
      break;
    case csShaderVariable::VECTOR2:
      {
        node->SetAttribute ("type", "vector2");
        csString val;
        csVector2 vec;
        var.GetValue (vec);
        val.Format ("%f,%f", vec.x, vec.y);
        node->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValue (val);
      }
      break;
    case csShaderVariable::VECTOR3:
      {
        node->SetAttribute ("type", "vector3");
        csString val;
        csVector3 vec;
        var.GetValue (vec);
        val.Format ("%f,%f,%f", vec.x, vec.y, vec.z);
        node->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValue (val);
      }
      break;
    case csShaderVariable::VECTOR4:
      {
        node->SetAttribute ("type", "vector4");
        csString val;
        csVector4 vec;
        var.GetValue (vec);
        val.Format ("%f,%f,%f,%f", vec.x, vec.y, vec.z, vec.w);
        node->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValue (val);
      }
      break;
    case csShaderVariable::TEXTURE:
      {
        node->SetAttribute ("type", "texture");
        iTextureWrapper* val;
        var.GetValue (val);
        if (val)
          node->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValue (val->QueryObject ()->GetName ());
      }
      break;
    default:
      break;
  };
  return true;
}

bool csTextSyntaxService::ParseAlphaMode (iDocumentNode* node, 
					  iStringSet* strings,
					  csAlphaMode& alphaMode, 
					  bool allowAutoMode)
{
  CS_ASSERT (!allowAutoMode || (strings != 0));

#define ALPHAMODE_WARN					\
  if (modeSet)						\
  {							\
    if (!warned)					\
    {							\
      Report ("crystalspace.syntax.alphamode",		\
        CS_REPORTER_SEVERITY_WARNING,			\
	child,						\
	"Multiple alphamodes specified! "		\
	"Only first one will be used.");		\
      warned = true;					\
    }							\
  }							\
  else
  bool warned = false;
  bool modeSet = false;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_NONE:
	ALPHAMODE_WARN
	{
	  alphaMode.autoAlphaMode = false;
	  alphaMode.alphaType = csAlphaMode::alphaNone;
	  modeSet = true;
	}
	break;
      case XMLTOKEN_BINARY:
	ALPHAMODE_WARN
	{
	  alphaMode.autoAlphaMode = false;
	  alphaMode.alphaType = csAlphaMode::alphaBinary;
	  modeSet = true;
	}
	break;
      case XMLTOKEN_SMOOTH:
	ALPHAMODE_WARN
	{
	  alphaMode.autoAlphaMode = false;
	  alphaMode.alphaType = csAlphaMode::alphaSmooth;
	  modeSet = true;
	}
	break;
      case XMLTOKEN_AUTO:
	if (!allowAutoMode)
	{
	  ReportBadToken (child);
	  return false;
	}
	ALPHAMODE_WARN
	{
	  const char* def = node->GetAttributeValue("texture");

	  if (!def || (*def == 0))
	  {
	    def = CS_MATERIAL_TEXTURE_DIFFUSE;
	  }

	  alphaMode.autoAlphaMode = true;
	  alphaMode.autoModeTexture = strings->Request (def);
	  modeSet = true;
	}
	break;
      default:
        ReportBadToken (child);
        return false;
    }
  }

  if (!modeSet)
  {
    Report ("crystalspace.syntax.alphamode",		
      CS_REPORTER_SEVERITY_WARNING,			
      node, "Empty alphamode specification.");		
    return false;
  }

  return true;

#undef ALPHAMODE_WARN
}

bool csTextSyntaxService::WriteAlphaMode (iDocumentNode* node, 
					  iStringSet* strings,
					  const csAlphaMode& alphaMode)
{
  if (alphaMode.autoAlphaMode)
  {
    csRef<iDocumentNode> automode = node->CreateNodeBefore (CS_NODE_ELEMENT, 0);
    automode->SetValue ("auto");
    if (alphaMode.autoModeTexture != strings->Request (CS_MATERIAL_TEXTURE_DIFFUSE))
      automode->SetAttribute ("texture", strings->Request (alphaMode.autoModeTexture));
  }
  else if (alphaMode.alphaType == csAlphaMode::alphaSmooth)
    node->CreateNodeBefore (CS_NODE_ELEMENT, 0)->SetValue ("smooth");
  else if (alphaMode.alphaType == csAlphaMode::alphaBinary)
    node->CreateNodeBefore (CS_NODE_ELEMENT, 0)->SetValue ("binary");
  else
    node->CreateNodeBefore (CS_NODE_ELEMENT, 0)->SetValue ("none");

  return true;
}

bool csTextSyntaxService::ParseZMode (iDocumentNode* node, 
				      csZBufMode& zmode,    
				      bool allowZmesh)
{
  if (node->GetType () != CS_NODE_ELEMENT) return false;

  const char* value = node->GetValue ();
  csStringID id = xmltokens.Request (value);
  switch (id)
  {
    case XMLTOKEN_ZNONE:
      zmode = CS_ZBUF_NONE;
      break;
    case XMLTOKEN_ZFILL:
      zmode = CS_ZBUF_FILL;
      break;
    case XMLTOKEN_ZTEST:
      zmode = CS_ZBUF_TEST;
      break;
    case XMLTOKEN_ZUSE:
      zmode = CS_ZBUF_USE;
      break;
    case XMLTOKEN_ZEQUAL:
      zmode = CS_ZBUF_EQUAL;
      break;
    case XMLTOKEN_ZMESH:
      if (!allowZmesh) return false;
      zmode = CS_ZBUF_MESH;
      break;
    case XMLTOKEN_ZMESH2:
      if (!allowZmesh) return false;
      zmode = CS_ZBUF_MESH2;
      break;
    default:
      return false;
  }
  return true;
}

bool csTextSyntaxService::WriteZMode (iDocumentNode* node, 
				      csZBufMode zmode,    
				      bool allowZmesh)
{
  switch (zmode)
  {
  case CS_ZBUF_NONE:
    node->CreateNodeBefore (CS_NODE_ELEMENT, 0)->SetValue ("znone");
    break;
  case CS_ZBUF_FILL:
    node->CreateNodeBefore (CS_NODE_ELEMENT, 0)->SetValue ("zfill");
    break;
  case CS_ZBUF_TEST:
    node->CreateNodeBefore (CS_NODE_ELEMENT, 0)->SetValue ("ztest");
    break;
  case CS_ZBUF_USE:
    //node->CreateNodeBefore (CS_NODE_ELEMENT, 0)->SetValue ("zuse");
    break;
  case CS_ZBUF_EQUAL:
    node->CreateNodeBefore (CS_NODE_ELEMENT, 0)->SetValue ("zequal");
    break;
  case CS_ZBUF_MESH:
    if (allowZmesh)
      node->CreateNodeBefore (CS_NODE_ELEMENT, 0)->SetValue ("zmesh");
    break;
  case CS_ZBUF_MESH2:
    if (allowZmesh)
      node->CreateNodeBefore (CS_NODE_ELEMENT, 0)->SetValue ("zmesh2");
    break;
  default:
    break;
  };
  return true;
}

bool csTextSyntaxService::ParseKey (iDocumentNode *node, iKeyValuePair* &keyvalue)
{
  const char* name = node->GetAttributeValue ("name");
  if (!name)
  {
    ReportError ("crystalspace.syntax.key",
    	        node, "Missing 'name' attribute for 'key'!");
    return false;
  }
  csKeyValuePair* cskvp = new csKeyValuePair (name);
  csRef<iDocumentAttributeIterator> atit = node->GetAttributes ();
  while (atit->HasNext ())
  {
    csRef<iDocumentAttribute> at = atit->Next ();
    cskvp->SetValue (at->GetName (), at->GetValue ());
  }
  csRef<iKeyValuePair> kvp = SCF_QUERY_INTERFACE (cskvp, iKeyValuePair);
  
  keyvalue = kvp;
  return true;
}

bool csTextSyntaxService::WriteKey (iDocumentNode *node, iKeyValuePair *keyvalue)
{
  node->SetAttribute ("name", keyvalue->GetKey ());
  csRef<iStringArray> vnames = keyvalue->GetValueNames ();
  for (size_t i=0; i<vnames->Length (); i++)
  {
    const char* name = vnames->Get (i);
    node->SetAttribute (name, keyvalue->GetValue (name));
  }
  return true;
}

CS_IMPLEMENT_STATIC_VAR (GetBufferParseError, csString, ())

struct vhInt
{
  template <class T>
  static void Get (T& v, iDocumentNode* node, const char* attr)
  { v = node->GetAttributeValueAsInt (attr); }

  template <class T>
  static void Set (const T& v, iDocumentNode* node, const char* attr)
  { node->SetAttributeAsInt (attr, (int)v); }
};


struct vhFloat
{
  template <class T>
  static void Get (T& v, iDocumentNode* node, const char* attr)
  { v = node->GetAttributeValueAsFloat (attr); }

  template <class T>
  static void Set (const T& v, iDocumentNode* node, const char* attr)
  { node->SetAttributeAsFloat (attr, v); }  
};

template <class ValGetter>
struct BufferParser
{
  template <class T>
  static const char* Parse (iDocumentNode* node, int compNum,
    csDirtyAccessArray<T>& buf)
  {
    csString compAttrName;

    csRef<iDocumentNodeIterator> it = node->GetNodes();
    while (it->HasNext())
    {
      csRef<iDocumentNode> child = it->Next();
      if (child->GetType() != CS_NODE_ELEMENT) continue;
      if ((strcmp (child->GetValue(), "element") != 0)
	&& (strcmp (child->GetValue(), "e") != 0)
	&& (strcmp (child->GetValue(), "dongledome") != 0))
      {
	GetBufferParseError()->Format ("unexpected node '%s'", 
	  child->GetValue());
	return GetBufferParseError()->GetData();
      }
      for (int c = 0; c < compNum; c++)
      {
	compAttrName.Format ("c%d", c);
	T v;
	ValGetter::Get (v, child, compAttrName);
	buf.Push (v);
      }
    }
    return 0;
  }
};



template<typename T>
static csRef<iRenderBuffer> FillBuffer (const csDirtyAccessArray<T>& buf,
                                        csRenderBufferComponentType compType,
                                        int componentNum,
                                        bool indexBuf)
{
  csRef<iRenderBuffer> buffer;
  size_t bufElems = buf.GetSize() / componentNum;
  if (indexBuf)
  {
    T min;
    T max;
    size_t i = 0;
    size_t n = buf.GetSize(); 
    if (n & 1)
    {
      min = max = csMax (buf[0], T (0));
      i++;
    }
    else
    {
      min = T (INT_MAX);
      max = 0;
    }
    for (; i < n; i += 2)
    {
      T a = buf[i]; T b = buf[i+1];
      if (a < b)
      {
        min = csMin (min, a);
        max = csMax (max, b);
      }
      else
      {
        min = csMin (min, b);
        max = csMax (max, a);
      }
    }
    buffer = csRenderBuffer::CreateIndexRenderBuffer (bufElems, CS_BUF_STATIC,
      compType, size_t (min), size_t (max));
  }
  else
  {
    buffer = csRenderBuffer::CreateRenderBuffer (bufElems,
	  CS_BUF_STATIC, compType, (uint)componentNum);
  }
  buffer->CopyInto (buf.GetArray(), bufElems);
  return buffer;
}


template <class ValSetter>
struct BufferWriter
{
  template<class T>
  static const char* Write (iDocumentNode* node, int compNum,
    const T* buf, size_t bufferSize)
  {
    csString compAttrName;
    for (size_t i = 0; i < bufferSize / compNum; ++i)
    {
      csRef<iDocumentNode> child = node->CreateNodeBefore (CS_NODE_ELEMENT);
      child->SetValue ("e");
      for (int c = 0; c < compNum; ++c)
      {
        compAttrName.Format ("c%d", c);
        ValSetter::Set (buf[i*compNum + c], child, compAttrName);
      }
    }
    return 0;
  }
};

csRef<iRenderBuffer> csTextSyntaxService::ParseRenderBuffer (iDocumentNode* node)
{
  static const char* msgid = "crystalspace.syntax.renderbuffer";

  const char* componentType = node->GetAttributeValue ("type");
  if (componentType == 0)
  {
    ReportError (msgid, node, "no 'type' attribute");
    return 0;
  }
  int componentNum = node->GetAttributeValueAsInt ("components");
  if (componentNum <= 0)
  {
    ReportError (msgid, node, "bogus 'components' attribute: %d", componentNum);
    return 0;
  }

  bool indexBuf = false;
  {
    const char* index = node->GetAttributeValue("indices");
    if (index && *index)
    {
      indexBuf = ((strcmp (index, "yes") == 0)
		  || (strcmp (index, "true") == 0)
		  || (strcmp (index, "on") == 0));
    }
  }

  if (indexBuf && (componentNum != 1))
  {
    ReportError (msgid, node, "index buffers are required to have 1 component", componentNum);
    return 0;
  }
  
  const char* err;
  csRef<iRenderBuffer> buffer;
  if ((strcmp (componentType, "int") == 0) 
    || (strcmp (componentType, "i") == 0))
  {
    csDirtyAccessArray<int> buf;
    err = BufferParser<vhInt>::Parse (node, componentNum, buf);
    if (err == 0)
    {
      buffer = FillBuffer<int> (buf, CS_BUFCOMP_INT, componentNum, indexBuf);
    }
  }
  else if ((strcmp (componentType, "uint") == 0) 
    || (strcmp (componentType, "ui") == 0))
  {
    csDirtyAccessArray<uint> buf;
    err = BufferParser<vhInt>::Parse (node, componentNum, buf);
    if (err == 0)
    {
      buffer = FillBuffer<uint> (buf, CS_BUFCOMP_UNSIGNED_INT, componentNum, indexBuf);
    }
  }
  else if ((strcmp (componentType, "byte") == 0) 
    || (strcmp (componentType, "b") == 0))
  {
    csDirtyAccessArray<char> buf;
    err = BufferParser<vhInt>::Parse (node, componentNum, buf);
    if (err == 0)
    {
      buffer = FillBuffer<char> (buf, CS_BUFCOMP_BYTE, componentNum, indexBuf);
    }
  }
  else if ((strcmp (componentType, "ubyte") == 0) 
    || (strcmp (componentType, "ub") == 0))
  {
    csDirtyAccessArray<unsigned char> buf;
    err = BufferParser<vhInt>::Parse (node, componentNum, buf);
    if (err == 0)
    {
      buffer = FillBuffer<unsigned char> (buf, CS_BUFCOMP_UNSIGNED_BYTE, componentNum, indexBuf);
    }
  }
  else if ((strcmp (componentType, "short") == 0) 
    || (strcmp (componentType, "s") == 0))
  {
    csDirtyAccessArray<short> buf;
    err = BufferParser<vhInt>::Parse (node, componentNum, buf);
    if (err == 0)
    {
      buffer = FillBuffer<short> (buf, CS_BUFCOMP_SHORT, componentNum, indexBuf);
    }
  }
  else if ((strcmp (componentType, "ushort") == 0) 
    || (strcmp (componentType, "us") == 0))
  {
    csDirtyAccessArray<unsigned short> buf;
    err = BufferParser<vhInt>::Parse (node, componentNum, buf);
    if (err == 0)
    {
      buffer = FillBuffer<unsigned short> (buf, CS_BUFCOMP_UNSIGNED_SHORT, componentNum, indexBuf);
    }
  }
  else if ((strcmp (componentType, "float") == 0) 
    || (strcmp (componentType, "f") == 0))
  {
    csDirtyAccessArray<float> buf;
    err = BufferParser<vhFloat>::Parse (node, componentNum, buf);
    if (err == 0)
    {
      buffer = FillBuffer<float> (buf, CS_BUFCOMP_FLOAT, componentNum, indexBuf);
    }
  }
  else if ((strcmp (componentType, "double") == 0) 
    || (strcmp (componentType, "d") == 0))
  {
    csDirtyAccessArray<double> buf;
    err = BufferParser<vhFloat>::Parse (node, componentNum, buf);
    if (err == 0)
    {
      buffer = FillBuffer<double> (buf, CS_BUFCOMP_DOUBLE, componentNum, indexBuf);
    }
  }
  else
  {
    ReportError (msgid, node, "unknown value for 'type': %s", componentType);
    return 0;
  }
  if (err != 0)
  {
    ReportError (msgid, node, "%s", err);
    return 0;
  }
  return buffer;
}


bool csTextSyntaxService::WriteRenderBuffer (iDocumentNode* node, iRenderBuffer* buffer)
{
  static const char* msgid = "crystalspace.syntax.renderbuffer";

  if (buffer == 0)
  {
    ReportError (msgid, node, "no buffer specified");
    return false;
  }

  if (buffer->GetMasterBuffer () != 0)
  {
    ReportError (msgid, node, "cannot save child-buffer");
    return false;
  }

  const char* err = 0;

  // Common attribute
  int componentCount = buffer->GetComponentCount ();
  node->SetAttributeAsInt ("components", componentCount);

  switch (buffer->GetComponentType ())
  {
  case CS_BUFCOMP_BYTE:
    {
      node->SetAttribute ("type", "byte");
      csRenderBufferLock<int8> lock (buffer, CS_BUF_LOCK_READ);
      err = BufferWriter<vhInt>::Write (node, componentCount, (int8*)lock, 
        lock.GetSize ()*componentCount);
      break;
    }
  case CS_BUFCOMP_UNSIGNED_BYTE:
    {
      node->SetAttribute ("type", "ubyte");
      csRenderBufferLock<uint8> lock (buffer, CS_BUF_LOCK_READ);
      err = BufferWriter<vhInt>::Write (node, componentCount, (uint8*)lock, 
        lock.GetSize ()*componentCount);
      break;
    }

  case CS_BUFCOMP_SHORT:
    {
      node->SetAttribute ("type", "short");
      csRenderBufferLock<int16> lock (buffer, CS_BUF_LOCK_READ);
      err = BufferWriter<vhInt>::Write (node, componentCount, (int16*)lock, 
        lock.GetSize ()*componentCount);
      break;
    }
  case CS_BUFCOMP_UNSIGNED_SHORT:
    {
      node->SetAttribute ("type", "ushort");
      csRenderBufferLock<uint16> lock (buffer, CS_BUF_LOCK_READ);
      err = BufferWriter<vhInt>::Write (node, componentCount, (uint16*)lock, 
        lock.GetSize ()*componentCount);
      break;
    }
    
  case CS_BUFCOMP_INT:
    {
      node->SetAttribute ("type", "int");
      csRenderBufferLock<int32> lock (buffer, CS_BUF_LOCK_READ);
      err = BufferWriter<vhInt>::Write (node, componentCount, (int32*)lock, 
        lock.GetSize ()*componentCount);
      break;
    }
  case CS_BUFCOMP_UNSIGNED_INT:
    {
      node->SetAttribute ("type", "uint");
      csRenderBufferLock<uint32> lock (buffer, CS_BUF_LOCK_READ);
      err = BufferWriter<vhInt>::Write (node, componentCount, (uint32*)lock, 
        lock.GetSize ()*componentCount);
      break;
    }

  case CS_BUFCOMP_FLOAT:
    {
      node->SetAttribute ("type", "float");
      csRenderBufferLock<float> lock (buffer, CS_BUF_LOCK_READ);
      err = BufferWriter<vhFloat>::Write (node, componentCount, (float*)lock, 
        lock.GetSize ()*componentCount);
      break;
    }
  case CS_BUFCOMP_DOUBLE:
    {
      node->SetAttribute ("type", "double");
      csRenderBufferLock<double> lock (buffer, CS_BUF_LOCK_READ);
      err = BufferWriter<vhFloat>::Write (node, componentCount, (double*)lock, 
        lock.GetSize ()*componentCount);
      break;
    }
    
  default:
    return false;
  }

  return true;
}

csRef<iShader> csTextSyntaxService::ParseShaderRef (iDocumentNode* node)
{
  // @@@ FIXME: unify with csLoader::ParseShader()?
  static const char* msgid = "crystalspace.syntax.shaderred";

  const char* shaderName = node->GetAttributeValue ("name");
  if (shaderName == 0)
  {
    ReportError (msgid, node, "no 'name' attribute");
    return 0;
  }

  csRef<iShaderManager> shmgr = CS_QUERY_REGISTRY(object_reg, iShaderManager);
  csRef<iShader> shader = shmgr->GetShader (shaderName);
  if (shader.IsValid()) return shader;

  const char* shaderFileName = node->GetAttributeValue ("file");
  if (shaderFileName != 0)
  {
    csRef<iVFS> vfs = CS_QUERY_REGISTRY(object_reg, iVFS);
    csVfsDirectoryChanger dirChanger (vfs);
    csString filename (shaderFileName);
    csRef<iFile> shaderFile = vfs->Open (filename, VFS_FILE_READ);

    if(!shaderFile)
    {
      Report (msgid, CS_REPORTER_SEVERITY_WARNING, node,
	"Unable to open shader file '%s'!", shaderFileName);
      return 0;
    }

    csRef<iDocumentSystem> docsys =
      CS_QUERY_REGISTRY(object_reg, iDocumentSystem);
    if (docsys == 0)
      docsys.AttachNew (new csTinyDocumentSystem ());
    csRef<iDocument> shaderDoc = docsys->CreateDocument ();
    const char* err = shaderDoc->Parse (shaderFile, false);
    if (err != 0)
    {
      Report (msgid, CS_REPORTER_SEVERITY_WARNING, node,
	"Could not parse shader file '%s': %s",
	shaderFileName, err);
      return 0;
    }
    csRef<iDocumentNode> shaderNode = 
      shaderDoc->GetRoot ()->GetNode ("shader");
    dirChanger.ChangeTo (filename);

    const char* type = shaderNode->GetAttributeValue ("compiler");
    if (type == 0)
      type = shaderNode->GetAttributeValue ("type");
    if (type == 0)
    {
      ReportError (msgid, shaderNode,
	"%s: 'compiler' attribute is missing!", shaderFileName);
      return 0;
    }
    csRef<iShaderCompiler> shcom = shmgr->GetCompiler (type);
    if (!shcom.IsValid()) 
    {
      ReportError (msgid, shaderNode,
        "Could not get shader compiler '%s'", type);
      return false;
    }
    shader = shcom->CompileShader (shaderNode);
    if (shader && (strcmp (shader->QueryObject()->GetName(), shaderName) == 0))
    {
      shader->SetFileName (shaderFileName);
      shmgr->RegisterShader (shader);
    }
    else 
      return 0;
    return shader;
  }

  return 0;
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

static const char* GetDescriptiveAttribute (iDocumentNode* n,
					    const char*& attrName)
{
  static const char* descriptiveAttrs[] = {
    "name",
    "priority",
    0
  };

  const char* attr;
  const char** currentAttr = descriptiveAttrs;
  while (*currentAttr != 0)
  {
    attr = n->GetAttributeValue (*currentAttr);
    if (attr != 0)
    {
      attrName = *currentAttr;
      return attr;
    }
    currentAttr++;
  }
  return 0;
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
    const char* attrName = 0;
    const char* name = GetDescriptiveAttribute (n, attrName);
    if (name || (v && *v))
    {
      if (first) { nodepath = nodepath; first = false; }
      else nodepath = "," + nodepath;
      if (name)
      {
        nodepath = ")" + nodepath;
        nodepath = name + nodepath;
	if (attrName != 0)
	{
	  nodepath = "=" + nodepath;
	  nodepath = attrName + nodepath;
	}
        nodepath = "(" + nodepath;
      }
      if (v && *v)
      {
        nodepath = v + nodepath;
      }
    }
    n = n->GetParent ();
  }
  if (nodepath.IsEmpty())
    csReport (object_reg, severity, msgid, "%s", (const char*)errmsg);
  else
    csReport (object_reg, severity, msgid, "%s\n[node: %s]",
    	(const char*)errmsg, (const char*)nodepath);
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

csPtr<iString> csTextSyntaxService::UnitTest ()
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
        <alpha>.5</alpha>\
      </mixmode>\
    </root>\
  ", true);
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
  uint desired_mixmode = 0;
  desired_mixmode &= ~CS_FX_MASK_ALPHA;
  desired_mixmode |= CS_FX_SETALPHA (.5);
  SYN_ASSERT (mixmode == desired_mixmode, "mixmode");

  return 0;
}

}
CS_PLUGIN_NAMESPACE_END(SyntaxService)
