# Driver description
DESCRIPTION.cssocket = Crystal Space socket network driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make cssocket     Make the $(DESCRIPTION.cssocket)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cssocket cssocketclean
all plugins drivers netdrivers: cssocket

cssocket:
	$(MAKE_TARGET) MAKE_DLL=yes
cssocketclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/net/driver/socket

ifeq ($(USE_PLUGINS),yes)
  CSSOCKET = $(OUTDLL)/cssocket$(DLL)
  LIB.CSSOCKET = $(foreach d,$(DEP.CSSOCKET),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSSOCKET)
else
  CSSOCKET = $(OUT)/$(LIB_PREFIX)cssocket$(LIB)
  DEP.EXE += $(CSSOCKET)
  LIBS.EXE += $(SOCKET.LFLAGS)
  SCF.STATIC += cssocket
  TO_INSTALL.STATIC_LIBS += $(CSSOCKET)
endif

INC.CSSOCKET = $(wildcard plugins/net/driver/socket/*.h)
SRC.CSSOCKET = $(wildcard plugins/net/driver/socket/*.cpp)
OBJ.CSSOCKET = $(addprefix $(OUT)/,$(notdir $(SRC.CSSOCKET:.cpp=$O)))
DEP.CSSOCKET = CSUTIL CSSYS CSUTIL

MSVC.DSP += CSSOCKET
DSP.CSSOCKET.NAME = cssocket
DSP.CSSOCKET.TYPE = plugin
DSP.CSSOCKET.LIBS = wsock32

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cssocket cssocketclean

cssocket: $(OUTDIRS) $(CSSOCKET)

$(CSSOCKET): $(OBJ.CSSOCKET) $(LIB.CSSOCKET)
	$(DO.PLUGIN) $(SOCKET.LFLAGS)

clean: cssocketclean
cssocketclean:
	$(RM) $(CSSOCKET) $(OBJ.CSSOCKET)

ifdef DO_DEPEND
depend: $(OUTOS)/socket.dep
$(OUTOS)/socket.dep: $(SRC.CSSOCKET)
	$(DO.DEP)
else
-include $(OUTOS)/socket.dep
endif

endif # ifeq ($(MAKESECTION),targets)
