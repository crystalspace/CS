/*
    Copyright (C) 2006 by Michael D. Adams

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
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

#include "tridraw_fill.h"

CS_PLUGIN_NAMESPACE_BEGIN(Soft3D)
{

#define PIX Pix_Generic<uint32>
DEFINE_FILL(PIX, TriDrawMatrixFiller_Fill_Generic_uint32)
#undef PIX

}
CS_PLUGIN_NAMESPACE_END(Soft3D)
