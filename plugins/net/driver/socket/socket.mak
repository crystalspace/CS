# This is a subinclude file used to define the rules needed
# to build the Socket-based network driver.

# Driver description
DESCRIPTION.socket = Crystal Space socket network driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make socket       Make the $(DESCRIPTION.socket)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: socket socketclean

all plugins netdrivers: socket

socket:
	$(MAKE_TARGET) MAKE_DLL=yes
socketclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(NEED_SOCKET_LIB),yes)
  LIBS.LOCAL.SOCKET=$(LFLAGS.l)socket
endif

# The NULL Network driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  SOCKET=$(OUTDLL)cssocket$(DLL)
  LIBS.SOCKET=$(LIBS.LOCAL.SOCKET) $(LIBS.SOCKET.SYSTEM)
  DEP.SOCKET=$(CSSYS.LIB) $(CSUTIL.LIB)
  TO_INSTALL.DYNAMIC_LIBS+=$(SOCKET)
else
  SOCKET=$(OUT)$(LIB_PREFIX)cssocket$(LIB)
  DEP.EXE+=$(SOCKET)
  LIBS.EXE+=$(LIBS.LOCAL.SOCKET)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_NETSOCKET
  TO_INSTALL.STATIC_LIBS+=$(SOCKET)
endif
DESCRIPTION.$(SOCKET) = $(DESCRIPTION.socket)
SRC.SOCKET = $(wildcard plugins/net/driver/socket/*.cpp)
OBJ.SOCKET = $(addprefix $(OUT),$(notdir $(SRC.SOCKET:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp plugins/net/driver/socket

.PHONY: socket socketclean

# Chain rules
net: socket
clean: socketclean

socket: $(OUTDIRS) $(SOCKET)

$(SOCKET): $(OBJ.SOCKET) $(DEP.SOCKET)
	$(DO.PLUGIN) $(LIBS.SOCKET)

socketclean:
	$(RM) $(SOCKET) $(OBJ.SOCKET) $(OUTOS)socket.dep

ifdef DO_DEPEND
depend: $(OUTOS)socket.dep
$(OUTOS)socket.dep: $(SRC.SOCKET)
	$(DO.DEP)
else
-include $(OUTOS)socket.dep
endif

endif # ifeq ($(MAKESECTION),targets)
