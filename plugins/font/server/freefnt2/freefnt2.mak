DESCRIPTION.freefnt2 = Crystal Space FreeType font server

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make freefnt2     Make the $(DESCRIPTION.freefnt2)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: freefnt2 freefnt2clean
all plugins: freefnt2
freefnt2:
	$(MAKE_TARGET) MAKE_DLL=yes
freefnt2clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  FREEFNT2 = $(OUTDLL)/freefnt2$(DLL)
  LIB.FREEFNT2 = $(foreach d,$(DEP.FREEFNT2),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(FREEFNT2)
else
  FREEFNT2 = $(OUT)/$(LIB_PREFIX)freefnt2$(LIB)
  DEP.EXE += $(FREEFNT2)
  SCF.STATIC += freefnt2
  TO_INSTALL.STATIC_LIBS += $(FREEFNT2)
endif

INF.FREEFNT2 = $(SRCDIR)/plugins/font/server/freefnt2/freefnt2.csplugin
INC.FREEFNT2 = \
  $(wildcard $(addprefix $(SRCDIR)/,plugins/font/server/freefnt2/*.h))
SRC.FREEFNT2 = \
  $(wildcard $(addprefix $(SRCDIR)/,plugins/font/server/freefnt2/*.cpp))
OBJ.FREEFNT2 = $(addprefix $(OUT)/,$(notdir $(SRC.FREEFNT2:.cpp=$O)))
DEP.FREEFNT2 = CSUTIL
CFG.FREEFNT2 = $(SRCDIR)/data/config/freetype.cfg

TO_INSTALL.CONFIG += $(CFG.FREEFNT2)

MSVC.DSP += FREEFNT2
DSP.FREEFNT2.NAME = freefnt2
DSP.FREEFNT2.TYPE = plugin
#DSP.FREEFNT2.LFLAGS = /nodefaultlib:"MSVCRT"
DSP.FREEFNT2.CFLAGS = /I "..\..\include\cssys\win32\freetype2"
DSP.FREEFNT2.LIBS = freetype2

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: freefnt2 freefnt2clean
freefnt2: $(OUTDIRS) $(FREEFNT2)

$(OUT)/%$O: $(SRCDIR)/plugins/font/server/freefnt2/%.cpp
	$(DO.COMPILE.CPP) $(FT2.CFLAGS)

$(FREEFNT2): $(OBJ.FREEFNT2) $(LIB.FREEFNT2)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(FT2.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

clean: freefnt2clean
freefnt2clean:
	-$(RMDIR) $(FREEFNT2) $(OBJ.FREEFNT2) \
	$(OUTDLL)/$(notdir $(INF.FREEFNT2))

ifdef DO_DEPEND
dep: $(OUTOS)/freefnt2.dep
$(OUTOS)/freefnt2.dep: $(SRC.FREEFNT2)
	$(DO.DEP1) $(FT2.CFLAGS) $(DO.DEP2)
else
-include $(OUTOS)/freefnt2.dep
endif

endif # ifeq ($(MAKESECTION),targets)
