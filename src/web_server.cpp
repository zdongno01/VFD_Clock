#include <Webserver.h>
#include <vfd08.h>

#include <iostream>
#include <sstream>

#include <esp_partition.h>
#include "FFat.h"
#include <Preferences.h>

#include <iostream>
#include <sstream>

WebServer server(80);
Preferences prefs;

unsigned char ws_brig; //  屏幕亮度
bool ws_vfd_inverse;   //  显示倒转  真”（ true ）、“假”（ false ）
bool ws_ipadd_onff;    //  IP地址显示开关
int ws_Tick;           //  时间间隔1
int ws_Tock;           //  时间间隔2
int ws_no_Toggle;      //  开机特效
int ws_tm_Toggle;      //  时间切换特效
int ws_ttfa;           //  时间字体1
int ws_ttfb;           //  时间字体2
String ws_hostname;    //  主机名
String ws_wifi_ssid1;  // WIFI_SSID1
String ws_wifi_pass1;  // WIFI_PASS1
String ws_wifi_ssid2;  // WIFI_SSID2
String ws_wifi_pass2;  // WIFI_PASS2
String ws_wifi_ssid3;  // WIFI_SSID3
String ws_wifi_pass3;  // WIFI_PASS3

// void set_brig();

String function(String ht, String id, String onc)
{
    return "function " + onc + "() {\
         var value = document.getElementById('" +
           id + "').value;\
         xhttp.open('GET', '/" +
           ht + "?" + id + "=' + value, true);\
         xhttp.send() }";
}
String input_text(String tx, String id, String onc, String vi)
{
    return "<body> " + tx + "<input type='text' id='" + id + "' value='" + vi + "' onchange='" + onc + "()'></body>";
}
String option_Toggle(String tx, String id, String onc, int vi) // String vi)
{
    String HTML, Z, R, L, D, U, A, O;
    if (vi == 1)
        R = "selected";
    else if (vi == 2)
        L = "selected";
    else if (vi == 3)
        D = "selected";
    else if (vi == 4)
        U = "selected";
    else if (vi == 5)
        A = "selected";
    else if (vi == 6)
        O = "selected";
    else
        vi = 0;
    Z = "selected";

    return "<P><body> " + tx + " <select id='" + id + "' value='" + vi + "' onchange='" + onc + "()'>\
        <option value='0'" +
           Z + ">无</option>\
        <option value='1'" +
           R + ">从左滑入</option>\
        <option value='2'" +
           L + ">从右滑入</option>\
        <option value='3'" +
           D + ">从上滑入</option>\
        <option value='4'" +
           U + ">从下滑入</option>\
        <option value='5'" +
           A + ">溶解</option>\
        <option value='6'" +
           O + ">合并</option>\
        </select></body>";
}
String Ti_Tock(String tx, String id, String onc, int vi) // 时间间隔样式
{
    String HTML, t1, t2, t3, t4, t5, t0;
    if (vi == 5)
        t1 = "selected";
    else if (vi == 6)
        t2 = "selected";
    else if (vi == 7)
        t3 = "selected";
    else if (vi == 8)
        t4 = "selected";
    else if (vi == 9)
        t5 = "selected";
    else
        vi = 4;
    t0 = "selected";

    return "<P><body> " + tx + " <select id='" + id + "' value='" + vi + "' onchange='" + onc + "()'>\
        <option value='4'" +
           t0 + ">无</option>\
        <option value='5'" +
           t1 + "> : </option>\
        <option value='6'" +
           t2 + "> ▲ </option>\
        <option value='7'" +
           t3 + "> ▼ </option>\
        <option value='8'" +
           t4 + "> O </option>\
        <option value='9'" +
           t5 + "> || </option>\
        </select></body>";
}
String tm_ttf(String tx, String id, String onc, int vi) // 字体
{
    String HTML, tf0, tf1, tf2, tf3, tf4, tf5, tf6;
    if (vi == 1)
        tf1 = "selected";
    else if (vi == 2)
        tf2 = "selected";
    else if (vi == 3)
        tf3 = "selected";
    else if (vi == 4)
        tf4 = "selected";
    else if (vi == 5)
        tf5 = "selected";
    else if (vi == 6)
        tf6 = "selected";
    else
        vi = 0;
    tf0 = "selected";

    return "<P><body> " + tx + " <select id='" + id + "' value='" + vi + "' onchange='" + onc + "()'>\
        <option value='0'" +
           tf0 + ">FF0000</option>\
        <option value='1'" +
           tf1 + ">Plop</option>\
        <option value='2'" +
           tf2 + ">Plop_f</option>\
        <option value='3'" +
           tf3 + ">BMBIA</option>\
        <option value='4'" +
           tf4 + ">BMBIA_f</option>\
        <option value='5'" +
           tf5 + ">Pixel</option>\
        <option value='6'" +
           tf6 + ">bold</option>\
        </select></body>";
}
String off_on(String tx, String id, String onc, bool vi) // 显示开关
{
    String HTML, F, R;
    if (vi)
    {
        R = "selected";
        F = "";
    }
    else
    {
        F = "selected";
        R = "";
    }

    return "<P><body> " + tx + " <select id='" + id + "' value='" + vi + "' onchange='" + onc + "()'>\
        <option value='0'" +
           F + ">关</option>\
        <option value='1'" +
           R + ">开</option>\
        </select></body>";
}

