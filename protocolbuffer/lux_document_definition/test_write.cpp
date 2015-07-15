#include <iostream>
#include <fstream>
#include <string>
#include "document_definition.pb.h"

int main(int argc, char *argv[])
{
  if (argc != 2) {
    std::cerr << argv[0] << " outfile" << std::endl;
    exit(1);
  }

  GOOGLE_PROTOBUF_VERIFY_VERSION;
  Lux::DocumentDefinition dd;

  /* FIELD test */
  Lux::FieldDefinition *fd = dd.add_fd();
  fd->set_field_name("title");

  Lux::FieldDefinition::Index *index = fd->mutable_indexed();
  index->set_indexed(true);
  index->set_index_to("default");

  fd->set_display(true);

  Lux::FieldDefinition::AttrIndex *attr_index = fd->mutable_attr_indexed();
  attr_index->set_indexed(true);
  attr_index->set_type(Lux::FieldDefinition::INT);
  attr_index->set_size(4);

  /* FIELD test */
  fd = dd.add_fd();
  fd->set_field_name("desc");

  index = fd->mutable_indexed();
  index->set_indexed(true);
  index->set_index_to("default");

  fd->set_display(true);

  attr_index = fd->mutable_attr_indexed();
  attr_index->set_indexed(true);
  attr_index->set_type(Lux::FieldDefinition::INT);
  attr_index->set_size(4);

  std::fstream output(argv[1], std::ios::out | std::ios::trunc | std::ios::binary);
  if (!dd.SerializeToOstream(&output)) {
    std::cerr << "Failed to write." << std::endl;
    return -1;
  }

  return 0; 
}
