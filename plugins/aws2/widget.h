#ifndef __AWS_WIDGET_H__
#define __AWS_WIDGET_H__

#include "frame.h"
#include "registrar.h"
#include "property.h"

#include "csutil/scfstr.h"

namespace aws
{
  class widget : public autom::function::slot
  {    
  protected:
    /// The property bag.  This maintains a list of all of our properties.
    property_bag prop_bag;
    
    
  public:
    
  protected:
     /// Sets up whatever automation is necessary.
     virtual void SetupAutomation(const csString &object_name);
  };
}

#endif
