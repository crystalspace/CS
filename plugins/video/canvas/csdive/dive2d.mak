# This is a subinclude file used to define the rules needed
# to build the OS/2 DIVE 2D driver

# Driver description
DESCRIPTION.csdive = Crystal Space OS/2 DIVE driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make csdive       Make the $(DESCRIPTION.csdive)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csdive csdiveclean
all plugins drivers drivers2d: csdive

csdive:
	$(MAKE_TARGET) MAKE_DLL=yes
csdiveclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/canvas/csdive

# Resource file for DIVE library
DIVE2D.RES = $(OUTOS)libDIVE.res

# Additional library which contains functions undefined in os2.a
CSOS2.LIB = $(OUT)$(LIB_PREFIX)csos2$(LIB)

# The 2D OS/2 DIVE driver
ifeq ($(USE_PLUGINS),yes)
  CSDIVE = $(OUTDLL)csdive$(DLL)
  LIB.CSDIVE = $(foreach d,$(DEP.CSDIVE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSDIVE)
else
  CSDIVE = $(OUT)$(LIB_PREFIX)csdive$(LIB)
  DEP.EXE += $(DIVE2D.RES) $(CSDIVE) $(CSOS2.LIB)
  SCF.STATIC += csdive
  TO_INSTALL.STATIC_LIBS += $(CSDIVE)
endif

INC.CSDIVE = $(wildcard plugins/video/canvas/csdive/*.h $(INC.COMMON.DRV2D))
SRC.CSDIVE = $(wildcard plugins/video/canvas/csdive/*.cpp \
  plugins/video/canvas/common/pc-keys.cpp $(SRC.COMMON.DRV2D))
OBJ.CSDIVE = $(addprefix $(OUT),$(notdir $(SRC.CSDIVE:.cpp=$O)))
DEP.CSDIVE = CSUTIL CSSYS CSUTIL

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csdive csdiveclean

csdive: $(OUTDIRS) $(CSDIVE)

$(CSDIVE): $(OBJ.CSDIVE) $(LIB.CSDIVE) $(DIVE2D.RES) $(CSOS2.LIB) 
	$(DO.PLUGIN)

$(CSOS2.LIB): plugins/video/canvas/csdive/csdive.imp
	emximp -o $@ $<

$(DIVE2D.RES): plugins/video/canvas/csdive/libDIVE.rc
	$(RC) $(RCFLAGS) $< $@

clean: csdiveclean
csdiveclean:
	$(RM) $(CSDIVE) $(OBJ.CSDIVE)

ifdef DO_DEPEND
dep: $(OUTOS)csdive.dep
$(OUTOS)csdive.dep: $(SRC.CSDIVE)
	$(DO.DEP)
else
-include $(OUTOS)csdive.dep
endif

endif # ifeq ($(MAKESECTION),targets)
