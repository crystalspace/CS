#ifndef __AWS_WINDOW_H__
#define __AWS_WINDOW_H__

#include "widget.h"

namespace aws
{
  class window : public widget
  {
    /// The name of the window.
    csString name;

    /// The template name of the window (the definition template that it was taken from.)
    csString template_name;
    
  public:
    window();
    virtual ~window() {}
 
  protected:
    /// Sets up whatever automation is necessary.
    virtual void SetupAutomation(const csString &object_name);
  };
}

#endif
