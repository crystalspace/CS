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

#ifndef __CORE_CSOBJECTMAPLISTENER_H__
#define __CORE_CSOBJECTMAPLISTENER_H__

#include "ieditor/editor.h"
#include "ieditor/objectlist.h"

#include <csutil/scf_implementation.h>
#include <iutil/comp.h>
#include <iengine/engine.h>

#include <wx/bitmap.h>

using namespace CS::EditorApp;

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

/// Registers CS objects with the editor when a map is loaded
class CSObjectMapListener : public scfImplementation2<CSObjectMapListener,iMapListener,iComponent>
{
public:
  CSObjectMapListener (iBase* parent);
  virtual ~CSObjectMapListener ();

  // iComponent
  virtual bool Initialize (iObjectRegistry* obj_reg);

  // iMapListener
  virtual void OnMapLoaded (const char* path, const char* filename);
  virtual void OnLibraryLoaded (const char* path, const char* filename, iCollection* collection);
  
private:
  iObjectRegistry* object_reg;
  
  wxBitmap* meshBmp;
  wxBitmap* lightBmp;
  wxBitmap* defaultBmp;

  csRef<iEditor> editor;
  csRef<iEngine> engine;
  
  csRef<iObjectList> objects;
};

}
CS_PLUGIN_NAMESPACE_END(CSE)

#endif
