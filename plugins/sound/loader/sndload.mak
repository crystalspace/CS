# This is a subinclude file used to define the rules needed
# to build the software sound renderer

# Driver description
DESCRIPTION.sndload = Crystal Space sound loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make sndload      Make the $(DESCRIPTION.sndload)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sndload sndloadclean

all plugins drivers snddrivers: sndload

sndload:
	$(MAKE_TARGET) MAKE_DLL=yes
sndloadclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/loader

# The Sound loader

#
# First set up library linking...ie. the name of the runtime library to be
# used
#

ifeq ($(USE_SHARED_PLUGINS),yes)

  SNDLOAD=$(OUTDLL)sndload$(DLL)

  DEP.SNDLOAD=$(CSUTIL.LIB) $(CSSYS.LIB) $(CSSNDLDR.LIB) $(CSSFXLDR.LIB)
  LIBS.LOCAL.SNDLOAD=$(LIBS.SNDLOAD)
  TO_INSTALL.DYNAMIC_LIBS+=$(SNDLOAD)

else
# Generate static libs
  SNDLOAD=$(OUT)$(LIB_PREFIX)sndload$(LIB)
  TO_INSTALL.STATIC_LIBS+=$(SNDLOAD)
  DEP.EXE+=$(SNDLOAD)

  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_SNDLOAD
endif

TO_INSTALL.CONFIG += data/config/sound.cfg
DESCRIPTION.$(SNDLOAD) = $(DESCRIPTION.sndload)

SRC.SNDLOAD = $(wildcard plugins/sound/loader/*.cpp)
OBJ.SNDLOAD = $(addprefix $(OUT),$(notdir $(SRC.SNDLOAD:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndload sndloadclean

# Chain rules
snd: sndload
clean: sndloadclean

sndload: $(OUTDIRS) $(SNDLOAD)

$(SNDLOAD): $(OBJ.SNDLOAD) $(DEP.SNDLOAD)
	$(DO.PLUGIN)

sndloadclean:
	$(RM) $(SNDLOAD) $(OBJ.SNDLOAD) $(OUTOS)sndload.dep

ifdef DO_DEPEND
dep: $(OUTOS)sndload.dep
$(OUTOS)sndload.dep: $(SRC.SNDLOAD)
	$(DO.DEP)
else
-include $(OUTOS)sndload.dep
endif

endif # ifeq ($(MAKESECTION),targets)
