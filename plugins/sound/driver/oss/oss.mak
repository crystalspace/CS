# This is a subinclude file used to define the rules needed
# to build the OpenSoundSystem driver -- ossdrv

# Driver description
DESCRIPTION.oss = Crystal Space OSS sound driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += $(NEWLINE)echo $"  make oss          Make the $(DESCRIPTION.oss)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: oss

all plugins drivers snddrivers: oss

oss:
	$(MAKE_TARGET) MAKE_DLL=yes
ossclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/cssnddrv/oss

# The OSS sound driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  SNDOSS=$(OUTDLL)ossdrv$(DLL)
  DEP.OSS+=$(CSUTIL.LIB) $(CSSYS.LIB)
else
  SNDOSS=$(OUT)$(LIB_PREFIX)ossdrv.a
  DEP.EXE+=$(SNDOSS)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_SNDOSS
endif
DESCRIPTION.$(SNDOSS) = $(DESCRIPTION.oss)
SRC.SNDOSS = $(wildcard libs/cssnddrv/oss/*.cpp)
OBJ.SNDOSS = $(addprefix $(OUT),$(notdir $(SRC.SNDOSS:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: oss ossclean

# Chain rules
clean: ossclean

oss: $(OUTDIRS) $(SNDOSS)

$(SNDOSS): $(OBJ.SNDOSS) $(DEP.OSS)
	$(DO.PLUGIN)

ossclean:
	$(RM) $(SNDOSS) $(OBJ.SNDOSS)

ifdef DO_DEPEND
depend: $(OUTOS)oss.dep
$(OUTOS)oss.dep: $(SRC.SNDOSS)
	$(DO.DEP)
else
-include $(OUTOS)oss.dep
endif

endif # ifeq ($(MAKESECTION),targets)
