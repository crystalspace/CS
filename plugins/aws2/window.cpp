#include "cssysdef.h"
#include "window.h"
#include "registrar.h"

namespace aws
{
  window::window()
  {
    csString oname("win.");

    oname+=template_name+".";
    oname+=name;

    SetupAutomation(oname);
  }

  void window::SetupAutomation(const csString &object_name)
  {  
    widget::SetupAutomation(object_name);
  
    prop_bag.CreateProperty("Title", property(true, true));
  }

}