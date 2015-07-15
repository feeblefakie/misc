/*
 * Compile with:
 * g++ -I/usr/local/include -o event-test event-test.cpp -L/usr/local/lib -levent
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <iostream>

#include <event.h>

class Test;

typedef struct {
  Test *t;
  struct event *e;
} arg_t;

class Test {
public:
  Test(void)
  : val(10)
  {}

  static void run(int fd, short event, void *instance)
  {
    ((Test *) (((arg_t *) instance)->t))->print(fd, event, ((arg_t *) instance)->e);
  }

  void print(int fd, short event, void *arg)
  {
    std::cout << "printing!!! " << val << std::endl;
    char buf[255];
    int len;
    struct event *ev = (struct event *) arg;

    /* Reschedule this event */
    event_add(ev, NULL);

    fprintf(stderr, "fifo_read called with fd: %d, event: %d, arg: %p\n", fd, event, (struct event *) arg);
    len = read(fd, buf, sizeof(buf) - 1);

    if (len == -1) {
      perror("read");
      return;
    } else if (len == 0) {
      fprintf(stderr, "Connection closed\n");
      return;
    }

    buf[len] = '\0';
    fprintf(stdout, "Read: %s\n", buf);
  }

  void set_event(void)
  {
    struct event evfifo;
    struct stat st;
    const char *fifo = "event.fifo";
    int socket;
   
    if (lstat (fifo, &st) == 0) {
      if ((st.st_mode & S_IFMT) == S_IFREG) {
        errno = EEXIST;
        perror("lstat");
        exit (1);
      }
    }

    unlink (fifo);
    if (mkfifo (fifo, 0600) == -1) {
      perror("mkfifo");
      exit (1);
    }

    /* Linux pipes are broken, we need O_RDWR instead of O_RDONLY */
    socket = open (fifo, O_RDWR | O_NONBLOCK, 0);

    if (socket == -1) {
      perror("open");
      exit (1);
    }

    fprintf(stderr, "Write data to %s\n", fifo);
    /* Initalize the event library */
    event_init();

    arg_t arg;
    arg.t = this;
    arg.e = &evfifo;
    event_set(&evfifo, socket, EV_READ, Test::run, &arg);

    event_add(&evfifo, NULL);
    event_dispatch();
  }

private:
  int val;
};


int
main (int argc, char **argv)
{
  Test t;

  t.set_event();

	return (0);
}
