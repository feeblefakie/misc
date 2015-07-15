#include <iostream>
#include <fstream>
#include <string>
#include "server-config.pb.h"

int main(int argc, char *argv[])
{
  if (argc != 2) {
    std::cerr << argv[0] << " outfile" << std::endl;
    exit(1);
  }

  GOOGLE_PROTOBUF_VERIFY_VERSION;
  Lux::Config::ServerConfig config;

  Lux::Config::Clusters *clusters = config.mutable_clusters();
  Lux::Config::Cluster *cluster = clusters->add_cluster();
  cluster->set_id(0);
  Lux::Config::Server *server = cluster->add_server();
  server->set_id(0);
  server->set_host("localhost");
  server->set_port(4306);
  server = cluster->add_server();
  server->set_id(1);
  server->set_host("localhost2");
  server->set_port(4406);

  Lux::Config::Dispatchers *dispatchers = config.mutable_dispatchers();
  Lux::Config::Dispatcher *dispatcher = dispatchers->add_dispatcher();
  dispatcher->set_cluster_id(0);
  Lux::Config::Server *server2 = dispatcher->mutable_server();
  server2->set_id(0);
  server2->set_host("localhost");
  server2->set_port(5555);

  std::fstream output(argv[1], std::ios::out | std::ios::trunc | std::ios::binary);
  if (!config.SerializeToOstream(&output)) {
    std::cerr << "Failed to write." << std::endl;
    return -1;
  }

  return 0; 
}
