/*
    Copyright (C) 2001 by Jorrit Tyberghein

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
#include "csgeom/math3d.h"
#include "csgeom/matrix3.h"
#include "csgeom/transfrm.h"
#include "csutil/util.h"
#include "csutil/scanstr.h"
#include "csutil/cscolor.h"
#include "csutil/csstring.h"
#include "csutil/scfstr.h"
#include "csutil/dirtyaccessarray.h"
#include "iutil/vfs.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iengine/texture.h"
#include "iengine/material.h"
#include "iengine/portal.h"
#include "iengine/portalcontainer.h"
#include "imesh/object.h"
#include "imesh/thing.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "imap/services.h"
#include "imap/ldrctxt.h"
#include "imap/parser.h"
#include "ivaria/reporter.h"
#include "thingldr.h"
#include "csqint.h"

CS_IMPLEMENT_PLUGIN

enum
{
  XMLTOKEN_CLONE = 1,
  XMLTOKEN_COSFACT,
  XMLTOKEN_FACTORY,
  XMLTOKEN_FOG,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_REPLACEMATERIAL,
  XMLTOKEN_MIXMODE,
  XMLTOKEN_SHADING,
  XMLTOKEN_TEXMAP,
  XMLTOKEN_PORTAL,
  XMLTOKEN_VISCULL,
  XMLTOKEN_COLLDET,
  XMLTOKEN_ALPHA,
  XMLTOKEN_LIGHTING,
  XMLTOKEN_UV,
  XMLTOKEN_UVEC,
  XMLTOKEN_VVEC,
  XMLTOKEN_UVSHIFT,
  XMLTOKEN_PLANE,
  XMLTOKEN_FIRSTLEN,
  XMLTOKEN_SECONDLEN,
  XMLTOKEN_FIRST,
  XMLTOKEN_SECOND,
  XMLTOKEN_FIRSTREF,
  XMLTOKEN_SECONDREF,
  XMLTOKEN_LEN,
  XMLTOKEN_MATRIX,
  XMLTOKEN_ORIG,
  XMLTOKEN_ORIGREF,
  XMLTOKEN_MOVEABLE,
  XMLTOKEN_PART,
  XMLTOKEN_P,
  XMLTOKEN_TEXLEN,
  XMLTOKEN_VISTREE,
  XMLTOKEN_V,
  XMLTOKEN_SMOOTH,
  XMLTOKEN_RENDERBUFFER
};

SCF_IMPLEMENT_IBASE (csThingLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csThingSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csThingLoader)
SCF_IMPLEMENT_FACTORY (csThingFactoryLoader)
SCF_IMPLEMENT_FACTORY (csThingSaver)
SCF_IMPLEMENT_FACTORY (csThingFactorySaver)


#define MAXLINE 200 /* max number of chars per line... */

//---------------------------------------------------------------------------

csThingLoader::csThingLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csThingLoader::~csThingLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csThingLoader::Initialize (iObjectRegistry* object_reg)
{
  csThingLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  xmltokens.Register ("clone", XMLTOKEN_CLONE);
  xmltokens.Register ("cosfact", XMLTOKEN_COSFACT);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("fog", XMLTOKEN_FOG);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("replacematerial", XMLTOKEN_REPLACEMATERIAL);
  xmltokens.Register ("moveable", XMLTOKEN_MOVEABLE);
  xmltokens.Register ("part", XMLTOKEN_PART);
  xmltokens.Register ("p", XMLTOKEN_P);
  xmltokens.Register ("smooth", XMLTOKEN_SMOOTH);
  xmltokens.Register ("texlen", XMLTOKEN_TEXLEN);
  xmltokens.Register ("vistree", XMLTOKEN_VISTREE);
  xmltokens.Register ("v", XMLTOKEN_V);
  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("shading", XMLTOKEN_SHADING);
  xmltokens.Register ("texmap", XMLTOKEN_TEXMAP);
  xmltokens.Register ("portal", XMLTOKEN_PORTAL);
  xmltokens.Register ("viscull", XMLTOKEN_VISCULL);
  xmltokens.Register ("colldet", XMLTOKEN_COLLDET);
  xmltokens.Register ("alpha", XMLTOKEN_ALPHA);
  xmltokens.Register ("lighting", XMLTOKEN_LIGHTING);
  xmltokens.Register ("uv", XMLTOKEN_UV);
  xmltokens.Register ("uvec", XMLTOKEN_UVEC);
  xmltokens.Register ("vvec", XMLTOKEN_VVEC);
  xmltokens.Register ("uvshift", XMLTOKEN_UVSHIFT);
  xmltokens.Register ("plane", XMLTOKEN_PLANE);
  xmltokens.Register ("firstlen", XMLTOKEN_FIRSTLEN);
  xmltokens.Register ("secondlen", XMLTOKEN_SECONDLEN);
  xmltokens.Register ("first", XMLTOKEN_FIRST);
  xmltokens.Register ("second", XMLTOKEN_SECOND);
  xmltokens.Register ("firstref", XMLTOKEN_FIRSTREF);
  xmltokens.Register ("secondref", XMLTOKEN_SECONDREF);
  xmltokens.Register ("len", XMLTOKEN_LEN);
  xmltokens.Register ("matrix", XMLTOKEN_MATRIX);
  xmltokens.Register ("orig", XMLTOKEN_ORIG);
  xmltokens.Register ("origref", XMLTOKEN_ORIGREF);
  xmltokens.Register ("renderbuffer", XMLTOKEN_RENDERBUFFER);
  return true;
}

