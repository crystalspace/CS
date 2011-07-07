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

#include "cssysdef.h"
#include "csutil/scf.h"
#include "csutil/sysfunc.h"

#include "iengine/sector.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"

#include <wx/wx.h>
#include <wx/dnd.h>
#include <wx/treectrl.h>

#include "assetbrowserpanel.h"

#include "dataobject.h"
#include "stdactions.h"

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

#include "data/editor/images/sceneIcon.xpm"
  
SCF_IMPLEMENT_FACTORY (AssetBrowserPanel)

BEGIN_EVENT_TABLE(AssetBrowserPanel, wxPanel)
  EVT_SIZE(AssetBrowserPanel::OnSize)
END_EVENT_TABLE()

#if USE_GENERIC_TREECTRL
BEGIN_EVENT_TABLE(AssetBrowserTreeCtrl, wxGenericTreeCtrl)
#else
BEGIN_EVENT_TABLE(AssetBrowserTreeCtrl, wxTreeCtrl)
#endif
  EVT_TREE_BEGIN_LABEL_EDIT(AssetBrowser_Ctrl, AssetBrowserTreeCtrl::OnBeginLabelEdit)
  EVT_TREE_END_LABEL_EDIT(AssetBrowser_Ctrl, AssetBrowserTreeCtrl::OnEndLabelEdit)
  EVT_TREE_BEGIN_DRAG(AssetBrowser_Ctrl, AssetBrowserTreeCtrl::OnBeginDrag)
  EVT_TREE_END_DRAG(AssetBrowser_Ctrl, AssetBrowserTreeCtrl::OnEndDrag)
  EVT_TREE_KEY_DOWN(AssetBrowser_Ctrl, AssetBrowserTreeCtrl::OnKeyDown)
  EVT_TREE_SEL_CHANGED(AssetBrowser_Ctrl, AssetBrowserTreeCtrl::OnSelChanged)
  EVT_TREE_ITEM_RIGHT_CLICK(AssetBrowser_Ctrl, AssetBrowserTreeCtrl::OnRightClick)
END_EVENT_TABLE()


AssetBrowserPanel::AssetBrowserPanel (iBase* parent)
 : scfImplementationType (this, parent)
{
  treectrl = 0;
}

bool AssetBrowserPanel::Initialize (iObjectRegistry* obj_reg)
{
  object_reg = obj_reg;
  
  panelManager = csQueryRegistry<iPanelManager> (object_reg);
  if (!panelManager)
    return false;

  editor = csQueryRegistry<iEditor> (object_reg);
  if (!editor)
    return false;

  actionManager = csQueryRegistry<iActionManager> (object_reg);
  if (!actionManager)
    return false;

  // Create the panel
  Create (editor->GetWindow (), -1, wxPoint (0, 0), wxSize (250, 250));
  
  // Create the controls
  treectrl = new AssetBrowserTreeCtrl (object_reg, editor, this, AssetBrowser_Ctrl,
    wxPoint (0, 0), wxSize(100, 100), wxTR_MULTIPLE | wxTR_FULL_ROW_HIGHLIGHT | wxTR_EDIT_LABELS | wxTR_HAS_BUTTONS);
  
  // Add it to the panel manager
  panelManager->AddPanel (this);
  
  return true;
}

AssetBrowserPanel::~AssetBrowserPanel ()
{
}

wxWindow* AssetBrowserPanel::GetWindow ()
{
  return this;
}

const wxChar* AssetBrowserPanel::GetCaption () const
{
  return wxT("Asset Browser");
}

PanelDockPosition AssetBrowserPanel::GetDefaultDockPosition () const
{
  return DockPositionRight;
}

void AssetBrowserPanel::OnSize (wxSizeEvent& event)
{
  // Resize the tree control
  if (treectrl)
    treectrl->SetSize (event.GetSize());
  event.Skip();
}


// ----------------------------------------------------------------------------

AssetBrowserTreeCtrl::AssetBrowserTreeCtrl(iObjectRegistry* obj_reg, iEditor* editor,
                       wxWindow *parent, const wxWindowID id,
                       const wxPoint& pos, const wxSize& size,
                       long style)
  : scfImplementationType (this),
    wxTreeCtrl(parent, id, pos, size, style),
    editor (editor), editingLabel (false),
    ignoreSelectionChanges (false)
{
  engine = csQueryRegistry<iEngine> (obj_reg);
  actionManager = csQueryRegistry<iActionManager> (obj_reg);
  
  // Register for object list events
  objects = editor->GetObjects ();
  objects->AddListener (this);
  
  // Register for selection events
  selection = editor->GetSelection ();
  selection->AddListener (this);

  imageList = new wxImageList (16, 16);
  AssignImageList(imageList);
  
  wxBitmap sceneBmp (sceneIcon_xpm);
  int rootIconIdx = imageList->Add (sceneBmp);
  AddRoot (wxT("Assets"), rootIconIdx);
}

