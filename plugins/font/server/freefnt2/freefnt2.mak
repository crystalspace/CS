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
  FREEFONT2 = $(OUTDLL)/freefnt2$(DLL)
  LIB.FREEFONT2 = $(foreach d,$(DEP.FREEFONT2),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(FREEFONT2)
else
  FREEFONT2 = $(OUT)/$(LIB_PREFIX)freefnt2$(LIB)
  DEP.EXE += $(FREEFONT2)
  SCF.STATIC += freefnt2
  TO_INSTALL.STATIC_LIBS += $(FREEFONT2)
endif

INC.FREEFONT2 = $(wildcard plugins/font/server/freefnt2/*.h)
SRC.FREEFONT2 = $(wildcard plugins/font/server/freefnt2/*.cpp)
OBJ.FREEFONT2 = $(addprefix $(OUT)/,$(notdir $(SRC.FREEFONT2:.cpp=$O)))
DEP.FREEFONT2 = CSUTIL CSSYS
CFG.FREEFONT2 = data/config/freetype.cfg

TO_INSTALL.CONFIG += $(CFG.FREEFONT2)

MSVC.DSP += FREEFONT2
DSP.FREEFONT2.NAME = freefnt2
DSP.FREEFONT2.TYPE = plugin
#DSP.FREEFONT2.LFLAGS = /nodefaultlib:"MSVCRT"
DSP.FREEFONT2.CFLAGS = /I "..\..\include\cssys\win32\freetype2"
#DSP.FREEFONT2.LIBS = libfreetype
DSP.FREEFONT2.LIBS = freetype2

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: freefnt2 freefnt2clean
freefnt2: $(OUTDIRS) $(FREEFONT2)

$(OUT)/%$O: plugins/font/server/freefnt2/%.cpp
	$(DO.COMPILE.CPP) $(FT2.CFLAGS)

$(FREEFONT2): $(OBJ.FREEFONT2) $(LIB.FREEFONT2)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(FT2.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

clean: freefnt2clean
freefnt2clean:
	-$(RM) $(FREEFONT2) $(OBJ.FREEFONT2)

ifdef DO_DEPEND
dep: $(OUTOS)/freefont2.dep
$(OUTOS)/freefont2.dep: $(SRC.FREEFONT2)
	$(DO.DEP1) \
	$(FT2.CFLAGS) \
	$(DO.DEP2)
else
-include $(OUTOS)/freefont2.dep
endif

endif # ifeq ($(MAKESECTION),targets)
