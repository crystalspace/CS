#include "settingslider.h"

CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  const CEGUI::String SettingSlider::EventNamespace("SettingSlider");
  const CEGUI::String SettingSlider::WidgetTypeName("CEGUI/SettingSlider");

  SettingProperties::MinimumValue SettingSlider::d_MinimumValue;

  //Child Widget name suffix constants
  const CEGUI::String SettingSlider::SliderNameSuffix( "__auto_slider__" );
  const CEGUI::String SettingSlider::ValueNameSuffix( "__auto_value__" );


  //CEGUI_DEFINE_WINDOW_FACTORY(SettingSlider)
  SettingSlider::SettingSlider(const CEGUI::String& type, const CEGUI::String& name, iObjectRegistry* obj_reg) 
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
    CEGUI::Slider* slider = getSliderW();
    CEGUI::Window* value = getValueW();
    for (size_t i = 0; i < settings.GetSize(); i++)
    {
      csRef<Setting> setting = settings.Get(i);
      if (setting->IsValid())
      {
        float v; setting->Get(v);
        slider->setCurrentValue(Scale0Max(v, getMinimumValue(), slider->getMaxValue()));
        value->setText(setting->GetAsString());
      }
    }  
  }

  void SettingSlider::onFontChanged(CEGUI::WindowEventArgs& e)
  {
    using namespace CEGUI;
    Window* name = getNameW();
    Slider* slider = getSliderW();
    Window* value = getValueW();
    name->setFont(Window::getFont());
    slider->setFont(Window::getFont());
    value->setFont(Window::getFont());
  }

  bool SettingSlider::onSliderChanged(const CEGUI::EventArgs& e)
  {
    CEGUI::Slider* slider = getSliderW();
    float val = slider->getCurrentValue();
    val = ScaleMinMax(val, getMinimumValue(), slider->getMaxValue());
    CEGUI::Window* value = getValueW();   

    for (size_t i = 0; i < settings.GetSize(); i++)
    {
      csRef<Setting> setting = settings.Get(i);
      if (setting->IsValid())
      {
        setting->SetFromOther(val);
        value->setText(setting->GetAsString());
      }
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

  CEGUI::Slider* SettingSlider::getSliderW() const
  {
    return static_cast<CEGUI::Slider*>(CEGUI::WindowManager::getSingleton().getWindow(getName() + SliderNameSuffix));
  }

  CEGUI::Window* SettingSlider::getValueW() const
  {
    return CEGUI::WindowManager::getSingleton().getWindow(getName() + ValueNameSuffix);
  }

  void SettingSlider::initialiseComponents()
  {
    CEGUI::Slider* slider = getSliderW();

    // Hack, Init max value because else the
    // sliderthumb doesn't get positioned correctly later.
    slider->setMaxValue(100000.0f);

    // internal event wiring
    slider->subscribeEvent(CEGUI::Slider::EventValueChanged, CEGUI::Event::Subscriber(&SettingSlider::onSliderChanged, this));

    // put components in their initial positions
    performChildWindowLayout();
  }

} CS_PLUGIN_NAMESPACE_END(cegui)
