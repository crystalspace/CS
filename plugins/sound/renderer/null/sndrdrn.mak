# This is a subinclude file used to define the rules needed
# to build the NULL sound renderer

# Driver description
DESCRIPTION.sndrdrn = Crystal Space NULL sound renderer

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += $(NEWLINE)echo $"  make sndrdrn      Make the $(DESCRIPTION.sndrdrn)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sndrdrn

all plugins drivers snddrivers: sndrdrn

sndrdrn:
	$(MAKE_TARGET) MAKE_DLL=yes
sndrdrnclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/cssndrdr/null

# The NULL Sound renderer
ifeq ($(USE_SHARED_PLUGINS),yes)
  SNDRDRN=$(OUTDLL)sndrdrn$(DLL)
  DEP.SNDRDRN=$(CSUTIL.LIB) $(CSSYS.LIB)
else
  SNDRDRN=$(OUT)$(LIB_PREFIX)sndrdrn$(LIB)
  DEP.EXE+=$(SNDRDRN)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_SNDRDRN
endif
DESCRIPTION.$(SNDRDRN) = $(DESCRIPTION.sndrdrn)
SRC.SNDRDRN = $(wildcard libs/cssndrdr/null/*.cpp)
OBJ.SNDRDRN = $(addprefix $(OUT),$(notdir $(SRC.SNDRDRN:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndrdrn sndrdrnclean

# Chain rules
snd: sndrdrn
clean: sndrdrnclean

sndrdrn: $(OUTDIRS) $(SNDRDRN)

$(SNDRDRN): $(OBJ.SNDRDRN) $(DEP.SNDRDRN)
	$(DO.PLUGIN)

sndrdrnclean:
	$(RM) $(SNDRDRN) $(OBJ.SNDRDRN)

ifdef DO_DEPEND
depend: $(OUTOS)sndrdrn.dep
$(OUTOS)sndrdrn.dep: $(SRC.SNDRDRN)
	$(DO.DEP)
else
-include $(OUTOS)sndrdrn.dep
endif

endif # ifeq ($(MAKESECTION),targets)
