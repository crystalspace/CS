DESCRIPTION.rlloader = Crystal Space render loop loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plugin
PLUGINHELP += \
  $(NEWLINE)@echo $"  make rlloader     Make the $(DESCRIPTION.rlloader)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: rlloader rlloaderclean

all plugins: rlloader

rlloader:
	$(MAKE_TARGET) MAKE_DLL=yes

rlloaderclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

endif # ifeq ($(MAKESECTION),defines)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  RLLOADER = $(OUTDLL)/rlloader$(DLL)
  LIB.RLLOADER = $(foreach d,$(DEP.RLLOADER),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(RLLOADER)
else
  RLLOADER = $(OUT)/$(LIBPREFIX)rlloader$(LIB)
  DEP.EXE += $(RLLOADER)
  SCF.STATIC += rlloader
  TO_INSTALL.STATIC_LIBS += $(RLLOADER)
endif

DIR.RLLOADER = plugins/video/render3d/renderloop/loader
OUT.RLLOADER = $(OUT)/$(DIR.RLLOADER)
INF.RLLOADER = $(SRCDIR)/$(DIR.RLLOADER)/rlloader.csplugin
INC.RLLOADER = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.RLLOADER)/*.h)) \
  $(wildcard $(addprefix $(SRCDIR)/,$(DIR.RLLOADER)/../common/parserenderstep.h)) 
SRC.RLLOADER = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.RLLOADER)/*.cpp)) \
  $(wildcard $(addprefix $(SRCDIR)/,$(DIR.RLLOADER)/../common/parserenderstep.cpp)) 
OBJ.RLLOADER = $(addprefix $(OUT.RLLOADER)/,$(notdir $(SRC.RLLOADER:.cpp=$O)))
DEP.RLLOADER = CSSYS CSUTIL

OUTDIRS += $(OUT.RLLOADER)

MSVC.DSP += RLLOADER
DSP.RLLOADER.NAME = rlloader
DSP.RLLOADER.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: rlloader rlloaderclean rlloadercleandep

rlloader: $(OUTDIRS) $(RLLOADER)
$(RLLOADER): $(OBJ.RLLOADER) $(LIB.RLLOADER)
	$(DO.PLUGIN)

$(OUT.RLLOADER)/%$O: $(SRCDIR)/$(DIR.RLLOADER)/%.cpp
	$(DO.COMPILE.CPP) $(RLLOADER.CFLAGS)

$(OUT.RLLOADER)/%$O: $(SRCDIR)/$(DIR.RLLOADER)/../common/%.cpp
	$(DO.COMPILE.CPP) $(RLLOADER.CFLAGS)

clean: rlloaderclean
rlloaderclean:
	-$(RMDIR) $(RLLOADER) $(OBJ.RLLOADER) $(OUTDLL)/$(notdir $(INF.RLLOADER))

cleandep: rlloadercleandep
rlloadercleandep:
	-$(RM) $(OUT.RLLOADER)/rlloader.dep

ifdef DO_DEPEND
dep: $(OUT.RLLOADER) $(OUT.RLLOADER)/rlloader.dep
$(OUT.RLLOADER)/rlloader.dep: $(SRC.RLLOADER)
	$(DO.DEPEND)
else
-include $(OUT.RLLOADER)/rlloader.dep
endif

endif # ifeq ($(MAKESECTION),targets)
