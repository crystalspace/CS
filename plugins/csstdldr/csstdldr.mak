# The standard Crystal Space geometry loader
ifneq (,$(findstring stdldr,$(PLUGINS)))

# Library description
DESCRIPTION.stdldr = Standard Crystal Space geometry loader plug-in

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
PLUGINHELP += $(NEWLINE)echo $"  make stdldr       Make the $(DESCRIPTION.stdldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: stdldr

all plugins: stdldr

stdldr:
	$(MAKE_TARGET) MAKE_DLL=yes
stdldrclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_SHARED_PLUGINS),yes)
#  STDLDR = $(OUTDLL)stdldr$(DLL)
  STDLDR = stdldr$(EXE)
  DEP.STDLDR = $(CSUTIL.LIB) $(CSGEOM.LIB)
else
  STDLDR = $(OUT)$(LIB_PREFIX)stdldr$(LIB)
#  DEP.EXE += $(STDLDR)
#  CFLAGS.STATIC_SCF += $(CFLAGS.D)SCL_STDLDR
endif
DESCRIPTION.$(STDLDR) = $(DESCRIPTION.stdldr)
SRC.STDLDR = plugins/csstdldr/stdparse.cpp plugins/csstdldr/stdldr.cpp \
  plugins/csstdldr/test.cpp
OBJ.STDLDR = $(addprefix $(OUT),$(notdir $(SRC.STDLDR:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp plugins/csstdldr

.PHONY: stdldr stdldrclean

all: $(STDLDR.LIB)
stdldr: $(OUTDIRS) $(STDLDR)
clean: stdldrclean

plugins/csstdldr/stdparse.cpp: plugins/csstdldr/stdparse.y
	bison -d -o $* $<
	mv -f $* $@

$(STDLDR): $(OBJ.STDLDR) $(DEP.STDLDR)
	$(DO.LINK.CONSOLE.EXE)
#	$(DO.PLUGIN)

stdldrclean:
	-$(RM) $(OBJ.STDLDR) $(STDLDR)

ifdef DO_DEPEND
depend: $(OUTOS)stdldr.dep
$(OUTOS)stdldr.dep: $(SRC.STDLDR)
	$(DO.DEP)
else
-include $(OUTOS)stdldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)

endif # ifneq (,$(findstring stdldr,$(PLUGINS)))
