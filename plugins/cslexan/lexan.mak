#------------------------------------------------------------------------------
# Lexical Analyzer plugin submakefile
#------------------------------------------------------------------------------
DESCRIPTION.lexan = Crystal Space lexical analyzer plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make lexan        Make the $(DESCRIPTION.lexan)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: lexan lexanclean
all plugins: lexan

lexan:
	$(MAKE_TARGET) MAKE_DLL=yes
lexanclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/cslexan

ifeq ($(USE_PLUGINS),yes)
  LEXAN = $(OUTDLL)cslexan$(DLL)
  LIB.LEXAN = $(foreach d,$(DEP.LEXAN),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(LEXAN)
else
  LEXAN = $(OUT)$(LIB_PREFIX)cslexan$(LIB)
  DEP.EXE += $(LEXAN)
  SCF.STATIC += lexan
  TO_INSTALL.STATIC_LIBS += $(LEXAN)
endif

INC.LEXAN = $(wildcard plugins/cslexan/*.h)
SRC.LEXAN = $(wildcard plugins/cslexan/*.cpp)
OBJ.LEXAN = $(addprefix $(OUT),$(notdir $(SRC.LEXAN:.cpp=$O)))
DEP.LEXAN = CSUTIL CSSYS

MSVC.DSP += LEXAN
DSP.LEXAN.NAME = cslexan
DSP.LEXAN.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: lexan lexanclean
lexan: $(OUTDIRS) $(LEXAN)

$(LEXAN): $(OBJ.LEXAN) $(LIB.LEXAN)
	$(DO.PLUGIN)

clean: lexanclean
lexanclean:
	-$(RM) $(LEXAN) $(OBJ.LEXAN)

ifdef DO_DEPEND
dep: $(OUTOS)lexan.dep
$(OUTOS)lexan.dep: $(SRC.LEXAN)
	$(DO.DEP)
else
-include $(OUTOS)lexan.dep
endif

endif # ifeq ($(MAKESECTION),targets)
