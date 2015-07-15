prog=test_read test_write dump_text_format compile_text_format

all: $(prog)

compile:
	protoc -I=. --cpp_out=. document_definition.proto     

test_read: test_read.cpp
	g++ $< document_definition.pb.cc -lprotobuf -o $@

test_write: test_write.cpp
	g++ $< document_definition.pb.cc -lprotobuf -o $@

dump_text_format: dump_text_format.cpp
	g++ $< document_definition.pb.cc -lprotobuf -o $@

compile_text_format: compile_text_format.cpp
	g++ $< document_definition.pb.cc -lprotobuf -o $@

clean:
	rm -f $(prog) *.o

