#ifndef SETTINGSLIDER_H
#define SETTINGSLIDER_H

#include "../ceguiimports.h"
#include "settingbase.h"


CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  /*!
  \brief
    Base class for the SettingSlider widget
  */
  class SettingSlider : public SettingBase
  {
  private:
    static SettingProperties::MinimumValue d_MinimumValue;

  private:
    virtual void onFontChanged(CEGUI::WindowEventArgs& e);

  private:
    bool onSliderChanged(const CEGUI::EventArgs& e);

  private:
    virtual void Update();
    float minVal;

  protected:
    virtual bool testClassName_impl(const CEGUI::String& class_name) const
    {
      if (class_name=="SettingSlider") return true;
      return Window::testClassName_impl(class_name);
    }

    CEGUI::Slider* getSliderW() const;
    CEGUI::Window* getValueW() const;

  public:
    /// Namespace for global events
    static const CEGUI::String EventNamespace;
    /// Window factory name
    static const CEGUI::String WidgetTypeName;

    static const CEGUI::String SliderNameSuffix;
    static const CEGUI::String ValueNameSuffix;

  public:
    virtual void initialiseComponents(void);

  public:
    SettingSlider(const CEGUI::String& type, const CEGUI::String& name, iObjectRegistry* obj_reg);
    virtual ~SettingSlider(void);

    float getMinimumValue() const;
    void setMinimumValue(const float& value);
  };

  //CEGUI_DECLARE_WINDOW_FACTORY(SettingSlider);

} CS_PLUGIN_NAMESPACE_END(cegui)

#endif // SETTINGSLIDER_H