bool csThingLoader::ParseTextureMapping (
	iDocumentNode* node, const csVector3* vref, uint &texspec,
	csVector3 &tx_orig, csVector3 &tx1, csVector3 &tx2, csVector3 &len,
	csMatrix3 &tx_m, csVector3 &tx_v,
	csVector2 &uv_shift,
	int &idx1, csVector2 &uv1,
	int &idx2, csVector2 &uv2,
	int &idx3, csVector2 &uv3,
	const char *polyname)
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
        synldr->ParseMatrix (child, tx_m);
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
	synldr->ReportError ("crystalspace.thingldr.texture", child,
                "<plane> for <texmap> no longer supported! Use levtool -planes to convert map!");
        return false;
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
              synldr->ReportError ("crystalspace.thingldr.texture", child,
                "Too many <uv> nodes inside <texmap>! Only 3 allowed");
	      return false;
	  }
	  cur_uvidx++;
	}
        break;
      default:
        synldr->ReportBadToken (child);
        return false;
    }
  }

  if (texspec & CSTEX_V2)
  {
    if (!len.y)
    {
      synldr->ReportError ("crystalspace.thingldr.texture", node,
        "Bad texture specification for polygon '%s'", polyname);
      len.y = 1;
      return false;
    }
    if (!len.z)
    {
      synldr->ReportError ("crystalspace.thingldr.texture", node,
        "Bad texture specification for polygon '%s'", polyname);
      len.z = 1;
      return false;
    }
  }
  else
  {
    if (!len.y)
    {
      synldr->ReportError ("crystalspace.thingldr.texture", node,
        "Bad texture specification for polygon '%s'", polyname);
      len.y = 1;
      return false;
    }
  }

  return true;
}

class MissingSectorCallback : public iPortalCallback
{
public:
  csRef<iLoaderContext> ldr_context;
  char* sectorname;
  bool autoresolve;

  SCF_DECLARE_IBASE;
  MissingSectorCallback (iLoaderContext* ldr_context, const char* sector,
  	bool autoresolve)
  {
    SCF_CONSTRUCT_IBASE (0);
    MissingSectorCallback::ldr_context = ldr_context;
    sectorname = csStrNew (sector);
    MissingSectorCallback::autoresolve = autoresolve;
  }
  virtual ~MissingSectorCallback ()
  {
    delete[] sectorname;
    SCF_DESTRUCT_IBASE ();
  }
  
  virtual bool Traverse (iPortal* portal, iBase* /*context*/)
  {
    iSector* sector = ldr_context->FindSector (sectorname);
    if (!sector) return false;
    portal->SetSector (sector);
    // For efficiency reasons we deallocate the name here.
    if (!autoresolve)
    {
      delete[] sectorname;
      sectorname = 0;
      portal->RemoveMissingSectorCallback (this);
    }
    return true;
  }
};

SCF_IMPLEMENT_IBASE (MissingSectorCallback)
  SCF_IMPLEMENTS_INTERFACE (iPortalCallback)
SCF_IMPLEMENT_IBASE_END

bool csThingLoader::ParsePortal (
	iDocumentNode* node, iLoaderContext* ldr_context,
	uint32 &flags, bool &mirror, bool &warp, int& msv,
	csMatrix3 &m, csVector3 &before, csVector3 &after,
	iString* destSector, bool& autoresolve)
{
  destSector->Clear ();
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    //const char* value = child->GetValue ();
    //csStringID id = xmltokens.Request (value);
    bool handled;
    if (!synldr->HandlePortalParameter (child, ldr_context,
        flags, mirror, warp, msv, m, before, after, destSector, handled,
	autoresolve))
    {
      return false;
    }
    if (!handled)
    {
      synldr->ReportBadToken (child);
      return false;
    }
  }
  if (destSector->Length () == 0)
  {
    synldr->ReportError ("crystalspace.thingldr.portal", node,
      "Missing sector in portal!");
    return false;
  }

  return true;
}

struct ParsedRB
{ 
  csString name;
  csRef<iRenderBuffer> buf; 
  csRef<iDocumentNode> node;
  ParsedRB (const char* name, iRenderBuffer* b, iDocumentNode* n) : 
    name (name), buf (b), node (n) {}
};

