# This is a subinclude file used to define the rules needed
# to build the OS/2 DIVE 2D driver

# Driver description
DESCRIPTION.csdive = Crystal Space OS/2 DIVE driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += $(NEWLINE)echo $"  make csdive       Make the $(DESCRIPTION.csdive)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csdive

all plugins drivers drivers2d: csdive

csdive:
	$(MAKE_TARGET) MAKE_DLL=yes
csdiveclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# Resource file for DIVE library
DIVE2D.RES=$(OUTOS)libDIVE.res

# Additional library which contains functions undefined in os2.a
CSOS2.LIB=$(OUT)$(LIB_PREFIX)csos2$(LIB)

# The 2D OS/2 DIVE driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  CSDIVE=csdive$(DLL)
  DEP.CSDIVE=$(DIVE2D.RES) $(CSOS2.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  CSDIVE=$(OUT)$(LIB_PREFIX)csdive$(LIB)
  DEP.EXE+=$(DIVE2D.RES) $(CSDIVE) $(CSOS2.LIB)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_DIVE2D
endif
DESCRIPTION.$(CSDIVE)=$(DESCRIPTION.csdive)
SRC.CSDIVE = $(wildcard libs/cs2d/csdive/*.cpp $(SRC.COMMON.DRV2D))
OBJ.CSDIVE = $(addprefix $(OUT),$(notdir $(SRC.CSDIVE:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp libs/cs2d/csdive

.PHONY: csdive csdiveclean

# Chain rules
clean: csdiveclean

csdive: $(OUTDIRS) $(CSDIVE)

$(CSDIVE): $(OBJ.CSDIVE) $(DEP.CSDIVE)
	$(DO.PLUGIN)

$(CSOS2.LIB): libs/cs2d/csdive/csdive.imp
	emximp -o $@ $<

$(DIVE2D.RES): libs/cs2d/csdive/libDIVE.rc
	$(RC) $(RCFLAGS) $< $@

csdiveclean:
	$(RM) $(CSDIVE) $(OBJ.CSDIVE)

ifdef DO_DEPEND
depend: $(OUTOS)csdive.dep
$(OUTOS)csdive.dep: $(SRC.CSDIVE)
	$(DO.DEP)
else
-include $(OUTOS)csdive.dep
endif

endif # ifeq ($(MAKESECTION),targets)
