# This is a subinclude file used to define the rules needed
# to build the BeOS Glide2 2D driver -- glidebe2d

# Driver description
DESCRIPTION.glidebe2d = Crystal Space BeOS Glide2 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make glidebe2d    Make the $(DESCRIPTION.glidebe2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glidebe2d glidebe2dclean
all plugins drivers drivers2d: glidebe2d

glidebe2d:
	$(MAKE_TARGET) MAKE_DLL=yes
glidebe2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CFLAGS.GLIDEBE2D += $(GLIDE2_PATH)
LIB.GLIDEBE2D.SYSTEM += /boot/develop/lib/x86/glide2x.so

ifeq ($(USE_PLUGINS),yes)
  GLIDEBE2D = $(OUTDLL)glidebe2d$(DLL)
  LIB.GLIDEBE2D = $(foreach d,$(DEP.GLIDEBE2D),$($d.LIB))
  LIB.GLIDEBE2D.SPECIAL = $(LIB.GLIDEBE2D.SYSTEM)
  TO_INSTALL.DYNAMIC_LIBS += $(GLIDEBE2D)
else
  GLIDEBE2D = $(OUT)$(LIB_PREFIX)glidebe2d$(LIB)
  DEP.EXE += $(GLIDEBE2D)
  LIBS.EXE += $(LIB.GLIDEBE2D.SYSTEM)
  SCF.STATIC += glidebe2d
  TO_INSTALL.STATIC_LIBS += $(GLIDEBE2D)
endif

INC.GLIDEBE2D = $(wildcard plugins/video/canvas/beglide2/*.h \
	$(INC.COMMON.DRV2D.GLIDE) $(INC.COMMON.DRV2D))
SRC.GLIDEBE2D = $(wildcard plugins/video/canvas/beglide2/*.cpp \
	$(SRC.COMMON.DRV2D.GLIDE) $(SRC.COMMON.DRV2D))
OBJ.GLIDEBE2D = $(addprefix $(OUT),$(notdir $(SRC.GLIDEBE2D:.cpp=$O)))
DEP.GLIDEBE2D = CSUTIL CSSYS

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glidebe2d glidebe2dclean

# Chain rules
clean: glidebe2dclean

glidebe2d: $(OUTDIRS) $(GLIDEBE2D)

$(OUT)%$O: plugins/video/canvas/beglide2/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GLIDEBE2D)
$(OUT)%$O: plugins/video/canvas/glide2common/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GLIDEBE2D)
 
$(GLIDEBE2D): $(OBJ.GLIDEBE2D) $(LIB.GLIDEBE2D)
	$(DO.PLUGIN) $(LIB.GLIDEBE2D.SPECIAL)

glidebe2dclean:
	$(RM) $(GLIDEBE2D) $(OBJ.GLIDEBE2D)

ifdef DO_DEPEND
dep: $(OUTOS)glidebe2d.dep
$(OUTOS)glidebe2d.dep: $(SRC.GLIDEBE2D)
	$(DO.DEP1) $(CFLAGS.GLIDEBE2D) $(DO.DEP2)
else
-include $(OUTOS)glidebe2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
