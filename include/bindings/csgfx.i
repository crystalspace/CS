
#ifndef SWIGIMPORTED
  %module csgfx
  %include "bindings/allinterfaces.i"
  #define APPLY_FOR_ALL_INTERFACES_PRE APPLY_FOR_ALL_INTERFACES
  #define APPLY_FOR_ALL_INTERFACES_POST

  %include "bindings/basepre.i"
#endif

%import "bindings/cspace.i" /* needed for iImage */


%rename(asRGBcolor) csRGBpixel::operator csRGBcolor;
%include "csgfx/rgbpixel.h"
%ignore ShaderVarName;
%include "csgfx/shadervar.h"
%template(csShaderVariableArrayReadOnly) iArrayReadOnly<csShaderVariable * >;
%template(csShaderVariableArrayChangeElements)
iArrayChangeElements<csShaderVariable * >;
%template(csShaderVariableArray) iArrayChangeAll<csShaderVariable * >;

%template(csImageBaseBase) scfImplementation1<csImageBase, iImage>;
%include "csgfx/imagebase.h"
%template(csImageMemoryBase) scfImplementationExt0<csImageMemory, csImageBase>;
%include "csgfx/imagememory.h"

#if defined(SWIGPYTHON)
%include "bindings/python/pyshadervar.i"
#endif


#ifndef SWIGIMPORTED
  %include "bindings/basepost.i"
#endif

