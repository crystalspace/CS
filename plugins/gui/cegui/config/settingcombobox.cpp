
#include "settingcombobox.h"

#include <elements/CEGUICombobox.h>
#include <elements/CEGUIComboDropList.h>
#include <elements/CEGUIEditbox.h>
#include <elements/CEGUIListboxTextItem.h>
#include <elements/CEGUIScrollbar.h>
#include <CEGUIWindowManager.h>

CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  using namespace CEGUI;

  const String SettingComboBox::EventNamespace("SettingComboBox");
  const String SettingComboBox::WidgetTypeName("CEGUI/SettingComboBox");

  SettingProperties::Values SettingComboBox::d_Values;

  //Child Widget name suffix constants
  const String SettingComboBox::ComboboxSuffix( "__auto_combobox__" );


  SettingComboBox::SettingComboBox(const String& type, const String& name, iObjectRegistry* obj_reg) : SettingBase(type, name, obj_reg)
  {
    addProperty(&d_Values);
  }

  SettingComboBox::~SettingComboBox(void)
  {
  }

  void SettingComboBox::Update()
  {
    if (!setting)
    {
      setting.SetValueType(configType.c_str());
      setting.SetValueName(configName.c_str());
    }
    else
    {
      Combobox* box = getComboBoxW();
      box->getEditbox()->setText(GetKey(setting.GetAsString()));
      if (box->getItemCount() == 0)
      {
        std::vector<String> keys = GetKeys();
        for (size_t i =0; i < keys.size(); i++)
        {
          CEGUI::ListboxTextItem* item = new CEGUI::ListboxTextItem(keys[i], (CEGUI::uint)i);
          item->setTextColours(colour(0.f, 0.f, 0.f)); 
          box->getDropList()->addItem(item);
        }
      }
    }
  }

  void SettingComboBox::onFontChanged(WindowEventArgs& e)
  {
    Window* name = getNameW();
    Combobox* box = getComboBoxW();
    name->setFont(Window::getFont());
    box->setFont(Window::getFont());
  }

  void SettingComboBox::onSized(WindowEventArgs& e)
  {
    Window::onSized(e);
    Combobox* box = getComboBoxW();
    box->getEditbox()->setReadOnly(true);


    // Calculate height.
    float h = box->getEditbox()->getPixelSize().d_height+5.0f;
    for (size_t i =0; i < box->getItemCount(); i++)
    {
      h += box->getListboxItemFromIndex(i)->getPixelSize().d_height;
    }

    box->setHeight(UDim(0.0f, h));
    box->setClippedByParent(false);
    box->getDropList()->getVertScrollbar()->setEnabled(false);
    box->getDropList()->getVertScrollbar()->setAlpha(0.0f);
  }

  std::vector<String> Split(const String& string, const char& c)
  {
    std::vector<String> strs;
    String tmp(string);
    while (tmp.find_first_of(c) != String::npos)
    {
      String s(tmp.substr(0, tmp.find_first_of(c)));
      strs.push_back(s);
      tmp = tmp.substr(tmp.find_first_of(c)+1);
    }
    if (!tmp.empty())
      strs.push_back(tmp);
    return strs;
  }

  String SettingComboBox::GetValue(const String& key)
  {
    std::vector<String> strs = Split(values, ';');
    for (size_t i =0; i < strs.size(); i++)
    {
      std::vector<String> pair = Split(strs[i], ':');
      if (pair.size() == 2 && pair[0] == key)
        return pair[1];
    }
    return "ERROR 1";
  }

  String SettingComboBox::GetKey(const String& value)
  {
    // If the value is empty, default setting is assumed.
    if (value.empty()) return "Default";

    std::vector<String> strs = Split(values, ';');
    for (size_t i =0; i < strs.size(); i++)
    {
      std::vector<String> pair = Split(strs[i], ':');
      if (pair.size() == 2 && pair[1] == value)
        return pair[0];
    }
    return "ERROR 2";
  }

  std::vector<String> SettingComboBox::GetKeys()
  {
    std::vector<String> keys;
    std::vector<String> strs = Split(values, ';');
    for (size_t i =0; i < strs.size(); i++)
    {
      std::vector<String> pair = Split(strs[i], ':');
      if (pair.size() == 2)
        keys.push_back(pair[0]);
    }
    return keys;
  }

  bool SettingComboBox::onSelectionAccepted(const EventArgs& e)
  {
    Combobox* box = getComboBoxW();
    String key = box->getSelectedItem()->getText();
    if (setting)
    {
      setting.Set(GetValue(key));
    }

    return true;
  }

  String SettingComboBox::getValues() const
  {
    return values;
  }

  void SettingComboBox::setValues(const String& value)
  {
    values = value;
    Update();
  }

  Combobox* SettingComboBox::getComboBoxW() const
  {
    return static_cast<Combobox*>(WindowManager::getSingleton().getWindow(getName() + ComboboxSuffix));
  }

  void SettingComboBox::initialiseComponents(void)
  {
    Combobox* box = getComboBoxW();

    // internal event wiring
    box->subscribeEvent(Combobox::EventListSelectionAccepted, Event::Subscriber(&SettingComboBox::onSelectionAccepted, this));

    // put components in their initial positions
    performChildWindowLayout();
  }

} CS_PLUGIN_NAMESPACE_END(cegui)
