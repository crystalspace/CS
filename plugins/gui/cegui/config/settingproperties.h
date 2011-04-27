#ifndef SETTING_PROPERTIES_H
#define SETTING_PROPERTIES_H

#include "cssysdef.h"
#include "../ceguiimports.h"


// Start of CEGUI namespace section
CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
namespace SettingProperties
{

  class ConfigTypes : public CEGUI::Property
  {
  public:
    ConfigTypes() : CEGUI::Property("ConfigTypes",
                            "Value is a text string.",
                            "")
    {}

    CEGUI::String get(const CEGUI::PropertyReceiver* receiver) const;
    void set(CEGUI::PropertyReceiver* receiver, const CEGUI::String& value);
  };

  class ConfigNames : public CEGUI::Property
  {
  public:
    ConfigNames() : CEGUI::Property("ConfigNames",
                            "Value is a text string.",
                            "")
    {}

    CEGUI::String get(const CEGUI::PropertyReceiver* receiver) const;
    void set(CEGUI::PropertyReceiver* receiver, const CEGUI::String& value);
  };

  class MinimumValue : public CEGUI::Property
  {
  public:
    MinimumValue() : CEGUI::Property("MinimumValue",
                            "Value is a float.",
                            "0.0")
    {}

    CEGUI::String get(const CEGUI::PropertyReceiver* receiver) const;
    void set(CEGUI::PropertyReceiver* receiver, const CEGUI::String& value);
  };

  class Values : public CEGUI::Property
  {
  public:
    Values() : CEGUI::Property("Values",
                        "Value is a ; seperated list of Name:value pairs.",
                        "")
    {}

    CEGUI::String get(const CEGUI::PropertyReceiver* receiver) const;
    void set(CEGUI::PropertyReceiver* receiver, const CEGUI::String& value);
  };


} // end SettingProperties
} CS_PLUGIN_NAMESPACE_END(cegui)


#endif // SETTING_PROPERTIES_H
