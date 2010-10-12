/*
    Copyright (C) 2009 Development Team of Peragro Tempus

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General public License for more details.

    You should have received a copy of the GNU General public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "setting.h"

#include <iostream>
#include <sstream>

CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
    CEGUI::String Setting::GetAsString()
    {
      switch (settingType)
      {
      case Bool:
        {
          bool v; Get(v);
          return CEGUI::PropertyHelper::boolToString(v);
        } break;
      case Int:
        {
          int v; Get(v);
          return CEGUI::PropertyHelper::intToString(v);
        } break;
      case Float:
        {
          float v; Get(v);
          return CEGUI::PropertyHelper::floatToString(v);
        } break;
      case String:
        {
          CEGUI::String v; Get(v);
          return v;
        } break;
      default:
        return "ERROR 3";
      } // end switch
    }

    void Setting::SetValueType(const std::string& value)
    {
      if (value == "bool")
        settingType = Bool;
      else if (value == "int")
        settingType = Int;
      else if (value == "float")
        settingType = Float;
      else if (value == "string")
        settingType = String;
      else
        settingType = Undefined;
    }

    std::string Setting::GetValueType()
    {
      if (settingType == Bool)
        return "bool";
      else if (settingType == Int)
        return "int";
      else if (settingType == Float)
        return "float";
      else if (settingType == String)
        return "string";
      else
        return "ERROR";
    }

    bool Setting::IsDefault()
    {
      csRef<iConfigManager> app_cfg = csQueryRegistry<iConfigManager> (obj_reg); 
      return !app_cfg->KeyExists(name.c_str());
    }

    //--[Generic]---------------------------
    void Setting::SetFromString(const CEGUI::String& value)
    {
      csRef<iConfigManager> app_cfg = csQueryRegistry<iConfigManager> (obj_reg); 
      switch (settingType)
      {
      case Bool:
        {
          Set<bool>(CEGUI::PropertyHelper::stringToBool(value));
        } break;
      case Int:
        {
          Set<int>(CEGUI::PropertyHelper::stringToInt(value));
        } break;
      case Float:
        {
          Set<float>(CEGUI::PropertyHelper::stringToFloat(value));
        } break;
      case String:
        {
          Set<CEGUI::String>(value);
        } break;
      case Undefined:
      default:
        break;
      }
    }

    template<typename T>
    void Setting::SetFromOther(const T& value)
    {
      std::stringstream str;
      str << value;
      SetFromString(str.str().c_str());
    }
    
    template void Setting::SetFromOther(const bool& value);
    template void Setting::SetFromOther(const int& value);
    template void Setting::SetFromOther(const float& value);

    //--[Bool]------------------------------
    template<>
    void Setting::Get<bool>(bool& value)
    {
      csRef<iConfigManager> app_cfg = csQueryRegistry<iConfigManager> (obj_reg); 
      value = app_cfg->GetBool(name.c_str(), false);
    }

    template<>
    void Setting::Set<bool>(const bool& value)
    {
      csRef<iConfigManager> app_cfg = csQueryRegistry<iConfigManager> (obj_reg); 
      app_cfg->SetBool(name.c_str(), value);
      app_cfg->Save();
    }

    //--[Int]------------------------------
    template<>
    void Setting::Get<int>(int& value)
    {
      csRef<iConfigManager> app_cfg = csQueryRegistry<iConfigManager> (obj_reg); 
      value = app_cfg->GetInt(name.c_str(), 0);
    }

    template<>
    void Setting::Set<int>(const int& value)
    {
      csRef<iConfigManager> app_cfg = csQueryRegistry<iConfigManager> (obj_reg); 
      app_cfg->SetInt(name.c_str(), value);
      app_cfg->Save();
    }

    //--[Float]------------------------------
    template<>
    void Setting::Get<float>(float& value)
    {
      csRef<iConfigManager> app_cfg = csQueryRegistry<iConfigManager> (obj_reg); 
      value = app_cfg->GetFloat(name.c_str(), 0.0f);
    }

    template<>
    void Setting::Set<float>(const float& value)
    {
      csRef<iConfigManager> app_cfg = csQueryRegistry<iConfigManager> (obj_reg); 
      app_cfg->SetFloat(name.c_str(), value);
      app_cfg->Save();
    }

    //--[String]------------------------------
    template<>
    void Setting::Get<CEGUI::String>(CEGUI::String& value)
    {
      csRef<iConfigManager> app_cfg = csQueryRegistry<iConfigManager> (obj_reg); 
      value = app_cfg->GetStr(name.c_str(), "");
    }

    template<>
    void Setting::Set<CEGUI::String>(const CEGUI::String& value)
    {
      csRef<iConfigManager> app_cfg = csQueryRegistry<iConfigManager> (obj_reg); 
      app_cfg->SetStr(name.c_str(), value.c_str());
      app_cfg->Save();
    }

    //--[string]------------------------------
    template<>
    void Setting::Get<std::string>(std::string& value)
    {
      csRef<iConfigManager> app_cfg = csQueryRegistry<iConfigManager> (obj_reg); 
      value = app_cfg->GetStr(name.c_str(), "");
    }

    template<>
    void Setting::Set<std::string>(const std::string& value)
    {
      csRef<iConfigManager> app_cfg = csQueryRegistry<iConfigManager> (obj_reg); 
      app_cfg->SetStr(name.c_str(), value.c_str());
      app_cfg->Save();
    }

} CS_PLUGIN_NAMESPACE_END(cegui)
