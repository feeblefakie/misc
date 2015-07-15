CXX = g++
CXXFLAGS = -O3 -Wall -Wno-deprecated -I. -I.. -I/usr/local/include/thrift
LDFLAGS = -L/usr/local/lib -lthrift

TARGETS = ttydic-server

all: $(TARGETS)

ttydic-server: *.cpp
	${CXX} ${CXXFLAGS} ${LDFLAGS} -o $@ $^

clean:
	rm -f *.o ${TARGETS}

