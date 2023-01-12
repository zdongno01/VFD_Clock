// // NTP网络授时
#include <NTPClient.h>
#include <DS3231M.h>
#include "vfd08.h"

static uint8_t secs;
static uint8_t minute;
static uint8_t hour;

unsigned char TTF_data[12][5];  // 时钟字体缓存

DS3231M_Class DS3231M;
void set_vfmtm(int tm, unsigned char n)     //时间个十位拆分
{
    int m, t, n1 = n + 1;
    if (tm >= 50)
    {
        m = 50;
        t = 5;
    }
    else if (tm >= 40)
    {
        m = 40;
        t = 4;
    }
    else if (tm >= 30)
    {
        m = 30;
        t = 3;
    }
    else if (tm >= 20)
    {
        m = 20;
        t = 2;
    }
    else if (tm >= 10)
    {
        m = 10;
        t = 1;
    }
    else
    {
        m = 0;
        t = 0;
    }
    int tmw = tm - m;

    VFD_WriteWater(n, TTF_data[t]);
    VFD_WriteWater(n1, TTF_data[tmw]);
}
void tm_separate(unsigned char x, int y) // 显示时间间隔符
{
    VFD_WriteWater(x, TTF_data[y]);
    VFD_updata_Z(x);
}
void DS3231M_init() // DS3231模块初始化
{
    DS3231M.begin();         // Initialize RTC communications
    DS3231M.pinSquareWave(); // Make INT/SQW pin toggle at 1Hz

    // DS3231M.adjust(
    //       DateTime(now.year(), now.month(), now.day(), hour, now.minute(), now.second()));  //调整RTC日期/时间
    // DateTime(timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));  //调整RTC日期/时间
}
void DS3231M_load(char *no_Toggle) // DS3231时间加载动画
{
    DateTime now = DS3231M.now();
    secs = now.second() + 1;
    if (secs == 60)
    {
        secs = 00;
        minute = now.minute() + 1;
    }
    else
        minute = now.minute();

    if (minute == 60)
    {
        minute = 00;
        hour = now.hour() + 1;
        if (hour = 24)
            hour = 00;
    }
    else
        hour = now.hour();

    set_vfmtm(hour, 0);
    set_vfmtm(minute, 3);
    set_vfmtm(secs, 6);

    // set_vfmtm(now.hour(), 0);
    // set_vfmtm(now.minute(), 3);
    // set_vfmtm(now.second(), 6);
    VFD_updata(no_Toggle, false); //  true  false
    delay(100);
}
void print_DS3231_Time(char *Toggle) // 显示DS3231时间
{
    const uint8_t SPRINTF_BUFFER_SIZE{32};   ///< Buffer size for sprintf()
    char output_buffer[SPRINTF_BUFFER_SIZE]; ///< Temporary buffer for sprintf()
    DateTime now = DS3231M.now();
    // sprintf(output_buffer, "%02d %02d %02d", now.hour(), now.minute(), now.second());
    //  VFD_WriteStr(0, output_buffer);
    if (secs != now.second()) // 每秒钟整执行
    {
        if (Toggle != "ASCII")
        {
            if (minute != now.minute()) // 每分钟整执行
            {
                if (hour != now.hour()) // 每小时整执行
                {
                    hour = now.hour();
                    set_vfmtm(now.hour(), 0);
                }
                minute = now.minute();
                tm_separate(2, 11);
                set_vfmtm(now.minute(), 3);
            }
            set_vfmtm(now.second(), 6);
            VFD_updata(Toggle, true);
        }
        else
        {
            sprintf(output_buffer, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
            VFD_WriteStr(0, output_buffer);
        }

        secs = now.second(); // Set the counter variable
        tm_separate(5, 11);  //:
        delay(300);

        tm_separate(2, 10); //:
        tm_separate(5, 10); //:
    }
}
void Reset_time(char *Toggle)
{
    DateTime now = DS3231M.now();
    set_vfmtm(now.hour(), 0);
    set_vfmtm(now.minute(), 3);
    set_vfmtm(now.second(), 6);
    tm_separate(2, 10);
    tm_separate(5, 10);
    VFD_updata(Toggle, true); // false  true
}

void adjust_DS3231M()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        return;
    }
    else
    {
        DS3231M.adjust(
            DateTime(timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec)); // 调整RTC日期/时间
        return;
    }
}
void load_fonts(unsigned char x, unsigned char *s) // 加载字体
{
    for (int i = 0; i < 5; i++)
    {
        TTF_data[x][i] = s[i]; // | 0x7f;  //s[i] 字模列 0-4
    }
}