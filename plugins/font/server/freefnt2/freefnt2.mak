DESCRIPTION.freefont2 = Crystal Space FreeType font server

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make freefont2    Make the $(DESCRIPTION.freefont2)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: freefont2 freefont2clean
all plugins: freefont2
freefont2:
	$(MAKE_TARGET) MAKE_DLL=yes
freefont2clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

LIB.EXTERNAL.FREEFONT2 = -lfreetype

ifeq ($(USE_PLUGINS),yes)
  FREEFONT2 = $(OUTDLL)freefnt2$(DLL)
  LIB.FREEFONT2 = $(foreach d,$(DEP.FREEFONT2),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(FREEFONT2)
else
  FREEFONT2 = $(OUT)$(LIB_PREFIX)freefnt2$(LIB)
  DEP.EXE += $(FREEFONT2)
  SCF.STATIC += freefnt2
  TO_INSTALL.STATIC_LIBS += $(FREEFONT2)
endif

INC.FREEFONT2 = $(wildcard plugins/font/server/freefnt2/*.h)
SRC.FREEFONT2 = $(wildcard plugins/font/server/freefnt2/*.cpp)
OBJ.FREEFONT2 = $(addprefix $(OUT),$(notdir $(SRC.FREEFONT2:.cpp=$O)))
DEP.FREEFONT2 = CSUTIL CSSYS
CFG.FREEFONT2 = data/config/freetype.cfg

TO_INSTALL.CONFIG += $(CFG.FREEFONT2)

MSVC.DSP += FREEFONT2
DSP.FREEFONT2.NAME = freefnt2
DSP.FREEFONT2.TYPE = plugin
DSP.FREEFONT2.LFLAGS = /nodefaultlib:"MSVCRT"
DSP.FREEFONT2.CFLAGS = /I "..\..\include\cssys\win32\freetype2"
DSP.FREEFONT2.LIBS = freetype208

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: freefont2 freefont2clean
freefont2: $(OUTDIRS) $(FREEFONT2)

$(OUT)%$O: plugins/font/server/freefnt2/%.cpp
	$(DO.COMPILE.CPP) -I/usr/include/freetype2

$(FREEFONT2): $(OBJ.FREEFONT2) $(LIB.FREEFONT2)
	$(DO.PLUGIN) $(LIB.EXTERNAL.FREEFONT2)

clean: freefont2clean
freefont2clean:
	-$(RM) $(FREEFONT2) $(OBJ.FREEFONT2)

ifdef DO_DEPEND
dep: $(OUTOS)freefont2.dep
$(OUTOS)freefont2.dep: $(SRC.FREEFONT2)
	$(DO.DEP)
else
-include $(OUTOS)freefont2.dep
endif

endif # ifeq ($(MAKESECTION),targets)
