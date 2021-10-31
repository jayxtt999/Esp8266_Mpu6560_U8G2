#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <DNSServer.h> //密码直连将其三个库注释
#include <ESP8266WebServer.h>
#include <CustomWiFiManager.h>
#include <coredecls.h>
#include <ESP8266_Seniverse.h>
#include <ArduinoJson.h>
#include <MPU6050_tockn.h>

U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /*SCL*/ 14, /*SDA*/ 4, /*reset*/ U8X8_PIN_NONE);
MPU6050 mpu6050(Wire);
const char *WIFI_SSID = "TP-LINK_2E4B50"; //填写你的WIFI名称及密码
const char *WIFI_PWD = "130413041304";

/**************************天气*******************************/
#define Oled_font u8g2_font_open_iconic_weather_4x_t //u8g2天气图像font库
unsigned long lastConnectionTime = 3600000;          //设定查询时间
const unsigned long postingInterval = 3600000;       // 一小时查询一次
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp1.aliyun.com", 60 * 60 * 8, 30 * 60 * 1000);
//心知天气配置
WeatherNow weatherNow; // 建立WeatherNow对象用于获取心知天气信息
String reqUserKey = "SPo-0Lonq5pHGUtg8";
String reqLocation = "shenzhen";
String reqUnit = "c";

//现在
String weatherText = "";
int weatherCode;
int temperature;
String address = "";

DynamicJsonDocument jsonBuffer(2048);
HTTPClient http;

bool first = true;                                                                    //首次更新标志
int menu = 1;                                                                         //当前菜单位置
const String WDAY_NAMES[] = {"周六", "周天", "周一", "周二", "周三", "周四", "周五"}; //星期
const int stockX[] = {0, 55, 100};
int page = 1;
float offsetX = 3;
float offsetY = 0.75;
int change = 0;

void setup()
{
    Serial.begin(115200);
    u8g2.begin();
    u8g2.enableUTF8Print(); // enable UTF8 support for the Arduino print() function
    u8g2.setDisplayRotation(U8G2_R2);

    Wire.begin(D3, D4);
    mpu6050.begin();
    mpu6050.calcGyroOffsets(true);

    //Web配网，密码直连请注释
    //webconnect();
    wificonnect();
    timeClient.begin();
    delay(200);
}

void webconnect()
{ ////Web配网，密码直连将其注释
    showWifiTips();
    WiFiManager wifiManager;                                //实例化WiFiManager
    wifiManager.setDebugOutput(false);                      //关闭Debug
    wifiManager.setPageTitle("请选择你的WIFI并且配置密码"); //设置页标题
    if (!wifiManager.autoConnect("baidu"))
    { //AP模式
        Serial.println("连接失败并超时");
        //重新设置并再试一次，或者让它进入深度睡眠状态
        ESP.restart();
        delay(1000);
    }
    Serial.println("connected...^_^");
    yield();
}

void wificonnect()
{ //WIFI密码连接，Web配网请注释
    WiFi.begin(WIFI_SSID, WIFI_PWD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("wificonnect!!!");
    delay(500);
    updateDatas();
}

void showWifiTips()
{

    u8g2.setFont(u8g2_font_wqy12_t_gb2312);
    u8g2.setFontDirection(0);
    u8g2.clearBuffer();
    u8g2.drawUTF8(32, 15, "配置网络");
    u8g2.drawHLine(0, 16, 128);
    u8g2.drawUTF8(0, 30, " 1.打开WIFI");
    u8g2.drawUTF8(0, 45, " 2.连接WIFI \"baidu\"");
    u8g2.drawUTF8(0, 60, " 3.管理路由器");
    u8g2.sendBuffer();
}

void loop()
{
    if (first)
    {
        //首次加载
        //updateDatas();
        first = false;
    }
    draw();
    delay(500);
}

void draw()
{
    switch (menu)
    {
    case 1:
        showTime();
        break;
    case 2:
        showWeather();
        break;
    case 3:
        showStockData();
        break;
    }
}

void updateDatas()
{
    //更新时间
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_wqy12_t_gb2312);
    u8g2.drawUTF8(32, 15, "加载时间");
    progressBar(u8g2, 10, 28, 90, 10, 0);
    delay(1000);

    u8g2.clearBuffer();
    u8g2.drawUTF8(32, 15, "加载天气");
    progressBar(u8g2, 10, 28, 90, 10, 30);
    getWeatherData();
    delay(1000);

    u8g2.clearBuffer();
    u8g2.drawUTF8(32, 15, "加载股票");
    progressBar(u8g2, 10, 28, 90, 10, 60);
    delay(1000);

    u8g2.clearBuffer();
    u8g2.drawUTF8(32, 15, "加载完毕");
    progressBar(u8g2, 10, 28, 90, 10, 100);
    delay(1000);
}

