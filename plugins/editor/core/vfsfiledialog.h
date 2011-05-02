/*
    Copyright (C) 2003 by Philipp Aumayr

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
#ifndef __VFSFILEDIALOG_H__
#define __VFSFILEDIALOG_H__

struct iVFS;
class wxTreeCtrl;
class wxTextCtrl;
class wxComboBox;
class wxTreeItemId;
class wxTreeEvent;
class wxListCtrl;
class wxListEvent;
class wxTreeItemData;
class wxImageList;
class wxBitmap;
class wxButton;
class wxBitmapButton;
class wxStaticText;
class wxChoice;

#include <wx/treectrl.h>
#include <wx/dialog.h>
#include "csutil/csstring.h"
#include "csutil/scf.h"

enum fileDialogType
{
  VFS_OPEN,
  VFS_SAVE
};

class cssVFSFileDlg : public wxDialog
{
public:
  cssVFSFileDlg
    ( wxWindow *parent,
      wxWindowID id,
      const wxString &title,
      const wxPoint& pos = wxDefaultPosition,
      const wxSize& size = wxDefaultSize,
      long style = wxDEFAULT_DIALOG_STYLE,
      iVFS* vfs = 0,
      const csString& startpath = "/",
      fileDialogType dialogType = VFS_OPEN
      );
  ~cssVFSFileDlg();

  const char* GetPath() { return curdvpath.GetData(); }
  const char* GetFilename() { return filename.GetData(); }

protected:
  //EVENT Methods
  void OnDirTreeExpandBranch( wxTreeEvent& event );
  void OnDirTreeCollapseBranch( wxTreeEvent& event );
  void OnDirTreeSelChange( wxTreeEvent& event );
  void OnRightClickItem( wxTreeEvent& event);

  void OnDirViewSelChange( wxListEvent& event );
  void OnDirViewActivated( wxListEvent & event );

  void OnPathTextEnter(wxCommandEvent& event);

  virtual void OnOk( wxCommandEvent& event );
  virtual void OnCancel( wxCommandEvent& event );

  void OnViewModeSelect( wxCommandEvent& event );

  void OnOpenParent( wxCommandEvent& event );

private:
  //Enumerators
  enum ctrllist
  {
    DIRECTORY_TREE = wxID_HIGHEST + 1,
    DIRECTORY_LIST,
		
    TEXT_PATH,

    CHOICE_FILTER,

    BUTTON_OPEN,
    BUTTON_CANCEL,
    BUTTON_NEW_FOLDER,
    BUTTON_PARENT,

    CHOICE_VIEW_MODE
  };

  enum imageId
  {
    IMAGE_FOLDER_OPEN,
    IMAGE_FOLDER_CLOSED,
    IMAGE_FILE,
    IMAGE_NEW_DIR,
    IMAGE_TO_PARENT,
    IMAGE_COUNT		//Should always be at the bottom.
  };

  enum viewTypes
  {
    LIST,
    REPORT,
    ICON,
    DETAIL
  };

  enum fileTypes
  {
    FOLDER,
    FILE
  };

  //Operations
  wxImageList* CreateImageList();
  csString GetPathByTreeItem(wxTreeItemId id);
  bool HasChildDir( wxTreeItemId parent );
  void InitFilterList(wxString choices []);
  void InitViewTypeList();
  bool IsDir( const char *path );
  bool IsFile( const char *path );
  bool LoadVFSDirView( const char *path);
  bool LoadVFSTreeBranch(wxTreeItemId parent, const char *path);

  void OpenTreeItemChild( wxString childName );

  //Images
  wxImageList* imageList;
  int imageIdList[IMAGE_COUNT];

  //Frame Elements
  wxTreeCtrl *dirtree;
  wxTextCtrl *thepath;
  wxChoice *filter;
  wxButton *okbutton;
  wxButton *cancelbut;
  wxStaticText *pathtext;
  wxListCtrl *fileView;
  wxChoice *viewTypes;
  wxBitmapButton *newFolderButton;
  wxBitmapButton *parentFolderButton;

  csString curdvpath;
  csString filename;
  fileDialogType dialogType;

  csRef<iVFS> vfs;

  DECLARE_EVENT_TABLE();
};

class FileDlgTreeItemData : public wxTreeItemData
{
public:
  explicit FileDlgTreeItemData(const wxString& newData ) : data(newData) {}

  const wxChar *GetData() const { return data.c_str(); }
  void SetData(wxString newData) { data = newData; }

private:
  wxString data;
};

#endif
