all : build

TOP=../../..

JAVASDK = $(HOME)/j2sdk1.4.2

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
    src/com/crystalspace/CS.java \
    src/com/crystalspace/csJEventHandler.java \
    src/com/crystalspace/examples/SimpleRoom.java

all : build

paths :
	@-mkdir -p src/com/crystalspace/examples

csjava.cpp : $(DEPS)
	cd src ; swig $(SWIG.FLAGS) -c++ -java -package 'com.crystalspace' -I$(TOP)/../include -o csjava.cpp $(TOP)/../include/ivaria/cspace.i
	mv src/*.java src/com/crystalspace/
	cp src/csjava.cpp .

src/com/crystalspace/CS.java : com/crystalspace/CS.java
	cp -a com/crystalspace/CS.java src/com/crystalspace/

src/com/crystalspace/csJEventHandler.java : com/crystalspace/csJEventHandler.java
	cp -a com/crystalspace/csJEventHandler.java src/com/crystalspace/

src/com/crystalspace/examples/SimpleRoom.java : com/crystalspace/examples/SimpleRoom.java
	cp -a com/crystalspace/examples/SimpleRoom.java src/com/crystalspace/examples/

csjava.o : csjava.cpp
	g++ -I$(TOP)/include -I $(JAVASDK)/include/ -I $(JAVASDK)/include/linux/ -c csjava.cpp

javamod.o : javamod.cpp
	g++ -I$(TOP)/include -I $(JAVASDK)/include/ -I $(JAVASDK)/include/linux/ -c javamod.cpp

swig : csjava.cpp

libcsjava.so : csjava.o javamod.o
	g++ -shared -o $(@) csjava.o javamod.o $(LIBS) -lbfd -liberty

build : paths libcsjava.so crystalspace.jar

jar : crystalspace.jar

crystalspace.jar : $(JAVA_SOURCES) csjava.cpp
	ant -emacs -Dbuild.compiler.emacs=true

clean :
	rm -fr src
	rm -f csjava.cpp csjava.o javamod.o libcsjava.so
	ant clean
	rm -fr build
