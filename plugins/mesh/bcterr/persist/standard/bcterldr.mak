DESCRIPTION.bcterrldr = Bezier Terrain object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make bcterrldr    Make the $(DESCRIPTION.bcterrldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: bcterrldr bcterrldrclean
plugins all: bcterrldr

bcterrldrclean:
	$(MAKE_CLEAN)
bcterrldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/bcterr/persist/standard

ifeq ($(USE_PLUGINS),yes)
  BCTERRLDR = $(OUTDLL)/bcterrldr$(DLL)
  LIB.BCTERRLDR = $(foreach d,$(DEP.BCTERRLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(BCTERRLDR)
else
  BCTERRLDR = $(OUT)/$(LIB_PREFIX)bcterrldr$(LIB)
  DEP.EXE += $(BCTERRLDR)
  SCF.STATIC += bcterrldr
  TO_INSTALL.STATIC_LIBS += $(BCTERRLDR)
endif

INC.BCTERRLDR = $(wildcard plugins/mesh/bcterr/persist/standard/*.h)
SRC.BCTERRLDR = $(wildcard plugins/mesh/bcterr/persist/standard/*.cpp)
OBJ.BCTERRLDR = $(addprefix $(OUT)/,$(notdir $(SRC.BCTERRLDR:.cpp=$O)))
DEP.BCTERRLDR = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += BCTERRLDR
DSP.BCTERRLDR.NAME = bcterrldr
DSP.BCTERRLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: bcterrldr bcterrldrclean
bcterrldr: $(OUTDIRS) $(BCTERRLDR)

$(BCTERRLDR): $(OBJ.BCTERRLDR) $(LIB.BCTERRLDR)
	$(DO.PLUGIN)

clean: bcterrldrclean
bcterrldrclean:
	-$(RM) $(BCTERRLDR) $(OBJ.BCTERRLDR)

ifdef DO_DEPEND
dep: $(OUTOS)/bcterrldr.dep
$(OUTOS)/bcterrldr.dep: $(SRC.BCTERRLDR)
	$(DO.DEP)
else
-include $(OUTOS)/bcterrldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
