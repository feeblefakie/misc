#include <iostream>
#include <fstream>
#include <string>
#include "server-config.pb.h"

int main(int argc, char *argv[])
{
  if (argc != 2) {
    std::cerr << argv[0] << " infile" << std::endl;
    exit(1);
  }

  GOOGLE_PROTOBUF_VERIFY_VERSION;
  Lux::Config::ServerConfig config;

  std::fstream input(argv[1], std::ios::in | std::ios::binary);
  if (!config.ParseFromIstream(&input)) {
    std::cerr << "Failed to parse." << std::endl;
    return -1;
  }
  std::cout << "size: " << config.ByteSize() << std::endl;

  std::cout << "=== CLUSTERS ===" << std::endl;
  Lux::Config::Clusters clusters = config.clusters();
  for (int i = 0; i < clusters.cluster_size(); i++) {
    std::cout << "=== CLUSTER " << i << " ===" << std::endl;

    Lux::Config::Cluster cluster = clusters.cluster(i);
    for (int j = 0; j < cluster.server_size(); j++) {
      Lux::Config::Server server = cluster.server(j);

      std::cout << "host: " << server.host() << std::endl;

    }
  }


  std::cout << "=== DISPATCHERS ===" << std::endl;
  Lux::Config::Dispatchers dispatchers = config.dispatchers();

  for (int i = 0; i < dispatchers.dispatcher_size(); i++) {
    std::cout << "=== DISPATCHERS " << i << " ===" << std::endl;

    Lux::Config::Dispatcher dispatcher = dispatchers.dispatcher(i);
    std::cout << dispatcher.cluster_id() << std::endl;

    Lux::Config::Server server = dispatcher.server();
    std::cout << "host: " << server.host() << std::endl;
  }

  return 0; 
}
