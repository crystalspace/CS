# This is a subinclude file used to define the rules needed
# to build the stencil plug-in.

# Driver description
DESCRIPTION.stencil = Crystal Space Stencil Shadow renderstep plugin

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make stencil      Make the $(DESCRIPTION.stencil)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: stencil stencilclean
all plugins: stencil

stencil:
	$(MAKE_TARGET) MAKE_DLL=yes
stencilclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  STENCIL = $(OUTDLL)/stencil$(DLL)
  LIB.STENCIL = $(foreach d,$(DEP.STENCIL),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(STENCIL)
else
  STENCIL = $(OUT)/$(LIB_PREFIX)stencil$(LIB)
  DEP.EXE += $(STENCIL)
  SCF.STATIC += stencil
  TO_INSTALL.STATIC_LIBS += $(STENCIL)
endif

DIR.STENCIL = plugins/video/render3d/renderloop
INF.STENCIL = $(SRCDIR)/$(DIR.STENCIL)/shadow/stencil/stencil.csplugin
INC.STENCIL = $(wildcard $(addprefix $(SRCDIR)/, \
   $(DIR.STENCIL)/shadow/stencil/*.h \
   $(DIR.STENCIL)/common/parserenderstep.h \
   $(DIR.STENCIL)/common/basesteptype.h \
   $(DIR.STENCIL)/common/basestepfactory.h \
   $(DIR.STENCIL)/common/basesteploader.h))
SRC.STENCIL = $(wildcard $(addprefix $(SRCDIR)/, \
   $(DIR.STENCIL)/shadow/stencil/*.cpp \
   $(DIR.STENCIL)/common/parserenderstep.cpp \
   $(DIR.STENCIL)/common/basesteptype.cpp \
   $(DIR.STENCIL)/common/basestepfactory.cpp \
   $(DIR.STENCIL)/common/basesteploader.cpp ))
OBJ.STENCIL = $(addprefix $(OUT)/,$(notdir $(SRC.STENCIL:.cpp=$O)))
DEP.STENCIL = CSTOOL CSGEOM CSUTIL CSSYS CSUTIL

TO_INSTALL.CONFIG += $(CFG.STENCIL)

MSVC.DSP += STENCIL
DSP.STENCIL.NAME = stencil
DSP.STENCIL.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: stencil stencilclean

stencil: $(OUTDIRS) $(STENCIL)

$(STENCIL): $(OBJ.STENCIL) $(LIB.STENCIL)
	$(DO.PLUGIN)

$(OUT)/%$O: $(SRCDIR)/$(DIR.STENCIL)/shadow/stencil/%.cpp
	$(DO.COMPILE.CPP) $(STENCIL.CFLAGS)

$(OUT)/%$O: $(SRCDIR)/$(DIR.STENCIL)/common/%.cpp
	$(DO.COMPILE.CPP) $(STENCIL.CFLAGS)

clean: stencilclean
stencilclean:
	$(RM) $(STENCIL) $(OBJ.STENCIL)

ifdef DO_DEPEND
dep: $(OUTOS)/stencil.dep
$(OUTOS)/stencil.dep: $(SRC.STENCIL)
	$(DO.DEP)
else
-include $(OUTOS)/stencil.dep
endif

endif # ifeq ($(MAKESECTION),targets)
