# This is a subinclude file used to define the rules needed
# to build the rendstep_stencil plug-in.

# Driver description
DESCRIPTION.rendstep_stencil = Crystal Space Stencil Shadow renderstep plugin

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make rendstep_stencil      Make the $(DESCRIPTION.rendstep_stencil)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: rendstep_stencil rendstep_stencilclean
all plugins: rendstep_stencil

rendstep_stencil:
	$(MAKE_TARGET) MAKE_DLL=yes
rendstep_stencilclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  RENDSTEP_STENCIL = $(OUTDLL)/rendstep_stencil$(DLL)
  LIB.RENDSTEP_STENCIL = $(foreach d,$(DEP.RENDSTEP_STENCIL),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(RENDSTEP_STENCIL)
else
  RENDSTEP_STENCIL = $(OUT)/$(LIB_PREFIX)rendstep_stencil$(LIB)
  DEP.EXE += $(RENDSTEP_STENCIL)
  SCF.STATIC += rendstep_stencil
  TO_INSTALL.STATIC_LIBS += $(RENDSTEP_STENCIL)
endif

DIR.RENDSTEP_STENCIL = plugins/engine/renderloop
INF.RENDSTEP_STENCIL = $(SRCDIR)/$(DIR.RENDSTEP_STENCIL)/shadow/stencil/rendstep_stencil.csplugin
INC.RENDSTEP_STENCIL = $(wildcard $(addprefix $(SRCDIR)/, \
   $(DIR.RENDSTEP_STENCIL)/shadow/stencil/*.h \
  plugins/engine/renderloop/common/*.h))
SRC.RENDSTEP_STENCIL = $(wildcard $(addprefix $(SRCDIR)/, \
   $(DIR.RENDSTEP_STENCIL)/shadow/stencil/*.cpp \
  plugins/engine/renderloop/common/*.cpp))
OBJ.RENDSTEP_STENCIL = $(addprefix $(OUT)/,$(notdir $(SRC.RENDSTEP_STENCIL:.cpp=$O)))
DEP.RENDSTEP_STENCIL = CSTOOL CSGEOM CSUTIL CSSYS CSUTIL

TO_INSTALL.CONFIG += $(CFG.RENDSTEP_STENCIL)

MSVC.DSP += RENDSTEP_STENCIL
DSP.RENDSTEP_STENCIL.NAME = rendstep_stencil
DSP.RENDSTEP_STENCIL.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: rendstep_stencil rendstep_stencilclean

rendstep_stencil: $(OUTDIRS) $(RENDSTEP_STENCIL)

$(RENDSTEP_STENCIL): $(OBJ.RENDSTEP_STENCIL) $(LIB.RENDSTEP_STENCIL)
	$(DO.PLUGIN)

$(OUT)/%$O: $(SRCDIR)/$(DIR.RENDSTEP_STENCIL)/shadow/stencil/%.cpp
	$(DO.COMPILE.CPP) $(RENDSTEP_STENCIL.CFLAGS)

$(OUT)/%$O: $(SRCDIR)/$(DIR.RENDSTEP_STENCIL)/common/%.cpp
	$(DO.COMPILE.CPP) $(RENDSTEP_STENCIL.CFLAGS)

clean: rendstep_stencilclean
rendstep_stencilclean:
	$(RM) $(RENDSTEP_STENCIL) $(OBJ.RENDSTEP_STENCIL) \
        $(OUTDLL)/$(notdir $(INF.RENDSTEP_STENCIL))

ifdef DO_DEPEND
dep: $(OUTOS)/rendstep_stencil.dep
$(OUTOS)/rendstep_stencil.dep: $(SRC.RENDSTEP_STENCIL)
	$(DO.DEP)
else
-include $(OUTOS)/rendstep_stencil.dep
endif

endif # ifeq ($(MAKESECTION),targets)
