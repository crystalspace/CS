/*
 * Copyright (C) 1998 by Jorrit Tyberghein
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

// This file is _NOT_ automatically generated.
/*
  Use this file to add your own extension initialization code. Just add a 
  #define INITMORE_<nameofyourext>. This code is then inserted into the generic
  extension initialization and called if it succeeded (that is, if all function 
  addresses required by this extension where successfully retrieved).
 */

#define INITMORE_ARB_multitexture \
    GLint maxtextures;									\
    glGetIntegerv (GL_MAX_TEXTURE_UNITS_ARB, &maxtextures);				\
    if (maxtextures > 1)								\
    {											\
      m_config_options.do_multitexture_level = maxtextures;				\
      Report (CS_REPORTER_SEVERITY_NOTIFY,						\
      "Using multitexture extension with %d texture units", maxtextures);		\
    }											\
    else										\
    {											\
      ARB_multitexture = false;								\
      Report (CS_REPORTER_SEVERITY_NOTIFY, "WARNING: driver supports multitexture"	\
	" extension but only allows one texture unit!");				\
    }											
    
    

