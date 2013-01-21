OUT		= freetron
SRC		= ${wildcard *.cpp}
OBJ		= ${SRC:.cpp=.o}
DEPENDS		= .depends

CC		= g++
CFLAGS		:= -ffast-math -funroll-loops -O2 -std=c++11 -Wall ${CFLAGS}
LDFLAGS		:= -lpodofo -lIL -ltiff -ltiffxx -pthread ${LDFLAGS}

PREFIX		?= /usr/local
MANPREFIX	?= ${PREFIX}/share/man

all: ${OUT}

${OUT}: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

.cpp.o:
	${CC} -c -o $@ $< ${CFLAGS}

${DEPENDS}: ${SRC}
	rm -f ./${DEPENDS}
	${CC} ${CFLAGS} -MM $^ >> ./${DEPENDS}

depends: ${DEPENDS}

install:
	install -Dm755 ${OUT} ${DESTDIR}${PREFIX}/bin/freetron
	
uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/freetron

clean:
	${RM} ${OUT} ${OBJ}

include ${DEPENDS}
.PHONY: all depends install uninstall clean
