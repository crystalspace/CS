# Driver description
DESCRIPTION.ensocket = Crystal Space ensocket network driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make ensocket     Make the $(DESCRIPTION.ensocket)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: ensocket ensocketclean
all plugins drivers netdrivers: ensocket

ensocket:
	$(MAKE_TARGET) MAKE_DLL=yes
ensocketclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/net/driver/ensocket

ifeq ($(USE_PLUGINS),yes)
  ENSOCKET = $(OUTDLL)/ensocket$(DLL)
  LIB.ENSOCKET = $(foreach d,$(DEP.ENSOCKET),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(ENSOCKET)
else
  ENSOCKET = $(OUT)/$(LIB_PREFIX)ensocket$(LIB)
  DEP.EXE += $(ENSOCKET)
  LIBS.EXE += $(SOCKET.LFLAGS)
  SCF.STATIC += ensocket
  TO_INSTALL.STATIC_LIBS += $(ENSOCKET)
endif

INF.ENSOCKET = $(SRCDIR)/plugins/net/driver/ensocket/ensocket.csplugin
INC.ENSOCKET = $(wildcard $(addprefix $(SRCDIR)/,plugins/net/driver/ensocket/*.h))
SRC.ENSOCKET = $(wildcard $(addprefix $(SRCDIR)/,plugins/net/driver/ensocket/*.cpp))
OBJ.ENSOCKET = $(addprefix $(OUT)/,$(notdir $(SRC.ENSOCKET:.cpp=$O)))
DEP.ENSOCKET = CSUTIL CSUTIL

MSVC.DSP += ENSOCKET
DSP.ENSOCKET.NAME = ensocket
DSP.ENSOCKET.TYPE = plugin
DSP.ENSOCKET.LIBS = wsock32

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: ensocket ensocketclean

ensocket: $(OUTDIRS) $(ENSOCKET)

$(ENSOCKET): $(OBJ.ENSOCKET) $(LIB.ENSOCKET)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(SOCKET.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

clean: ensocketclean
ensocketclean:
	-$(RMDIR) $(ENSOCKET) $(OBJ.ENSOCKET) $(OUTDLL)/$(notdir $(INF.ENSOCKET))

ifdef DO_DEPEND
depend: $(OUTOS)/ensocket.dep
$(OUTOS)/ensocket.dep: $(SRC.ENSOCKET)
	$(DO.DEP)
else
-include $(OUTOS)/ensocket.dep
endif

endif # ifeq ($(MAKESECTION),targets)
