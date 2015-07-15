CPP = g++44
#CPP = g++
PROGS = main build_column pos_test ops_test em_pipe em_para lm_pipe lm_para idx_lookup_test idx_dump gen_rand_nums gen_rand_nums2 toyotaq toyotaqooo ooo_test idx_build idx_build_batch memaccess column_scan_bench column_lookup_bench join copy_nocomp cat_column_header scan toyota_typical_q3 toyota_typical_q16-4
CFLAGS = -g -O3 #-ltcmalloc 
CFLAGSDEBUG = -g
LDFLAGS = -L/usr/local/BerkeleyDB.5.2/lib -ldb_cxx -lpthread
INCLUDES = -I/usr/local/BerkeleyDB.5.2/include

all: $(PROGS)

main: column.cpp main.cpp
	$(CPP) $(CFLAGS) $(LDFLAGS) $(INCLUDES) column.cpp main.cpp -o $@

build_column: column.cpp build_column.cpp
	$(CPP) $(CFLAGS) $(LDFLAGS) $(INCLUDES) column.cpp build_column.cpp -o $@

idx_lookup_test: idx_lookup_test.cpp column.cpp
	$(CPP) $(CFLAGS) $(LDFLAGS) $(INCLUDES) column.cpp idx_lookup_test.cpp -o $@

idx_dump: idx_dump.cpp column.cpp
	$(CPP) $(CFLAGS) $(LDFLAGS) $(INCLUDES) column.cpp idx_dump.cpp -o $@

pos_test: column.cpp pos_test.cpp
	$(CPP) $(CFLAGS) $(LDFLAGS) $(INCLUDES) column.cpp pos_test.cpp -o $@

ops_test: column.cpp ops.cpp ops_test.cpp
	$(CPP) $(CFLAGS) $(LDFLAGS) $(INCLUDES) column.cpp ops.cpp ops_test.cpp -o $@

em_pipe: em_pipe.cpp column.cpp ops.cpp 
	$(CPP) $(CFLAGS) $(LDFLAGS) $(INCLUDES) em_pipe.cpp column.cpp ops.cpp -o $@

em_para: em_para.cpp column.cpp ops.cpp 
	$(CPP) $(CFLAGS) $(LDFLAGS) $(INCLUDES) em_para.cpp column.cpp ops.cpp -o $@

lm_pipe: lm_pipe.cpp column.cpp ops.cpp 
	$(CPP) $(CFLAGS) $(LDFLAGS) $(INCLUDES) lm_pipe.cpp column.cpp ops.cpp -o $@

lm_para: lm_para.cpp column.cpp ops.cpp 
	$(CPP) $(CFLAGS) $(LDFLAGS) $(INCLUDES) lm_para.cpp column.cpp ops.cpp -o $@

gen_rand_nums: gen_rand_nums.cpp
	$(CPP) $(CFLAGS) $(LDFLAGS) $(INCLUDES) gen_rand_nums.cpp -o $@

gen_rand_nums2: gen_rand_nums2.cpp
	$(CPP) $(CFLAGS) $(LDFLAGS) $(INCLUDES) gen_rand_nums2.cpp -o $@

toyotaq: toyotaq.cpp column.cpp ops.cpp 
	$(CPP) $(CFLAGS) $(LDFLAGS) $(INCLUDES) toyotaq.cpp column.cpp ops.cpp -o $@

toyotaqooo: toyotaqooo.cpp column.cpp ops.cpp 
	$(CPP) $(CFLAGS) $(LDFLAGS) $(INCLUDES) toyotaqooo.cpp column.cpp ops.cpp -o $@

idx_build: idx_build.cpp
	$(CPP) $(CFLAGS) $(LDFLAGS) $(INCLUDES) idx_build.cpp -o $@

idx_build_batch: idx_build_batch.cpp
	$(CPP) $(CFLAGS) $(LDFLAGS) $(INCLUDES) idx_build_batch.cpp column.cpp -o $@

ooo_test: ooo_test.cpp
	$(CPP) $(CFLAGS) $(LDFLAGS) $(INCLUDES) ooo_test.cpp -o $@

memaccess: memaccess.cpp
	$(CPP) -g -lpthread memaccess.cpp -o $@

column_scan_bench: column_scan_bench.cpp ops.cpp column.cpp
	$(CPP) $(CFLAGS) $(LDFLAGS) $(INCLUDES) $< ops.cpp column.cpp -o $@

column_lookup_bench: column_lookup_bench.cpp ops.cpp column.cpp
	$(CPP) $(CFLAGS) $(LDFLAGS) $(INCLUDES) $< ops.cpp column.cpp -o $@

join: join.cpp column.cpp ops.cpp 
	$(CPP) $(CFLAGS) $(CFLAGSDEBUG) $(LDFLAGS) $(INCLUDES) -std=c++0x join.cpp column.cpp ops.cpp -o $@

scan: scan.cpp column.cpp ops.cpp 
	$(CPP) $(CFLAGS) $(CFLAGSDEBUG) $(LDFLAGS) $(INCLUDES) -std=c++0x scan.cpp column.cpp ops.cpp -o $@

toyota_typical_q3: toyota_typical_q3.cpp column.cpp ops.cpp 
	$(CPP) $(CFLAGS) $(CFLAGSDEBUG) $(LDFLAGS) $(INCLUDES) -std=c++0x toyota_typical_q3.cpp column.cpp ops.cpp -o $@

toyota_typical_q16-4: toyota_typical_q16-4.cpp column.cpp ops.cpp 
	$(CPP) $(CFLAGS) $(CFLAGSDEBUG) $(LDFLAGS) $(INCLUDES) -std=c++0x toyota_typical_q16-4.cpp column.cpp ops.cpp -o $@

copy_nocomp: copy_nocomp.cpp
	$(CPP) $(CFLAGSDEBUG) $(LDFLAGS) $(INCLUDES) -std=c++0x $< -o $@

cat_column_header: cat_column_header.cpp
	$(CPP) $(CFLAGSDEBUG) $(LDFLAGS) $(INCLUDES) -std=c++0x $< -o $@

.cpp.o:
	$(CPP) $(CFLAGS) $(LDFLAGS) $(INCLUDES) -c $<

clean: 
	rm -f *.o
	rm -f col1 col3 col3.posidx
	rm -f core*
	rm -f vgcore*
	rm -f $(PROGS)
