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
#include "imesh/thing/thing.h"
#include "imesh/thing/polygon.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "imap/services.h"
#include "imap/ldrctxt.h"
#include "imap/parser.h"
#include "ivaria/reporter.h"
#include "thingldr.h"
#include "qint.h"

CS_IMPLEMENT_PLUGIN

enum
{
  XMLTOKEN_CLONE = 1,
  XMLTOKEN_COSFACT,
  XMLTOKEN_FACTORY,
  XMLTOKEN_FASTMESH,
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
  XMLTOKEN_SMOOTH
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


#define MAXLINE 200 /* max number of chars per line... */

//---------------------------------------------------------------------------

csThingLoader::csThingLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csThingLoader::~csThingLoader ()
{
}

bool csThingLoader::Initialize (iObjectRegistry* object_reg)
{
  csThingLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  xmltokens.Register ("clone", XMLTOKEN_CLONE);
  xmltokens.Register ("cosfact", XMLTOKEN_COSFACT);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("fastmesh", XMLTOKEN_FASTMESH);
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
  return true;
}

void csThingLoader::OptimizePolygon (iPolygon3DStatic *p)
{
  if (!p->GetPortal () || p->GetAlpha () || !p->IsTextureMappingEnabled () ||
  	p->GetMixMode () != 0)
    return;
  iMaterialWrapper *mat = p->GetMaterial ();
  if (mat)
  {
    iMaterial *m = mat->GetMaterial ();
    iTextureHandle *th = m ? m->GetTexture () : 0;
    if (th && th->GetKeyColor ())
      return;
  }

  p->EnableTextureMapping (false);
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
	synldr->ReportError ("crystalspace.syntax.texture", child,
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
              synldr->ReportError ("crystalspace.syntax.texture", child,
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
      synldr->ReportError ("crystalspace.syntax.texture", node,
        "Bad texture specification for polygon '%s'", polyname);
      len.y = 1;
      return false;
    }
    if (!len.z)
    {
      synldr->ReportError ("crystalspace.syntax.texture", node,
        "Bad texture specification for polygon '%s'", polyname);
      len.z = 1;
      return false;
    }
  }
  else
  {
    if (!len.y)
    {
      synldr->ReportError ("crystalspace.syntax.texture", node,
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

  SCF_DECLARE_IBASE;
  MissingSectorCallback (iLoaderContext* ldr_context, const char* sector)
  {
    SCF_CONSTRUCT_IBASE (0);
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
    sectorname = 0;
    portal->RemoveMissingSectorCallback (this);
    return true;
  }
};

SCF_IMPLEMENT_IBASE (MissingSectorCallback)
  SCF_IMPLEMENTS_INTERFACE (iPortalCallback)
SCF_IMPLEMENT_IBASE_END

bool csThingLoader::ParsePoly3d (
        iDocumentNode* node,
	iLoaderContext* ldr_context,
	iEngine* engine, iPolygon3DStatic* poly3d,
	float default_texlen,
	iThingFactoryState* thing_fact_state, int vt_offset,
	bool& poly_delete, iMeshWrapper* mesh)
{
  poly_delete = false;
  iMaterialWrapper* mat = 0;

  if (!thing_type)
  {
    csRef<iPluginManager> plugin_mgr (
    	CS_QUERY_REGISTRY (object_reg, iPluginManager));
    CS_ASSERT (plugin_mgr != 0);
    thing_type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	  "crystalspace.mesh.object.thing", iMeshObjectType);
    if (!thing_type)
      thing_type = CS_LOAD_PLUGIN (plugin_mgr,
    	  "crystalspace.mesh.object.thing", iMeshObjectType);
  }

  CS_ASSERT (thing_type != 0);
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
  csVector2 uv_shift (0, 0);

  int set_colldet = 0; // If 1 then set, if -1 then reset, else default.
  int set_viscull = 0; // If 1 then set, if -1 then reset, else default.

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
          synldr->ReportError ("crystalspace.syntax.polygon", child,
            "Couldn't find material named '%s'!", child->GetContentsValue ());
          return false;
        }
        poly3d->SetMaterial (mat);
        break;
      case XMLTOKEN_LIGHTING:
        {
          bool do_lighting;
	  if (!synldr->ParseBool (child, do_lighting, true))
	    return false;
          poly3d->GetFlags ().Set (CS_POLY_LIGHTING,
	  	do_lighting ? CS_POLY_LIGHTING : 0);
        }
        break;
      case XMLTOKEN_ALPHA:
        poly3d->SetAlpha (child->GetContentsValueAsInt () * 655 / 256);
	// Disable vis culling for alpha objects.
	if (!set_viscull) set_viscull = -1;
	if_portal_delete_polygon = false;
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
	  synldr->ReportError ("crystalspace.syntax.polygon", child,
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
		poly3d->GetName ()))
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
			poly3d->GetName ());
	      ignore = true;
	    }
	  }
	  if (!ignore)
	    poly3d->CreateVertex (vt_idx+vt_offset);
        }
        break;
      case XMLTOKEN_SHADING:
	{
	  bool shading;
	  if (!synldr->ParseBool (child, shading, true))
	    return false;
	  poly3d->EnableTextureMapping (shading);
	}
        break;
      case XMLTOKEN_MIXMODE:
        {
          uint mixmode;
	  if (synldr->ParseMixmode (child, mixmode))
	  {
	    if (poly3d->IsTextureMappingEnabled ())
	    {
              poly3d->SetMixMode (mixmode);
	      if (mixmode & CS_FX_MASK_ALPHA)
	        poly3d->SetAlpha (mixmode & CS_FX_MASK_ALPHA);
	      if_portal_delete_polygon = false;
	    }
	  }
	}
        break;
      default:
        synldr->ReportBadToken (child);
        return false;
    }
  }

  if (poly3d->GetVertexCount () < 3)
  {
    synldr->ReportError ("crystalspace.syntax.polygon", node,
      "Polygon '%s' contains just %d vertices!",
      poly3d->GetName(),
      poly3d->GetVertexCount ());
    return false;
  }

  if (!set_viscull)
  {
    if (portal_node)
    {
      set_viscull = -1;
    }
    else
    {
      mat = poly3d->GetMaterial ();
      csRef<iMaterialEngine> mateng = SCF_QUERY_INTERFACE (mat,
      	iMaterialEngine);
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
  }

  if (set_viscull == 1)
    poly3d->GetFlags ().Set (CS_POLY_VISCULL);
  else if (set_viscull == -1)
    poly3d->GetFlags ().Reset (CS_POLY_VISCULL);

  if (texspec & CSTEX_UV)
  {
    if (tx_uv_i1 > poly3d->GetVertexCount())
    {
        synldr->ReportError ("crystalspace.syntax.polygon", node,
	  "Bad texture specification: vertex index 1 doesn't exist!");
	return false;
    }
    if (tx_uv_i2 > poly3d->GetVertexCount())
    {
        synldr->ReportError ("crystalspace.syntax.polygon", node,
	  "Bad texture specification: vertex index 2 doesn't exist!");
	return false;
    }
    if (tx_uv_i3 > poly3d->GetVertexCount())
    {
        synldr->ReportError ("crystalspace.syntax.polygon", node,
	  "Bad texture specification: vertex index 3 doesn't exist!");
	return false;
    }
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
        synldr->ReportError ("crystalspace.syntax.polygon", node,
          "Bad texture specification!");
	return false;
      }
      else if ((tx2-tx_orig) < SMALL_EPSILON)
      {
        synldr->ReportError ("crystalspace.syntax.polygon", node,
          "Bad texture specification!");
	return false;
      }
      else poly3d->SetTextureSpace (tx_orig, tx1, tx_len.y, tx2, tx_len.z);
    }
    else
    {
      if ((tx1-tx_orig) < SMALL_EPSILON)
      {
        synldr->ReportError ("crystalspace.syntax.polygon", node,
          "Bad texture specification!");
	return false;
      }
      else poly3d->SetTextureSpace (tx_orig, tx1, tx_len.x);
    }
  }
  else if (tx_len.x)
  {
    // If a length is given (with 'LEN') we will first see if the polygon
    // is coplanar with the X, Y, or Z plane. In that case we will use
    // a standard offset. Otherwise we will just create a plane specific
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
      poly3d->SetTextureSpace (csVector3 (v.x, 0, 0), csVector3 (v.x, 0, 1),
			     tx_len.x, csVector3 (v.x, 1, 0), tx_len.x);
    }
    else if (same_y)
    {
      poly3d->SetTextureSpace (csVector3 (0, v.y, 0), csVector3 (1, v.y, 0),
			     tx_len.x, csVector3 (0, v.y, 1), tx_len.x);
    }
    else if (same_z)
    {
      poly3d->SetTextureSpace (csVector3 (0, 0, v.z), csVector3 (1, 0, v.z),
			     tx_len.x, csVector3 (0, 1, v.z), tx_len.x);
    }
    else
      poly3d->SetTextureSpace (poly3d->GetVertex (0), poly3d->GetVertex (1),
			       tx_len.x);
  }
  else
    poly3d->SetTextureSpace (tx_matrix, tx_vector);

  if (texspec & CSTEX_UV_SHIFT)
  {
    if (poly3d->IsTextureMappingEnabled ())
    {
      poly3d->GetTextureSpace (tx_matrix, tx_vector);
      // T = Mot * (O - Vot)
      // T = Mot * (O - Vot) + Vuv      ; Add shift Vuv to final texture map
      // T = Mot * (O - Vot) + Mot * Mot-1 * Vuv
      // T = Mot * (O - Vot + Mot-1 * Vuv)
      csVector3 shift (uv_shift.x, uv_shift.y, 0);
      tx_vector -= tx_matrix.GetInverse () * shift;
      poly3d->SetTextureSpace (tx_matrix, tx_vector);
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

    if (synldr->ParsePortal (portal_node, ldr_context,
	      flags, do_mirror, do_warp, msv,
	      m_w, v_w_before, v_w_after, &destSectorName))
    {
      iSector* destSector = ldr_context->FindSector (destSectorName.GetData ());
#if 0
// @@@ Works more or less but not fully!
      csVector3* portal_verts = new csVector3[poly3d->GetVertexCount ()];
      int i;
      for (i = 0 ; i < poly3d->GetVertexCount () ; i++)
        portal_verts[i] = poly3d->GetVertex (i);
      csRef<iMeshWrapper> portal_mesh = engine->CreatePortal (mesh, destSector,
      	portal_verts, poly3d->GetVertexCount ());
      delete[] portal_verts;

      csRef<iPortalContainer> pc = SCF_QUERY_INTERFACE (
      	portal_mesh->GetMeshObject (), iPortalContainer);
      CS_ASSERT (pc != 0);
      iPortal* portal = pc->GetPortal (0);
      if (!destSector)
      {
	MissingSectorCallback* mscb = new MissingSectorCallback (
	    	ldr_context, destSectorName.GetData ());
	portal->SetMissingSectorCallback (mscb);
	mscb->DecRef ();
      }

      poly_delete = if_portal_delete_polygon;
#else
      iPortal* portal;
      if (destSector)
      {
	portal = poly3d->CreatePortal (destSector);
      }
      else
      {
	poly3d->CreateNullPortal ();
	portal = poly3d->GetPortal ();
	MissingSectorCallback* mscb = new MissingSectorCallback (
	    	ldr_context, destSectorName.GetData ());
	portal->SetMissingSectorCallback (mscb);
	mscb->DecRef ();
      }
#endif

      portal->GetFlags ().Set (flags);

      if (do_mirror)
      {
        if (!set_colldet) set_colldet = 1;
        portal->SetWarp (csTransform::GetReflect (poly3d->GetObjectPlane ()));
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

  if (set_colldet == 1)
    poly3d->GetFlags ().Set (CS_POLY_COLLDET);
  else if (set_colldet == -1)
    poly3d->GetFlags ().Reset (CS_POLY_COLLDET);

  OptimizePolygon (poly3d);

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
      case XMLTOKEN_FASTMESH:
        CHECK_TOPLEVEL("fastmesh");
	CHECK_DONTEXTENDFACTORY;
	CREATE_FACTORY_IF_NEEDED;
        info.thing_fact_state->GetFlags ().Set (CS_THING_FASTMESH);
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
              child, "Object '%s' is not a thing factory!", factname);
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
	  iPolygon3DStatic* poly3d = info.thing_fact_state->CreatePolygon (
			  child->GetAttributeValue ("name"));
	  if (info.default_material)
	    poly3d->SetMaterial (info.default_material);
	  bool poly_delete = false;
	  bool success = ParsePoly3d (child, ldr_context,
	  			    engine, poly3d,
				    info.default_texlen, info.thing_fact_state,
				    vt_offset, poly_delete, mesh);
	  if (poly_delete || !success)
	  {
	    info.thing_fact_state->RemovePolygon (
	      info.thing_fact_state->FindPolygonIndex (poly3d));
	  }
	  if (!success) return false;
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
      default:
        synldr->ReportBadToken (child);
	return false;
    }
  }

  if (!info.thing_fact_state) {
    synldr->ReportError ("crystalspace.thingloader.loadpart",
	node, "No Vertex or face in params node found.");
    return false;
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
  csRef<iMeshWrapper> mesh = SCF_QUERY_INTERFACE (context, iMeshWrapper);

  if (!LoadThingPart (te, node, ldr_context, object_reg, reporter, synldr, info,
  	engine, 0, true, mesh))
  {
    info.obj = 0;
  }
  else
  {
    int i;
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
}

bool csThingSaver::Initialize (iObjectRegistry* object_reg)
{
  csThingSaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

void csThingSaver::WriteDown (iBase* /*obj*/, iFile *file)
{
  csString str;
  csRef<iFactory> fact (SCF_QUERY_INTERFACE (this, iFactory));
  char buf[MAXLINE];
  char name[MAXLINE];
  csFindReplace (name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf (buf, "FACTORY ('%s')\n", name);
  str.Append (buf);
  file->Write ((const char*)str, str.Length ());
}

