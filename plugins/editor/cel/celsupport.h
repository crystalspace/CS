/*
    Copyright (C) 2007 by Seth Yastrov

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

#ifndef __EDITOR_CEL_CELSUPPORT_H__
#define __EDITOR_CEL_CELSUPPORT_H__

#include "ieditor/editor.h"
#include "ieditor/objectlist.h"

#include <csutil/scf_implementation.h>
#include <iutil/comp.h>
#include <iengine/engine.h>

struct iCelPlLayer;
struct iCelBlLayer;

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

/// Loads CEL plugins and initializes CEL
class CELSupport : public scfImplementation2<CELSupport,iMapListener,iComponent>
{
public:
  CELSupport (iBase* parent);
  virtual ~CELSupport ();

  // iComponent
  virtual bool Initialize (iObjectRegistry* obj_reg);

  // iMapListener
  virtual void OnMapLoaded (const char* path, const char* filename);
  virtual void OnLibraryLoaded (const char* path, const char* filename, iCollection* collection);
  
private:
  iObjectRegistry* object_reg;
  
  csRef<iCelPlLayer> pl;
  csRef<iCelBlLayer> bl;

  csRef<iEditor> editor;
  csRef<iEngine> engine;
};

}
CS_PLUGIN_NAMESPACE_END(CSE)

#endif
