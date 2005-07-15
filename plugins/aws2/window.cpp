#include "cssysdef.h"
#include "window.h"
#include "registrar.h"

namespace aws
{

  void window::setup_automation()
  {
    scfString oname("win.");

    oname+=template_name+".";
    oname+=name;

    //autom::Registrar()->assign("SetProperty@win.", std::make_pair(this, (autom::function::slot_mem_ptr)&int_builtin::bits));
  }

}