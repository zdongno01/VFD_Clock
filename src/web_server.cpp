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

// html script 函数生成器
String generateFunction(String funcName, String elementId, String endpoint, String paramName)
{
    return "        function " + funcName + "() {\n" +
           "            var value = document.getElementById('" + elementId + "').value;\n" +
           "            xhttp.open('GET', '/" + endpoint + "?" + paramName + "=' + value, true);\n" +
           "            xhttp.send();\n" +
           "        }\n";
}

// 通用函数，用于生成表单组
String generateFormGroup(String labelText, String inputHtml)
{
    return "    <div class=\"form-group\">\n <label>" + labelText + "</label>\n" +
           inputHtml +
           "    </div>\n";
}

String generateFormGroup(String labelText1, String labelText2, String labelText3, String labelText4)
{
    return "<div style=\"flex: 1;\">\
        <label>" + labelText1 + "</label>\
        <input type='text' id='"  + labelText2 +  "' value='" + labelText4 + "' onchange='"  + labelText3 +  "'>\
    </div>";
}
// 通用函数，用于生成选择框
String generateSelect(String id, String value, String onChange, String options)
{
    return "        <select id='" + id + "' value='" + value + "' onchange='" + onChange + "()'>\n" +
           options +
           "        </select>\n";
}

// 生成选择框选项
String generateToggleOptions(int selectedValue)
{
    String options[6] = {"无", "从左滑入", "从右滑入", "从上滑入", "从下滑入", "溶解"};
    String result = "";
    for (int i = 0; i < 6; i++)
    {
        result += "            <option value='" + String(i) + "'" + (i == selectedValue ? " selected" : "") + ">" + options[i] + "</option>\n";
    }
    return result;
}

String generateTickTockOptions(int selectedValue)
{
    String options[6] = {"无", " : ", " ▲ ", " ▼ ", " O ", " || "};
    String result = "";
    for (int i = 0; i < 6; i++)
    {
        result += "            <option value='" + String(i + 4) + "'" + (i + 4 == selectedValue ? " selected" : "") + ">" + options[i] + "</option>\n";
    }
    return result;
}

String generateFontOptions(int selectedValue)
{
    String options[7] = {"FF0000", "Plop", "Plop_f", "BMBIA", "BMBIA_f", "Pixel", "bold"};
    String result = "";
    for (int i = 0; i < 7; i++)
    {
        result += "            <option value='" + String(i) + "'" + (i == selectedValue ? " selected" : "") + ">" + options[i] + "</option>\n";
    }
    return result;
}

String generateOnOffOptions(bool isOn)
{
    if (isOn)
    {
        return "            <option value='0'>关</option>\n <option value='1' selected>开</option>\n";
    }
    else
    {
        return "            <option value='0' selected>关</option>\n <option value='1'>开</option>\n";
    };
}