void handelRoot() // WEB页面初始化
{

    prefs.begin("VFD_Config", true);
    ws_brig = (prefs.getUChar("brig", ws_brig) / 25) + 48;
    ws_vfd_inverse = prefs.getBool("vfd_inv", false);  // 显示倒转
    ws_ipadd_onff = prefs.getBool("ipadd_onff", true); // IP地址显示开关
    ws_no_Toggle = prefs.getInt("no_Toggle", 0);       //  开机特效
    ws_tm_Toggle = prefs.getInt("tm_Toggle", 0);       //  时间切换特效

    ws_ttfa = prefs.getInt("ttfa", 0); // 时间字体1
    ws_ttfb = prefs.getInt("ttfb", 0); // 时间字体2

    ws_Tick = prefs.getInt("Tick", 4); // 时间间隔 Tick
    ws_Tock = prefs.getInt("Tock", 5); // 时间间隔 Tock

    ws_hostname = prefs.getString("hostname", ws_hostname);       // 主机名
    ws_wifi_ssid1 = prefs.getString("wifi_ssid1", ws_wifi_ssid1); // WIFI_SSID1
    ws_wifi_pass1 = prefs.getString("wifi_pass1", ws_wifi_pass1); // WIFI_PASS1
    ws_wifi_ssid2 = prefs.getString("wifi_ssid2", ws_wifi_ssid2); // WIFI_SSID2
    ws_wifi_pass2 = prefs.getString("wifi_pass2", ws_wifi_pass2); // WIFI_PASS2
    ws_wifi_ssid3 = prefs.getString("wifi_ssid3", ws_wifi_ssid3); // WIFI_SSID3
    ws_wifi_pass3 = prefs.getString("wifi_pass3", ws_wifi_pass3); // WIFI_PASS3

    std::ostringstream ss;
    ss << ws_brig;
    String ss_brig = ss.str().c_str();
    Serial.println(ws_Tick);
    Serial.println(ws_Tock);

    String HTML = "<!--DOCTPYE html--><html><head><meta charset='utf-8'><script>\
        var xhttp = new XMLHttpRequest();" +
                  function("nm", "name", "hostname") +
                  function("br", "brig", "change_br") +
                  function("inv", "vfd_inv", "vfd_inverse") +
                  function("lu", "lum", "change_lu") +
                  function("ip", "ip_off", "ipadd_onff") +
                  function("tm", "tmto", "tm_Toggle") +
                  function("ti", "Tick", "tm_Tick") +
                  function("to", "Tock", "tm_Tock") +
                  function("tfa", "ttfa", "tm_ttfa") +
                  function("tfb", "ttfb", "tm_ttfb") +
                  function("wi1", "wiss1", "wifi_ssid1") +
                  function("wp1", "wpa1", "wifi_pass1") +
                  function("wi2", "wiss2", "wifi_ssid2") +
                  function("wp2", "wpa2", "wifi_pass2") +
                  function("wi3", "wiss3", "wifi_ssid3") +
                  function("wp3", "wpa3", "wifi_pass3") +
                  "function restart() {xhttp.open('GET', '/re', false); xhttp.send() }" +
                  "</script></head><P>" +
                  input_text("时钟名称 ", "name", "hostname", ws_hostname) +
                  "<P><body> 屏幕亮度 <input id='brig' type='range' min='1' max='10' value='" +
                  ss_brig + "' oninput='change_br()' />\
    <script type='text/javascript'>\
    </script></body>" +
                  off_on("显示倒转", "vfd_inv", "vfd_inverse", ws_vfd_inverse) +
                  option_Toggle("开机样式", "lum", "change_lu", ws_no_Toggle) +
                  option_Toggle("时间样式", "tmto", "tm_Toggle", ws_tm_Toggle) +
                  Ti_Tock("时间间隔1", "Tick", "tm_Tick", ws_Tick) +
                  Ti_Tock("时间间隔2", "Tock", "tm_Tock", ws_Tock) +
                  off_on("IP显示开关", "ip_off", "ipadd_onff", ws_ipadd_onff) +
                  tm_ttf("时间字体1", "ttfa", "tm_ttfa", ws_ttfa) +
                  tm_ttf("时间字体2", "ttfb", "tm_ttfb", ws_ttfb) + "<P>" +
                  input_text("WIFI1: ", "wiss1", "wifi_ssid1", ws_wifi_ssid1) +
                  input_text("密码: ", "wpa1", "wifi_pass1", ws_wifi_pass1) + "<P>" +
                  input_text("WIFI2: ", "wiss2", "wifi_ssid2", ws_wifi_ssid2) +
                  input_text("密码: ", "wpa2", "wifi_pass2", ws_wifi_pass2) + "<P>" +
                  input_text("WIFI3: ", "wiss3", "wifi_ssid3", ws_wifi_ssid3) +
                  input_text("密码: ", "wpa3", "wifi_pass3", ws_wifi_pass3) + "<P>" +
                  "<P><input type='button' value='重启ESP' onclick='location.href=(\"re\")'"
                  "</html>";

    server.send(200, "text/html", HTML);
    prefs.end();
}
//   + "<P><input type='button' value='重启ESP' onchange='restart()'>"
void restart() // ESP重启
{
    String HTML = "<!--DOCTPYE html--><html><head><meta charset='utf-8'><strong>ESP已重启</strong></html>";
    server.send(200, "text/html", HTML);
    Serial.println("ESP.restart");
    delay(500);
    ESP.restart();
}
void hostname() // 修改时钟名称
{
    ws_hostname = server.arg("name");

    prefs.begin("VFD_Config", false);
    prefs.putString("hostname", ws_hostname);
    prefs.end();

    String HTML = "<!--DOCTPYE html--><html></html>";
    server.send(200, "text/html", HTML);
}
void change_br() // 修改屏幕亮度
{
    String state = server.arg("brig");
    ws_brig = atoi(state.c_str()) * 25;
    VFD_set_bright(ws_brig);

    prefs.begin("VFD_Config", false);
    prefs.putUChar("brig", ws_brig);
    prefs.end();

    String HTML = "<!--DOCTPYE html--><html></html>";
    server.send(200, "text/html", HTML);
}
void vfd_inverse() // 显示倒转
{
    // bool ipadd_onff = false;
    String s_vfd_inverse = server.arg("vfd_inv");
    if (s_vfd_inverse == "1")
        ws_vfd_inverse = true;
    else
        ws_vfd_inverse = false;

    prefs.begin("VFD_Config", false);
    prefs.putBool("vfd_inv", ws_vfd_inverse);
    prefs.end();

    String HTML = "<!--DOCTPYE html--><html></html>";
    server.send(200, "text/html", HTML);
}
void change_lu() // 修改开机样式
{
    String state = server.arg("lum");
    ws_no_Toggle = atoi(state.c_str());

    prefs.begin("VFD_Config", false);
    prefs.putInt("no_Toggle", ws_no_Toggle);
    prefs.end();

    String HTML = "<!--DOCTPYE html--><html></html>";
    server.send(200, "text/html", HTML);
}
void ipadd_onff() // 开机IP地址显示开关
{
    // bool ipadd_onff = false;
    String s_ipadd_onff = server.arg("ip_off");
    if (s_ipadd_onff == "1")
        ws_ipadd_onff = true;
    else
        ws_ipadd_onff = false;

    prefs.begin("VFD_Config", false);
    prefs.putBool("ipadd_onff", ws_ipadd_onff);
    prefs.end();

    String HTML = "<!--DOCTPYE html--><html></html>";
    server.send(200, "text/html", HTML);
}
void tm_Toggle() // 时间切换样式
{

    String state = server.arg("tmto");
    ws_tm_Toggle = atoi(state.c_str());

    prefs.begin("VFD_Config", false);
    prefs.putInt("tm_Toggle", ws_tm_Toggle);
    prefs.end();

    String HTML = "<!--DOCTPYE html--><html></html>";
    server.send(200, "text/html", HTML);
}
void tm_Tick() // 时间间隔
{
    String state = server.arg("Tick");
    ws_Tick = atoi(state.c_str());

    prefs.begin("VFD_Config", false);
    prefs.putInt("Tick", ws_Tick);
    prefs.end();

    String HTML = "<!--DOCTPYE html--><html></html>";
    server.send(200, "text/html", HTML);
}

