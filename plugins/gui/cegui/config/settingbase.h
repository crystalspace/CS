
#ifndef SETTINGBASE_H
#define SETTINGBASE_H

#include "cssysdef.h"

//#include <CEGUI.h>
#include <CEGUIBase.h>
#include <CEGUIWindow.h>
#include <CEGUIWindowFactory.h>

#include "setting.h"
#include "settingproperties.h"


#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable : 4251)
#endif

CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  /*!
  \brief
    Base class for a Setting widget
  */
  class /*CEGUIEXPORT*/ SettingBase : public CEGUI::Window
  {
  private:
    static SettingProperties::ConfigType d_ConfigType;
    static SettingProperties::ConfigName d_ConfigName;

  private:
    virtual void onTextChanged(CEGUI::WindowEventArgs& e);
    virtual void onFontChanged(CEGUI::WindowEventArgs& e);

  protected:
    virtual void Update() = 0;
    iObjectRegistry* obj_reg;
    Setting setting;
    CEGUI::String configType;
    CEGUI::String configName;
    
  protected:
    CEGUI::Window* getNameW() const;

  public:
    static const CEGUI::String NameNameSuffix;

  public:
    virtual bool isHit(const CEGUI::Point& position) const {return false;}
    //virtual void initialiseComponents(void);

  public:
    SettingBase(const CEGUI::String& type, const CEGUI::String& name, iObjectRegistry* obj_reg);
    virtual ~SettingBase(void);

    const CEGUI::String& getConfigType() const;
    void setConfigType(const CEGUI::String& value);

    const CEGUI::String& getConfigName() const;
    void setConfigName(const CEGUI::String& value);
  };
} CS_PLUGIN_NAMESPACE_END(cegui)

#if defined(_MSC_VER)
#  pragma warning(pop)
#endif

#endif // SETTINGBASE_H
