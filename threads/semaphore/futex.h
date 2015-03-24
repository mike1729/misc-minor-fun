#ifndef H_FUTEX
#define H_FUTEX

#include <atomic>
#include <linux/futex.h>
#include <sys/syscall.h>

struct futex : public std::atomic<int>
{
    futex(int value = 0) : std::atomic<int>(value)
    {
    }

    using std::atomic<int>::operator=;

    int wait(int expected) const
    {
        return syscall(__NR_futex, this, FUTEX_WAIT, expected, 0);
    }

    int wake(int count) const
    {
        return syscall(__NR_futex, this, FUTEX_WAKE, count);
    }
};

#endif // H_FUTEX
