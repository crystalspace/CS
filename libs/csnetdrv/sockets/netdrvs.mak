# This is a subinclude file used to define the rules needed
# to build the NULL network driver

# Driver description
DESCRIPTION.netdrvs = Crystal Space socket network driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += $(NEWLINE)echo $"  make netdrvs      Make the $(DESCRIPTION.netdrvs)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: netdrvs

all plugins drivers netdrivers: netdrvs

netdrvs:
	$(MAKE_TARGET) MAKE_DLL=yes
netdrvsclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(NEED_SOCKET_LIB),yes)
  LIBS.LOCAL.NETDRVS=$(LFLAGS.l)socket
endif

# The NULL Network driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  NETDRVS=$(OUTDLL)netdrvs$(DLL)
  LIBS.NETDRVS=$(LIBS.LOCAL.NETDRVS)
  DEP.NETDRVS=$(CSSYS.LIB) $(CSUTIL.LIB)
else
  NETDRVS=$(OUT)$(LIB_PREFIX)netdrvs$(LIB)
  DEP.EXE+=$(NETDRVS)
  LIBS.EXE+=$(LIBS.LOCAL.NETDRVS)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_NETSOCKS
endif
DESCRIPTION.$(NETDRVS) = $(DESCRIPTION.netdrvs)
SRC.NETDRVS = $(wildcard libs/csnetdrv/sockets/*.cpp)
OBJ.NETDRVS = $(addprefix $(OUT),$(notdir $(SRC.NETDRVS:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp libs/csnetdrv/sockets

.PHONY: netdrvs netdrvsclean

# Chain rules
net: netdrvs
clean: netdrvsclean

netdrvs: $(OUTDIRS) $(NETDRVS)

$(NETDRVS): $(OBJ.NETDRVS) $(DEP.NETDRVS)
	$(DO.PLUGIN) $(LIBS.NETDRVS)

netdrvsclean:
	$(RM) $(NETDRVS) $(OBJ.NETDRVS)

ifdef DO_DEPEND
depend: $(OUTOS)netdrvs.dep
$(OUTOS)netdrvs.dep: $(SRC.NETDRVS)
	$(DO.DEP)
else
-include $(OUTOS)netdrvs.dep
endif

endif # ifeq ($(MAKESECTION),targets)
