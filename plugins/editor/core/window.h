/*
    Copyright (C) 2011 by Jelle Hellemans

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

#ifndef __CSEDITOR_WINDOW_H__
#define __CSEDITOR_WINDOW_H__

#include "cssysdef.h"

#include <wx/splitter.h>
#include <wx/panel.h>
#include <wx/bmpcbox.h>
#include "wx/aui/aui.h"
#include <wx/toolbar.h>
#include <wx/button.h>
#include <wx/menu.h>

#include "ieditor/layout.h"

namespace CS {
namespace EditorApp {

class ViewControl;
class BitmapComboBox;

class Window : public wxSplitterWindow
{
public:
  Window (iObjectRegistry* obj_reg, wxWindow* parent, bool horizontal = false);
  Window (iObjectRegistry* obj_reg, wxWindow* parent, ViewControl* control, bool horizontal = false);
  virtual ~Window ();
  
  bool Split();

private:
  void OnDClick(wxSplitterEvent& event);
  void OnUnsplitEvent(wxSplitterEvent& event);
  void OnSize (wxSizeEvent& ev);
  DECLARE_EVENT_TABLE()
  
private:
  iObjectRegistry* object_reg;
  bool horizontal;
};

class ViewControl : public wxPanel
{
public:
  ViewControl (iObjectRegistry* obj_reg, wxWindow* parent);
  virtual ~ViewControl ();
  wxWindow* GetRegion() { return toolbar; }
  void SetLayout(iLayout* l) { layout = l; }
private:
  void OnClicked (wxCommandEvent& event);
  
  void OnSize (wxSizeEvent& ev);
  DECLARE_EVENT_TABLE()
private:
  iObjectRegistry* object_reg;
  csRef<iSpace> space;
  friend class BitmapComboBox;
  wxBoxSizer* box;
  wxWindow* toolbar;
  csRef<iLayout> layout;
};

class BitmapComboBox : public wxBitmapComboBox
{
public:
  BitmapComboBox (iObjectRegistry* obj_reg, wxWindow* parent, ViewControl* ctrl);
  virtual ~BitmapComboBox ();
private:
 void OnSelected (wxCommandEvent& event);
private:
  iObjectRegistry* object_reg;
  ViewControl* control;
};

} // namespace EditorApp
} // namespace CS

#endif
