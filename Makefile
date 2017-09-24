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
LDFLAGS   += -lpodofo -lIL -ltiff -ltiffxx -pthread -lcppcms -lbooster -lcppdb -lssl -lcrypto

TMPLCC    ?= cppcms_tmpl_cc
PREFIX    ?= /usr/local
MANPREFIX ?= ${PREFIX}/share/man

all: CXXFLAGS += -O2
all: ${OUT}

debug: CXXFLAGS += -O2 -g -Wall -Wextra -Wpedantic
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

min:
	${RM} -f website/files/*.min.*
	yuglify website/files/*.js
	yuglify website/files/*.css
	sed -i '1s#^#// (c) Chris Veness 2002-2014\n#' website/files/sha256.min.js
	sed -i '1s#^#// http://www.boost.org/LICENSE_1_0.txt)\n#' website/files/jsonrpc.min.js
	sed -i '1s#^#// accompanying file LICENSE_1_0.txt or copy at\n#' website/files/jsonrpc.min.js
	sed -i '1s#^#// Distributed under the Boost Software License, Version 1.0. (See\n#' website/files/jsonrpc.min.js
	sed -i '1s#^#// (c) 2011 Artyom Beilis (Tonkikh)\n#' website/files/jsonrpc.min.js

install: ${OUT}
	install -Dm755 ${OUT} ${DESTDIR}${PREFIX}/bin/freetron
	mkdir -p -m 700 ${DESTDIR}/srv/freetron/uploads
	mkdir -p ${DESTDIR}/srv/freetron/files
	mkdir -p ${DESTDIR}${PREFIX}/lib/systemd/system
	install -Dm644 website/files/*.min.* ${DESTDIR}/srv/freetron/files/
	install -Dm644 website/files/*.pdf ${DESTDIR}/srv/freetron/files/
	install -Dm644 website/config.js ${DESTDIR}/srv/freetron/config.js
	install -Dm644 website/freetron.service ${DESTDIR}${PREFIX}/lib/systemd/system/
    
uninstall:
	${RM} -f ${DESTDIR}${PREFIX}/bin/freetron
	${RM} -f ${DESTDIR}${PREFIX}/lib/systemd/system/freetron.service
	${RM} -rf ${DESTDIR}/srv/freetron/files/

clean:
	${RM} ${OUT} ${OBJ} ${DEPENDS} ${SKIN} ${SKINOBJ} ${WEBOBJ}
	${RM} -r cmake/CMakeFiles cmake/CMakeCache.txt cmake/cmake_install.cmake
	${RM} -r cmake/Makefile cmake/freetron cmake/install_manifest.txt

-include ${DEPENDS}
.PHONY: all debug depends install uninstall clean min
