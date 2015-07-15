#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#define URLBUF 1024
#define HEADER_BUF 4096
#define MAX_URL_NUM 1000000
#define DEFAULT_SLOW_THRESHOLD 1.0
#define DEFAULT_HTTP_VERSION "1.0"

struct proc_table {
	int pid;
	int req_cnt;
	int err_cnt;
	struct proc_table *nextp;
};

struct stats {
	unsigned long request_cnt;
	unsigned long send_bytes;
	unsigned long recv_bytes;
	unsigned long fail_cnt;
  double ave_resp_time;
};

struct params {
    unsigned int proc_cnt;
    unsigned int reqs_per_p;
    unsigned int slow_query_enabled;
    double resp_slow_threshold;
    char *req_file;
    char *header_conf_file;
    char *http_version;
    char hostname[URLBUF];
};

static void wait_child(int sig);
static void intr(int sig);
static void child_term(int sig);
static int get_urls(struct params *p);
char *get_header_str(const char *header_conf_file);
static void usage(void);
static int get_options(int argc, char *argv[], struct params *p);
static int http_request(struct hostent *hp, char *path, int *send, int *recv,
                        char *header_str, struct params *param);
static void child_proc(int child_id, struct hostent *hp, char *header_str, struct params *param);
static void parent_proc(struct params *p);
double gettimeofday_sec(void);

int termf;
static struct stats *score;
static struct proc_table *headp;
static char *target_url[MAX_URL_NUM];
static unsigned int urlcount = 1;

