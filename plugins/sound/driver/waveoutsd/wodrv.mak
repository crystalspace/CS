# This is a subinclude file used to define the rules needed
# to build the OpenSoundSystem driver -- ossdrv

# Driver description
DESCRIPTION.wos = Crystal Space WaveOut sound driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make wos          Make the $(DESCRIPTION.wos)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: wos wosclean

all plugins drivers snddrivers: wos

wos:
	$(MAKE_TARGET) MAKE_DLL=yes
wosclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/driver/waveoutsd

# The WaveOut sound driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  SNDWOS=$(OUTDLL)sndwaveout$(DLL)
  DEP.WOS+=$(CSUTIL.LIB) $(CSSYS.LIB)
  LDFLAGS.WOS=$(LIBS.SOUND.SYSTEM)
  TO_INSTALL.DYNAMIC_LIBS+=$(SNDWOS)
else
  SNDWOS=$(OUT)$(LIB_PREFIX)sndwaveout.a
  DEP.EXE+=$(SNDWOS)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_SNDWOS
  TO_INSTALL.STATIC_LIBS+=$(SNDWOS)
endif
DESCRIPTION.$(SNDWOS) = $(DESCRIPTION.wos)
SRC.SNDWOS = $(wildcard plugins/sound/driver/waveoutsd/*.cpp)
OBJ.SNDWOS = $(addprefix $(OUT),$(notdir $(SRC.SNDWOS:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: wos wosclean

# Chain rules
clean: wosclean

wos: $(OUTDIRS) $(SNDWOS)

$(SNDWOS): $(OBJ.SNDWOS) $(DEP.WOS)
	$(DO.PLUGIN) $(LDFLAGS.WOS)

wosclean:
	$(RM) $(SNDWOS) $(OBJ.SNDWOS) $(OUTOS)waveoutsd.dep

ifdef DO_DEPEND
dep: $(OUTOS)wos.dep
$(OUTOS)waveoutsd.dep: $(SRC.SNDWOS)
	$(DO.DEP)
else
-include $(OUTOS)wos.dep
endif

endif # ifeq ($(MAKESECTION),targets)
