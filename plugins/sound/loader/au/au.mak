# Plug-in description
DESCRIPTION.au = Crystal Space au sound loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make au           Make the $(DESCRIPTION.au)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: au auclean
all plugins drivers snddrivers: au

au:
	$(MAKE_TARGET) MAKE_DLL=yes
auclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/loader/au plugins/sound/loader/common

ifeq ($(USE_PLUGINS),yes)
  AU = $(OUTDLL)sndau$(DLL)
  LIB.AU = $(foreach d,$(DEP.AU),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(AU)
else
  AU = $(OUT)$(LIB_PREFIX)sndau$(LIB)
  DEP.EXE += $(AU)
  SCF.STATIC += sndau
  TO_INSTALL.STATIC_LIBS += $(AU)
endif

INC.AU = $(wildcard plugins/sound/loader/au/*.h) \
  $(wildcard plugins/sound/loader/common/*.h)
SRC.AU = $(wildcard plugins/sound/loader/au/*.cpp) \
  $(wildcard plugins/sound/loader/common/*.cpp)
OBJ.AU = $(addprefix $(OUT),$(notdir $(SRC.AU:.cpp=$O)))
DEP.AU = CSUTIL CSSYS CSUTIL

MSVC.DSP += AU
DSP.AU.NAME = sndau
DSP.AU.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: au auclean

au: $(OUTDIRS) $(AU)

$(AU): $(OBJ.AU) $(LIB.AU)
	$(DO.PLUGIN)

clean: auclean
auclean:
	$(RM) $(AU) $(OBJ.AU)

ifdef DO_DEPEND
dep: $(OUTOS)sndau.dep
$(OUTOS)sndau.dep: $(SRC.AU)
	$(DO.DEP)
else
-include $(OUTOS)sndau.dep
endif

endif # ifeq ($(MAKESECTION),targets)
