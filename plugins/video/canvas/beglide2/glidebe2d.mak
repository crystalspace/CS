# This is a subinclude file used to define the rules needed
# to build the Glide 2D driver -- glidebe2d

# Driver description
DESCRIPTION.glidebe2d = Crystal Space BeOS/Glide 2D driver

include libs/cs2d/glide2common/glide2common2d.mak

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRVHELP += $(NEWLINE)echo $"  make glidebe2d    Make the $(DESCRIPTION.glidebe2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glidebe2d

all drivers drivers2d: glidebe2d

glidebe2d:
	$(MAKE_TARGET) MAKE_DLL=yes
glidebe2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# local CFLAGS
CFLAGS.GLIDEBE2D+=-I/boot/develop/headers/3dfx/glide2
LIBS._GLIDEBE2D+=/boot/develop/lib/x86/glide2x.so

# The 2D BeOS/Glide driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  GLIDEBE2D=$(OUTDLL)glidebe2d$(DLL)
  LIBS.GLIDEBE2D=$(LIBS._GLIDEBE2D)
else
  GLIDEBE2D=$(OUT)$(LIB_PREFIX)glidebe2d$(LIB)
  DEP.EXE+=$(GLIDEBE2D)
  LIBS.EXE+=$(LIBS._GLIDEBE2D)
  CFLAGS.STATIC_COM+=$(CFLAGS.D)SCL_GLIDEBE2D
endif
DESCRIPTION.$(GLIDEBE2D) = $(DESCRIPTION.glidebe2d)
SRC.GLIDEBE2D = $(wildcard libs/cs2d/beglide2/*.cpp \
	$(SRC.COMMON.DRV2D.GLIDE) $(SRC.COMMON.DRV2D))
OBJ.GLIDEBE2D = $(addprefix $(OUT),$(notdir $(SRC.GLIDEBE2D:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glidebe2d glidebeclean

# Chain rules
clean: glidebeclean

glidebe2d: $(OUTDIRS) $(GLIDEBE2D)

$(OUT)%$O: libs/cs2d/beglide2/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GLIDEBE2D)
 
$(GLIDEBE2D): $(OBJ.GLIDEBE2D) $(CSCOM.LIB) $(CSGEOM.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
	$(DO.PLUGIN) $(LIBS.GLIDEBE2D)

glidebeclean:
	$(RM) $(GLIDEBE2D) $(OBJ.GLIDEBE2D)

ifdef DO_DEPEND
depend: $(OUTOS)glidebe2d.dep
$(OUTOS)glidebe2d.dep: $(SRC.GLIDEBE2D)
	$(DO.DEP)
else
-include $(OUTOS)glidebe2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
