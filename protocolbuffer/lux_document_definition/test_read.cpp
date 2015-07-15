#include <iostream>
#include <fstream>
#include <string>
#include "document_definition.pb.h"

int main(int argc, char *argv[])
{
  if (argc != 2) {
    std::cerr << argv[0] << " infile" << std::endl;
    exit(1);
  }

  GOOGLE_PROTOBUF_VERIFY_VERSION;
  Lux::Config::Document doc_config;

  std::fstream input(argv[1], std::ios::in | std::ios::binary);
  if (!doc_config.ParseFromIstream(&input)) {
    std::cerr << "Failed to parse." << std::endl;
    return -1;
  }

  for (int i = 0; i < doc_config.field_size(); i++) {
    const Lux::Config::Field &field = doc_config.field(i);

    std::cout << "field_name: " << field.name() << std::endl;
    std::cout << "--- INDEX ---" << std::endl;
    std::cout << "indexing: " << field.index().indexing() << std::endl;
    std::cout << "index_to: " << field.index().index_to() << std::endl;
    std::cout << "exact: " << field.index().exact() << std::endl;
    std::cout << "--- DISPLAY ---" << std::endl;
    std::cout << "display: " << field.display() << std::endl;
    std::cout << "--- ATTRINDEX ---" << std::endl;
    std::cout << "indexing: " << field.attr_index().indexing() << std::endl;
    std::cout << "type: " << field.attr_index().type() << std::endl;
    std::cout << "size: " << field.attr_index().size() << std::endl;

    std::cout << "--------------------" << std::endl;
  }

  return 0; 
}
