/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
   $Id$

    This file is part of Crystal Space Virtual Object System Abstract
    3D Layer plugin (csvosa3dl).

    Copyright (C) 2004 Peter Amstutz

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "cssysdef.h"

// Hack: Work around problems caused by #defining 'new'.
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# undef new
#endif
#include <new>

#include "csgfx/memimage.h"
#include "csgfx/xorpat.h"
#include "iutil/objreg.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/txtmgr.h"

#include "vosmaterial.h"
#include "vostexture.h"
#include "vosobject3d.h"

csRef<iMaterialWrapper> csMetaMaterial::checkerboard;
iObjectRegistry* csMetaMaterial::object_reg;

using namespace VUtil;
using namespace VOS;

/// MaterialUpdateTask ///

class MaterialUpdateTask : public Task
{
public:
  vRef<csMetaMaterial> mm;
  bool* needListener;
  virtual void doTask();
};


void MaterialUpdateTask::doTask()
{
  if(*needListener) {
    mm->addChildListener(mm);
    *needListener = false;
  }

  // now go through our parents and let them know that this material is ready
  for(ParentSetIterator i = mm->getParents(); i.hasMore(); i++) {
    if((*i)->getContextualName() == "a3dl:material") {
      vRef<csMetaObject3D> obj3d = meta_cast<csMetaObject3D>((*i)->getParent());
      if(obj3d.isValid()) obj3d->updateMaterial();
    }
  }
}


struct TextureLayer
{
  float uscale, vscale;
  int mode;
};

/// ConstructMaterialTask ///

class ConstructMaterialTask : public Task
{
public:
  vRef<csMetaTexture> base;
  std::vector<csMetaTexture*> layers;
  TextureLayer* coords;
  vRef<csMetaMaterial> metamaterial;
  iObjectRegistry *object_reg;
  bool iscolor;
  float R, G, B;
  bool* needListener;

  ConstructMaterialTask(iObjectRegistry *objreg, csMetaMaterial* mm);
  virtual ~ConstructMaterialTask();
  virtual void doTask();
};

ConstructMaterialTask::ConstructMaterialTask(iObjectRegistry *objreg,
                                             csMetaMaterial* mm)
  : coords(0), metamaterial(mm, true), object_reg(objreg)
{
}

ConstructMaterialTask::~ConstructMaterialTask()
{
}

void ConstructMaterialTask::doTask()
{
  LOG("vosmaterial", 3, "Constructing material");

  csRef<iEngine> engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  csRef<iGraphics3D> g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  csRef<iTextureManager> txtmgr = g3d->GetTextureManager();

  csRef<iMaterial> imat;

  if(base.isValid())
  {
    csRef<iTextureWrapper> basetw = base->GetTextureWrapper();

    if(layers.size() > 0)
    {
      iTextureWrapper** layertws = (iTextureWrapper**)malloc(
        layers.size() * sizeof(iTextureWrapper*));
      for(unsigned int i = 0; i < layers.size(); i++)
      {
        layertws[i] = layers[i]->GetTextureWrapper();
        layers[i]->release();
      }

      imat = engine->CreateBaseMaterial(basetw);

      // XXX add texture layers using shaders

      free(layertws);
      free(coords);
    }
    else
    {
      imat = engine->CreateBaseMaterial(basetw);
    }
  }
  else
  {
    if(iscolor)
    {

      // we tried some alternate ways of doing colors!
    /*
      csRef<iMaterial> color  = engine->CreateBaseMaterial(0);
      int red = strtol(newproperty.substr(1, 2).c_str(), 0, 16);
      int green = strtol(newproperty.substr(3, 2).c_str(), 0, 16);
      int blue = strtol(newproperty.substr(5, 2).c_str(), 0, 16);
      color->SetFlatColor(csRGBcolor(red, green, blue));
      if(!material) {
      material = engine->GetMaterialList()->NewMaterial(color,
        p.getURL().getString().c_str());
      }
      material->SetMaterial(color);
      material->Register(txtmgr);
    */

    /*
      csRef<iMaterial> color = engine->CreateBaseMaterial(0);
      color->SetFlatColor(csRGBcolor((int)(r*255), (int)(g*255), (int)(b*255)));
      if(!material) {
      material = engine->GetMaterialList()->NewMaterial(color,
        p.getURL().getString().c_str());
      }
      material->SetMaterial(color);
      material->Register(txtmgr);
    */


      iTextureWrapper* txtwrap;
      txtwrap = engine->CreateBlackTexture(metamaterial->getURLstr().c_str(),
                                           64, 64, 0, CS_TEXTURE_3D);

      csImageMemory* img = new csImageMemory(64, 64);
      csRGBpixel px((int)(R*255.0), (int)(G*255.0), (int)(B*255.0));
      img->Clear(px);
      txtwrap->SetImageFile(img);

      txtwrap->Register(txtmgr);

      imat = engine->CreateBaseMaterial(txtwrap);
    }
  }

  if(imat.IsValid())
  {
    csRef<iMaterialWrapper> material = engine->GetMaterialList()->NewMaterial(
      imat, 0);
    if(!material) return;
    //material->Register(txtmgr);

    if(metamaterial->materialwrapper.IsValid())
      engine->GetMaterialList()->Remove(metamaterial->materialwrapper);

    metamaterial->materialwrapper = material;

    MaterialUpdateTask* mut = new MaterialUpdateTask;
    mut->mm = metamaterial;
    mut->needListener = needListener;
    TaskQueue::defaultTQ().addTask(mut);
  }
}


