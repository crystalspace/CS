DESCRIPTION.lghtng = Lightning mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make lghtng       Make the $(DESCRIPTION.lghtng)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: lghtng lghtngclean
plugins meshes all: lghtng

lghtngclean:
	$(MAKE_CLEAN)
lghtng:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/mesh/lghtng/object

ifeq ($(USE_PLUGINS),yes)
  LGHTNG = $(OUTDLL)/lghtng$(DLL)
  LIB.LGHTNG = $(foreach d,$(DEP.LGHTNG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(LGHTNG)
else
  LGHTNG = $(OUT)/$(LIB_PREFIX)lghtng$(LIB)
  DEP.EXE += $(LGHTNG)
  SCF.STATIC += lghtng
  TO_INSTALL.STATIC_LIBS += $(LGHTNG)
endif

INF.LGHTNG = $(SRCDIR)/plugins/mesh/lghtng/object/lghtng.csplugin
INC.LGHTNG = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/lghtng/object/*.h))
SRC.LGHTNG = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/lghtng/object/*.cpp))
OBJ.LGHTNG = $(addprefix $(OUT)/,$(notdir $(SRC.LGHTNG:.cpp=$O)))
DEP.LGHTNG = CSGEOM CSUTIL CSUTIL

MSVC.DSP += LGHTNG
DSP.LGHTNG.NAME = lghtng
DSP.LGHTNG.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: lghtng lghtngclean
lghtng: $(OUTDIRS) $(LGHTNG)

$(LGHTNG): $(OBJ.LGHTNG) $(LIB.LGHTNG)
	$(DO.PLUGIN)

clean: lghtngclean
lghtngclean:
	-$(RMDIR) $(LGHTNG) $(OBJ.LGHTNG) $(OUTDLL)/$(notdir $(INF.LGHTNG))

ifdef DO_DEPEND
dep: $(OUTOS)/lghtng.dep
$(OUTOS)/lghtng.dep: $(SRC.LGHTNG)
	$(DO.DEP)
else
-include $(OUTOS)/lghtng.dep
endif

endif # ifeq ($(MAKESECTION),targets)
