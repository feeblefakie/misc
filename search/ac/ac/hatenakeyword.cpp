/*
  hatenakeyword.cpp: hatena keyword annotation program
                     based on Double-Array TRIE
  Copyright (C) 2005 Taku Kudo <taku@chasen.org>
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.
*/

#include "darts.h"
#include <iostream>

void encodeURL(std::ostream &os, char *begin, char *end)
{
  static const char  *digits = "0123456789abcdef";
  for (char *p = begin; p != end; ++p) {
    if (isascii(*p) && (isdigit(*p) || isalpha(*p)))
      os << *p;
    else 
      os << '%' << digits[(*p >> 4) & 0x0f] << digits[*p & 0x0f];
  }
}

int main(int argc, char **argv)
{
  Darts::DoubleArray da;

  if (da.open("keys.da") != 0) {
    std::cerr << "cannot open keywordlist.da" << std::endl;
    return -1;
  }

  char text[8192];
  Darts::DoubleArray::result_pair_type daresult[512];

  while(std::cin.getline(text, 8192)) {

    char *begin = text;
    char *end   = text + strlen(text);

    while (begin < end) {

      size_t size = da.commonPrefixSearch(begin, daresult, 
				       512, (size_t)(end - begin));
      size_t seekto = 0;       

      if (size) {
	 
        // finding the longest match ?
	for (size_t i = 0; i < size; ++i) {
	  if (seekto < daresult[i].length)
	    seekto = daresult[i].length;
	}
	 
	if (seekto) {
	  std::cout << "<a href=\"http://d.hatena.ne.jp/keyword/";
	  encodeURL(std::cout, begin, begin + seekto);
	  std::cout << "\">";
	  std::cout.write(begin, seekto);
	  std::cout << "</a>";
	  begin += seekto;
	}
      }
       
      if (seekto == 0) {
	std::cout.write(begin, 1);
	++begin; 
      }
    }
     
    std::cout << std::endl;
  }
}
