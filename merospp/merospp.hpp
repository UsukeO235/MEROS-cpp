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

template< std::size_t... StackSize >
class Scheduler
{
    private:
    template< typename Head >
    constexpr static std::size_t total_stack_size( Head&& head )
    {
        return head;
    }

    template< typename Head, typename... Tail >
    constexpr static std::size_t total_stack_size( Head&& head, Tail&&... tail )
    {
        return head + total_stack_size( tail... );
    }
    
    static_assert( total_stack_size(StackSize...) <= MAXIMUM_TOTAL_STACK_SIZE, "Total stack size exceeds MAXIMUM_TOTAL_STACK_SIZE" );

    volatile std::size_t current_task_number_ = 0;
    std::array< Task*, sizeof...(StackSize) > task_ptrs_;
    std::array< bool, sizeof...(StackSize)+1 > tasks_initialized_;

    //std::array< uint32_t, total_stack_size(StackSize...) > stack_;
    //std::array< uint32_t, SYSTEM_STACK_SIZE > kernel_stack_;  // 後々stackに結合したい
    std::array< uint32_t, total_stack_size(StackSize...) + SYSTEM_STACK_SIZE > stack_;

    std::array< uint32_t*, sizeof...(StackSize)+1 > sp_vals_;

    std::array< uint32_t, sizeof...(StackSize) > wait_counters_;

    /*
    template< typename T >
    constexpr T* get_pointer( T& d )
    {
        return &d; 
    }
    */
    template< typename... Args >
    constexpr std::array< Task*, sizeof...(Args) > make_array_of_pointers( Args&&... args )
    {
        //return {get_pointer(args)...};
        return {static_cast<Task*>(&args)...};
    }

    private:

    template< typename... T >
    Scheduler( T&... args )
    : task_ptrs_( make_array_of_pointers(args...) )
    {
        //static_assert( 1 <= sizeof...(T) && sizeof...(T) <= 32, "N must be between 1 and 32" );
        //static_assert( sizeof...(args) == sizeof...(T), "Number of tasks must be equal to N" );
        
        //static_assert( total_stack_size(args...) <= MAXIMUM_TOTAL_STACK_SIZE, "" );
        //constexpr std::size_t x = total_stack_size(args...);

        //expand( task_ptrs_.data(), args... );  // 再帰的にパラメータを展開する
        
        std::array< std::size_t, sizeof...(StackSize) > stack_sizes = {StackSize...};

        sp_vals_[0] = &(stack_[SYSTEM_STACK_SIZE-1]);
        for( std::size_t i = 0; i < sizeof...(StackSize); i ++ )
        {
            if( i == 0 )
            {
                //sp_vals_[1] = stack_.data() + (stack_sizes[i] - 1);
                sp_vals_[1] = stack_.data() + (SYSTEM_STACK_SIZE + stack_sizes[i] - 1);
            }
            else
            {
                //sp_vals_[i+1] = sp_vals_[i] + (stack_sizes[i] - 1);
                sp_vals_[i+1] = sp_vals_[i] + (stack_sizes[i] - 1);
            }

            wait_counters_[i] = 1;
        }

        for( std::size_t i = 0; i < sizeof...(StackSize)+1; i ++ )
        {
            tasks_initialized_[i] = false;
        }
    }

    public:

    // Compile with -fno-threadsafe-statics option
    template< typename... T >
    static Scheduler< StackSize... >& get_instance( T&... args )
    {
        static Scheduler< StackSize... > scheduler( args... );
        return scheduler;
    }

    Scheduler< StackSize... >( const Scheduler< StackSize... >& ) = delete;
    Scheduler< StackSize... >( Scheduler< StackSize... >&& ) = delete;
    Scheduler< StackSize... >& operator=( const Scheduler< StackSize... >& ) = delete;
    Scheduler< StackSize... >& operator=( const Scheduler< StackSize... >&& ) = delete;

    void schedule()
    {
        disable_interrupt();
        store_stack_pointer( &sp_vals_[current_task_number_] );  // スタックポインタ保存
        
        restore_stack_pointer( &sp_vals_[0] );  // スタックポインタ復帰
        enable_interrupt();  // RTOS以外の割り込みを許可

        // tasks_initialized_[0]は常にfalse
        if( current_task_number_ == 0 )
        {
            current_task_number_ ++;
        }
        else
        {
            if( tasks_initialized_[current_task_number_] == false )
            {
                tasks_initialized_[current_task_number_] = true;
                if( current_task_number_ < sizeof...(StackSize) )
                {
                    current_task_number_ ++;
                }   
            } 
        }

        // 全てのタスクが初期化されるまでは順番にタスクを切り替える
        if( tasks_initialized_[current_task_number_] == false )  // タスクがまだ初期化されていないとき
        {
            // スタックの内容をコピー
            for( std::size_t i = 0; i < SYSTEM_STACK_SIZE; i ++ )
            {
                // sp_vals_[current_task_number_] : End of each task's stack
                *(sp_vals_[current_task_number_] - i) = stack_[SYSTEM_STACK_SIZE-1-i];
            }
            // スタックポインタのオフセット量をコピー
            sp_vals_[current_task_number_] -= (&stack_[SYSTEM_STACK_SIZE-1]-sp_vals_[0]);  // sp_vals_[0]==&kernel_stack_[127]-x
        }
        else  // 全タスク初期化後
        {
            volatile std::size_t next_task_number = 0;
            // タスクが優先度昇順にソートされていることを前提とする
            for( std::size_t i = 0; i < sizeof...(StackSize); i ++ )
            {
                if( wait_counters_[i] != 0 )
                {
                    wait_counters_[i] --;
                }

                if( wait_counters_[i] == 0 )
                {
                    if( next_task_number == 0 )
                    {
                        next_task_number = i+1;
                    }
                    else
                    {
                        if( task_ptrs_[i]->get_priority() > task_ptrs_[next_task_number-1]->get_priority() )
                        {
                            next_task_number = i+1;
                        }
                    }
                }
            }
            current_task_number_ = next_task_number;  // タスク切り替え
        }

        disable_interrupt();
        store_stack_pointer( &sp_vals_[0] );  // スタックポインタ保存

        restore_stack_pointer( &sp_vals_[current_task_number_] );  // スタックポインタ復帰
        enable_interrupt();
    }

    void run()
    {
        // リセットシーケンス
        disable_interrupt();
        restore_stack_pointer( &sp_vals_[0] );  // スタックポインタの値を書き換える
        enable_interrupt();

        while( tasks_initialized_[current_task_number_] == false );
        while( true )
        {
            task_ptrs_[current_task_number_-1]->call();
            // wait
            wait_counters_[current_task_number_-1] = task_ptrs_[current_task_number_-1]->get_period();
            while( wait_counters_[current_task_number_-1] != 0 );
        }
    }
};

}

#endif