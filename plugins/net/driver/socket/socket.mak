# Driver description
DESCRIPTION.socket = Crystal Space socket network driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
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

vpath %.cpp plugins/net/driver/socket

ifeq ($(USE_PLUGINS),yes)
  SOCKET = $(OUTDLL)cssocket$(DLL)
  LIB.SOCKET = $(foreach d,$(DEP.SOCKET),$($d.LIB))
  LIB.SOCKET.SPECIAL = $(LIBS.SOCKET.SYSTEM)
  TO_INSTALL.DYNAMIC_LIBS += $(SOCKET)
else
  SOCKET = $(OUT)$(LIB_PREFIX)cssocket$(LIB)
  DEP.EXE += $(SOCKET)
  LIBS.EXE += $(LIBS.SOCKET.SYSTEM)
  SCF.STATIC += cssocket
  TO_INSTALL.STATIC_LIBS += $(SOCKET)
endif

INC.SOCKET = $(wildcard plugins/net/driver/socket/*.h)
SRC.SOCKET = $(wildcard plugins/net/driver/socket/*.cpp)
OBJ.SOCKET = $(addprefix $(OUT),$(notdir $(SRC.SOCKET:.cpp=$O)))
DEP.SOCKET = CSSYS CSUTIL

MSVC.DSP += SOCKET
DSP.SOCKET.NAME = cssocket
DSP.SOCKET.TYPE = plugin
DSP.SOCKET.LIBS = wsock32

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: socket socketclean

socket: $(OUTDIRS) $(SOCKET)

$(SOCKET): $(OBJ.SOCKET) $(LIB.SOCKET)
	$(DO.PLUGIN) $(LIB.SOCKET.SPECIAL)

clean: socketclean
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
