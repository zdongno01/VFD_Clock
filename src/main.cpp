#include <Arduino.h>
#include <wifi.h>
#include <TTF.h>
#include <Wire.h>

// 存取配置数据
#include <esp_partition.h>
#include "FFat.h"
#include <Preferences.h>

// WIFI_AP
IPAddress apIP(192, 168, 4, 1); // 设置AP的IP地址

// #define ASCII_VDF //系统字体
#define TTF_VDF   // 自定义字体+特效
#define DS3231    // 启用DS3231时钟
#define SRAND     // 随机字体
#define WEBServer // WEB控制

#ifdef WEBServer
#include "web_server.h"
#endif

// VFD显示屏
#include "vfd08.h"
#include "timevio.h"

// 随机字体
#ifdef SRAND
#include <iostream>
#include <cstdlib>
#include <chrono>
using namespace std;
using namespace chrono;
#endif

const uint8_t SPRINTF_BUFFER_SIZE{32};   ///< Buffer size for sprintf()
char output_buffer[SPRINTF_BUFFER_SIZE]; ///< Temporary buffer for sprintf()
int refresh = 100;                       // 刷新间隔  X ms
// int separate;
bool wifi_ap = false; // wifi_ap 模式: 真”（TRUE）、“假”（FALSE）
char *toogle[] =
    {const_cast<char *>("Z"),
     const_cast<char *>("R"),
     const_cast<char *>("L"),
     const_cast<char *>("D"),
     const_cast<char *>("U"),
     const_cast<char *>("A"),
     const_cast<char *>("O")};

//  设 置
unsigned char brig = 0x55; // 屏幕亮度
bool VFD_inverse = false;  // 显示倒转  真”（ true ）、“假”（ false ）
bool SRAND_onff = true;    // 随机字体开关
bool ipadd_onff = true;    // IP地址显示开关
int Tick = 4;              // 时间间隔
int Tock = 5;              // 时间间隔
char *no_Toggle;           // 开机特效
char *tm_Toggle;           // 时间切换特效
String s_hostname = "ZD_VFD";
String s_wifi_ssid1 = "";           // WIFI_SSID1
String s_wifi_pass1 = "";           // WIFI_PASS1
String s_wifi_ssid2;                // WIFI_SSID2
String s_wifi_pass2;                // WIFI_PASS2
String s_wifi_ssid3;                // WIFI_SSID3
String s_wifi_pass3;                // WIFI_PASS3
const char *ssid;                   // WiFi名
const char *password;               // WiFi密码
const char *hostname = s_hostname.c_str();

unsigned seed; // Random generator seed

#define NTP_SERVER1 "cn.ntp.org.cn"    // 中国授时
#define NTP_SERVER2 "ntp.ntsc.ac.cn"   // 中科院
#define NTP_SERVER3 "time6.aliyun.com" // 阿里
#define NTP_SERVER4 "cn.pool.ntp.org"  // 国际通用

WiFiUDP ntpUDP;

