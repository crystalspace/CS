DESCRIPTION.mballldr = MetaBalls mesh object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make mballldr     Make the $(DESCRIPTION.mballldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: mballldr mballldrclean
plugins meshes all: mballldr

mballldrclean:
	$(MAKE_CLEAN)
mballldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/loader/metaball

ifeq ($(USE_PLUGINS),yes)
  MBALLLDR = $(OUTDLL)mballldr$(DLL)
  LIB.MBALLLDR = $(foreach d,$(DEP.MBALLLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(MBALLLDR)
else
  MBALLLDR = $(OUT)$(LIB_PREFIX)mballldr$(LIB)
  DEP.EXE += $(MBALLLDR)
  SCF.STATIC += mballldr
  TO_INSTALL.STATIC_LIBS += $(MBALLLDR)
endif

INC.MBALLLDR = $(wildcard plugins/mesh/loader/metaball/*.h)
SRC.MBALLLDR = $(wildcard plugins/mesh/loader/metaball/*.cpp)
OBJ.MBALLLDR = $(addprefix $(OUT),$(notdir $(SRC.MBALLLDR:.cpp=$O)))
DEP.MBALLLDR = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += MBALLLDR
DSP.MBALLLDR.NAME = mballldr
DSP.MBALLLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: mballldr mballldrclean
mballldr: $(OUTDIRS) $(MBALLLDR)

$(MBALLLDR): $(OBJ.MBALLLDR) $(LIB.MBALLLDR)
	$(DO.PLUGIN)

clean: mballldrclean
mballldrclean:
	-$(RM) $(MBALLLDR) $(OBJ.MBALLLDR)

ifdef DO_DEPEND
dep: $(OUTOS)mballldr.dep
$(OUTOS)mballldr.dep: $(SRC.MBALLLDR)
	$(DO.DEP)
else
-include $(OUTOS)mballldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
