objs=test random_aio
all : $(objs)
test: test.cpp
	g++ -O2 $< -o $@ -laio
random_aio: random_aio.cpp
	g++ -O2 $< -o $@ -laio

clean:
	rm -rf $(objs)
