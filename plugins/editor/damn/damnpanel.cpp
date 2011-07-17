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

#include "cssysdef.h"
#include "csutil/scf.h"
#include "csutil/sysfunc.h"

#include "iengine/sector.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"

#include "inetwork/http.h"

#include "csutil/resource.h"

#include "include/resource.h"

#include <igraphic/image.h>
#include <csutil/databuf.h>

#include <ivaria/pmeter.h>

#include <csgfx/packrgb.h>

#include <wx/wx.h>
#include <wx/dnd.h>
#include <wx/srchctrl.h>
#include <wx/sizer.h>

#include <wx/mstream.h>

#include "damnpanel.h"

#include "json/json.h"

#include <iostream>
#include <sstream>
#include <string>

const int ICONSIZE = 128;


CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{
  
SCF_IMPLEMENT_FACTORY (DAMNPanel)

BEGIN_EVENT_TABLE(DAMNPanel, wxPanel)
  EVT_SIZE(DAMNPanel::OnSize)
END_EVENT_TABLE()


DAMNPanel::DAMNPanel (iBase* parent)
 : scfImplementationType (this, parent), sizer(0), srchCtrl(0), previewList(0)
{
}

bool DAMNPanel::Initialize (iObjectRegistry* obj_reg)
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
  
  using namespace CS::Network::HTTP;  
  plugmgr = csQueryRegistry<iPluginManager> (object_reg);
  http = csLoadPlugin<iHTTPConnectionFactory> (plugmgr, "crystalspace.network.factory.http");
  damn = csLoadPlugin<iResourceManager> (plugmgr, "crystalspace.resources.managers.damn");
  
  csRef<iFormatAbstractor> abs = scfQueryInterface<iFormatAbstractor>(damn);
  abs->AddAbstraction("preview", "format=image/png&sizex=128&sizey=128&angley=1.54");
  abs->AddAbstraction("image", "format=image/png&sizex=512&sizey=512");
  abs->AddAbstraction("mesh", "format=application/x-crystalspace.library%2Bxml");

  // Create the panel
  Create (editor->GetWindow (), -1, wxPoint (0, 0), wxSize (ICONSIZE*2, 250));

  // Add it to the panel manager
  panelManager->AddPanel (this);
  
  wxBoxSizer* box = new wxBoxSizer(wxVERTICAL);
  SetSizer(box);
 
  srchCtrl = new wxSearchCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxSize(250, -1), 0);
  srchCtrl->ShowCancelButton(true);
  
  this->Connect(srchCtrl->GetId(), wxEVT_COMMAND_SEARCHCTRL_SEARCH_BTN, wxCommandEventHandler(DAMNPanel::OnSearchButton), 0, this);
  this->Connect(srchCtrl->GetId(), wxEVT_COMMAND_SEARCHCTRL_CANCEL_BTN, wxCommandEventHandler(DAMNPanel::OnCancelButton), 0, this);
  box->Add(srchCtrl, 0, wxTOP|wxEXPAND, 0);
  
  
  previewList = new wxScrolledWindow(this);
  previewList->SetScrollbars(0, 1, 0, 0);
  box->Add(previewList, 1, wxALL|wxEXPAND, 0);
  
  sizer = new wxGridSizer(1, 2, 0, 0);
  previewList->SetSizer(sizer);
    
  return true;
}

DAMNPanel::~DAMNPanel ()
{
}

wxWindow* DAMNPanel::GetWindow ()
{
  return this;
}

const wxChar* DAMNPanel::GetCaption () const
{
  return wxT("DAMN Browser");
}

PanelDockPosition DAMNPanel::GetDefaultDockPosition () const
{
  return DockPositionRight;
}

void DAMNPanel::OnSize (wxSizeEvent& event)
{
  // Resize the tree control
  /*if (listCtrl)
    listCtrl->SetSize (event.GetSize());*/
  if (sizer)
  {
    //TODO: this should really be in the previewList::OnSize
    sizer->SetCols(std::max(1, event.GetSize().GetWidth()/128));
  }
  event.Skip();
}

void DAMNPanel::OnLoaded (iLoadingResource* resource)
{
  if (resource->Get()) {
    csRef<iImage> image = scfQueryInterface<iImage>(resource->Get());
    SearchResults::const_iterator found = searchResults.find(resource);
    std::string subName;
    if (found != searchResults.end())
    {
      subName =  found->second;
    }
    
    Preview* preview = new Preview(previewList, image, subName);
    sizer->Add(preview, 1, wxSHAPED);
    previewList->FitInside();
  }
  meter->Step();
  if (meter->GetCurrent () == meter->GetTotal ())
    meter->SetProgressDescription("DAMN: ", "Done.");
}

