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

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "cssysdef.h"
#include "csutil/ref.h"
#include "iutil/stringarray.h"
#include "iutil/vfs.h"

#include <wx/treectrl.h>
#include <wx/filename.h>
#include <wx/listctrl.h>
#include <wx/image.h>
#include <wx/imaglist.h>
#include <wx/combobox.h>
#include <wx/artprov.h>

#include "vfsfiledialog.h"

//Image Stuff
#define DEFAULT_IMAGE_SIZE 16

#include "data/editor/images/file.xpm"
#include "data/editor/images/folder_closed.xpm"
#include "data/editor/images/folder_open.xpm"
//#include "images/new_dir.xpm"
//#include "images/toparent.xpm"

BEGIN_EVENT_TABLE (cssVFSFileDlg, wxDialog)
	EVT_TREE_SEL_CHANGED(				DIRECTORY_TREE, cssVFSFileDlg::OnDirTreeSelChange)
	EVT_TREE_ITEM_EXPANDING(		DIRECTORY_TREE, cssVFSFileDlg::OnDirTreeExpandBranch)
	EVT_TREE_ITEM_COLLAPSING(		DIRECTORY_TREE, cssVFSFileDlg::OnDirTreeCollapseBranch)
  EVT_TREE_ITEM_RIGHT_CLICK(  DIRECTORY_TREE, cssVFSFileDlg::OnRightClickItem)

	EVT_LIST_ITEM_SELECTED(			DIRECTORY_LIST, cssVFSFileDlg::OnDirViewSelChange )
	EVT_LIST_ITEM_ACTIVATED(		DIRECTORY_LIST, cssVFSFileDlg::OnDirViewActivated )


	EVT_TEXT_ENTER( TEXT_PATH, cssVFSFileDlg::OnPathTextEnter )

	EVT_BUTTON( BUTTON_CANCEL, cssVFSFileDlg::OnCancel )
	EVT_BUTTON( BUTTON_OPEN, cssVFSFileDlg::OnOk )
	EVT_BUTTON( BUTTON_PARENT, cssVFSFileDlg :: OnOpenParent)

	EVT_CHOICE(CHOICE_VIEW_MODE, cssVFSFileDlg::OnViewModeSelect ) 
END_EVENT_TABLE()


