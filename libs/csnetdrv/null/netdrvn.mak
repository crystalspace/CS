# This is a subinclude file used to define the rules needed
# to build the NULL network driver

# Driver description
DESCRIPTION.netdrvn = Crystal Space NULL network driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRVHELP += $(NEWLINE)echo $"  make netdrvn      Make the $(DESCRIPTION.netdrvn)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: netdrvn

ifeq ($(USE_DLL),yes)
all drivers netdrivers: netdrvn
endif

netdrvn:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csnetdrv/null

# The NULL Network driver
ifeq ($(USE_DLL),yes)
  NETDRVN=$(OUTDLL)netdrvn$(DLL)
  DEP.NETDRVN=$(CSCOM.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  NETDRVN=$(OUT)$(LIB_PREFIX)netdrvn$(LIB)
  DEP.EXE+=$(NETDRVN)
  CFLAGS.STATIC_COM+=$(CFLAGS.D)SCL_NETNULL
endif
DESCRIPTION.$(NETDRVN) = $(DESCRIPTION.netdrvn)
SRC.NETDRVN = $(wildcard libs/csnetdrv/null/*.cpp)
OBJ.NETDRVN = $(addprefix $(OUT),$(notdir $(SRC.NETDRVN:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: netdrvn netdrvnclean netdrvncleanlib

# Chain rules
net: netdrvn
clean: netdrvnclean
cleanlib: netdrvncleanlib

netdrvn: $(OUTDIRS) $(NETDRVN)

$(NETDRVN): $(OBJ.NETDRVN) $(DEP.NETDRVN)
	$(DO.LIBRARY)

netdrvnclean:
	$(RM) $(NETDRVN)

netdrvncleanlib:
	$(RM) $(OBJ.NETDRVN) $(NETDRVN)

ifdef DO_DEPEND
$(OUTOS)netdrvn.dep: $(SRC.NETDRVN)
	$(DO.DEP)
endif

-include $(OUTOS)netdrvn.dep

endif # ifeq ($(MAKESECTION),targets)
