# This is a subinclude file used to define the rules needed
# to build the software sound renderer

# Driver description
DESCRIPTION.sndsoft = Crystal Space software sound renderer

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += $(NEWLINE)echo $"  make sndsoft      Make the $(DESCRIPTION.sndsoft)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sndsoft

all plugins drivers snddrivers: sndsoft

sndsoft:
	$(MAKE_TARGET) MAKE_DLL=yes
sndsoftclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/renderer/software

# The Software Sound renderer
ifeq ($(USE_SHARED_PLUGINS),yes)
  SNDSOFT=$(OUTDLL)sndsoft$(DLL)
  DEP.SNDSOFT=$(CSUTIL.LIB) $(CSSYS.LIB) $(CSSNDLDR.LIB) $(CSSFXLDR.LIB)
else
  SNDSOFT=$(OUT)$(LIB_PREFIX)sndsoft$(LIB)
  DEP.EXE+=$(SNDSOFT)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_SNDSOFT
endif
DESCRIPTION.$(SNDSOFT) = $(DESCRIPTION.sndsoft)
SRC.SNDSOFT = $(wildcard plugins/sound/renderer/software/*.cpp)
OBJ.SNDSOFT = $(addprefix $(OUT),$(notdir $(SRC.SNDSOFT:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndsoft sndsoftclean

# Chain rules
snd: sndsoft
clean: sndsoftclean

sndsoft: $(OUTDIRS) $(SNDSOFT)

$(SNDSOFT): $(OBJ.SNDSOFT) $(DEP.SNDSOFT)
	$(DO.PLUGIN)

sndsoftclean:
	$(RM) $(SNDSOFT) $(OBJ.SNDSOFT)

ifdef DO_DEPEND
dep: $(OUTOS)sndsoft.dep
$(OUTOS)sndsoft.dep: $(SRC.SNDSOFT)
	$(DO.DEP)
else
-include $(OUTOS)sndsoft.dep
endif

endif # ifeq ($(MAKESECTION),targets)
