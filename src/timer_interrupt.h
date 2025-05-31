#ifndef TIMER_INTERRUPT_H
#define TIMER_INTERRUPT_H

// 初始化定时器中断
void initTimerInterrupt();

// 定时器中断服务函数
void IRAM_ATTR onTimer();

// 添加新函数声明，用于接收wifi连接诶成功标识
void recordwifiSyncSuccess(bool success);

 // 添加新函数声明，用于接收AP同步成功标识
void recordAPSyncSuccess(bool success);

// 停止定时器中断
void stopTimerInterrupt();

#endif