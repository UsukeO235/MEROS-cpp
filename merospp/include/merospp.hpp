#ifndef __MEROSPP_H
#define __MEROSPP_H

#include "merospp_port.hpp"
#include <array>

namespace merospp
{

class TaskImpl
{
    friend bool register_tasks_impl( TaskImpl**, const std::size_t );
    friend void run();
    friend void schedule();

    private:
    const std::size_t STACK_SIZE;
    const uint32_t PRIORITY;
    const uint32_t PERIOD;
    void (*const callback_)(void);

    bool initialized = false;
    bool callback_executing = false;
    uint32_t* stack_pointer = nullptr;
    uint32_t wait_counter = 1;

    public:
    TaskImpl() = delete;
    TaskImpl( const std::size_t stack_size, uint32_t priority, const uint32_t period, void (*callback)(void) )
    : STACK_SIZE( stack_size )
    , PRIORITY( priority )
    , PERIOD( period )
    , callback_( callback )
    {

    }

    inline std::size_t get_stack_size_impl()
    {
        return STACK_SIZE;
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

template< std::size_t StackSize >
class Task : public TaskImpl
{
    friend bool register_tasks_impl( TaskImpl**, const std::size_t );
    friend void run();
    friend void schedule();

    private:
    
    public:
    Task()
    {

    }
    Task( const uint32_t priority, const uint32_t period, void (*callback)(void) )
    : TaskImpl( StackSize, priority, period, callback )
    {
        
    }

    constexpr static std::size_t get_stack_size()
    {
        return StackSize;
    }
};

bool register_tasks_impl( TaskImpl** task_ptr, const std::size_t number_of_tasks );
void run();
void schedule();

template< typename Head >
constexpr static std::size_t total_stack_size( Head&& head )
{
    return head.get_stack_size();
}

template< typename Head, typename... Tail >
constexpr static std::size_t total_stack_size( Head&& head, Tail&&... tail )
{
    return head.get_stack_size() + total_stack_size( tail... );
}

template< typename... T >
bool register_tasks( T&... args )
{
    static_assert( sizeof...(args) < MAXIMUM_NUMBER_OF_TASKS, "" );
    static_assert( total_stack_size(args...) <= TOTAL_STACK_SIZE-SYSTEM_STACK_SIZE, "" );
    
    return register_tasks_impl( &(std::array< TaskImpl*, sizeof...(args) >{static_cast<TaskImpl*>(&args)...}[0]), sizeof...(args) );
}

}

#endif