bool csThingLoader::ParsePoly3d (
        iDocumentNode* node,
	iLoaderContext* ldr_context,
	iEngine* engine,
	float default_texlen,
	iThingFactoryState* thing_fact_state, int vt_offset,
	bool& poly_delete, iMeshWrapper* mesh)
{
  poly_delete = false;
  iMaterialWrapper* mat = 0;

  if (!thing_type)
  {
    csRef<iPluginManager> plugin_mgr =
    	CS_QUERY_REGISTRY (object_reg, iPluginManager);
    CS_ASSERT (plugin_mgr != 0);
    thing_type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	  "crystalspace.mesh.object.thing", iMeshObjectType);
    if (!thing_type)
      thing_type = CS_LOAD_PLUGIN (plugin_mgr,
    	  "crystalspace.mesh.object.thing", iMeshObjectType);
  }

  CS_ASSERT (thing_type != 0);
  csRef<iThingEnvironment> te = SCF_QUERY_INTERFACE (
  	thing_type, iThingEnvironment);

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
  csVector2 uv_shift (0, 0);

  int set_colldet = 0; // If 1 then set, if -1 then reset, else default.
  int set_viscull = 0; // If 1 then set, if -1 then reset, else default.
  csDirtyAccessArray<int> vertices_to_add;
  csArray<ParsedRB> renderbuffers;

  // For portals.
  csRef<iDocumentNode> portal_node;
  bool if_portal_delete_polygon = true;	// If portal we delete polygon.

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
        if (mat == 0)
        {
          synldr->ReportError ("crystalspace.thingldr.polygon", child,
            "Couldn't find material named '%s'!", child->GetContentsValue ());
          return false;
        }
	thing_fact_state->SetPolygonMaterial (CS_POLYRANGE_LAST, mat);
        break;
      case XMLTOKEN_LIGHTING:
        {
          bool do_lighting;
	  if (!synldr->ParseBool (child, do_lighting, true))
	    return false;
	  thing_fact_state->SetPolygonFlags (CS_POLYRANGE_LAST,
          	CS_POLY_LIGHTING, do_lighting ? CS_POLY_LIGHTING : 0);
        }
        break;
      case XMLTOKEN_VISCULL:
        {
          bool do_viscull;
	  if (!synldr->ParseBool (child, do_viscull, true))
	    return false;
	  if (do_viscull) set_viscull = 1;
	  else set_viscull = -1;
        }
        break;
      case XMLTOKEN_COLLDET:
        {
          bool do_colldet;
	  if (!synldr->ParseBool (child, do_colldet, true))
	    return false;
	  if (do_colldet) set_colldet = 1;
	  else set_colldet = -1;
        }
        break;
      case XMLTOKEN_PORTAL:
        if (!mesh)
	{
	  // If we don't have a mesh then we can't correctly define
	  // portals.
	  synldr->ReportError ("crystalspace.thingldr.polygon", child,
	    "Internal error! Mesh wrapper is missing for loading a portal!");
	  return false;
	}
        portal_node = child;
        break;
      case XMLTOKEN_TEXMAP:
	if (!ParseTextureMapping (child, thing_fact_state->GetVertices (),
		texspec, tx_orig, tx1, tx2, tx_len,
		tx_matrix, tx_vector,
		uv_shift,
		tx_uv_i1, tx_uv1,
		tx_uv_i2, tx_uv2,
		tx_uv_i3, tx_uv3,
		thing_fact_state->GetPolygonName (CS_POLYINDEX_LAST)))
	{
	  return false;
	}
        break;
      case XMLTOKEN_V:
        {
	  int vt_idx = child->GetContentsValueAsInt ();
	  bool ignore = false;
	  int cnt = vertices_to_add.Length ();
	  for (int i = 0 ; i < cnt ; i++)
	  {
	    if (vertices_to_add[i] == vt_idx+vt_offset)
	    {
	      csPrintf ("Duplicate vertex-index found! "
			"(polygon '%s') ignored ...\n",
			thing_fact_state->GetPolygonName (CS_POLYINDEX_LAST));
	      ignore = true;
	    }
	  }
	  if (!ignore)
	    vertices_to_add.Push (vt_idx+vt_offset);
        }
        break;
      case XMLTOKEN_SHADING:
	{
	  bool shading;
	  if (!synldr->ParseBool (child, shading, true))
	    return false;
	  thing_fact_state->SetPolygonTextureMappingEnabled (CS_POLYRANGE_LAST,
	  	shading);
	}
        break;
      case XMLTOKEN_ALPHA:
	synldr->ReportError (
	  "crystalspace.thingldr.polygon",
	  child, "<alpha> for polygons is no longer supported! Use <mixmode> for the entire mesh instead!");
	return false;
        break;
      case XMLTOKEN_MIXMODE:
	synldr->ReportError (
	  "crystalspace.thingldr.polygon",
	  child, "<mixmode> for polygons is no longer supported! Use <mixmode> for the entire mesh instead!");
	return false;
      case XMLTOKEN_RENDERBUFFER:
	{
	  const char *name = child->GetAttributeValue("name");
	  if ((name == 0) || (*name == 0))
	  {
	    synldr->ReportError ("crystalspace.thingldr.polygon",
	      child, "<renderbuffer>s must have names");
	    return false;
	  }

	  csRef<iRenderBuffer> rb = synldr->ParseRenderBuffer (child);
	  if (!rb.IsValid()) return false;
	  renderbuffers.Push (ParsedRB (name, rb, child));
	}
	break;
      default:
        synldr->ReportBadToken (child);
        return false;
    }
  }

  if (vertices_to_add.Length () < 3)
  {
    synldr->ReportError ("crystalspace.thingldr.polygon", node,
      "Polygon '%s' contains just %d vertices!",
      thing_fact_state->GetPolygonName (CS_POLYINDEX_LAST),
      thing_fact_state->GetPolygonVertexCount (CS_POLYINDEX_LAST));
    return false;
  }

  thing_fact_state->SetPolygonVertexIndices (CS_POLYRANGE_LAST,
  	vertices_to_add.Length (), vertices_to_add.GetArray ());

  mat = thing_fact_state->GetPolygonMaterial (CS_POLYINDEX_LAST);
  csRef<iMaterialEngine> mateng = SCF_QUERY_INTERFACE (mat->GetMaterial (),
  	iMaterialEngine);
  bool is_texture_transparent = false;
  if (mateng)
  {
    iTextureWrapper* tw = mateng->GetTextureWrapper ();
    if (tw)
    {
      int r, g, b;
      tw->GetKeyColor (r, g, b);
      if (r != -1)
        is_texture_transparent = true;
      else
      {
        iImage* im = tw->GetImageFile ();
        if (im)
        {
          is_texture_transparent = im->HasKeyColor () ||
	    (im->GetFormat () & CS_IMGFMT_ALPHA);
        }
      }
    }
  }

  if (texspec & CSTEX_UV)
  {
    int cnt = thing_fact_state->GetPolygonVertexCount (CS_POLYINDEX_LAST);
    if (tx_uv_i1 > cnt)
    {
      synldr->ReportError ("crystalspace.thingldr.polygon", node,
	  "Bad texture specification: vertex index 1 doesn't exist!");
      return false;
    }
    if (tx_uv_i2 > cnt)
    {
      synldr->ReportError ("crystalspace.thingldr.polygon", node,
	  "Bad texture specification: vertex index 2 doesn't exist!");
      return false;
    }
    if (tx_uv_i3 > cnt)
    {
      synldr->ReportError ("crystalspace.thingldr.polygon", node,
	  "Bad texture specification: vertex index 3 doesn't exist!");
      return false;
    }
    thing_fact_state->SetPolygonTextureMapping (
    	CS_POLYRANGE_LAST,
        thing_fact_state->GetPolygonVertex (CS_POLYINDEX_LAST, tx_uv_i1), tx_uv1,
        thing_fact_state->GetPolygonVertex (CS_POLYINDEX_LAST, tx_uv_i2), tx_uv2,
        thing_fact_state->GetPolygonVertex (CS_POLYINDEX_LAST, tx_uv_i3), tx_uv3);
  }
  else if (texspec & CSTEX_V1)
  {
    if (texspec & CSTEX_V2)
    {
      if ((tx1-tx_orig) < SMALL_EPSILON)
      {
        synldr->ReportError ("crystalspace.thingldr.polygon", node,
          "Bad texture specification!");
	return false;
      }
      else if ((tx2-tx_orig) < SMALL_EPSILON)
      {
        synldr->ReportError ("crystalspace.thingldr.polygon", node,
          "Bad texture specification!");
	return false;
      }
      else thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
        tx_orig, tx1, tx_len.y, tx2, tx_len.z);
    }
    else
    {
      if ((tx1-tx_orig) < SMALL_EPSILON)
      {
        synldr->ReportError ("crystalspace.thingldr.polygon", node,
          "Bad texture specification!");
	return false;
      }
      else thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      	tx_orig, tx1, tx_len.x);
    }
  }
  else if (tx_len.x)
  {
    // If a length is given (with 'LEN') we will first see if the polygon
    // is coplanar with the X, Y, or Z plane. In that case we will use
    // a standard offset. Otherwise we will just create a plane specific
    // for this case given the first two vertices.
    bool same_x = true, same_y = true, same_z = true;
    const csVector3& v = thing_fact_state->GetPolygonVertex (CS_POLYINDEX_LAST, 0);
    for (int i = 1 ; i < thing_fact_state->GetPolygonVertexCount (CS_POLYINDEX_LAST) ; i++)
    {
      const csVector3& v2 = thing_fact_state->GetPolygonVertex (CS_POLYINDEX_LAST, i);
      if (same_x && ABS (v.x-v2.x) >= SMALL_EPSILON) same_x = false;
      if (same_y && ABS (v.y-v2.y) >= SMALL_EPSILON) same_y = false;
      if (same_z && ABS (v.z-v2.z) >= SMALL_EPSILON) same_z = false;
    }
    if (same_x)
    {
      thing_fact_state->SetPolygonTextureMapping (
        CS_POLYRANGE_LAST,
      	csVector3 (v.x, 0, 0), csVector3 (v.x, 0, 1),
	tx_len.x, csVector3 (v.x, 1, 0), tx_len.x);
    }
    else if (same_y)
    {
      thing_fact_state->SetPolygonTextureMapping (
        CS_POLYRANGE_LAST,
      	csVector3 (0, v.y, 0), csVector3 (1, v.y, 0),
	tx_len.x, csVector3 (0, v.y, 1), tx_len.x);
    }
    else if (same_z)
    {
      thing_fact_state->SetPolygonTextureMapping (
      	CS_POLYRANGE_LAST,
	csVector3 (0, 0, v.z), csVector3 (1, 0, v.z),
	tx_len.x, csVector3 (0, 1, v.z), tx_len.x);
    }
    else
      thing_fact_state->SetPolygonTextureMapping (
      	CS_POLYRANGE_LAST,
	tx_len.x);
  }
  else
    thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST, tx_matrix, tx_vector);

  if (texspec & CSTEX_UV_SHIFT)
  {
    if (thing_fact_state->IsPolygonTextureMappingEnabled (CS_POLYINDEX_LAST))
    {
      thing_fact_state->GetPolygonTextureMapping (CS_POLYINDEX_LAST, tx_matrix, tx_vector);
      // T = Mot * (O - Vot)
      // T = Mot * (O - Vot) + Vuv      ; Add shift Vuv to final texture map
      // T = Mot * (O - Vot) + Mot * Mot-1 * Vuv
      // T = Mot * (O - Vot + Mot-1 * Vuv)
      csVector3 shift (uv_shift.x, uv_shift.y, 0);
      tx_vector -= tx_matrix.GetInverse () * shift;
      thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST, tx_matrix, tx_vector);
    }
  }

  if (portal_node)
  {
    csMatrix3 m_w; m_w.Identity ();
    csVector3 v_w_before (0, 0, 0);
    csVector3 v_w_after (0, 0, 0);
    uint32 flags = 0;
    bool do_warp = false;
    bool do_mirror = false;
    int msv = -1;
    scfString destSectorName;

    bool autoresolve = false;
    if (ParsePortal (portal_node, ldr_context,
	      flags, do_mirror, do_warp, msv,
	      m_w, v_w_before, v_w_after, &destSectorName, autoresolve))
    {
      iSector* destSector;
      // If autoresolve is true we clear the sector since we want the callback
      // to be used.
      if (autoresolve)
        destSector = 0;
      else
        destSector = ldr_context->FindSector (destSectorName.GetData ());
      int cnt = thing_fact_state->GetPolygonVertexCount (CS_POLYINDEX_LAST);
      csVector3* portal_verts = new csVector3[cnt];
      int i;
      for (i = 0 ; i < cnt ; i++)
        portal_verts[i] = thing_fact_state->GetPolygonVertex (
		CS_POLYINDEX_LAST, i);

      int portal_pri = engine->GetPortalRenderPriority ();
      if (portal_pri == 0)
        portal_pri = mesh->GetRenderPriority ();
      char pc_name[100];
      sprintf (pc_name, "__portals_%d_%s__", portal_pri,
      	destSectorName.GetData ());

      iPortal* portal;
      csRef<iMeshWrapper> portal_mesh = engine->CreatePortal (
      	pc_name, mesh, destSector,
      	portal_verts, cnt, portal);
      delete[] portal_verts;

      portal_mesh->SetRenderPriority (portal_pri);
      if (thing_fact_state->GetPolygonName (CS_POLYINDEX_LAST))
        portal->SetName (thing_fact_state->GetPolygonName (CS_POLYINDEX_LAST));

      if (!destSector)
      {
	MissingSectorCallback* mscb = new MissingSectorCallback (
	    	ldr_context, destSectorName.GetData (), autoresolve);
	portal->SetMissingSectorCallback (mscb);
	mscb->DecRef ();
      }

      if (is_texture_transparent)
      {
        poly_delete = false;
	if (!set_colldet)
	  set_colldet = -1;
	if (!set_viscull)
	  set_viscull = -1;
      }
      else
      {
        poly_delete = if_portal_delete_polygon;
      }

      portal->GetFlags ().Set (flags);

      if (do_mirror)
      {
        if (!set_colldet) set_colldet = 1;
        portal->SetWarp (csTransform::GetReflect (
		thing_fact_state->GetPolygonObjectPlane (CS_POLYINDEX_LAST)));
      }
      else if (do_warp)
      {
        portal->SetWarp (m_w, v_w_before, v_w_after);
      }

      if (msv != -1)
      {
        portal->SetMaximumSectorVisit (msv);
      }
    }
  }

  if (!set_viscull)
  {
    if (portal_node || is_texture_transparent)
      set_viscull = -1;
  }
  if (set_viscull == 1)
    thing_fact_state->SetPolygonFlags (CS_POLYRANGE_LAST, CS_POLY_VISCULL);
  else if (set_viscull == -1)
    thing_fact_state->ResetPolygonFlags (CS_POLYRANGE_LAST, CS_POLY_VISCULL);

  if (set_colldet == 1)
    thing_fact_state->SetPolygonFlags (CS_POLYRANGE_LAST, CS_POLY_COLLDET);
  else if (set_colldet == -1)
    thing_fact_state->ResetPolygonFlags (CS_POLYRANGE_LAST, CS_POLY_COLLDET);

  for (size_t i  = 0; i < renderbuffers.Length(); i++)
  {
    const ParsedRB& rb = renderbuffers[i];
    if (rb.buf->GetElementCount() != vertices_to_add.Length())
    {
      synldr->ReportError ("crystalspace.thingldr.polygon", rb.node,
	"Render buffer element count does not match polygon vertex count: "
	"%u != %u", (uint)rb.buf->GetElementCount(), 
	(uint)vertices_to_add.Length());
      return false;
    }
    if (!thing_fact_state->AddPolygonRenderBuffer (CS_POLYINDEX_LAST, 
      rb.name, rb.buf))
    {
      synldr->ReportError ("crystalspace.thingldr.polygon", rb.node,
	"Either a renderbuffer '%s' was already attached to the polygon "
	"or the format does not match other buffers of the same name attached "
	"to other polygons.", rb.name.GetData());
      return false;
    }
  }

  return true;
}

