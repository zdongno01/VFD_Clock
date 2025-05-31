#include "Arduino.h"
#include "timer_interrupt.h"

// 定时器句柄
hw_timer_t *timer = NULL;

bool ledSuccess = false;

bool apSyncSuccess = false;  // AP同步状态
bool wifiSyncSuccess = false; // NTP同步状态
// 新增变量用于呼吸灯控制
static int brightness = 0; // 亮度值
static int fadeAmount = 5; // 每次亮度变

// 初始化定时器中断
void initTimerInterrupt()
{
    // 选择定时器 0，分频系数为 80，上升沿触发
    timer = timerBegin(0, 80, true);
    // 绑定中断服务函数
    timerAttachInterrupt(timer, &onTimer, true);
    // 设置定时器周期为 1 秒（1000000 微秒）
    timerAlarmWrite(timer, 1000000, true);
    // 启动定时器
    timerAlarmEnable(timer);
}

// 定时器中断服务函数
void IRAM_ATTR onTimer()
{
    // 打开 LED 灯 D12 和 D13
    if (apSyncSuccess)
    {
        if (ledSuccess)
        {
            analogWrite(12, 60);
            analogWrite(13, 0);
            ledSuccess = false;
        }
        else
        {
            analogWrite(12, 0);
            analogWrite(13, 60);
            ledSuccess = true;
        }
    }
    else
    {
        if (ledSuccess)
        {
            if (!wifiSyncSuccess)
            {
                analogWrite(12, 60);
                analogWrite(13, 60);
            }
            else
            {
                analogWrite(13, 60);
            }
            ledSuccess = false;
        }
        else
        {

            analogWrite(12, 0);
            analogWrite(13, 0);
            // digitalWrite(12, LOW); // HIGH输出高电平，LOW就是低电平
            // digitalWrite(13, LOW); // HIGH输出高电平，LOW就是低电平
            ledSuccess = true;
        }
    }
}

// 实现新函数，记录wifi连接成功标识
void recordwifiSyncSuccess(bool success)
{
    wifiSyncSuccess = success;
}

void recordAPSyncSuccess(bool success)
{
    apSyncSuccess = success;
}
//停止定时器中断
void stopTimerInterrupt() {
    if (timer != NULL) {
        analogWrite(12, 0);
        analogWrite(13, 0);
        timerEnd(timer); // 停止定时器
        timer = NULL;    // 重置定时器句柄
    }
}