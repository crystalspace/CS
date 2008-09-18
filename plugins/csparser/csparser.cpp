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
#include "csqint.h"

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
#include "iengine/material.h"
#include "iengine/movable.h"
#include "iengine/portalcontainer.h"
#include "iengine/renderloop.h"
#include "iengine/texture.h"
#include "iengine/sharevar.h"

#include "igeom/trimesh.h"

#include "igraphic/animimg.h"

#include "imap/ldrctxt.h"
#include "imap/services.h"

#include "imesh/object.h"
#include "imesh/objmodel.h"

#include "itexture/iproctex.h"

#include "iutil/object.h"
#include "iutil/stringarray.h"
#include "iutil/vfs.h"

#include "ivaria/reporter.h"

#include "ivideo/material.h"

#include "csthreadedloader.h"
#include "loadtex.h"

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{
#define PLUGIN_LEGACY_TEXTYPE_PREFIX  "crystalspace.texture.loader."

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

  bool csThreadedLoader::ParseMaterialList (iLoaderContext* ldr_context,
    iDocumentNode* node, csWeakRefArray<iMaterialWrapper> &materialArray, const char* prefix)
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
      case XMLTOKEN_MATERIAL:
        if (!ParseMaterial (ldr_context, child, materialArray, prefix))
          return false;
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return false;
      }
    }

    return true;
  }

  bool csThreadedLoader::ParseTextureList (iLoaderContext* ldr_context, iDocumentNode* node,
    csSafeCopyArray<ProxyTexture> &proxyTextures)
  {
    if (!ImageLoader)
    {
      SyntaxService->ReportError (
        "crystalspace.maploader.parse.textures",
        node, "Image loader is missing!");
      return false;
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
      case XMLTOKEN_TEXTURE:
        if (!ParseTexture (ldr_context, child, proxyTextures))
        {
          failedTextures->Push(child->GetAttributeValue("name"));
          return false;
        }
        break;
      case XMLTOKEN_CUBEMAP:
        if (!ParseCubemap (ldr_context, child))
        {
          failedTextures->Push(child->GetAttributeValue("name"));
          return false;
        }
        break;
      case XMLTOKEN_TEXTURE3D:
        if (!ParseTexture3D (ldr_context, child))
        {
          failedTextures->Push(child->GetAttributeValue("name"));
          return false;
        }
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return false;
      }
    }

    return true;
  }

  bool csThreadedLoader::ParseTexture (iLoaderContext* ldr_context,
    iDocumentNode* node, csSafeCopyArray<ProxyTexture> &proxyTextures)
  {
    const char* txtname = node->GetAttributeValue ("name");

    iTextureWrapper* t = ldr_context->FindTexture (txtname);
    if (t)
    {
      ldr_context->AddToCollection(t->QueryObject ());
      return true;
    }

    if(!AddLoadingTexture(txtname))
    {
      t = ldr_context->FindTexture (txtname);
      if (t)
      {
        ldr_context->AddToCollection(t->QueryObject ());
        return true;
      }
    }

    static bool deprecated_warned = false;

    csRef<iTextureWrapper> tex;
    csRef<iLoaderPlugin> plugin;

    csString filename = node->GetAttributeValue ("file");
    csColor transp (0, 0, 0);
    bool do_transp = false;
    bool keep_image = false;
    bool always_animate = false;
    TextureLoaderContext context (txtname);
    csRef<iDocumentNode> ParamsNode;
    csString type;
    csAlphaMode::AlphaType alphaType = csAlphaMode::alphaNone;
    bool overrideAlphaType = false;

    csRefArray<iDocumentNode> key_nodes;

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_KEY:
        key_nodes.Push (child);
        break;
      case XMLTOKEN_FOR2D:
        {
          bool for2d;
          if (!SyntaxService->ParseBool (child, for2d, true))
          {
            RemoveLoadingTexture(txtname);
            return false;
          }
          if (for2d)
            context.SetFlags (context.GetFlags() | CS_TEXTURE_2D);
          else
            context.SetFlags (context.GetFlags() & ~CS_TEXTURE_2D);
        }
        break;
      case XMLTOKEN_FOR3D:
        {
          bool for3d;
          if (!SyntaxService->ParseBool (child, for3d, true))
          {
            RemoveLoadingTexture(txtname);
            return false;
          }
          if (for3d)
            context.SetFlags (context.GetFlags() | CS_TEXTURE_3D);
          else
            context.SetFlags (context.GetFlags() & ~CS_TEXTURE_3D);
        }
        break;
      case XMLTOKEN_TRANSPARENT:
        do_transp = true;
        if (!SyntaxService->ParseColor (child, transp))
          return false;
        break;
      case XMLTOKEN_FILE:
        {
          const char* fname = child->GetContentsValue ();
          if (!fname)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.texture",
              child, "Expected VFS filename for 'file'!");
            RemoveLoadingTexture(txtname);
            return false;
          }
          filename = fname;
        }
        break;
      case XMLTOKEN_MIPMAP:
        {
          bool mm;
          if (!SyntaxService->ParseBool (child, mm, true))
          {
            RemoveLoadingTexture(txtname);
            return false;
          }
          if (!mm)
            context.SetFlags (context.GetFlags() | CS_TEXTURE_NOMIPMAPS);
          else
            context.SetFlags (context.GetFlags() & ~CS_TEXTURE_NOMIPMAPS);
        }
        break;
      case XMLTOKEN_NPOTS:
        {
          bool npots;
          if (!SyntaxService->ParseBool (child, npots, true))
          {
            RemoveLoadingTexture(txtname);
            return false;
          }
          if (npots)
            context.SetFlags (context.GetFlags() | CS_TEXTURE_NPOTS);
          else
            context.SetFlags (context.GetFlags() & ~CS_TEXTURE_NPOTS);
        }
        break;
      case XMLTOKEN_KEEPIMAGE:
        {
          if (!SyntaxService->ParseBool (child, keep_image, true))
          {
            RemoveLoadingTexture(txtname);
            return false;
          }
        }
        break;
      case XMLTOKEN_PARAMS:
        ParamsNode = child;
        break;
      case XMLTOKEN_TYPE:
        type = child->GetContentsValue ();
        if (type.IsEmpty ())
        {
          SyntaxService->ReportError (
            "crystalspace.maploader.parse.texture",
            child, "Expected plugin ID for <type>!");
          RemoveLoadingTexture(txtname);
          return false;
        }
        break;
      case XMLTOKEN_SIZE:
        {
          csRef<iDocumentAttribute> attr_w, attr_h;
          if ((attr_w = child->GetAttribute ("width")) &&
            (attr_h = child->GetAttribute ("height")))
          {
            context.SetSize (attr_w->GetValueAsInt(),
              attr_h->GetValueAsInt());
          }
        }
        break;
      case XMLTOKEN_ALWAYSANIMATE:
        if (!SyntaxService->ParseBool (child, always_animate, true))
          return false;
        break;
      case XMLTOKEN_CLAMP:
        {
          bool c;
          if (!SyntaxService->ParseBool (child, c, true))
          {
            RemoveLoadingTexture(txtname);
            return false;
          }
          if (c)
            context.SetFlags (context.GetFlags() | CS_TEXTURE_CLAMP);
          else
            context.SetFlags (context.GetFlags() & ~CS_TEXTURE_CLAMP);
        }
        break;
      case XMLTOKEN_FILTER:
        {
          bool c;
          if (!SyntaxService->ParseBool (child, c, true))
          {
            RemoveLoadingTexture(txtname);
            return false;
          }
          if (c)
            context.SetFlags (context.GetFlags() & ~CS_TEXTURE_NOFILTER);
          else
            context.SetFlags (context.GetFlags() | CS_TEXTURE_NOFILTER);
        }
        break;
      case XMLTOKEN_CLASS:
        {
          context.SetClass (child->GetContentsValue ());
        }
        break;
      case XMLTOKEN_ALPHA:
        {
          csAlphaMode am;
          if (!SyntaxService->ParseAlphaMode (child, 0, am, false))
          {
            RemoveLoadingTexture(txtname);
            return false;
          }
          overrideAlphaType = true;
          alphaType = am.alphaType;
        }
        break;
      default:
        {
          SyntaxService->ReportBadToken (child);
          RemoveLoadingTexture(txtname);
          return false;
        }
      }
    }

    csString texClass = context.GetClass();

    // Proxy texture loading if the loader isn't specified
    // and we don't need to load them immediately.
    if(txtname && type.IsEmpty() && ldr_context->GetKeepFlags() == KEEP_USED &&
      ldr_context->GetCollection())
    {
      if (filename.IsEmpty())
      {
        filename = txtname;
      }

      // Get absolute path (on VFS) of the file.
      csRef<iDataBuffer> absolutePath = vfs->ExpandPath(filename);
      filename = absolutePath->GetData();

      ProxyTexture proxTex;
      csRef<iLoader> loader = scfQueryInterface<iLoader>(this);
      proxTex.img.AttachNew (new ProxyImage (loader, filename, object_reg));
      proxTex.always_animate = always_animate;

      tex = Engine->GetTextureList()->CreateTexture (proxTex.img);
      tex->SetTextureClass(context.GetClass());
      tex->SetFlags(context.GetFlags());
      tex->QueryObject()->SetName(txtname);
      AddTextureToList(tex);
      RemoveLoadingTexture(txtname);

      proxTex.alphaType = csAlphaMode::alphaNone;
      if(overrideAlphaType)
      {
        proxTex.alphaType = alphaType;
      }

      if(keep_image)
        tex->SetKeepImage(true);

      proxTex.keyColour.do_transp = do_transp;
      if(do_transp)
      {
        proxTex.keyColour.colours = transp;
      }

      proxTex.textureWrapper = tex;
      ldr_context->AddToCollection(proxTex.textureWrapper->QueryObject());
      proxyTextures.Push(proxTex);

      return true;
    }

    // @@@ some more comments
    if (type.IsEmpty () && filename.IsEmpty ())
    {
      filename = txtname;
    }

    iTextureManager* texman;
    texman = g3d ? g3d->GetTextureManager() : 0;
    int Format;
    Format = texman ? texman->GetTextureFormat () : CS_IMGFMT_TRUECOLOR;
    csRef<iLoaderPlugin> BuiltinImageTexLoader;
    if (!filename.IsEmpty ())
    {
      csRef<iThreadReturn> ret = csPtr<iThreadReturn>(new csLoaderReturn(threadman));
      if(!LoadImageTC (ret, filename, Format, false))
      {
        SyntaxService->Report("crystalspace.maploader.parse.texture",
          CS_REPORTER_SEVERITY_ERROR, node, "Could not load image %s!", filename.GetData());
      }

      csRef<iImage> image = scfQueryInterfaceSafe<iImage>(ret->GetResultRefPtr());
      context.SetImage (image);
      if (image.IsValid() && type.IsEmpty ())
      {
        // special treatment for animated textures
        csRef<iAnimatedImage> anim = scfQueryInterface<iAnimatedImage> (image);
        if (anim && anim->IsAnimated())
        {
          type = PLUGIN_TEXTURELOADER_ANIMIMG;
        }
        else
        {
          csImageTextureLoader* itl = new csImageTextureLoader (0);
          itl->Initialize (object_reg);
          BuiltinImageTexLoader.AttachNew(itl);
          plugin = BuiltinImageTexLoader;
        }
      }
    }

    iLoaderPlugin* Plug; Plug = 0;
    iBinaryLoaderPlugin* Binplug;
    if ((!type.IsEmpty ()) && !plugin)
    {
      iDocumentNode* defaults = 0;
      if (!loaded_plugins.FindPlugin (type, Plug, Binplug, defaults))
      {
        if ((type == "dots") ||
          (type == "plasma") ||
          (type == "water") ||
          (type == "fire"))
        {
          // old style proctex type
          if (!deprecated_warned)
          {
            SyntaxService->Report (
              "crystalspace.maploader.parse.texture",
              CS_REPORTER_SEVERITY_NOTIFY,
              node,
              "Deprecated syntax used for proctex! "
              "Specify a plugin classid or map the old types to their "
              "plugin counterparts in the <plugins> node.");
            deprecated_warned = true;
          }

          csString newtype = PLUGIN_LEGACY_TEXTYPE_PREFIX;
          newtype += type;
          type = newtype;

          loaded_plugins.FindPlugin (type, Plug, Binplug, defaults);
        }
      }
      plugin = Plug;

      if (defaults != 0)
      {
        ReportWarning (
          "crystalspace.maploader.parse.texture",
          node, "'defaults' section is ignored for textures!");
      }
    }

    if ((!type.IsEmpty ()) && !plugin)
    {
      SyntaxService->Report (
        "crystalspace.maploader.parse.texture",
        CS_REPORTER_SEVERITY_WARNING,
        node, "Could not get plugin '%s', using default", (const char*)type);

      if (!BuiltinImageTexLoader)
      {
        csImageTextureLoader* itl = new csImageTextureLoader (0);
        itl->Initialize (object_reg);
        BuiltinImageTexLoader.AttachNew (itl);
      }
      plugin = BuiltinImageTexLoader;
    }
    if (plugin)
    {
      csRef<iBase> b = plugin->Parse (ParamsNode,
        0/*ssource*/, ldr_context, static_cast<iBase*> (&context), failedMeshFacts);
      if (b) tex = scfQueryInterface<iTextureWrapper> (b);
    }

    if (!tex)
    {
      SyntaxService->Report (
        "crystalspace.maploader.parse.texture",
        CS_REPORTER_SEVERITY_WARNING,
        node, "Could not load texture '%s', using checkerboard instead", txtname);

      csRef<iLoaderPlugin> BuiltinErrorTexLoader;
      BuiltinErrorTexLoader.AttachNew(new csMissingTextureLoader (0));
      csRef<iBase> b = BuiltinErrorTexLoader->Parse (ParamsNode,
        0, ldr_context, static_cast<iBase*> (&context), failedMeshFacts);
      if (!b.IsValid())
      {
        static bool noMissingWarned = false;
        if (!noMissingWarned)
        {
          SyntaxService->Report (
            "crystalspace.maploader.parse.texture",
            CS_REPORTER_SEVERITY_ERROR,
            node, "Could not create default texture!");
          noMissingWarned = true;
          RemoveLoadingTexture(txtname);
          return false;
        }
      }
      tex = scfQueryInterface<iTextureWrapper> (b);
    }

    if (tex)
    {
      CS_ASSERT_MSG("Texture loader did not register texture", 
        tex->GetTextureHandle());
      tex->QueryObject ()->SetName (txtname);
      if (keep_image) tex->SetKeepImage (true);
      if (do_transp)
        tex->SetKeyColor (csQint (transp.red * 255.99),
        csQint (transp.green * 255.99), csQint (transp.blue * 255.99));
      tex->SetTextureClass (context.GetClass ());
      if (overrideAlphaType)
        tex->GetTextureHandle()->SetAlphaType (alphaType);

      csRef<iProcTexture> ipt = scfQueryInterface<iProcTexture> (tex);
      if (ipt)
        ipt->SetAlwaysAnimate (always_animate);
      ldr_context->AddToCollection(tex->QueryObject ());

      size_t i;
      for (i = 0 ; i < key_nodes.GetSize () ; i++)
      {
        if (!ParseKey (key_nodes[i], tex->QueryObject()))
        {
          RemoveLoadingTexture(txtname);
          return false;
        }
      }
    }

    if (texman)
    {
      if (!tex->GetTextureHandle ()) tex->Register (texman);
    }

    AddTextureToList(tex);
    RemoveLoadingTexture(txtname);
    return true;
  }

  bool csThreadedLoader::ParseMaterial (iLoaderContext* ldr_context,
    iDocumentNode* node, csWeakRefArray<iMaterialWrapper> &materialArray, const char *prefix)
  {
    const char* matname = node->GetAttributeValue ("name");

    iMaterialWrapper* m = ldr_context->FindMaterial (matname);
    if (m)
    {
      ldr_context->AddToCollection(m->QueryObject ());
      return true;
    }

    if(!AddLoadingMaterial(matname))
    {
      m = ldr_context->FindMaterial (matname);
      if(m)
      {
        ldr_context->AddToCollection(m->QueryObject ());
        return true;
      }
    }

    iTextureWrapper* texh = 0;
    bool col_set = false;
    csColor col;

    bool shaders_mentioned = false;	// If true there were shaders.
    csArray<csStringID> shadertypes;
    csArray<iShader*> shaders;
    csRefArray<csShaderVariable> shadervars;

    csRefArray<iDocumentNode> key_nodes;

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_KEY:
        key_nodes.Push (child);
        break;
      case XMLTOKEN_TEXTURE:
        {
          const char* txtname = child->GetContentsValue ();
          texh = ldr_context->FindTexture (txtname);
          csTicks start = csGetTicks();
          while (!texh && (csGetTicks() - start < 60000))
          {
            texh = ldr_context->FindTexture (txtname);
          }
          if (!texh)
          {
            ReportError (
              "crystalspace.maploader.parse.material",
              "Cannot find texture '%s' for material `%s'", txtname, matname);
            RemoveLoadingMaterial(matname);
            return false;
          }
        }
        break;
      case XMLTOKEN_COLOR:
        {
          col_set = true;
          if (!SyntaxService->ParseColor (child, col))
          {
            RemoveLoadingMaterial(matname);
            return false;
          }
        }
        break;
      case XMLTOKEN_DIFFUSE:
      case XMLTOKEN_AMBIENT:
      case XMLTOKEN_REFLECTION:
        ReportWarning ("crystalspace.maploader.parse.material",
          child, "Syntax not supported any more. Use shader variables instead");
        break;
      case XMLTOKEN_SHADER:
        {
          shaders_mentioned = true;
          csRef<iShaderManager> shaderMgr = 
            csQueryRegistry<iShaderManager> (object_reg);
          if (!shaderMgr)
          {
            ReportNotify ("iShaderManager not found, ignoring shader!");
            break;
          }
          const char* shadername = child->GetContentsValue ();
          iShader* shader = ldr_context->FindShader (shadername);
          if (!shader)
          {
            ReportNotify (
              "Shader (%s) couldn't be found for material %s, ignoring it",
              shadername, matname);
            break;
          }
          const char* shadertype = child->GetAttributeValue ("type");
          if (!shadertype)
          {
            ReportNotify (
              "No shadertype for shader %s in material %s, ignoring it",
              shadername, matname);
            break;
          }
          shadertypes.Push (stringSet->Request(shadertype));
          shaders.Push (shader);
        }
        break;
      case XMLTOKEN_SHADERVAR:
        {
          //create a new variable
          csRef<csShaderVariable> var;
          var.AttachNew (new csShaderVariable);

          if (!SyntaxService->ParseShaderVar (ldr_context, child, *var, failedTextures))
          {
            break;
          }
          shadervars.Push (var);
        }
        break;
      default:
        {
          SyntaxService->ReportBadToken (child);
          RemoveLoadingMaterial(matname);
          return false;
        }
      }
    }

    csRef<iMaterial> material = Engine->CreateBaseMaterial (texh);

    if (col_set)
    {
      csShaderVariable* flatSV = material->GetVariableAdd (
        stringSetSvName->Request (CS_MATERIAL_VARNAME_FLATCOLOR));
      flatSV->SetValue (col);
    }

    csRef<iMaterialWrapper> mat;

    if (prefix)
    {
      char *prefixedname = new char [strlen (matname) + strlen (prefix) + 2];
      strcpy (prefixedname, prefix);
      strcat (prefixedname, "_");
      strcat (prefixedname, matname);
      mat = Engine->GetMaterialList ()->CreateMaterial (material, prefixedname);
      delete [] prefixedname;
    }
    else
    {
      mat = Engine->GetMaterialList ()->CreateMaterial (material, matname);
    }
    AddMaterialToList(mat);
    RemoveLoadingMaterial(matname);

    size_t i;
    for (i=0; i<shaders.GetSize (); i++)
      //if (shaders[i]->Prepare ())
      material->SetShader (shadertypes[i], shaders[i]);
    for (i=0; i<shadervars.GetSize (); i++)
      material->AddVariable (shadervars[i]);

    // dereference material since mat already incremented it

    for (i = 0 ; i < key_nodes.GetSize () ; i++)
    {
      if (!ParseKey (key_nodes[i], mat->QueryObject()))
        return false;
    }
    ldr_context->AddToCollection(mat->QueryObject ());

    materialArray.Push(mat);

    return true;
  }

  /// Parse a Cubemap texture definition and add the texture to the engine
  iTextureWrapper* csThreadedLoader::ParseCubemap (iLoaderContext* ldr_context,
    iDocumentNode* node)
  {
    static bool cubemapDeprecationWarning = false;
    if (!cubemapDeprecationWarning)
    {
      cubemapDeprecationWarning = true;
      SyntaxService->Report ("crystalspace.maploader.parse.texture",
        CS_REPORTER_SEVERITY_WARNING, node,
        "'<cubemap>...' is deprecated, use '<texture><type>"
        PLUGIN_TEXTURELOADER_CUBEMAP "</type><params>...' instead");
    }

    csRef<csCubemapTextureLoader> plugin;
    plugin.AttachNew (new csCubemapTextureLoader (0));
    plugin->Initialize (object_reg);

    csRef<TextureLoaderContext> context;
    const char* txtname = node->GetAttributeValue ("name");
    context.AttachNew (new TextureLoaderContext (txtname));

    csRef<iBase> b = plugin->Parse (node, 0/*ssource*/, ldr_context, context);
    csRef<iTextureWrapper> tex;
    if (b) tex = scfQueryInterface<iTextureWrapper> (b);

    if (tex)
    {
      tex->QueryObject ()->SetName (txtname);
      ldr_context->AddToCollection(tex->QueryObject ());
      iTextureManager* tm = g3d ? g3d->GetTextureManager() : 0;
      if (tm) tex->Register (tm);
    }

    return tex;
  }

  iTextureWrapper* csThreadedLoader::ParseTexture3D (iLoaderContext* ldr_context,
    iDocumentNode* node)
  {
    static bool volmapDeprecationWarning = false;
    if (!volmapDeprecationWarning)
    {
      volmapDeprecationWarning = true;
      SyntaxService->Report ("crystalspace.maploader.parse.texture",
        CS_REPORTER_SEVERITY_WARNING, node,
        "'<texture3d>...' is deprecated, use '<texture><type>"
        PLUGIN_TEXTURELOADER_TEX3D "</type><params>...' instead");
    }

    csRef<csTexture3DLoader> plugin;
    plugin.AttachNew (new csTexture3DLoader (0));
    plugin->Initialize (object_reg);

    csRef<TextureLoaderContext> context;
    const char* txtname = node->GetAttributeValue ("name");
    context.AttachNew (new TextureLoaderContext (txtname));

    csRef<iBase> b = plugin->Parse (node, 0/*ssource*/, ldr_context, context);
    csRef<iTextureWrapper> tex;
    if (b) tex = scfQueryInterface<iTextureWrapper> (b);

    if (tex)
    {
      tex->QueryObject ()->SetName (txtname);
      ldr_context->AddToCollection(tex->QueryObject ());
      iTextureManager* tm = g3d ? g3d->GetTextureManager() : 0;
      if (tm) tex->Register (tm);
    }

    return tex;
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
          if (!SyntaxService->ParseShaderVar (ldr_context, child, *var, failedTextures))
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

  bool csThreadedLoader::ParseShaderList (iLoaderContext* ldr_context,
    iDocumentNode* node)
  {
    csRef<iShaderManager> shaderMgr (
      csQueryRegistry<iShaderManager> (object_reg));

    if(!shaderMgr)
    {
      ReportNotify ("iShaderManager not found, ignoring shaders!");
      return true;
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
      case XMLTOKEN_SHADER:
        {
          ParseShader (ldr_context, child, shaderMgr);
        }
        break;
      }
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
    iDocumentNode* node, iStreamSource* ssource)
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
          LoadMeshRef(child, sector, ldr_context, ssource);
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
          csRef<iThreadReturn> itr = LoadMeshObject (ldr_context, mesh, 0, child, ssource, sector);
          AddLoadingMeshObject(meshname, itr);
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
          csRef<iThreadReturn> itr = LoadMeshObject (ldr_context, mesh, 0, child, ssource, sector);
          AddLoadingMeshObject(meshname, itr);
        }
        break;
      case XMLTOKEN_LIGHT:
        {
          iLight* sl = ParseStatlight (ldr_context, child);
          if (!sl) return 0;
          sector->AddLight (sl);
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
      vfs->ChDir(dir);
    }
    csRef<iBase> base = plugin->Parse(node, ssource, ldr_context, context, failedMeshFacts);
    ret->SetResult(base);
    return base.IsValid();    
  }

  THREADED_CALLABLE_IMPL5(csThreadedLoader, ParseAddOnBinary, csRef<iBinaryLoaderPlugin> plugin,
    csRef<iDataBuffer> dbuf, csRef<iStreamSource> ssource, csRef<iLoaderContext> ldr_context,
    csRef<iBase> context)
  {
    csRef<iBase> base = plugin->Parse(dbuf, ssource, ldr_context, context, failedMeshFacts);
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
