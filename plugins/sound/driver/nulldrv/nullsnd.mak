# This is a subinclude file used to define the rules needed
# to build the OpenSoundSystem driver -- ossdrv

# Driver description
DESCRIPTION.nullsnddrv = Crystal Space NULL sound driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make nullsnddrv   Make the $(DESCRIPTION.nullsnddrv)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: nullsnddrv nullsnddrvclean

all plugins drivers snddrivers: nullsnddrv

nullsnddrv:
	$(MAKE_TARGET) MAKE_DLL=yes
nullsnddrvclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/driver/nulldrv

# The NULLSNDDRV sound driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  NULLSNDDRV=$(OUTDLL)snddrv0$(DLL)
  DEP.NULLSNDDRV+=$(CSUTIL.LIB) $(CSSYS.LIB)
  TO_INSTALL.DYNAMIC_LIBS+=$(NULLSNDDRV)
else
  NULLSNDDRV=$(OUT)$(LIB_PREFIX)snddrv0.a
  DEP.EXE+=$(NULLSNDDRV)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_NULLSNDDRV
  TO_INSTALL.STATIC_LIBS+=$(NULLSNDDRV)
endif
DESCRIPTION.$(NULLSNDDRV) = $(DESCRIPTION.nullsnddrv)
SRC.NULLSNDDRV = $(wildcard plugins/sound/driver/nulldrv/*.cpp)
OBJ.NULLSNDDRV = $(addprefix $(OUT),$(notdir $(SRC.NULLSNDDRV:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: nullsnddrv nullsnddrvclean

# Chain rules
clean: nullsnddrvclean

nullsnddrv: $(OUTDIRS) $(NULLSNDDRV)

$(NULLSNDDRV): $(OBJ.NULLSNDDRV) $(DEP.NULLSNDDRV)
	$(DO.PLUGIN)

nullsnddrvclean:
	$(RM) $(NULLSNDDRV) $(OBJ.NULLSNDDRV) $(OUTOS)nullsnddrv.dep

ifdef DO_DEPEND
dep: $(OUTOS)nullsnddrv.dep
$(OUTOS)nullsnddrv.dep: $(SRC.NULLSNDDRV)
	$(DO.DEP)
else
-include $(OUTOS)nullsnddrv.dep
endif

endif # ifeq ($(MAKESECTION),targets)
