/*
Copyright (C) 2010 by Alin Baciu

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __VPLLOADER_H__
#define __VPLLOADER_H__

#define QUALIFIED_PLUGIN_NAME "crystalspace.vpl.loader"

#include <iutil/comp.h>
#include <ivideodecode/medialoader.h>
#include <ivideodecode/mediacontainer.h>
#include <csutil/scf_implementation.h>

struct iObjectRegistry;

/**
  * This is the implementation for our API and
  * also the implementation of the plugin.
  */
class vplLoader : public scfImplementation2<vplLoader,iMediaLoader,iComponent>
{
private:
  iObjectRegistry* object_reg;

  /// Theora video loader interface
  csRef<iMediaLoader> m_pThOggLoader;

public:
  vplLoader (iBase* parent);
  virtual ~vplLoader ();

  // From iComponent.
  virtual bool Initialize (iObjectRegistry*);

  virtual csRef<iMediaContainer> LoadMedia (const char * pFileName, const char *pDescription=0/*, const char* pMediaType = "AutoDetect"*/);

  virtual void Create (csString path,csArray<Language> languages) {}
};

#endif // __VPLLOADER_H__
