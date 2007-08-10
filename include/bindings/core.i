%module core
%include "bindings/common/core.i"

#undef APPLY_FOR_ALL_INTERFACES_POST
#define APPLY_FOR_ALL_INTERFACES_POST CORE_APPLY_FOR_EACH_INTERFACE
%include "bindings/common/basepost.i"
cs_lang_include(corepost.i)