void DAMNPanel::OnSearchButton (wxCommandEvent& event)
{
  printf("DAMNPanel::OnSearchButton %s\n", (const char*)srchCtrl->GetValue().mb_str());
  std::string searchTerm((const char*)srchCtrl->GetValue().mb_str());
  
  if (searchTerm == "") searchTerm = "shield";

  sizer->Clear();
  SearchResults::const_iterator it = searchResults.begin();
  for(; it != searchResults.end();it++)
  {
    it->first->RemoveListener(this);
  }
  searchResults.clear();
  
  meter = editor->GetProgressMeter ();

  if (http)
  {
    meter->SetProgressDescription("DAMN: ", "Searching...");
    using namespace CS::Network::HTTP;
    csRef<iHTTPConnection> client = http->Create("http://damn.peragro.org/");
    //csRef<iResponse> response = client->Get("assets/search/", "tags=5&tags=6&formats=142&search=shield");
    csRef<iResponse> response = client->Get("assets/search/", ("formats=656&search="+searchTerm).c_str());
    if (response && response->GetState() == OK && response->GetCode() == 200)
    {
      Json::Value root;
      Json::Reader reader;
      std::istringstream is(response->GetData()->GetData());
      bool parsingSuccessful = reader.parse(is, root);
      
      if (root.size())
      {
        meter->SetProgressDescription("DAMN: ", "Downloading...");
        meter->SetTotal((int)root.size());
      }
      else
      {
        meter->SetProgressDescription("DAMN: ", "No results");
      }
  
      for (unsigned int index = 0; index < root.size(); ++index ) 
      {
        std::string pk = root[index]["pk"].asString();
        std::string subName = root[index]["fields"]["subName"].asString();
        //printf("test1 %s\n", pk.c_str());
        csRef<iLoadingResource> image = damn->Get(CS::Resource::HashID ("image"), (pk+"::preview").c_str());
        searchResults[image] = subName;
        image->AddListener(this);
      }
    }
    else if (response)
    {
      meter->SetProgressDescription("DAMN: ", "Error %s (%d)", response->GetError(), response->GetCode());
    }
    else
    {
      meter->SetProgressDescription("DAMN: ", "Invalid response!");
    }
  }

}

void DAMNPanel::OnCancelButton (wxCommandEvent& event)
{
   printf("success c\n");
   csRef<iLoadingResource> image = damn->Get(CS::Resource::HashID ("factory"), "782b83441a749df48b085f35655558700d1f1f17::mesh");
}


// ----------------------------------------------------------------------------

Preview::Preview(wxWindow* parent, csRef<iImage> image, const std::string& label) : wxPanel(parent), bitmapBtn(0), text(0), sizer(0)
{
  size_t width = image->GetWidth (); 
  size_t height = image->GetHeight (); 
  size_t total = width * height;
  
  bool alphac = true;
  int channels = 4;

  wxImage wximage = wxImage(width, height, false);

  unsigned char* rgb = 0;
  unsigned char* alpha = 0;
  rgb = (unsigned char*) malloc (total * 3); //Memory owned and free()d by wxImage
  if (alphac)
    alpha = (unsigned char*) malloc (total); //Memory owned and free()d by wxImage
  unsigned char* source = (unsigned char*)image->GetImageData();
  for (size_t i = 0; i < total; i++)
  {
    rgb[3*i] = source[channels*i];
    rgb[(3*i)+1] = source[(channels*i)+1];
    rgb[(3*i)+2] = source[(channels*i)+2];
    if (alphac)
      alpha[i] = source[(channels*i)+3];
  }
  wximage.SetData(rgb);
  if (alphac) wximage.SetAlpha(alpha);
  
  sizer = new wxBoxSizer(wxVERTICAL);
  bitmapBtn = new wxBitmapButton(this, -1, wxBitmap(wximage), wxDefaultPosition, wxSize(ICONSIZE,ICONSIZE));
  
  sizer->Add(bitmapBtn, 1, wxEXPAND | wxALL, 3);
  
  text = new wxStaticText(this, wxID_ANY, wxString(label.c_str(), wxConvUTF8));
  sizer->Add(text, 0, wxALIGN_CENTRE | wxALL, 3);
  SetSizer(sizer);
  sizer->SetSizeHints(this);
  this->Connect(bitmapBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Preview::OnClicked), 0, this);
  this->Connect(this->GetId(), wxEVT_SIZE, wxSizeEventHandler(Preview::OnSize), 0, this);
}

void Preview::OnClicked (wxCommandEvent& event)
{
  printf("OnClicked\n");
}

void Preview::OnSize (wxSizeEvent& event)
{
/*
  printf("Preview::OnSize %d - %d\n", event.GetSize().GetWidth(), event.GetSize().GetHeight());
  bitmapBtn->SetSize (event.GetSize());
  event.Skip();
*/
}

  // ----------------------------------------------------------------------------
}
CS_PLUGIN_NAMESPACE_END(CSE)
