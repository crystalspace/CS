DESCRIPTION.shadermgr = Shader manager

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make shadermgr    Make the $(DESCRIPTION.shadermgr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: shadermgr shadermgrclean
all plugins: shadermgr

shadermgr:
	$(MAKE_TARGET) MAKE_DLL=yes
shadermgrclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  SHADERMGR = $(OUTDLL)/shadermgr$(DLL)
  LIB.SHADERMGR = $(foreach d,$(DEP.SHADERMGR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SHADERMGR)
else
  SHADERMGR = $(OUT)/$(LIB_PREFIX)shadermgr$(LIB)
  DEP.EXE += $(SHADERMGR)
  SCF.STATIC += shadermgr
  TO_INSTALL.STATIC_LIBS += $(SHADERMGR)
endif

DIR.SHADERMGR = plugins/video/render3d/shader/shadermgr
OUT.SHADERMGR = $(OUT)/$(DIR.SHADERMGR)
INF.SHADERMGR = $(SRCDIR)/$(DIR.SHADERMGR)/shadermgr.csplugin
INC.SHADERMGR = $(wildcard $(addprefix $(SRCDIR)/, \
  $(DIR.SHADERMGR)/*.h plugins/video/render3d/shader/common/*.h))
SRC.SHADERMGR = $(wildcard $(addprefix $(SRCDIR)/, \
  $(DIR.SHADERMGR)/*.cpp plugins/video/render3d/shader/common/*.cpp))
OBJ.SHADERMGR = \
  $(addprefix $(OUT.SHADERMGR)/,$(notdir $(SRC.SHADERMGR:.cpp=$O)))
DEP.SHADERMGR = CSTOOL CSGFX CSGEOM CSUTIL

OUTDIRS += $(OUT.SHADERMGR)

MSVC.DSP += SHADERMGR
DSP.SHADERMGR.NAME = shadermgr
DSP.SHADERMGR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: shadermgr shadermgrclean shadermgrcleandep

shadermgr: $(OUTDIRS) $(SHADERMGR)

$(OUT.SHADERMGR)/%$O: $(SRCDIR)/$(DIR.SHADERMGR)/%.cpp
	$(DO.COMPILE.CPP)

$(SHADERMGR): $(OBJ.SHADERMGR) $(LIB.SHADERMGR)
	$(DO.PLUGIN)

clean: shadermgrclean
shadermgrclean:
	-$(RMDIR) $(SHADERMGR) $(OBJ.SHADERMGR) \
	$(OUTDLL)/$(notdir $(INF.SHADERMGR))

cleandep: shadermgrcleandep
shadermgrcleandep:
	-$(RM) $(OUT.SHADERMGR)/shadermgr.dep

ifdef DO_DEPEND
dep: $(OUT.SHADERMGR) $(OUT.SHADERMGR)/shadermgr.dep
$(OUT.SHADERMGR)/shadermgr.dep: $(SRC.SHADERMGR)
	$(DO.DEPEND)
else
-include $(OUT.SHADERMGR)/shadermgr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
