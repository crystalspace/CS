/*
    Copyright (C) 2003 by Jorrit Tyberghein
              (C) 2003 by Frank Richter

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

#ifndef __CS_LOADTEX_H__
#define __CS_LOADTEX_H__

#include "csutil/scf.h"
#include "itexture/itexloaderctx.h"
#include "igraphic/image.h"

#define PLUGIN_TEXTURELOADER_IMAGE    "crystalspace.texture.loader.image"
#define PLUGIN_TEXTURELOADER_ANIMIMG  "crystalspace.texture.loader.animimg"
#define PLUGIN_TEXTURELOADER_CHECKERS "crystalspace.texture.loader.checkerboard"
#define PLUGIN_TEXTURELOADER_CUBEMAP  "crystalspace.texture.loader.cubemap"
#define PLUGIN_TEXTURELOADER_TEX3D    "crystalspace.texture.loader.tex3d"

/// Default texture loader context
class TextureLoaderContext : public iTextureLoaderContext
{
  bool has_flags;
  int flags;
  bool has_image;
  csRef<iImage> image;
  bool has_size;
  int width, height;
  const char* texname;
  char* texClass;
public:
  SCF_DECLARE_IBASE;

  TextureLoaderContext (const char* texname);
  virtual ~TextureLoaderContext ();

  void SetFlags (int Flags);
  virtual bool HasFlags ();
  virtual int GetFlags ();
  
  void SetImage (iImage* Image);
  virtual bool HasImage ();
  virtual iImage* GetImage();

  void SetSize (int w, int h);
  virtual bool HasSize ();
  virtual void GetSize (int& w, int& h);

  virtual void SetClass (const char* className);
  virtual const char* GetClass ();

  virtual const char* GetName ();
};

/// Base texture loader pseudo-plugin
class csBaseTextureLoader : public iLoaderPlugin, public iComponent  
{
protected:
  iObjectRegistry* object_reg;
public:
  SCF_DECLARE_IBASE;

  csBaseTextureLoader (iBase *p);
  virtual ~csBaseTextureLoader ();

  virtual bool Initialize(iObjectRegistry *object_reg);

  virtual csPtr<iBase> Parse (iDocumentNode* node, iLoaderContext* ldr_context,
  	iBase* context) = 0;
};  

/// Image texture loader pseudo-plugin
class csImageTextureLoader : public csBaseTextureLoader
{
public:
  csImageTextureLoader (iBase *p);

  virtual csPtr<iBase> Parse (iDocumentNode* node, iLoaderContext* ldr_context,
  	iBase* context);
};

/// Checkerboard texture loader pseudo-plugin
class csCheckerTextureLoader : public csBaseTextureLoader
{
public:
  csCheckerTextureLoader (iBase *p);

  virtual csPtr<iBase> Parse (iDocumentNode* node, iLoaderContext* ldr_context,
  	iBase* context);
};

/// Cubemap texture loader pseudo-plugin
class csCubemapTextureLoader : public csBaseTextureLoader
{
  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE "plugins/csparser/cubemaploader.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE 
public:
  csCubemapTextureLoader (iBase *p);

  virtual csPtr<iBase> Parse (iDocumentNode* node, iLoaderContext* ldr_context,
    iBase* context);
};

/// 3D texture loader pseudo-plugin
class csTexture3DLoader : public csBaseTextureLoader
{
  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE "plugins/csparser/tex3dloader.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE 
public:
  csTexture3DLoader (iBase *p);

  virtual csPtr<iBase> Parse (iDocumentNode* node, iLoaderContext* ldr_context,
    iBase* context);
};

#endif
