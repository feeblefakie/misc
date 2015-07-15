/* gcc signal_recv.c -o signal_recv -g -W -Wall -lpthread */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
struct {
  int    sigid;
  char * sigmess;
} siginfo[] = {
  {   SIGINT,   "signal_func: 'SIGINT'  Recv.",  },
  {   SIGQUIT,  "signal_func: 'SIGQUIT' Recv.",  },
  {   SIGTERM,  "signal_func: 'SIGTERM' Recv.",  },
};

#define array( a )  ( sizeof( a )/sizeof( a[0] ))
void *signal_func( void * arg );
void *worker_func( void * arg );
pthread_t id[2];
int end_flg = 0;

int main( ) {
  sigset_t    ss;
  size_t      i;

  sigemptyset( &ss );
  for( i = 0; i < array( siginfo ); i ++ ) {
    sigaddset( &ss, siginfo[i].sigid );
  }
  sigprocmask( SIG_BLOCK, &ss, 0 );

  pthread_create( &id[0], 0, &signal_func, 0 );
  pthread_create( &id[1], 0, &worker_func, 0 );
  pthread_join( id[0], 0 );
  pthread_join( id[1], 0 );
  return 0;
}

void * signal_func( void * arg ) {

  ( void )arg;
  sigset_t    ss;
  int         sig;
  size_t      i;
  pthread_detach( pthread_self( ));

  sigemptyset( &ss );
  for( i = 0; i < array( siginfo ); i ++ ) {
    sigaddset( &ss, siginfo[i].sigid );
  }
  pthread_sigmask( SIG_BLOCK, &ss, 0 );

  while( end_flg == 0 ) {
    if( sigwait( &ss, &sig )) {
      printf( "not SIGINT signal!!\n" );
      continue;
    }
    for( i = 0; i < array( siginfo ); i ++ ) {
      if( siginfo[i].sigid != sig ) {
        continue;
      }
      printf( "%s (%2d) \n", siginfo[i].sigmess, sig );
      break;
    }
  }
  return 0;
}

void * worker_func( void * arg ) {

  ( void )arg;
  int i = 0;
  for( i = 0; i < 10; i ++ ) {
    puts( "in worker_func..." );
    sleep( 2 );
  }
  end_flg = 1;
  pthread_kill( id[0], SIGTERM );
  return 0;
}

