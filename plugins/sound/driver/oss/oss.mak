# This is a subinclude file used to define the rules needed
# to build the OpenSoundSystem driver -- ossdrv

# Driver description
DESCRIPTION.oss = Crystal Space OSS sound driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRVHELP += $(NEWLINE)echo $"  make oss          Make the $(DESCRIPTION.oss)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: oss

all drivers snddrivers: oss

oss:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/cssnddrv/oss

# The OSS sound driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  SNDOSS=$(OUTDLL)SoundDriverOSS$(DLL)
  DEP.OSS+=$(CSCOM.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  SNDOSS=$(OUT)$(LIB_PREFIX)SoundDriverOSS.a
  DEP.EXE+=$(SNDOSS)
  CFLAGS.STATIC_COM+=$(CFLAGS.D)SCL_SNDOSS
endif
DESCRIPTION.$(SNDOSS) = $(DESCRIPTION.oss)
SRC.SNDOSS = $(wildcard libs/cssnddrv/oss/*.cpp)
OBJ.SNDOSS = $(addprefix $(OUT),$(notdir $(SRC.SNDOSS:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: oss ossclean osscleanlib

# Chain rules
clean: ossclean
cleanlib: osscleanlib

oss: $(OUTDIRS) $(SNDOSS)

$(SNDOSS): $(OBJ.SNDOSS) $(DEP.OSS)
	$(DO.PLUGIN)

ossclean:
	$(RM) $(SNDOSS)

osscleanlib:
	$(RM) $(OBJ.SNDOSS) $(SNDOSS)

ifdef DO_DEPEND
depend: $(OUTOS)oss.dep
$(OUTOS)oss.dep: $(SRC.SNDOSS)
	$(DO.DEP)
else
-include $(OUTOS)oss.dep
endif

endif # ifeq ($(MAKESECTION),targets)
