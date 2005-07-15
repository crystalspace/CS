#ifndef __AWS_WINDOW_H__
#define __AWS_WINDOW_H__

#include "frame.h"
#include "registrar.h"
#include "csutil/scfstr.h"

namespace aws
{
  class window : public autom::function::slot
  {
    /// The name of the window.
    scfString name;

    /// The template name of the window (the definition template that it was taken from.)
    scfString template_name;
    
  public:
    
  private:
     void setup_automation();    
  };
}

#endif
