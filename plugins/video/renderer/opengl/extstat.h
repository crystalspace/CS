/*
    Copyright (C) 2001 by Norman Krämer

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

#ifndef _CSEXT_STATIC_H_
#define _CSEXT_STATIC_H_

/**
 * If your opengl implementation does not have a facility to dynamically (at runtime that is)
 * determine the address of GL extensions (or ARBs) then you should tell the opengl renderer what
 * extensions are present (and thus what function pointers it can rely on). Of course, you should be carefull
 * to ship such an executable to your customer :). Note that you can still enable/disable the _usage_ of existant
 * extension in opengl.cfg
 *
 * Simply comment or uncomment the lines below.
 */

//#define CSGL_EXT_STATIC_ASSERTION_ARB_multitexture
//#define CSGL_EXT_STATIC_ASSERTION_ARB_texture_compression
//#define CSGL_EXT_STATIC_ASSERTION_NV_vertex_array_range
//#define CSGL_EXT_STATIC_ASSERTION_ARB_texture_env_combine
//#define CSGL_EXT_STATIC_ASSERTION_SGIS_generate_mipmap

#endif
