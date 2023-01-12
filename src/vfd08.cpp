#include <Arduino.h>
#include "stdint.h"

uint8_t din = 3;              // DA SDI SPI数据输入
uint8_t clk = 2;              // CK CLK SPI时钟
uint8_t cs = 7;               // CS SPI 片选
uint8_t Reset = 6;            // RS VFD屏幕的复位 低电平有效，正常使用拉高/无reset是因为模组内置RC 硬件复位电路
uint8_t en = 8;               // EN VFD模组电源部分使能，EN 高电平使能，建议置高后100ms 以上再进行 VFD初始化命令发送，避免模块电源还没稳定就发出命令，若不用此功能，直接VCC短接/无EN 是因为模组EN与VCC已经内部链接好
bool inverse = true;          // 显示倒转  真”（ true ）、“假”（ false ）
unsigned char vfd_data[8][6]; // 当前屏幕内容
unsigned char ww_data[8][7];  // 屏幕待更新内容

void spi_write_data(unsigned char w_data) // 内部使用,传输数据
{
    unsigned char i;
    for (i = 0; i < 8; i++)
    {
        digitalWrite(clk, LOW);
        // 若使用处理速度较快的 arduino设备 如ESP32 请加一些延时，VFD SPI时钟频率如手册描述 0.5MHZ
        if ((w_data & 0x01) == 0x01)
        {
            digitalWrite(din, HIGH);
        }
        else
        {
            digitalWrite(din, LOW);
        }
        w_data >>= 1;

        digitalWrite(clk, HIGH);
    }
}

void VFD_cmd(unsigned char command) // 内部使用,传输数据
{
    digitalWrite(cs, LOW);
    spi_write_data(command);
    digitalWrite(cs, HIGH);
    delayMicroseconds(5);
}

void VFD_show(void) // 内部使用,传输完数据显示内容
{
    digitalWrite(cs, LOW);  // 开始传输
    spi_write_data(0xe8);   // 开显示命令
    digitalWrite(cs, HIGH); // 停止传输
}

/******************************
  在指定位置打印一个字符(用户自定义,所有CG-ROM中的)
  x:位置;chr:要显示的字符编码
*******************************/
void VFD_WriteOneChar(unsigned char x, unsigned char chr)
{
    digitalWrite(cs, LOW);      // 开始传输
    spi_write_data(0x20 + x);   // 地址寄存器起始位置
    spi_write_data(chr + 0x30); // 显示内容数据
    digitalWrite(cs, HIGH);     // 停止传输
    VFD_show();
}
/******************************
  在指定位置打印字符串
  (仅适用于英文,标点,数字)
  x:位置;str:要显示的字符串
*******************************/
void VFD_WriteStr(unsigned char x, char *str)
{
    digitalWrite(cs, LOW);    // 开始传输
    spi_write_data(0x20 + x); // 地址寄存器起始位置
    while (*str)
    {
        spi_write_data(*str); // ascii与对应字符表转换
        str++;
    }
    digitalWrite(cs, HIGH); // 停止传输
    VFD_show();
}
void VFD_WriteStr(unsigned char x, char str)
{
    digitalWrite(cs, LOW);    // 开始传输
    spi_write_data(0x20 + x); // 地址寄存器起始位置
    while (str)
    {
        spi_write_data(str); // ascii与对应字符表转换
        str++;
    }
    digitalWrite(cs, HIGH); // 停止传输
    VFD_show();
}
/******************************
  在指定位置打印自定义字符

  x:位置，有ROOM位置;*s:要显示的字符字模
*******************************/
void VFD_WriteUserFont(size_t x, size_t y, unsigned char *s)
{
    unsigned char i = 0;
    unsigned char s_data[5];
    int x_data;
    int y_data;

    if (inverse)
    {
        int a[8]{7, 6, 5, 4, 3, 2, 1, 0};
        int b[5]{4, 3, 2, 1, 0};
        x_data = a[x];
        y_data = a[y];
        for (i = 0; i < 5; i++)
            s_data[b[i]] = (s[i] & 0x01) << 6 |
                           (s[i] & 0x02) << 4 |
                           (s[i] & 0x04) << 2 |
                           (s[i] & 0x08) |
                           (s[i] & 0x10) >> 2 |
                           (s[i] & 0x20) >> 4 |
                           (s[i] & 0x40) >> 6;
    }
    else
    {
        for (i = 0; i < 5; i++)
            s_data[i] = s[i];
        x_data = x;
        y_data = y;
    }

    digitalWrite(cs, LOW); // 开始传输
    // spi_write_data(0x40 + y); // 地址寄存器起始位置
    // for (i = 0; i < 5; i++)
    //      spi_write_data(s[i]);
    spi_write_data(0x40 + y_data); // 地址寄存器起始位置
    for (i = 0; i < 5; i++)
        spi_write_data(s_data[i]);
    digitalWrite(cs, HIGH); // 停止传输

    digitalWrite(cs, LOW);
    spi_write_data(0x20 + x_data);
    spi_write_data(0x00 + y_data);
    // spi_write_data(0x20 + x);
    // spi_write_data(0x00 + y);
    digitalWrite(cs, HIGH);

    VFD_show();
    // delay(100);
}

