#ifndef LOCKER
#define LOCKER

#include<semaphore.h>
#include<pthread.h>
#include<exception>
class sema{
    public:
        sema(){
            if(sem_init(&sem,0,0)){
                throw std::exception();
            }
        }
        ~sema(){
            sem_destroy(&sem);
        }
        bool wait(){
            return sem_wait(&sem)==0;
        }
        bool post(){
            return sem_post(&sem)==0;
        }
    private:
        sem_t sem;
};
class mutex_lock{
    public:
        mutex_lock(){
            if(pthread_mutex_init(&v_mutex,NULL)){
                throw std::exception();
            }
        }
        ~mutex_lock(){
            pthread_mutex_destroy(&v_mutex);
        }
        bool lock(){
            return pthread_mutex_lock(&v_mutex)==0;
        }
        bool unlock(){
            return pthread_mutex_unlock(&v_mutex)==0;
        }
    private:
        pthread_mutex_t v_mutex;
};

#endif