#ifndef fifo_h
#define fifo_h 

// fifo template
// synchronized with mutex and cond

#include <assert.h>
#include <errno.h>
#include <list>
#include <sys/time.h>
#include <time.h>

template<class T>
class fifo {
 public:
  fifo();
  ~fifo();
  void enq(T);
  T deq();
  void cleanup() {
    pthread_mutex_unlock(&m);
    waiting = false;
  }
 private:
  std::list<T> q;
  pthread_mutex_t m;
  pthread_cond_t c; // q went non-empty
  bool waiting;
  bool loop;
  void wait_for_lock();
  static void cleanup_lock(void *arg) {
    fifo<T> *f = (fifo<T> *) arg;
    f->cleanup();
  };
};

template<class T>
fifo<T>::fifo() : waiting(false), loop(true)
{
  assert(pthread_mutex_init(&m, 0) == 0);
  assert(pthread_cond_init(&c, 0) == 0);
}

template<class T>
fifo<T>::~fifo()
{
  wait_for_lock(); // make sure the thread calling recv has been fully cancelled
  int s;
  while((s = pthread_mutex_destroy(&m)) != 0 && errno == EAGAIN) {
#ifndef __APPLE__
    pthread_yield();
#else
    // pthread_yield isn't available on Mac
    pthread_yield_np();
#endif
  }
  if( s != 0 ) {
    perror( "FIFO errno" );
  }
  assert(s == 0);
  assert(pthread_cond_destroy(&c) == 0);
}

template<class T> void
fifo<T>::wait_for_lock() {
  loop = false;
  while(waiting) {
#ifndef __APPLE__
    pthread_yield();
#else
    // pthread_yield isn't available on Mac
    pthread_yield_np();
#endif
  }
}

template<class T> void
fifo<T>::enq(T e)
{
  assert(pthread_mutex_lock(&m) == 0);
  q.push_back(e);
  assert(pthread_cond_broadcast(&c) == 0);
  assert(pthread_mutex_unlock(&m) == 0);
}

template<class T> T
fifo<T>::deq()
{
  waiting = true;
  T e;
  assert(pthread_mutex_lock(&m) == 0);
  pthread_cleanup_push(&fifo::cleanup_lock, (void *) this);

  while(loop){
    if(q.empty()){
      assert (pthread_cond_wait(&c, &m) == 0);
    } else {
      e = q.front();
      q.pop_front();
      break;
    }
  }

  pthread_cleanup_pop(0);
  assert(pthread_mutex_unlock(&m) == 0);
  waiting = false;
  return e;
}

#endif
