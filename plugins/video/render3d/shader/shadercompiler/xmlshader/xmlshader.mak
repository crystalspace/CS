DESCRIPTION.xmlshader = XML-format shader compiler

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make xmlshader    Make the $(DESCRIPTION.xmlshader)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: xmlshader xmlshaderclean
all plugins: xmlshader

xmlshader:
	$(MAKE_TARGET) MAKE_DLL=yes
xmlshaderclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  XMLSHADER = $(OUTDLL)/xmlshader$(DLL)
  LIB.XMLSHADER = $(foreach d,$(DEP.XMLSHADER),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(XMLSHADER)
else
  XMLSHADER = $(OUT)/$(LIB_PREFIX)xmlshader$(LIB)
  DEP.EXE += $(XMLSHADER)
  SCF.STATIC += xmlshader
  TO_INSTALL.STATIC_LIBS += $(XMLSHADER)
endif

DIR.XMLSHADER = plugins/video/render3d/shader/shadercompiler/xmlshader
OUT.XMLSHADER = $(OUT)/$(DIR.XMLSHADER)
INF.XMLSHADER = $(SRCDIR)/$(DIR.XMLSHADER)/xmlshader.csplugin
INC.XMLSHADER = $(wildcard $(addprefix $(SRCDIR)/, \
  $(DIR.XMLSHADER)/*.h plugins/video/render3d/shader/common/*.h))
SRC.XMLSHADER = $(wildcard $(addprefix $(SRCDIR)/, \
  $(DIR.XMLSHADER)/*.cpp plugins/video/render3d/shader/common/*.cpp))
OBJ.XMLSHADER = \
  $(addprefix $(OUT.XMLSHADER)/,$(notdir $(SRC.XMLSHADER:.cpp=$O)))
DEP.XMLSHADER = CSTOOL CSGFX CSGEOM CSUTIL

OUTDIRS += $(OUT.XMLSHADER)

MSVC.DSP += XMLSHADER
DSP.XMLSHADER.NAME = xmlshader
DSP.XMLSHADER.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: xmlshader xmlshaderclean xmlshadercleandep

xmlshader: $(OUTDIRS) $(XMLSHADER)

$(OUT.XMLSHADER)/%$O: $(SRCDIR)/$(DIR.XMLSHADER)/%.cpp
	$(DO.COMPILE.CPP)

$(XMLSHADER): $(OBJ.XMLSHADER) $(LIB.XMLSHADER)
	$(DO.PLUGIN)

clean: xmlshaderclean
xmlshaderclean:
	-$(RMDIR) $(XMLSHADER) $(OBJ.XMLSHADER) \
	$(OUTDLL)/$(notdir $(INF.XMLSHADER))

cleandep: xmlshadercleandep
xmlshadercleandep:
	-$(RM) $(OUT.XMLSHADER)/xmlshader.dep

ifdef DO_DEPEND
dep: $(OUT.XMLSHADER) $(OUT.XMLSHADER)/xmlshader.dep
$(OUT.XMLSHADER)/xmlshader.dep: $(SRC.XMLSHADER)
	$(DO.DEPEND)
else
-include $(OUT.XMLSHADER)/xmlshader.dep
endif

endif # ifeq ($(MAKESECTION),targets)
