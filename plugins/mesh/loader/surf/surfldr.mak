DESCRIPTION.surfldr = Surface mesh object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make surfldr      Make the $(DESCRIPTION.surfldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: surfldr surfldrclean
plugins meshes all: surfldr

surfldrclean:
	$(MAKE_CLEAN)
surfldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/loader/surf

ifeq ($(USE_PLUGINS),yes)
  SURFLDR = $(OUTDLL)surfldr$(DLL)
  LIB.SURFLDR = $(foreach d,$(DEP.SURFLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SURFLDR)
else
  SURFLDR = $(OUT)$(LIB_PREFIX)surfldr$(LIB)
  DEP.EXE += $(SURFLDR)
  SCF.STATIC += surfldr
  TO_INSTALL.STATIC_LIBS += $(SURFLDR)
endif

INC.SURFLDR = $(wildcard plugins/mesh/loader/surf/*.h)
SRC.SURFLDR = $(wildcard plugins/mesh/loader/surf/*.cpp)
OBJ.SURFLDR = $(addprefix $(OUT),$(notdir $(SRC.SURFLDR:.cpp=$O)))
DEP.SURFLDR = CSGEOM CSUTIL CSSYS

MSVC.DSP += SURFLDR
DSP.SURFLDR.NAME = surfldr
DSP.SURFLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: surfldr surfldrclean
surfldr: $(OUTDIRS) $(SURFLDR)

$(SURFLDR): $(OBJ.SURFLDR) $(LIB.SURFLDR)
	$(DO.PLUGIN)

clean: surfldrclean
surfldrclean:
	-$(RM) $(SURFLDR) $(OBJ.SURFLDR)

ifdef DO_DEPEND
dep: $(OUTOS)surfldr.dep
$(OUTOS)surfldr.dep: $(SRC.SURFLDR)
	$(DO.DEP)
else
-include $(OUTOS)surfldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