/// csMetaMaterial ///

csMetaMaterial::csMetaMaterial(VobjectBase* superobject)
  : A3DL::Material(superobject), alreadyLoaded(false), needListener(true)
{
}

csMetaMaterial::~csMetaMaterial()
{
}

void csMetaMaterial::CreateCheckerboard()
{
  csRef<iEngine> engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  csRef<iGraphics3D> g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);

  csRef<iImage> im = csCreateXORPatternImage(64, 64, 6);
  csRef<iTextureManager> txtmgr = g3d->GetTextureManager();
  csRef<iTextureHandle> th = txtmgr->RegisterTexture (im, CS_TEXTURE_3D);
  csRef<iTextureWrapper> tw = engine->GetTextureList()->NewTexture(th);
  tw->SetImageFile(im);
  csRef<iMaterial> mat = engine->CreateBaseMaterial(tw);
  checkerboard = engine->GetMaterialList()->NewMaterial (mat, 0);
  //checkerboard->Register(txtmgr);
}

  /** Return CS iMaterialWrapper interface for this object */
csRef<iMaterialWrapper> csMetaMaterial::GetCheckerboard()
{
  if(! checkerboard.IsValid()) CreateCheckerboard();
  return checkerboard;
}

void csMetaMaterial::Setup(csVosA3DL* vosa3dl)
{
  if(alreadyLoaded) return;
  else alreadyLoaded = true;

  this->vosa3dl = vosa3dl;

  LOG("csMetaMaterial", 3, "setting up material");
  ConstructMaterialTask* cmt = new ConstructMaterialTask(
    vosa3dl->GetObjectRegistry(), this);

  cmt->needListener = &needListener;

  A3DL::TextureIterator txt = getTextureLayers();
  if(txt.hasMore())
  {
    vRef<csMetaTexture> base = meta_cast<csMetaTexture>(*txt);
    if(base.isValid())
    {
      base->Setup(vosa3dl);
      cmt->base = base;
    }

    txt++;
    if(txt.hasMore())
    {
      cmt->coords = (TextureLayer*)malloc(txt.remaining()
                                          * sizeof(TextureLayer));
      float uscale = 1, vscale = 1, ushift = 0, vshift = 0;
      for(int i = 0; txt.hasMore(); txt++, i++)
      {
        vRef<csMetaTexture> mt = meta_cast<csMetaTexture>(*txt);
        if(mt.isValid())
        {
          mt->Setup(vosa3dl);
          mt->acquire();
          cmt->layers.push_back(mt);
          try
          {
            mt->getUVScaleAndShift(uscale, vscale, ushift, vshift);
          }
          catch(std::runtime_error& e)
          {
            uscale = 1; vscale = 1; ushift = 0; vshift = 0;
          }
          cmt->coords[i].uscale = uscale;
          cmt->coords[i].vscale = vscale;
          //cmt->coords[i].ushift = ushift;
          //cmt->coords[i].vshift = vshift;

          switch(mt->getBlendMode())
          {
            case A3DL::Material::BLEND_NORMAL:
              try
              {
                double alpha = mt->getAlpha();
                if(alpha == 0.0)
                  cmt->coords[i].mode = CS_FX_TRANSPARENT;
                else if(alpha == 1.0)
                  cmt->coords[i].mode = CS_FX_COPY;
                else
                  cmt->coords[i].mode = CS_FX_SETALPHA(alpha);
              }
              catch(...)
              {
                cmt->coords[i].mode = CS_FX_COPY;
              }
              break;
            case A3DL::Material::BLEND_ADD:
              cmt->coords[i].mode = CS_FX_ADD;
              break;
            case A3DL::Material::BLEND_MULTIPLY:
              cmt->coords[i].mode = CS_FX_MULTIPLY;
              break;
            case A3DL::Material::BLEND_DOUBLE_MULTIPLY:
              cmt->coords[i].mode = CS_FX_MULTIPLY2;
              break;
          }
          try
          {
            if(!mt->getShaded())
              cmt->coords[i].mode |= CS_FX_FLAT;
          }
          catch(...) { }
          cmt->coords[i].mode |= CS_FX_TILING;   // for software renderer :P
        }
      }
    }
    cmt->iscolor = false;
  }
  else
  {
    getColor(cmt->R, cmt->G, cmt->B);
    LOG("CSA3DL Material", 2,
            "CS Material: material has no textures, assuming it's color. (" <<
            cmt->R << ", " << cmt->G << ", " << cmt->B << ")");
    cmt->iscolor = true;
  }

  vosa3dl->mainThreadTasks.push(cmt);

  return;
}