// 在 handelRoot 函数中使用通用函数生成 HTML
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

    String html = "<!DOCTYPE html>\n";
    html += "<html>\n";
    html += "<head>\n";
    html += "    <meta charset='utf-8'>\n";
    html += "    <style>\n";
    html += "        body {\n";
    html += "            font-family: Arial, sans-serif;\n";
    html += "            max-width: 800px;\n";
    html += "            margin: 20px auto;\n";
    html += "            padding: 20px;\n";
    html += "            background-color: #f4f4f4;\n";
    html += "        }\n";
    html += "        .form-group {\n";
    html += "            margin-bottom: 15px;\n";
    html += "        }\n";
    html += "        label {\n";
    html += "            display: block;\n";
    html += "            margin-bottom: 5px;\n";
    html += "            font-weight: bold;\n";
    html += "        }\n";
    html += "        input[type=\"text\"],\n";
    html += "        input[type=\"range\"],\n";
    html += "        select {\n";
    html += "            width: 100%;\n";
    html += "            padding: 8px;\n";
    html += "            border: 1px solid #ccc;\n";
    html += "            border-radius: 4px;\n";
    html += "            box-sizing: border-box;\n";
    html += "        }\n";
    html += "        input[type=\"button\"] {\n";
    html += "            background-color: #4CAF50;\n";
    html += "            color: white;\n";
    html += "            padding: 10px 20px;\n";
    html += "            border: none;\n";
    html += "            border-radius: 4px;\n";
    html += "            cursor: pointer;\n";
    html += "        }\n";
    html += "        input[type=\"button\"]:hover {\n";
    html += "            background-color: #45a049;\n";
    html += "        }\n";
    html += "    </style>\n";
    html += "    <script>\n";
    html += "        var xhttp = new XMLHttpRequest();\n";

    // 生成各个script函数
    html += generateFunction("hostname", "name", "nm", "name");
    html += generateFunction("change_br", "brig", "br", "brig");
    html += generateFunction("vfd_inverse", "vfd_inv", "inv", "vfd_inv");
    html += generateFunction("change_lu", "lum", "lu", "lum");
    html += generateFunction("ipadd_onff", "ip_off", "ip", "ip_off");
    html += generateFunction("tm_Toggle", "tmto", "tm", "tmto");
    html += generateFunction("tm_Tick", "Tick", "ti", "Tick");
    html += generateFunction("tm_Tock", "Tock", "to", "Tock");
    html += generateFunction("tm_ttfa", "ttfa", "tfa", "ttfa");
    html += generateFunction("tm_ttfb", "ttfb", "tfb", "ttfb");
    html += generateFunction("wifi_ssid1", "wiss1", "wi1", "wiss1");
    html += generateFunction("wifi_pass1", "wpa1", "wp1", "wpa1");
    html += generateFunction("wifi_ssid2", "wiss2", "wi2", "wiss2");
    html += generateFunction("wifi_pass2", "wpa2", "wp2", "wpa2");
    html += generateFunction("wifi_ssid3", "wiss3", "wi3", "wiss3");
    html += generateFunction("wifi_pass3", "wpa3", "wp3", "wpa3");

    // 重启script
    html += "        function restart() {\n";
    html += "            xhttp.open('GET', '/re', false);\n";
    html += "            xhttp.send();\n";
    html += "        }\n";
    html += "    </script>\n";
    html += "</head>\n";
    html += "<body>\n";
    html += "    <h1>时钟设置</h1>\n";
    html += generateFormGroup("时钟名称", "        <input type='text' id='name' value='" + ws_hostname + "' onchange='hostname()'>\n");
    html += generateFormGroup("屏幕亮度", "        <input id='brig' type='range' min='1' max='10' value='" + ss_brig + "' oninput='change_br()'>\n");
    html += generateFormGroup("显示倒转", generateSelect("vfd_inv", String(ws_vfd_inverse), "vfd_inverse", generateOnOffOptions(ws_vfd_inverse)));
    html += generateFormGroup("开机样式", generateSelect("lum", String(ws_no_Toggle), "change_lu", generateToggleOptions(ws_no_Toggle)));
    html += generateFormGroup("时间样式", generateSelect("tmto", String(ws_tm_Toggle), "tm_Toggle", generateToggleOptions(ws_tm_Toggle)));
    html += "    <div class=\"form-group\" style=\"display: flex; gap: 10px;\">\n";
    html += "        <div style=\"flex: 1;\">\n";
    html += "            <label>时间间隔</label>\n";
    html += generateSelect("Tick", String(ws_Tick), "tm_Tick", generateTickTockOptions(ws_Tick));
    html += "        </div>\n";
    html += "        <div style=\"flex: 1;\">\n";
    html += "            <label>　</label>\n";
    html += generateSelect("Tock", String(ws_Tock), "tm_Tock", generateTickTockOptions(ws_Tock));
    html += "        </div>\n";
    html += "    </div>\n";
    html += generateFormGroup("IP显示开关", generateSelect("ip_off", String(ws_ipadd_onff), "ipadd_onff", generateOnOffOptions(ws_ipadd_onff)));
    html += "    <div class=\"form-group\" style=\"display: flex; gap: 10px;\">\n";
    html += "        <div style=\"flex: 1;\">\n";
    html += "            <label>时间字体</label>\n";
    html += generateSelect("ttfa", String(ws_ttfa), "tm_ttfa", generateFontOptions(ws_ttfa));
    html += "        </div>\n";
    html += "        <div style=\"flex: 1;\">\n";
    html += "            <label>　</label>\n";
    html += generateSelect("ttfb", String(ws_ttfb), "tm_ttfb", generateFontOptions(ws_ttfb));
    html += "        </div>\n";
    html += "    </div>\n";
    html += "    <div class=\"form-group\" style=\"display: flex; gap: 10px;\">";
    html += generateFormGroup("WIFI1", "wiss1", "wifi_ssid1()", ws_wifi_ssid1);
    html += generateFormGroup("Pass1", "wpa1",  "wifi_pass1()", ws_wifi_pass1);
    html += "    </div>\n";

    html += "    <div class=\"form-group\" style=\"display: flex; gap: 10px;\">";
    html += generateFormGroup("WIFI2", "wiss2", "wifi_ssid2()", ws_wifi_ssid2);
    html += generateFormGroup("Pass2", "wpa2",  "wifi_pass2()", ws_wifi_pass2);
    html += "    </div>\n";
    
    html += "    <div class=\"form-group\" style=\"display: flex; gap: 10px;\">";
    html += generateFormGroup("WIFI3", "wiss3", "wifi_ssid3()", ws_wifi_ssid3);
    html += generateFormGroup("Pass3", "wpa3",  "wifi_pass3()", ws_wifi_pass3);
    html += "    </div>\n";

    html += "    <input type='button' value='重启ESP' onclick='restart()'>\n";
    html += "</body>\n";
    html += "</html>";

    server.send(200, "text/html", html);
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