int main(int argc, char **argv)
{
  int pid, ppid;
  int i;
  struct hostent *hp;
  caddr_t m;
  char *header_str;

  /* get command line arguments */
  struct params param;
  if (get_options(argc, argv, &param) < 0) {
    exit(1); 
  }

  /* headp points to the NULL first */
  headp = (struct proc_table *) NULL;

  /* get target urls to access */
  if (get_urls(&param) < 0) {
      exit(1);
  }

  signal(SIGCHLD, wait_child);
  signal(SIGINT, intr);
  ppid = getpid();

  printf("process = %d, reqs/p = %d\n", param.proc_cnt, param.reqs_per_p);
  printf("hostname [%s]\n", param.hostname);

  hp = gethostbyname(param.hostname);
  if (!hp) {
    exit(1);
  }

  header_str = get_header_str(param.header_conf_file);

  m = mmap((caddr_t) 0,  sizeof(struct stats) * param.proc_cnt, 
           PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
  if (m == (caddr_t) -1) {
    perror("mmap");
    exit(1);
  }
  score = (struct stats *) m;
  memset(score, 0, sizeof(struct stats) * param.proc_cnt);

  for(i = 0; i < (int) param.proc_cnt; i++) {
    pid = fork();
    if (pid == 0) { /* Child Process */

      /* child main: sending http requests */
      child_proc(i, hp, header_str, &param);
      exit(0);

    } else if (pid > 0) { /* Parent Process */
      struct proc_table *p;

      if (!headp) {
        headp = malloc(sizeof(struct proc_table));
        memset(headp, 0, sizeof(struct proc_table));
        p = headp;
      } else {
        p->nextp = malloc(sizeof(struct proc_table));
        memset(p->nextp, 0, sizeof(struct proc_table));
        p = p->nextp;
      }
      p->pid = pid;
    } else { /* fork error */
      perror("fork");
    }
  }

  /* parent main: add up the requests */
  parent_proc(&param);

  munmap(m, sizeof(struct stats) * param.proc_cnt);
    
  return 0;
}

int
http_request(struct hostent *hp, char *path, int *send, int *recv,
             char *header_str, struct params *param)
{
  char sendbuf[URLBUF], recvbuf[256];
  struct sockaddr_in sin;
  int s;
  int ret;
  int on;

  s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (s <= 0) {
    return -1;
  }

  memset(&sin, 0, sizeof(sin));
  memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
  sin.sin_port = htons(80);
  sin.sin_family = AF_INET;
  if (connect(s, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
    close(s);
    return -1;
  }

  if (header_str == NULL) {
    sprintf(sendbuf, "GET %s HTTP/%s\n\n", path, param->http_version);
  } else {
    sprintf(sendbuf, "GET %s HTTP/%s\n%s\n\n", path, param->http_version, header_str);
  }

  ret = write(s, sendbuf, strlen(sendbuf));
  *send = ret;
  if (ret <= 0) {
    close(s);
    return -1;
  }

  while((ret = read(s, recvbuf, (sizeof(recvbuf) - 1))) > 0) {
    *recv += ret;
    recvbuf[ret] = '\0';
  }
  close(s);

  return 1;
}

void
wait_child(int sig)
{
  struct proc_table *p;
  struct proc_table *tmp;
  int pid;

  while((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
    if (headp && headp->pid == pid) {
      tmp = headp->nextp;
      free(headp);
      headp = tmp;
    } else if (headp) {
      for(p = headp; p; p = p->nextp) {
        if (p->nextp && pid == p->nextp->pid) {
          tmp = p->nextp->nextp;
          free(p->nextp);
          p->nextp = tmp;
          break;
        }
      }
    }
    if (pid == -1) {
      perror("wait");
      exit(1);
    }
  }
  signal(SIGCHLD, wait_child);
}

void 
intr(int sig)
{
  struct proc_table *p;

  for(p = headp; p; p = p->nextp) {
    kill(SIGTERM, p->pid);
  }
  termf = 1;
  headp = NULL;
}

void child_term(int sig)
{
	termf = 1;
}

int 
get_options(int argc, char *argv[], struct params *p)
{
  char ch;

  p->resp_slow_threshold = DEFAULT_SLOW_THRESHOLD;
  p->slow_query_enabled = 0;
  p->http_version = DEFAULT_HTTP_VERSION;
  while((ch = getopt(argc, argv, "p:c:i:t:sh:v:")) != EOF) {
    switch(ch) {
      case 'p':
        p->proc_cnt = atoi(optarg);
        break;
      case 'c':
        p->reqs_per_p = atoi(optarg);
        break;
      case 'i':
        p->req_file = optarg;
        break;
      case 't': // threshold for response time
        p->resp_slow_threshold = atof(optarg);
        break;
      case 's':
        p->slow_query_enabled = 1;
        break;
      case 'h':
        p->header_conf_file = optarg;
        break;
      case 'v':
        p->http_version = optarg;
        break;
      default:
        usage();
        return -1;
    }
  }
  argc -= optind;
  argv += optind;
  if (*argv == NULL) {
    usage();
    return -1;
  } else {
    strcpy(p->hostname, *argv);
  }
  return 0;
}

static int
get_urls(struct params *p)
{
  FILE *fp;

  fp = fopen(p->req_file, "r");
  if (!fp) {
    fprintf(stderr, "%s is not found. requesting root ...\n", p->req_file); 
    target_url[0] = "/";
    urlcount = 1;
  } else {
    char line[URLBUF];
    char *cp;
    int cnt = 0;

    while(fgets(line, URLBUF, fp) != NULL) {
      if (line[0] != '\n') {
        target_url[cnt] = (char *) malloc(strlen(line) + 1);
        cp = (char *) strrchr(line, '\n');
        if (cp) {
          *cp = '\0';
        } else {
          fprintf(stderr, "line buffer overflows. consider changing the buffer size.\n");
          continue; 
        }
        strcpy(target_url[cnt], line);
        cnt++;
      }
    }
    urlcount = cnt;
    fclose(fp);

    if (urlcount < (p->reqs_per_p * p->proc_cnt)) {
      fprintf(stderr, "need more than %d urls for these parameters.\n", 
              (p->reqs_per_p * p->proc_cnt));
      return -1;
    }
  }
  return 0;
}

void
child_proc(int child_id, struct hostent *hp, char *header_str, struct params *param)
{
  struct stats c_stat;
  struct timeval tv1_child, tv2_child;
  struct timeval tv1_req, tv2_req;
  int req_cnt, send_byte, recv_byte;
  double req_per_s;
  long sec;
  double total_resp_time = 0.0;
  char sq_log[64];
  char *path;
  FILE *fp;
  unsigned int counter = 0;
  int start_p;

  if (param->slow_query_enabled) {
    sprintf(sq_log, "slow_queries.log.%d", child_id);
    fp = fopen(sq_log, "w");
    if (!fp) {
      fprintf(stderr, "can't open %s for write.\n", sq_log);
    }
  }

  termf = 0;
  signal(SIGTERM, child_term);
  req_cnt = send_byte = recv_byte = 0;
  memset(&c_stat, 0, sizeof(struct stats));

  double t1 = gettimeofday_sec();
  start_p = param->reqs_per_p * child_id;
  while(1) {
    /* send request message */
    if (urlcount == 1) {
      path = target_url[0];
    } else {
      path = target_url[start_p + counter];
    }
    ++counter;
    double t3 = gettimeofday_sec();
    if (http_request(hp, path, &send_byte, &recv_byte, header_str, param) > 0) {
      c_stat.request_cnt++;
      c_stat.send_bytes += send_byte;
      c_stat.recv_bytes += recv_byte;
      send_byte = 0; recv_byte = 0;
    } else {
      c_stat.fail_cnt++;
    }
    double t4 = gettimeofday_sec();
    double resp_time = t4 - t3;
    if (param->slow_query_enabled) {
      if (resp_time > param->resp_slow_threshold) {
        // logging slow queries
        char buf[256];
        sprintf(buf, "slow query [%s] takes %lf (s)\n", path, resp_time);
        fwrite(buf, sizeof(char), strlen(buf), fp);
      }
    }
    total_resp_time += resp_time;
    c_stat.ave_resp_time = total_resp_time / c_stat.request_cnt;
    memcpy(&score[child_id], &c_stat, sizeof(struct stats));
    usleep(10);

    if (param->reqs_per_p == counter) {
      fflush(fp);
      break;
    }
  }
  double t2 = gettimeofday_sec();
  sec = t2 - t1;
  req_per_s = (double) ((double) score[child_id].request_cnt / (double) sec);
  
  return;
}

void
parent_proc(struct params *p)
{
  struct timeval tv1, tv2;
  struct stats p_stat;
  int last = 0;

  gettimeofday(&tv1, 0);
  printf("request  \tsend_byte\trecv_byte\tfailed   \treq/sec\t  ave_latency\n");
  while (1) {
    if (!headp || termf) {
      last = 1;
    }
    unsigned int h;
    memset(&p_stat, 0, sizeof(struct stats));
    for (h = 0; h < p->proc_cnt; h++) {
      p_stat.request_cnt += score[h].request_cnt;
      p_stat.send_bytes += score[h].send_bytes;
      p_stat.recv_bytes += score[h].recv_bytes;
      p_stat.fail_cnt += score[h].fail_cnt;
      p_stat.ave_resp_time += score[h].ave_resp_time;
    }
    p_stat.ave_resp_time /= p->proc_cnt;
    gettimeofday(&tv2, 0);
    if (!termf) {
      long sec = tv2.tv_sec - tv1.tv_sec;

      printf("%-8d\t%-8d\t%-8d\t%-8d\t%6.2f\t%6.2lf\r", 
             (int) p_stat.request_cnt, 
             (int) p_stat.send_bytes, 
             (int) p_stat.recv_bytes, 
             (int) p_stat.fail_cnt,
             sec ? (double) ((double)p_stat.request_cnt / (double) sec) : 0,
             (double) p_stat.ave_resp_time);
    }
    fflush(stdout);
    if (last) {
      break;
    }
    usleep(1000);
  }
  printf("\n"); 

  return;
}

char *
get_header_str(const char *header_conf_file)
{
  FILE *fp;
  char *header_str;

  fp = fopen(header_conf_file, "r");
  if (!fp) {
    return NULL;
  } else {
    header_str = (char *) malloc(sizeof(char) * HEADER_BUF);
    fread(header_str, HEADER_BUF, 1, fp);
    return header_str;
  }
}

double gettimeofday_sec(void)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

void
usage(void)
{
  fprintf(stderr, "Usage: bench [-p procs] [-c reqs/proc] [-i req_file] [-s] [-t resp_threshold] [-h header_conf_file] hostname\n\n");
  fprintf(stderr, "-p\t the number of processes.\n");
  fprintf(stderr, "-c\t the number of requests per process.\n");
  fprintf(stderr, "-i\t request file.\n");
  fprintf(stderr, "-s\t enable slow query.\n");
  fprintf(stderr, "-t\t threshold for response time.\n");
  fprintf(stderr, "-h\t request header configuration file.\n");
  fprintf(stderr, "-v\t HTTP version (1.0 in default).\n");
}
