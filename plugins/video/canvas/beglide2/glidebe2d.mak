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

# local CFLAGS
CFLAGS.GLIDEBE2D+=$(GLIDE2_PATH)
LIBS._GLIDEBE2D+=/boot/develop/lib/x86/glide2x.so

# The 2D BeOS/Glide2 driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  GLIDEBE2D=$(OUTDLL)glidebe2d$(DLL)
  LIBS.GLIDEBE2D=$(LIBS._GLIDEBE2D)
  TO_INSTALL.DYNAMIC_LIBS += $(GLIDEBE2D)
else
  GLIDEBE2D=$(OUT)$(LIB_PREFIX)glidebe2d$(LIB)
  DEP.EXE+=$(GLIDEBE2D)
  LIBS.EXE+=$(LIBS._GLIDEBE2D)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_GLIDEBE2D
  TO_INSTALL.STATIC_LIBS += $(GLIDEBE2D)
endif
DESCRIPTION.$(GLIDEBE2D) = $(DESCRIPTION.glidebe2d)
SRC.GLIDEBE2D = $(wildcard plugins/video/canvas/beglide2/*.cpp \
	$(SRC.COMMON.DRV2D.GLIDE) $(SRC.COMMON.DRV2D))
OBJ.GLIDEBE2D = $(addprefix $(OUT),$(notdir $(SRC.GLIDEBE2D:.cpp=$O)))

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
 
$(GLIDEBE2D): $(OBJ.GLIDEBE2D) $(CSUTIL.LIB) $(CSSYS.LIB)
	$(DO.PLUGIN) $(LIBS.GLIDEBE2D)

glidebe2dclean:
	$(RM) $(GLIDEBE2D) $(OBJ.GLIDEBE2D) $(OUTOS)glidebe2d.dep

ifdef DO_DEPEND
dep: $(OUTOS)glidebe2d.dep
$(OUTOS)glidebe2d.dep: $(SRC.GLIDEBE2D)
	$(DO.DEP1) $(CFLAGS.GLIDEBE2D) $(DO.DEP2)
else
-include $(OUTOS)glidebe2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
