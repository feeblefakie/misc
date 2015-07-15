#include <iostream>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_PORT 5320
#define BUFSIZE 128

int main(int argc, char *argv[])
{
  int port = DEFAULT_PORT;
  if (argc == 2) {
    if ((port = atoi(argv[1])) == 0) {
      struct servent *se;
      if ((se = getservbyname(argv[1], "tcp")) == NULL) {
        perror("error in getservbyname");
      } else {
        port = (int) ntohs((uint16_t) se->s_port);
      }
    }
  }
  std::cout << "port: " << port << std::endl;

  int s0;
  if ((s0 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }

  sockaddr_in server;
  memset((char *) &server, 0, sizeof(sockaddr_in));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(port);
  if (bind(s0, (sockaddr *) &server, sizeof(sockaddr)) < 0) {
    perror("bind");
    exit(1);
  }

  //listen(s0, 5);
  listen(s0, 50);

  int s;
  sockaddr_in client;
  char buf[BUFSIZE];
  while (1) {
    memset(buf, 0, BUFSIZE);
    int len = sizeof(sockaddr_in);
    if ((s = accept(s0, (sockaddr *) &client, (socklen_t *) &len)) < 0) {
      perror("accept");
      exit(1);
    }
    std::cout << "connection from " << inet_ntoa(client.sin_addr) << std::endl;

    if (fork() != 0) {
      close(s);
      continue;
    }
    close(s0);

    int i = 0;
    while (1) {
      int rn;
      if ((rn = recv(s, &buf[i], 1, 0)) < 0) {
        break;
      }

      if (buf[i] != '\n') {
        ++i;
        //continue;
        break;
        /*
        if (i < BUFSIZE - 1) {
          continue;
        }
        */
      } else {
        buf[i] = '\0';
        std::cout << "received: " << buf << std::endl;
        memset(buf, 0, BUFSIZE);
        i = 0;
      }

    }
    std::cout << "processing ... " << std::endl;
    usleep(1000 * 500); // 500 milliseconds

    char buf[128] = "received";
    if (send(s, buf, strlen(buf), 0) <= 0) {
      std::cerr << "send() failed" << std::endl;
    }
    std::cout << "connection closed" << std::endl;
    close(s);
    exit(0);
  }

  // usually never comes here

  return 0;
}
