DESCRIPTION.gmeshldr = General mesh object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make gmeshldr     Make the $(DESCRIPTION.gmeshldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: gmeshldr gmeshldrclean
plugins meshes all: gmeshldr

gmeshldrclean:
	$(MAKE_CLEAN)
gmeshldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/genmesh/persist/classic

ifeq ($(USE_PLUGINS),yes)
  GMESHLDR = $(OUTDLL)gmeshldr$(DLL)
  LIB.GMESHLDR = $(foreach d,$(DEP.GMESHLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(GMESHLDR)
else
  GMESHLDR = $(OUT)$(LIB_PREFIX)gmeshldr$(LIB)
  DEP.EXE += $(GMESHLDR)
  SCF.STATIC += gmeshldr
  TO_INSTALL.STATIC_LIBS += $(GMESHLDR)
endif

INC.GMESHLDR = $(wildcard plugins/mesh/genmesh/persist/classic/*.h)
SRC.GMESHLDR = $(wildcard plugins/mesh/genmesh/persist/classic/*.cpp)
OBJ.GMESHLDR = $(addprefix $(OUT),$(notdir $(SRC.GMESHLDR:.cpp=$O)))
DEP.GMESHLDR = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += GMESHLDR
DSP.GMESHLDR.NAME = gmeshldr
DSP.GMESHLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: gmeshldr gmeshldrclean
gmeshldr: $(OUTDIRS) $(GMESHLDR)

$(GMESHLDR): $(OBJ.GMESHLDR) $(LIB.GMESHLDR)
	$(DO.PLUGIN)

clean: gmeshldrclean
gmeshldrclean:
	-$(RM) $(GMESHLDR) $(OBJ.GMESHLDR)

ifdef DO_DEPEND
dep: $(OUTOS)gmeshldr.dep
$(OUTOS)gmeshldr.dep: $(SRC.GMESHLDR)
	$(DO.DEP)
else
-include $(OUTOS)gmeshldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
