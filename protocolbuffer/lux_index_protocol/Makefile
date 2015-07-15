prog=test_read test_write

all: $(prog)

compile:
	protoc -I=. --cpp_out=. update-protocol.proto

test_read: test_read.cpp
	g++ $< update-protocol.pb.cc -lprotobuf -o $@

test_write: test_write.cpp
	g++ $< update-protocol.pb.cc -lprotobuf -o $@

clean:
	rm -f $(prog) *.o

