# This is a subinclude file used to define the rules needed
# to build the Ascii Art 2D driver using libaa

# Driver description
DESCRIPTION.asciiart = Crystal Space Ascii Art driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make asciiart     Make the $(DESCRIPTION.asciiart)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: asciiart asciiartclean
all plugins drivers drivers2d: asciiart

asciiart:
	$(MAKE_TARGET) MAKE_DLL=yes
asciiartclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# The AsciiArt library
LIB.ASCIIART.SYSTEM += $(LFLAGS.l)aa
# On Unix we need additional libraries
ifdef X11_PATH
  LIB.ASCIIART.SYSTEM += \
    -lgpm -lslang -L$(X11_PATH)/lib -lXext -lX11 $(X11_EXTRA_LIBS)
endif

# The 2D Ascii Art driver
ifeq ($(USE_PLUGINS),yes)
  ASCIIART = $(OUTDLL)asciiart$(DLL)
  LIB.ASCIIART = $(foreach d,$(DEP.ASCIIART),$($d.LIB))
  LIB.ASCIIART.SPECIAL = $(LIB.ASCIIART.SYSTEM)
  TO_INSTALL.DYNAMIC_LIBS += $(ASCIIART)
else
  ASCIIART = $(OUT)$(LIB_PREFIX)asciiart$(LIB)
  DEP.EXE += $(ASCIIART)
  LIBS.EXE += $(LIB.ASCIIART.SYSTEM)
  SCF.STATIC += asciiart
  TO_INSTALL.STATIC_LIBS += $(ASCIIART)
endif

INC.ASCIIART = $(wildcard plugins/video/canvas/asciiart/*.h \
  $(INC.COMMON.DRV2D))
SRC.ASCIIART = $(wildcard plugins/video/canvas/asciiart/*.cpp \
  $(SRC.COMMON.DRV2D))
OBJ.ASCIIART = $(addprefix $(OUT),$(notdir $(SRC.ASCIIART:.cpp=$O)))
DEP.ASCIIART = CSUTIL CSSYS CSUTIL
CFG.ASCIIART = data/config/asciiart.cfg

TO_INSTALL.CONFIG += $(CFG.ASCIIART)

#MSVC.DSP += ASCIIART
#DSP.ASCIIART.NAME = asciiart
#DSP.ASCIIART.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp plugins/video/canvas/asciiart

.PHONY: asciiart asciiartclean
clean: asciiartclean

asciiart: $(OUTDIRS) $(ASCIIART)

$(ASCIIART): $(OBJ.ASCIIART) $(LIB.ASCIIART)
	$(DO.PLUGIN) $(LIB.ASCIIART.SPECIAL)

asciiartclean:
	$(RM) $(ASCIIART) $(OBJ.ASCIIART)

ifdef DO_DEPEND
dep: $(OUTOS)asciiart.dep
$(OUTOS)asciiart.dep: $(SRC.ASCIIART)
	$(DO.DEP)
else
-include $(OUTOS)asciiart.dep
endif

endif # ifeq ($(MAKESECTION),targets)
