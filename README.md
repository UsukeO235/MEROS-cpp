# MEROS-C++
Minimal Embedded Real-time OS written in C++

# How to port MEROS-C++ into your project
1. Modify 4 functions in merospp_port.hpp
    - disable_interrupt()
    - enable_interrupt()
    - store_stack_pointer()
    - restore_stack_pointer()

    Default functions are for STM32 Cortex M4.

1. Include merospp.hpp, then instantiate Tasks and the Scheduler

    ```cpp
    #include "merospp.hpp"
    
    // You can derive Task class like:
    class TaskDerived : public merospp::Task
    {
        public:
        TaskDerived( const uint32_t priority, const uint32_t period, void (*callback)(void) )
        : merospp::Task( priority, period, callback )
        {

        }
    };
    
    void callback1()
    {
        // do something
        // DO NOT PUT INFINITE LOOP inside
    }
    void callback2()
    {
        // do something
        // DO NOT PUT INFINITE LOOP inside
    }
    
    merospp::Task t1(5, 500, callback1);  // callback1 is called every 500ms
    TaskDerived t2(4, 1000, callback2);  // callback2 is called every 1000ms
    
    // Get the instance of Scheduler
    // Stack size: 64 words for t1, 128 words for t2
    auto& scheduler = merospp::Scheduler< 64, 128 >::get_instance( t1, t2 );
    ```

1. Place schedule() inside an interrupt handler that is called every 1 ms

    ```cpp
    void timer_callback()  // "HAL_TIM_PeriodElapsedCallback" for STM32
    {
        scheduler.schedule();
    }
    ```

1. Place run() inside main()

    ```cpp
    int main()
    {
        /* Peripheral setup start */
        
        /* Peripheral setup end */
        
        scheduler.run();
        return 0;
    }
    ```

1. Build your project
    
    Please refer to CMakeLists_sample.txt for recommended compile options.

# Acknowledgments
- [CMake example project for STM32 and CubeMX](https://github.com/idt12312/STM32_CMake)
