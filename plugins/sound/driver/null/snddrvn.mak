# This is a subinclude file used to define the rules needed
# to build the NULL sound driver

# Driver description
DESCRIPTION.snddrvn = Crystal Space NULL sound driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += $(NEWLINE)echo $"  make snddrvn      Make the $(DESCRIPTION.snddrvn)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: snddrvn

all plugins drivers snddrivers: snddrvn

snddrvn:
	$(MAKE_TARGET) MAKE_DLL=yes
snddrvnclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/cssnddrv/null

# The NULL Sound driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  SNDDRVN=$(OUTDLL)snddrvn$(DLL)
  DEP.SNDDRVN=$(CSUTIL.LIB) $(CSSYS.LIB)
else
  SNDDRVN=$(OUT)$(LIB_PREFIX)snddrvn$(LIB)
  DEP.EXE+=$(SNDDRVN)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_SNDDRVN
endif
DESCRIPTION.$(SNDDRVN) = $(DESCRIPTION.snddrvn)
SRC.SNDDRVN = $(wildcard libs/cssnddrv/null/*.cpp)
OBJ.SNDDRVN = $(addprefix $(OUT),$(notdir $(SRC.SNDDRVN:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: snddrvn snddrvnclean

# Chain rules
snd: snddrvn
clean: snddrvnclean

snddrvn: $(OUTDIRS) $(SNDDRVN)

$(SNDDRVN): $(OBJ.SNDDRVN) $(DEP.SNDDRVN)
	$(DO.PLUGIN)

snddrvnclean:
	$(RM) $(SNDDRVN) $(OBJ.SNDDRVN)

ifdef DO_DEPEND
depend: $(OUTOS)snddrvn.dep
$(OUTOS)snddrvn.dep: $(SRC.SNDDRVN)
	$(DO.DEP)
else
-include $(OUTOS)snddrvn.dep
endif

endif # ifeq ($(MAKESECTION),targets)
