#include <iostream>
#include <fstream>
#include <string>
#include "update-protocol.pb.h"

int main(int argc, char *argv[])
{
  if (argc != 2) {
    std::cerr << argv[0] << " outfile" << std::endl;
    exit(1);
  }

  GOOGLE_PROTOBUF_VERIFY_VERSION;
  Lux::Protocol::Updates updates;

  Lux::Protocol::Update *update = updates.add_update();
  update->set_type(Lux::Protocol::Update::ADD);
  Lux::Protocol::Document *document = update->mutable_document();
  document->set_id("http://hoge.com");
  Lux::Protocol::Field *field = document->add_field();
  field->set_name("title");
  field->set_value("this is a title of a field.");

  Lux::Protocol::Update *update2 = updates.add_update();
  update2->set_type(Lux::Protocol::Update::DELETE);
  Lux::Protocol::Document *document2 = update2->mutable_document();
  document2->set_id("http://fuga.com");

  std::fstream output(argv[1], std::ios::out | std::ios::trunc | std::ios::binary);
  if (!updates.SerializeToOstream(&output)) {
    std::cerr << "Failed to write." << std::endl;
    return -1;
  }

  return 0; 
}
