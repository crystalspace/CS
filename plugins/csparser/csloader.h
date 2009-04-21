/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
    Written by Ivan Avramovic <ivan@avramovic.com>

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

#ifndef __CS_CSLOADER_H__
#define __CS_CSLOADER_H__

#include "csutil/weakref.h"
#include "imap/loader.h"
#include "iutil/comp.h"

struct iEngine;

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{
#include "csutil/deprecated_warn_off.h"

  /**
  * The loader for Crystal Space maps.
  */
  class csLoader : public scfImplementation2<csLoader, iLoader, iComponent>
  {
  public:
    // constructor
    csLoader(iBase *p);

    // destructor
    virtual ~csLoader();

    // initialize the plug-in
    virtual bool Initialize(iObjectRegistry *object_reg);

    virtual csPtr<iImage> LoadImage (iDataBuffer* buf, int Format);

    virtual csPtr<iImage> LoadImage (const char *fname, int Format);

    virtual csPtr<iTextureHandle> LoadTexture (iDataBuffer* buf,
      int Flags = CS_TEXTURE_3D, iTextureManager *tm = 0, csRef<iImage>* img=0);

    virtual iTextureWrapper* LoadTexture (const char *name, iDataBuffer* buf,
      int Flags = CS_TEXTURE_3D, iTextureManager *tm = 0, bool reg = true,
      bool create_material = true, bool free_image = true);

    virtual csPtr<iTextureHandle> LoadTexture (const char* fname,
      int Flags = CS_TEXTURE_3D, iTextureManager *tm = 0, csRef<iImage>* img=0);

    virtual csPtr<iSndSysData> LoadSoundSysData (const char *fname);

    virtual csPtr<iSndSysStream> LoadSoundStream (const char *fname, int mode3d);

    virtual iSndSysWrapper* LoadSoundWrapper (const char *name, const char *fname);

    virtual csPtr<iMeshFactoryWrapper> LoadMeshObjectFactory (const char* fname, iStreamSource* ssource);

    virtual csPtr<iMeshWrapper> LoadMeshObject (const char* fname, iStreamSource* ssource);

    virtual csRef<iShader> LoadShader (const char* filename, bool registerShader = true);

    virtual iTextureWrapper* LoadTexture (const char *Name,
      const char *FileName, int Flags = CS_TEXTURE_3D, iTextureManager *tm = 0,
      bool reg = true, bool create_material = true, bool free_image = true,
      iCollection* collection = 0, uint keepFlags = KEEP_ALL);

    virtual bool LoadMapFile (const char* filename, bool clearEngine = true,
      iCollection* collection = 0, bool searchCollectionOnly = true, bool checkDupes = false,
      iStreamSource* ssource = 0, iMissingLoaderData* missingdata = 0, uint keepFlags = KEEP_ALL);

    virtual bool LoadMap (iDocumentNode* world_node, bool clearEngine = true,
      iCollection* collection = 0, bool searchCollectionOnly = true, bool checkDupes = false,
      iStreamSource* ssource = 0, iMissingLoaderData* missingdata = 0, uint keepFlags = KEEP_ALL);

    virtual bool LoadLibraryFile (const char* filename, iCollection* collection = 0,
      bool searchCollectionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
      iMissingLoaderData* missingdata = 0, uint keepFlags = KEEP_ALL);

    virtual bool LoadLibrary (iDocumentNode* lib_node, iCollection* collection = 0,
      bool searchCollectionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
      iMissingLoaderData* missingdata = 0, uint keepFlags = KEEP_ALL);

    virtual csLoadResult Load (const char* fname, iCollection* collection = 0,
      bool searchCollectionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
      const char* override_name = 0, iMissingLoaderData* missingdata = 0,
      uint keepFlags = KEEP_ALL);

    virtual csLoadResult Load (iDataBuffer* buffer, iCollection* collection = 0,
      bool searchCollectionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
      const char* override_name = 0, iMissingLoaderData* missingdata = 0,
      uint keepFlags = KEEP_ALL);

    virtual csLoadResult Load (iDocumentNode* node, iCollection* collection = 0,
      bool searchCollectionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
      const char* override_name = 0, iMissingLoaderData* missingdata = 0, uint keepFlags = KEEP_ALL);

  private:
    csRef<iThreadedLoader> loader;
    csRef<iVFS> vfs;
  };

#include "csutil/deprecated_warn_on.h"
}
CS_PLUGIN_NAMESPACE_END(csparser)

#endif // __CS_CSLOADER_H__
