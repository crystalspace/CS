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
  SNDOSS = $(OUTDLL)/ossdrv$(DLL)
  LIB.SNDOSS = $(foreach d,$(DEP.SNDOSS),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SNDOSS)
else
  SNDOSS = $(OUT)/$(LIB_PREFIX)ossdrv.a
  DEP.EXE += $(SNDOSS)
  SCF.STATIC += ossdrv
  TO_INSTALL.STATIC_LIBS += $(SNDOSS)
endif

SRC.SNDOSS = $(wildcard $(addprefix $(SRCDIR)/,plugins/sound/driver/oss/*.cpp))
OBJ.SNDOSS = $(addprefix $(OUT)/,$(notdir $(SRC.SNDOSS:.cpp=$O)))
DEP.SNDOSS = CSUTIL CSSYS CSUTIL

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: ossdrv ossdrvclean

ossdrv: $(OUTDIRS) $(SNDOSS)

$(SNDOSS): $(OBJ.SNDOSS) $(LIB.SNDOSS)
	$(DO.PLUGIN)

clean: ossdrvclean
ossdrvclean:
	$(RM) $(SNDOSS) $(OBJ.SNDOSS)

ifdef DO_DEPEND
dep: $(OUTOS)/oss.dep
$(OUTOS)/oss.dep: $(SRC.SNDOSS)
	$(DO.DEP)
else
-include $(OUTOS)/oss.dep
endif

endif # ifeq ($(MAKESECTION),targets)
