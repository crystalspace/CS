# This is a subinclude file used to define the rules needed
# to build the NULL network driver

# Driver description
DESCRIPTION.netdrvs = Crystal Space socket network driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRVHELP += $(NEWLINE)echo $"  make netdrvs      Make the $(DESCRIPTION.netdrvs)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: netdrvs

all drivers netdrivers: netdrvs

netdrvs:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(NEED_SOCKET_LIB),yes)
  LIBS._NETDRVS=$(LFLAGS.l)socket
endif

# The NULL Network driver
ifeq ($(USE_DLL),yes)
  NETDRVS=$(OUTDLL)netdrvs$(DLL)
  LIBS.NETDRVS=$(LIBS._NETDRVS)
  DEP.NETDRVS=$(CSCOM.LIB) $(CSSYS.LIB) $(CSUTIL.LIB)
else
  NETDRVS=$(OUT)$(LIB_PREFIX)netdrvs$(LIB)
  DEP.EXE+=$(NETDRVS)
  LIBS.EXE+=$(LIBS._NETDRVS)
  CFLAGS.STATIC_COM+=$(CFLAGS.D)SCL_NETSOCKS
endif
DESCRIPTION.$(NETDRVS) = $(DESCRIPTION.netdrvs)
SRC.NETDRVS = $(wildcard libs/csnetdrv/sockets/*.cpp)
OBJ.NETDRVS = $(addprefix $(OUT),$(notdir $(SRC.NETDRVS:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp libs/csnetdrv/sockets

.PHONY: netdrvs netdrvsclean netdrvscleanlib

# Chain rules
net: netdrvs
clean: netdrvsclean
cleanlib: netdrvscleanlib

netdrvs: $(OUTDIRS) $(NETDRVS)

$(NETDRVS): $(OBJ.NETDRVS) $(DEP.NETDRVS)
	$(DO.LIBRARY) $(LIBS.NETDRVS)

netdrvsclean:
	$(RM) $(NETDRVS)

netdrvscleanlib:
	$(RM) $(OBJ.NETDRVS) $(NETDRVS)

ifdef DO_DEPEND
$(OUTOS)netdrvs.dep: $(SRC.NETDRVS)
	$(DO.DEP)
endif

-include $(OUTOS)netdrvs.dep

endif # ifeq ($(MAKESECTION),targets)
