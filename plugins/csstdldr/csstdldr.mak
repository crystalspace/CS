# Plugin description
DESCRIPTION.stdldr = Crystal Space geometry loader plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plugin-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make stdldr       Make the $(DESCRIPTION.stdldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: stdldr stdldrclean

#@@@ Do not enable till stdldr is ready
#all plugins: stdldr

stdldr:
	$(MAKE_TARGET) MAKE_DLL=yes
stdldrclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# We need our own bison.simple file
export BISON_SIMPLE = support/gnu/bison.sim

ifeq ($(USE_PLUGINS),yes)
# STDLDR = $(OUTDLL)stdldr$(DLL)
  STDLDR = testldr$(EXE)
  LIB.STDLDR = $(foreach d,$(DEP.STDLDR),$($d.LIB))
# TO_INSTALL.DYNAMIC_LIBS += $(STDLDR)
else
  STDLDR = $(OUT)$(LIB_PREFIX)stdld$(LIB)
  DEP.EXE += $(STDLDR)
  SCF.STATIC += stdldr
# TO_INSTALL.STATIC_LIBS += $(STDLDR)
endif

INC.STDLDR = plugins/csstdldr/stdparse.h plugins/csstdldr/stdldr.h
SRC.STDLDR = plugins/csstdldr/stdparse.cpp plugins/csstdldr/stdldr.cpp \
  plugins/csstdldr/test.cpp
OBJ.STDLDR = $(addprefix $(OUT),$(notdir $(SRC.STDLDR:.cpp=$O)))
DEP.STDLDR = CSUTIL CSGEOM

#@@@ to be removed
DEP.STDLDR += CSENGINE CSTERR CSGEOM CSGFXLDR CSUTIL CSSYS CSOBJECT 

#MSVC.DSP += STDLDR
#DSP.STDLDR.NAME = stdldr
#DSP.STDLDR.TYPE = plugin
#DSP.STDLDR.RESOURCES = $(wildcard plugin/csstdldr/*.y)

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp plugins/csstdldr

.PHONY: stdldr stdldrclean
stdldr: $(OUTDIRS) $(STDLDR)
clean: stdldrclean

plugins/csstdldr/stdparse.cpp: plugins/csstdldr/stdparse.y
	bison -d -o $* $<
	mv -f $* $@

$(STDLDR): $(OBJ.STDLDR) $(LIB.STDLDR)
	$(DO.LINK.CONSOLE.EXE)
#	$(DO.PLUGIN)

stdldrclean:
	-$(RM) $(STDLDR) $(OBJ.STDLDR)

ifdef DO_DEPEND
dep: $(OUTOS)stdldr.dep
$(OUTOS)stdldr.dep: $(SRC.STDLDR)
	$(DO.DEP)
else
-include $(OUTOS)stdldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
