#include <fstream>
#include <iostream>
#include <string>
#include <google/protobuf/text_format.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "document_definition.pb.h"

using namespace google::protobuf;

int main (int argc, char **argv)
{
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  Lux::DocumentDefinition dd;

  std::fstream input("out.bin", std::ios::in | std::ios::binary);
  if (!dd.ParseFromIstream(&input)) {
    std::cerr << "Failed to parse." << std::endl;
    return -1;
  }

  std::fstream foutput("dd.cnf", std::ios::out | std::ios::trunc); 
  io::OstreamOutputStream* output = new io::OstreamOutputStream(&foutput); 
  if (!TextFormat::Print(dd, output)) {     
    std::cerr << "Failed to write." << std::endl;
    return -1;
  }
  delete output;

  return 0;
}
