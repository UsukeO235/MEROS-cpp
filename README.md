# MEROS-C++
Minimal Embedded Real-time OS written in C++

# How to port MEROS-C++ into your project
1. Modify 4 functions in merospp_port.hpp
    - disable_interrupt()
    - enable_interrupt()
    - store_stack_pointer()
    - restore_stack_pointer()

    Default functions are for STM32 Cortex M4.

1. Include merospp.hpp
1. Build your project
    
    Please refer to CMakeLists_sample.txt for recommended compile options.

# Acknowledgments
- [CMake example project for STM32 and CubeMX](https://github.com/idt12312/STM32_CMake)
