#------------------------------------------------------------------------------
# Map File Parser plugin makefile
#------------------------------------------------------------------------------
DESCRIPTION.csparser = Crystal Space map file parser

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make csparser     Make the $(DESCRIPTION.csparser)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csparser csparserclean
all plugins: csparser

csparser:
	$(MAKE_TARGET) MAKE_DLL=yes
csparserclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/csparser

ifeq ($(USE_PLUGINS),yes)
  CSPARSER = $(OUTDLL)/csparser$(DLL)
  LIB.CSPARSER = $(foreach d,$(DEP.CSPARSER),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSPARSER)
else
  CSPARSER = $(OUT)/$(LIB_PREFIX)csparser$(LIB)
  DEP.EXE += $(CSPARSER)
  SCF.STATIC += csparser
  TO_INSTALL.STATIC_LIBS += $(CSPARSER)
endif

INF.CSPARSER = $(SRCDIR)/plugins/csparser/csparser.csplugin
INC.CSPARSER = $(wildcard $(addprefix $(SRCDIR)/,plugins/csparser/*.h))
SRC.CSPARSER = $(wildcard $(addprefix $(SRCDIR)/,plugins/csparser/*.cpp))
OBJ.CSPARSER = $(addprefix $(OUT)/,$(notdir $(SRC.CSPARSER:.cpp=$O)))
DEP.CSPARSER = CSUTIL CSTOOL CSUTIL CSGEOM CSTOOL CSGFX

MSVC.DSP += CSPARSER
DSP.CSPARSER.NAME = csparser
DSP.CSPARSER.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csparser csparserclean
csparser: $(OUTDIRS) $(CSPARSER)

$(CSPARSER): $(OBJ.CSPARSER) $(LIB.CSPARSER)
	$(DO.PLUGIN)

clean: csparserclean
csparserclean:
	-$(RMDIR) $(CSPARSER) $(OBJ.CSPARSER) $(OUTDLL)/$(notdir $(INF.CSPARSER))

ifdef DO_DEPEND
dep: $(OUTOS)/csparser.dep
$(OUTOS)/csparser.dep: $(SRC.CSPARSER)
	$(DO.DEP)
else
-include $(OUTOS)/csparser.dep
endif

endif # ifeq ($(MAKESECTION),targets)
