# This is a subinclude file used to define the rules needed
# to build the CoreAudio sound driver for MacOS X

# Driver description
DESCRIPTION.casnddrv = Crystal Space CoreAudio sound driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make casnddrv     Make the $(DESCRIPTION.casnddrv)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: casnddrv casnddrvclean
all plugins drivers snddrivers: casnddrv

casnddrv:
	$(MAKE_TARGET) MAKE_DLL=yes
casnddrvclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/driver/coreaudio

# The CoreAudio sound driver
ifeq ($(USE_PLUGINS),yes)
  CASNDDRV = $(OUTDLL)casnddrv$(DLL)
  LIB.CASNDDRV = $(foreach d,$(DEP.CASNDDRV),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CASNDDRV)
else
  CASNDDRV = $(OUT)$(LIB_PREFIX)casnddrv.a
  DEP.EXE += $(CASNDDRV)
  SCF.STATIC += casnddrv
  TO_INSTALL.STATIC_LIBS += $(CASNDDRV)
endif

SRC.CASNDDRV = $(wildcard plugins/sound/driver/coreaudio/*.cpp)
OBJ.CASNDDRV = $(addprefix $(OUT),$(notdir $(SRC.CASNDDRV:.cpp=$O)))
LIB.CASNDDRV.COREAUDIO = -framework CoreAudio
DEP.CASNDDRV = CSUTIL CSSYS CSUTIL

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: casnddrv casnddrvclean

casnddrv: $(OUTDIRS) $(CASNDDRV)

$(CASNDDRV): $(OBJ.CASNDDRV) $(LIB.CASNDDRV)
	$(DO.PLUGIN.PREAMBLE) $(DO.PLUGIN.CORE) $(LIB.CASNDDRV.COREAUDIO) $(DO.PLUGIN.POSTAMBLE)

clean: casnddrvclean
casnddrvclean:
	$(RM) $(CASNDDRV) $(OBJ.CASNDDRV)

ifdef DO_DEPEND
dep: $(OUTOS)casnddrv.dep
$(OUTOS)casnddrv.dep: $(SRC.CASNDDRV)
	$(DO.DEP)
else
-include $(OUTOS)casnddrv.dep
endif

endif # ifeq ($(MAKESECTION),targets)
