#------------------------------------------------------------------------------
# ReadXML document system submakefile
#------------------------------------------------------------------------------
DESCRIPTION.xmlread = Crystal Space XMLRead document system

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make xmlread     Make the $(DESCRIPTION.xmlread)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: xmlread xmlreadclean
all plugins: xmlread

xmlread:
	$(MAKE_TARGET) MAKE_DLL=yes
xmlreadclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  XMLREAD = $(OUTDLL)/xmlread$(DLL)
  LIB.XMLREAD = $(foreach d,$(DEP.XMLREAD),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(XMLREAD)
else
  XMLREAD = $(OUT)/$(LIB_PREFIX)xmlread$(LIB)
  DEP.EXE += $(XMLREAD)
  SCF.STATIC += xmlread
  TO_INSTALL.STATIC_LIBS += $(XMLREAD)
endif

DIR.XMLREAD = plugins/documentsystem/xmlread
OUT.XMLREAD = $(OUT)/$(DIR.XMLREAD)
INC.XMLREAD = $(wildcard $(DIR.XMLREAD)/*.h)
SRC.XMLREAD = $(wildcard $(DIR.XMLREAD)/*.cpp)
OBJ.XMLREAD = $(addprefix $(OUT.XMLREAD)/,$(notdir $(SRC.XMLREAD:.cpp=$O)))
DEP.XMLREAD = CSUTIL CSTOOL CSSYS CSUTIL

OUTDIRS += $(OUT.XMLREAD)

MSVC.DSP += XMLREAD
DSP.XMLREAD.NAME = xmlread
DSP.XMLREAD.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: xmlread xmlreadclean xmlreadcleandep
xmlread: $(OUTDIRS) $(XMLREAD)

$(OUT.XMLREAD)/%$O: $(DIR.XMLREAD)/%.cpp
	$(DO.COMPILE.CPP)

$(XMLREAD): $(OBJ.XMLREAD) $(LIB.XMLREAD)
	$(DO.PLUGIN)

clean: xmlreadclean
xmlreadclean:
	-$(RM) $(XMLREAD) $(OBJ.XMLREAD)

cleandep: xmlreadcleandep
xmlreadcleandep:
	-$(RM) $(OUT.XMLREAD)/xmlread.dep

ifdef DO_DEPEND
dep: $(OUT.XMLREAD) $(OUT.XMLREAD)/xmlread.dep
$(OUT.XMLREAD)/xmlread.dep: $(SRC.XMLREAD)
	$(DO.DEPEND)
else
-include $(OUT.XMLREAD)/xmlread.dep
endif

endif # ifeq ($(MAKESECTION),targets)
