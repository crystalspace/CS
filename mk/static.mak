#==============================================================================
#
#    Automatic symbolic reference generation makefile
#    Copyright (C) 2000 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.
#
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.
#
#    You should have received a copy of the GNU Library General Public
#    License along with this library; if not, write to the Free
#    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
#------------------------------------------------------------------------------
# static.mak
#
#	A makefile for synthesizing hard references to symbols within libraries
#	in order to ensure that those libraries are linked into the
#	application.
#
#	When Crystal Space applications are configured to dynamically load
#	plug-in modules at run-time (that is, the makefile variable USE_PLUGINS
#	is "yes" or the configure script was invoked with --enable-plugins,
#	which is the default), the code for plug-in modules is packaged into a
#	dynamic-link library.  On the other hand, when applications are
#	configured to link statically (that is, USE_PLUGINS=no or
#	--disable-plugins was specified), plug-in modules must be linked
#	directly into the application (since they can not be loaded at
#	run-time).
#
#	In this case, the code for plug-in modules is packaged into a static
#	library.  In order to ensure that the linker actually incorporates
#	these libraries into the application, it is necessary to force a hard
#	reference to symbols within the library.  If this reference is not
#	made, then today's so-called "smart" linkers will not actually link the
#	library into the application even though the library is mentioned in
#	the linkage statement.
#
#	This makefile facilitates the process of static linking by synthesizing
#	a C++ source file which contains hard references to symbols within each
#	plug-in module.  The synthesis is accomplished by examining the
#	makefile variable SCF.STATIC, and INF.PROJECT and generating
#	invocations to the SCF macros SCF_REGISTER_STATIC_LIBRARY() and
#	SCF_REGISTER_FACTORY_FUNC() for each module listed in SCF.STATIC.  The
#	synthesized C++ file is then added to the application's dependency
#	list, with the result that it is both compiled and linked into the
#	final application.
#
#	The contents of the SCF.STATIC list can change if the user
#	re-configures the makefile system or modifies the list of desired
#	plug-in modules in CS/mk/user.mak.  This makefile is smart enough to
#	recognize such events and regenerate the synthesized C++ file as
#	necessary.
#
# IMPORTS
#	The makefile variable SCF.STATIC is examined during synthesis of the
#	C++ source file which symbolically references each plug-in module.
#	Each item listed in SCF.STATIC should be the core name of a plug-in
#	module (such as `soft3d', `vfs', etc.)
#
#	The synthesized C++ file must be regenerated when certain external
#	events occur (such as when the user alters the list of desired plug-in
#	modules in CS/mk/user.mak).  The list of resources which must be
#	monitored in order to facilitate automatic regeneration, is contained
#	in the makefile variable DEP.LIBREF.EXTRA (below).
#
# EXPORTS
#	If the project is configured to link plug-in modules into applications
#	statically (that is, if USE_PLUGINS=no or --disable-plugins was
#	specified), then the makefile variable DEP.EXE is augmented with the
#	name of the object file corresponding to the synthesized C++ file.
#
#	The name of the synthesized C++ file is contained in the makefile
#	variable SRC.LIBREF.  The location of the file is mentioned in
#	DIR.LIBREF.
#------------------------------------------------------------------------------

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

DIR.LIBREF = $(OUTDERIVED)
SRC.LIBREF = $(DIR.LIBREF)/cslibref.cpp
OBJ.LIBREF = $(addprefix $(OUT)/,$(notdir $(SRC.LIBREF:.cpp=$O)))

# List of resources to monitor for changes.  If any of these resources change,
# then the file referenced by SRC.LIBREF will be regenerated.
DEP.LIBREF.EXTRA = \
  config.mak \
  $(SRCDIR)/mk/user.mak \
  $(wildcard $(SRCDIR)/mk/local.mak) \
  $(wildcard local.mak) \
  $(SRCDIR)/$(TARGET_MAKEFILE)

# If plug-in modules will be linked into applications statically, then also
# link against the synthesized reference file.
ifneq ($(USE_PLUGINS),yes)
  DEP.EXE += $(OBJ.LIBREF)
endif

# Contents of the reference file.  Makes a call to the SCF macros
# SCF_REGISTER_STATIC_LIBRARY() and SCF_REGISTER_FACTORY_FUNC() for each item
# listed in the makefile variable SCF.STATIC.
define LIBREF.CONTENT
  echo $"// Do not modify.  This file is automatically generated.$"\
    >$(SRC.LIBREF)
  echo $"#include "cssysdef.h"$">>$(SRC.LIBREF)
  echo $"#if defined(CS_STATIC_LINKED)$">>$(SRC.LIBREF)
  echo $"#include "csutil/scf.h"$">>$(SRC.LIBREF)
  echo $"$">>$(SRC.LIBREF)
  $(foreach r,$(SCF.STATIC),$(LIBREF.BODY))
  echo $"#endif // CS_STATIC_LINKED$">>$(SRC.LIBREF)
endef

# Emit the actual SCF_REGISTER_STATIC_LIBRARY() invocation on behalf of
# LIBREF.CONTENT for the given module.  This macro is invoked by LIBREF.CONTENT
# once for each item listed in the SCF.STATIC variable.  For each module, it
# also parse the modules INF.PROJECT meta-data file looking for
# <implementation> nodes, and emits SCF_REGISTER_FACTORY_FUNC() for each
# implementation.  DO _NOT_ remove the blank line from this macro definition.
# Its presence forces a newline in the output in order to ensure that each
# command appears on its own line.  This is required on some platforms (such as
# DOS) which impose a maximum limit on the length of an invoked command.
define LIBREF.BODY
  echo $"static char const $(r)_metainfo[] =$">>$(SRC.LIBREF)
  $(SED) 's:\\:\\\\:g;s:":\\":g;s:\(.*\):"\1":' < $(INF.$(strip $(UPCASE_V))) >>$(SRC.LIBREF)
  echo $";$">>$(SRC.LIBREF)
  echo $"SCF_REGISTER_STATIC_LIBRARY($r,$(r)_metainfo)$">>$(SRC.LIBREF)
  #$(SED) '/<implementation>/!d;s:[ 	]*<implementation>\(..*\)</implementation>:  SCF_REGISTER_FACTORY_FUNC(\1):g' < $(INF.$(strip $(UPCASE_V))) >>$(SRC.LIBREF)
  $(SED) '/<implementation>/!d;s:[ 	]*<implementation>\(..*\)</implementation>:  #ifndef \1_FACTORY_REGISTERED \
  #define \1_FACTORY_REGISTERED \
    SCF_REGISTER_FACTORY_FUNC(\1) \
  #endif:g' < $(INF.$(strip $(UPCASE_V))) >>$(SRC.LIBREF)
  echo $"$">>$(SRC.LIBREF)

endef

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

# Rule to synthesize SRC.LIBREF.
$(SRC.LIBREF): $(DEP.LIBREF.EXTRA) $(DIR.LIBREF)
	-$(RM) $(SRC.LIBREF)
	@$(LIBREF.CONTENT)
	@echo "Generated $(SRC.LIBREF)"

# Rule to compile SRC.LIBREF into OBJ.LIBREF.
$(OBJ.LIBREF): $(SRC.LIBREF)
	$(DO.COMPILE.CPP)

endif # ifeq ($(MAKESECTION),targets)
