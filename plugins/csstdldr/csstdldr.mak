# Plugin description
DESCRIPTION.stdldr = Standard Crystal Space geometry loader plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plugin-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make stdldr       Make the $(DESCRIPTION.stdldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: stdldr stdldrclean

#@@@ dont enable till stdldr will be ready
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

ifeq ($(USE_SHARED_PLUGINS),yes)
# STDLDR = $(OUTDLL)stdldr$(DLL)
  STDLDR = testldr$(EXE)
  DEP.STDLDR = $(CSUTIL.LIB) $(CSGEOM.LIB)
# TO_INSTALL.DYNAMIC_LIBS += $(STDLDR)
else
  STDLDR = $(OUT)$(LIB_PREFIX)stdld$(LIB)
  DEP.EXE += $(STDLDR)
  CFLAGS.STATIC_SCF += $(CFLAGS.D)SCL_STDLDR
# TO_INSTALL.STATIC_LIBS += $(STDLDR)
endif
DESCRIPTION.$(STDLDR) = $(DESCRIPTION.stdldr)
SRC.STDLDR = plugins/csstdldr/stdparse.cpp plugins/csstdldr/stdldr.cpp \
  plugins/csstdldr/test.cpp
OBJ.STDLDR = $(addprefix $(OUT),$(notdir $(SRC.STDLDR:.cpp=$O)))

#@@@ to be removed
DEP.STDLDR += \
$(CSENGINE.LIB) \
$(CSTERR.LIB) \
$(CSGEOM.LIB) \
$(CSGFXLDR.LIB) \
$(CSUTIL.LIB) \
$(CSSYS.LIB) \
$(CSOBJECT.LIB) 

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp plugins/csstdldr

.PHONY: stdldr stdldrclean

all: $(STDLDR)
stdldr: $(OUTDIRS) $(STDLDR)
clean: stdldrclean

plugins/csstdldr/stdparse.cpp: plugins/csstdldr/stdparse.y
	bison -d -o $* $<
	mv -f $* $@

$(STDLDR): $(OBJ.STDLDR) $(DEP.STDLDR)
	$(DO.LINK.CONSOLE.EXE)
#	$(DO.PLUGIN)

stdldrclean:
	-$(RM) $(STDLDR) $(OBJ.STDLDR) $(OUTOS)stdldr.dep

ifdef DO_DEPEND
dep: $(OUTOS)stdldr.dep
$(OUTOS)stdldr.dep: $(SRC.STDLDR)
	$(DO.DEP)
else
-include $(OUTOS)stdldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
