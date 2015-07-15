#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <event.h>

#define MAXFILES 20

struct file {
  uint32_t ip;
  int fd;
  int flags;
} file[MAXFILES];

#define F_CONNECTING 1
#define F_READING 2
#define F_DONE 4

int nconn, nfiles, nlefttoconn, nlefttoread, maxfd;
fd_set rset, wset;

typedef struct {
  struct event *event;
  struct file *file;
} arg_t;


void event_handler(int fd, short event, void *arg);

int main(int argc, char *argv[])
{
  fd_set rs, ws;

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

  nfiles = 20;
  for (int i = 0; i < nfiles; ++i) {
    file[i].ip = dst_ip;
    file[i].flags = 0;
  }

  arg_t args[nfiles];

  // init event
  struct event ev[nfiles];
  event_init();

  //FD_ZERO(&rset);
  //FD_ZERO(&wset);
  maxfd = -1;
  nlefttoread = nlefttoconn = nfiles;
  nconn = 0;

  struct sockaddr_in server;
  memset((char *) &server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = dst_ip;
  server.sin_port = htons(atoi(argv[2]));

  char body[256];
  memset(body, 0, 256);
  strcpy(body, "vaio AND sony\n");

  int i = 0;
  while (nlefttoread > 0) {
    while (nlefttoconn > 0) {
      for (i = 0; i < nfiles; i++) {
        if (file[i].flags == 0) { break; }
      }
      if (i == nfiles) {
        std::cerr << "error" << std::endl;
      }

      /* start_connect START */
      if ((file[i].fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
      }

      int flags;
      flags = fcntl(file[i].fd, F_GETFL, 0);
      fcntl(file[i].fd, F_SETFL, flags | O_NONBLOCK);

      int n;
      if ((n = connect(file[i].fd, (struct sockaddr *) &server, sizeof(server))) < 0) {
        if (errno != EINPROGRESS) {
          std::cerr << "nonblocking connect error" << std::endl;
        }
        file[i].flags = F_CONNECTING;
        
        args[i].event = &(ev[i]);
        args[i].file = &(file[i]);
        event_set(&(ev[i]), file[i].fd, EV_READ | EV_WRITE, event_handler, &args[i]);
        std::cout << "event set for " << file[i].fd << std::endl;
        event_add(&(ev[i]), NULL);
        //FD_SET(file[i].fd, &rset);
        //FD_SET(file[i].fd, &wset);
        if (file[i].fd > maxfd) {
          maxfd = file[i].fd;
        }
      } else if (n >= 0) {
        std::cout << "connection established right away" << std::endl;
        if (send(file[i].fd, (void *) body, strlen(body), 0) <= 0) {
          std::cerr << "send failed" << std::endl;
          break;
        }
      }
      /* start_connect END */
      nconn++;
      nlefttoconn--;
    }

    event_dispatch();

    std::cout << "received all from all the sockets" << std::endl;

    //rs = rset;
    //ws = wset;

    //int n = select(maxfd+1, &rs, &ws, NULL, NULL);
    /*
    for (int i = 0; i < nfiles; i++) {
      int flags = file[i].flags;
      if (flags == 0 || flags & F_DONE) { continue; }
      int fd = file[i].fd;
      if (flags & F_CONNECTING && (FD_ISSET(fd, &rs) || FD_ISSET(fd, &ws))) {
        int error;
        n = sizeof(error);
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *) &n) < 0 || error != 0) {
          std::cerr << "nonblocking connect failed for " << file[i].ip << std::endl;
        }
        std::cout << "connection established for " << file[i].ip << " - " << file[i].fd << std::endl;
        FD_CLR(fd, &wset);
        if (send(fd, (void *) body, strlen(body), 0) <= 0) {
          std::cerr << "send failed" << std::endl;
          break;
        }
        file[i].flags = F_READING;
        FD_SET(file[i].fd , &rset);
        if (file[i].fd > maxfd) {
          maxfd = file[i].fd;
        }
      } else if (flags & F_READING && FD_ISSET(fd, &rs)) {
        char buf[65536];
        memset(buf, 0, 65536);
        if ((n = recv(fd, (char *) buf, sizeof(buf), 0)) == 0) {
          std::cout << "received all!!!" << std::endl;
          close(fd);
          file[i].flags = F_DONE;
          FD_CLR(fd, &rset);
          nconn--;
          nlefttoread--;
        } else {
          std::cout << "received: " << buf << " from: " << fd << std::endl;
        }
      }
    }
    */
  }

  return 0;
}

void event_handler(int fd, short event, void *arg)
{
  struct event *e = ((arg_t *) arg)->event;
  struct file *f = ((arg_t *) arg)->file;

  if (f->flags & F_CONNECTING) {
    int error;
    int n = sizeof(error);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *) &n) < 0 || error != 0) {
      std::cerr << "nonblocking connect failed for " << f->ip << std::endl;
    }
    std::cout << "connection established for " << f->ip << " - " << f->fd << std::endl;
    //FD_CLR(fd, &wset);
    char body[256];
    memset(body, 0, 256);
    strcpy(body, "vaio AND sony\n");
    if (send(fd, (void *) body, strlen(body), 0) <= 0) {
      std::cerr << "send failed" << std::endl;
      //break;
    }
    f->flags = F_READING;
    event_set(e, fd, EV_READ, event_handler, arg); // set event for read only
    event_add(e, NULL);
    //FD_SET(file[i].fd , &rset);
    //if (file[i].fd > maxfd) {
    //  maxfd = file[i].fd;
    //}

  } else if (f->flags & F_READING) {
    char buf[65536];
    memset(buf, 0, 65536);
    int n;
    if ((n = recv(fd, (char *) buf, sizeof(buf), 0)) == 0) {
      std::cout << "received all!!!" << fd << std::endl;
      close(fd);
      f->flags = F_DONE;
      //FD_CLR(fd, &rset);
      nconn--;
      nlefttoread--;
    } else {
      std::cout << "received: [" << buf << "] from: " << fd << std::endl;
      event_set(e, fd, EV_READ, event_handler, arg); // set event for read only
      event_add(e, NULL);
    }
  }
}
