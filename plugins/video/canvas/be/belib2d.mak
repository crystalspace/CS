# This is a subinclude file used to define the rules needed
# to build the BeOS 2D driver -- be2d

# Driver description
DESCRIPTION.be2d = Crystal Space BeLib 2D driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRVHELP += $(NEWLINE)echo $"  make be2d        Make the $(DESCRIPTION.be2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: be2d

ifeq ($(USE_DLL),yes)
all drivers drivers2d: be2d
endif

be2d:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/cs2d/be

# The 2D Belib driver
ifeq ($(USE_DLL),yes)
  BE2D=$(OUTDLL)be2d$(DLL)
  DEP.BE2D = $(CSCOM.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  BE2D=be2d.a
  DEP.EXE+=$(BE2D)
  CFLAGS.STATIC_COM+=$(CFLAGS.D)SCL_BE2D
endif
DESCRIPTION.$(BE2D) = $(DESCRIPTION.be2d)
SRC.BE2D = $(wildcard libs/cs2d/be/*.cpp $(SRC.COMMON.DRV2D))
OBJ.BE2D = $(addprefix $(OUT),$(notdir $(SRC.BE2D:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp libs/cs2d/be

.PHONY: be2d beclean becleanlib

# Chain rules
clean: beclean
cleanlib: becleanlib

be2d: $(OUTDIRS) $(BE2D)

$(BE2D): $(OBJ.BE2D) $(DEP.BE2D)
	$(DO.LIBRARY)

beclean:
	$(RM) $(BE2D)

becleanlib:
	$(RM) $(OBJ.BE2D) $(BE2D)

ifdef DO_DEPEND
$(OUTOS)be2d.dep: $(SRC.BE2D)
	$(DO.DEP) $(OUTOS)be2d.dep
endif

-include $(OUT)be2d.dep

endif # ifeq ($(MAKESECTION),targets)
