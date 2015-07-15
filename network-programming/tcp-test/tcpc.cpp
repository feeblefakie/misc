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

  while (1) {
    int s;
    struct sockaddr_in server;
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket");
      exit(1);
    }

    memset((char *) &server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = dst_ip;
    server.sin_port = htons(atoi(argv[2]));

    if (connect(s, (struct sockaddr *) &server, sizeof(server)) < 0) {
      perror("connect");
      exit(1);
    }
    std::cout << "connected: " << s << std::endl;

    char body[256];
    memset(body, 0, 256);
    strcpy(body, "vaio AND sony\n");

    if (send(s, (void *) body, strlen(body), 0) <= 0) {
      std::cerr << "send failed" << std::endl;
      break;
    }

    // must merge all the response
    memset(body, 0, 256);
    if (!recv(s, (char *) body, sizeof(body), 0)) {
      return false;
    }

    std::cout << "received: [" << body << "]" << std::endl;

    close(s);
    sleep(1);
  }

}
