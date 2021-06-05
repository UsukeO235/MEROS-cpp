# MEROS-C++
Minimal Embedded Real-time OS written in C++

# How to port MEROS-C++ into your project
1. Modify 4 functions in merospp_port.hpp
    - disable_interrupt()
    - enable_interrupt()
    - store_stack_pointer()
    - restore_stack_pointer()
1. Include merospp.hpp 