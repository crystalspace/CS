# This is a subinclude file used to define the rules needed
# to build the NULL sound driver

# Driver description
DESCRIPTION.snddrvn = Crystal Space NULL sound driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRVHELP += $(NEWLINE)echo $"  make snddrvn      Make the $(DESCRIPTION.snddrvn)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: snddrvn

all drivers snddrivers: snddrvn

snddrvn:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/cssnddrv/null

# The NULL Sound driver
ifeq ($(USE_DLL),yes)
  SNDDRVN=$(OUTDLL)snddrvn$(DLL)
  DEP.SNDDRVN=$(CSCOM.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  SNDDRVN=$(OUT)$(LIB_PREFIX)snddrvn$(LIB)
  DEP.EXE+=$(SNDDRVN)
  CFLAGS.STATIC_COM+=$(CFLAGS.D)SCL_SNDDRVN
endif
DESCRIPTION.$(SNDDRVN) = $(DESCRIPTION.snddrvn)
SRC.SNDDRVN = $(wildcard libs/cssnddrv/null/*.cpp)
OBJ.SNDDRVN = $(addprefix $(OUT),$(notdir $(SRC.SNDDRVN:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: snddrvn snddrvnclean snddrvncleanlib

# Chain rules
snd: snddrvn
clean: snddrvnclean
cleanlib: snddrvncleanlib

snddrvn: $(OUTDIRS) $(SNDDRVN)

$(SNDDRVN): $(OBJ.SNDDRVN) $(DEP.SNDDRVN)
	$(DO.LIBRARY)

snddrvnclean:
	$(RM) $(SNDDRVN)

snddrvncleanlib:
	$(RM) $(OBJ.SNDDRVN) $(SNDDRVN)

ifdef DO_DEPEND
$(OUTOS)snddrvn.dep: $(SRC.SNDDRVN)
	$(DO.DEP)
endif

-include $(OUTOS)snddrvn.dep

endif # ifeq ($(MAKESECTION),targets)