bool csThingLoader::LoadThingPart (iThingEnvironment* te, iDocumentNode* node,
	iLoaderContext* ldr_context,
	iObjectRegistry* object_reg, iReporter* reporter,
	iSyntaxService *synldr, ThingLoadInfo& info,
	iEngine* engine, int vt_offset, bool isParent,
	iMeshWrapper* mesh)
{
#define CHECK_TOPLEVEL(m) \
if (!isParent) { \
synldr->ReportError ("crystalspace.thingloader.parse", child, \
	"'%s' flag only for top-level thing!", m); \
return false; \
}

#define CHECK_OBJECTONLY(m) \
if (info.load_factory) { \
synldr->ReportError ("crystalspace.thingloader.parse", child, \
	"'%s' is not for factories!", m); \
return false; \
} \
if (!info.thing_state) { \
synldr->ReportError ("crystalspace.thingloader.parse", child, \
	"Factory must be given before using '%s'!", m); \
return false; \
}

#define CHECK_DONTEXTENDFACTORY \
if ((!info.load_factory) && info.global_factory) { \
synldr->ReportError ("crystalspace.thingloader.parse", child, \
	"Can't change the object when using 'factory' or 'clone'!"); \
return false; \
}

#define CREATE_FACTORY_IF_NEEDED \
if (!info.thing_fact_state) \
{ \
  csRef<iMeshObjectFactory> fact; \
  fact = info.type->NewFactory (); \
  info.thing_fact_state = SCF_QUERY_INTERFACE (fact, \
	iThingFactoryState); \
  info.obj = fact->NewInstance (); \
  info.thing_state = SCF_QUERY_INTERFACE (info.obj, \
	iThingState); \
}

  uint mixmode = CS_FX_COPY;
  csArray<int> polygons_to_delete;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_VISTREE:
	synldr->ReportError (
	    "crystalspace.thingloader.parse.vistree",
	    child, "'vistree' no longer supported! Convert your level to Dynavis using 'levtool'!\n");
	printf ("'vistree' no longer supported! Convert your level to Dynavis using 'levtool'!\n");
	return false;
      case XMLTOKEN_COSFACT:
        CHECK_TOPLEVEL("cosfact");
	CHECK_DONTEXTENDFACTORY;
	CREATE_FACTORY_IF_NEEDED;
        info.thing_fact_state->SetCosinusFactor (
		child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_MOVEABLE:
        CHECK_TOPLEVEL("moveable");
	CHECK_OBJECTONLY("moveable");
        info.thing_state->SetMovingOption (CS_THING_MOVE_OCCASIONAL);
	synldr->Report ("crystalspace.thingloader.parse",
		CS_REPORTER_SEVERITY_WARNING, child,
		"<moveable/> is deprecated and no longer needed!");
        break;
      case XMLTOKEN_FACTORY:
        CHECK_TOPLEVEL("factory");
        if (info.load_factory)
	{
	  synldr->ReportError (
	    "crystalspace.thingloader.parse.factory",
	    child, "Can't use 'factory' when parsing a factory!");
	  return false;
	}
        if (info.thing_fact_state)
	{
	  synldr->ReportError (
	    "crystalspace.thingloader.parse.factory",
	    child, "'factory' already specified!");
	  return false;
	}
	info.global_factory = true;

        {
	  const char* factname = child->GetContentsValue ();
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
          if (!fact)
          {
	    synldr->ReportError (
	      "crystalspace.thingloader.parse.factory",
              child, "Couldn't find thing factory '%s'!", factname);
            return false;
          }
	  info.thing_fact_state = SCF_QUERY_INTERFACE (
	  	fact->GetMeshObjectFactory (), iThingFactoryState);
	  if (!info.thing_fact_state)
	  {
	    synldr->ReportError (
	      "crystalspace.thingloader.parse.factory",
              child, "Factory '%s' is not a thing factory!", factname);
            return false;
	  }
	  info.obj = fact->GetMeshObjectFactory ()->NewInstance ();
	  info.thing_state = SCF_QUERY_INTERFACE (info.obj, iThingState);
        }
        break;
      case XMLTOKEN_CLONE:
        CHECK_TOPLEVEL("clone");
        if (info.load_factory)
	{
	  synldr->ReportError (
	    "crystalspace.thingloader.parse.factory",
	    child, "Parsing a factory, so can't use 'clone'!");
	  return false;
	}
        if (info.thing_fact_state)
	{
	  synldr->ReportError (
	    "crystalspace.thingloader.parse.factory",
	    child, "'factory' already specified, so can't use 'clone'!");
	  return false;
	}
	info.global_factory = true;

        {
	  const char* meshname = child->GetContentsValue ();
	  iMeshWrapper* wrap = ldr_context->FindMeshObject (meshname);
          if (!wrap)
          {
	    synldr->ReportError (
	      "crystalspace.thingloader.parse.clone",
              child, "Couldn't find thing '%s'!", meshname);
            return false;
          }

	  csRef<iThingState> other_thing_state (SCF_QUERY_INTERFACE (
	  	wrap->GetMeshObject (), iThingState));
	  if (!other_thing_state)
	  {
	    synldr->ReportError (
	      "crystalspace.thingloader.parse.clone",
              child, "Object '%s' is not a thing!", meshname);
            return false;
	  }
	  info.thing_fact_state = other_thing_state->GetFactory ();
	  info.obj = wrap->GetFactory ()->GetMeshObjectFactory ()
	  	->NewInstance ();
	  info.thing_state = SCF_QUERY_INTERFACE (info.obj, iThingState);
        }
        break;
      case XMLTOKEN_PART:
	if (!LoadThingPart (te, child, ldr_context, object_reg, reporter,
		synldr, info, engine, info.thing_fact_state
			? info.thing_fact_state->GetVertexCount () : 0,
		false, mesh))
	  return false;
        break;
      case XMLTOKEN_V:
	CHECK_DONTEXTENDFACTORY;
	CREATE_FACTORY_IF_NEEDED;
        {
	  csVector3 v;
	  if (!synldr->ParseVector (child, v))
	    return false;
          info.thing_fact_state->CreateVertex (v);
        }
        break;
      case XMLTOKEN_FOG:
	synldr->ReportError (
	      "crystalspace.thingloader.parse.fog",
      	      child, "FOG for things is currently not supported!\n\
Nag to Jorrit about this feature if you want it.");
	return false;

      case XMLTOKEN_P:
	CHECK_DONTEXTENDFACTORY;
	CREATE_FACTORY_IF_NEEDED;
        {
	  int idx = info.thing_fact_state->AddEmptyPolygon ();
	  info.thing_fact_state->SetPolygonName (CS_POLYRANGE_LAST,
			  child->GetAttributeValue ("name"));
	  if (info.default_material)
	    info.thing_fact_state->SetPolygonMaterial (CS_POLYRANGE_LAST,
	    	info.default_material);
	  bool poly_delete = false;
	  bool success = ParsePoly3d (child, ldr_context,
	  			    engine, info.default_texlen, info.thing_fact_state,
				    vt_offset, poly_delete, mesh);
	  if (!success)
	  {
	    info.thing_fact_state->RemovePolygon (idx);
	    return false;
	  }
	  if (poly_delete)
	  {
	    polygons_to_delete.Push (idx);
	  }
        }
        break;

      case XMLTOKEN_REPLACEMATERIAL:
        CHECK_TOPLEVEL("replacematerial");
        CHECK_OBJECTONLY("replacematerial");
	{
	  int idx = info.replace_materials.Push (RepMaterial ());
	  info.replace_materials[idx].oldmat = csStrNew (
		child->GetAttributeValue ("old"));
	  info.replace_materials[idx].newmat = csStrNew (
		child->GetAttributeValue ("new"));
	}
	break;

      case XMLTOKEN_MATERIAL:
	CHECK_DONTEXTENDFACTORY;
	CREATE_FACTORY_IF_NEEDED;
	{
	  const char* matname = child->GetContentsValue ();
          info.default_material = ldr_context->FindMaterial (matname);
          if (info.default_material == 0)
          {
	    synldr->ReportError (
	        "crystalspace.thingloader.parse.material",
                child, "Couldn't find material named '%s'!", matname);
            return false;
          }
	}
        break;
      case XMLTOKEN_TEXLEN:
	info.default_texlen = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_SMOOTH:
	CHECK_DONTEXTENDFACTORY;
	CREATE_FACTORY_IF_NEEDED;
	info.thing_fact_state->SetSmoothingFlag (true);
	break;

      case XMLTOKEN_MIXMODE:
        CHECK_TOPLEVEL("mixmode");
        CHECK_OBJECTONLY("mixmode");
        {
	  if (synldr->ParseMixmode (child, mixmode))
	  {
            info.thing_state->SetMixMode (mixmode);
	  }
	}
        break;

      default:
        synldr->ReportBadToken (child);
	return false;
    }
  }

  if (!info.thing_fact_state)
  {
    synldr->ReportError ("crystalspace.thingloader.loadpart",
	node, "No Vertex or face in params node found.");
    return false;
  }
 
  if (mixmode == CS_FX_COPY)
  {
    // Mixmode is copy, delete all polygons that became portals.
    int i;
    for (i = polygons_to_delete.Length ()-1 ; i >= 0 ; i--)
      info.thing_fact_state->RemovePolygon (polygons_to_delete[i]);
  }

  return true;
}