void VFD_SPI_init() // SPI初始化
{
    pinMode(en, OUTPUT);
    pinMode(clk, OUTPUT);
    pinMode(din, OUTPUT);
    pinMode(cs, OUTPUT);
    pinMode(Reset, OUTPUT);
}

void VFD_init(bool inv) // VFD屏幕初始化 inv:屏幕方向
{
    digitalWrite(en, HIGH); // 启用屏幕
    delay(10);
    digitalWrite(Reset, LOW); // VFD复位
    delayMicroseconds(5);
    digitalWrite(Reset, HIGH);
    // SET HOW MANY digtal numbers

    // 设置显示位数
    digitalWrite(cs, LOW);
    spi_write_data(0xe0);
    delayMicroseconds(5);
    spi_write_data(0x07); // 6 digtal 0x05 // 8 digtal 0x07//16 digtal 0x0f
    digitalWrite(cs, HIGH);
    delayMicroseconds(5);

    inverse = inv;
}
void VFD_set_bright(unsigned char brig) // 设置亮度
{
    // set bright
    digitalWrite(cs, LOW);
    spi_write_data(0xe4);
    delayMicroseconds(5);
    spi_write_data(brig); // 0xff max//0x01 min
    digitalWrite(cs, HIGH);
    delayMicroseconds(5);
}
void VFD_TEST() // 测试 屏幕
{
    VFD_cmd(0xE9); // 全亮测试 屏幕
    delay(1000);

    VFD_WriteOneChar(0, 1);
    VFD_WriteOneChar(1, 2);
    VFD_WriteOneChar(2, 3);
    VFD_WriteOneChar(3, 4);
    VFD_WriteOneChar(4, 5);
    VFD_WriteOneChar(5, 6);
    VFD_WriteOneChar(6, 7);
    VFD_WriteOneChar(7, 8);
    delay(1000);

    VFD_WriteStr(0, const_cast<char *>("ABCDEFGH"));
    delay(1000);

    VFD_set_bright(0x01);
    delay(1000);
    for (size_t i = 0; i < 254; i++)
    {
        VFD_set_bright(i);
        delay(20);
    }
    for (size_t i = 254; i > 0; i--)
    {
        VFD_set_bright(i);
        delay(20);
    }
    delay(1000);
    VFD_set_bright(150);
    delay(1000);

    // VFD_WriteUserFont(0, 0, Plop_data[0]); // 0号位字符 自定义字库中字模0
    // VFD_WriteUserFont(1, 1, Plop_data[1]);
    // VFD_WriteUserFont(2, 2, Plop_data[2]);
    // VFD_WriteUserFont(3, 3, Plop_data[3]);
    // VFD_WriteUserFont(4, 4, Plop_data[4]);
    // VFD_WriteUserFont(5, 5, Plop_data[5]);
    // VFD_WriteUserFont(6, 6, Plop_data[6]);
    // VFD_WriteUserFont(7, 7, Plop_data[7]);
    // delay(1000);
}
/******************************
  以列为单位,从待更新数据(ww_data)中取出数据 逐列 更新到显示区

  low:开始位置，high:结束位置, dire:更新方向(L: 向左进入 R: 向右进入)
*******************************/
void VFD_updata_col(int low, int high, const char *dire, int ms) // 逐列滚入更新 L: 向左进入 R: 向右进入
{
    size_t updey = (8 + low - high) * ms;
    if (dire == "L")
    {
        for (int in = low; in <= high; in++) // i: 显示位
        {
            vfd_data[in][0] = 0x00;
            for (size_t i = 0; i < 5; i++)
                vfd_data[in][i] = vfd_data[in][i + 1];

            if (in != high)
            {
                vfd_data[in][5] = vfd_data[in + 1][0];
            }
            else
            {
                vfd_data[in][5] = ww_data[low][0];
            }
        }

        for (int in = low; in <= high; in++) // i: 缓存
        {
            for (size_t i = 0; i < 5; i++)
                ww_data[in][i] = ww_data[in][i + 1];

            if (in != high)
            {
                ww_data[in][5] = ww_data[in + 1][0];
            }
            else
            {
                ww_data[in][5] = 0x00;
            }
        }
    }
    else if (dire == "R")
    {
        for (int in = high; in > low - 1; in--) // i: 显示位
        {
            for (size_t i = 5; i > 0; i--)
                vfd_data[in][i] = vfd_data[in][i - 1];

            if (in != low)
            {
                vfd_data[in][0] = vfd_data[in - 1][5];
            }
            else
            {
                vfd_data[in][0] = ww_data[high][5];
            }
        }

        for (int in = high; in > low - 1; in--) // i: 缓存
        {
            for (size_t i = 5; i > 0; i--)
                ww_data[in][i] = ww_data[in][i - 1];

            if (in != low)
            {
                ww_data[in][0] = ww_data[in - 1][5];
            }
            else
            {
                ww_data[in][0] = 0x00;
            }
        }
    }

    for (size_t in = low; in <= high; in++) // i: 显示位
    {
        VFD_WriteUserFont(in, in, vfd_data[in]);
        delay(updey);
    }
}
/******************************
  以字符为单位,从待更新数据(ww_data)中取出数据 直接 更新到显示区

  low:字符位置
*******************************/
void VFD_updata_Z(int low) // Z: 无特效直接更新指定位置
{
    if (ww_data[low][6] != 0x00)
    {
        for (size_t mi = 0; mi < 5; mi++) // mi: 列 5
        {
            vfd_data[low][mi] = ww_data[low][mi];
            ww_data[low][mi] = 0x00;
        }
        ww_data[low][6] = 0x00;
    }
    VFD_WriteUserFont(low, low, vfd_data[low]);
}
/******************************
  从待更新数据(ww_data)中取出数据更新到显示区

  dire: 更新方式 (U: 向上进入  D:向下进入 L:向左 R:向右 A:与过度 O:或过度)
  X:    只有L&R生效,百叶窗特效 真”（ true ）、“假”（ false ）
*******************************/
void VFD_updata(char *dire, bool x) // U: 向上进入  D:向下进入 L:向左 R:向右 A:与过度 O:或过度
{
    size_t low = 10;
    size_t high;

    if (dire == "L" | dire == "R")
    {
        for (size_t i = 0; i < 8; i++) // 确定待更新数据范围
        {
            // if (ww_data[i][6] != 0x00)
            if (ww_data[i][6] == 0xff)
            {
                if (low == 10)
                {
                    low = i;
                }
                high = i;
                if (x)
                    ww_data[i][6] = 0x3f;
                else
                    ww_data[i][6] = 0x00;
            }
        }
        if (low != 10)
        {
            if (x)
            {
                for (int im = 0; im < 6; im++) // i:列
                {
                    for (size_t ic = 0; ic < 8; ic++)
                    {
                        if (ww_data[ic][6] != 0x00)
                        {
                            ww_data[ic][6] >>= 1;
                            VFD_updata_col(ic, ic, dire, 0);
                        }
                    }
                    delay(40);
                }
            }
            else
            {
                for (size_t ic = low; ic <= high; ic++)
                {
                    for (int im = 0; im < 6; im++) // i:列
                    {
                        VFD_updata_col(low, high, dire, 4);
                    }
                }
            }
        }
        return;
    }

    if (dire == "U" | dire == "D")
    {
        for (size_t il = 0; il < 8; il++) // il 垂直7列
        {
            for (size_t i = 0; i < 8; i++) // i: 显示位
            {
                if (ww_data[i][6] != 0x00) // 5: 更新标识
                {
                    if (ww_data[i][6] == 0xff) // 5: 更新标识
                    {
                        ww_data[i][6] = 0x7f;
                        for (size_t mi = 0; mi < 5; mi++) // mi: 列 5
                        {
                            if (dire == "U") // 向上进入
                            {
                                vfd_data[i][mi] >>= 1;
                            }
                            else if (dire == "D") // 向下进入
                            {
                                vfd_data[i][mi] <<= 1;
                            }
                        }
                    }
                    else
                    {
                        for (size_t mi = 0; mi < 5; mi++) // mi: 列 5
                        {
                            if (dire == "U") // 5: 更新标识
                            {
                                vfd_data[i][mi] = (vfd_data[i][mi] >> 1) | ((ww_data[i][mi] & 0x01) << 6); // 更新到显示区
                                ww_data[i][mi] >>= 1;                                                      // 待更新数据 -1
                            }
                            else if (dire == "D")
                            {
                                vfd_data[i][mi] = (vfd_data[i][mi] << 1) & 0x7e | (ww_data[i][mi] >> 6) & 0x01; // 更新到显示区
                                ww_data[i][mi] <<= 1;                                                           // 待更新数据 -1
                            }
                        }
                        ww_data[i][6] >>= 1; // 更新标识-1
                    }
                    VFD_WriteUserFont(i, i, vfd_data[i]);
                }
            }

            delay(30);
        }
        return;
    }

    if (dire == "A" | dire == "O")
    {
        for (size_t i = 0; i < 8; i++) // i: 显示位
        {
            if (ww_data[i][6] == 0xff) // 5: 更新标识
            {
                for (size_t mi = 0; mi < 5; mi++) // mi: 列 5
                {
                    if (dire == "A")
                        vfd_data[i][mi] = vfd_data[i][mi] & ww_data[i][mi];
                    else
                        vfd_data[i][mi] = vfd_data[i][mi] | ww_data[i][mi];
                }
                ww_data[i][6] = 0x01;
                VFD_WriteUserFont(i, i, vfd_data[i]);
            }
        }
        delay(150);

        for (size_t i = 0; i < 8; i++) // i: 显示位
        {
            if (ww_data[i][6] == 0x01) // 5: 更新标识
            {
                for (size_t mi = 0; mi < 5; mi++) // mi: 列 5
                {
                    vfd_data[i][mi] = ww_data[i][mi];
                }
                ww_data[i][6] = 0x00;
                VFD_WriteUserFont(i, i, vfd_data[i]);
            }
        }
    }
    // dire = "Z" OR **
    for (size_t i = 0; i < 8; i++)
    {
        VFD_updata_Z(i);
    }
    return;
}

