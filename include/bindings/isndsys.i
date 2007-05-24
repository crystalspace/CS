
#ifndef SWIGIMPORTED
  %module isndsys
  %include "bindings/allinterfaces.i"
  #define APPLY_FOR_ALL_INTERFACES_PRE APPLY_FOR_ALL_INTERFACES
  #define APPLY_FOR_ALL_INTERFACES_POST ISNDSYS_APPLY_FOR_EACH_INTERFACE

  %include "bindings/basepre.i"
#endif


%include "isndsys/ss_data.h"
%include "isndsys/ss_filter.h"
%include "isndsys/ss_listener.h"
%include "isndsys/ss_loader.h"
%include "isndsys/ss_manager.h"
%include "isndsys/ss_source.h"
%include "isndsys/ss_structs.h"
%include "isndsys/ss_stream.h"
%include "isndsys/ss_renderer.h"
%include "isndsys/ss_driver.h"


#ifndef SWIGIMPORTED
  %include "bindings/basepost.i"
#endif

