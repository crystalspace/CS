DESCRIPTION.gtreeldr = General/Tree mesh object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make gtreeldr     Make the $(DESCRIPTION.gtreeldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: gtreeldr gtreeldrclean
plugins meshes all: gtreeldr

gtreeldrclean:
	$(MAKE_CLEAN)
gtreeldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/genmesh/persist/tree

ifeq ($(USE_PLUGINS),yes)
  GTREELDR = $(OUTDLL)gtreeldr$(DLL)
  LIB.GTREELDR = $(foreach d,$(DEP.GTREELDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(GTREELDR)
else
  GTREELDR = $(OUT)$(LIB_PREFIX)gtreeldr$(LIB)
  DEP.EXE += $(GTREELDR)
  SCF.STATIC += gtreeldr
  TO_INSTALL.STATIC_LIBS += $(GTREELDR)
endif

INC.GTREELDR = $(wildcard plugins/mesh/genmesh/persist/tree/*.h)
SRC.GTREELDR = $(wildcard plugins/mesh/genmesh/persist/tree/*.cpp)
OBJ.GTREELDR = $(addprefix $(OUT),$(notdir $(SRC.GTREELDR:.cpp=$O)))
DEP.GTREELDR = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += GTREELDR
DSP.GTREELDR.NAME = gtreeldr
DSP.GTREELDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: gtreeldr gtreeldrclean
gtreeldr: $(OUTDIRS) $(GTREELDR)

$(GTREELDR): $(OBJ.GTREELDR) $(LIB.GTREELDR)
	$(DO.PLUGIN)

clean: gtreeldrclean
gtreeldrclean:
	-$(RM) $(GTREELDR) $(OBJ.GTREELDR)

ifdef DO_DEPEND
dep: $(OUTOS)gtreeldr.dep
$(OUTOS)gtreeldr.dep: $(SRC.GTREELDR)
	$(DO.DEP)
else
-include $(OUTOS)gtreeldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
