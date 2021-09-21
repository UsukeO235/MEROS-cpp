#include "merospp.hpp"
#include "merospp_port.hpp"

namespace merospp
{

static Task< SYSTEM_STACK_SIZE > dummy_task( 0, 0, nullptr );

static bool stack_allocated = false;
static std::array< uint32_t, TOTAL_STACK_SIZE > stack;
static std::array< TaskImpl*, MAXIMUM_NUMBER_OF_TASKS+1 > task_ptrs;

static volatile std::size_t current_task_number = 0;
static std::size_t number_of_tasks = 1;


bool register_tasks_impl( TaskImpl** task_ptr, const std::size_t number_of_user_tasks )
{
    if( stack_allocated == true )
    {
        return false;
    }
    else
    {
        (&dummy_task)->stack_pointer = &(stack[0]) + (&dummy_task)->get_stack_size_impl();
        task_ptrs[0] = &dummy_task;

        for( std::size_t i = 0; i < number_of_user_tasks; i ++ )
        {
            if( i == 0 )
            {
                (*(task_ptr+i))->stack_pointer = (&dummy_task)->stack_pointer + (*(task_ptr+i))->get_stack_size_impl();
            }
            else
            {
                (*(task_ptr+i))->stack_pointer = (*(task_ptr+i-1))->stack_pointer + (*(task_ptr+i))->get_stack_size_impl();
            }

            task_ptrs[i+1] = (*(task_ptr+i));
        }

        stack_allocated = true;
        number_of_tasks = number_of_user_tasks + 1;
        
        return true;
    }
}

void run()
{
    // リセットシーケンス
    disable_interrupt();
    restore_stack_pointer( &(dummy_task.stack_pointer) );  // スタックポインタの値を書き換える
    enable_interrupt();

    while( task_ptrs[current_task_number]->initialized == false );
    while( true )
    {
        task_ptrs[current_task_number]->callback_executing = true;
        task_ptrs[current_task_number]->call();
        task_ptrs[current_task_number]->callback_executing = false;

        // wait
        while( task_ptrs[current_task_number]->wait_counter != 0 );
        
        task_ptrs[current_task_number]->wait_counter = task_ptrs[current_task_number]->get_period();
    }
}

void schedule()
{
    disable_interrupt();
    store_stack_pointer( &(task_ptrs[current_task_number]->stack_pointer) );  // スタックポインタ保存
    
    restore_stack_pointer( &(dummy_task.stack_pointer) );  // スタックポインタ復帰
    enable_interrupt();  // RTOS以外の割り込みを許可

    // tasks_initialized_[0]は常にfalse
    if( current_task_number == 0 )
    {
        current_task_number ++;
    }
    else
    {
        if( task_ptrs[current_task_number]->initialized == false )
        {
            task_ptrs[current_task_number]->initialized = true;
            if( current_task_number < number_of_tasks-1 )
            {
                current_task_number ++;
            }   
        } 
    }

    // 全てのタスクが初期化されるまでは順番にタスクを切り替える
    if( task_ptrs[current_task_number]->initialized == false )  // タスクがまだ初期化されていないとき
    {
        // スタックの内容をコピー
        for( std::size_t i = 0; i < SYSTEM_STACK_SIZE; i ++ )
        {
            // sp_vals_[current_task_number_] : End of each task's stack
            *(task_ptrs[current_task_number]->stack_pointer - i) = stack[SYSTEM_STACK_SIZE-1-i];
        }
        // スタックポインタのオフセット量をコピー
        task_ptrs[current_task_number]->stack_pointer -= (&stack[SYSTEM_STACK_SIZE-1]-dummy_task.stack_pointer);  // sp_vals_[0]==&kernel_stack_[127]-x
    }
    else  // 全タスク初期化後
    {
        volatile std::size_t next_task_number = 0;
        // タスクが優先度昇順にソートされていることを前提とする
        for( std::size_t i = 0; i < number_of_tasks-1; i ++ )
        {
            if( task_ptrs[i+1]->wait_counter != 0 )
            {
                task_ptrs[i+1]->wait_counter --;
            }

            if(( task_ptrs[i+1]->wait_counter == 0 ) || ( task_ptrs[i+1]->callback_executing == true ))
            {
                if( next_task_number == 0 )
                {
                    next_task_number = i+1;
                }
                else
                {
                    if( task_ptrs[i+1]->get_priority() > task_ptrs[next_task_number]->get_priority() )
                    {
                        next_task_number = i+1;
                    }
                }
            }
        }

        if( task_ptrs[current_task_number]->callback_executing == false )
        {
            current_task_number = next_task_number;  // タスク切り替え
        }
    }


    disable_interrupt();
    store_stack_pointer( &(dummy_task.stack_pointer) );  // スタックポインタ保存

    restore_stack_pointer( &(task_ptrs[current_task_number]->stack_pointer) );  // スタックポインタ復帰
    enable_interrupt();
}

}