cssVFSFileDlg::cssVFSFileDlg ( wxWindow *parent,
                           wxWindowID id,
                           const wxString &title,
                           const wxPoint &position,
                           const wxSize& size,
                           long style,
                           iVFS* vfs,
                           const wxString& startpath,
			   fileDialogType type) : 
  wxDialog( parent, id, title, position, size, style), vfs (vfs)
{
	dialogType = type;
	imageList = CreateImageList();

//create Sizers
	wxBoxSizer *boxSizerMain = new wxBoxSizer( wxHORIZONTAL );
	wxBoxSizer *boxSizerRight = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer *boxSizerTopRight = new wxBoxSizer( wxHORIZONTAL );
	wxFlexGridSizer *flexSizerBottomRight = new wxFlexGridSizer( 3, 0, 0 );
	flexSizerBottomRight->AddGrowableCol( 1 );
	flexSizerBottomRight->AddGrowableRow( 0 );
	flexSizerBottomRight->AddGrowableRow( 1 );

//Create widgets
	switch ( dialogType )
	{
	case VFS_OPEN:
		okbutton = new wxButton(this, BUTTON_OPEN, _("&Open"), wxDefaultPosition, wxDefaultSize, 0 );
		break;
	case VFS_SAVE:
		okbutton =  new wxButton( this, BUTTON_OPEN, _("&Save"), wxDefaultPosition, wxDefaultSize, 0 );
		break;
	}
	okbutton->SetDefault();

	dirtree = new wxTreeCtrl( this, DIRECTORY_TREE, wxDefaultPosition, wxSize(160,160), wxTR_HAS_BUTTONS|wxTR_LINES_AT_ROOT|wxSUNKEN_BORDER );
	fileView = new wxListCtrl( this, DIRECTORY_LIST, wxDefaultPosition, wxSize(340,210), wxLC_LIST|wxSUNKEN_BORDER );
		fileView->SetImageList( imageList, wxIMAGE_LIST_SMALL );
	filter = new wxChoice( this, CHOICE_FILTER, wxDefaultPosition, wxSize(230,-1));
	thepath = new wxTextCtrl( this, TEXT_PATH, wxT(""), wxDefaultPosition, wxSize(230,-1), 0 );
	pathtext = new wxStaticText( this, -1,  _("VFS Path:"), wxDefaultPosition, wxDefaultSize, 0 );
	cancelbut	= new wxButton( this, BUTTON_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	viewTypes = new wxChoice( this, CHOICE_VIEW_MODE, wxDefaultPosition, wxSize(100,-1));


	newFolderButton = new wxBitmapButton( this,
	  BUTTON_NEW_FOLDER,
	  wxArtProvider::GetBitmap (wxART_NEW_DIR, wxART_CMN_DIALOG),
	  wxDefaultPosition,
	  wxSize(25,25) );
	newFolderButton->SetToolTip( _("Create Directory") );

	parentFolderButton = new wxBitmapButton( this,
	  BUTTON_PARENT,
	  wxArtProvider::GetBitmap (wxART_GO_DIR_UP, wxART_CMN_DIALOG),
	  wxDefaultPosition,
	  wxSize(25,25) );
	parentFolderButton->SetToolTip( _("Parent Folder") );
  
	wxStaticText *fileFilterText = new wxStaticText( this, -1, _("File filter:"), wxDefaultPosition, wxDefaultSize, 0 );

//Set Objects into sizers
	boxSizerMain->Add( dirtree, 0, wxGROW|wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxTOP|wxBOTTOM, 5 );
	boxSizerTopRight->Add( parentFolderButton, 0, wxALIGN_CENTER|wxRIGHT, 5 );
	boxSizerTopRight->Add( newFolderButton, 0, wxALIGN_CENTER|wxRIGHT, 5 );
	boxSizerTopRight->Add( viewTypes, 1, wxALIGN_CENTER|wxLEFT|wxGROW , 5 );
	boxSizerRight->Add( boxSizerTopRight, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 0 );
	boxSizerRight->Add( fileView, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5 );
	flexSizerBottomRight->Add( pathtext, 1, wxALIGN_CENTER|wxALL, 5 );
	flexSizerBottomRight->Add( thepath, 1, wxGROW, 5 );
	flexSizerBottomRight->Add( okbutton, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT, 15 );
	flexSizerBottomRight->Add( fileFilterText, 0, wxALIGN_CENTER|wxALL|wxGROW, 5 );
	flexSizerBottomRight->Add( filter, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL|wxGROW, 0 );
	flexSizerBottomRight->Add( cancelbut, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT, 15 );
	boxSizerRight->Add( flexSizerBottomRight, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP, 5 );
	boxSizerMain->Add( boxSizerRight, 1, wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );

	this->SetAutoLayout( TRUE );
	this->SetSizer( boxSizerMain );
	boxSizerMain->Fit( this );
	boxSizerMain->SetSizeHints( this );

  dirtree->DeleteAllItems();
	wxTreeItemId rootId = dirtree->AddRoot( wxT("VFS") );
	dirtree->SetItemHasChildren( rootId );
	dirtree->SetImageList( imageList );
	dirtree->SetItemImage( rootId, imageIdList[IMAGE_FOLDER_OPEN] );

	FileDlgTreeItemData* data = new FileDlgTreeItemData(wxT("/"));
	dirtree->SetItemData( rootId, data );
	dirtree->SelectItem(rootId);
        dirtree->Expand (rootId);
        
        curdvpath = GetPathByTreeItem(rootId);
        LoadVFSDirView(curdvpath);
        thepath->SetValue(wxString(curdvpath.GetData(), wxConvUTF8));

	InitViewTypeList();
        
        //OpenTreeItemChild (startpath);
}

cssVFSFileDlg::~cssVFSFileDlg()
{
}

//Operations
wxImageList* cssVFSFileDlg :: CreateImageList()
{
	wxImageList* list = new wxImageList( DEFAULT_IMAGE_SIZE,
	  DEFAULT_IMAGE_SIZE,
	  true,
	  IMAGE_COUNT);
	wxImage image(DEFAULT_IMAGE_SIZE, DEFAULT_IMAGE_SIZE);

	imageIdList[IMAGE_FOLDER_OPEN] = list->Add(wxBitmap (folder_open_xpm));
	imageIdList[IMAGE_FOLDER_CLOSED] = list->Add(wxBitmap (folder_closed_xpm));
	imageIdList[IMAGE_FILE] = list->Add(wxBitmap (file_xpm)
	  /*wxArtProvider::GetBitmap (wxART_NORMAL_FILE, wxART_CMN_DIALOG)*/);

	return list;
}


csString cssVFSFileDlg :: GetPathByTreeItem(wxTreeItemId id)
{
  csString path = "";
  wxTreeItemId cur = id;
	

  while(cur != dirtree->GetRootItem())
  {
		FileDlgTreeItemData* data = (FileDlgTreeItemData*)dirtree->GetItemData( cur );

    if(data)
    {
		  path.Insert(0, data->GetData() );
    }
    cur = dirtree->GetItemParent(cur);
  }

 return path.Insert(0, "/");

}


bool cssVFSFileDlg :: HasChildDir( wxTreeItemId parent )
{
	csString path = GetPathByTreeItem( parent ); 
	csRef<iStringArray> childList = vfs->FindFiles( path );

	if( !childList )
		return false;

	if( childList->GetSize() > 0 && !strcmp( childList->Get(0), path))
		return false;

	size_t numChildren = childList->GetSize();
	size_t childIndex;

	for( childIndex = 0; childIndex < numChildren; childIndex++ )
	{
		const char* childPath = childList->Get( childIndex );
		if( IsDir( childPath) )
		{
			return true;
			break;
		}/*IF*/
	}/*FOR*/

	return false;
}

void cssVFSFileDlg :: InitViewTypeList()
{
	viewTypes->Append(_("List View"), (void*)LIST );
	viewTypes->Append(_("Details View"), (void*)DETAIL);
	viewTypes->Append(_("Icon View"), (void*)ICON);
	viewTypes->Append(_("Report View"), (void*)REPORT);
	
	viewTypes->SetSelection( 0 );
}

bool cssVFSFileDlg :: IsDir( const char* path )
{
	char lastChar = 0;
	lastChar = path[ strlen( path ) -1 ];

	return ( (lastChar == '/') ? TRUE : FALSE);
}
bool cssVFSFileDlg :: IsFile( const char* path )
{
	char lastChar = 0;
	lastChar = path[ strlen( path ) -1 ];

	return ( (lastChar != '/') ? TRUE : FALSE);
}


bool cssVFSFileDlg :: LoadVFSDirView(const char *path)
{
  fileView->DeleteAllItems();

  csRef<iStringArray> entry = vfs->FindFiles(path);
  
  if(!entry)
    return false;
    
  entry->Sort();

  long nentries = (long) entry->GetSize();
  int id= -1;

  for(long i = 0; i < nentries; i++)
  {
  //  char *curentry = (char*)entry->Get(i);
		wxString name = wxString(entry->Get(i), wxConvUTF8);	

		if( name.Last() == '/' ) //if DIR
		{
			name.RemoveLast();
			id = fileView->InsertItem(i, name.AfterLast('/'));
			fileView->SetItemImage( id, imageIdList[IMAGE_FOLDER_CLOSED], imageIdList[IMAGE_FOLDER_CLOSED]);
			fileView->SetItemData( id, FOLDER);
		}
		else
		{
			id = fileView->InsertItem(i, name.AfterLast('/'));
			fileView->SetItemImage( id, imageIdList[IMAGE_FILE], imageIdList[IMAGE_FILE]);
			fileView->SetItemData( id, FILE);
		}		
  }
  return true;
}



bool cssVFSFileDlg :: LoadVFSTreeBranch(wxTreeItemId parent, const char *path)
{
  csRef<iStringArray> childList = vfs->FindFiles(path);
	csRef<iStringArray> grandChildList = NULL;

  if(!childList)
    return false;

  if(childList->GetSize() > 0 && !strcmp(childList->Get(0), path))
    return false;
  
  childList->Sort();  
  
  size_t numberEntries = childList->GetSize();
  size_t entryIndex;

  for( entryIndex = 0; entryIndex < numberEntries; entryIndex++ )
  {
		wxString curTestPath = wxString(childList->Get( entryIndex ), wxConvUTF8);
		if( curTestPath.Last() == '/' )
    {
			curTestPath.RemoveLast();
		  curTestPath.Remove(0, curTestPath.Find('/', TRUE) +1);

			wxTreeItemId curTreeItem = dirtree->AppendItem(parent, curTestPath );
			FileDlgTreeItemData*  data = new FileDlgTreeItemData( curTestPath.Append('/') );
      
			
			dirtree->SetItemData( curTreeItem, data );
			dirtree->SetItemImage( curTreeItem, imageIdList[IMAGE_FOLDER_CLOSED]);

			if( HasChildDir( curTreeItem ) )
				dirtree->SetItemHasChildren( curTreeItem, TRUE );

		}//EndIf
  }//EndFor

  return true; // success
}




void cssVFSFileDlg :: OpenTreeItemChild( wxString childName )
{
#if wxCHECK_VERSION(2,5,2)
  wxTreeItemIdValue cookie;
#else
  long cookie;
#endif
  wxString curChildName;

  wxTreeItemId curTreeItem = dirtree->GetSelection();
  wxTreeItemId curTreeChild;

  csString curDirPath = GetPathByTreeItem( curTreeItem );

  if( dirtree->GetChildrenCount(curTreeItem, FALSE) == 0 )
  {		
    LoadVFSTreeBranch(curTreeItem, curDirPath.GetData());
  }


  curTreeChild = dirtree->GetFirstChild( curTreeItem, cookie);
  curChildName = dirtree->GetItemText( curTreeChild );

  while( (curChildName != childName) )
  {
    curTreeChild = dirtree->GetNextChild( curTreeItem, cookie);
    curChildName = dirtree->GetItemText( curTreeChild );
  }

  dirtree->EnsureVisible( curTreeChild );
  dirtree->SelectItem( curTreeChild );

}


//Event Methods
void cssVFSFileDlg :: OnCancel( wxCommandEvent& event )
{
	//Return FALSE tells main frame to do nothing
	EndModal(FALSE);
}

void cssVFSFileDlg :: OnDirTreeCollapseBranch( wxTreeEvent& event )
{
	wxTreeItemId curSelection = event.GetItem();
//	dirtree->SetItemImage( curSelection, imageIdList[IMAGE_FOLDER_CLOSED] );
}
void cssVFSFileDlg :: OnDirTreeExpandBranch( wxTreeEvent& event )
{
	wxTreeItemId curSelection = event.GetItem();
	csString curDirPath = GetPathByTreeItem( curSelection );
	
	if( dirtree->GetChildrenCount(curSelection, FALSE) == 0 )
	{		
		LoadVFSTreeBranch(curSelection, curDirPath.GetData());
	}

	//Select the item that was expanded
	dirtree->SelectItem( curSelection );
}

void cssVFSFileDlg :: OnDirTreeSelChange(wxTreeEvent& event)
{
  curdvpath = GetPathByTreeItem(event.GetItem());
  LoadVFSDirView(curdvpath);
  thepath->SetValue(wxString(curdvpath.GetData(), wxConvUTF8));
}


void cssVFSFileDlg :: OnDirViewActivated( wxListEvent & event )
{
	long curItem = event.GetIndex();
	long data = fileView->GetItemData( curItem );

	switch( data )
	{
	case FILE:
		EndModal( TRUE );
		break;
	case FOLDER:
		OpenTreeItemChild( fileView->GetItemText( event.GetIndex() ) );
		break;
	} 
}

void cssVFSFileDlg :: OnDirViewSelChange(wxListEvent& event)
{
  filename = (const char*)event.GetText().mb_str(wxConvUTF8);

  wxString fullpath (curdvpath, wxConvUTF8);
  fullpath.Append(wxString(filename, wxConvUTF8));

  thepath->SetValue(fullpath);
}
void cssVFSFileDlg :: OnOk( wxCommandEvent& event )
{
  filename = (const char*)thepath->GetValue().mb_str(wxConvUTF8);
  
  if (!filename)
    return;

  if (IsFile(filename))
  {
    EndModal(TRUE);
  }
  else if (IsDir(filename))
  {
    // TODO: Open the dir;
  }
}

void cssVFSFileDlg :: OnPathTextEnter( wxCommandEvent& event )
{
	if( IsFile( filename ) )
	{
		EndModal(TRUE);
	}
	else
	{
	}
}




void cssVFSFileDlg :: OnOpenParent( wxCommandEvent& event )
{
	wxTreeItemId rootItem = dirtree->GetRootItem();
	wxTreeItemId curItem = dirtree->GetSelection();

	if( rootItem != curItem )
	{
		wxTreeItemId parentItem = dirtree->GetItemParent( curItem );

		dirtree->EnsureVisible( parentItem );
		dirtree->SelectItem( parentItem );
	}
}

void cssVFSFileDlg :: OnViewModeSelect( wxCommandEvent& event )
{
	size_t data = (size_t) viewTypes->GetClientData( viewTypes->GetSelection() );

	switch( data )
	{
	case LIST:
          //fileView->SetWindowStyleFlag(wxLC_LIST);
		break;
	case REPORT:
          //fileView->SetWindowStyleFlag(wxLC_REPORT);
		break;
	case ICON:
          //fileView->SetWindowStyleFlag(wxLC_ICON);
		break;
	case DETAIL:
          //fileView->SetWindowStyleFlag(wxLC_SMALL_ICON);
		break;
	default:
		break;
	}
        fileView->Refresh ();
}


void cssVFSFileDlg::OnRightClickItem( wxTreeEvent& event)
{
  /// do nothing
}



