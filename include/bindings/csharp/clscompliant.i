/*
    Copyright (C) 2007 by Ronie Salgado <roniesalg@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// CLS compliants typemaps

// We wrap unsigned types because there aren't CLS Compliant

#ifdef SWIGCSHARP

%define WRAP_UNSIGNED_TYPE(typec, typeim, wraptype)
%typemap(ctype) typec #typec;
%typemap(imtype) typec #typeim;
%typemap(cstype) typec #wraptype;

%typemap(in) typec
{
  $1 = $input;
}

%typemap(out) typec
{
  $result = $1;
}

%typemap(csin) typec "(typeim) $csinput"
%typemap(csout, excode=SWIGEXCODE) typec
{
  typeim _ret_ = $imcall;
  $excode;
  return (wraptype) _ret_;
}

%enddef

%define WRAP_UNSIGNED_TYPE_EX(typec, wrapc, typeim, wraptype)
%typemap(ctype) typec #wrapc;
%typemap(imtype) typec #typeim;
%typemap(cstype) typec #wraptype;

%typemap(in) typec
{
  $1 = (wrapc)$input;
}

%typemap(out) typec
{
  $result = (typec)$1;
}

%typemap(csin) typec "(typeim) $csinput"
%typemap(csout, excode=SWIGEXCODE) typec
{
  typeim _ret_ = $imcall;
  $excode
  return (wraptype) _ret_;
}

%enddef

%define WRAP_UNSIGNED_TYPE_CONSTREF(typec, wrapc, typeim, wraptype)
%typemap(ctype) const typec&  #wrapc;
%typemap(imtype) const typec& #typeim;
%typemap(cstype) const typec& #wraptype;

%typemap(in) const typec&
%{
  typec _argvalc1 = (typec) $input;
  $1 = &_argvalc1;
%}

%typemap(out) const typec&
%{
  $result = $1;
%}

%typemap(csin) const typec& "(typeim) $csinput";
%typemap(csout, excode=SWIGEXCODE) const typec&
{
  typeim _ret_ = $imcall;
  $excode;
  return (wraptype) _ret_;
}

%enddef

WRAP_UNSIGNED_TYPE(size_t, uint, long)
WRAP_UNSIGNED_TYPE(uint32, uint, long)
WRAP_UNSIGNED_TYPE(utf32_char, uint, long)
WRAP_UNSIGNED_TYPE(uint, uint, long)
WRAP_UNSIGNED_TYPE(unsigned int, uint, long)

// NOTE: we use this because the type length may vary in LP64 system.
//	this truncates the interface ID into a 32 bits unsigned int
WRAP_UNSIGNED_TYPE_EX(scfInterfaceID, unsigned int, uint, long)
WRAP_UNSIGNED_TYPE_EX(csStringID, unsigned int, uint, long)
WRAP_UNSIGNED_TYPE_CONSTREF(csStringID, unsigned int, uint, long)
WRAP_UNSIGNED_TYPE_CONSTREF(csTicks, unsigned int, uint, long)

// We made the ivideo versions of csGetShaderVariableFromStack not CLS
// compliant and define a CLS compliant version on cspace.cs
//%csattributes csGetShaderVariableFromStack(const csShaderVarStack&, csStringID&) "[CLSCompliant(false)]";
//%csattributes csGetShaderVariableFromStack(const iShaderVarStack*, csStringID&) "[CLSCompliant(false)]";
%csattributes RegisterWeakListener(iEventQueue*, iEventHandler*, const csEventID&, csRef<iEventHandler>&) "[CLSCompliant(false)]";
%csattributes csInitializer::SetupEventHandler(iObjectRegistry*, iEventHandler *, const csEventID[]) "[CLSCompliant(false)]";
%csattributes csInitializer::RequestPlugins(iObjectRegistry*, csArray<csPluginRequest> const&) "[CLSCompliant(false)]";

// We ignore this because we don't need and alias which only differs in the
// case
%ignore csImageMemory::ClearKeycolor;
%ignore csImageMemory::SetKeycolor;
%ignore csImageMemory::ApplyKeycolor;

// We define all this constants directly in cspace.cs and in const-nocls for
// the renamed consts
%ignore CS_BOX_CORNER_xy;
%ignore CS_BOX_CORNER_xY;
%ignore CS_BOX_CORNER_Xy;
%ignore CS_BOX_CORNER_XY;

%ignore CS_BOX_EDGE_xy_Xy;
%ignore CS_BOX_EDGE_Xy_xy;
%ignore CS_BOX_EDGE_Xy_XY;
%ignore CS_BOX_EDGE_XY_Xy;
%ignore CS_BOX_EDGE_XY_xY;
%ignore CS_BOX_EDGE_xY_XY;
%ignore CS_BOX_EDGE_xY_xy;
%ignore CS_BOX_EDGE_xy_xY;

%ignore CS_BOX_CORNER_xyz;
%ignore CS_BOX_CORNER_xyZ;
%ignore CS_BOX_CORNER_xYz;
%ignore CS_BOX_CORNER_xYZ;
%ignore CS_BOX_CORNER_Xyz;
%ignore CS_BOX_CORNER_XyZ;
%ignore CS_BOX_CORNER_XYz;
%ignore CS_BOX_CORNER_XYZ;

%ignore CS_BOX_SIDE_x;
%ignore CS_BOX_SIDE_X;
%ignore CS_BOX_SIDE_y;
%ignore CS_BOX_SIDE_Y;
%ignore CS_BOX_SIDE_z;
%ignore CS_BOX_SIDE_Z;

%ignore CS_BOX_EDGE_Xyz_xyz;
%ignore CS_BOX_EDGE_xyz_Xyz;
%ignore CS_BOX_EDGE_xyz_xYz;
%ignore CS_BOX_EDGE_xYz_xyz;
%ignore CS_BOX_EDGE_xYz_XYz;
%ignore CS_BOX_EDGE_XYz_xYz;
%ignore CS_BOX_EDGE_XYz_Xyz;
%ignore CS_BOX_EDGE_Xyz_XYz;
%ignore CS_BOX_EDGE_Xyz_XyZ;
%ignore CS_BOX_EDGE_XyZ_Xyz;
%ignore CS_BOX_EDGE_XyZ_XYZ;
%ignore CS_BOX_EDGE_XYZ_XyZ;
%ignore CS_BOX_EDGE_XYZ_XYz;
%ignore CS_BOX_EDGE_XYz_XYZ;
%ignore CS_BOX_EDGE_XYZ_xYZ;
%ignore CS_BOX_EDGE_xYZ_XYZ;
%ignore CS_BOX_EDGE_xYZ_xYz;
%ignore CS_BOX_EDGE_xYz_xYZ;
%ignore CS_BOX_EDGE_xYZ_xyZ;
%ignore CS_BOX_EDGE_xyZ_xYZ;
%ignore CS_BOX_EDGE_xyZ_xyz;
%ignore CS_BOX_EDGE_xyz_xyZ;
%ignore CS_BOX_EDGE_xyZ_XyZ;
%ignore CS_BOX_EDGE_XyZ_xyZ;

#endif //SWIGCSHARP

