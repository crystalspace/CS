DESCRIPTION.rendloop_loader = Crystal Space render loop loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plugin
PLUGINHELP += \
  $(NEWLINE)@echo $"  make rendloop_loader Make the $(DESCRIPTION.rendloop_loader)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: rendloop_loader rendloop_loaderclean

all plugins: rendloop_loader

rendloop_loader:
	$(MAKE_TARGET) MAKE_DLL=yes

rendloop_loaderclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

endif # ifeq ($(MAKESECTION),defines)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  RENDLOOP_LOADER = $(OUTDLL)/rendloop_loader$(DLL)
  LIB.RENDLOOP_LOADER = $(foreach d,$(DEP.RENDLOOP_LOADER),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(RENDLOOP_LOADER)
else
  RENDLOOP_LOADER = $(OUT)/$(LIBPREFIX)rendloop_loader$(LIB)
  DEP.EXE += $(RENDLOOP_LOADER)
  SCF.STATIC += rendloop_loader
  TO_INSTALL.STATIC_LIBS += $(RENDLOOP_LOADER)
endif

DIR.RENDLOOP_LOADER = plugins/engine/renderloop/loader
OUT.RENDLOOP_LOADER = $(OUT)/$(DIR.RENDLOOP_LOADER)
INF.RENDLOOP_LOADER = $(SRCDIR)/$(DIR.RENDLOOP_LOADER)/rendloop_loader.csplugin
INC.RENDLOOP_LOADER = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.RENDLOOP_LOADER)/*.h \
  plugins/engine/renderloop/common/parserenderstep.h)) 
SRC.RENDLOOP_LOADER = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.RENDLOOP_LOADER)/*.cpp \
  plugins/engine/renderloop/common/parserenderstep.cpp)) 
OBJ.RENDLOOP_LOADER = $(addprefix $(OUT.RENDLOOP_LOADER)/,$(notdir $(SRC.RENDLOOP_LOADER:.cpp=$O)))
DEP.RENDLOOP_LOADER = CSSYS CSUTIL

OUTDIRS += $(OUT.RENDLOOP_LOADER)

MSVC.DSP += RENDLOOP_LOADER
DSP.RENDLOOP_LOADER.NAME = rendloop_loader
DSP.RENDLOOP_LOADER.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: rendloop_loader rendloop_loaderclean rendloop_loadercleandep

rendloop_loader: $(OUTDIRS) $(RENDLOOP_LOADER)
$(RENDLOOP_LOADER): $(OBJ.RENDLOOP_LOADER) $(LIB.RENDLOOP_LOADER)
	$(DO.PLUGIN)

$(OUT.RENDLOOP_LOADER)/%$O: $(SRCDIR)/$(DIR.RENDLOOP_LOADER)/%.cpp
	$(DO.COMPILE.CPP) $(RENDLOOP_LOADER.CFLAGS)

$(OUT.RENDLOOP_LOADER)/%$O: $(SRCDIR)/$(DIR.RENDLOOP_LOADER)/../common/%.cpp
	$(DO.COMPILE.CPP) $(RENDLOOP_LOADER.CFLAGS)

clean: rendloop_loaderclean
rendloop_loaderclean:
	-$(RMDIR) $(RENDLOOP_LOADER) $(OBJ.RENDLOOP_LOADER) $(OUTDLL)/$(notdir $(INF.RENDLOOP_LOADER))

cleandep: rendloop_loadercleandep
rendloop_loadercleandep:
	-$(RM) $(OUT.RENDLOOP_LOADER)/rendloop_loader.dep

ifdef DO_DEPEND
dep: $(OUT.RENDLOOP_LOADER) $(OUT.RENDLOOP_LOADER)/rendloop_loader.dep
$(OUT.RENDLOOP_LOADER)/rendloop_loader.dep: $(SRC.RENDLOOP_LOADER)
	$(DO.DEPEND)
else
-include $(OUT.RENDLOOP_LOADER)/rendloop_loader.dep
endif

endif # ifeq ($(MAKESECTION),targets)
