#include "em_timer.h"
#include "timers.h"

bool read_state_timeout = false;    // 间隔10s上报设备状态到手机
bool printer_timeout = false;       // 打印超过20s触发警报

/* 静态定时器内存控制块 */
StaticTimer_t myStateTimerBuffer;   // 状态定时器内存
StaticTimer_t myPrintTimerBuffer;   // 打印定时器内存

// 全局定时器句柄（声明为全局变量，方便后续操作）
TimerHandle_t myStateTimer = NULL;
TimerHandle_t myPrintTimer = NULL;

// 读取状态定时器的回调函数
void read_state_timer_callbackfun(void *param)
{
    printf("read_state now...\r\n");
    read_state_timeout = true;
}

/**
  * @brief 定时器初始化
  * @param  
  * @retval 
  */
void Timer_Init(void)
{
    myStateTimer = xTimerCreateStatic(
        "StateTimer",                   // 定时器名称（调试用）
        10000 / portTICK_PERIOD_MS,     // 周期10秒（转换为Tick数）
        pdTRUE,                         // 自动重载
        NULL,                           // 无参数传递给回调
        read_state_timer_callbackfun,   // 回调函数
        &myStateTimerBuffer             // 静态内存块
    );

    /* 检查定时器是否创建成功 */
    if (myStateTimer == NULL) {
        printf("Error: 状态定时器创建失败！\n");
        return;
    }

    /* 启动定时器（立即启动，不阻塞） */
    if (xTimerStart(myStateTimer, 0) != pdPASS) {
        printf("Error: 状态定时器启动失败！\n");
    }
}

// 读取state定时器是否超时
bool get_state_timeout(void)
{
    return read_state_timeout;
}

void clean_state_timeout(void)
{
    read_state_timeout = false;
}

void read_Printer_timer_callbackfun(void *param)
{
    printf("触发打印超时错误...\r\n");
    printer_timeout = true;
}

// 创建打印20s超时定时器
void open_printer_timeout_timer(void)
{
    // 检查是否已存在，若存在则先删除
    if (myPrintTimer != NULL) {
        xTimerDelete(myPrintTimer, 0);
    }
    
    myPrintTimer = xTimerCreateStatic(
        "PrintTimer",
        20000 / portTICK_PERIOD_MS,
        pdFALSE,        // 非自动重载（单次触发）
        NULL,
        read_Printer_timer_callbackfun,
        &myPrintTimerBuffer
    );
    
    if (myPrintTimer == NULL) {
        printf("打印机定时器创建失败！\n");
        return;
    }
    
    if (xTimerStart(myPrintTimer, 0) != pdPASS) {
        printf("Error: 打印机定时器启动失败！\n");
    }
}

void close_printer_timeout_timer(void)
{
    if (myPrintTimer != NULL) {
        xTimerStop(myPrintTimer, 0);
        // 注意：这里不设置myPrintTimer=NULL，因为静态定时器内存不能释放
        // 仅停止定时器，下次使用时可直接启动
    }
}

bool get_printer_timeout_status(void)
{
    return printer_timeout;
}
