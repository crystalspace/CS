DESCRIPTION.md2ie = MD2 Import/Export plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make md2ie        Make the $(DESCRIPTION.md2ie)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: md2ie md2ieclean
plugins meshes all: md2ie

md2ieclean:
	$(MAKE_CLEAN)
md2ie:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/impexp/md2

ifeq ($(USE_PLUGINS),yes)
  MD2IE = $(OUTDLL)md2ie$(DLL)
  LIB.MD2IE = $(foreach d,$(DEP.MD2IE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(MD2IE)
else
  MD2IE = $(OUT)$(LIB_PREFIX)md2ie$(LIB)
  DEP.EXE += $(MD2IE)
  SCF.STATIC += md2ie
  TO_INSTALL.STATIC_LIBS += $(MD2IE)
endif

INC.MD2IE = $(wildcard plugins/mesh/impexp/md2/*.h)
SRC.MD2IE = $(wildcard plugins/mesh/impexp/md2/*.cpp)
OBJ.MD2IE = $(addprefix $(OUT),$(notdir $(SRC.MD2IE:.cpp=$O)))
DEP.MD2IE = CSGEOM CSUTIL CSSYS CSUTIL CSTOOL

MSVC.DSP += MD2IE
DSP.MD2IE.NAME = md2ie
DSP.MD2IE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: md2ie md2ieclean
md2ie: $(OUTDIRS) $(MD2IE)

$(MD2IE): $(OBJ.MD2IE) $(LIB.MD2IE)
	$(DO.PLUGIN)

clean: md2ieclean
md2ieclean:
	-$(RM) $(MD2IE) $(OBJ.MD2IE)

ifdef DO_DEPEND
dep: $(OUTOS)md2ie.dep
$(OUTOS)md2ie.dep: $(SRC.MD2IE)
	$(DO.DEP)
else
-include $(OUTOS)md2ie.dep
endif

endif # ifeq ($(MAKESECTION),targets)