void csMetaMaterial::notifyPropertyChange(const PropertyEvent& event)
{
  LOG("vosmaterial", 4, "property change!");
  try
  {
    vRef<ParentChildRelation> pcr = event.getProperty()->findParent(this);
    if(pcr->getContextualName() == "a3dl:color"
      || pcr->getContextualName() == "a3dl:texture")
    {
      alreadyLoaded = false;
      Setup(vosa3dl);
    }
  }
  catch(NoSuchObjectError) { }
  catch(AccessControlError) { }
  catch(RemoteError) { }
}

csRef<iMaterialWrapper> csMetaMaterial::GetMaterialWrapper()
{
  if(! materialwrapper.IsValid()) return GetCheckerboard();
  return materialwrapper;
}

void csMetaMaterial::notifyChildInserted(VobjectEvent& event)
{
  LOG("vosmaterial", 4, "child inserted!");

  if(event.getContextualName() == "a3dl:texture")
  {
    updateMaterial();
  }
  vRef<Property> p = meta_cast<Property>(event.getNewChild());
  if(p.isValid()) p->addPropertyListener(this);
}

void csMetaMaterial::notifyChildReplaced(VobjectEvent& event)
{
  notifyChildRemoved(event);
  notifyChildInserted(event);
}

void csMetaMaterial::notifyChildRemoved(VobjectEvent& event)
{
  if(event.getContextualName() == "a3dl:texture")
  {
    updateMaterial();
  }
  vRef<Property> p = meta_cast<Property>(event.getOldChild());
  if(p.isValid()) p->removePropertyListener(this);
}

MetaObject* csMetaMaterial::new_csMetaMaterial(VobjectBase* superobject,
  const std::string& type)
{
  return new csMetaMaterial(superobject);
}

void csMetaMaterial::updateMaterial()
{
    alreadyLoaded = false;
    Setup(vosa3dl);
}
