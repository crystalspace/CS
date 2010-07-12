
#include "settingbase.h"

#include "elements/CEGUICombobox.h"
#include "elements/CEGUIEditbox.h"
#include "CEGUIWindowManager.h"

CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  using namespace CEGUI;

  SettingProperties::ConfigType SettingBase::d_ConfigType;
  SettingProperties::ConfigName SettingBase::d_ConfigName;

  //Child Widget name suffix constants
  const String SettingBase::NameNameSuffix( "__auto_name__" );

  SettingBase::SettingBase(const String& type, const String& name, iObjectRegistry* obj_reg) 
    : Window(type, name), obj_reg(obj_reg), setting(obj_reg)
  {
    addProperty(&d_ConfigType);
    addProperty(&d_ConfigName);
  }

  SettingBase::~SettingBase(void)
  {
  }

  void SettingBase::onTextChanged(WindowEventArgs& e)
  {
    Window* name = getNameW();
    name->setText(Window::getText());
  }

  void SettingBase::onFontChanged(WindowEventArgs& e)
  {
    Window* name = getNameW();
    name->setFont(Window::getFont());
  }

  const String& SettingBase::getConfigType() const
  {
    return configType;
  }

  void SettingBase::setConfigType(const String& value)
  {
    configType = value;
    Update();
  }

  const String& SettingBase::getConfigName() const
  {
    return configName;
  }

  void SettingBase::setConfigName(const String& value)
  {
    configName = value;
    Update();
  }

  Window* SettingBase::getNameW() const
  {
    return WindowManager::getSingleton().getWindow(getName() + NameNameSuffix);
  }

} CS_PLUGIN_NAMESPACE_END(cegui)
