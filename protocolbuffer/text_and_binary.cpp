#include <fstream>
#include <iostream>
#include <string>
#include <google/protobuf/text_format.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "addressbook.pb.h"

using namespace std;
using namespace google::protobuf;

int main (int argc, char **argv)
{
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  tutorial::AddressBook address_book;
  PromptForAddress(address_book.add_person()); // データ作成

  // Binary Output
  {
    fstream output("addressbook.bin", ios::out | ios::trunc | ios::binary);    
    if (!address_book.SerializeToOstream(&output)) {     
      cerr << "Failed to write address book." << endl;  
      return -1;  
    }
  }
  tutorial::AddressBook address_book2;
  // Binary Input
  {
    fstream input("addressbook.bin", ios::in | ios::binary);    
    if (!address_book2.ParseFromIstream(&input)) {    
      cerr << "Failed to parse address book." << endl;  
      return -1;  
    } 
  } 

  // Text Output
  {
    //fstream foutput("addressbook.txt", ios::out | ios::trunc | ios::binary); // 改行をどうするか
    fstream foutput("addressbook.txt", ios::out | ios::trunc); 
    io::OstreamOutputStream* output = new io::OstreamOutputStream(&foutput); 
    if (!TextFormat::Print(address_book, output)) {     
      cerr << "Failed to write address book." << endl;  
      return -1;
    }
    delete output;
  }
  tutorial::AddressBook address_book3;
  // Text Input
  {    
    //fstream finput("addressbook.txt", ios::in | ios::binary); // 改行をどうするか
    fstream finput("addressbook.txt", ios::in); 
    io::IstreamInputStream* input = new io::IstreamInputStream(&finput);
    TextFormat::Parser parser;
    if (!parser.Parse(input, &address_book3)) {
      cerr << "Failed to parse address book." << endl;
      return -1;  
    }
    delete input;
  } 

  return 0;
}
