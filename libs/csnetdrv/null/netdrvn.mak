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

all drivers netdrivers: netdrvn

netdrvn:
	$(MAKE_TARGET) MAKE_DLL=yes
netdrvnclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csnetdrv/null

# The NULL Network driver
ifeq ($(USE_SHARED_PLUGINS),yes)
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

.PHONY: netdrvn netdrvnclean

# Chain rules
net: netdrvn
clean: netdrvnclean

netdrvn: $(OUTDIRS) $(NETDRVN)

$(NETDRVN): $(OBJ.NETDRVN) $(DEP.NETDRVN)
	$(DO.PLUGIN)

netdrvnclean:
	$(RM) $(NETDRVN) $(OBJ.NETDRVN)

ifdef DO_DEPEND
depend: $(OUTOS)netdrvn.dep
$(OUTOS)netdrvn.dep: $(SRC.NETDRVN)
	$(DO.DEP)
else
-include $(OUTOS)netdrvn.dep
endif

endif # ifeq ($(MAKESECTION),targets)
