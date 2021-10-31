### Wemos D1 mini OLED 1.3 显示时间 天气 股票
#### 外壳 元器件 程序
元器件
- D1mini（理论上只要是esp8266的都可以）
- mpu6050
- 1.3寸OLED显示屏（此项目基于SH1106，其实无所谓，只要能使用U8g2库就行）
- 开关螺丝杜邦线若干

`https://www.thingiverse.com/thing:2934049` 基于此项目的改动，使用mpu6050来替换按键切换
左倾切换为上一屏 右倾切换为下一屏 后倾则回到第一屏

#### 接线
![image](https://github.com/jayxtt999/Esp8266_Mpu6560_U8G2/blob/master/image/Esp8266_Mpu6560_U8G2_bb.svg)
D1mini    Mpu6560
D3 -> SCL
D4 -> SDA
GND -> GND
VCC -> VCC

D1mini    OLED1.3 (SH1106)
D5 -> SCL
D2 -> SDA
GND -> GND
VCC -> VCC

#### 代码需要改动项目
修改`getStockData`方法中的地址，股票数据需要搭建一个服务提供接口用于显示数据，这里使用了php在(https://github.com/jayxtt999/Esp8266_Mpu6560_U8G2_PHP)

### TODO
- 倾斜切换需要调优，实际操作中不太灵敏
- 珍惜生命，原理A股[狗][狗][狗]


![image](https://github.com/jayxtt999/Esp8266_Mpu6560_U8G2/blob/master/image/20211031183837.jpg)
![image](https://github.com/jayxtt999/Esp8266_Mpu6560_U8G2/blob/master/image/20211031183115.jpg)
![image](https://github.com/jayxtt999/Esp8266_Mpu6560_U8G2/blob/master/image/20211031183224.jpg)
![image](https://github.com/jayxtt999/Esp8266_Mpu6560_U8G2/blob/master/image/test.gif)



