DESCRIPTION.rendstep_std = Crystal Space standard render loop steps

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plugin
PLUGINHELP += \
  $(NEWLINE)@echo $"  make rendstep_std       Make the $(DESCRIPTION.rendstep_std)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: rendstep_std rendstep_stdclean

all plugins: rendstep_std

rendstep_std:
	$(MAKE_TARGET) MAKE_DLL=yes

rendstep_stdclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

endif # ifeq ($(MAKESECTION),defines)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  RENDSTEP_STD = $(OUTDLL)/rendstep_std$(DLL)
  LIB.RENDSTEP_STD = $(foreach d,$(DEP.RENDSTEP_STD),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(RENDSTEP_STD)
else
  RENDSTEP_STD = $(OUT)/$(LIBPREFIX)rendstep_std$(LIB)
  DEP.EXE += $(RENDSTEP_STD)
  SCF.STATIC += rendstep_std
  TO_INSTALL.STATIC_LIBS += $(RENDSTEP_STD)
endif

DIR.RENDSTEP_STD = plugins/engine/renderloop/stdsteps
OUT.RENDSTEP_STD = $(OUT)/$(DIR.RENDSTEP_STD)
INF.RENDSTEP_STD = $(SRCDIR)/$(DIR.RENDSTEP_STD)/rendstep_std.csplugin
INC.RENDSTEP_STD = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.RENDSTEP_STD)/*.h \
  plugins/engine/renderloop/common/*.h))
SRC.RENDSTEP_STD = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.RENDSTEP_STD)/*.cpp \
  plugins/engine/renderloop/common/*.cpp))
OBJ.RENDSTEP_STD = $(addprefix $(OUT.RENDSTEP_STD)/,$(notdir $(SRC.RENDSTEP_STD:.cpp=$O)))
DEP.RENDSTEP_STD = CSGFX CSTOOL CSUTIL CSGEOM

OUTDIRS += $(OUT.RENDSTEP_STD)

MSVC.DSP += RENDSTEP_STD
DSP.RENDSTEP_STD.NAME = rendstep_std
DSP.RENDSTEP_STD.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: rendstep_std rendstep_stdclean rendstep_stdcleandep

rendstep_std: $(OUTDIRS) $(RENDSTEP_STD)
$(RENDSTEP_STD): $(OBJ.RENDSTEP_STD) $(LIB.RENDSTEP_STD)
	$(DO.PLUGIN)

$(OUT.RENDSTEP_STD)/%$O: $(SRCDIR)/$(DIR.RENDSTEP_STD)/%.cpp
	$(DO.COMPILE.CPP) $(RENDSTEP_STD.CFLAGS)

$(OUT.RENDSTEP_STD)/%$O: $(SRCDIR)/$(DIR.RENDSTEP_STD)/../common/%.cpp
	$(DO.COMPILE.CPP) $(RENDSTEP_STD.CFLAGS)

clean: rendstep_stdclean
rendstep_stdclean:
	-$(RMDIR) $(RENDSTEP_STD) $(OBJ.RENDSTEP_STD) $(OUTDLL)/$(notdir $(INF.RENDSTEP_STD))

cleandep: rendstep_stdcleandep
rendstep_stdcleandep:
	-$(RM) $(OUT.RENDSTEP_STD)/rendstep_std.dep

ifdef DO_DEPEND
dep: $(OUT.RENDSTEP_STD) $(OUT.RENDSTEP_STD)/rendstep_std.dep
$(OUT.RENDSTEP_STD)/rendstep_std.dep: $(SRC.RENDSTEP_STD)
	$(DO.DEPEND)
else
-include $(OUT.RENDSTEP_STD)/rendstep_std.dep
endif

endif # ifeq ($(MAKESECTION),targets)
