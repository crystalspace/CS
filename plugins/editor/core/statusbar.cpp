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

#include <cssysdef.h>
#include "csutil/scf.h"

#include <wx/event.h>

#include "statusbar.h"

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

BEGIN_EVENT_TABLE(StatusBar, wxStatusBar)
  EVT_SIZE(StatusBar::OnSize)
END_EVENT_TABLE()

StatusBar::StatusBar (wxWindow* parent)
  : wxStatusBar (parent)
{
  static const int widths[Field_Max] = {-1, 150, 30};

  SetFieldsCount(Field_Max);
  SetStatusWidths(Field_Max, widths);

  gauge = new wxGauge(this, wxID_ANY, 100);
  gauge->SetValue(0);
}

StatusBar::~StatusBar ()
{
}

void StatusBar::OnSize (wxSizeEvent& event)
{
  wxRect gaugeRect;
  GetFieldRect(Field_Gauge, gaugeRect);

  gauge->SetSize(gaugeRect);

  event.Skip();
}

}
CS_PLUGIN_NAMESPACE_END(CSE)
