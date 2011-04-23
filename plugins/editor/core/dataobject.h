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

#ifndef __CORE_DATAOBJECT_H__
#define __CORE_DATAOBJECT_H__

#include "ieditor/editorobject.h"

#include <csutil/weakref.h>

#include <wx/dataobj.h>

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

struct DnDObjectDump {
  csRef<CS::EditorApp::iEditorObject> obj;
};
    
  
class EditorDataObject : public wxDataObjectSimple
{
public:
  EditorDataObject () : wxDataObjectSimple (wxDataFormat (wxT("EditorObject")))
  {
  }
  
  // get the size of our data
  virtual size_t GetDataSize() const
  {
    return sizeof(DnDObjectDump);
  }

  // copy our data to the buffer
  virtual bool GetDataHere(void *buf) const
  {
    DnDObjectDump dnd = {obj};
    memcpy (buf, &dnd, GetDataSize ());
    return true;
  }

  // copy data from buffer to our data
  virtual bool SetData(size_t len, const void *buf)
  {
    DnDObjectDump* dnd = (DnDObjectDump*)buf;
    obj = dnd->obj;
    return true;
  }
  
  
  virtual CS::EditorApp::iEditorObject* GetEditorObject ()
  {
    return obj;
  }
  
  virtual void SetEditorObject (CS::EditorApp::iEditorObject* o)
  {
    obj = o;
  }

private:
  csRef<CS::EditorApp::iEditorObject> obj;
};

}
CS_PLUGIN_NAMESPACE_END(CSE)

#endif
