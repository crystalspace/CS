# This is a subinclude file used to define the rules needed
# to build the NULL sound renderer

# Driver description
DESCRIPTION.sndrdrn = Crystal Space NULL sound renderer

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRVHELP += $(NEWLINE)echo $"  make sndrdrn      Make the $(DESCRIPTION.sndrdrn)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sndrdrn

ifeq ($(USE_DLL),yes)
all drivers snddrivers: sndrdrn
endif

sndrdrn:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/cssndrdr/null

# The NULL Sound renderer
ifeq ($(USE_DLL),yes)
  SNDRDRN=$(OUTDLL)sndrdrn$(DLL)
  DEP.SNDRDRN=$(CSCOM.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  SNDRDRN=$(OUT)$(LIB_PREFIX)sndrdrn$(LIB)
  DEP.EXE+=$(SNDRDRN)
  CFLAGS.STATIC_COM+=$(CFLAGS.D)SCL_SNDRDRN
endif
DESCRIPTION.$(SNDRDRN) = $(DESCRIPTION.sndrdrn)
SRC.SNDRDRN = $(wildcard libs/cssndrdr/null/*.cpp)
OBJ.SNDRDRN = $(addprefix $(OUT),$(notdir $(SRC.SNDRDRN:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndrdrn sndrdrnclean sndrdrncleanlib

# Chain rules
snd: sndrdrn
clean: sndrdrnclean
cleanlib: sndrdrncleanlib

sndrdrn: $(OUTDIRS) $(SNDRDRN)

$(SNDRDRN): $(OBJ.SNDRDRN) $(DEP.SNDRDRN)
	$(DO.LIBRARY)

sndrdrnclean:
	$(RM) $(SNDRDRN)

sndrdrncleanlib:
	$(RM) $(OBJ.SNDRDRN) $(SNDRDRN)

ifdef DO_DEPEND
$(OUTOS)sndrdrn.dep: $(SRC.SNDRDRN)
	$(DO.DEP) $(OUTOS)sndrdrn.dep
endif

-include $(OUTOS)sndrdrn.dep

endif # ifeq ($(MAKESECTION),targets)
