objs=lmerge search dump parser lmerge-pline lmerge-pline2 lmerge-pline3 lmerge-pipe
all: $(objs)

lmerge: lmerge.cpp
	g++ $< -o $@ -L/usr/local/lib -lluxio -lmecab -I/usr/local/include -I/opt/local/include -g -O2

lmerge-pline: lmerge-pline.cpp
	g++ $< -o $@ -L/usr/local/lib -lluxio -lmecab -I/usr/local/include -I/opt/local/include -g -O2

lmerge-pline2: lmerge-pline2.cpp
	g++ $< -o $@ -L/usr/local/lib -lluxio -lmecab -I/usr/local/include -I/opt/local/include -g -O2

lmerge-pline3: lmerge-pline3.cpp
	g++ $< -o $@ -L/usr/local/lib -lluxio -lmecab -I/usr/local/include -I/opt/local/include -g -O2

lmerge-pline4: lmerge-pline4.cpp
	g++ $< -o $@ -L/usr/local/lib -lluxio -lmecab -I/usr/local/include -I/opt/local/include -g -O2

lmerge-pipe: lmerge-pipe.cpp
	g++ -Wall $< -o $@ -L/usr/local/lib -lluxio -lmecab -I/usr/local/include -I/opt/local/include -g -O2

lmerge-pipe2: lmerge-pipe2.cpp
	g++ -Wall $< -o $@ -L/usr/local/lib -lluxio -lmecab -I/usr/local/include -I/opt/local/include -g -O2
	
lmerge-pipe3: lmerge-pipe3.cpp
	g++ -Wall $< -o $@ -L/usr/local/lib -lluxio -lmecab -I/usr/local/include -I/opt/local/include -g -O2

lmerge-pipe4: lmerge-pipe4.cpp
	g++ -Wall $< -o $@ -L/usr/local/lib -lluxio -lmecab -I/usr/local/include -I/opt/local/include -g -O2

lmerge-pipe4-comp: lmerge-pipe4-comp.cpp
	g++ -Wall $< -o $@ -L/usr/local/lib -lluxio -lmecab -I/usr/local/include -I/opt/local/include -g -O2

lmerge-pipe5: lmerge-pipe5.cpp
	g++ -Wall $< -o $@ -L/usr/local/lib -lluxio -lmecab -I/usr/local/include -I/opt/local/include -g -O2

lmerge-pipe6: lmerge-pipe6.cpp
	g++ -Wall $< -o $@ -L/usr/local/lib -lluxio -lmecab -I/usr/local/include -I/opt/local/include -g -O2
	
lmerge-pipe7: lmerge-pipe7.cpp
	g++ -Wall $< -o $@ -L/usr/local/lib -lluxio -lmecab -I/usr/local/include -I/opt/local/include -g -O2

lmerge-pipe7-comp: lmerge-pipe7-comp.cpp
	g++ -Wall $< -o $@ -L/usr/local/lib -lluxio -lmecab -I/usr/local/include -I/opt/local/include -g -O2

lmerge-pipe8: lmerge-pipe8.cpp
	g++ -Wall $< -o $@ -L/usr/local/lib -lluxio -lmecab -I/usr/local/include -I/opt/local/include -g -O2

lmerge-pipe9: lmerge-pipe9.cpp
	g++ -Wall $< -o $@ -L/usr/local/lib -lluxio -lmecab -I/usr/local/include -I/opt/local/include -g -O2

lmerge-pipe9-2: lmerge-pipe9-2.cpp
	g++ -Wall $< -o $@ -L/usr/local/lib -lluxio -lmecab -I/usr/local/include -I/opt/local/include -g -O2

lmerge-pipe9-3: lmerge-pipe9-3.cpp
	g++ -Wall $< -o $@ -L/usr/local/lib -lluxio -lmecab -I/usr/local/include -I/opt/local/include -g -O2

lmerge-pipe9-4: lmerge-pipe9-4.cpp
	g++ -Wall $< -o $@ -L/usr/local/lib -lluxio -lmecab -I/usr/local/include -I/opt/local/include -g -O2

lmerge-pipe9-5: lmerge-pipe9-5.cpp
	g++ -Wall $< -o $@ -L/usr/local/lib -lluxio -lmecab -I/usr/local/include -I/opt/local/include -g 

search: search.cpp
	g++ $< -o $@ -L/usr/local/lib -lluxio -I/usr/local/include -g

gsn_check: gsn_check.cpp
	g++ $< -o $@ -L/usr/local/lib -lluxio -I/usr/local/include -g

dump: dump.cpp
	g++ $< -o $@ -L/usr/local/lib -lluxio -I/usr/local/include -g

parser: parser.cpp
	g++ $< -o $@ -L/usr/local/lib -lmecab -I/usr/local/include -g

gsn_search_bench: gsn_search_bench.cpp
	g++ $< -o $@ -L/usr/local/lib -lmecab -lluxio -I/usr/local/include -g 

search_bench: search_bench.cpp
	g++ $< -o $@ -L/usr/local/lib -lmecab -lluxio -I/usr/local/include -g -O2

search_bench: search_bench.cpp
	g++ $< -o $@ -L/usr/local/lib -lmecab -lluxio -I/usr/local/include -g -O2

clean:
	rm -rf $(objs) *.o *.dSYM

