/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
    Copyright (C) 1998-2000 by Ivan Avramovic <ivan@avramovic.com>

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

#include "csgeom/box.h"
#include "csgeom/poly3d.h"
#include "csgeom/trimesh.h"
#include "cstool/mapnode.h"
#include "cstool/vfsdirchange.h"
#include "csutil/csobject.h"
#include "csutil/scfstr.h"
#include "csutil/xmltiny.h"
#include "iengine/engine.h"
#include "iengine/halo.h"
#include "iengine/imposter.h"
#include "iengine/movable.h"
#include "iengine/portalcontainer.h"
#include "iengine/renderloop.h"
#include "iengine/sharevar.h"
#include "igeom/trimesh.h"
#include "imap/ldrctxt.h"
#include "imap/services.h"
#include "imesh/object.h"
#include "imesh/objmodel.h"
#include "iutil/object.h"
#include "ivaria/reporter.h"
#include "csloadercontext.h"
#include "csthreadedloader.h"

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{
#undef CS_CLO
#undef CS_CON
#define CS_CLO (CS_TRIMESH_CLOSED|CS_TRIMESH_NOTCLOSED)
#define CS_CON (CS_TRIMESH_CONVEX|CS_TRIMESH_NOTCONVEX)
  static void SetTriMeshFlags (csFlags& flags, bool closed, bool notclosed,
    bool convex, bool notconvex)
  {
    if (closed) flags.Set (CS_CLO, CS_TRIMESH_CLOSED);
    if (notclosed) flags.Set (CS_CLO, CS_TRIMESH_NOTCLOSED);
    if (convex) flags.Set (CS_CON, CS_TRIMESH_CONVEX);
    if (notconvex) flags.Set (CS_CON, CS_TRIMESH_NOTCONVEX);
  }
#undef CS_CLO
#undef CS_CON

  bool csThreadedLoader::ParseTriMesh (iDocumentNode* node, iObjectModel* objmodel)
  {
    csRef<iTriangleMesh> trimesh;
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    bool convex = false;
    bool notconvex = false;
    bool closed = false;
    bool notclosed = false;
    bool use_default_mesh = false;
    csArray<csStringID> ids;
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_DEFAULT:
        if (trimesh)
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.parse.trimesh", child,
            "Use either <default>, <box>, or <mesh>!");
          return false;
        }
        use_default_mesh = true;
        break;
      case XMLTOKEN_BOX:
        if (trimesh || use_default_mesh)
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.parse.trimesh", child,
            "Use either <default>, <box>, or <mesh>!");
          return false;
        }
        if (!ParseTriMeshChildBox (child, trimesh))
          return false;
        break;
      case XMLTOKEN_MESH:
        if (trimesh || use_default_mesh)
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.parse.trimesh", child,
            "Use either <default>, <box>, or <mesh>!");
          return false;
        }
        if (!ParseTriMeshChildMesh (child, trimesh))
          return false;
        break;
      case XMLTOKEN_CLOSED:
        closed = true;
        notclosed = false;
        break;
      case XMLTOKEN_NOTCLOSED:
        closed = false;
        notclosed = true;
        break;
      case XMLTOKEN_CONVEX:
        convex = true;
        notconvex = false;
        break;
      case XMLTOKEN_NOTCONVEX:
        convex = false;
        notconvex = true;
        break;
      case XMLTOKEN_COLLDET:
        ReportWarning (
          "crystalspace.maploader.parse.trimesh",
          child, "<colldet> is deprecated. Use <id>colldet</id> instead.");
        ids.Push (stringSet->Request ("colldet"));
        break;
      case XMLTOKEN_VISCULL:
        ReportWarning (
          "crystalspace.maploader.parse.trimesh",
          child, "<viscull> is deprecated. Use <id>viscull</id> instead.");
        ids.Push (stringSet->Request ("viscull"));
        break;
      case XMLTOKEN_SHADOWS:
        ReportWarning (
          "crystalspace.maploader.parse.trimesh",
          child, "<shadows> is deprecated. Use <id>shadows</id> instead.");
        ids.Push (stringSet->Request ("shadows"));
        break;
      case XMLTOKEN_ID:
        ids.Push (stringSet->Request (child->GetContentsValue ()));
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return false;
      }
    }
    if (ids.GetSize () == 0)
    {
      SyntaxService->ReportError (
        "crystalspace.maploader.parse.trimesh",
        node, "No id's for this triangle mesh!");
      return false;
    }
    size_t i;
    if (use_default_mesh)
    {
      for (i = 0 ; i < ids.GetSize () ; i++)
      {
        if (objmodel->GetTriangleData (ids[i]))
        {
          csFlags& flags = objmodel->GetTriangleData (ids[i])->GetFlags ();
          SetTriMeshFlags (flags, closed, notclosed, convex, notconvex);
        }
      }
    }
    else
    {
      if (trimesh)
      {
        csFlags& flags = trimesh->GetFlags ();
        SetTriMeshFlags (flags, closed, notclosed, convex, notconvex);
      }

      for (i = 0 ; i < ids.GetSize () ; i++)
        objmodel->SetTriangleData (ids[i], trimesh);
    }

    return true;
  }

  bool csThreadedLoader::ParseImposterSettings (iImposter* imposter,
    iDocumentNode *node)
  {
    const char *s = node->GetAttributeValue ("active");
    if (s && !strcmp (s, "no"))
      imposter->SetImposterActive (false);
    else
      imposter->SetImposterActive (true);

    iSharedVariable *var = 0;

    s = node->GetAttributeValue ("range");
    if (s)
      var = Engine->GetVariableList()->FindByName (s);
    if (!s || !var)
    {
      SyntaxService->ReportError (
        "crystalspace.maploader.parse.meshobject",
        node, "Imposter range variable (%s) doesn't exist!", s);
      return false;
    }
    imposter->SetMinDistance (var);

    s = node->GetAttributeValue ("tolerance");
    if (s)
      var = Engine->GetVariableList ()->FindByName (s);
    if (!s || !var)
    {
      SyntaxService->ReportError (
        "crystalspace.maploader.parse.meshobject", node,
        "Imposter rotation tolerance variable (%s) doesn't exist!",
        s);
      return false;
    }
    imposter->SetRotationTolerance (var);

    s = node->GetAttributeValue ("camera_tolerance");
    if (s)
      var = Engine->GetVariableList ()->FindByName (s);
    if (!s || !var)
    {
      SyntaxService->ReportError (
        "crystalspace.maploader.parse.meshobject", node,
        "Imposter camera rotation tolerance variable (%s) doesn't exist!",
        s);
      return false;
    }
    imposter->SetCameraRotationTolerance (var);
    return true;
  }

  bool csThreadedLoader::ParseTriMeshChildBox (iDocumentNode* child,
    csRef<iTriangleMesh>& trimesh)
  {
    csBox3 b;
    if (!SyntaxService->ParseBox (child, b))
      return false;
    trimesh = csPtr<iTriangleMesh> (new csTriangleMeshBox (b));
    return true;
  }

  bool csThreadedLoader::ParseTriMeshChildMesh (iDocumentNode* child,
    csRef<iTriangleMesh>& trimesh)
  {
    int num_vt = 0;
    int num_tri = 0;
    csRef<iDocumentNodeIterator> child_it = child->GetNodes ();
    while (child_it->HasNext ())
    {
      csRef<iDocumentNode> child_child = child_it->Next ();
      if (child_child->GetType () != CS_NODE_ELEMENT) continue;
      const char* child_value = child_child->GetValue ();
      csStringID child_id = xmltokens.Request (child_value);
      switch (child_id)
      {
      case XMLTOKEN_V: num_vt++; break;
      case XMLTOKEN_T: num_tri++; break;
      default:
        SyntaxService->ReportBadToken (child_child);
        return false;
      }
    }

    csTriangleMesh* cstrimesh = new csTriangleMesh ();
    trimesh.AttachNew (cstrimesh);

    num_vt = 0;
    num_tri = 0;

    child_it = child->GetNodes ();
    while (child_it->HasNext ())
    {
      csRef<iDocumentNode> child_child = child_it->Next ();
      if (child_child->GetType () != CS_NODE_ELEMENT) continue;
      const char* child_value = child_child->GetValue ();
      csStringID child_id = xmltokens.Request (child_value);
      switch (child_id)
      {
      case XMLTOKEN_V:
        {
          csVector3 vt;
          if (!SyntaxService->ParseVector (child_child, vt))
            return false;
          cstrimesh->AddVertex (vt);
          num_vt++;
        }
        break;
      case XMLTOKEN_T:
        {
          int a = child_child->GetAttributeValueAsInt ("v1");
          int b = child_child->GetAttributeValueAsInt ("v2");
          int c = child_child->GetAttributeValueAsInt ("v3");
          cstrimesh->AddTriangle (a, b, c);
          num_tri++;
        }
        break;
      default:
        SyntaxService->ReportBadToken (child_child);
        return false;
      }
    }
    return true;
  }

  bool csThreadedLoader::ParsePortal (iLoaderContext* ldr_context,
    iDocumentNode* node, iSector* sourceSector, const char* container_name,
    iMeshWrapper*& container_mesh, iMeshWrapper* parent)
  {
    const char* name = node->GetAttributeValue ("name");
    iSector* destSector = 0;
    csPoly3D poly;

    CS::Utility::PortalParameters params;
    csRef<csRefCount> parseState;
    scfString destSectorName;
    params.destSector = &destSectorName;

    // Array of keys we need to parse later.
    csRefArray<iDocumentNode> key_nodes;

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      bool handled;
      if (!SyntaxService->HandlePortalParameter (child, ldr_context,
        parseState, params, handled))
      {
        return false;
      }
      if (!handled)
      {
        const char* value = child->GetValue ();
        csStringID id = xmltokens.Request (value);
        switch (id)
        {
        case XMLTOKEN_KEY:
          key_nodes.Push (child);
          break;
        case XMLTOKEN_V:
          {
            csVector3 vec;
            if (!SyntaxService->ParseVector (child, vec))
              return false;
            poly.AddVertex (vec);
          }
          break;
        default:
          SyntaxService->ReportBadToken (child);
          return false;
        }
      }
    }

    iPortal* portal;
    // If autoresolve is true we clear the sector since we want the callback
    // to be used.
    if (params.autoresolve)
      destSector = 0;
    else
      destSector = ldr_context->FindSector (destSectorName.GetData ());
    csRef<iMeshWrapper> mesh;
    if (container_mesh)
    {
      mesh = container_mesh;
      csRef<iPortalContainer> pc = 
        scfQueryInterface<iPortalContainer> (mesh->GetMeshObject ());
      CS_ASSERT (pc != 0);
      portal = pc->CreatePortal (poly.GetVertices (),
        (int)poly.GetVertexCount ());
      portal->SetSector (destSector);
    }
    else if (parent)
    {
      CS_ASSERT (sourceSector == 0);
      mesh = Engine->CreatePortal (
        container_name ? container_name : name,
        parent, destSector,
        poly.GetVertices (), (int)poly.GetVertexCount (), portal);
      ldr_context->AddToCollection(mesh->QueryObject ());
    }
    else
    {
      mesh = Engine->CreatePortal (
        container_name ? container_name : name,
        sourceSector, csVector3 (0), destSector,
        poly.GetVertices (), (int)poly.GetVertexCount (), portal);
      ldr_context->AddToCollection(mesh->QueryObject ());
    }
    container_mesh = mesh;
    if (name)
      portal->SetName (name);
    if (!destSector)
    {
      // Create a callback to find the sector at runtime when the
      // portal is first used.
      csRef<csMissingSectorCallback> missing_cb;
      missing_cb.AttachNew (new csMissingSectorCallback (
        ldr_context, destSectorName.GetData (), params.autoresolve));
      portal->SetMissingSectorCallback (missing_cb);
    }

    portal->GetFlags ().Set (params.flags);
    if (params.mirror)
    {
      csPlane3 p = poly.ComputePlane ();
      portal->SetWarp (csTransform::GetReflect (p));
    }
    else if (params.warp)
    {
      portal->SetWarp (params.m, params.before, params.after);
    }
    if (params.msv != -1)
    {
      portal->SetMaximumSectorVisit (params.msv);
    }

    size_t i;
    for (i = 0 ; i < key_nodes.GetSize () ; i++)
    {
      if (!ParseKey (key_nodes[i], container_mesh->QueryObject()))
        return false;
    }

    return true;
  }

  bool csThreadedLoader::ParsePortals (iLoaderContext* ldr_context,
    iDocumentNode* node, iSector* sourceSector,
    iMeshWrapper* parent, iStreamSource* ssource)
  {
    const char* container_name = node->GetAttributeValue ("name");
    iMeshWrapper* container_mesh = 0;
    csString priority;
    bool staticpos = false;
    bool staticshape = false;
    bool zbufSet = false;
    bool prioSet = false;
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      bool handled;
      if (!HandleMeshParameter (ldr_context, container_mesh, parent, child, id,
        handled, priority, true, staticpos, staticshape, zbufSet, prioSet,
        false, ssource))
        return false;
      if (!handled) switch (id)
      {
      case XMLTOKEN_PORTAL:
        if (!ParsePortal (ldr_context, child, sourceSector,
          container_name, container_mesh, parent))
          return false;
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return false;
      }
    }

    if (!priority.IsEmpty ())
      container_mesh->SetRenderPriority (Engine->GetRenderPriority (priority));
    container_mesh->GetMeshObject ()->GetFlags ().SetBool (CS_MESH_STATICPOS,
      staticpos);
    container_mesh->GetMeshObject ()->GetFlags ().SetBool (CS_MESH_STATICSHAPE,
      staticshape);

    return true;
  }

  iLight* csThreadedLoader::ParseStatlight (iLoaderContext* ldr_context,
    iDocumentNode* node)
  {
    const char* lightname = node->GetAttributeValue ("name");

    csVector3 pos;

    csVector3 attenvec (0, 0, 0);
    float spotfalloffInner = 1, spotfalloffOuter = 0;
    csLightType type = CS_LIGHT_POINTLIGHT;
    csFlags lightFlags;

    bool use_light_transf = false;
    bool use_light_transf_vector = false;
    csReversibleTransform light_transf;

    float distbright = 1;

    float influenceRadius = 0;
    bool influenceOverride = false;

    csLightAttenuationMode attenuation = CS_ATTN_LINEAR;
    float dist = 0;

    csColor color;
    csColor specular (0, 0, 0);
    bool userSpecular = false;
    csLightDynamicType dyn;
    csRefArray<csShaderVariable> shader_variables;
    struct csHaloDef
    {
      int type;
      union
      {
        struct
        {
          float Intensity;
          float Cross;
        } cross;
        struct
        {
          int Seed;
          int NumSpokes;
          float Roundness;
        } nova;
        struct
        {
          iMaterialWrapper* mat_center;
          iMaterialWrapper* mat_spark1;
          iMaterialWrapper* mat_spark2;
          iMaterialWrapper* mat_spark3;
          iMaterialWrapper* mat_spark4;
          iMaterialWrapper* mat_spark5;
        } flare;
      };
    } halo;

    // This csObject will contain all key-value pairs as children
    csObject Keys;

    memset (&halo, 0, sizeof (halo));

    // New format.
    pos.x = pos.y = pos.z = 0;
    color.red = color.green = color.blue = 1;
    dyn = CS_LIGHT_DYNAMICTYPE_STATIC;

    dist = 1;


    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_RADIUS:
        {
          dist = child->GetContentsValueAsFloat ();
          csRef<iDocumentAttribute> attr;
          if (attr = child->GetAttribute ("brightness"))
          {
            distbright = attr->GetValueAsFloat();
          }
        }
        break;
      case XMLTOKEN_CENTER:
        if (!SyntaxService->ParseVector (child, pos))
          return 0;
        break;
      case XMLTOKEN_COLOR:
        if (!SyntaxService->ParseColor (child, color))
          return 0;
        break;
      case XMLTOKEN_SPECULAR:
        if (!SyntaxService->ParseColor (child, specular))
          return 0;
        userSpecular = true;
        break;
      case XMLTOKEN_DYNAMIC:
        {
          bool d;
          if (!SyntaxService->ParseBool (child, d, true))
            return 0;
          if (d)
            dyn = CS_LIGHT_DYNAMICTYPE_PSEUDO;
          else
            dyn = CS_LIGHT_DYNAMICTYPE_STATIC;
        }
        break;
      case XMLTOKEN_KEY:
        if (!ParseKey (child, &Keys))
          return false;
        break;
      case XMLTOKEN_HALO:
        {
          const char* type;
          csRef<iDocumentNode> typenode = child->GetNode ("type");
          if (!typenode)
          {
            // Default halo, type 'cross' assumed.
            type = "cross";
          }
          else
          {
            type = typenode->GetContentsValue ();
          }

          if (!strcasecmp (type, "cross"))
          {
            halo.type = 1;
            halo.cross.Intensity = 2.0f;
            halo.cross.Cross = 0.45f;
            csRef<iDocumentNode> intnode = child->GetNode ("intensity");
            if (intnode)
            {
              halo.cross.Intensity = intnode->GetContentsValueAsFloat ();
            }
            csRef<iDocumentNode> crossnode = child->GetNode ("cross");
            if (crossnode)
            {
              halo.cross.Cross = crossnode->GetContentsValueAsFloat ();
            }
          }
          else if (!strcasecmp (type, "nova"))
          {
            halo.type = 2;
            halo.nova.Seed = 0;
            halo.nova.NumSpokes = 100;
            halo.nova.Roundness = 0.5;
            csRef<iDocumentNode> seednode = child->GetNode ("seed");
            if (seednode)
            {
              halo.nova.Seed = seednode->GetContentsValueAsInt ();
            }
            csRef<iDocumentNode> spokesnode = child->GetNode ("numspokes");
            if (spokesnode)
            {
              halo.nova.NumSpokes = spokesnode->GetContentsValueAsInt ();
            }
            csRef<iDocumentNode> roundnode = child->GetNode ("roundness");
            if (roundnode)
            {
              halo.nova.Roundness = roundnode->GetContentsValueAsFloat ();
            }
          }
          else if (!strcasecmp (type, "flare"))
          {
            halo.type = 3;
            iLoaderContext* lc = ldr_context;
            csRef<iDocumentNode> matnode;

            matnode = child->GetNode ("centermaterial");
            halo.flare.mat_center = matnode ? lc->FindMaterial (
              matnode->GetContentsValue ()) : 0;
            if (!halo.flare.mat_center)
            {
              SyntaxService->ReportError (
                "crystalspace.maploader.parse.light",
                child, "Can't find material for flare!");
              return 0;
            }
            matnode = child->GetNode ("spark1material");
            halo.flare.mat_spark1 = matnode ? lc->FindMaterial (
              matnode->GetContentsValue ()) : 0;
            if (!halo.flare.mat_spark1)
            {
              SyntaxService->ReportError (
                "crystalspace.maploader.parse.light",
                child, "Can't find material for flare!");
              return 0;
            }
            matnode = child->GetNode ("spark2material");
            halo.flare.mat_spark2 = matnode ? lc->FindMaterial (
              matnode->GetContentsValue ()) : 0;
            if (!halo.flare.mat_spark2)
            {
              SyntaxService->ReportError (
                "crystalspace.maploader.parse.light",
                child, "Can't find material for flare!");
              return 0;
            }
            matnode = child->GetNode ("spark3material");
            halo.flare.mat_spark3 = matnode ? lc->FindMaterial (
              matnode->GetContentsValue ()) : 0;
            if (!halo.flare.mat_spark3)
            {
              SyntaxService->ReportError (
                "crystalspace.maploader.parse.light",
                child, "Can't find material for flare!");
              return 0;
            }
            matnode = child->GetNode ("spark4material");
            halo.flare.mat_spark4 = matnode ? lc->FindMaterial (
              matnode->GetContentsValue ()) : 0;
            if (!halo.flare.mat_spark4)
            {
              SyntaxService->ReportError (
                "crystalspace.maploader.parse.light",
                child, "Can't find material for flare!");
              return 0;
            }
            matnode = child->GetNode ("spark5material");
            halo.flare.mat_spark5 = matnode ? lc->FindMaterial (
              matnode->GetContentsValue ()) : 0;
            if (!halo.flare.mat_spark5)
            {
              SyntaxService->ReportError (
                "crystalspace.maploader.parse.light",
                child, "Can't find material for flare!");
              return 0;
            }
          }
          else
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.light",
              child,
              "Unknown halo type '%s'. Use 'cross', 'nova' or 'flare'!",
              type);
            return 0;
          }
        }
        break;
      case XMLTOKEN_ATTENUATION:
        {
          const char* att = child->GetContentsValue();
          if (att)
          {
            if (!strcasecmp (att, "none"))
              attenuation = CS_ATTN_NONE;
            else if (!strcasecmp (att, "linear"))
              attenuation = CS_ATTN_LINEAR;
            else if (!strcasecmp (att, "inverse"))
              attenuation = CS_ATTN_INVERSE;
            else if (!strcasecmp (att, "realistic"))
              attenuation = CS_ATTN_REALISTIC;
            else if (!strcasecmp (att, "clq"))
              attenuation = CS_ATTN_CLQ;
            else
            {
              SyntaxService->ReportBadToken (child);
              return 0;
            }
          }
          else
          {
            attenuation = CS_ATTN_CLQ;
          }

          attenvec.x = child->GetAttributeValueAsFloat ("c");
          attenvec.y = child->GetAttributeValueAsFloat ("l");
          attenvec.z = child->GetAttributeValueAsFloat ("q");
        }
        break;
      case XMLTOKEN_INFLUENCERADIUS:
        {
          influenceRadius = child->GetContentsValueAsFloat();
          influenceOverride = true;
        }
        break;
      case XMLTOKEN_ATTENUATIONVECTOR:
        {
          //@@@ should be scrapped in favor of specification via
          // "attenuation".
          if (!SyntaxService->ParseVector (child, attenvec))
            return 0;
          attenuation = CS_ATTN_CLQ;
        }
        break;
      case XMLTOKEN_TYPE:
        {
          const char* t = child->GetContentsValue ();
          if (t)
          {
            if (!strcasecmp (t, "point") || !strcasecmp (t, "pointlight"))
              type = CS_LIGHT_POINTLIGHT;
            else if (!strcasecmp (t, "directional"))
              type = CS_LIGHT_DIRECTIONAL;
            else if (!strcasecmp (t, "spot") || !strcasecmp (t, "spotlight"))
              type = CS_LIGHT_SPOTLIGHT;
            else
            {
              SyntaxService->ReportBadToken (child);
              return 0;
            }
          }
        }
        break;
      case XMLTOKEN_MOVE:
        {
          use_light_transf = true;
          csRef<iDocumentNode> matrix_node = child->GetNode ("matrix");
          if (matrix_node)
          {
            csMatrix3 m;
            if (!SyntaxService->ParseMatrix (matrix_node, m))
              return false;
            light_transf.SetO2T (m);
          }
          csRef<iDocumentNode> vector_node = child->GetNode ("v");
          if (vector_node)
          {
            csVector3 v;
            if (!SyntaxService->ParseVector (vector_node, v))
              return false;
            use_light_transf_vector = true;
            light_transf.SetO2TTranslation (v);
          }
        }
        break;
      case XMLTOKEN_DIRECTION:
        SyntaxService->ReportError ("crystalspace.maploader.light", child,
          "'direction' is no longer support for lights. Use 'move'!");
        return 0;
      case XMLTOKEN_SPOTLIGHTFALLOFF:
        {
          spotfalloffInner = child->GetAttributeValueAsFloat ("inner");
          spotfalloffInner *= (PI/180);
          spotfalloffInner = cosf(spotfalloffInner);
          spotfalloffOuter = child->GetAttributeValueAsFloat ("outer");
          spotfalloffOuter *= (PI/180);
          spotfalloffOuter = cosf(spotfalloffOuter);
        }
        break;

      case XMLTOKEN_SHADERVAR:
        {
          const char* varname = child->GetAttributeValue ("name");
          csRef<csShaderVariable> var;
          var.AttachNew (new csShaderVariable (stringSetSvName->Request (varname)));
          if (!SyntaxService->ParseShaderVar (ldr_context, child, *var))
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.load.meshobject", child,
              "Error loading shader variable '%s' in light '%s'.", 
              varname, lightname);
            break;
          }
          //svc->AddVariable (var);
          shader_variables.Push(var);
        }
        break;
      case XMLTOKEN_NOSHADOWS:
        {
          bool flag;
          if (!SyntaxService->ParseBool (child, flag, true))
            return false;
          lightFlags.SetBool (CS_LIGHT_NOSHADOWS, flag);
        }
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return 0;
      }
    }

    // implicit radius
    if (dist == 0)
    {
      if (color.red > color.green && color.red > color.blue) dist = color.red;
      else if (color.green > color.blue) dist = color.green;
      else dist = color.blue;
    }

    csRef<iLight> l = Engine->CreateLight (lightname, pos,
      dist, color, dyn);
    ldr_context->AddToCollection(l->QueryObject ());
    l->SetType (type);
    l->GetFlags() = lightFlags;
    l->SetSpotLightFalloff (spotfalloffInner, spotfalloffOuter);

    for (size_t i = 0; i < shader_variables.GetSize (); i++)
    {
      l->GetSVContext()->AddVariable(shader_variables[i]);
    }

    if (userSpecular) l->SetSpecularColor (specular);

    if (use_light_transf)
    {
      if (!use_light_transf_vector)
        light_transf.SetOrigin (pos);
      l->GetMovable ()->SetTransform (light_transf);
      l->GetMovable ()->UpdateMove ();
    }

    switch (halo.type)
    {
    case 1:
      l->CreateCrossHalo (halo.cross.Intensity,
        halo.cross.Cross);
      break;
    case 2:
      l->CreateNovaHalo (halo.nova.Seed, halo.nova.NumSpokes,
        halo.nova.Roundness);
      break;
    case 3:
      {
        iMaterialWrapper* ifmc = halo.flare.mat_center;
        iMaterialWrapper* ifm1 = halo.flare.mat_spark1;
        iMaterialWrapper* ifm2 = halo.flare.mat_spark2;
        iMaterialWrapper* ifm3 = halo.flare.mat_spark3;
        iMaterialWrapper* ifm4 = halo.flare.mat_spark4;
        iMaterialWrapper* ifm5 = halo.flare.mat_spark5;
        iFlareHalo* flare = l->CreateFlareHalo ();
        flare->AddComponent (0.0f, 1.2f, 1.2f, CS_FX_ADD, ifmc);
        flare->AddComponent (0.3f, 0.1f, 0.1f, CS_FX_ADD, ifm3);
        flare->AddComponent (0.6f, 0.4f, 0.4f, CS_FX_ADD, ifm4);
        flare->AddComponent (0.8f, 0.05f, 0.05f, CS_FX_ADD, ifm5);
        flare->AddComponent (1.0f, 0.7f, 0.7f, CS_FX_ADD, ifm1);
        flare->AddComponent (1.3f, 0.1f, 0.1f, CS_FX_ADD, ifm3);
        flare->AddComponent (1.5f, 0.3f, 0.3f, CS_FX_ADD, ifm4);
        flare->AddComponent (1.8f, 0.1f, 0.1f, CS_FX_ADD, ifm5);
        flare->AddComponent (2.0f, 0.5f, 0.5f, CS_FX_ADD, ifm2);
        flare->AddComponent (2.1f, 0.15f, 0.15f, CS_FX_ADD, ifm3);
        flare->AddComponent (2.5f, 0.2f, 0.2f, CS_FX_ADD, ifm3);
        flare->AddComponent (2.8f, 0.4f, 0.4f, CS_FX_ADD, ifm4);
        flare->AddComponent (3.0f, 3.0f, 3.0f, CS_FX_ADD, ifm1);
        flare->AddComponent (3.1f, 0.05f, 0.05f, CS_FX_ADD, ifm5);
        flare->AddComponent (3.3f, 0.15f, 0.15f, CS_FX_ADD, ifm2);
      }
      break;
    }
    l->SetAttenuationMode (attenuation);
    if (attenuation == CS_ATTN_CLQ)
    {
      if (attenvec.IsZero())
      {
        //@@TODO:
      }
      else
      {
        l->SetAttenuationConstants (csVector4 (attenvec, 0));
      }
    }

    if (influenceOverride) l->SetCutoffDistance (influenceRadius);
    else l->SetCutoffDistance (dist);

    // Move the key-value pairs from 'Keys' to the light object
    l->QueryObject ()->ObjAddChildren (&Keys);
    Keys.ObjRemoveAll ();

    l->IncRef ();	// To make sure smart pointer doesn't release.
    return l;
  }

  bool csThreadedLoader::ParseShaderList (csLoaderContext* ldr_context)
  {
    if(!ldr_context->availShaders.IsEmpty())
    {
      csRef<iShaderManager> shaderMgr (
        csQueryRegistry<iShaderManager> (object_reg));

      if(!shaderMgr)
      {
        ReportNotify ("iShaderManager not found, ignoring shaders!");
        return true;
      }

      for(size_t i=0; i<ldr_context->availShaders.GetSize(); ++i)
      {
        if(!ParseShader (ldr_context, ldr_context->availShaders[i].node, shaderMgr))
          return false;
      }

      ldr_context->availShaders.DeleteAll();
    }
    return true;
  }

  bool csThreadedLoader::ParseShader (iLoaderContext* ldr_context,
    iDocumentNode* node,
    iShaderManager* shaderMgr)
  {
    // @@@ FIXME: unify with csTextSyntaxService::ParseShaderRef()?

    /*csRef<iShader> shader (shaderMgr->CreateShader ());
    //test if we have a childnode named file, if so load from file, else
    //use inline loading
    csRef<iDocumentNode> fileChild = child->GetNode ("file");
    if (fileChild)
    {
    csRef<iDataBuffer> db = VFS->ReadFile (fileChild->GetContentsValue ());
    shader->Load (db);
    }
    else
    {
    shader->Load (child);
    }*/

    csRef<iDocumentNode> shaderNode;
    csRef<iDocumentNode> fileChild = node->GetNode ("file");

    csVfsDirectoryChanger dirChanger (vfs);

    if (fileChild)
    {
      csString filename (fileChild->GetContentsValue ());
      csRef<iFile> shaderFile = vfs->Open (filename, VFS_FILE_READ);

      if(!shaderFile)
      {
        ReportWarning ("crystalspace.maploader",
          "Unable to open shader file '%s'!", filename.GetData());
        return false;
      }

      csRef<iDocumentSystem> docsys =
        csQueryRegistry<iDocumentSystem> (object_reg);
      if (docsys == 0)
        docsys.AttachNew (new csTinyDocumentSystem ());
      csRef<iDocument> shaderDoc = docsys->CreateDocument ();
      const char* err = shaderDoc->Parse (shaderFile, false);
      if (err != 0)
      {
        ReportWarning ("crystalspace.maploader",
          "Could not parse shader file '%s': %s",
          filename.GetData(), err);
        return false;
      }
      shaderNode = shaderDoc->GetRoot ()->GetNode ("shader");
      if (!shaderNode)
      {
        SyntaxService->ReportError ("crystalspace.maploader", node,
          "Shader file '%s' is not a valid shader XML file!",
          filename.GetData ());
        return false;
      }

      dirChanger.ChangeTo (filename);
    }
    else
    {
      shaderNode = node->GetNode ("shader");
      if (!shaderNode)
      {
        SyntaxService->ReportError ("crystalspace.maploader", node,
          "'shader' or 'file' node is missing!");
        return false;
      }
    }

    csRef<iShader> shader = SyntaxService->ParseShader (ldr_context, shaderNode);
    if (shader.IsValid())
    {
      ldr_context->AddToCollection(shader->QueryObject ());
    }
    return shader.IsValid();
  }

  bool csThreadedLoader::ParseVariableList (iLoaderContext* ldr_context,
    iDocumentNode* node)
  {
    if (!Engine) return false;

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_VARIABLE:
        if (!ParseSharedVariable (ldr_context, child))
          return false;
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return false;
      }
    }

    return true;
  }

  bool csThreadedLoader::ParseSharedVariable (iLoaderContext* ldr_context,
    iDocumentNode* node)
  {
    csRef<iSharedVariable> v = Engine->GetVariableList()->New ();
    v->SetName (node->GetAttributeValue ("name"));

    if (v->GetName ())
    {
      csRef<iDocumentNode> colornode = node->GetNode ("color");
      csRef<iDocumentNode> vectornode = node->GetNode ("v");
      csRef<iDocumentAttribute> stringattr = node->GetAttribute ("string");
      if (colornode)
      {
        csColor c;
        if (!SyntaxService->ParseColor (colornode, c))
          return false;
        v->SetColor (c);
      }
      else if (vectornode)
      {
        csVector3 vec;
        if (!SyntaxService->ParseVector (vectornode, vec))
          return false;
        v->SetVector (vec);
      }
      else if (stringattr)
      {
        v->SetString (stringattr->GetValue ());
      }
      else
      {
        v->Set (node->GetAttributeValueAsFloat ("value"));
      }
      AddSharedVarToList(v);
    }
    else
    {
      SyntaxService->ReportError ("crystalspace.maploader",
        node, "Variable tag does not have 'name' attribute.");
      return false;
    }

    ldr_context->AddToCollection(v->QueryObject ());
    return true;
  }

  bool csThreadedLoader::ParseStart (iDocumentNode* node, iCameraPosition* campos)
  {
    csString start_sector = "room";
    csVector3 pos (0, 0, 0);
    csVector3 up (0, 1, 0);
    csVector3 forward (0, 0, 1);

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_SECTOR:
        start_sector = child->GetContentsValue ();
        break;
      case XMLTOKEN_POSITION:
        if (!SyntaxService->ParseVector (child, pos))
          return false;
        break;
      case XMLTOKEN_UP:
        if (!SyntaxService->ParseVector (child, up))
          return false;
        break;
      case XMLTOKEN_FORWARD:
        if (!SyntaxService->ParseVector (child, forward))
          return false;
        break;
      case XMLTOKEN_FARPLANE:
        {
          csPlane3 p;
          p.A () = child->GetAttributeValueAsFloat ("a");
          p.B () = child->GetAttributeValueAsFloat ("b");
          p.C () = child->GetAttributeValueAsFloat ("c");
          p.D () = child->GetAttributeValueAsFloat ("d");
          campos->SetFarPlane (&p);
        }
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return false;
      }
    }

    campos->Set (start_sector, pos, forward, up);
    return true;
  }

  iSector* csThreadedLoader::ParseSector (iLoaderContext* ldr_context,
    iDocumentNode* node, iStreamSource* ssource, csRefArray<iThreadReturn>& threadReturns)
  {
    const char* secname = node->GetAttributeValue ("name");

    bool do_culler = false;
    csString culplugname;

    iSector* sector = ldr_context->FindSector (secname);
    if (sector == 0)
    {
      sector = Engine->CreateSector (secname, false);
      AddSectorToList(sector);
      ldr_context->AddToCollection(sector->QueryObject ());
    }

    csRef<iDocumentNode> culler_params;

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_AMBIENT:
        {
          csColor c;
          if (!SyntaxService->ParseColor (child, c))
            return false;
          sector->SetDynamicAmbientLight (c);
        }
        break;
      case XMLTOKEN_MESHGEN:
        if (!LoadMeshGen (ldr_context, child, sector))
          return 0;
        break;
      case XMLTOKEN_ADDON:
        if (!LoadAddOn (ldr_context, child, sector, false, ssource))
          return 0;
        break;
      case XMLTOKEN_META:
        if (!LoadAddOn (ldr_context, child, sector, true, ssource))
          return 0;
        break;
      case XMLTOKEN_PORTAL:
        {
          iMeshWrapper* container_mesh = 0;
          if (!ParsePortal (ldr_context, child, sector, 0, container_mesh, 0))
            return 0;
        }
        break;
      case XMLTOKEN_PORTALS:
        if (!ParsePortals (ldr_context, child, sector, 0, ssource))
          return 0;
        break;
      case XMLTOKEN_CULLERP:
        {
          const char* pluginname = child->GetAttributeValue ("plugin");
          if (pluginname)
          {
            // New way to write cullerp.
            culplugname = pluginname;
            culler_params = child;	// Remember for later.
          }
          else
          {
            // Old way.
            culplugname = child->GetContentsValue ();
            culler_params = 0;
          }
          if (culplugname.IsEmpty ())
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.sector",
              child,
              "CULLERP expects the name of a visibility culling plugin!");
            return 0;
          }
          else
          {
            do_culler = true;
          }
        }
        break;
      case XMLTOKEN_MESHREF:
        {
          threadReturns.Push(LoadMeshRef(child, sector, ldr_context, ssource));
        }
        break;
      case XMLTOKEN_TRIMESH:
        {
          const char* meshname = child->GetAttributeValue ("name");
          if (!meshname)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.load.trimesh",
              child, "'trimesh' requires a name in sector '%s'!",
              secname ? secname : "<noname>");
            return 0;
          }
          csRef<iMeshWrapper> mesh = Engine->CreateMeshWrapper (
            "crystalspace.mesh.object.null", meshname, 0, csVector3(0), false);
          if (!LoadTriMeshInSector (ldr_context, mesh, child, ssource))
          {
            // Error is already reported.
            return 0;
          }
          else
          {
            AddMeshToList(mesh);
            ldr_context->AddToCollection(mesh->QueryObject ());
          }
          mesh->GetMovable ()->SetSector (sector);
          mesh->GetMovable ()->UpdateMove ();
        }
        break;
      case XMLTOKEN_MESHOBJ:
        {
          const char* meshname = child->GetAttributeValue ("name");
          if (!meshname)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.load.meshobject",
              child, "'meshobj' requires a name in sector '%s'!",
              secname ? secname : "<noname>");
            return 0;
          }
          csRef<iMeshWrapper> mesh = Engine->CreateMeshWrapper (meshname, false);
          csRef<iThreadReturn> itr = LoadMeshObject (ldr_context, mesh, 0, child, ssource, sector, meshname, vfs->GetCwd());
          AddLoadingMeshObject(meshname, itr);
          threadReturns.Push(itr);
        }
        break;
      case XMLTOKEN_MESHLIB:
        {
          const char* meshname = child->GetAttributeValue ("name");
          if (!meshname)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.load.meshobject",
              child, "'meshlib' requires a name (sector '%s')!",
              secname ? secname : "<noname>");
            return 0;
          }
          iMeshWrapper* mesh = Engine->GetMeshes ()->FindByName (meshname);
          if (!mesh)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.load.meshobject",
              child,
              "Could not find mesh object '%s' (sector '%s') for MESHLIB!",
              meshname, secname ? secname : "<noname>");
            return 0;
          }
          csRef<iThreadReturn> itr = LoadMeshObject (ldr_context, mesh, 0, child, ssource, sector, meshname, vfs->GetCwd());
          AddLoadingMeshObject(meshname, itr);
          threadReturns.Push(itr);
        }
        break;
      case XMLTOKEN_LIGHT:
        {
          iLight* sl = ParseStatlight (ldr_context, child);
          if (!sl) return 0;
          AddLightToList(sl, sl->QueryObject()->GetName());
          threadReturns.Push(sector->AddLight (sl));
          sl->DecRef ();
        }
        break;
      case XMLTOKEN_NODE:
        {
          iMapNode *n = ParseNode (child, sector);
          if (n)
          {
            n->DecRef ();
          }
          else
          {
            return 0;
          }
        }
        break;
      case XMLTOKEN_FOG:
        {
          csFog f;
          f.color.red = child->GetAttributeValueAsFloat ("red");
          f.color.green = child->GetAttributeValueAsFloat ("green");
          f.color.blue = child->GetAttributeValueAsFloat ("blue");
          f.density = child->GetAttributeValueAsFloat ("density");
          f.start = child->GetAttributeValueAsFloat ("start");
          f.end = child->GetAttributeValueAsFloat ("end");
          csRef<iDocumentAttribute> mode_attr = child->GetAttribute ("mode");
          if (mode_attr)
          {
            const char* str_mode = mode_attr->GetValue();
            if (!strcmp(str_mode, "linear"))
              f.mode = CS_FOG_MODE_LINEAR;
            else if (!strcmp(str_mode, "exp"))
              f.mode = CS_FOG_MODE_EXP;
            else if (!strcmp(str_mode, "exp2"))
              f.mode = CS_FOG_MODE_EXP2;
            else
              f.mode = CS_FOG_MODE_NONE;
          }
          else
            f.mode = CS_FOG_MODE_CRYSTALSPACE;
          sector->SetFog (f);
        }
        break;
      case XMLTOKEN_KEY:
        if (!ParseKey (child, sector->QueryObject()))
          return 0;
        break;
      case XMLTOKEN_RENDERLOOP:
        {
          bool set;
          iRenderLoop* loop = ParseRenderLoop (child, set);
          if (!loop)
          {
            return false;
          }
          if (set)
          {
            sector->SetRenderLoop (loop);
          }
        }
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return 0;
      }
    }
    if (do_culler)
    {
      SetSectorVisibilityCuller(sector, culplugname.GetData(), culler_params);
    }

    return sector;
  }

  iRenderLoop* csThreadedLoader::ParseRenderLoop (iDocumentNode* node, bool& set)
  {
    set = true;
    const char* varname = node->GetAttributeValue ("variable");
    if (varname)
    {
      iSharedVariableList* vl = Engine->GetVariableList ();
      iSharedVariable* var = vl->FindByName (varname);
      csRef<iDocumentNode> default_node;
      csRef<iDocumentNodeIterator> it = node->GetNodes ();
      iRenderLoop* loop = 0;
      while (it->HasNext ())
      {
        csRef<iDocumentNode> child = it->Next ();
        if (child->GetType () != CS_NODE_ELEMENT) continue;
        const char* value = child->GetValue ();
        csStringID id = xmltokens.Request (value);
        switch (id)
        {
        case XMLTOKEN_CONDITION:
          if (var && var->GetString ())
          {
            csString value = child->GetAttributeValue ("value");
            if (value == var->GetString ())
            {
              loop = Engine->GetRenderLoopManager ()->Retrieve (
                child->GetContentsValue ());
            }
          }
          break;
        case XMLTOKEN_DEFAULT:
          default_node = child;
          break;
        default:
          SyntaxService->ReportBadToken (child);
          return 0;
        }
      }
      if (!loop && default_node)
      {
        loop = Engine->GetRenderLoopManager ()->Retrieve (
          default_node->GetContentsValue ());
        if (!loop)
        {
          SyntaxService->Report ("crystalspace.maploader.parse.settings",
            CS_REPORTER_SEVERITY_ERROR,
            node, "No suitable renderloop found!");
          return 0;
        }
      }
      if (!loop)
      {
        loop = Engine->GetCurrentDefaultRenderloop ();
        set = false;
      }

      return loop;
    }
    else
    {
      const char* loopName = node->GetContentsValue ();
      if (loopName)
      {
        iRenderLoop* loop = Engine->GetRenderLoopManager()->Retrieve (loopName);
        if (!loop)
        {
          SyntaxService->Report ("crystalspace.maploader.parse.settings",
            CS_REPORTER_SEVERITY_ERROR,
            node, "Render loop '%s' not found",
            loopName);
          return 0;
        }
        return loop;
      }
      else
      {
        SyntaxService->Report (
          "crystalspace.maploader.parse.settings",
          CS_REPORTER_SEVERITY_ERROR,
          node, "Expected render loop name: %s",
          loopName);
        return 0;
      }
    }
    return 0;
  }

  THREADED_CALLABLE_IMPL6(csThreadedLoader, ParseAddOn, csRef<iLoaderPlugin> plugin,
    csRef<iDocumentNode> node, csRef<iStreamSource> ssource, csRef<iLoaderContext> ldr_context,
    csRef<iBase> context, const char* dir)
  {
    if(dir)
    {
      vfs->PushDir();
      vfs->ChDir(dir);
    }
    csRef<iBase> base = plugin->Parse(node, ssource, ldr_context, context);
    ret->SetResult(base);
    if(dir)
    {
      vfs->PopDir();
    }
    return base.IsValid();    
  }

  THREADED_CALLABLE_IMPL5(csThreadedLoader, ParseAddOnBinary, csRef<iBinaryLoaderPlugin> plugin,
    csRef<iDataBuffer> dbuf, csRef<iStreamSource> ssource, csRef<iLoaderContext> ldr_context,
    csRef<iBase> context)
  {
    csRef<iBase> base = plugin->Parse(dbuf, ssource, ldr_context, context);
    ret->SetResult(base);
    return base.IsValid(); 
  }

  THREADED_CALLABLE_IMPL3(csThreadedLoader, SetSectorVisibilityCuller,
    csRef<iSector> sector, const char* culplugname, csRef<iDocumentNode> culler_params)
  {
    bool rc = sector->SetVisibilityCullerPlugin (culplugname, culler_params);
    if (!rc)
    {
      SyntaxService->ReportError (
        "crystalspace.maploader.load.sector",
        culler_params, "Could not load visibility culler for sector '%s'!",
        sector->QueryObject()->GetName() ? sector->QueryObject()->GetName()
        : "<noname>");
      return false;
    }
    return true;
  }

  iMapNode* csThreadedLoader::ParseNode (iDocumentNode* node, iSector* sec)
  {
    iMapNode* pNode = (iMapNode*)(new csMapNode (
      node->GetAttributeValue ("name")));
    pNode->SetSector (sec);

    csVector3 pos, v;

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_ADDON:
        SyntaxService->ReportError (
          "crystalspace.maploader.parse.node",
          child, "'addon' not yet supported in node!");
        return 0;
      case XMLTOKEN_META:
        SyntaxService->ReportError (
          "crystalspace.maploader.parse.node",
          child, "'meta' not yet supported in node!");
        return 0;
      case XMLTOKEN_KEY:
        if (!ParseKey (child, pNode->QueryObject()))
          return false;
        break;
      case XMLTOKEN_POSITION:
        if (!SyntaxService->ParseVector (child, pos))
          return 0;
        break;
      case XMLTOKEN_XVECTOR:
        if (!SyntaxService->ParseVector (child, v))
          return 0;
        pNode->SetXVector (v);
        break;
      case XMLTOKEN_YVECTOR:
        if (!SyntaxService->ParseVector (child, v))
          return 0;
        pNode->SetYVector (v);
        break;
      case XMLTOKEN_ZVECTOR:
        if (!SyntaxService->ParseVector (child, v))
          return 0;
        pNode->SetZVector (v);
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return 0;
      }
    }

    pNode->SetPosition (pos);

    return pNode;
  }
}
CS_PLUGIN_NAMESPACE_END(csparser)
