objs=tcpc tcps tcpc-c
all: $(objs)

tcpc: tcpc.cpp
	g++ $< -o $@

tcps: tcps.cpp
	g++ $< -o $@

tcpc-c: tcpc-connect-first.cpp
	g++ $< -o $@

clean:
	rm -rf $(objs)
