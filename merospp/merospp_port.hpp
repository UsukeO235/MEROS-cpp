#ifndef __MEROSPP_PORT_H
#define __MEROSPP_PORT_H

#include "stm32f3xx_hal.h"

namespace merospp
{
constexpr std::size_t MAXIMUM_TOTAL_STACK_SIZE = 1024;
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

}

#endif