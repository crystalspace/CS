#include "cssysdef.h"
#include "cssaver/cssaver.h"
#include "iutil/vfs.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/string.h"
#include "iengine/engine.h"
#include "iengine/texture.h"
#include "iengine/material.h"
#include "igraphic/image.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "ivideo/txtmgr.h"
#include "ivideo/graph3d.h"
#include "imap/writer.h"
#include "csutil/xmltiny.h"
#include "csutil/scfstr.h"
#include "csgfx/rgbpixel.h"

#define ONE_OVER_256 (1.0/255.0)

SCF_IMPLEMENT_IBASE(csSaver);
  SCF_IMPLEMENTS_INTERFACE(iSaver);
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent);
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (csSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY(csSaver);

SCF_EXPORT_CLASS_TABLE (cssaver)
  SCF_EXPORT_CLASS_DEP (csSaver, "crystalspace.level.saver",
    "Level and library file saver", "crystalspace.kernel., ")
SCF_EXPORT_CLASS_TABLE_END

CS_IMPLEMENT_PLUGIN

csSaver::csSaver(iBase *p) {
  SCF_CONSTRUCT_IBASE(p);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);

  object_reg = NULL;
}

csSaver::~csSaver() {

}

bool csSaver::Initialize(iObjectRegistry *p) {
  object_reg=p;

  engine=CS_QUERY_REGISTRY(object_reg, iEngine);

  return 1;
}

csRef<iDocumentNode> csSaver::CreateNode(csRef<iDocumentNode>& parent, const char* name) {
  csRef<iDocumentNode> child = parent->CreateNodeBefore(CS_NODE_ELEMENT, NULL);
  child->SetValue (name);
  return child;
}

csRef<iDocumentNode> csSaver::CreateValueNode(csRef<iDocumentNode>& parent, const char* name, const char* value) {
  csRef<iDocumentNode> child = CreateNode(parent, name);
  csRef<iDocumentNode> text = child->CreateNodeBefore(CS_NODE_TEXT, NULL);
  text->SetValue (value);
  return child;
}

csRef<iDocumentNode> csSaver::CreateValueNodeAsFloat(csRef<iDocumentNode>& parent, const char* name, float value) {
  csRef<iDocumentNode> child = CreateNode(parent, name);
  csRef<iDocumentNode> text = child->CreateNodeBefore(CS_NODE_TEXT, NULL);
  text->SetValueAsFloat (value);
  return child;
}

csRef<iDocumentNode> csSaver::CreateValueNodeAsColor(csRef<iDocumentNode>& parent, const char* name, const csColor &color) {
  csRef<iDocumentNode> child=CreateNode(parent, name);
  child->SetAttributeAsFloat("red", color.red);
  child->SetAttributeAsFloat("green", color.green);
  child->SetAttributeAsFloat("blue", color.blue);
  return child;
}

bool csSaver::SaveTextures(csRef<iDocumentNode>& parent) {
  csRef<iDocumentNode> current = CreateNode(parent, "textures");

  iTextureList *texList=engine->GetTextureList();
  for(int i=0; i<texList->GetCount(); i++) {
    iTextureWrapper *texWrap=texList->Get(i);

    csRef<iDocumentNode> child = current->CreateNodeBefore(CS_NODE_ELEMENT, NULL);

    const char *name=texWrap->QueryObject()->GetName();
    if(name && *name) child->SetAttribute("name", name);

    int flags=texWrap->GetFlags();
    iImage *img=texWrap->GetImageFile();

    if(flags & CS_TEXTURE_PROC) {

      child->SetValue("proctex");
      CreateValueNode(child, "type", name); //TODO hack for now

    } else if(img) {

      child->SetValue("texture");
      const char *filename=img->GetName();
      if(filename && *filename) CreateValueNode(child, "file", filename);

      //TODO behle not currently read from map, so no point to write back out
/*      if(img->HasKeycolor()==1) {
        int r,g,b;
        img->GetKeycolor(r, g, b);
        CreateValueNodeAsColor(child, "transparent", csColor(r*ONE_OVER_256, g*ONE_OVER_256, b*ONE_OVER_256));
      }*/

    } else {
      CS_ASSERT(0);
    }
  }
  return 1;
}

