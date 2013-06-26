OUT        = freetron
SRC        = ${wildcard *.cpp}
OBJ        = ${SRC:.cpp=.o}
DEPENDS    = .depends

#CXX		   = clang++
CXXFLAGS  := ${CXXFLAGS}  -g -ffast-math -funroll-loops -O2 -std=c++11 -Wall -Wextra -Wpedantic
LDFLAGS   := ${LDFLAGS} -lpodofo -lIL -ltiff -ltiffxx -pthread

PREFIX    ?= /usr/local
MANPREFIX ?= ${PREFIX}/share/man

all: ${OUT}

${OUT}: ${OBJ}
	${CXX} -o $@ ${OBJ} ${LDFLAGS}

.cpp.o:
	${CXX} -c -o $@ $< ${CXXFLAGS}

${DEPENDS}: ${SRC}
	rm -f ./${DEPENDS}
	${CXX} ${CXXFLAGS} -MM $^ >> ./${DEPENDS}

depends: ${DEPENDS}

install:
	install -Dm755 ${OUT} ${DESTDIR}${PREFIX}/bin/freetron
    
uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/freetron

clean:
	${RM} ${OUT} ${OBJ}

include ${DEPENDS}
.PHONY: all depends install uninstall clean
