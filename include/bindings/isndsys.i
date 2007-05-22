%module isndsys
%import "bindings/cspace.i"
%{
#include "crystalspace.h"
%}
LANG_FUNCTIONS

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

#undef INTERFACE_APPLY
#define INTERFACE_APPLY(x) INTERFACE_POST(x)
ISNDSYS_APPLY_FOR_EACH_INTERFACE

