%template(iShaderVarStringSetBase) iStringSetBase<CS::StringSetTag::ShaderVar>;

%include "igraphic/image.h"
%immutable csImageIOFileFormatDescription::mime;
%immutable csImageIOFileFormatDescription::subtype;
%template (csImageIOFileFormatDescriptions) csArray<csImageIOFileFormatDescription const*>;
%include "igraphic/imageio.h"
%include "igraphic/animimg.h"
%include "itexture/iproctex.h"

%rename(asRGBcolor) csRGBpixel::operator csRGBcolor;
%include "csgfx/rgbpixel.h"
%ignore ShaderVarName;
%include "csgfx/shadervar.h"
ARRAY_CHANGE_ALL_TEMPLATE_PTR(csShaderVariable)

%template(csImageBaseBase) scfImplementation1<csImageBase, iImage>;
%include "csgfx/imagebase.h"
%template(csImageMemoryBase) scfImplementationExt0<csImageMemory, csImageBase>;
%include "csgfx/imagememory.h"

%include "csgfx/imagemanipulate.h"

/* POST */
#ifndef SWIGIMPORTED
#undef APPLY_FOR_ALL_INTERFACES_POST
#define APPLY_FOR_ALL_INTERFACES_POST CSGFX_APPLY_FOR_EACH_INTERFACE
#endif

%include "bindings/common/basepost.i"

#ifndef SWIGIMPORTED
cs_apply_all_interfaces
#endif

cs_lang_include(csgfxpost.i)