void tm_Tock() // 时间间隔
{
    String state = server.arg("Tock");
    ws_Tock = atoi(state.c_str());

    prefs.begin("VFD_Config", false);
    prefs.putInt("Tock", ws_Tock);
    prefs.end();

    String HTML = "<!--DOCTPYE html--><html></html>";
    server.send(200, "text/html", HTML);
}
void tm_ttfa() // 时间字体A
{
    String state = server.arg("ttfa");
    ws_ttfa = atoi(state.c_str());

    prefs.begin("VFD_Config", false);
    prefs.putInt("ttfa", ws_ttfa);
    prefs.end();

    String HTML = "<!--DOCTPYE html--><html></html>";
    server.send(200, "text/html", HTML);
}
void tm_ttfb() // 时间字体B
{
    String state = server.arg("ttfb");
    ws_ttfb = atoi(state.c_str());

    prefs.begin("VFD_Config", false);
    prefs.putInt("ttfb", ws_ttfb);
    prefs.end();

    String HTML = "<!--DOCTPYE html--><html></html>";
    server.send(200, "text/html", HTML);
}

void wifi_ssid1() // wifi_ssid1
{
    ws_wifi_ssid1 = server.arg("wiss1");

    prefs.begin("VFD_Config", false);
    prefs.putString("wifi_ssid1", ws_wifi_ssid1);
    prefs.end();

    String HTML = "<!--DOCTPYE html--><html></html>";
    server.send(200, "text/html", HTML);
}
void wifi_pass1() // wifi_passwd1
{
    ws_wifi_pass1 = server.arg("wpa1");

    prefs.begin("VFD_Config", false);
    prefs.putString("wifi_pass1", ws_wifi_pass1);
    prefs.end();

    String HTML = "<!--DOCTPYE html--><html></html>";
    server.send(200, "text/html", HTML);
}
void wifi_ssid2() // wifi_ssid2
{
    ws_wifi_ssid2 = server.arg("wiss2");

    prefs.begin("VFD_Config", false);
    prefs.putString("wifi_ssid2", ws_wifi_ssid2);
    prefs.end();
    String HTML = "<!--DOCTPYE html--><html></html>";
    server.send(200, "text/html", HTML);
}
void wifi_pass2() // wifi_passwd2
{
    ws_wifi_pass2 = server.arg("wpa2");

    prefs.begin("VFD_Config", false);
    prefs.putString("wifi_pass2", ws_wifi_pass2);
    prefs.end();
    String HTML = "<!--DOCTPYE html--><html></html>";
    server.send(200, "text/html", HTML);
}
void wifi_ssid3() // wifi_ssid3
{
    ws_wifi_ssid3 = server.arg("wiss3");

    prefs.begin("VFD_Config", false);
    prefs.putString("wifi_ssid3", ws_wifi_ssid3);
    prefs.end();
    String HTML = "<!--DOCTPYE html--><html></html>";
    server.send(200, "text/html", HTML);
}
void wifi_pass3() // wifi_passwd3
{
    ws_wifi_pass3 = server.arg("wpa3");

    prefs.begin("VFD_Config", false);
    prefs.putString("wifi_pass3", ws_wifi_pass3);
    prefs.end();
    String HTML = "<!--DOCTPYE html--><html></html>";
    server.send(200, "text/html", HTML);
}
void WEB_server_begin()
{
    server.on("/", handelRoot);
    server.on("/nm", hostname);
    server.on("/br", change_br);
    server.on("/inv", vfd_inverse);
    server.on("/lu", change_lu);
    server.on("/ip", ipadd_onff);
    server.on("/tm", tm_Toggle);
    server.on("/ti", tm_Tick);
    server.on("/to", tm_Tock);
    server.on("/tfa", tm_ttfa);
    server.on("/tfb", tm_ttfb);
    server.on("/wi1", wifi_ssid1);
    server.on("/wp1", wifi_pass1);
    server.on("/wi2", wifi_ssid2);
    server.on("/wp2", wifi_pass2);
    server.on("/wi3", wifi_ssid3);
    server.on("/wp3", wifi_pass3);
    server.on("/re", restart);

    server.begin();
}
void WEB_handle()
{
    server.handleClient();
}