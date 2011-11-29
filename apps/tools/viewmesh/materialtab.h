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

#ifndef MATERIALTAB_H__
#define MATERIALTAB_H__

#include "tabbase.h"

#include <ivaria/reporter.h>

class MaterialTab : public TabBase
{
private:
  csPtr<iStringArray> GetMaterials ();
  bool UpdateMaterialSVs (const CEGUI::EventArgs& e);
  bool SetSV (const CEGUI::EventArgs& e);

  bool AddSVItem(size_t i, const char* name, const char* value);

public:
  MaterialTab(iObjectRegistry* obj_reg, AssetBase* ass);
  virtual ~MaterialTab();

  void Update(bool submeshes);
};

//-------------------------------------------------------

MaterialTab::MaterialTab(iObjectRegistry* obj_reg, AssetBase* ass) 
  : TabBase(obj_reg, ass)
{
  LoadLayout("materialtab.layout");
  AddToTabs();

  CEGUI::Window* btn = 0;

  btn = winMgr->getWindow("Materials/MatList");
  btn->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,
    CEGUI::Event::Subscriber(&MaterialTab::UpdateMaterialSVs, this));

  btn = winMgr->getWindow("Materials/SetSV");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&MaterialTab::SetSV, this));

  CEGUI::MultiColumnList* multiColumnList = static_cast<CEGUI::MultiColumnList*>(winMgr->getWindow("Materials/SVList"));
  multiColumnList->addColumn("Name", 0, CEGUI::UDim(0.6f, 0));
  multiColumnList->addColumn("Value", 1, CEGUI::UDim(0.7f, 0));
  multiColumnList->setSelectionMode(CEGUI::MultiColumnList::RowSingle);
  
}

MaterialTab::~MaterialTab() 
{
}

void MaterialTab::Update (bool submeshes)
{
  csRef<iStringArray> arr = GetMaterials();
  if (submeshes) UpdateList(arr, "SubMeshes/MatList");
  UpdateList(arr, "Materials/MatList");
  UpdateMaterialSVs(CEGUI::EventArgs());
}

csPtr<iStringArray> MaterialTab::GetMaterials ()
{
  scfStringArray* arr = new scfStringArray;
  csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
  iCollection* collection = engine->CreateCollection ("viewmesh_region");
  for (int i=0; i<engine->GetMaterialList()->GetCount(); ++i)
  {
    iMaterialWrapper* mat = engine->GetMaterialList()->Get(i);
    if(!collection->IsParentOf(mat->QueryObject()))
      continue;

    const char* name = mat->QueryObject()->GetName();
    if (name)
    {
      csString s = name;
      arr->Push (s);
    }
  }

  return csPtr<iStringArray>(arr);
}

bool MaterialTab::AddSVItem(size_t i, const char* name, const char* value)
{
  CEGUI::MultiColumnList* multiColumnList = static_cast<CEGUI::MultiColumnList*>(winMgr->getWindow("Materials/SVList"));

  multiColumnList->addRow();

  CEGUI::ListboxTextItem* item = new CEGUI::ListboxTextItem(name, 100 + (uint)i);
  item->setTextColours(CEGUI::colour(0,0,0));
  item->setSelectionBrushImage("ice", "TextSelectionBrush");
  item->setSelectionColours(CEGUI::colour(0.5f,0.5f,1));
  multiColumnList->setItem(item, 0, (uint)i); // ColumnID, RowID

  item = new CEGUI::ListboxTextItem(value, 101 + (uint)i);
  item->setTextColours(CEGUI::colour(0,0,0));
  item->setSelectionBrushImage("ice", "TextSelectionBrush");
  item->setSelectionColours(CEGUI::colour(0.5f,0.5f,1));
  multiColumnList->setItem(item, 1, (uint)i);

  return true;
}

bool MaterialTab::UpdateMaterialSVs (const CEGUI::EventArgs& e)
{
  csString matName;
  if (!GetSelectedItemText("Materials/MatList", matName)) return true;

  csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
  iMaterialWrapper* mat = engine->GetMaterialList()->FindByName(matName);
  if(mat)
  {
    CEGUI::MultiColumnList* multiColumnList = static_cast<CEGUI::MultiColumnList*>(winMgr->getWindow("Materials/SVList"));
    multiColumnList->resetList();
    csRef<iShaderVarStringSet> svstrings = csQueryRegistryTagInterface<iShaderVarStringSet>(object_reg,
      "crystalspace.shader.variablenameset");

    csString tmp;
    
    for(size_t i=0; i<mat->GetMaterial()->GetShaderVariables().GetSize(); ++i)
    {
      csShaderVariable* sv = mat->GetMaterial()->GetShaderVariables().Get(i);
      switch(sv->GetType())
      {
      case csShaderVariable::TEXTURE:
        {
          iTextureWrapper* tex;
          sv->GetValue(tex);
	  csString name = tex ? tex->QueryObject()->GetName() : "not found";

          AddSVItem(i, svstrings->Request(sv->GetName()), name);
          break;
        }
      case csShaderVariable::INT:
        {
          int var;
          sv->GetValue(var);
          AddSVItem(i, svstrings->Request(sv->GetName()), tmp.Format("%d", var));
          break;
        }
      case csShaderVariable::FLOAT:
        {
          float var;
          sv->GetValue(var);
          AddSVItem(i, svstrings->Request(sv->GetName()), tmp.Format("%.2f", var));
          break;
        }
      case csShaderVariable::VECTOR2:
        {
          csVector2 vec;
          sv->GetValue(vec);
          AddSVItem(i, svstrings->Request(sv->GetName()), tmp.Format("%.2f, %.2f", vec.x, vec.y));
          break;
        }
      case csShaderVariable::VECTOR3:
        {
          csVector3 vec;
          sv->GetValue(vec);
          AddSVItem(i, svstrings->Request(sv->GetName()), tmp.Format("%.2f, %.2f, %.2f", vec.x, vec.y, vec.z));
          break;
        }
      case csShaderVariable::VECTOR4:
        {
          csVector4 vec;
          sv->GetValue(vec);
          AddSVItem(i, svstrings->Request(sv->GetName()), tmp.Format("%.2f, %.2f, %.2f, %.2f", vec.x, vec.y, vec.z, vec.w));
          break;
        }
      default:
        {
          continue;
        }
      }
    }

    return true;
  }

  return true;
}

