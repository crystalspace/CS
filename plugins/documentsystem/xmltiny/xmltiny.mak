#------------------------------------------------------------------------------
# TinyXML document system submakefile
#------------------------------------------------------------------------------
DESCRIPTION.xmltiny = Crystal Space TinyXML document system

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make xmltiny      Make the $(DESCRIPTION.xmltiny)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: xmltiny xmltinyclean
all plugins: xmltiny

xmltiny:
	$(MAKE_TARGET) MAKE_DLL=yes
xmltinyclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  XMLTINY = $(OUTDLL)/xmltiny$(DLL)
  LIB.XMLTINY = $(foreach d,$(DEP.XMLTINY),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(XMLTINY)
else
  XMLTINY = $(OUT)/$(LIB_PREFIX)xmltiny$(LIB)
  DEP.EXE += $(XMLTINY)
  SCF.STATIC += xmltiny
  TO_INSTALL.STATIC_LIBS += $(XMLTINY)
endif

DIR.XMLTINY = plugins/documentsystem/xmltiny
OUT.XMLTINY = $(OUT)/$(DIR.XMLTINY)
INF.XMLTINY = $(SRCDIR)/$(DIR.XMLTINY)/xmltiny.csplugin
INC.XMLTINY = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.XMLTINY)/*.h))
SRC.XMLTINY = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.XMLTINY)/*.cpp))
OBJ.XMLTINY = $(addprefix $(OUT.XMLTINY)/,$(notdir $(SRC.XMLTINY:.cpp=$O)))
DEP.XMLTINY = CSUTIL CSTOOL CSUTIL

OUTDIRS += $(OUT.XMLTINY)

MSVC.DSP += XMLTINY
DSP.XMLTINY.NAME = xmltiny
DSP.XMLTINY.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: xmltiny xmltinyclean xmltinycleandep
xmltiny: $(OUTDIRS) $(XMLTINY)

$(OUT.XMLTINY)/%$O: $(SRCDIR)/$(DIR.XMLTINY)/%.cpp
	$(DO.COMPILE.CPP)

$(XMLTINY): $(OBJ.XMLTINY) $(LIB.XMLTINY)
	$(DO.PLUGIN)

clean: xmltinyclean
xmltinyclean:
	-$(RMDIR) $(XMLTINY) $(OBJ.XMLTINY) $(OUTDLL)/$(notdir $(INF.XMLTINY))

cleandep: xmltinycleandep
xmltinycleandep:
	-$(RM) $(OUT.XMLTINY)/xmltiny.dep

ifdef DO_DEPEND
dep: $(OUT.XMLTINY) $(OUT.XMLTINY)/xmltiny.dep
$(OUT.XMLTINY)/xmltiny.dep: $(SRC.XMLTINY)
	$(DO.DEPEND)
else
-include $(OUT.XMLTINY)/xmltiny.dep
endif

endif # ifeq ($(MAKESECTION),targets)
