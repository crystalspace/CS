#------------------------------------------------------------------------------
# Map File Parser plugin makefile
#------------------------------------------------------------------------------
DESCRIPTION.cssynldr = Crystal Space format loader services

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make cssynldr     Make the $(DESCRIPTION.cssynldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cssynldr cssynldrclean
all plugins: cssynldr

cssynldr:
	$(MAKE_TARGET) MAKE_DLL=yes
cssynldrclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/csparser/services

ifeq ($(USE_PLUGINS),yes)
  CSSYNLDR = $(OUTDLL)cssynldr$(DLL)
  LIB.CSSYNLDR = $(foreach d,$(DEP.CSSYNLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSSYNLDR)
else
  CSSYNLDR = $(OUT)$(LIB_PREFIX)cssynldr$(LIB)
  DEP.EXE += $(CSSYNLDR)
  SCF.STATIC += cssynldr
  TO_INSTALL.STATIC_LIBS += $(CSSYNLDR)
endif

INC.CSSYNLDR = $(wildcard plugins/csparser/services/*.h)
SRC.CSSYNLDR = $(wildcard plugins/csparser/services/*.cpp)
OBJ.CSSYNLDR = $(addprefix $(OUT),$(notdir $(SRC.CSSYNLDR:.cpp=$O)))
DEP.CSSYNLDR = CSUTIL CSTOOL CSSYS CSUTIL CSGEOM CSTOOL

MSVC.DSP += CSSYNLDR
DSP.CSSYNLDR.NAME = cssynldr
DSP.CSSYNLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cssynldr cssynldrclean
cssynldr: $(OUTDIRS) $(CSSYNLDR)

$(CSSYNLDR): $(OBJ.CSSYNLDR) $(LIB.CSSYNLDR)
	$(DO.PLUGIN)

clean: cssynldrclean
cssynldrclean:
	-$(RM) $(CSSYNLDR) $(OBJ.CSSYNLDR)

ifdef DO_DEPEND
dep: $(OUTOS)cssynldr.dep
$(OUTOS)cssynldr.dep: $(SRC.CSSYNLDR)
	$(DO.DEP)
else
-include $(OUTOS)cssynldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
