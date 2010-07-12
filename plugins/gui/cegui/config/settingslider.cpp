
#include "settingslider.h"

#include <elements/CEGUICombobox.h>
#include <elements/CEGUIEditbox.h>
#include <elements/CEGUISlider.h>
#include <CEGUIWindowManager.h>

CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  using namespace CEGUI;

  const String SettingSlider::EventNamespace("SettingSlider");
  const String SettingSlider::WidgetTypeName("CEGUI/SettingSlider");

  SettingProperties::MinimumValue SettingSlider::d_MinimumValue;

  //Child Widget name suffix constants
  const String SettingSlider::SliderNameSuffix( "__auto_slider__" );
  const String SettingSlider::ValueNameSuffix( "__auto_value__" );


  //CEGUI_DEFINE_WINDOW_FACTORY(SettingSlider)
  SettingSlider::SettingSlider(const String& type, const String& name, iObjectRegistry* obj_reg) 
    : SettingBase(type, name, obj_reg), minVal(0.0f)
  {
    addProperty(&d_MinimumValue);
  }

  SettingSlider::~SettingSlider(void)
  {
  }

  float Scale0Max(float value, float min, float max)
  {
    return ((value - min) * max) / (max - min);
  }

  float ScaleMinMax(float value, float min, float max)
  {
    return ((value / max) * (max - min)) + min;
  }

  void SettingSlider::Update()
  {
    if (!setting)
    {
      setting.SetValueType(configType.c_str());
      setting.SetValueName(configName.c_str());
    }
    else
    {
      Slider* slider = getSliderW();
      float v; setting.Get(v);
      slider->setCurrentValue(Scale0Max(v, getMinimumValue(), slider->getMaxValue()));
      Window* value = getValueW();
      value->setText(setting.GetAsString());
    }
  }

  void SettingSlider::onFontChanged(WindowEventArgs& e)
  {
    Window* name = getNameW();
    Slider* slider = getSliderW();
    Window* value = getValueW();
    name->setFont(Window::getFont());
    slider->setFont(Window::getFont());
    value->setFont(Window::getFont());
  }

  bool SettingSlider::onSliderChanged(const EventArgs& e)
  {
    Slider* slider = getSliderW();
    float val = slider->getCurrentValue();
    val = ScaleMinMax(val, getMinimumValue(), slider->getMaxValue());
    if (setting)
    {
      setting.Set(val);
      Window* value = getValueW();
      value->setText(setting.GetAsString());
    }

    return true;
  }

  float SettingSlider::getMinimumValue() const
  {
    return minVal;
  }

  void SettingSlider::setMinimumValue(const float& value)
  {
    minVal = value;
    Update();
  }

  Slider* SettingSlider::getSliderW() const
  {
    return static_cast<Slider*>(WindowManager::getSingleton().getWindow(getName() + SliderNameSuffix));
  }

  Window* SettingSlider::getValueW() const
  {
    return WindowManager::getSingleton().getWindow(getName() + ValueNameSuffix);
  }

  void SettingSlider::initialiseComponents(void)
  {
    Slider* slider = getSliderW();

    // Hack, Init max value because else the
    // sliderthumb doesn't get positioned correctly later.
    slider->setMaxValue(100000.0f);

    // internal event wiring
    slider->subscribeEvent(Slider::EventValueChanged, Event::Subscriber(&SettingSlider::onSliderChanged, this));

    // put components in their initial positions
    performChildWindowLayout();
  }

} CS_PLUGIN_NAMESPACE_END(cegui)
