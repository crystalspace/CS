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

vpath %.cpp plugins/net/driver/ensocket

ifeq ($(USE_PLUGINS),yes)
  ENSOCKET = $(OUTDLL)ensocket$(DLL)
  LIB.ENSOCKET = $(foreach d,$(DEP.ENSOCKET),$($d.LIB))
  LIB.ENSOCKET.SPECIAL = $(LIBS.ENSOCKET.SYSTEM)
  TO_INSTALL.DYNAMIC_LIBS += $(ENSOCKET)
else
  ENSOCKET = $(OUT)$(LIB_PREFIX)ensocket$(LIB)
  DEP.EXE += $(ENSOCKET)
  LIBS.EXE += $(LIBS.ENSOCKET.SYSTEM)
  SCF.STATIC += ensocket
  TO_INSTALL.STATIC_LIBS += $(ENSOCKET)
endif

INC.ENSOCKET = $(wildcard plugins/net/driver/ensocket/*.h)
SRC.ENSOCKET = $(wildcard plugins/net/driver/ensocket/*.cpp)
OBJ.ENSOCKET = $(addprefix $(OUT),$(notdir $(SRC.ENSOCKET:.cpp=$O)))
DEP.ENSOCKET = CSUTIL CSSYS CSUTIL

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
	$(DO.PLUGIN) $(LIB.ENSOCKET.SPECIAL)

clean: ensocketclean
ensocketclean:
	$(RM) $(ENSOCKET) $(OBJ.ENSOCKET)

ifdef DO_DEPEND
depend: $(OUTOS)ensocket.dep
$(OUTOS)ensocket.dep: $(SRC.ENSOCKET)
	$(DO.DEP)
else
-include $(OUTOS)ensocket.dep
endif

endif # ifeq ($(MAKESECTION),targets)
