 # This is a subinclude file used to define the rules needed
# to build the xmlshader plug-in.

# Driver description
DESCRIPTION.xmlshader = XML-format shader compiler

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make xmlshader    Make the $(DESCRIPTION.shadermgr)$"

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

vpath %.cpp $(SRCDIR)/plugins/video/render3d/shader/shadercompiler/xmlshader

ifeq ($(USE_PLUGINS),yes)
  XMLSHADER = $(OUTDLL)/shadermgr$(DLL)
  LIB.XMLSHADER = $(foreach d,$(DEP.XMLSHADER),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(XMLSHADER)
else
  XMLSHADER = $(OUT)/$(LIB_PREFIX)xmlshader$(LIB)
  DEP.EXE += $(XMLSHADER)
  SCF.STATIC += xmlshader
  TO_INSTALL.STATIC_LIBS += $(XMLSHADER)
endif

INF.XMLSHADER = $(SRCDIR)/plugins/video/render3d/shader/shadercompiler/xmlshader/xmlshader.csplugin
INC.XMLSHADER = $(wildcard $(addprefix $(SRCDIR)/,plugins/video/render3d/shader/shadercompiler/xmlshader/*.h \
  plugins/video/render3d/shader/common/*.h))
SRC.XMLSHADER = $(wildcard $(addprefix $(SRCDIR)/,plugins/video/render3d/shader/shadercompiler/xmlshader/*.cpp \
  plugins/video/render3d/shader/common/*.cpp))
OBJ.XMLSHADER = $(addprefix $(OUT)/,$(notdir $(SRC.XMLSHADER:.cpp=$O)))
DEP.XMLSHADER = CSGFX CSTOOL CSGEOM CSUTIL CSUTIL

MSVC.DSP += XMLSHADER
DSP.XMLSHADER.NAME = xmlshader
DSP.XMLSHADER.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: xmlshader xmlshaderclean

xmlshader: $(OUTDIRS) $(XMLSHADER)

$(XMLSHADER): $(OBJ.XMLSHADER) $(LIB.XMLSHADER)
	$(DO.PLUGIN)

clean: xmlshaderclean
xmlshaderclean:
	-$(RMDIR) $(XMLSHADER) $(OBJ.XMLSHADER) $(OUTDLL)/$(notdir $(INF.XMLSHADER))

ifdef DO_DEPEND
dep: $(OUTOS)/xmlshader.dep
$(OUTOS)/xmlshader.dep: $(SRC.XMLSHADER)
	$(DO.DEP)
else
-include $(OUTOS)/xmlshader.dep
endif

endif # ifeq ($(MAKESECTION),targets)
