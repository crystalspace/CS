#include "settingcombobox.h"

CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  const CEGUI::String SettingComboBox::EventNamespace("SettingComboBox");
  const CEGUI::String SettingComboBox::WidgetTypeName("CEGUI/SettingComboBox");

  SettingProperties::Values SettingComboBox::d_Values;

  //Child Widget name suffix constants
  const CEGUI::String SettingComboBox::ComboboxSuffix( "__auto_combobox__" );


  SettingComboBox::SettingComboBox(const CEGUI::String& type, const CEGUI::String& name, iObjectRegistry* obj_reg) : SettingBase(type, name, obj_reg)
  {
    addProperty(&d_Values);
  }

  SettingComboBox::~SettingComboBox(void)
  {
  }

  void SettingComboBox::Update()
  {
    CEGUI::Combobox* box = getComboBoxW();

    if (settings.GetSize())
    {
      csRef<Setting> setting = settings.Get(0);
      if (setting->IsValid())
      {
        if (setting->IsDefault())
          box->getEditbox()->setText("Default");
        else
          box->getEditbox()->setText(GetKey(setting->GetAsString().c_str()));
      }
    }

    if (box->getItemCount() == 0)
    {
      Values::const_iterator it = values.begin();
      for (size_t i = 0; it != values.end(); it++, i++)
      {
        CEGUI::ListboxTextItem* item = new CEGUI::ListboxTextItem(it->first.c_str(), (CEGUI::uint)i);
        item->setTextColours(CEGUI::colour(0.f, 0.f, 0.f)); 
        box->getDropList()->addItem(item);
      }
    }
  }

  void SettingComboBox::onFontChanged(CEGUI::WindowEventArgs& e)
  {
    CEGUI::Window* name = getNameW();
    CEGUI::Combobox* box = getComboBoxW();
    name->setFont(CEGUI::Window::getFont());
    box->setFont(CEGUI::Window::getFont());
  }

  void SettingComboBox::onSized(CEGUI::WindowEventArgs& e)
  {
    CEGUI::Window::onSized(e);
    CEGUI::Combobox* box = getComboBoxW();
    box->getEditbox()->setReadOnly(true);

    // Calculate height.
    float h = box->getEditbox()->getPixelSize().d_height+5.0f;
    for (size_t i =0; i < box->getItemCount(); i++)
    {
      h += box->getListboxItemFromIndex(i)->getPixelSize().d_height;
    }

    box->setHeight(CEGUI::UDim(0.0f, h));
    box->setClippedByParent(false);
    box->getDropList()->getVertScrollbar()->setEnabled(false);
    box->getDropList()->getVertScrollbar()->setAlpha(0.0f);
  }

  std::string SettingComboBox::GetValue(const std::string& key)
  {
    Values::const_iterator it = values.find(key);
    if (it != values.end() && it->second.size()) return it->second[0];

    return "ERROR 1";
  }

  std::string SettingComboBox::GetKey(const std::string& value)
  {
    Values::const_iterator it = values.begin();
    for (; it != values.end(); it++)
    {
      std::vector<std::string>::const_iterator itt = it->second.begin();
      for (; itt != it->second.end(); itt++)
      {
        if (*itt == value) return it->first;
      }
    }

    return "ERROR 2";
  }

  bool SettingComboBox::onSelectionAccepted(const CEGUI::EventArgs& e)
  {
    CEGUI::Combobox* box = getComboBoxW();
    CEGUI::String key = box->getSelectedItem()->getText();
    const std::vector<std::string>& vals = values[key.c_str()];

    if (vals.size() != settings.GetSize())
    {
      printf("E: onSelectionAccepted failed!\n");
      return true;
    }

    std::vector<std::string>::const_iterator it = vals.begin();
    for (size_t i = 0; it != vals.end(); it++, i++)
    {
      settings.Get(i)->SetFromString(*it);
    }

    return true;
  }

  const CEGUI::String& SettingComboBox::getValues() const
  {
    return valuesString;
  }

  void SettingComboBox::setValues(const CEGUI::String& value)
  {
    valuesString = value;
    values.clear();

    std::vector<std::string> pairs;
    Tokenize(value.c_str(), pairs, ";");

    std::vector<std::string>::const_iterator it = pairs.begin();
    for (; it != pairs.end(); it++)
    {
      std::vector<std::string> pair;
      Tokenize(*it, pair, ":");

      if (pair.size() != 2)
      {
        printf("E: setValues failed.\n");
        continue;
      }

      std::vector<std::string> vals;
      Tokenize(pair[1], vals, ",");
      values[pair[0]] = vals;
    }

    Update();
  }

  CEGUI::Combobox* SettingComboBox::getComboBoxW() const
  {
    return static_cast<CEGUI::Combobox*>(CEGUI::WindowManager::getSingleton().getWindow(getName() + ComboboxSuffix));
  }

  void SettingComboBox::initialiseComponents()
  {
    CEGUI::Combobox* box = getComboBoxW();

    // internal event wiring
    box->subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&SettingComboBox::onSelectionAccepted, this));

    // put components in their initial positions
    performChildWindowLayout();
  }

} CS_PLUGIN_NAMESPACE_END(cegui)
