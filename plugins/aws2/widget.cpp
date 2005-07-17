#include "cssysdef.h"
#include "widget.h"

namespace aws
{

  void widget::SetupAutomation(const csString &object_name)
  {
    // Setup automation for our property bag. Automation objects will be able to get to us through this name.
    // Therefore, if the window's template name is 'toolbar' and the name is '1' then an automation script would
    // simply call :Set@win.toolbar.1.prop(name="Title", value="Toolbar") to set that property.
    prop_bag.SetupAutomation(object_name);
  }

}