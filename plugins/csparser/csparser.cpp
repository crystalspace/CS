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
#include "csgeom/trimesh.h"

#include "iengine/engine.h"
#include "iengine/imposter.h"
#include "iengine/sharevar.h"

#include "igeom/trimesh.h"

#include "imap/services.h"

#include "imesh/objmodel.h"

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

    iSharedVariable *var;

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
}
CS_PLUGIN_NAMESPACE_END(csparser)
