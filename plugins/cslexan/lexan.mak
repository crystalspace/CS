#------------------------------------------------------------------------------
# Lexical Analyzer plugin submakefile
#------------------------------------------------------------------------------
DESCRIPTION.cslexan = Crystal Space lexical analyzer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make cslexan      Make the $(DESCRIPTION.cslexan)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cslexan cslexanclean
all plugins: cslexan

cslexan:
	$(MAKE_TARGET) MAKE_DLL=yes
cslexanclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/cslexan

ifeq ($(USE_PLUGINS),yes)
  CSLEXAN = $(OUTDLL)/cslexan$(DLL)
  LIB.CSLEXAN = $(foreach d,$(DEP.CSLEXAN),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSLEXAN)
else
  CSLEXAN = $(OUT)/$(LIB_PREFIX)cslexan$(LIB)
  DEP.EXE += $(CSLEXAN)
  SCF.STATIC += cslexan
  TO_INSTALL.STATIC_LIBS += $(CSLEXAN)
endif

INC.CSLEXAN = $(wildcard plugins/cslexan/*.h)
SRC.CSLEXAN = $(wildcard plugins/cslexan/*.cpp)
OBJ.CSLEXAN = $(addprefix $(OUT)/,$(notdir $(SRC.CSLEXAN:.cpp=$O)))
DEP.CSLEXAN = CSUTIL CSSYS CSUTIL

MSVC.DSP += CSLEXAN
DSP.CSLEXAN.NAME = cslexan
DSP.CSLEXAN.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cslexan cslexanclean
cslexan: $(OUTDIRS) $(CSLEXAN)

$(CSLEXAN): $(OBJ.CSLEXAN) $(LIB.CSLEXAN)
	$(DO.PLUGIN)

clean: cslexanclean
cslexanclean:
	-$(RM) $(CSLEXAN) $(OBJ.CSLEXAN)

ifdef DO_DEPEND
dep: $(OUTOS)/cslexan.dep
$(OUTOS)/cslexan.dep: $(SRC.CSLEXAN)
	$(DO.DEP)
else
-include $(OUTOS)/cslexan.dep
endif

endif # ifeq ($(MAKESECTION),targets)
