all : build

TOP=../../..

DEPS = \
	$(TOP)/include/ivaria/cspace.i \
	$(TOP)/include/ivaria/javapre.i \
	$(TOP)/include/ivaria/javapost.i

LIBS = \
	$(TOP)/out/linuxx86/optimize/libs/cstool/libcstool.a \
	$(TOP)/out/linuxx86/optimize/libs/csgfx/libcsgfx.a \
	$(TOP)/out/linuxx86/optimize/libs/csgeom/libcsgeom.a \
	$(TOP)/out/linuxx86/optimize/libs/csutil/libcsutil.a

JAVA_SOURCES = \
    src/net/sourceforge/crystal/CS.java \
    src/net/sourceforge/crystal/csJEventHandler.java \
    src/net/sourceforge/crystal/examples/SimpleRoom.java

all : build

paths :
	@-mkdir -p src/net/sourceforge/crystal/examples

csjava.cpp : $(DEPS)
	cd src ; swig -c++ -java -package 'net.sourceforge.crystal' -I$(TOP)/../include -o csjava.cpp $(TOP)/../include/ivaria/cspace.i
	mv src/*.java src/net/sourceforge/crystal/
	mv src/csjava.cpp .

src/net/sourceforge/crystal/CS.java : paths net/sourceforge/crystal/CS.java
	cp -a net/sourceforge/crystal/CS.java src/net/sourceforge/crystal/

src/net/sourceforge/crystal/csJEventHandler.java : paths net/sourceforge/crystal/csJEventHandler.java
	cp -a net/sourceforge/crystal/csJEventHandler.java src/net/sourceforge/crystal/

src/net/sourceforge/crystal/examples/SimpleRoom.java : paths net/sourceforge/crystal/examples/SimpleRoom.java
	cp -a net/sourceforge/crystal/examples/SimpleRoom.java src/net/sourceforge/crystal/examples/

csjava.o : csjava.cpp
	g++ -I$(TOP)/include -I /usr/java/j2sdk/include/ -I /usr/java/j2sdk/include/linux/ -c csjava.cpp

javamod.o : javamod.cpp
	g++ -I$(TOP)/include -I /usr/java/j2sdk/include/ -I /usr/java/j2sdk/include/linux/ -c javamod.cpp

swig : csjava.cpp

libcsjava.so : csjava.o javamod.o
	g++ -shared -o $(@) csjava.o javamod.o $(LIBS) -lbfd -liberty

build : libcsjava.so crystalspace.jar

jar : crystalspace.jar

crystalspace.jar : $(JAVA_SOURCES) csjava.cpp
	ant build

clean :
	rm -fr src/
	rm -f csjava.cpp csjava.o javamod.o libcsjava.so
	ant clean

