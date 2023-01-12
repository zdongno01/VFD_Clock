//  初始化
void spi_write_data(unsigned char w_data);
void VFD_show(void);
void VFD_SPI_init();
void VFD_init(bool inv);

//  基础函数
void VFD_set_bright(unsigned char brig);                   //亮度设置
void VFD_WriteOneChar(unsigned char x, unsigned char chr); // 在指定位置打印一个字符
void VFD_WriteStr(unsigned char x, char *str);             // 在指定位置打印字符串
void VFD_WriteStr(unsigned char x, char str);              // 在指定位置打印字符串
void VFD_WriteUserFont(size_t x, size_t y,unsigned char *s);  // 在指定位置打印自定义字符

//  应用函数
void VFD_cmd(unsigned char command);                    // 直接发送指令 - 0xE9(全亮测试 屏幕)
void VFD_TEST() ;                                       // 测试

void VFD_updata_Z(int low); // Z: 直接更新指定位置
void VFD_updata_col(int low, int high, const char *dire, int ms);   // 单列压入压出 L: 向左进入 r: 向右进入
void VFD_updata(char *dire, bool X);					            // U: 向上进入  D:向下进入 L:向左 R:向右 X:在L和R时百叶窗特效
void VFD_updata(int low, int high, char *dire, bool x);  	        // 指定范围, U: 向上进入  D:向下进入 L:向左 R:向右 X:在L和R时百叶窗特效
void VFD_updata(int low, int high, int ii, char *dire);             // 指定范围,指定字符数滚动(L&R)进入字符
void VFD_WriteWater(unsigned char x, unsigned char *s, bool p);     // 写入到缓存(强制更新)
void VFD_WriteWater(unsigned char x, unsigned char *s);             // 写入到缓存(自动判断是否需要更新)





