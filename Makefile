OUT        = freetron
SRC        = ${wildcard *.cpp}
OBJ        = ${SRC:.cpp=.o}
DEPENDS    = .depends

CXXFLAGS  += -ffast-math -funroll-loops -std=c++11
LDFLAGS   += -lpodofo -lIL -ltiff -ltiffxx -pthread

PREFIX    ?= /usr/local
MANPREFIX ?= ${PREFIX}/share/man

all: CXXFLAGS += -O2
all: ${OUT}

debug: CXXFLAGS += -g -Wall -Wextra -Wpedantic -DENABLE_DEBUG
debug: ${OUT}

${OUT}: ${OBJ}
	${CXX} -o $@ ${OBJ} ${LDFLAGS}

.cpp.o:
	${CXX} -c -o $@ $< ${CXXFLAGS}

${DEPENDS}: ${SRC}
	${RM} -f ./${DEPENDS}
	${CXX} ${CXXFLAGS} -MM $^ >> ./${DEPENDS}

depends: ${DEPENDS}

install:
	install -Dm755 ${OUT} ${DESTDIR}${PREFIX}/bin/freetron
    
uninstall:
	${RM} -f ${DESTDIR}${PREFIX}/bin/freetron

clean:
	${RM} ${OUT} ${OBJ}
	${RM} -rf cmake/CMakeFiles cmake/CMakeCache.txt cmake/cmake_install.cmake cmake/Makefile cmake/freetron

-include ${DEPENDS}
.PHONY: all depends install uninstall clean
