#include <fstream>
#include <iostream>
#include <string>
#include <google/protobuf/text_format.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "server-config.pb.h"

using namespace google::protobuf;

int main (int argc, char **argv)
{
  if (argc != 3) {
    std::cerr << argv[0] << " infile outfile" << std::endl;
    exit(1);
  }

  GOOGLE_PROTOBUF_VERIFY_VERSION;
  Lux::Config::ServerConfig config;

  std::fstream input(argv[1], std::ios::in | std::ios::binary);
  if (!config.ParseFromIstream(&input)) {
    std::cerr << "Failed to parse." << std::endl;
    return -1;
  }

  std::fstream foutput(argv[2], std::ios::out | std::ios::trunc); 
  io::OstreamOutputStream* output = new io::OstreamOutputStream(&foutput); 
  if (!TextFormat::Print(config, output)) {     
    std::cerr << "Failed to write." << std::endl;
    return -1;
  }
  delete output;

  return 0;
}
