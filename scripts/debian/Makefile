BUILDDIR=$(HOME)/debian/crystal
DATE=$(shell date '+%Y%m%d')

default:
	@echo "=============================================================="
	@echo "You can build from your local copy"
	@echo "Typing:"
	@echo "	make fromlocal"
	@echo "Or from a CVS export (edit DATE in Makefile if you want a particular date)"
	@echo "Typing:"
	@echo "	make fromcvs"
	@echo "=============================================================="

# BUILD FROM CVS EXPORT
fromcvs: start cvsexport debfiles debbuild
# BUILD FROM LOCAL COPY
# just binaries
justbin: start copy debfiles debcopy debclean debconfigure debbinary
# don't rebuild all
retry: start copy debfiles debcopy debconfigure debbinary
# full with debuild
fromlocal: start copy debfiles debcopy debbuild

start:
	@echo "Building Crystalspace-$(DATE)"

# Just to copy from local files
copy: $(BUILDDIR) $(BUILDDIR)/copy-stamp
$(BUILDDIR)/copy-stamp:
	echo "Copying files"
	cd ../../..; find CS | grep -v CVS | cpio -pdumvB $(BUILDDIR)/crystalspace-$(DATE)
	touch $(BUILDDIR)/copy-stamp

# To anonymous login, do it only one time is needed
cvslogin:
	echo "Login crystalspace CVS archive as anonymous"
	echo "Password is <Enter>"
	cvs -d:pserver:anonymous@cvs.crystal.sourceforge.net:/cvsroot/crystal login

# If you want to export a special date
cvsexport: $(BUILDDIR) $(BUILDDIR)/cvsexport-stamp
$(BUILDDIR)/cvsexport-stamp: 
	echo "Exporting CVS"
	cd $(BUILDDIR) && cvs -d:pserver:anonymous@cvs.crystal.sourceforge.net:/cvsroot/crystal export -d crystalspace-$(DATE) -D $(DATE) crystal
	touch $(BUILDDIR)/cvsexport-stamp

# Prepare debian files
debfiles: $(BUILDDIR)/debfile-stamp
$(BUILDDIR)/debfile-stamp:
	echo "Moving debian files"
	mv $(BUILDDIR)/crystalspace-$(DATE)/CS/scripts/debian/files $(BUILDDIR)/crystalspace-$(DATE)/debian
	touch $(BUILDDIR)/debfile-stamp

debcopy:
	cd files ; find . | cpio -pdumvB $(BUILDDIR)/crystalspace-$(DATE)/debian

debclean:
	cd $(BUILDDIR)/crystalspace-$(DATE); fakeroot debian/rules clean
	
debconfigure:
	cd $(BUILDDIR)/crystalspace-$(DATE); fakeroot debian/rules configure

debbinary:
	cd $(BUILDDIR)/crystalspace-$(DATE);dch -v $(DATE)-1 Autobuild ; fakeroot debian/rules binary

debbuild:
	cd $(BUILDDIR)/crystalspace-$(DATE);dch -v $(DATE)-1 Autobuild ; debuild -us -uc

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -f $(BUILDDIR)/copy-stamp $(BUILDDIR)/cvsexport-stamp