/******************************
  从待更新数据(ww_data)中取出数据更新到显示区
    指定开始位置与结束位置方式更新数据

  low:  开始位置
  high: 结束位置
  dire: 更新方式 (U: 向上进入  D:向下进入 L:向左 R:向右 A:与过度 O:或过度)
  X:    只有L&R生效,百叶窗特效 真”（ true ）、“假”（ false ）
*******************************/
void VFD_updata(int low, int high, char *dire, bool x)
{
    for (size_t i = 0; i < 8; i++) // 更新数据范围
    {
        if (i<low | i> high)
            ww_data[i][6] = 0x00;
        else
            ww_data[i][6] = 0xff;
    }

    VFD_updata(dire, x);
    return;
}
/******************************
  从待更新数据(ww_data)中取出数据更新到显示区
    指定开始位置与结束位置方式更新数据

  low:  开始位置
  high: 结束位置
  dire: 更新方式 (L:向左 R:向右)
  ii:   写入字符数量
*******************************/
void VFD_updata(int low, int high, int ii, char *dire)
{
    for (size_t i = 0; i < 8; i++) // 已传输更新字符,清除待更新标识数据
        ww_data[i][6] = 0x00;

    for (size_t ic = 0; ic < ii; ic++)
    {
        for (int im = 0; im < 6; im++) // i:列
        {
            VFD_updata_col(low, high, dire, 4);
        }
    }

    return;
}
/******************************
  将要显示的新数据写入到待更新缓存中(ww_data)
    写入过程会判断新数据与现数据是否相同,不同才会有待更新标识
  x: 字符位置 s: 内容 p: 强制更新
*******************************/

void VFD_WriteWater(unsigned char x, unsigned char *s) // 写入到缓存
{
    bool upc = false;
    // X: 字输出位置  S: 字模
    for (int i = 0; i < 5; i++)
    {
        ww_data[x][i] = s[i]; // | 0x7f;  //s[i] 字模列 0-4

        if (ww_data[x][i] != vfd_data[x][i])
        {
            upc = true;
        }
    }
    if (upc)
    {
        ww_data[x][6] = 0xff;
    }
}

void VFD_WriteWater(unsigned char x, unsigned char *s, bool p) // 写入到缓存 并全部打更新标识
{
    VFD_WriteWater(x, s);
    if (p)
        ww_data[x][6] = 0xff;
}
