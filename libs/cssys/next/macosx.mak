#==============================================================================
#
#	Copyright (C)1999-2001 by Eric Sunshine <sunshine@sunshineco.com>
#
# The contents of this file are copyrighted by Eric Sunshine.  This work is
# distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.  You may distribute this file provided that this
# copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
#
#==============================================================================
#------------------------------------------------------------------------------
# macosx.mak
#
#	Build-time configuration options for the MacOS/X platform.
#
#------------------------------------------------------------------------------

NEXT.TARGET=macosx
NEXT.TARGET.DESCRIPTION=$(NEXT.TARGET)$(SPACE)$(SPACE)
NEXT.FLAVOR=MACOSX
NEXT.DESCRIPTION=MacOS/X
NEXT.DESCRIPTION.OS=MacOS/X
NEXT.ARCHS=ppc
NEXT.SOURCE_DIRS=macosx openstep
NEXT.INCLUDE_DIRS=-FAppKit -FFoundation
NEXT.CFLAGS.GENERAL=-Wmost
NEXT.CFLAGS.OPTIMIZE=-O3 -finline-functions
NEXT.CFLAGS.DEBUG=-finline-functions
NEXT.CFLAGS.DLL=
NEXT.LIBS=
NEXT.LFLAGS.GENERAL=-framework AppKit -framework Foundation
NEXT.LFLAGS.EXE=
NEXT.LFLAGS.DLL=-bundle -undefined suppress
NEXT.PLUGINS=video/renderer/opengl

NEXT.FRIEND=yes
include libs/cssys/next/next.mak
NEXT.FRIEND=no

CFLAGS.GL3D=-D'CS_OPENGL_PATH=OpenGL' -FOpenGL
LIBS.OPENGL.SYSTEM=-framework OpenGL
