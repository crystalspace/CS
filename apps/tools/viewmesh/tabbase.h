/*
    Copyright (C) 2009 by Jelle Hellemans

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

#ifndef ITAB_H__
#define ITAB_H__

#include "cssysdef.h"
#include "csutil/ref.h"
#include "csutil/refcount.h"
#include "csutil/scfstringarray.h"

#include "ivaria/icegui.h"

#include "assetbase.h"

struct TabBase : public csRefCount
{
protected:
  iObjectRegistry* object_reg;
  csRef<AssetBase> asset;
  CEGUI::Window* window;

  csRef<iCEGUI> cegui;
  CEGUI::WindowManager* winMgr;

  bool LoadLayout(const char* layoutFile);
  bool AddToTabs();
  void UpdateList (csRef<iStringArray> arr, const char* window);
  bool GetSelectedItemText(const char* window, csString& value);

public:
  TabBase(iObjectRegistry* obj_reg, AssetBase* ass);
  virtual ~TabBase();

  CEGUI::Window* GetWindow() { return window; }

  AssetBase* GetAsset() { return asset; }
  void SetAsset(AssetBase* ass) { asset = ass; }

  //virtual iMeshWrapper* GetWindow() = 0;
};

//-------------------------------------------------------

TabBase::TabBase(iObjectRegistry* obj_reg, AssetBase* ass) 
  : object_reg(obj_reg), asset(ass), window(0) 
{
  cegui = csQueryRegistry<iCEGUI> (object_reg);
  winMgr = cegui->GetWindowManagerPtr ();
}

TabBase::~TabBase() 
{
  if (window)
  {
    CEGUI::TabControl* ts = (CEGUI::TabControl*)winMgr->getWindow("Root/Control/Tabs");
    ts->removeTab(window->getName());
    winMgr->destroyWindow(window);
  }
}

bool TabBase::LoadLayout(const char* layoutFile)
{
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (object_reg);

  vfs->PushDir();
  vfs->ChDir ("/ceguitest/0.5/");
  
  window = winMgr->loadWindowLayout(layoutFile);

  vfs->PopDir();

  return true;
}

bool TabBase::AddToTabs()
{
  CEGUI::TabControl* ts = (CEGUI::TabControl*)winMgr->getWindow("Root/Control/Tabs");
  ts->addTab(window);

  return true;
}

void TabBase::UpdateList (csRef<iStringArray> arr, const char* window)
{
  CEGUI::Listbox* list = (CEGUI::Listbox*)winMgr->getWindow(window);
  list->resetList();

  if (!arr) return;

  for (size_t i = 0; i < arr->GetSize(); i++)
  {
    if (!arr->Get(i)) continue;
    CEGUI::ListboxTextItem* item = new CEGUI::ListboxTextItem(arr->Get(i));
    item->setTextColours(CEGUI::colour(0,0,0));
    item->setSelectionBrushImage("ice", "TextSelectionBrush");
    item->setSelectionColours(CEGUI::colour(0.5f,0.5f,1));
    list->addItem(item);
    if (i == 0) item->setSelected(true);
  }
}

bool TabBase::GetSelectedItemText(const char* window, csString& value)
{
  csRef<iCEGUI> cegui = csQueryRegistry<iCEGUI> (object_reg);
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Listbox* list = (CEGUI::Listbox*)winMgr->getWindow(window);

  CEGUI::ListboxItem* item = list->getFirstSelectedItem();
  if(!item) return false;

  const CEGUI::String& text = item->getText();
  if (text.empty()) return false;

  value = text.c_str();

  return true;
}


#endif // ITAB_H__
