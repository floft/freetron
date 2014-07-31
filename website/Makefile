OUT        = a.out
SRC        = ${wildcard *.cpp}
OBJ        = ${SRC:.cpp=.o}
TSRC       = ${wildcard *.tmpl}
SKIN       = skin.cpp
TOBJ       = ${SKIN:.cpp=.o}
#TCSRC      = ${TSRC:.tmpl=.cpp}
#TOBJ       = ${TSRC:.tmpl=.o}
DEPENDS    = .depends

CXXFLAGS  += -g -O2 -Wall -std=c++11
LDFLAGS   += -lcppcms -lbooster -lcppdb

TMPLCC    ?= cppcms_tmpl_cc
PREFIX    ?= /usr/local
MANPREFIX ?= ${PREFIX}/share/man

all: ${OUT}

${OUT}: ${OBJ} ${TOBJ}
	${CXX} -o $@ ${OBJ} ${TOBJ} ${LDFLAGS}

.cpp.o:
	${CXX} -c -o $@ $< ${CXXFLAGS}

#${TCSRC}: ${TSRC}
#	${TMPLCC} $< -o $@
${SKIN}: master.tmpl ${TSRC}
	${TMPLCC} $^ -o ${SKIN}

${DEPENDS}: ${SRC}
	${RM} -f ./${DEPENDS}
	${CXX} ${CXXFLAGS} -MM $^ >> ./${DEPENDS}

install:
	install -Dm755 ${OUT} ${DESTDIR}${PREFIX}/bin/${OUT}

uninstall:
	${RM} ${DESTDIR}${PREFIX}/bin/${OUT}

clean:
	${RM} ${OUT} ${OBJ} ${TOBJ} ${TCSRC} ${SKIN} ${DEPENDS}

-include ${DEPENDS}
.PHONY: all install uninstall clean
