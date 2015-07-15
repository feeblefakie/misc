#include <fstream>
#include <iostream>
#include <string>
#include <google/protobuf/text_format.h>
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

  {
    fstream finput(argv[1], ios::in); 
    io::IstreamInputStream input(&finput);
    TextFormat::Parser parser;
    if (!parser.Parse(&input, &config)) {
      std::cerr << "Failed to parse." << std::endl;
      return -1;  
    }
  }

  {
    fstream output(argv[2], ios::out | ios::trunc | ios::binary);    
    if (!config.SerializeToOstream(&output)) {     
      std::cerr << "Failed to write." << std::endl;  
      return -1;  
    }
  }

  return 0;
}