void AssetBrowserTreeCtrl::OnObjectAdded (iObjectList* list, iEditorObject* obj)
{
  if (!obj->HasInterface (scfInterfaceTraits<iMeshFactoryWrapper>::GetID()))
    return;
  
  if (list == objects)
  {
    int imageIdx = imageList->Add (*obj->GetIcon ());
    
    obj->AddListener (this);
    
    // Find the parent item
    wxTreeItemId parentId = itemMap.Get (obj->GetParent (), GetRootItem ());

    // Append to tree
    wxTreeItemId id = AppendItem (parentId,
				  wxString(obj->GetName (), *wxConvCurrent),
				  imageIdx, -1,
				  new AssetBrowserTreeItemData (obj));
    ExpandAll ();
    
    // Put id in map
    itemMap.Put (obj->GetIBase (), id);
  }
  else if (list == selection)
  {
    if (ignoreSelectionChanges)
      return;
    
    wxTreeItemId id = itemMap.Get (obj->GetIBase (), wxTreeItemId());
    if (id.IsOk())
    {
      SelectItem (id);
    }
  }
}

void AssetBrowserTreeCtrl::OnObjectRemoved (iObjectList* list, iEditorObject* obj)
{
  if (!obj->HasInterface (scfInterfaceTraits<iMeshFactoryWrapper>::GetID()))
    return;
  
  if (list == objects)
  {
    // TODO: Move this to private function
    wxTreeItemId item = itemMap.Get (obj->GetIBase (), wxTreeItemId ());
    if (!item)
      return;
    Delete (item);
    itemMap.DeleteAll (obj->GetIBase ());
  }
  else if (list == selection)
  {
    if (ignoreSelectionChanges)
      return;
    
    wxTreeItemId id = itemMap.Get (obj->GetIBase (), wxTreeItemId());
    if (id.IsOk())
    {
      SelectItem (id, false);
    }
  }
}

void AssetBrowserTreeCtrl::OnObjectsCleared (iObjectList* list)
{
  if (list == objects)
  {
    // Remove each object
    csRef<iEditorObjectIterator> it = list->GetIterator ();
    while (it->HasNext ())
    {
      OnObjectRemoved (list, it->Next ());
    }
  }
  else if (list == selection)
  {
    if (ignoreSelectionChanges)
      return;
    
    UnselectAll ();
  }
}

void AssetBrowserTreeCtrl::OnObjectChanged (iEditorObject* obj)
{
  wxTreeItemId item = itemMap.Get (obj->GetIBase (), wxTreeItemId ());
  if (!item.IsOk())
    return;
  
  SetItemText (item, wxString (obj->GetName (), *wxConvCurrent));

  // Try to update its parent
  wxTreeItemId parentItem;
  iBase* parent = obj->GetParent ();
  if (parent)
    parentItem = itemMap.Get (parent, wxTreeItemId ());
  else
    parentItem = GetRootItem ();
  
  Reparent (item, parentItem);
}

void AssetBrowserTreeCtrl::OnSelChanged(wxTreeEvent& event)
{
  ignoreSelectionChanges = true;
  
  wxArrayTreeItemIds selectionIds;
  unsigned int selSize = GetSelections(selectionIds);
  
  selection->Clear();
  
  for (unsigned int i = 0; i < selSize; i++)
  {
    wxTreeItemId itemId = selectionIds[i];
    if (itemId == GetRootItem())
    {
      continue;
    }
    
    AssetBrowserTreeItemData* data = static_cast<AssetBrowserTreeItemData*> (GetItemData(itemId));
    selection->Add(data->GetEditorObject());
  }
  
  ignoreSelectionChanges = false;
}

void AssetBrowserTreeCtrl::OnBeginDrag(wxTreeEvent& event)
{
  draggedItem = event.GetItem();
  if (event.GetItem() != GetRootItem())
    event.Allow();
}

void AssetBrowserTreeCtrl::OnEndDrag(wxTreeEvent& event)
{
  wxTreeItemId itemSrc = draggedItem;
  wxTreeItemId itemDst = event.GetItem();
  draggedItem = (wxTreeItemId)0l;

  AssetBrowserTreeItemData* data = static_cast<AssetBrowserTreeItemData*> (GetItemData(itemSrc));

  if (CanReparent (itemSrc, itemDst))
  {
    AssetBrowserTreeItemData* dstData = static_cast<AssetBrowserTreeItemData*> (GetItemData(itemDst));

    // Get the new parent
    csRef<iBase> newParent;
    if (itemDst == GetRootItem())
      newParent = 0;
    else
      newParent = dstData->GetEditorObject ()->GetIBase ();

    // Perform the action
    csRef<SetObjectParentAction> action (new SetObjectParentAction (
        data->GetEditorObject (), newParent));
    actionManager->Do (action);
  }
}

