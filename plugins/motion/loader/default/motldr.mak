# Plug-in description
DESCRIPTION.motldr = Crystal Space skeletal motion loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plugin-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make motldr       Make the $(DESCRIPTION.motldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: motldr motldrclean
all plugins: motldr

motldr:
	$(MAKE_TARGET) MAKE_DLL=yes
motldrclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/motion/loader/default

ifeq ($(USE_PLUGINS),yes)
  MOTLDR = $(OUTDLL)motldr$(DLL)
  LIB.MOTLDR = $(foreach d,$(DEP.MOTLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(MOTLDR)
else
  MOTLDR = $(OUT)$(LIB_PREFIX)motldr$(LIB)
  DEP.EXE += $(MOTLDR)
  SCF.STATIC += motldr
  TO_INSTALL.STATIC_LIBS += $(MOTLDR)
endif

INC.MOTLDR = $(wildcard plugins/motion/loader/default/*.h)
SRC.MOTLDR = $(wildcard plugins/motion/loader/default/*.cpp)
OBJ.MOTLDR = $(addprefix $(OUT),$(notdir $(SRC.MOTLDR:.cpp=$O)))
DEP.MOTLDR = CSGEOM CSSYS CSUTIL

MSVC.DSP += MOTLDR
DSP.MOTLDR.NAME = motldr
DSP.MOTLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: motldr motldrclean

motldr: $(OUTDIRS) $(MOTLDR)

$(MOTLDR): $(OBJ.MOTLDR) $(LIB.MOTLDR)
	$(DO.PLUGIN)

clean: motldrclean
motldrclean:
	$(RM) $(MOTLDR) $(OBJ.MOTLDR)

ifdef DO_DEPEND
dep: $(OUTOS)motldr.dep
$(OUTOS)motldr.dep: $(SRC.MOTLDR)
	$(DO.DEP)
else
-include $(OUTOS)motldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
