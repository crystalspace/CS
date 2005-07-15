#include "cssysdef.h"
#include "window.h"
#include "registrar.h"

namespace aws
{
  window::window()
  {
    scfString oname("win.");

    oname+=template_name+".";
    oname+=name;

    SetupAutomation(oname);
  }

  void window::SetupAutomation(const scfString &object_name)
  {  
    widget::SetupAutomation(object_name);


  }

}