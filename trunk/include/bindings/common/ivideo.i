%ignore iGraphics2D::PerformExtensionV;
%ignore iGraphics3D::PerformExtensionV;
%rename(GetRGBA) iGraphics2D::GetRGB(int, int&, int&, int&, int&);
%include "ivideo/graph2d.h"
%include "ivideo/graph3d.h"
%include "ivideo/cursor.h"

%ignore iNativeWindowManager::AlertV;
%include "ivideo/natwin.h"
%immutable CS::Graphics::CoreRenderMesh::db_mesh_name;
%ignore CS::Graphics::RenderMeshModes::flipCulling;
%include "ivideo/rendermesh.h"

%ignore GetGlyphSize(uint8, int &, int &);
%ignore GetGlyphBitmap(uint8, int &, int &);
%ignore GetGlyphAlphaBitmap(uint8, int &, int &);
%ignore GetDimensions(char const *, int &, int &);
%include "ivideo/fontserv.h"

%include "ivideo/halo.h"
%include "ivideo/shader/shader.h"
%template(csRefShaderStringIDHash) csHash<csRef<iShader>, csStringID > ;
%template(iShaderArray) csArray<csRef<iShader> > ;

%rename(GetKeyColorStatus) iTextureHandle::GetKeyColor() const;
%include "ivideo/texture.h"

%include "ivideo/txtmgr.h"
%include "ivideo/material.h"

// ivideo/graph3d.h
#define _CS_FX_SETALPHA(a) CS_FX_SETALPHA(a)
#undef CS_FX_SETALPHA
uint _CS_FX_SETALPHA (uint);
#define _CS_FX_SETALPHA_INT(a) CS_FX_SETALPHA_INT(a)
#undef CS_FX_SETALPHA_INT
uint _CS_FX_SETALPHA_INT (uint);

/* POST */
#ifndef SWIGIMPORTED
#undef APPLY_FOR_ALL_INTERFACES_POST
#define APPLY_FOR_ALL_INTERFACES_POST IVIDEO_APPLY_FOR_EACH_INTERFACE
#endif

%include "bindings/common/basepost.i"

#ifndef SWIGIMPORTED
cs_apply_all_interfaces
#endif

cs_lang_include(ivideopost.i)

