#ifndef __MEROSPP_H
#define __MEROSPP_H

#include "merospp_port.hpp"
#include <array>

namespace merospp
{

template< std::size_t... StackSize >
class Task
{
    private:
    const uint32_t PRIORITY;
    const uint32_t PERIOD;
    void (*const callback_)(void);

    bool initialized_;
    bool callback_executing_;
    uint32_t* stack_pointer_;
    uint32_t wait_counter_;

    std::array< uint32_t, sizeof...(StackSize) > stack_;

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