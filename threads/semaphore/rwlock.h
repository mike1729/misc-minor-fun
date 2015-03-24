#ifndef H_RWLOCK
#define H_RWLOCK

#include "futex.h"
#include "xlock.h"
#include <cstdio>

struct msemaphore
{
    static const int MAX_READERS = 1<<30;

    msemaphore(int avail = 0) : avail(avail), waiters(0)
    {
    }

    void up()
    {
        int v = ++avail;
        if (waiters > 0)
            avail.wake(v);
    }

    void down()
    {
        int v;
        while (true)
        {
            v = avail;
            if (v <= 0)
            {
                waiters++;
                avail.wait(v);
                waiters--;
            }
            else if (avail.compare_exchange_strong(v, v-1))
                return;
        }
    }
    
    void disable()
    {
        int v;
        do  v = avail;
        while (!avail.compare_exchange_strong(v, v-MAX_READERS)) ;
        while (true)
        {
            v = avail;
            if (v!=0)
            {
                waiters++;
                avail.wait(v);
                waiters--;
            }
            else
                return;
        }
    }
    
    void enable()
    {
        rd_lock.unlock();
        avail.store(MAX_READERS);    
    }

    private:
    futex               avail;
    xlock               rd_lock;
    std::atomic<int>    waiters, old_readers;
};


struct rwlock
{
    static const int MAX_READERS = 1<<30;

    rwlock(): rd_semaphore(MAX_READERS)
    {
    }
    
    void lockR()
    {
       rd_semaphore.down();         
    }

    void unlockR()
    {
       rd_semaphore.up();
    }

    void lock()
    {
		wr_lock.lock();
		rd_semaphore.disable();
		
    }

    void unlock()
    {
    	rd_semaphore.enable();
		wr_lock.unlock();
    }
    
private:
    xlock wr_lock;
    msemaphore rd_semaphore;
};

#endif // H_RWLOCK