csPtr<iBase> csThingLoader::Parse (iDocumentNode* node,
			     iLoaderContext* ldr_context, iBase* context)
{
  ThingLoadInfo info;
  info.load_factory = false;
  info.global_factory = false;

  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (object_reg,
  	iPluginManager);
  info.type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.thing", iMeshObjectType);
  if (!info.type)
  {
    info.type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.thing",
    	iMeshObjectType);
  }
  if (!info.type)
  {
    synldr->ReportError (
		"crystalspace.thingloader.setup.objecttype",
		node, "Could not load the thing mesh object plugin!");
    return 0;
  }
  csRef<iThingEnvironment> te = SCF_QUERY_INTERFACE (info.type,
  	iThingEnvironment);
  csRef<iEngine> engine = CS_QUERY_REGISTRY (object_reg, iEngine);

  // It is possible that the mesh wrapper is null.
  csRef<iMeshWrapper> mesh = SCF_QUERY_INTERFACE_SAFE (context, iMeshWrapper);

  if (!LoadThingPart (te, node, ldr_context, object_reg, reporter, synldr, info,
  	engine, 0, true, mesh))
  {
    info.obj = 0;
  }
  else
  {
    if (info.thing_fact_state->GetPolygonCount () == 0)
    {
      synldr->ReportError ("crystalspace.thingloader.loadpart",
	node, "No more polygons left after converting to portals! "
	"This is not supported!");
      return 0;
    }
    size_t i;
    for (i = 0 ; i < info.replace_materials.Length () ; i++)
    {
      RepMaterial& rm = info.replace_materials[i];
      iMaterialWrapper* old_mat = ldr_context->FindMaterial (rm.oldmat);
      if (!old_mat)
      {
	synldr->ReportError (
	        "crystalspace.thingloader.parse.material",
                node, "Couldn't find material named '%s'!", rm.oldmat);
	return 0;
      }
      iMaterialWrapper* new_mat = ldr_context->FindMaterial (rm.newmat);
      if (!new_mat)
      {
	synldr->ReportError (
	        "crystalspace.thingloader.parse.material",
                node, "Couldn't find material named '%s'!", rm.newmat);
	return 0;
      }
      info.thing_state->ReplaceMaterial (old_mat, new_mat);
    }
  }

  return csPtr<iBase> (info.obj);
}