// 通用函数，用于处理参数更新和保存
template <typename T>
void handleParameterUpdate(const String &argName, T &globalVar, const char *prefKey, T (*converter)(const String &))
{
    String state = server.arg(argName);
    globalVar = converter(state);

    prefs.begin("VFD_Config", false);
    // 由于 is_same_v 可能在某些编译器环境下不可用，使用 std::is_same 替代
    if constexpr (std::is_same<T, String>::value)
    {
        prefs.putString(prefKey, globalVar);
    }
    else if constexpr (std::is_same<T, bool>::value)
    {
        prefs.putBool(prefKey, globalVar);
    }
    else if constexpr (std::is_same<T, int>::value)
    {
        prefs.putInt(prefKey, globalVar);
    }
    else if constexpr (std::is_same<T, unsigned char>::value)
    {
        prefs.putUChar(prefKey, globalVar);
    }
    prefs.end();

    String HTML = "<!--DOCTPYE html--><html></html>";
    server.send(200, "text/html", HTML);
}

// 转换字符串为布尔值
bool stringToBool(const String &str)
{
    return str == "1";
}

// 转换字符串为整数
int stringToInt(const String &str)
{
    return atoi(str.c_str());
}

void hostname() // 修改时钟名称
{
    handleParameterUpdate<String>("name", ws_hostname, "hostname", [](const String &s)
                                  { return s; });
}

void vfd_inverse() // 显示倒转
{
    handleParameterUpdate<bool>("vfd_inv", ws_vfd_inverse, "vfd_inv", stringToBool);
}

void change_lu() // 修改开机样式
{
    handleParameterUpdate<int>("lum", ws_no_Toggle, "no_Toggle", stringToInt);
}

void ipadd_onff() // 开机IP地址显示开关
{
    handleParameterUpdate<bool>("ip_off", ws_ipadd_onff, "ipadd_onff", stringToBool);
}

void tm_Toggle() // 时间切换样式
{
    handleParameterUpdate<int>("tmto", ws_tm_Toggle, "tm_Toggle", stringToInt);
}

void tm_Tick() // 时间间隔
{
    handleParameterUpdate<int>("Tick", ws_Tick, "Tick", stringToInt);
}

void tm_Tock() // 时间间隔
{
    handleParameterUpdate<int>("Tock", ws_Tock, "Tock", stringToInt);
}

void tm_ttfa() // 时间字体A
{
    handleParameterUpdate<int>("ttfa", ws_ttfa, "ttfa", stringToInt);
}

void tm_ttfb() // 时间字体B
{
    handleParameterUpdate<int>("ttfb", ws_ttfb, "ttfb", stringToInt);
}

void wifi_ssid1() // wifi_ssid1
{
    handleParameterUpdate<String>("wiss1", ws_wifi_ssid1, "wifi_ssid1", [](const String &s)
                                  { return s; });
}

void wifi_pass1() // wifi_passwd1
{
    handleParameterUpdate<String>("wpa1", ws_wifi_pass1, "wifi_pass1", [](const String &s)
                                  { return s; });
}

void wifi_ssid2() // wifi_ssid2
{
    handleParameterUpdate<String>("wiss2", ws_wifi_ssid2, "wifi_ssid2", [](const String &s)
                                  { return s; });
}

void wifi_pass2() // wifi_passwd2
{
    handleParameterUpdate<String>("wpa2", ws_wifi_pass2, "wifi_pass2", [](const String &s)
                                  { return s; });
}

void wifi_ssid3() // wifi_ssid3
{
    handleParameterUpdate<String>("wiss3", ws_wifi_ssid3, "wifi_ssid3", [](const String &s)
                                  { return s; });
}

void wifi_pass3() // wifi_passwd3
{
    handleParameterUpdate<String>("wpa3", ws_wifi_pass3, "wifi_pass3", [](const String &s)
                                  { return s; });
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