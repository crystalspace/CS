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

#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"
//#include "imap/loader.h"
#include "igraphic/imageio.h"
#include "iutil/vfs.h"
#include "vostexture.h"

using namespace VOS;


class ConstructTextureTask : public Task
{
public:
  iObjectRegistry *object_reg;
  std::string texturename;
  std::string texturedata;
  std::string cachefilename;
  vRef<csMetaTexture> metatxt;

  ConstructTextureTask(iObjectRegistry *objreg, const std::string& name,
                         const std::string& cache, csMetaTexture* mt);
  virtual ~ConstructTextureTask();
  virtual void doTask();
};

ConstructTextureTask::ConstructTextureTask(iObjectRegistry *objreg,
                                           const std::string& name,
                                           const std::string& cache,
                                           csMetaTexture* mt)
  : object_reg(objreg), texturename(name), cachefilename(cache),
    metatxt(mt, true)
{
}

ConstructTextureTask::~ConstructTextureTask()
{
}

void ConstructTextureTask::doTask()
{
  csRef<iEngine> engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  csRef<iGraphics3D> g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  csRef<iTextureManager> txtmgr = g3d->GetTextureManager();
  csRef<iImageIO> io = CS_QUERY_REGISTRY (object_reg, iImageIO);

  if (!io)
  {
    LOG("ConstructTextureTask", 1,
      "Error: Could not get the iLoader plugin from object registry!");
    return;
  }
  if(!txtmgr)
  {
    LOG("ConstructTextureTask", 1, "No texture manager");
    return;
  }

  csRef<iImage> image (io->Load ((uint8*) texturedata.c_str(),
                   texturedata.size(),
                 engine->GetTextureFormat()));

  csRef<iTextureHandle> handle (txtmgr->RegisterTexture (image, CS_TEXTURE_3D));

  csRef<iTextureWrapper> texture = engine->GetTextureList()->NewTexture ( handle);

  if(!texture)
  {
    LOG("ConstructTextureTask", 1,
      "Error: could not create texture (would be in cache file \""
      << cachefilename << "\")!");
    return;
  }

//  texture->Register (txtmgr);
  texture->GetTextureHandle()->Prepare ();

  metatxt->texturewrapper = texture;
}

csMetaTexture::csMetaTexture(VobjectBase* superobject)
    : A3DL::Texture(superobject), alreadyLoaded(false)
{
}

csMetaTexture::~csMetaTexture()
{
}

void csMetaTexture::Setup(csVosA3DL* vosa3dl)
{
  if(alreadyLoaded) return;
  else alreadyLoaded = true;

  vRef<Property> imagedata = getImage();
  char cachefilename[256];
  vRef<Site> site = imagedata->getSite();
  /*snprintf(cachefilename, sizeof(cachefilename), "/csvosa3dl_cache/%s/%s",
    site->getURL().getHost().c_str(),
    imagedata->getSiteName().c_str());*/
  snprintf(cachefilename, sizeof(cachefilename), "/tmp/%s_%s",
           site->getURL().getHost().c_str(),
           imagedata->getSiteName().c_str());

  // VFS uses ':' as a seperator
  for (int i=0; cachefilename[i]; i++)
  {
    if ((cachefilename[i] == ':'))
        cachefilename[i] = '_';
  }

  ConstructTextureTask* ctt = new ConstructTextureTask(
                   vosa3dl->GetObjectRegistry(), getURLstr(),
                   cachefilename, this);
  imagedata->read(ctt->texturedata);
  vosa3dl->mainThreadTasks.push(ctt);

  //addChildListener(this);

#if 0    // this code will be replaced by code using the CrystalZilla (iMozilla) plugin for text (and html etc) rendering
    if(p->getDataType() == "text/plain") {
        texture = engine->CreateBlackTexture(p->getURL().getString().c_str(), 256, 256, 0, CS_TEXTURE_3D);
        texture->Register(txtmgr);
        texture->GetTextureHandle()->Prepare ();

        g3d->SetRenderTarget (texture->GetTextureHandle(), false);

        if(g3d->BeginDraw (CSDRAW_2DGRAPHICS))
        {
            csRef<iFont> font = g3d->GetDriver2D()->GetFontServer()->LoadFont(CSFONT_LARGE);
            int fg = g3d->GetDriver2D()->FindRGB(255, 255, 255);
            int bg = g3d->GetDriver2D()->FindRGB(0, 0, 0);

            g3d->GetDriver2D()->Clear(bg);

            string str = p->read();
            string::size_type start, end;
            int pos=4;
            for(start = 0, end = 0; end < str.length(); end++) {
                for(start = end; end < str.length() && str[end] != '\n'; end++);
                g3d->GetDriver2D()->Write(font, 4, pos, fg, bg, str.substr(start, end - start).c_str());
                pos += 14;
                start = end;
            }
            g3d->GetDriver2D()->Write(font, 10, pos, fg, bg, str.substr(start, end).c_str());

            g3d->FinishDraw ();
        }
    } else
#endif
}

void csMetaTexture::notifyPropertyChange(const PropertyEvent& event)
{
  try
  {
    vRef<ParentChildRelation> pcr = event.getProperty()->findParent(*this);
    if(pcr->getContextualName() == "a3dl:image")
    {
      // XXX reload the image and stuff
    }
  }
  catch(NoSuchObjectError) { }
  catch(AccessControlError) { }
  catch(RemoteError) { }
}

csRef<iTextureWrapper> csMetaTexture::GetTextureWrapper()
{
  return texturewrapper;
}

void csMetaTexture::notifyChildInserted(VobjectEvent& event)
{
  vRef<Property> p = meta_cast<Property>(event.getNewChild());
  if(p.isValid()) p->addPropertyListener(this);
}

void csMetaTexture::notifyChildReplaced(VobjectEvent& event)
{
  notifyChildRemoved(event);
  notifyChildInserted(event);
}

void csMetaTexture::notifyChildRemoved(VobjectEvent& event)
{
  vRef<Property> p = meta_cast<Property>(event.getOldChild());
  if(p.isValid()) p->removePropertyListener(this);
}

MetaObject* csMetaTexture::new_csMetaTexture(VobjectBase* superobject,
                                             const std::string& type)
{
  return new csMetaTexture(superobject);
}
