# This is a subinclude file used to define the rules needed
# to build the Glide 2.x 2D driver common functionality

# Driver description
DESCRIPTION.glide2common2d = Crystal Space Glide 2.x 2D driver basic functions

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)


endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)


endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# Local CFLAGS and libraries
CFLAGS.GLIDE2COMMON2D+=-I/boot/develop/headers/3dfx/glide2
LIBS._GLIDE2COMMON2D+=-lglide2x  

# The 2D modules
DESCRIPTION.$(GLIDE2COMMON2D) = $(DESCRIPTION.glide2common2d)
SRC.GLIDE2COMMON2D = $(wildcard libs/cs2d/glide2common/*.cpp)
OBJ.GLIDE2COMMON2D = $(addprefix $(OUT),$(notdir $(SRC.GLIDE2COMMON2D:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY:  glide2common2dclean

# Chain rules
clean: glide2common2dclean


$(OUT)%$O: libs/cs2d/glide2common/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GLIDE2COMMON2D)
 
glide2common2dclean:
	$(RM) $(GLIDEBE2D)

ifdef DO_DEPEND
$(OUTOS)glide2common2d.dep: $(SRC.GLIDE2COMMON2D)
	$(DO.DEP)
endif

-include $(OUTOS)glide2common2d.dep


endif
