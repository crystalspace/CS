DESCRIPTION.fountldr = Fountain mesh object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make fountldr     Make the $(DESCRIPTION.fountldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: fountldr fountldrclean
plugins all: fountldr

fountldrclean:
	$(MAKE_CLEAN)
fountldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/meshldr/fountain

ifeq ($(USE_PLUGINS),yes)
  FOUNTLDR = $(OUTDLL)fountldr$(DLL)
  LIB.FOUNTLDR = $(foreach d,$(DEP.FOUNTLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(FOUNTLDR)
else
  FOUNTLDR = $(OUT)$(LIB_PREFIX)fountldr$(LIB)
  DEP.EXE += $(FOUNTLDR)
  SCF.STATIC += fountldr
  TO_INSTALL.STATIC_LIBS += $(FOUNTLDR)
endif

INC.FOUNTLDR = $(wildcard plugins/meshldr/fountain/*.h)
SRC.FOUNTLDR = $(wildcard plugins/meshldr/fountain/*.cpp)
OBJ.FOUNTLDR = $(addprefix $(OUT),$(notdir $(SRC.FOUNTLDR:.cpp=$O)))
DEP.FOUNTLDR = CSGEOM CSUTIL CSSYS

MSVC.DSP += FOUNTLDR
DSP.FOUNTLDR.NAME = fountldr
DSP.FOUNTLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: fountldr fountldrclean
fountldr: $(OUTDIRS) $(FOUNTLDR)

$(FOUNTLDR): $(OBJ.FOUNTLDR) $(LIB.FOUNTLDR)
	$(DO.PLUGIN)

clean: fountldrclean
fountldrclean:
	-$(RM) $(FOUNTLDR) $(OBJ.FOUNTLDR) $(OUTOS)fountldr.dep

ifdef DO_DEPEND
dep: $(OUTOS)fountldr.dep
$(OUTOS)fountldr.dep: $(SRC.FOUNTLDR)
	$(DO.DEP)
else
-include $(OUTOS)fountldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
