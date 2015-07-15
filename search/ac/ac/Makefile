CXX = g++
CXXFLAGS = -O3 -Wall -Wno-deprecated -I.
TARGETS = hatenakeyword mkdarts mktrie search

all: $(TARGETS)

hatenakeyword: hatenakeyword.o
	${CXX} ${CXXFLAGS} ${LDFLAGS} -o hatenakeyword hatenakeyword.o ${LDFLAGS}

mkdarts: mkdarts.o
	${CXX} ${CXXFLAGS} ${LDFLAGS} -o mkdarts mkdarts.o ${LDFLAGS}
	
mktrie: mktrie.cpp
	${CXX} ${CXXFLAGS} ${LDFLAGS} -o $@ $^ ${LDFLAGS}

search: search.cpp
	${CXX} ${CXXFLAGS} ${LDFLAGS} -o $@ $^ ${LDFLAGS}

clean:
	rm -f *.o ${TARGETS}