bool csSaver::SaveMaterials(csRef<iDocumentNode>& parent) {
  csRef<iDocumentNode> current = CreateNode(parent, "materials");

  iMaterialList *matList=engine->GetMaterialList();
  for(int i=0; i<matList->GetCount(); i++) {
    iMaterialWrapper *matWrap=matList->Get(i);
    CS_ASSERT(matWrap);
    iMaterial *mat=matWrap->GetMaterial();
    CS_ASSERT(mat);
    csRef<iMaterialEngine> matEngine(SCF_QUERY_INTERFACE(mat, iMaterialEngine));
    CS_ASSERT(matEngine);

    iTextureWrapper *texWrap=matEngine->GetTextureWrapper();
    //Don't add fake materials that wrap a procedural texture
    if(texWrap && (texWrap->GetFlags() & CS_TEXTURE_PROC) )
      continue;

    csRef<iDocumentNode> child = CreateNode(current, "material");

    const char *name=matWrap->QueryObject()->GetName();
    if(name && *name) child->SetAttribute ("name", name);

    csRGBpixel color;
    matWrap->GetMaterial()->GetFlatColor(color, 0);
    if(color.red!=255 || color.green!=255 || color.blue!=255) {
      CreateValueNodeAsColor(child, "color", csColor(color.red*ONE_OVER_256, color.green*ONE_OVER_256, color.blue*ONE_OVER_256));
    }

    if(texWrap) {
      const char *texname=texWrap->QueryObject()->GetName();
      if(texname && *texname) CreateValueNode(child, "texture", texname);
    }

    int layerCount=mat->GetTextureLayerCount();
    for(int i=0; i<layerCount; i++) {
      csRef<iDocumentNode> layerItem=CreateNode(child, "layer");

      iTextureWrapper *layerTexWrap=matEngine->GetTextureWrapper(i);
      if(layerTexWrap) {
        const char *texname=layerTexWrap->QueryObject()->GetName();
        if(texname && *texname) CreateValueNode(layerItem, "texture", texname);
      }

      csTextureLayer *texLayer=mat->GetTextureLayer(i);
      if(texLayer->uscale!=1.0f || texLayer->vscale!=1.0f) {
        csRef<iDocumentNode> scaleItem=CreateNode(layerItem, "scale");
        scaleItem->SetAttributeAsFloat("u", texLayer->uscale);
        scaleItem->SetAttributeAsFloat("v", texLayer->vscale);
      }
      if(texLayer->ushift!=0.0f || texLayer->vshift!=0.0f) {
        csRef<iDocumentNode> shiftItem=CreateNode(layerItem, "shift");
        shiftItem->SetValue("shift");
        shiftItem->SetAttributeAsFloat("u", texLayer->ushift);
        shiftItem->SetAttributeAsFloat("v", texLayer->vshift);
      }

      if(texLayer->mode != (CS_FX_ADD|CS_FX_TILING) ) {
        int blendmode=texLayer->mode & CS_FX_MASK_MIXMODE;

        csRef<iDocumentNode> mixmodeItem=CreateNode(layerItem, "mixmode");
        if(blendmode == CS_FX_ALPHA) {
          int alpha=texLayer->mode & CS_FX_MASK_ALPHA;
          CreateValueNodeAsFloat(mixmodeItem, "alpha", alpha*ONE_OVER_256);
        }
        if(blendmode == CS_FX_ADD) {
          CreateNode(mixmodeItem, "add");
        }
        if(blendmode == CS_FX_MULTIPLY) {
          CreateNode(mixmodeItem, "multiply");
        }
        if(blendmode == CS_FX_MULTIPLY2) {
          CreateNode(mixmodeItem, "multiply2");
        }
        if(blendmode == CS_FX_TRANSPARENT) {
          CreateNode(mixmodeItem, "transparent");
        }
        if(texLayer->mode & CS_FX_KEYCOLOR) {
          CreateNode(mixmodeItem, "keycolor");
        }
        if(texLayer->mode & CS_FX_TILING) {
          CreateNode(mixmodeItem, "tiling");
        }
      }
    }
  }
  return 1;
}

iString* csSaver::SaveMapFile() {
  csRef<iDocumentSystem> xml(csPtr<iDocumentSystem>(new csTinyDocumentSystem()));
  csRef<iDocument> doc = xml->CreateDocument();
  csRef<iDocumentNode> root = doc->CreateRoot();
  csRef<iDocumentNode> parent = root->CreateNodeBefore(CS_NODE_ELEMENT, NULL);
  parent->SetValue("world");

  if(!SaveTextures(parent)) return NULL;
  if(!SaveMaterials(parent)) return NULL;

  iString* str=new scfString();
  if(doc->Write(str)!=NULL) {
    delete str;
    return NULL;
  }

  return str;
}

bool csSaver::SaveMapFile(const char *filename) {
  csRef<iVFS> vfs(CS_QUERY_REGISTRY(object_reg, iVFS));
  CS_ASSERT(vfs);

  csRef<iString> str(SaveMapFile());
  if(!str) return 0;

  return vfs->WriteFile(filename, str->GetData(), str->Length());
}

