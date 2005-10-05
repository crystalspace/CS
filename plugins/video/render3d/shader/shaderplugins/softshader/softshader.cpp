/*
Copyright (C) 2002 by Anders Stenberg
                      Marten Svanfeldt

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "csgeom/vector3.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"
#include "softshader_vp.h"
#include "softshader_fp.h"
#include "softshader.h"

CS_IMPLEMENT_PLUGIN

namespace cspluginSoftshader
{

SCF_IMPLEMENT_FACTORY (csSoftShader)

SCF_IMPLEMENT_IBASE(csSoftShader)
  SCF_IMPLEMENTS_INTERFACE(iShaderProgramPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSoftShader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END


csSoftShader::csSoftShader(iBase* parent) : object_reg(0), enable(false), 
  scanlineRenderer(0)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSoftShader::~csSoftShader()
{
  delete scanlineRenderer;
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

////////////////////////////////////////////////////////////////////
//                      iShaderProgramPlugin
////////////////////////////////////////////////////////////////////
bool csSoftShader::SupportType(const char* type)
{
  Open();
  if (!enable)
    return false;
  if( strcasecmp(type, "vp") == 0)
    return true;
  if( strcasecmp(type, "fp") == 0)
    return true;
  return false;
}

csPtr<iShaderProgram> csSoftShader::CreateProgram(const char* type)
{
  Open();
  if (!enable)
    return 0;
  if( strcasecmp(type, "vp") == 0)
    return csPtr<iShaderProgram>(new csSoftShader_VP(object_reg));
  else if( strcasecmp(type, "fp") == 0)
    return csPtr<iShaderProgram> (new csSoftShader_FP (this));
  else return 0;
}

void csSoftShader::Open()
{
  if(!object_reg)
    return;

  csRef<iGraphics3D> r = CS_QUERY_REGISTRY(object_reg,iGraphics3D);
  csRef<iFactory> f = SCF_QUERY_INTERFACE (r, iFactory);
  if (f != 0 && strcmp ("crystalspace.graphics3d.software", 
      f->QueryClassID ()) == 0)
    enable = true;
  
  if (enable)
  {
    softSRI = scfQueryInterface<iSoftShaderRenderInterface> (r);
    if (!softSRI.IsValid())
    {
      enable = false;
      return;
    }
    
    const csPixelFormat& pfmt = *(r->GetDriver2D()->GetPixelFormat());
    if (pfmt.PixelBytes == 4)
    {
      if ((pfmt.BlueMask == 0x0000ff)
	&& (pfmt.GreenMask == 0x00ff00)
	&& (pfmt.RedMask == 0xff0000))
	scanlineRenderer = (new ScanlineRenderer<
	  Pix_Fix<uint32, 24, 0, 0xff,
			  16, 0, 0xff,
			  8,  0, 0xff,
			  0,  0, 0xff> > (pfmt));
      else if ((pfmt.BlueMask == 0xff0000) 
	&& (pfmt.GreenMask == 0x00ff00)
	&& (pfmt.RedMask == 0x0000ff))
	scanlineRenderer = (new ScanlineRenderer<
	  Pix_Fix<uint32, 24, 0, 0xff,
			  0,  0, 0xff,
			  8,  0, 0xff,
			  16, 0, 0xff> > (pfmt));
      else if (pfmt.RedMask > pfmt.BlueMask)
	scanlineRenderer = (
	  new ScanlineRenderer<Pix_Generic<uint32, 1> > (pfmt));
      else
	scanlineRenderer = (
	  new ScanlineRenderer<Pix_Generic<uint32, 0> > (pfmt));
    }
    else
    {
      if ((pfmt.BlueMask == 0xf800)   // BGR 565
	&& (pfmt.GreenMask == 0x07e0)
	&& (pfmt.RedMask == 0x001f))
	scanlineRenderer = (new ScanlineRenderer<
	  Pix_Fix<uint16, 0,  0, 0,
			  0,  3, 0xf8,
			  3,  0, 0xfc,
			  8,  0, 0xf8> > (pfmt));
      else if ((pfmt.RedMask == 0xf800) // RGB 565
	&& (pfmt.GreenMask == 0x07e0)
	&& (pfmt.BlueMask == 0x001f))
	scanlineRenderer = (new ScanlineRenderer<
	  Pix_Fix<uint16, 0,  0, 0,
			  8,  0, 0xf8,
			  3,  0, 0xfc,
			  0,  3, 0xf8> > (pfmt));
      else if ((pfmt.BlueMask == 0x7c00) // BGR 555
	&& (pfmt.GreenMask == 0x03e0)
	&& (pfmt.RedMask == 0x001f))
	scanlineRenderer = (new ScanlineRenderer<
	  Pix_Fix<uint16, 0,  0, 0,
			  0,  3, 0xf8,
			  2,  0, 0xf8,
			  7,  0, 0xf8> > (pfmt));
      else if ((pfmt.RedMask == 0x7c00) // RGB 555
	&& (pfmt.GreenMask == 0x03e0)
	&& (pfmt.BlueMask == 0x001f))
	scanlineRenderer = (new ScanlineRenderer<
	  Pix_Fix<uint16, 0,  0, 0,
			  7,  0, 0xf8,
			  2,  0, 0xf8,
			  0,  3, 0xf8> > (pfmt));
      else if (pfmt.RedMask > pfmt.BlueMask)
	scanlineRenderer = (
	  new ScanlineRenderer<Pix_Generic<uint16, 1> > (pfmt));
      else
	scanlineRenderer = (
	  new ScanlineRenderer<Pix_Generic<uint16, 0> > (pfmt));
    }
  }
}

////////////////////////////////////////////////////////////////////
//                          iComponent
////////////////////////////////////////////////////////////////////
bool csSoftShader::Initialize(iObjectRegistry* reg)
{
  object_reg = reg;
  return true;
}

} // namespace cspluginSoftshader
