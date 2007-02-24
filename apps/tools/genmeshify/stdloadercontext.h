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

#ifndef __CS_STDLOADERCONTEXT_H__
#define __CS_STDLOADERCONTEXT_H__

namespace genmeshify
{

  class App;

  /**
   * Context class for the standard loader.
   */
  class StdLoaderContext : public scfImplementation1<StdLoaderContext,
			   iLoaderContext>
  {
  private:
    App* app;
    iEngine* Engine;
    iRegion* region;
    bool checkDupes;

  public:
    StdLoaderContext (App* app, iEngine* Engine, iRegion* region, 
      bool checkDupes = false);
    virtual ~StdLoaderContext ();

    virtual iSector* FindSector (const char* name);
    virtual iMaterialWrapper* FindMaterial (const char* name);
    virtual iMaterialWrapper* FindNamedMaterial (const char* name,
        const char *filename);
    virtual iMeshFactoryWrapper* FindMeshFactory (const char* name);
    virtual iMeshWrapper* FindMeshObject (const char* name);
    virtual iTextureWrapper* FindTexture (const char* name);
    virtual iTextureWrapper* FindNamedTexture (const char* name,
        const char *filename);
    virtual iLight* FindLight (const char *name);
    virtual iShader* FindShader (const char *name);
    virtual bool CheckDupes () const { return checkDupes; }
    virtual iRegion* GetRegion () const { return region; }
    virtual bool CurrentRegionOnly () const { return false; }
  };

} // namespace genmeshify

#endif // __CS_STDLOADERCONTEXT_H__