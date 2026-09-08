/*!
    \file      FreeRTOSConfig.h
    \brief     FreeRTOS 配置文件（GD32F407VGT6 @ 240MHz，Keil AC5 + RVDS/ARM_CM4F 移植）
    \version   2024-01-01, V1.0.0

    \note  中断优先级说明（GD32F407 为 Cortex-M4，4 位优先级，16 级）：
           - 数值越小优先级越高
           - 优先级 0~4   ：高于 configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY，
                             中断服务程序中【禁止】调用任何 FreeRTOS API
           - 优先级 5~14  ：中断服务程序中可以调用以 "FromISR" 结尾的 FreeRTOS API
           - 优先级 15    ：最低，被 FreeRTOS 内核占用（PendSV/SysTick）
*/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "gd32f4xx.h"

/* ------------------------------------------------------------------ */
/* 基本配置                                                           */
/* ------------------------------------------------------------------ */

/* CPU 主频（system_gd32f4xx.c 中配置为 240MHz PLL @ 25MHz HXTAL） */
#define configCPU_CLOCK_HZ                      ( SystemCoreClock )

/* 系统节拍频率 */
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 )

/* Cortex-M4，4 位中断优先级 */
#define configPRIO_BITS                         4

/* 最低中断优先级（15，移位后 0xF0）*/
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15
/* 可屏蔽中断的最高优先级（5，移位后 0x50）：优先级数值 >= 5 的中断
   才允许调用 FromISR API */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

#define configKERNEL_INTERRUPT_PRIORITY \
        ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
        ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )

/* 抢占式调度 + 时间片轮转 */
#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                  1
#define configIDLE_SHOULD_YIELD                 1

/* 使用硬件 CLZ 指令选择最高优先级任务（Cortex-M4 支持，优先级数须 <= 32） */
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1

/* 32 位节拍 */
#define configUSE_16_BIT_TICKS                  0

/* 任务优先级数与栈参数 */
#define configMAX_PRIORITIES                    7
#define configMINIMAL_STACK_SIZE                ( ( uint16_t ) 128 )
#define configMAX_TASK_NAME_LEN                 16
#define configSTACK_DEPTH_TYPE                  uint16_t

/* FreeRTOS 堆大小：heap_4.c 使用（GD32F407VGT6 主 SRAM 为 128KB） */
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 64 * 1024 ) )

/* ------------------------------------------------------------------ */
/* 内核对象                                                            */
/* ------------------------------------------------------------------ */

#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_TASK_NOTIFICATIONS            1
#define configQUEUE_REGISTRY_SIZE               8

/* 软件定时器 */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            ( configMINIMAL_STACK_SIZE * 2 )

/* 协程（不使用） */
#define configUSE_CO_ROUTINES                   0

/* ------------------------------------------------------------------ */
/* 钩子与调试                                                          */
/* ------------------------------------------------------------------ */

#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_MALLOC_FAILED_HOOK            1
#define configCHECK_FOR_STACK_OVERFLOW          2

/* 运行时统计（如需 CPU 使用率统计置 1，并实现相应时钟源） */
#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_TRACE_FACILITY                0
#define configUSE_STATS_FORMATTING_FUNCTIONS    0

#define configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS 0

/* 断言：失败时进入 vAssertCalled */
extern void vAssertCalled( const char *pcFile, int ulLine );
#define configASSERT( x )  if( ( x ) == 0 ) vAssertCalled( __FILE__, __LINE__ )

/* ------------------------------------------------------------------ */
/* 可选 API 包含控制                                                    */
/* ------------------------------------------------------------------ */

#define INCLUDE_vTaskPrioritySet            1
#define INCLUDE_uxTaskPriorityGet           1
#define INCLUDE_vTaskDelete                 1
#define INCLUDE_vTaskSuspend                1
#define INCLUDE_vTaskDelayUntil             1
#define INCLUDE_vTaskDelay                  1
#define INCLUDE_xTaskGetSchedulerState      1
#define INCLUDE_xTaskGetCurrentTaskHandle   1
#define INCLUDE_xTaskGetIdleTaskHandle      1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_eTaskGetState               1
#define INCLUDE_xTaskAbortDelay             0
#define INCLUDE_xTaskGetHandle              0
#define INCLUDE_xTaskResumeFromISR          1
#define INCLUDE_xTimerPendFunctionCall      0

/* ------------------------------------------------------------------ */
/* 中断服务函数映射                                                    */
/* port.c 中定义的 vPortSVCHandler / xPortPendSVHandler /               */
/* xPortSysTickHandler 直接映射到启动文件向量表中的标准名称。            */
/* 注意：gd32f4xx_it.c 中不能再定义这三个同名中断服务函数！              */
/* ------------------------------------------------------------------ */

#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