void asc_to_VFDww(unsigned char x, const char *s)
{
    char cs[1];
    strncpy(cs, s, 1);
    int iss = cs[0] - 55; 
    VFD_WriteWater(x, FF0000[iss]);
}
void while_Wifi(const char *ssid, const char *pass, int wi)
{
    if (ssid != "")
        WiFi.begin(ssid, pass);

#ifdef DS3231
    // print_DS3231_Time(const_cast<char *>("Z"));
    for (int i = 0; i < 50; i++)
    {
#ifdef ASCII_VDF
        print_DS3231_Time("ASCII");
#endif
#ifdef TTF_VDF
        // print_DS3231_Time("M");
        print_DS3231_Time(tm_Toggle);

#endif
        delay(refresh);
    }
#else

    VFD_WriteWater(0, icon_data[0], false);
    VFD_WriteWater(7, Plop_data[wi], false);
    // VFD_updata(tm_Toggle, true);
    VFD_updata(const_cast<char *>("Z"), true);

    for (size_t i = 0; i < 10; i++)
    {
        VFD_WriteWater(5, icon_data[4], true);
        VFD_WriteWater(6, icon_data[2], true);
        VFD_updata(1, 6, 2, const_cast<char *>("R"));
    }
#endif

    return;
}
void while_NTP()
{

    // ESP时钟未准备好, 使用DS3231时钟
    struct tm timeinfo; // ESP时钟
    bool clvf = true;

    // while (!getLocalTime(&timeinfo))
    // {
#ifdef DS3231
        for (int i = 0; i < 50; i++)
        {

#ifdef ASCII_VDF
            print_DS3231_Time("ASCII");
#endif
#ifdef TTF_VDF
            print_DS3231_Time(tm_Toggle);
#endif
            delay(refresh);
        }
#else
        if (clvf)
        {
        for (size_t i = 1; i < 8; i++)
            VFD_WriteWater(i, icon_data[4], true);
        VFD_WriteWater(0, icon_data[1]);
        VFD_updata(0, 7, const_cast<char *>("L"), true);
        }
        
        // VFD_WriteWater(7, icon_data[4]);
        // VFD_updata(tm_Toggle, true);

        for (size_t i = 0; i < 10; i++)
        {
            
        VFD_WriteWater(1, icon_data[3], true);
        VFD_WriteWater(2, icon_data[4], true);
        VFD_updata(1, 7, 2, const_cast<char *>("L"));
        }

#endif
    // };

    pinMode(13, OUTPUT);   // 开启引脚输出模式      LED D5
    digitalWrite(13, LOW); // HIGH输出高电平，LOW就是低电平
#ifdef DS3231
    adjust_DS3231M(); // 同步系统时间到M3231M
#endif
    return;
}
void WiFi_softAP()
{
    WiFi.mode(WIFI_AP);                                         // 配置为AP模式
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)); // 设置AP热点IP和子网掩码

    if (WiFi.softAP(s_hostname.c_str())) // 开启AP热点,如需要密码则添加第二个参数
    {
        wifi_ap = true;

        // 打印相关信息
        Serial.println("ESP-32S SoftAP is right.");
        Serial.print("Soft-AP IP address = ");
        Serial.println(WiFi.softAPIP());                                            // 接入点ip
        Serial.println(String("MAC address = ") + WiFi.softAPmacAddress().c_str()); // 接入点mac

        // AP:SETUP
        asc_to_VFDww(0, "A");
        asc_to_VFDww(1, "P");
        VFD_WriteWater(2, icon_data[5]);
        asc_to_VFDww(3, "S");
        asc_to_VFDww(4, "E");
        asc_to_VFDww(5, "T");
        asc_to_VFDww(6, "U");
        asc_to_VFDww(7, "P");

        VFD_updata(const_cast<char *>("U"), false);

        delay(2000);
        VFD_WriteWater(2, icon_data[4]);
        VFD_WriteWater(5, icon_data[4]);
    }
    else // 开启AP热点失败
    {
        Serial.println("WiFiAP Failed");
        delay(1000);
        Serial.println("restart now...");
        ESP.restart(); // 重启复位esp32
    }
}

void test()
{
    VFD_TEST();     //屏幕功能测试
    
    //切换加载自定义字符
    VFD_set_bright(0x01); 
    delay(1000);
    VFD_set_bright(brig); 
    for (size_t i = 0; i < 8; i++)
    {
        size_t is = i + 10;
        VFD_WriteWater(i, FF0000[is]);
    }
    VFD_updata(const_cast<char *>("Z"), false);
    delay(2000);

    //向右推入两个字符
    VFD_WriteWater(6, FF0000[8]);
    VFD_WriteWater(7, FF0000[9]);
    VFD_updata(0, 7, 2, const_cast<char *>("R"));
    delay(2000);

    //向左推入两个字符
    // VFD_WriteWater(0, FF0000[16]);
    // VFD_WriteWater(1, FF0000[17]);
    VFD_updata(0, 7, 2, const_cast<char *>("L"));
    delay(2000);

    //向上推入字符
    for (size_t i = 0; i < 8; i++)
        VFD_WriteWater(i, FF0000[i]);
    VFD_updata(const_cast<char *>("U"), false);
    delay(2000);

    //向下推入字符
    for (size_t i = 0; i < 8; i++)
    {
        size_t is = i + 10;
        VFD_WriteWater(i, FF0000[is]);
    }
    VFD_updata(const_cast<char *>("D"), false);
    delay(2000);

    //向右百叶窗
    for (size_t i = 0; i < 8; i++)
        VFD_WriteWater(i, FF0000[i]);
    VFD_updata(const_cast<char *>("R"), true);
    delay(2000);

    //向左百叶窗
    VFD_WriteWater(0, icon_data[0]);
    VFD_WriteWater(7, Plop_data[0]);
    for (size_t i = 1; i < 7; i++)
        VFD_WriteWater(i, icon_data[4]);
    VFD_updata(const_cast<char *>("L"), true);
    delay(2000);
    
    //部分字符推入/推出
    for (size_t i = 1; i < 7; i++)
        VFD_WriteWater(i, icon_data[2]);
    VFD_updata(1, 6, 6, const_cast<char *>("R"));
    for (size_t i = 1; i < 7; i++)
        VFD_WriteWater(i, icon_data[4]);
    VFD_updata(1, 6, 6, const_cast<char *>("R"));
    delay(2000);
    
    //全部推出
    VFD_WriteWater(7, icon_data[4]);
    VFD_updata(const_cast<char *>("L"), true);
    for (size_t i = 0; i < 8; i++)
        VFD_WriteWater(i, icon_data[4]);
    VFD_updata(const_cast<char *>("L"), false);
    delay(2000);
}

