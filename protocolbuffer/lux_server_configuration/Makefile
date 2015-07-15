prog=test_read test_write dump_text_format compile_text_format

all: $(prog)

compile:
	protoc -I=. --cpp_out=. server-config.proto

test_read: test_read.cpp
	g++ $< server-config.pb.cc -lprotobuf -o $@

test_write: test_write.cpp
	g++ $< server-config.pb.cc -lprotobuf -o $@

dump_text_format: dump_text_format.cpp
	g++ $< server-config.pb.cc -lprotobuf -o $@

compile_text_format: compile_text_format.cpp
	g++ $< server-config.pb.cc -lprotobuf -o $@

clean:
	rm -f $(prog) *.o

