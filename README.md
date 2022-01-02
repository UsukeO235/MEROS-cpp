# MEROS-cpp
Minimal Embedded Real-time OS written in C++

## Features
- C++ interface
- Static memory allocation: Stack area of each task is statically allocated
- Easy to port: Few knowledge of Assembly language required

## Requirements
- Compiler: arm-none-eabi-gcc, arm-none-eabi-g++ 4.9.3

## How to Port MEROS-cpp into your project
1. Include merospp.hpp in main.cpp
    ```cpp
    #include "merospp.hpp"

    int main( void )
    {
        return 0;
    }
    ```

1. Create tasks
    ```cpp
    void callback1()
    {
        // do something
        // DO NOT write infinite loop
    }
    
    void callback2()
    {
        // do something
        // DO NOT write infinite loop
    }
    
    merospp::Task<64> t1( 5, 500, callback1 );
    merospp::Task<128> t2( 4, 1000, callback2 );
    ```
    
1. Put schedule() in an interrupt handler
    ```cpp
    void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef *htim )  // called every 0.1ms (10kHz)
    {
        if( htim->Instance == TIM17 )
        {
            merospp::schedule();
        }
    }
    ```
    
1. Register tasks and call run() inside main()
    ```cpp
    int main( void )
    {
        /* Peripheral Initialization */
        
        /* ************************* */
        
        merospp::disable_interrupt();
        merospp::register_tasks( t1, t2 );
        merospp::run();
        merospp::enable_interrupt();
        
        return 0;  // The program does not reach here
    }
    ```

## Acknowledgements
- [CMake example project for STM32 and CubeMX](https://github.com/idt12312/STM32_CMake)
