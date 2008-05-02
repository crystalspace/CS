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

/* POST */
#ifndef SWIGIMPORTED
#undef APPLY_FOR_ALL_INTERFACES_POST
#define APPLY_FOR_ALL_INTERFACES_POST ISNDSYS_APPLY_FOR_EACH_INTERFACE
#endif

%include "bindings/common/basepost.i"

#ifndef SWIGIMPORTED
cs_apply_all_interfaces
#endif

cs_lang_include(isndsyspost.i)