bool AssetBrowserTreeCtrl::CanReparent (const wxTreeItemId& itemSrc, const wxTreeItemId& itemDst)
{
  // Make sure destination item exists
  if (!itemDst.IsOk())
    return false;

  // Don't drop on the same item
  if (itemSrc == itemDst)
    return false;

  // Don't drop into same parent
  if (itemDst == GetItemParent(itemSrc))
    return false;

  // Find the parent item
  wxTreeItemId parentId = GetItemParent(itemDst);

  // Make sure we don't drop into a child item
  while (parentId.IsOk())
  {
    if (parentId == itemSrc)
      return false;
    
    parentId = GetItemParent(parentId);
  }

  return true;
}

void AssetBrowserTreeCtrl::Reparent (const wxTreeItemId& itemSrc, const wxTreeItemId& itemDst)
{
  AssetBrowserTreeItemData* data = static_cast<AssetBrowserTreeItemData*> (GetItemData(itemSrc));

  wxTreeItemId itemNew = InsertItem(itemDst, 0, GetItemText(itemSrc),
      GetItemImage(itemSrc), GetItemImage(itemSrc, wxTreeItemIcon_Normal),
      new AssetBrowserTreeItemData(data));

  CopyChildren(itemSrc, itemNew);

  itemMap.PutUnique (data->GetEditorObject ()->GetIBase (), itemNew);

  if (IsExpanded(itemSrc))
    Expand(itemNew);
  
  Delete(itemSrc);

  EnsureVisible(itemNew);
}

void AssetBrowserTreeCtrl::CopyChildren (const wxTreeItemId& parent, const wxTreeItemId& newParent)
{
  if (!ItemHasChildren(parent))
    return;
  
  wxTreeItemIdValue cookie = (void*)1;

  wxTreeItemId child = GetFirstChild(parent, cookie);
  
  while (child.IsOk())
  {
    AssetBrowserTreeItemData* data = (AssetBrowserTreeItemData*) GetItemData(child);

    wxTreeItemId itemNew = InsertItem(newParent, 0, GetItemText(child),
      GetItemImage(child), GetItemImage(child, wxTreeItemIcon_Normal),
      new AssetBrowserTreeItemData(data));

    itemMap.PutUnique (data->GetEditorObject ()->GetIBase (), itemNew);
    
    CopyChildren (child, itemNew);

    if (IsExpanded(child))
      Expand(itemNew);

    child = GetNextChild(parent, cookie);
  }
}

void AssetBrowserTreeCtrl::OnBeginLabelEdit(wxTreeEvent& event)
{
  if (editingLabel || event.GetItem() == GetRootItem())
  {
    event.Veto();
  }
  else
  {
    editingLabel = true;
  }
}

void AssetBrowserTreeCtrl::OnEndLabelEdit(wxTreeEvent& event)
{
  editingLabel = false;
  
  // Veto it - the action listener will update the label for us.
  event.Veto ();
  
  // Don't do anything if they cancelled
  if (!event.GetLabel())
    return;
  
  wxTreeItemId id = event.GetItem();
  AssetBrowserTreeItemData* data = static_cast<AssetBrowserTreeItemData*> (GetItemData(id));

  csRef<SetObjectNameAction> action (new SetObjectNameAction (
      data->GetEditorObject (), (const char*) event.GetLabel().mb_str(wxConvUTF8)));
  actionManager->Do (action);
}

void AssetBrowserTreeCtrl::OnKeyDown(wxTreeEvent& event)
{
  if (event.GetKeyCode() == WXK_F2)
  {
    EditLabel(GetSelection());
  }
  else
  {
    event.Skip ();
  }
}

void AssetBrowserTreeCtrl::OnRightClick(wxTreeEvent& event)
{
  // Instantiate the factory
  wxTreeItemId id = event.GetItem();
  AssetBrowserTreeItemData* data = static_cast<AssetBrowserTreeItemData*> (GetItemData(id));
  
  csRef<iEditorObject> obj = data->GetEditorObject();

  EditorDataObject dataObject;
  dataObject.SetEditorObject (obj);
  
  wxDropSource* dropSource = new wxDropSource(dataObject, GetParent ());
  dropSource->DoDragDrop ();
}

}
CS_PLUGIN_NAMESPACE_END(CSE)