void setup()
{
#ifdef SRAND
    typedef std::chrono::high_resolution_clock Clock;
    auto t1 = Clock::now(); // 计时开始
#endif

    String tmps;
    Serial.begin(9600);
    Preferences prefs;

    Serial.println("get_Config");

    prefs.begin("VFD_Config"); // 打开命名空间

    brig = prefs.getUChar("brig", 0x55);                  // 屏幕亮度
    ipadd_onff = prefs.getBool("ipadd_onff", true);       // IP地址显示开关
    VFD_inverse = prefs.getBool("vfd_inv", VFD_inverse);  // 显示倒转
    int i_no_Toggle = prefs.getInt("no_Toggle", 0);       // 开机特效
    int i_tm_Toggle = prefs.getInt("tm_Toggle", 0);       // 时间切换特效
    int i_ttfa = prefs.getInt("ttfa", 0);                 // 时间字体1
    int i_ttfb = prefs.getInt("ttfb", 0);                 // 时间字体2
    Tick = prefs.getInt("Tick", Tick);                    // 时间间隔 Tick
    Tock = prefs.getInt("Tock", Tock);                    // 时间间隔 Tock
    s_hostname = prefs.getString("hostname", s_hostname); // 主机名

    s_wifi_ssid1 = prefs.getString("wifi_ssid1", ssid);         // WIFI_SSID1
    s_wifi_pass1 = prefs.getString("wifi_pass1", password);     // WIFI_PASS1
    s_wifi_ssid2 = prefs.getString("wifi_ssid2", s_wifi_ssid2); // WIFI_SSID2
    s_wifi_pass2 = prefs.getString("wifi_pass2", s_wifi_pass2); // WIFI_PASS2
    s_wifi_ssid3 = prefs.getString("wifi_ssid3", s_wifi_ssid3); // WIFI_SSID3
    s_wifi_pass3 = prefs.getString("wifi_pass3", s_wifi_pass3); // WIFI_PASS3

    prefs.end();

    no_Toggle = toogle[i_no_Toggle];
    tm_Toggle = toogle[i_tm_Toggle];

    Serial.println("VFD_init");
    VFD_SPI_init(); // 初始化vfd08屏幕

    ssid = s_wifi_ssid1.c_str();
    password = s_wifi_pass1.c_str();
    WiFi.setHostname(hostname); // 设置主机名
    WiFi.begin(ssid, password); // 启动网络连接1

    // Serial.println("VFD_init");
    VFD_init(VFD_inverse); // 启用vfd08屏幕
    VFD_set_bright(brig);  // 设置屏幕亮度

    // test();

#ifdef DS3231
    DS3231M_init(); // 初始化 DS3231时钟
#endif
#ifdef TTF_VDF
#ifdef SRAND
    auto t2 = Clock::now(); // 计时结束
    unsigned seed;          // 随机数种子
    seed = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
    srand(seed);
#endif
    for (int i = 0; i < 10; i++) // 加载字体
    {
#ifdef SRAND
        if (!(rand() % 3) && SRAND_onff) // 获取随机数(限定 0 or 1)
        {
            if (i_ttfb == 1)
                load_fonts(i, Plop_data[i]);
            else if (i_ttfb == 2)
                load_fonts(i, Plop_f_data[i]);
            else if (i_ttfb == 3)
                load_fonts(i, BMBIA_data[i]);
            else if (i_ttfb == 4)
                load_fonts(i, BMBIA_f_data[i]);
            else if (i_ttfb == 5)
                load_fonts(i, Pixel_data[i]);
            else if (i_ttfb == 6)
                load_fonts(i, bold_data[i]);
            else
                load_fonts(i, FF0000[i]);
        }
        else
#endif
        {
            if (i_ttfa == 1)
                load_fonts(i, Plop_data[i]);
            else if (i_ttfa == 2)
                load_fonts(i, Plop_f_data[i]);
            else if (i_ttfa == 3)
                load_fonts(i, BMBIA_data[i]);
            else if (i_ttfa == 4)
                load_fonts(i, BMBIA_f_data[i]);
            else if (i_ttfa == 5)
                load_fonts(i, Pixel_data[i]);
            else if (i_ttfb == 6)
                load_fonts(i, bold_data[i]);
            else
                load_fonts(i, FF0000[i]);
        }
    };
    load_fonts(10, icon_data[Tick]);
    load_fonts(11, icon_data[Tock]);

#endif
#ifdef DS3231
    DS3231M_load(no_Toggle); // 载入时钟
#endif

    Serial.println("while_Wifi_1");
    while_Wifi("", "", 1);
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("while_Wifi_2");
        while_Wifi(s_wifi_ssid2.c_str(), s_wifi_pass2.c_str(), 2);
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("while_Wifi_3");
            while_Wifi(s_wifi_ssid3.c_str(), s_wifi_pass3.c_str(), 3);
            if (WiFi.status() != WL_CONNECTED)
            {
                WiFi_softAP();
            }
        }
    }

    pinMode(12, OUTPUT);   // 开启引脚输出模式  LED D4
    digitalWrite(12, LOW); // HIGH输出高电平，LOW就是低电平

    if (!wifi_ap)
    {
        configTime(8 * 3600, 0, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3); //, NTP_SERVER4);

        Serial.println("while_NTP");
        while_NTP();

        //  显示WIFI IP地址
        if (ipadd_onff)
        {
            String ipadd = WiFi.localIP().toString().c_str();
            char ipadd_c[14];
            int in;

            strncpy(ipadd_c, ipadd.c_str(), ipadd.length());

            VFD_WriteWater(0, icon_data[4]);   //
            asc_to_VFDww(1, "I");
            asc_to_VFDww(2, "P");
            // VFD_WriteWater(1, FF0000[18]);     //  I
            // VFD_WriteWater(2, FF0000[25]);     //  P
            VFD_WriteWater(3, icon_data[5]);   //  :
            for (size_t i = 0; i < 4 * 6; i++) // 推入字符"IP:"
                VFD_updata_col(0, 7, "L", 4);

            for (size_t i = 0; i < ipadd.length(); i++) // 推入IP地址
            {
                in = ipadd_c[i];
                if (in == 46)
                {
                    VFD_WriteWater(0, icon_data[10]);
                }
                else
                {
                    in = in - 48;
                    VFD_WriteWater(0, FF0000[in]);
                }
                for (size_t i = 0; i < 6; i++)
                    VFD_updata_col(0, 7, "L", 4);
            }
            delay(3000);
        }
    }

