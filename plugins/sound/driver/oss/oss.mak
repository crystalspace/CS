# This is a subinclude file used to define the rules needed
# to build the OpenSoundSystem driver -- ossdrv

# Driver description
DESCRIPTION.ossdrv = Crystal Space OSS sound driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make ossdrv       Make the $(DESCRIPTION.ossdrv)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: ossdrv ossdrvclean
all plugins drivers snddrivers: ossdrv

ossdrv:
	$(MAKE_TARGET) MAKE_DLL=yes
ossdrvclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/sound/driver/oss

# The OSS sound driver
ifeq ($(USE_PLUGINS),yes)
  OSSDRV = $(OUTDLL)/ossdrv$(DLL)
  LIB.OSSDRV = $(foreach d,$(DEP.OSSDRV),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(OSSDRV)
else
  OSSDRV = $(OUT)/$(LIB_PREFIX)ossdrv.a
  DEP.EXE += $(OSSDRV)
  SCF.STATIC += ossdrv
  TO_INSTALL.STATIC_LIBS += $(OSSDRV)
endif

INF.OSSDRV = $(SRCDIR)/plugins/sound/driver/oss/ossdrv.csplugin
INC.OSSDRV = $(wildcard $(addprefix $(SRCDIR)/,plugins/sound/driver/oss/*.h))
SRC.OSSDRV = $(wildcard $(addprefix $(SRCDIR)/,plugins/sound/driver/oss/*.cpp))
OBJ.OSSDRV = $(addprefix $(OUT)/,$(notdir $(SRC.OSSDRV:.cpp=$O)))
DEP.OSSDRV = CSUTIL CSUTIL

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: ossdrv ossdrvclean

ossdrv: $(OUTDIRS) $(OSSDRV)

$(OSSDRV): $(OBJ.OSSDRV) $(LIB.OSSDRV)
	$(DO.PLUGIN)

clean: ossdrvclean
ossdrvclean:
	-$(RMDIR) $(OSSDRV) $(OBJ.OSSDRV) $(OUTDLL)/$(notdir $(INF.OSSDRV))

ifdef DO_DEPEND
dep: $(OUTOS)/oss.dep
$(OUTOS)/oss.dep: $(SRC.OSSDRV)
	$(DO.DEP)
else
-include $(OUTOS)/oss.dep
endif

endif # ifeq ($(MAKESECTION),targets)