csPtr<iBase> csThingFactoryLoader::Parse (iDocumentNode* node,
			     iLoaderContext* ldr_context, iBase*)
{
  ThingLoadInfo info;
  info.load_factory = true;
  info.global_factory = false;

  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  info.type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.thing", iMeshObjectType);
  if (!info.type)
  {
    info.type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.thing",
    	iMeshObjectType);
  }
  if (!info.type)
  {
    synldr->ReportError (
		"crystalspace.thingloader.setup.objecttype",
		node, "Could not load the thing mesh object plugin!");
    return 0;
  }
  csRef<iThingEnvironment> te = SCF_QUERY_INTERFACE (info.type,
  	iThingEnvironment);
  csRef<iEngine> engine = CS_QUERY_REGISTRY (object_reg, iEngine);

  csRef<iMeshObjectFactory> fact;
  fact = info.type->NewFactory ();
  info.thing_fact_state = SCF_QUERY_INTERFACE (fact, iThingFactoryState);

  if (!LoadThingPart (te, node, ldr_context, object_reg, reporter, synldr, info,
  	engine, 0, true, 0))
  {
    fact = 0;
  }
  return csPtr<iBase> (fact);
}

//---------------------------------------------------------------------------

csThingSaver::csThingSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csThingSaver::~csThingSaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csThingSaver::Initialize (iObjectRegistry* object_reg)
{
  csThingSaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}
