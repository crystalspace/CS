%module csgeom
%import "bindings/cspace.i"
%{
#include "crystalspace.h"
%}
LANG_FUNCTIONS

%include "bindings/common/csgeom.i"
