DESCRIPTION.spirldr = Spiral mesh object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make spirldr      Make the $(DESCRIPTION.spirldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: spirldr spirldrclean
plugins meshes all: spirldr

spirldrclean:
	$(MAKE_CLEAN)
spirldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/loader/spiral

ifeq ($(USE_PLUGINS),yes)
  SPIRLDR = $(OUTDLL)spirldr$(DLL)
  LIB.SPIRLDR = $(foreach d,$(DEP.SPIRLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SPIRLDR)
else
  SPIRLDR = $(OUT)$(LIB_PREFIX)spirldr$(LIB)
  DEP.EXE += $(SPIRLDR)
  SCF.STATIC += spirldr
  TO_INSTALL.STATIC_LIBS += $(SPIRLDR)
endif

INC.SPIRLDR = $(wildcard plugins/mesh/loader/spiral/*.h)
SRC.SPIRLDR = $(wildcard plugins/mesh/loader/spiral/*.cpp)
OBJ.SPIRLDR = $(addprefix $(OUT),$(notdir $(SRC.SPIRLDR:.cpp=$O)))
DEP.SPIRLDR = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += SPIRLDR
DSP.SPIRLDR.NAME = spirldr
DSP.SPIRLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: spirldr spirldrclean
spirldr: $(OUTDIRS) $(SPIRLDR)

$(SPIRLDR): $(OBJ.SPIRLDR) $(LIB.SPIRLDR)
	$(DO.PLUGIN)

clean: spirldrclean
spirldrclean:
	-$(RM) $(SPIRLDR) $(OBJ.SPIRLDR)

ifdef DO_DEPEND
dep: $(OUTOS)spirldr.dep
$(OUTOS)spirldr.dep: $(SRC.SPIRLDR)
	$(DO.DEP)
else
-include $(OUTOS)spirldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
