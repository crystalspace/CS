#ifndef SETTINGBASE_H
#define SETTINGBASE_H

#include "cssysdef.h"
#include "csutil/refarr.h"
#include "../ceguiimports.h"

#include <string>
#include <vector>

#include "setting.h"
#include "settingproperties.h"


CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  /*!
  \brief
    Base class for a Setting widget
  */
  class SettingBase : public CEGUI::Window
  {
  private:
    static SettingProperties::ConfigTypes d_ConfigTypes;
    static SettingProperties::ConfigNames d_ConfigNames;

  private:
    virtual void onTextChanged(CEGUI::WindowEventArgs& e);
    virtual void onFontChanged(CEGUI::WindowEventArgs& e);

  protected:
    virtual void Update() = 0;
    iObjectRegistry* obj_reg;
    csRefArray<Setting> settings;

    void Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters = " ");
    
  protected:
    CEGUI::Window* getNameW() const;

  public:
    static const CEGUI::String NameNameSuffix;

  public:
    virtual bool isHit(const CEGUI::Point& position) const {return false;}

  public:
    SettingBase(const CEGUI::String& type, const CEGUI::String& name, iObjectRegistry* obj_reg);
    virtual ~SettingBase(void);

    CEGUI::String getConfigTypes() const;
    void setConfigTypes(const CEGUI::String& value);

    CEGUI::String getConfigNames() const;
    void setConfigNames(const CEGUI::String& value);
  };
} CS_PLUGIN_NAMESPACE_END(cegui)

#endif // SETTINGBASE_H
