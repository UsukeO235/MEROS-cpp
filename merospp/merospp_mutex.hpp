#ifndef __MEROSPP_MUTEX_H
#define __MEROSPP_MUTEX_H

#include "merospp_port.hpp"

namespace merospp
{
class Mutex
{
    private:
    uint32_t lock_state_ = 0;

    public:
    bool try_lock()
    {
        return merospp::try_lock( &lock_state_ );
    }

    void release_lock()
    {
        merospp::release_lock( &lock_state_ );
    }
};
}

#endif