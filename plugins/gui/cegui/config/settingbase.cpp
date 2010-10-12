#include "settingbase.h"

CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  SettingProperties::ConfigTypes SettingBase::d_ConfigTypes;
  SettingProperties::ConfigNames SettingBase::d_ConfigNames;

  //Child Widget name suffix constants
  const CEGUI::String SettingBase::NameNameSuffix( "__auto_name__" );

  SettingBase::SettingBase(const CEGUI::String& type, const CEGUI::String& name, iObjectRegistry* obj_reg) 
    : Window(type, name), obj_reg(obj_reg)
  {
    addProperty(&d_ConfigTypes);
    addProperty(&d_ConfigNames);
  }

  SettingBase::~SettingBase(void)
  {
  }

  void SettingBase::onTextChanged(CEGUI::WindowEventArgs& e)
  {
    CEGUI::Window* name = getNameW();
    name->setText(CEGUI::Window::getText());
  }

  void SettingBase::onFontChanged(CEGUI::WindowEventArgs& e)
  {
    CEGUI::Window* name = getNameW();
    name->setFont(CEGUI::Window::getFont());
  }

  CEGUI::String SettingBase::getConfigTypes() const
  {
    CEGUI::String s;
    for (size_t i = 0; i < settings.GetSize(); i++)
    {
      s += settings.Get(i)->GetValueType();
      if (i < settings.GetSize()-1) s += ",";
    }
    return s;
  }

  void SettingBase::setConfigTypes(const CEGUI::String& value)
  {
    std::vector<std::string> types;

    Tokenize(value.c_str(), types, ",");

    if (types.size() == settings.GetSize())
    {
      std::vector<std::string>::const_iterator it = types.begin();
      for (size_t i = 0; it != types.end(); it++, i++)
      {
        settings.Get(i)->SetValueType(*it);
      }
    }
    else
    {
      settings.DeleteAll();
      std::vector<std::string>::const_iterator it = types.begin();
      for (; it != types.end(); it++)
      {
        csRef<Setting> s; s.AttachNew(new Setting(obj_reg));
        s->SetValueType(*it);
        settings.Push(s);
      }
    }

    Update();
  }

  CEGUI::String SettingBase::getConfigNames() const
  {
    CEGUI::String s;
    for (size_t i = 0; i < settings.GetSize(); i++)
    {
      s += settings.Get(i)->GetValueName();
      if (i < settings.GetSize()-1) s += ",";
    }
    return s;
  }

  void SettingBase::setConfigNames(const CEGUI::String& value)
  {
    std::vector<std::string> names;

    Tokenize(value.c_str(), names, ",");

    if (names.size() == settings.GetSize())
    {
      std::vector<std::string>::const_iterator it = names.begin();
      for (size_t i = 0; it != names.end(); it++, i++)
      {
        settings.Get(i)->SetValueName(*it);
      }
    }
    else
    {
      settings.DeleteAll();
      std::vector<std::string>::const_iterator it = names.begin();
      for (; it != names.end(); it++)
      {
        csRef<Setting> s; s.AttachNew(new Setting(obj_reg));
        s->SetValueName(*it);
        settings.Push(s);
      }
    }

    Update();
  }

  CEGUI::Window* SettingBase::getNameW() const
  {
    return CEGUI::WindowManager::getSingleton().getWindow(getName() + NameNameSuffix);
  }

  void SettingBase::Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters)
  {
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);
    while (std::string::npos != pos || std::string::npos != lastPos)
    {
      tokens.push_back(str.substr(lastPos, pos - lastPos));
      lastPos = str.find_first_not_of(delimiters, pos);
      pos = str.find_first_of(delimiters, lastPos);
    }
  }


} CS_PLUGIN_NAMESPACE_END(cegui)
