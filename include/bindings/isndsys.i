%module isndsys

%import "bindings/allinterfaces.i"

%include "bindings/basepre.i"

#undef APPLY_FOR_ALL_INTERFACES
#define APPLY_FOR_ALL_INTERFACES ISNDSYS_APPLY_FOR_EACH_INTERFACE

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

%include "bindings/basepost.i"

