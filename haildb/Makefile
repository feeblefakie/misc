objs=index get joinindex dump_columns index_uncompressed index_rle index_key_rle index_ji q3-1_1 q3-1_1-mt drop_table sec_key_test q3-1_1-hjsim
all: $(objs)

index: index.cpp
	 g++ -O2 $< -o $@ -I/usr/local/include -L/usr/local/lib -lhaildb 

get: get.cpp
	 g++ -O2 $< -o $@ -I/usr/local/include -L/usr/local/lib -lhaildb 

joinindex: joinindex.cpp
	 g++ -O2 $< -o $@ -I/usr/local/include -L/usr/local/lib -lhaildb 

dump_columns: dump_columns.cpp
	 g++ -O2 $< -o $@ -I/usr/local/include -L/usr/local/lib -lhaildb 

index_uncompressed: index_uncompressed.cpp
	 g++ -O2 $< -o $@ -I/usr/local/include -L/usr/local/lib -lhaildb 

index_rle: index_rle.cpp
	 g++ -O2 $< -o $@ -I/usr/local/include -L/usr/local/lib -lhaildb 

index_key_rle: index_key_rle.cpp
	 g++ -O2 $< -o $@ -I/usr/local/include -L/usr/local/lib -lhaildb 

index_ji: index_ji.cpp
	 g++ -O2 $< -o $@ -I/usr/local/include -L/usr/local/lib -lhaildb 

q3-1_1: q3-1_1.cpp
	 g++ -O2 $< -o $@ -I/usr/local/include -L/usr/local/lib -lhaildb 

q3-1_1-mt: q3-1_1-mt.cpp
	 g++ -O2 $< -o $@ -I/usr/local/include -L/usr/local/lib -lhaildb 

drop_table: drop_table.cpp
	 g++ -O2 $< -o $@ -I/usr/local/include -L/usr/local/lib -lhaildb 

sec_key_test: sec_key_test.cpp
	 g++ -O2 $< -o $@ -I/usr/local/include -L/usr/local/lib -lhaildb -DNDEBUG

q3-1_1-hjsim: q3-1_1-hjsim.cpp
	 g++ -O2 $< -o $@ -I/usr/local/include -L/usr/local/lib -lhaildb 

clean:
	rm -rf *.o $(objs)
