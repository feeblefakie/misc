objs=tcpc tcps tcpc-libevent
all: $(objs)

tcpc: tcpc.cpp
	g++ $< -o $@

tcps: tcps.cpp
	g++ $< -o $@

tcpc-libevent: tcpc-libevent.cpp
	g++ $< -o $@ -L/usr/local/lib -levent -I/usr/local/include

clean:
	rm -rf $(objs)
