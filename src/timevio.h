
void DS3231M_init();
void DS3231M_load(char *no_Toggle);
void catitm(char *retchar, int tim_ti);
void set_vfmtm(int tm, unsigned char n);
void tm_separate(unsigned char x, int y);
void print_DS3231_Time(char *dire);
void Reset_time(char *Toggle);
void adjust_DS3231M();
void load_fonts(unsigned char x, unsigned char *s);
