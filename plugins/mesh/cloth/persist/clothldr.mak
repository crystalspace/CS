DESCRIPTION.clothldr = Cloth mesh object factory loader plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make clothldr        Make the $(DESCRIPTION.clothldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: clothldr clothldrclean
plugins meshes all: clothldr

clothldrclean:
	$(MAKE_CLEAN)
clothldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/cloth/persist

ifeq ($(USE_PLUGINS),yes)
  CLOTHLDR = $(OUTDLL)/clothldr$(DLL)
  LIB.CLOTHLDR = $(foreach d,$(DEP.CLOTHLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CLOTHLDR)
else
  CLOTHLDR = $(OUT)/$(LIB_PREFIX)clothldr$(LIB)
  DEP.EXE += $(CLOTHLDR)
  SCF.STATIC += clothldr
  TO_INSTALL.STATIC_LIBS += $(CLOTHLDR)
endif

INC.CLOTHLDR = $(wildcard plugins/mesh/cloth/persist/*.h)
SRC.CLOTHLDR = $(wildcard plugins/mesh/cloth/persist/*.cpp)
OBJ.CLOTHLDR = $(addprefix $(OUT)/,$(notdir $(SRC.CLOTHLDR:.cpp=$O)))
DEP.CLOTHLDR = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += CLOTHLDR
DSP.CLOTHLDR.NAME = clothldr
DSP.CLOTHLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: clothldr clothldrclean
clothldr: $(OUTDIRS) $(CLOTHLDR)

$(CLOTHLDR): $(OBJ.CLOTHLDR) $(LIB.CLOTHLDR)
	$(DO.PLUGIN)

clean: clothldrclean
clothldrclean:
	-$(RM) $(CLOTHLDR) $(OBJ.CLOTHLDR)

ifdef DO_DEPEND
dep: $(OUTOS)/clothldr.dep
$(OUTOS)/clothldr.dep: $(SRC.CLOTHLDR)
	$(DO.DEP)
else
-include $(OUTOS)/clothldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