//TBD
bool csThingSaver::WriteDown (iBase* obj, iDocumentNode* parent)
{
  if (!parent) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");
  if (obj)
  {
    csRef<iThingState> thing = SCF_QUERY_INTERFACE (obj, iThingState);
    csRef<iMeshObject> mesh = SCF_QUERY_INTERFACE (obj, iMeshObject);
    if (!thing) return false;
    if (!mesh) return false;

    csRef<iMeshWrapper> meshwrap = SCF_QUERY_INTERFACE(mesh->GetLogicalParent(), iMeshWrapper);
    iMeshFactoryWrapper* factwrap = meshwrap->GetFactory();
    if (factwrap)
    {
      const char* factname = factwrap->QueryObject()->GetName();
      if (factname && *factname)
      {
        csRef<iDocumentNode> factNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        factNode->SetValue("factory");
        factNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValue(factname);
      }    
    }
    else
    {
      WriteFactory(thing->GetFactory(), paramsNode);
    }
    //Writedown Mixmode tag
    int mixmode = thing->GetMixMode();
    csRef<iDocumentNode> mixmodeNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    mixmodeNode->SetValue("mixmode");
    synldr->WriteMixmode(mixmodeNode, mixmode, true);
  }
  return true;
}

//TBD
bool csThingSaver::WriteFactory (iBase* obj, iDocumentNode* parent)
{
  if (!parent) return false; //you never know...
  
  iDocumentNode* paramsNode = parent;

  if (obj)
  {
    csRef<iThingFactoryState> thingfact = SCF_QUERY_INTERFACE (obj, iThingFactoryState);
    if (!thingfact) return false;

    for (int vertidx = 0; vertidx < thingfact->GetVertexCount(); vertidx++)
    {
      csRef<iDocumentNode> vertNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      vertNode->SetValue("v");
      csVector3 vertex = thingfact->GetVertex(vertidx);
      synldr->WriteVector(vertNode, &vertex);
    }  
    iMaterialWrapper* material = 0;
    for (int polyidx = 0; polyidx < thingfact->GetPolygonCount(); polyidx++)
    {
      if (material != thingfact->GetPolygonMaterial(polyidx))
      {
        material = thingfact->GetPolygonMaterial(polyidx);
        csRef<iDocumentNode> materialNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        materialNode->SetValue("material");
        const char* materialname = material->QueryObject()->GetName();
        if (materialname && *materialname)
        {
          materialNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValue(materialname);
        }
      }
      csRef<iDocumentNode> polyNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      polyNode->SetValue("p");
      const char* polyname = thingfact->GetPolygonName(polyidx);
      if (polyname && *polyname)
      {
        polyNode->SetAttribute("name", polyname);
      }
      for (int pvertidx = 0; pvertidx < thingfact->GetPolygonVertexCount(polyidx); pvertidx++)
      {
        csRef<iDocumentNode> vertNode = polyNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        vertNode->SetValue("v");
        int vertex = thingfact->GetPolygonVertexIndices(polyidx)[pvertidx];
        vertNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsInt(vertex);
      }
      if (thingfact->IsPolygonTextureMappingEnabled(polyidx))
      {
        csRef<iDocumentNode> texmapNode = polyNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        texmapNode->SetValue("texmap");
        csMatrix3 m; csVector3 v;
        thingfact->GetPolygonTextureMapping(polyidx, m, v);
        csRef<iDocumentNode> matrixNode = texmapNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        matrixNode->SetValue("matrix");
        synldr->WriteMatrix(matrixNode, &m);
        csRef<iDocumentNode> vectorNode = texmapNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        vectorNode->SetValue("v");
        synldr->WriteVector(vectorNode, &v);
      }
      //Writedown Colldet tag
      bool colldet = thingfact->GetPolygonFlags(polyidx).Check(CS_POLY_COLLDET);
      synldr->WriteBool(polyNode, "colldet", colldet, true);
 
      //Writedown Lighting tag
      bool lighting = thingfact->GetPolygonFlags(polyidx).Check(CS_POLY_LIGHTING);
      synldr->WriteBool(polyNode, "lighting", lighting, true);

      //Writedown Viscull tag
      bool viscull = thingfact->GetPolygonFlags(polyidx).Check(CS_POLY_VISCULL);
      synldr->WriteBool(polyNode, "viscull", viscull, true);
    }
    //Writedown Smooth tag
    synldr->WriteBool(paramsNode, "smooth", thingfact->GetSmoothingFlag(), false);

    //Writedown Cosfact tag
    float cosfact = thingfact->GetCosinusFactor();
    csRef<iDocumentNode> cosfactNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    cosfactNode->SetValue("cosfact");
    cosfactNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(cosfact);
  }
  return true;
}