#ifdef WEBServer
    WEB_server_begin();
#endif

    // VFD_cmd(0xE9); // 全亮测试 屏幕
    // delay(1000);

    Reset_time(tm_Toggle); // 返回时间显示
    VFD_WriteWater(2, icon_data[4]);
    VFD_WriteWater(5, icon_data[4]);

    // ESP.restart(); // 重启系统
    Serial.println("setup_OK");
}

void loop()
{
    static uint8_t tm_secs;
    static uint8_t tm_minute;
    static uint8_t tm_hour;

    struct tm timeinfo; // ESP时钟

    if (wifi_ap || !getLocalTime(&timeinfo))
    {

#ifdef DS3231
        // ESP时钟失效, 使用DS3231时钟
        for (int i = 0; i < 100; i++)
        {

#ifdef ASCII_VDF
            print_DS3231_Time("ASCII");
#endif
#ifdef TTF_VDF
            print_DS3231_Time(tm_Toggle);
#endif

            digitalWrite(12, HIGH);
            delay(refresh);
            digitalWrite(12, LOW);
#ifdef WEBServer
            WEB_handle();
#endif
        }
#else
        while_NTP();
#endif
    }
    else
    {
        // ESP时钟
        if (tm_secs != timeinfo.tm_sec) // 每秒钟整执行
        {
            if (tm_minute != timeinfo.tm_min) // 每分钟整执行
            {
                if (tm_hour != timeinfo.tm_min) // 每小时整执行
                {
#ifdef DS3231
                    adjust_DS3231M(); // 同步系统时间到M3231M
#endif
                    tm_hour = timeinfo.tm_min;
#ifdef TTF_VDF
                    set_vfmtm(timeinfo.tm_hour, 0);
#endif
                }
                tm_minute = timeinfo.tm_min;
                tm_separate(2, 11);
#ifdef TTF_VDF
                set_vfmtm(timeinfo.tm_min, 3);
#endif
            }
#ifdef TTF_VDF
            set_vfmtm(timeinfo.tm_sec, 6);
            tm_separate(5, 11);
            VFD_updata(tm_Toggle, true);
#endif

#ifdef ASCII_VDF
            sprintf(output_buffer, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            VFD_WriteStr(0, output_buffer);
#endif

            tm_secs = timeinfo.tm_sec; // Set the counter variable
            delay(300);
            tm_separate(2, 10);
            tm_separate(5, 10);

            // VFD_WriteUserFont(2, 2, icon[0]);
            // VFD_WriteUserFont(5, 5, icon[0]);
        }
    }

#ifdef WEBServer
    WEB_handle();
#endif
    delay(refresh);
}