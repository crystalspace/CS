# This is a subinclude file used to define the rules needed
# to build the OpenSoundSystem driver -- ossdrv

# Driver description
DESCRIPTION.oss = Crystal Space OSS sound driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make oss          Make the $(DESCRIPTION.oss)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: oss ossclean
all plugins drivers snddrivers: oss

oss:
	$(MAKE_TARGET) MAKE_DLL=yes
ossclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/driver/oss

# The OSS sound driver
ifeq ($(USE_PLUGINS),yes)
  SNDOSS = $(OUTDLL)ossdrv$(DLL)
  LIB.SNDOSS = $(foreach d,$(DEP.SNDOSS),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SNDOSS)
else
  SNDOSS = $(OUT)$(LIB_PREFIX)ossdrv.a
  DEP.EXE += $(SNDOSS)
  SCF.STATIC += ossdrv
  TO_INSTALL.STATIC_LIBS += $(SNDOSS)
endif

SRC.SNDOSS = $(wildcard plugins/sound/driver/oss/*.cpp)
OBJ.SNDOSS = $(addprefix $(OUT),$(notdir $(SRC.SNDOSS:.cpp=$O)))
DEP.SNDOSS = CSUTIL CSSYS

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: oss ossclean

oss: $(OUTDIRS) $(SNDOSS)

$(SNDOSS): $(OBJ.SNDOSS) $(LIB.SNDOSS)
	$(DO.PLUGIN)

clean: ossclean
ossclean:
	$(RM) $(SNDOSS) $(OBJ.SNDOSS)

ifdef DO_DEPEND
dep: $(OUTOS)oss.dep
$(OUTOS)oss.dep: $(SRC.SNDOSS)
	$(DO.DEP)
else
-include $(OUTOS)oss.dep
endif

endif # ifeq ($(MAKESECTION),targets)
