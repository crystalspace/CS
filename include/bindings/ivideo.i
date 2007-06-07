%module ivideo
%import "bindings/cspace.i"
%import "bindings/csgfx.i"
%{
#include "crystalspace.h"
%}
LANG_FUNCTIONS

%include "bindings/common/ivideo.i"
