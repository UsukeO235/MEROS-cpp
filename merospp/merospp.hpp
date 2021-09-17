#ifndef __MEROSPP_H
#define __MEROSPP_H

#include "merospp_port.hpp"
#include <array>

namespace merospp
{

class Task
{
    private:
    const uint32_t PRIORITY;
    const uint32_t PERIOD;
    void (*const callback_)(void);

    public:
    Task() = delete;
    Task( const uint32_t priority, const uint32_t period, void (*callback)(void) )
    : PRIORITY( priority )
    , PERIOD( period )
    , callback_( callback )
    {
        
    }

    inline uint32_t get_priority() const
    {
        return PRIORITY;
    }

    inline uint32_t get_period() const
    {
        return PERIOD;
    }

    inline void call()
    {
        (*callback_)();
    }
};


}

#endif