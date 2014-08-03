OUT        = freetron
SRC        = ${wildcard *.cpp}
OBJ        = ${SRC:.cpp=.o}
DEPENDS    = .depends
WEBSRC     = ${wildcard website/*.cpp}
WEBOBJ     = ${WEBSRC:.cpp=.o}
SKIN       = website/skin.cpp
SKINOBJ    = ${SKIN:.cpp=.o}
SKINSRC    = ${wildcard website/*.tmpl}

CXXFLAGS  += -ffast-math -funroll-loops -std=c++11
LDFLAGS   += -lpodofo -lIL -ltiff -ltiffxx -pthread -lcppcms -lbooster -lcppdb

TMPLCC    ?= cppcms_tmpl_cc
PREFIX    ?= /usr/local
MANPREFIX ?= ${PREFIX}/share/man

all: CXXFLAGS += -O2 -Os
all: ${OUT}

debug: CXXFLAGS += -g -Wall -Wextra -Wpedantic
debug: ${OUT}

${OUT}: ${OBJ} ${WEBOBJ} ${SKINOBJ}
	${CXX} -o $@ ${OBJ} ${WEBOBJ} ${SKINOBJ} ${LDFLAGS}

.cpp.o:
	${CXX} -c -o $@ $< ${CXXFLAGS}

${DEPENDS}: ${SRC} ${WEBSRC}
	${RM} -f ./${DEPENDS}
	${CXX} ${CXXFLAGS} -MM $^ >> ./${DEPENDS}

${SKIN}: website/master.tmpl ${SKINSRC}
	${TMPLCC} $^ -o ${SKIN}

depends: ${DEPENDS}

install:
	install -Dm755 ${OUT} ${DESTDIR}${PREFIX}/bin/freetron
    
uninstall:
	${RM} -f ${DESTDIR}${PREFIX}/bin/freetron

clean:
	${RM} ${OUT} ${OBJ} ${DEPENDS} ${SKIN} ${SKINOBJ} ${WEBOBJ}
	${RM} -r cmake/CMakeFiles cmake/CMakeCache.txt cmake/cmake_install.cmake cmake/Makefile cmake/freetron

-include ${DEPENDS}
.PHONY: all debug depends install uninstall clean
