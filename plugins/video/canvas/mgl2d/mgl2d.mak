# This is a subinclude file used to define the rules needed
# to build the MGL 2D driver -- mgl2d

# Driver description
DESCRIPTION.mgl2d = Crystal Space MGL 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make mgl2d        Make the $(DESCRIPTION.mgl2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: mgl2d mgl2dclean
all plugins drivers drivers2d: mgl2d

mgl2d:
	$(MAKE_TARGET) MAKE_DLL=yes
mgl2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# We need also the MGL libs
ifdef MGL_PATH
  CFLAGS.MGL2D += -I$(MGL_PATH)/include
  LIB.MGL2D.SYSTEM += -L$(MGL_PATH)/lib
endif

LIB.MGL2D.SYSTEM += -lgm -lmgl -lpm

ifeq ($(USE_PLUGINS),yes)
  MGL2D = $(OUTDLL)mgl2d$(DLL)
  LIB.MGL2D = $(foreach d,$(DEP.MGL2D),$($d.LIB))
  LIB.MGL2D.SPECIAL = $(LIB.MGL2D.SYSTEM)
  TO_INSTALL.DYNAMIC_LIBS += $(MGL2D)
else
  MGL2D = $(OUT)$(LIB_PREFIX)mgl2d$(LIB)
  DEP.EXE += $(MGL2D)
  LIBS.EXE += $(LIB.MGL2D.SYSTEM)
  SCF.STATIC += mgl2d
  TO_INSTALL.STATIC_LIBS += $(MGL2D)
endif

INC.MGL2D = $(wildcard plugins/video/canvas/mgl2d/*.h   $(INC.COMMON.DRV2D))
SRC.MGL2D = $(wildcard plugins/video/canvas/mgl2d/*.cpp $(SRC.COMMON.DRV2D)) \
  plugins/video/canvas/common/pc-keys.cpp
OBJ.MGL2D = $(addprefix $(OUT),$(notdir $(SRC.MGL2D:.cpp=$O)))
DEP.MGL2D = CSUTIL

#MSVC.DSP += MGL2D
#DSP.MGL2D.NAME = mgl2d
#DSP.MGL2D.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: mgl2d mgl2dclean

mgl2d: $(OUTDIRS) $(MGL2D)

$(OUT)%$O: plugins/video/canvas/mgl2d/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.MGL2D)
 
$(MGL2D): $(OBJ.MGL2D) $(LIB.MGL2D)
	$(DO.PLUGIN) $(LIB.MGL2D.SPECIAL)

clean: mgl2dclean
mgl2dclean:
	$(RM) $(MGL2D) $(OBJ.MGL2D)

ifdef DO_DEPEND
dep: $(OUTOS)mgl2d.dep
$(OUTOS)mgl2d.dep: $(SRC.MGL2D)
	$(DO.DEP1) $(CFLAGS.MGL2D) $(DO.DEP2)
else
-include $(OUTOS)mgl2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