bool MaterialTab::SetSV (const CEGUI::EventArgs& e)
{
  csString matName;
  if (!GetSelectedItemText("Materials/MatList", matName)) return true;

  csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
  iCollection* collection = engine->CreateCollection ("viewmesh_region");
  iMaterialWrapper* mat = engine->GetMaterialList()->FindByName(matName);
  if(mat)
  {
    csRef<iShaderVarStringSet> svstrings = csQueryRegistryTagInterface<iShaderVarStringSet>(object_reg,
      "crystalspace.shader.variablenameset");
    CEGUI::MultiColumnList* svlist = (CEGUI::MultiColumnList*)winMgr->getWindow("Materials/SVList");
    CEGUI::ListboxItem* item = svlist->getFirstSelectedItem();
    if(!item) return true;
    const CEGUI::String& svs = item->getText();
    if (svs.empty()) return true;

    const char* svname = svs.c_str();
    for(size_t i=0; i<mat->GetMaterial()->GetShaderVariables().GetSize(); ++i)
    {
      csShaderVariable* sv = mat->GetMaterial()->GetShaderVariables().Get(i);
      if(!strcmp(svname, svstrings->Request(sv->GetName())))
      {
        switch(sv->GetType())
        {
        case csShaderVariable::TEXTURE:
          {
            CEGUI::Window* svinput = winMgr->getWindow("Materials/SVInput");
            CEGUI::String name = svinput->getProperty("Text");
            iTextureWrapper* tex = collection->FindTexture(name.c_str());
            if(!tex)
              return true;

            sv->SetValue(tex);

            svlist->getNextSelected(item)->setText(name.c_str());
            svlist->setItemSelectState(item, false); // Force update.
            svlist->setItemSelectState(item, true);
            break;
          }
        case csShaderVariable::INT:
          {
            CEGUI::Window* svinput = winMgr->getWindow("Materials/SVInput");
            CEGUI::String name = svinput->getProperty("Text");

            int var;
            sscanf(name.c_str(), "%d", &var);
            sv->SetValue(var);

            svlist->getNextSelected(item)->setText(name.c_str());
            svlist->setItemSelectState(item, false); // Force update.
            svlist->setItemSelectState(item, true);
            break;
          }
        case csShaderVariable::FLOAT:
          {
            CEGUI::Window* svinput = winMgr->getWindow("Materials/SVInput");
            CEGUI::String name = svinput->getProperty("Text");

            float var;
            sscanf(name.c_str(), "%f", &var);
            sv->SetValue(var);

            svlist->getNextSelected(item)->setText(name.c_str());
            svlist->setItemSelectState(item, false); // Force update.
            svlist->setItemSelectState(item, true);
            break;
          }
        case csShaderVariable::VECTOR2:
          {
            CEGUI::Window* svinput = winMgr->getWindow("Materials/SVInput");
            CEGUI::String name = svinput->getProperty("Text");

            csVector2 vec;
            sscanf(name.c_str(), "%f, %f", &vec.x, &vec.y);
            sv->SetValue(vec);

            svlist->getNextSelected(item)->setText(name.c_str());
            svlist->setItemSelectState(item, false); // Force update.
            svlist->setItemSelectState(item, true);
            break;
          }
        case csShaderVariable::VECTOR3:
          {
            CEGUI::Window* svinput = winMgr->getWindow("Materials/SVInput");
            CEGUI::String name = svinput->getProperty("Text");

            csVector3 vec;
            sscanf(name.c_str(), "%f, %f, %f", &vec.x, &vec.y, &vec.z);
            sv->SetValue(vec);

            svlist->getNextSelected(item)->setText(name.c_str());
            svlist->setItemSelectState(item, false); // Force update.
            svlist->setItemSelectState(item, true);
            break;
          }
        case csShaderVariable::VECTOR4:
          {
            CEGUI::Window* svinput = winMgr->getWindow("Materials/SVInput");
            CEGUI::String name = svinput->getProperty("Text");

            csVector4 vec;
            sscanf(name.c_str(), "%f, %f, %f, %f", &vec.x, &vec.y, &vec.z, &vec.w);
            sv->SetValue(vec);

            svlist->getNextSelected(item)->setText(name.c_str());
            svlist->setItemSelectState(item, false); // Force update.
            svlist->setItemSelectState(item, true);
            break;
          }
        default:
          {
            continue;
          }
        }
      }
    }

    return true;
  }

  return true;
}




#endif // MATERIALTAB_H__
