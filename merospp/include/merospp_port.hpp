#ifndef __MEROSPP_PORT_H
#define __MEROSPP_PORT_H

#include <cstdint>
#include "stm32f3xx_hal.h"

namespace merospp
{
constexpr std::size_t MAXIMUM_NUMBER_OF_TASKS = 8;
constexpr std::size_t TOTAL_STACK_SIZE = 256;
constexpr std::size_t SYSTEM_STACK_SIZE = 48;

inline void disable_interrupt()
{
    __disable_irq();
}

inline void enable_interrupt()
{
    __enable_irq();
}

inline void store_stack_pointer( uint32_t** const stack_pointer )
{
    asm volatile( "mov r0, %[in]\n\t" :: [in]"r"(stack_pointer) );
    asm volatile( "str r13, [r0]\n\t" );
}

inline void restore_stack_pointer( uint32_t** const stack_pointer )
{
    asm volatile( "mov r0, %[in]\n\t" :: [in]"r"(stack_pointer) );
    asm volatile( "ldr r13, [r0]\n\t" );
}

inline bool try_lock( uint32_t* p )
{
    return __sync_bool_compare_and_swap( p, 0, 1 );
}
/*
inline bool try_lock( uint32_t* p )
{
    disable_interrupt();
    return true;
}
*/
inline void release_lock( uint32_t* p )
{
    *p = 0;
}
/*
inline void release_lock( uint32_t* p )
{
    enable_interrupt();
}
*/
}

#endif