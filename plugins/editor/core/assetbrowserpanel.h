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

#ifndef __CORE_ASSETBROWSERPANEL_H__
#define __CORE_ASSETBROWSERPANEL_H__

#include "ieditor/objectlist.h"
#include "ieditor/panelmanager.h"
#include "ieditor/actionmanager.h"
#include "ieditor/editor.h"

#include <csutil/scf_implementation.h>
#include <iutil/comp.h>
#include <csutil/hash.h>
#include <iengine/engine.h>
#include <iengine/mesh.h>

#include <wx/event.h>
#include <wx/panel.h>
#include <wx/treectrl.h>
#include <wx/imaglist.h>

template<>
class csHashComputer<iBase*> : public csHashComputerIntegral<iBase*> {};

using namespace CS::EditorApp;

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

class AssetBrowserTreeCtrl;

class AssetBrowserPanel : public scfImplementation2<AssetBrowserPanel,iPanel,iComponent>,
  public wxPanel
{
public:
  AssetBrowserPanel (iBase* parent);
  virtual ~AssetBrowserPanel ();

  // iComponent
  virtual bool Initialize (iObjectRegistry* obj_reg);

  // iPanel
  virtual wxWindow* GetWindow ();
  virtual const wxChar* GetCaption () const;
  virtual PanelDockPosition GetDefaultDockPosition () const;

private:
  iObjectRegistry* object_reg;

  AssetBrowserTreeCtrl* treectrl;

  csRef<iEditor> editor;
  csRef<iPanelManager> panelManager;
  csRef<iActionManager> actionManager;

  csRef<iObjectList> objects;

  void OnSize (wxSizeEvent& event);
  
  DECLARE_EVENT_TABLE()
};

class AssetBrowserTreeItemData : public wxTreeItemData
{
public:
  AssetBrowserTreeItemData (iEditorObject* obj) : object (obj) {}
  AssetBrowserTreeItemData (AssetBrowserTreeItemData* data) : object (data->object) {}

  csRef<iEditorObject> GetEditorObject () { return object; }
  
private:
  csRef<iEditorObject> object;
};

class AssetBrowserTreeCtrl : public scfImplementation2<AssetBrowserTreeCtrl,iObjectListListener,
    iEditorObjectChangeListener>,
    public wxTreeCtrl
{
public:
  AssetBrowserTreeCtrl() : scfImplementationType (this) { }
  AssetBrowserTreeCtrl(iObjectRegistry* obj_reg, iEditor* editor, wxWindow *parent,
             const wxWindowID id, const wxPoint& pos, const wxSize& size,
             long style);
  virtual ~AssetBrowserTreeCtrl() { };

  // iObjectListListener
  virtual void OnObjectAdded (iObjectList* list, iEditorObject* obj);
  virtual void OnObjectRemoved (iObjectList* list, iEditorObject* obj);
  virtual void OnObjectsCleared (iObjectList* list);
  
  // iEditorObjectChangeListener
  virtual void OnObjectChanged (iEditorObject* obj);

  void OnSelChanged(wxTreeEvent& event);
  void OnBeginDrag(wxTreeEvent& event);
  void OnEndDrag(wxTreeEvent& event);
  void OnBeginLabelEdit(wxTreeEvent& event);
  void OnEndLabelEdit(wxTreeEvent& event);
  void OnKeyDown(wxTreeEvent& event);
  void OnRightClick(wxTreeEvent& event);

  /// Reparent an item, including moving its children.
  void Reparent (const wxTreeItemId& itemSrc, const wxTreeItemId& itemDst);

  /// Check whether the source item can be parented to the destination item.
  bool CanReparent (const wxTreeItemId& itemSrc, const wxTreeItemId& itemDst);

private:
  wxTreeItemId draggedItem;

  wxImageList* imageList;

  /// Map of iBase* to tree item ids
  csHash<wxTreeItemId, iBase*> itemMap;

  csRef<iEditor> editor;
  csRef<iActionManager> actionManager;
  csRef<iEngine> engine;
  csRef<iObjectList> objects, selection;
  
  bool editingLabel;
  bool ignoreSelectionChanges;

  /// Copies the parent item's children to the new parent, recursively
  void CopyChildren (const wxTreeItemId& parent, const wxTreeItemId& newParent);

  friend class AssetBrowserPanel;
  
  DECLARE_EVENT_TABLE()
};

// Menu and control ids
enum
{
  AssetBrowser_Ctrl = wxID_HIGHEST
};

}
CS_PLUGIN_NAMESPACE_END(CSE)

#endif
