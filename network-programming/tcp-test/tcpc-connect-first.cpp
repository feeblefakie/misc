#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char *argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " host_or_ip port" << std::endl;
    exit(1);
  }

  uint32_t dst_ip;
  if ((dst_ip = inet_addr(argv[1])) == INADDR_NONE) {
    struct hostent *he;
    if ((he = gethostbyname(argv[1])) == NULL) {
      std::cerr << "gethostbyname failed" << std::endl;
      exit(1);
    }
    memcpy((char *) &dst_ip, (char *) he->h_addr, he->h_length);
  }

  int port = atoi(argv[2]);
  int num_servers = 10;
  int num_socks = 30;
  int s[num_socks * num_servers];
  for (int j = 0; j < num_servers; ++j) {
  for (int i = 0; i < num_socks; ++i) {
    int idx = i + 30 * j;
    std::cout << "idx: " << idx << std::endl;
    if ((s[idx] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket");
      exit(1);
    }

    struct sockaddr_in server;
    memset((char *) &server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = dst_ip;
    //server.sin_port = htons(atoi(argv[2]));
    server.sin_port = htons(port + j);

    if (connect(s[idx], (struct sockaddr *) &server, sizeof(server)) < 0) {
      perror("connect");
      exit(1);
    }
    std::cout << "connected " << s[idx] << std::endl;
  }
  } 
  std::cout << "all connected" << std::endl;

  std::cout << "sleeping" << std::endl;
  sleep(100);


  return 0;
}
