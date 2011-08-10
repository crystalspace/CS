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

#ifndef __CSEDITOR_LAYOUTS_H__
#define __CSEDITOR_LAYOUTS_H__

#include "csutil/scf_implementation.h"
#include <csutil/refarr.h>
#include <csutil/hash.h>

#include "ieditor/layout.h"
#include "ieditor/menu.h"
#include "ieditor/operator.h"
#include "ieditor/context.h"

#include <wx/event.h>
#include <wx/collpane.h>

class wxWindow;
class wxCommandEvent;
class wxMenu;
class wxToolBar;
class wxBoxSizer;
class wxSizer;

namespace CS {
namespace EditorApp {
  
struct BaseLayout : public scfImplementation1<BaseLayout,iLayout>, public wxEvtHandler
{
  BaseLayout(iObjectRegistry* obj_reg);
  iObjectRegistry* object_reg;
  
  csHash<csRef<iOperator>, int> operators;
  void OnOperator (wxCommandEvent& event);
  
  csHash<csRef<iMenu>, int> menus;
  void OnMenu (wxCommandEvent& event);
  
  iOperator* GetOperator (const char* id);
  iMenu* GetMenu (const char* id);
  
  virtual wxWindow* GetWindow() = 0;
};

//----------------------------------------------------------------------

struct HeaderLayout : public BaseLayout
{
  virtual iOperator* AppendOperator(const char* id, const char* label, const char* icon);
  virtual iMenu* AppendMenu(const char* id, const char* label);
  virtual iProperty* AppendProperty(iResource*, const char* id);
  virtual void AppendLabel(const char* label);
  virtual void AppendSeperator();

  virtual iLayout* Row();
  virtual iLayout* Column();
  
  virtual wxWindow* GetWindow();
  
  HeaderLayout(iObjectRegistry* obj_reg, wxWindow* parent);
  ~HeaderLayout();
  wxToolBar* tb;
  wxBoxSizer* box;
};

//----------------------------------------------------------------------

struct MenuLayout : public BaseLayout
{
  virtual iOperator* AppendOperator(const char* id, const char* label, const char* icon);
  virtual iMenu* AppendMenu(const char* id, const char* label);
  virtual iProperty* AppendProperty(iResource*, const char* id);
  virtual void AppendLabel(const char* label);
  virtual void AppendSeperator();

  virtual iLayout* Row();
  virtual iLayout* Column();
  
  virtual wxWindow* GetWindow();
  
  MenuLayout(iObjectRegistry* obj_reg, wxWindow*, wxMenu*);
  ~MenuLayout();
  iObjectRegistry* object_reg;
  wxWindow* parent;
  wxMenu* menu;
};

//----------------------------------------------------------------------

struct PanelLayout : public BaseLayout
{
  virtual iOperator* AppendOperator(const char* id, const char* label, const char* icon);
  virtual iMenu* AppendMenu(const char* id, const char* label);
  virtual iProperty* AppendProperty(iResource*, const char* id);
  virtual void AppendLabel(const char* label);
  virtual void AppendSeperator();

  virtual iLayout* Row();
  virtual iLayout* Column();
  
  virtual wxWindow* GetWindow();
  
  PanelLayout(iObjectRegistry* obj_reg, wxWindow*);
  ~PanelLayout();
  iObjectRegistry* object_reg;
  wxWindow* parent;
  wxSizer* paneSz;
};

//----------------------------------------------------------------------

class CollapsiblePane : public wxCollapsiblePane
{
public:
  CollapsiblePane (iObjectRegistry*, wxWindow*, const char*);
  virtual ~CollapsiblePane ();
  void SetLayout(iLayout* l) { layout = l; }
private:
 void OnSize (wxSizeEvent& ev);
  DECLARE_EVENT_TABLE()
private:
  iObjectRegistry* object_reg;
  csRef<iLayout> layout;
};

//----------------------------------------------------------------------
	
} // namespace EditorApp
} // namespace CS

#endif
