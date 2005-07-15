#ifndef __AWS_WINDOW_H__
#define __AWS_WINDOW_H__

#include "widget.h"

namespace aws
{
  class window : public widget
  {
    /// The name of the window.
    scfString name;

    /// The template name of the window (the definition template that it was taken from.)
    scfString template_name;

    /// The property bag.  This maintains a list of all of our properties.
    property_bag prop_bag;

    /////////////// Properties /////////////
    autom::string title;

    
  public:
    window();
    virtual ~window() {}
 
  protected:
    /// Sets up whatever automation is necessary.
    virtual void SetupAutomation(const scfString &object_name);
  };
}

#endif
