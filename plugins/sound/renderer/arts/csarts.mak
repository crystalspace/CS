# Application description
DESCRIPTION.csarts = Crystal Space aRts sound renderer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make csarts       Make the $(DESCRIPTION.csarts)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csarts csartsclean csartsinstall csartslib

all apps: csarts 
#csartsinstall
csartsinstall csarts csartslib:
	$(MAKE_TARGET)
csartsclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp %.cc plugins/sound/renderer/arts

ARTSTEST.EXE = artstest$(EXE)
LIB.CSARTS = plugins/sound/renderer/arts/libcsarts.la
INC.ARTS = -I$(KDEDIR)/include/arts
LIB.ARTS=-L$(KDEDIR)/lib -lartsflow -lartsflow_idl -lartsmodules -lmcop -lsoundserver_idl -ldl -lstdc++
IDL.ARTSTEST = $(wildcard plugins/sound/renderer/arts/*.idl)
IDLSRC.ARTSTEST = $(IDL.ARTSTEST:.idl=.cc)
SRC.ARTSTEST = $(wildcard plugins/sound/renderer/arts/*.cpp) 
IMPLSRC.ARTSTEST = $(wildcard plugins/sound/renderer/arts/*_impl.cpp) 
MOOBJ.ARTSTEST = $(addprefix $(OUT),$(notdir $(IDLSRC.ARTSTEST:.cc=.moo)))
OBJ.ARTSTEST = $(MOOBJ.ARTSTEST) $(addprefix $(OUT),$(notdir $(SRC.ARTSTEST:.cpp=$O)))
DEP.ARTSTEST = CSGFX CSGEOM CSUTIL CSSYS CSUTIL 
LIB.ARTSTEST = $(foreach d,$(DEP.ARTSTEST),$($d.LIB))
ARTS.CXX = libtool --mode=compile g++
ARTS.LD=libtool --mode=link g++
ARTS.CP=libtool --mode=install cp
ARTS.LDFLAGS=-module -rpath $(KDEDIR)/lib -L$(KDEDIR)/lib -lartsflow -lartsflow_idl -lartsmodules -lmcop -ldl

CSARTS.LA.SRC = $(IDLSRC.ARTSTEST) $(IMPLSRC.ARTSTEST)

CSARTS.LA.OBJ = $(addprefix plugins/sound/renderer/arts/,$(notdir $(IDLSRC.ARTSTEST:.cc=.lo))) \
$(addprefix plugins/sound/renderer/arts/,$(notdir $(IMPLSRC.ARTSTEST:.cpp=.lo)))

TO_INSTALL.EXE += $(ARTSTEST.EXE)

MSVC.DSP += ARTSTEST
DSP.ARTSTEST.NAME = artstest
DSP.ARTSTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csarts csartsclean csartsinstall csartslib

csarts: $(OUTDIRS) $(ARTSTEST.EXE) $(LIB.CSARTS)
csartslib: $(LIB.CSARTS)
clean: csartsclean

$(ARTSTEST.EXE): $(OBJ.ARTSTEST) $(LIB.ARTSTEST)
	$(DO.LINK.EXE) $(LIB.ARTS)

$(LIB.CSARTS): $(CSARTS.LA.OBJ) $(LIB.ARTSTEST)
	$(ARTS.LD) -o $(LIB.CSARTS) $(ARTS.LDFLAGS) $(CSARTS.LA.OBJ)

$(OUT)%.moo: plugins/sound/renderer/arts/%.cc
	$(DO.COMPILE.CPP) $(INC.ARTS)

plugins/sound/renderer/arts/%.lo: plugins/sound/renderer/arts/%.cc
	cd plugins/sound/renderer/arts; $(ARTS.CXX) $(INC.ARTS) -c $(<F) ; cd ../../../..

plugins/sound/renderer/arts/%.lo: plugins/sound/renderer/arts/%.cpp
	cd plugins/sound/renderer/arts; $(ARTS.CXX) $(INC.ARTS) -c $(<F) ; cd ../../../..

$(OUT)%$O: plugins/sound/renderer/arts/%.cpp
	$(DO.COMPILE.CPP) $(INC.ARTS)

plugins/sound/renderer/arts/%.cc: plugins/sound/renderer/arts/%.idl
	mcopidl $(INC.ARTS) $(<<)
	mv $(basename $(@F)).* plugins/sound/renderer/arts

plugins/sound/renderer/arts/%.h: plugins/sound/renderer/arts/%.idl
	mcopidl $(INC.ARTS) $(<<)
	mv $(basename $(@F)).* plugins/sound/renderer/arts

csartsclean: 
	-$(RM) $(ARTSTEST.EXE) $(OBJ.ARTSTEST) $(CSARTS.LA.OBJ)

csartsinstall: $(LIB.CSARTS)
	$(ARTS.CP) $(LIB.CSARTS) $(KDEDIR)/lib
	$(ARTS.CP) plugins/sound/renderer/arts/csarts.mcopclass $(KDEDIR)/lib/mcop/Arts
	
ifdef DO_DEPEND
dep: $(OUTOS)csarts.dep
$(OUTOS)csarts.dep: $(SRC.ARTSTEST)
	$(DO.DEP)
else
-include $(OUTOS)csarts.dep
endif

endif # ifeq ($(MAKESECTION),targets)
