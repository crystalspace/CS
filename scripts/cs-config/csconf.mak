# Target description
DESCRIPTION.csconfig = cs-config compile help script

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PSEUDOHELP += \
  $(NEWLINE)echo $"  make csconfig     Build the $(DESCRIPTION.csconfig)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csconfig csconfigclean

csconfig:
	$(MAKE_TARGET)
csconfigclean:
	$(MAKE_CLEAN)

endif

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CSCONFIG.EXE = cs-config

TO_INSTALL.EXE	+= $(CSCONFIG.EXE)

endif

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csconfig csconfigclean $(CSCONFIG.EXE)

all: $(CSCONFIG.EXE)
csconfig: $(OUTDIRS) $(CSCONFIG.EXE)
clean: csconfigclean

$(CSCONFIG.EXE):
	@echo Generating cs-config script...
	scripts/cs-config/genscript.sh "$(INSTALL_DIR)" "$(CXXFLAGS)" "$(CFLAGS)" "$(LIBS.EXE)" scripts/cs-config

csconfigclean:
	-$(RM) $(CSCONFIG.EXE)

endif

