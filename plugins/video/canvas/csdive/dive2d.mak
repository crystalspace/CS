# This is a subinclude file used to define the rules needed
# to build the OS/2 DIVE 2D driver

# Driver description
DESCRIPTION.csdive = Crystal Space OS/2 DIVE driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRVHELP += $(NEWLINE)echo $"  make csdive       Make the $(DESCRIPTION.csdive)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csdive

all drivers drivers2d: csdive

csdive:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# Resource file for DIVE library
RES=$(OUTOS)libDIVE.res

# Additional library which contains functions undefined in os2.a
CSOS2.LIB=$(OUT)$(LIB_PREFIX)csos2$(LIB)

# The 2D OS/2 DIVE driver
ifeq ($(USE_DLL),yes)
  CSDIVE=csdive$(DLL)
  DEP.CSDIVE=$(RES) $(CSOS2.LIB) $(CSCOM.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  CSDIVE=$(OUT)$(LIB_PREFIX)csdive$(LIB)
  DEP.EXE+=$(RES) $(CSDIVE) $(CSOS2.LIB)
  CFLAGS.STATIC_COM+=$(CFLAGS.D)SCL_DIVE2D
endif
DESCRIPTION.$(CSDIVE)=$(DESCRIPTION.csdive)
SRC.CSDIVE = $(wildcard libs/cs2d/csdive/*.cpp $(SRC.COMMON.DRV2D))
OBJ.CSDIVE = $(addprefix $(OUT),$(notdir $(SRC.CSDIVE:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp libs/cs2d/csdive

.PHONY: csdive os2clean os2cleanlib

# Chain rules
clean: dive2dclean
cleanlib: dive2dcleanlib

csdive: $(OUTDIRS) $(CSDIVE)

$(CSDIVE): $(OBJ.CSDIVE) $(DEP.CSDIVE)
	$(DO.LIBRARY)

$(CSOS2.LIB): libs/cs2d/csdive/csdive.imp
	emximp -o $@ $<

$(RES): libs/cs2d/csdive/libDIVE.rc
	$(RC) $(RCFLAGS) $< $@

dive2dclean:
	$(RM) $(CSDIVE)

dive2dcleanlib:
	$(RM) $(OBJ.CSDIVE) $(CSDIVE)

ifdef DO_DEPEND
$(OUTOS)csdive.dep: $(SRC.CSDIVE)
	$(DO.DEP)
endif

-include $(OUTOS)csdive.dep

endif # ifeq ($(MAKESECTION),targets)
