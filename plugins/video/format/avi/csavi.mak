DESCRIPTION.csavi = Crystal Space AVI Streamformat

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make csavi        Make the $(DESCRIPTION.csavi)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csavi csaviclean
plugins all: csavi

csavi:
	$(MAKE_TARGET) MAKE_DLL=yes
csaviclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/format/avi

ifeq ($(USE_PLUGINS),yes)
  CSAVI = $(OUTDLL)csavi$(DLL)
  LIB.CSAVI = $(foreach d,$(DEP.CSAVI),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSAVI)
else
  CSAVI = $(OUT)$(LIB_PREFIX)csavi$(LIB)
  DEP.EXE += $(CSAVI)
  SCF.STATIC += csavi
  TO_INSTALL.STATIC_LIBS += $(CSAVI)
endif

INC.CSAVI = $(wildcard plugins/video/format/avi/*.h)
SRC.CSAVI = $(wildcard plugins/video/format/avi/*.cpp)
#SRC.CSAVI += $(wildcard plugins/video/format/avi/*.asm)
OBJ.CSAVI = $(addprefix $(OUT),$(notdir $(subst .asm,$O,$(SRC.CSAVI:.cpp=$O))))

NASMFLAGS.CSAVI = -i./plugins/video/renderer/software/i386/

DEP.CSAVI = CSGFX CSUTIL CSGEOM CSSYS
CFG.CSAVI = 

TO_INSTALL.CONFIG += $(CFG.CSAVI)
TO_INSTALL.DATA += 

MSVC.DSP += CSAVI
DSP.CSAVI.NAME = csavi
DSP.CSAVI.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csavi csaviclean
csavi: $(OUTDIRS) $(CSAVI)

#$(OUT)%$O: plugins/video/format/avi/%.asm
#	$(DO.COMPILE.ASM) $(NASMFLAGS.CSAVI)

$(CSAVI): $(OBJ.CSAVI) $(LIB.CSAVI)
	$(DO.PLUGIN)

clean: csaviclean
csaviclean:
	-$(RM) $(CSAVI) $(OBJ.CSAVI)

ifdef DO_DEPEND
dep: $(OUTOS)csavi.dep
$(OUTOS)csavi.dep: $(SRC.CSAVI)
	$(DO.DEP)
else
-include $(OUTOS)csavi.dep
endif

endif # ifeq ($(MAKESECTION),targets)
