#include <iostream>
#include <fstream>
#include <string>
#include "update-protocol.pb.h"

int main(int argc, char *argv[])
{
  if (argc != 2) {
    std::cerr << argv[0] << " infile" << std::endl;
    exit(1);
  }

  GOOGLE_PROTOBUF_VERIFY_VERSION;
  Lux::Protocol::Updates updates;

  std::fstream input(argv[1], std::ios::in | std::ios::binary);
  if (!updates.ParseFromIstream(&input)) {
    std::cerr << "Failed to parse." << std::endl;
    return -1;
  }

  for (int i = 0; i < updates.update_size(); i++) {
    const Lux::Protocol::Update &update = updates.update(i);
    std::cout << "type: " << update.type() << std::endl;

    const Lux::Protocol::Document &document = update.document();
    std::cout << "document: " << std::endl;
    std::cout << "id: " << document.id() << std::endl;

    for (int j = 0; j < document.field_size(); j++) {
      const Lux::Protocol::Field &field = document.field(j);

      std::cout << "field: " << std::endl;
      std::cout << "\tname: " << field.name() << std::endl;
      std::cout << "\tvalue: " << field.value() << std::endl;

    }
    std::cout << "--------------------" << std::endl;
  }

  return 0; 
}
