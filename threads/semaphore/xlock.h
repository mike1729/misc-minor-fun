#ifndef H_XLOCK
#define H_XLOCK

#include "futex.h"

struct xlock : private futex
{
    void lock()
    {
        int v = UNLOCKED;
        if (!compare_exchange_strong(v, LOCKED))
        {
            if (v != SLEEPING)
                v = exchange(SLEEPING);
            while (v != UNLOCKED)
            {
                wait(SLEEPING);
                v = exchange(SLEEPING);
            }
        }
    }

    void unlock()
    {
        int v;
        if ((v = fetch_sub(1)) != LOCKED)
        {
            store(UNLOCKED);
            wake(1);
        }
    }

    private:

    enum { UNLOCKED = 0, LOCKED, SLEEPING };
};

#endif // H_XLOCK
