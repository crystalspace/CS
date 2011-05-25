/*
Copyright (C) 2011 by Alin Baciu

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
#ifndef __THOGGLOADER_H__
#define __THOGGLOADER_H__

#include <iutil/comp.h>
#include <videodecode/vpl_loader.h>
#include <videodecode/vpl_structs.h>
#include <csutil/scf_implementation.h>
#include "thoggCodec.h"

struct iObjectRegistry;

/**
* This is the implementation for our API and
* also the implementation of the plugin.
*/
class thoggLoader : public scfImplementation2<thoggLoader,iVPLLoader,iComponent>
{
private:
  iObjectRegistry* object_reg;

public:
  thoggLoader (iBase* parent);
  virtual ~thoggLoader ();

  // From iComponent.
  virtual bool Initialize (iObjectRegistry*);

  
  virtual csPtr<iVPLCodec> LoadVideo (const char * pFileName, const char *pDescription=0, VideoType type=AutoDetect);
};

#endif // __THOGGLOADER_H__