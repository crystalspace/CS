
#ifndef SETTINGCOMBOBOX_H
#define SETTINGCOMBOBOX_H

#include "settingbase.h"

#include <map>

CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  /*!
  \brief
    Base class for the SettingComboBox widget
  */
  class SettingComboBox : public SettingBase
  {
  private:
    static SettingProperties::Values d_Values;

  private:
    virtual void onFontChanged(CEGUI::WindowEventArgs& e);
    virtual void onSized(CEGUI::WindowEventArgs& e);

  private:
    bool onSelectionAccepted(const CEGUI::EventArgs& e);

  private:
    virtual void Update();
    CEGUI::String values;

    CEGUI::String GetValue(const CEGUI::String& key);
    CEGUI::String GetKey(const CEGUI::String& value);
    std::vector<CEGUI::String> GetKeys();

  protected:
    virtual bool testClassName_impl(const CEGUI::String& class_name) const
    {
      if (class_name=="SettingComboBox") return true;
      return CEGUI::Window::testClassName_impl(class_name);
    }

    CEGUI::Combobox* getComboBoxW() const;

  public:
    /// Namespace for global events
    static const CEGUI::String EventNamespace;
    /// Window factory name
    static const CEGUI::String WidgetTypeName;

    static const CEGUI::String ComboboxSuffix;

  public:
    virtual void initialiseComponents(void);

  public:
    SettingComboBox(const CEGUI::String& type, const CEGUI::String& name, iObjectRegistry* obj_reg);
    virtual ~SettingComboBox(void);

    CEGUI::String getValues() const;
    void setValues(const CEGUI::String& value);
  };

} CS_PLUGIN_NAMESPACE_END(cegui)


#endif // SETTINGCOMBOBOX_H