void showTime()
{

    u8g2.firstPage();
    do
    {
        changeMenu();
        if (menu != 1)
        {
            return;
        }
        u8g2.clearBuffer();
        tmElements_t time;
        uint32_t delay_time;
        delay_time = 0;
        timeClient.update();
        unsigned long unix_epoch = timeClient.getEpochTime();
        time.Second = second(unix_epoch);
        time.Minute = minute(unix_epoch);
        time.Hour = hour(unix_epoch);
        time.Wday = weekday(unix_epoch);
        time.Day = day(unix_epoch);
        time.Month = month(unix_epoch);
        time.Year = year(unix_epoch);
        char buff[16];

        //u8g2.setDrawColor(1);
        u8g2.setFont(u8g2_font_logisoso34_tn);
        sprintf_P(buff, PSTR("%02d"), time.Hour);
        u8g2.drawStr(15, 45, buff);

        u8g2.setFont(u8g2_font_logisoso34_tn);
        sprintf_P(buff, PSTR("%02d"), time.Minute);
        u8g2.drawStr(80, 45, buff);

        u8g2.setFont(u8g2_font_smart_patrol_nbp_tn);
        sprintf_P(buff, PSTR("%02d"), time.Second);
        u8g2.drawStr(58, 30, buff);

        u8g2.setFont(u8g2_font_wqy12_t_gb2312);
        sprintf_P(buff, PSTR("%02d%s%02d%s"), time.Month, "月", time.Day, "日");
        u8g2.drawUTF8(10, 60, buff);

        sprintf_P(buff, PSTR("%s"), WDAY_NAMES[time.Wday].c_str());
        u8g2.drawUTF8(100, 60, buff);
        delay(delay_time);
        change = 1;
    } while (u8g2.nextPage());
}

void showWeather()
{

    u8g2.firstPage();
    do
    {
        changeMenu();
        if (menu != 2)
        {
            return;
        }

        drawWeatherSymbol(25, 35, weatherCode);

        // Define
        char buff[40];
        sprintf_P(buff, PSTR("%02d"), temperature);
        u8g2.setFont(u8g2_font_inb33_mn);
        u8g2.drawStr(60, 35, buff);

        sprintf_P(buff, PSTR("%s %s"), address.c_str(), weatherText.c_str());
        u8g2.setFont(u8g2_font_wqy12_t_gb2312);
        u8g2.drawUTF8(25, 56, buff);
        change = 1;
    } while (u8g2.nextPage());

    change = 1;
}

void progressBar(U8G2 u8g2, uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t percent)
{
    // can't draw it smaller than 10x8
    height = height < 8 ? 8 : height;
    width = width < 10 ? 10 : width;

    // draw percentage
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(x + width + 2, y + height / 2 + 2, (String(percent) + String("%")).c_str());

    // draw it
    u8g2.drawRFrame(x, y, width, height, 4);
    u8g2.drawBox(x + 2, y + 2, (width - 4) * (percent / 100.0), height - 4);
}

void getWeatherData()
{
    weatherNow.config(reqUserKey, reqLocation, reqUnit);
    if (weatherNow.update())
    { // 更新天气信息
        Serial.println(F("======Weahter Info======"));
        Serial.print("Server Response: ");
        Serial.println(weatherNow.getServerCode()); // 获取服务器响应码
        Serial.print(F("Weather Now: "));
        Serial.print(weatherNow.getWeatherText()); // 获取当前天气（字符串格式）
        Serial.print(F("Weather CODE: "));
        Serial.println(weatherNow.getWeatherCode()); // 获取当前天气（整数格式）
        Serial.print(F("Temperature: "));
        Serial.println(weatherNow.getDegree()); // 获取当前温度数值
        Serial.print(F("Last Update: "));
        Serial.println(weatherNow.getLastUpdate()); // 获取服务器更新天气信息时间
        Serial.print(F("Address: "));
        Serial.println(weatherNow.getAddress()); // 获取服务器更新天气信息时间
        Serial.println(F("========================"));

        weatherText = weatherNow.getWeatherText();
        weatherCode = weatherNow.getWeatherCode();
        temperature = weatherNow.getDegree();
        address = weatherNow.getAddress();

        lastConnectionTime = (signed long)millis();
    }
    else
    { // 更新失败
        Serial.println("Update Fail...");
        Serial.print("Server Response: ");          // 输出服务器响应状态码供用户查找问题
        Serial.println(weatherNow.getServerCode()); // 心知天气服务器错误代码说明可通过以下网址获取
    }
}

