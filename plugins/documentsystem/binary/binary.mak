#------------------------------------------------------------------------------
# Binary document system submakefile
#------------------------------------------------------------------------------

DESCRIPTION.bindoc = Crystal Space binary document system

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)


PLUGINHELP += \
  $(NEWLINE)echo $"  make bindoc     Make the $(DESCRIPTION.bindoc)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: bindoc bindocclean
all plugins: bindoc

bindoc:
	$(MAKE_TARGET) MAKE_DLL=yes
bindocclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  BINDOC = $(OUTDLL)/bindoc$(DLL)
  LIB.BINDOC = $(foreach d,$(DEP.BINDOC),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(BINDOC)
else
  BINDOC = $(OUT)/$(LIB_PREFIX)bindoc$(LIB)
  DEP.EXE += $(BINDOC)
  SCF.STATIC += bindoc
  TO_INSTALL.STATIC_LIBS += $(BINDOC)
endif

DIR.BINDOC = plugins/documentsystem/binary
OUT.BINDOC = $(OUT)/$(DIR.BINDOC)
INC.BINDOC = $(wildcard $(DIR.BINDOC)/*.h)
SRC.BINDOC = $(wildcard $(DIR.BINDOC)/*.cpp)
OBJ.BINDOC = $(addprefix $(OUT.BINDOC)/,$(notdir $(SRC.BINDOC:.cpp=$O)))
DEP.BINDOC = CSUTIL CSTOOL CSSYS CSUTIL

OUTDIRS += $(OUT.BINDOC)

MSVC.DSP += BINDOC
DSP.BINDOC.NAME = bindoc
DSP.BINDOC.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: bindoc bindocclean bindoccleandep

bindoc: $(OUTDIRS) $(BINDOC)

$(OUT.BINDOC)/%$O: $(DIR.BINDOC)/%.cpp
	$(DO.COMPILE.CPP)

$(BINDOC): $(OBJ.BINDOC) $(LIB.BINDOC)
	$(DO.PLUGIN)

clean: bindocclean
bindocclean:
	-$(RM) $(BINDOC) $(OBJ.BINDOC)

cleandep: bindoccleandep
bindoccleandep:
	-$(RM) $(OUT.BINDOC)/bindoc.dep

ifdef DO_DEPEND
dep: $(OUT.BINDOC) $(OUT.BINDOC)/bindoc.dep
$(OUT.BINDOC)/bindoc.dep: $(SRC.BINDOC)
	$(DO.DEPEND)
else
-include $(OUT.BINDOC)/bindoc.dep
endif

endif # ifeq ($(MAKESECTION),targets)
