#include "cpp/SpellCorrection.h"
#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>
#include <iostream>
#include <vector>
#include <string>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

double gettimeofday_sec()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << argv[0] << " query" << std::endl;
    exit(-1);
  }

  boost::shared_ptr<TSocket> socket(new TSocket("localhost", 9090));
  boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

  double t1 = gettimeofday_sec();
  Adingo::SpellCorrectionClient client(protocol);
  transport->open();
  std::vector<std::string> queries;
  queries.push_back(argv[1]);
  std::vector< std::vector<std::string> > results;
  client.correctme(results, queries);
  for (int i = 0; i < results.size(); ++i) {
    std::cout << "for query: [" << queries[i] << "]\n" << std::endl;
    std::cout << "candidates: " << std::endl;
    for (int j = 0; j < results[i].size(); ++j) {
      std::cout << results[i][j] << std::endl;
    }
  }
  transport->close();
  double t2 = gettimeofday_sec();
  std::cout << "processing time: " << t2 - t1 << " (s)" << std::endl;
  return 0;
}