void drawWeatherSymbol(int x, int y, uint8_t symbol)
{
    switch (symbol)
    {
    //case后面为心知天气气象代码
    case 0:
        u8g2.setFont(Oled_font);
        u8g2.drawGlyph(x, y, 69); //晴天
        break;
    case 1:
        u8g2.setFont(Oled_font);
        u8g2.drawGlyph(x, y, 68); //夜晚晴
        break;
    case 4:
        u8g2.setFont(Oled_font);
        u8g2.drawGlyph(x, y, 65); //晴间多云
        break;
    case 5:
        u8g2.setFont(Oled_font);
        u8g2.drawGlyph(x, y, 64); //多云
        break;
    case 6:
        u8g2.setFont(Oled_font);
        u8g2.drawGlyph(x, y, 64); //多云
        break;
    case 7:
        u8g2.setFont(Oled_font);
        u8g2.drawGlyph(x, y, 64); //多云
        break;
    case 8:
        u8g2.setFont(Oled_font);
        u8g2.drawGlyph(x, y, 64); //多云
        break;
    case 9:
        u8g2.setFont(Oled_font);
        u8g2.drawGlyph(x, y, 64); //多云
    case 30:
        u8g2.setFont(Oled_font);
        u8g2.drawGlyph(x, y, 64); //多云
        break;
    case 10:
        u8g2.setFont(Oled_font);
        u8g2.drawGlyph(x, y, 67); //雨
        break;
    case 11:
        u8g2.setFont(Oled_font);
        u8g2.drawGlyph(x, y, 67); //雨
        break;
    case 12:
        u8g2.setFont(Oled_font);
        u8g2.drawGlyph(x, y, 67); //雨
        break;
    case 13:
        u8g2.setFont(Oled_font);
        u8g2.drawGlyph(x, y, 67); //雨
        break;
    case 14:
        u8g2.setFont(Oled_font);
        u8g2.drawGlyph(x, y, 67); //雨
        break;
    case 15:
        u8g2.setFont(Oled_font);
        u8g2.drawGlyph(x, y, 67); //雨
        break;
    }
}

void getStockData()
{
    WiFiClient client;
    HTTPClient http;
    //String url = "http://103.39.214.86:3333/list?page=";//配置服务地址
    String url = "http://127.0.0.1:3333/list?page=";//配置服务地址
    url += page;
    url += "&page_size=5";
    Serial.println(url);
    http.setTimeout(5000);
    http.begin(client, url);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        //判断请求是否成功
        if (httpCode == HTTP_CODE_OK)
        {
            //读取响应内容
            String json = http.getString();
            Serial.println(json); //打印json数据
            deserializeJson(jsonBuffer, json);
        }
    }
    else
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}

void changeMenu()
{
    for (int i = 0; i < 200; i++)
    {
        mpu6050.update();
    }
    float angleX = mpu6050.getAngleX() - offsetX;
    float angleY = mpu6050.getAngleY() - offsetY;
    //Serial.println(angleX);
    Serial.println(angleY);
    if (angleX > 20)
    {
        menu = 1;
        return;
    }
    if (angleY > 15 && change == 1)
    {
        menu--;
        change = 0;
    }
    if (angleY < -15 && change == 1)
    {
        menu++;
        change = 0;
    }
    if (menu < 1)
    {
        menu = 1;
    }
    if (menu > 3)
    {
        menu = 3;
    }
    Serial.println(menu);
}

void showStockData()
{

    u8g2.firstPage();
    do
    {
        changeMenu();
        if (menu != 3)
        {
            return;
        }
        getStockData();

        // // Parse JSON object
        JsonObject root = jsonBuffer.as<JsonObject>();

        u8g2.setFont(u8g2_font_wqy12_t_gb2312);
        u8g2.drawUTF8(0, 10, "初始");
        u8g2.drawUTF8(55, 10, "最新");
        u8g2.drawUTF8(100, 10, "涨幅");
        //const char *message = root["message"];
        int _code = root["code"].as<int>();
        int _next_page = root["data"]["next_page"].as<int>();
        // // //u8g2.drawUTF8(40, 40,message);
        if (_code == 1)
        {
            JsonObject data_stock = root["data"]["stock"];
            int size = data_stock.size();
            if (size > 0)
            {
                int y = 20;
                for (JsonPair kv : data_stock)
                {
                    //Serial.println(kv.key().c_str());
                    JsonObject v2 = kv.value().as<JsonObject>();
                    u8g2.setFont(u8g2_font_wqy12_t_gb2312);
                    u8g2.drawUTF8(stockX[0], y, v2["title"].as<char *>());

                    u8g2.setFont(u8g2_font_profont10_tn);
                    u8g2.drawUTF8(stockX[1], y, v2["zx"].as<char *>());
                    u8g2.drawUTF8(stockX[2], y, v2["zdf"].as<char *>());
                    y += 10;
                }
                if (_next_page > 0)
                {
                    page = _next_page;
                }
                else
                {
                    page = 1;
                }
            }
        }
        else
        {
            u8g2.drawUTF8(40, 40, "请求失败...");
        }
        change = 1;
        delay(2000);
    } while (u8g2.nextPage());
}