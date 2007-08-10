%module cstool
%import "bindings/core.i"
%{
#include "crystalspace.h"
%}
LANG_FUNCTIONS

%include "bindings/common/cstool.i"
