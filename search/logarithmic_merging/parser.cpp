#include <iostream>
#include <string>
#include <mecab.h>
#include <cstring>

int get_length_of(const unsigned char *str);

int main(int argc, char *argv[])
{
  MeCab::Tagger *tagger = MeCab::createTagger(0, NULL);

  std::string line;
  int id = 0;
  while (getline(std::cin, line)) {
    unsigned int offset = 0;
    const MeCab::Node *node_ = tagger->parseToNode(line.c_str());
    for (; node_; node_ = node_->next) {
      if (node_->stat == MECAB_BOS_NODE ||
        node_->stat == MECAB_EOS_NODE) {
        continue; 
      }   
      std::string token(node_->surface, node_->length);
      unsigned int length = get_length_of((const unsigned char *) token.c_str());
      // mecab ignores spaces in default 
      // but they must be counted as offset from the beginning
      int head_space_len =  node_->rlength - node_->length;
      offset += head_space_len > 0 ? head_space_len : 0;

      std::cout << id << "" << token << "" << length << "" << offset << std::endl;
      //tokens_.push_back(Term(token, length, offset));
      offset += length;
    }   
    ++id;
  }
  return 0;
}

int get_length_of(const unsigned char *str)
{   
  size_t str_len = std::strlen((char *) str);
  int ustr_len = 0;

  for (int i = 0; i < str_len; i++) {
    if (str[i] <= 0x7f || (str[i] >= 0xc0 && str[i] <= 0xfd)) {
      ustr_len++;
    }   
  }   
  return ustr_len;
}
