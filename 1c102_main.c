
#include "ls1x.h"
#include "Config.h"
#include "ls1x_gpio.h"
#include "ls1x_latimer.h"
#include "ZigBee.h"
#include "ls1c102_interrupt.h"
#include "iic.h"
#include "UserGpio.h"
#include "oled.h"
#include "dht11.h"
#include "BEEP.h"
#include "key.h"
#include "led.h"
#include "queue.h"
#include "ls1x_clock.h"
#define LED 20

char str[50];
static uint16_t temperature;
static uint16_t humidity;
uint8_t cnt;
uint8_t received_data = 0;
uint8_t data[5];
uint8_t Read_Buffer[255]; // 设置接收缓冲数组
uint8_t Read_length;
int main(int arg, char *args[])
{
    SystemClockInit(); // 时钟等系统配置
    GPIOInit();        // io配置
    OLED_Init();
    Init_shock();
    EnableInt(); // 开总中断
    // Uart1_init(9600);
    DL_LN3X_Init(0X0003,CHANNEL,Network1_Id);//设置为主机（接收端），设置信道为0x12，网络地址为0x0003
    Queue_Init(&Circular_queue);
    BEEP_Init();
    DHT11_Init();                               // DHT11初始化  
    OLED_Show_Str(40, 0, "store 1", 16); // OLED显示界面
    OLED_Show_Str(20, 3, "温度:     ℃", 16);     // OLED显示界面
    OLED_Show_Str(20 ,6, "湿度:    %RH", 16);
    while (1)
    {
        DHT11_Read_Data(&temperature, &humidity);
        data[0] = 0x03;
        data[1] = temperature;
        // data[2] = temperature % 256;
        // data[2] = shock_get();
        data[3] = humidity;
        // data[4] = humidity % 256;
        // sprintf(str, "%2d", temperature / 10);
        // OLED_Show_Str(70, 3, str, 16);           //显示WEN度
        sprintf(str, "%2d", temperature);
        OLED_Show_Str(63, 3, str, 16);    // OLED显示界面
        sprintf(str, "%2d", humidity );
        OLED_Show_Str(63, 6, str, 16);    // OLED显示界面
        // if(shock_get()==1)
        // {
        // OLED_Show_Str(70, 6, "正常", 16);           //显示state
        // BEEP_OFF;
        // }
        // else
        // {
        // // OLED_Show_Str(70, 6, "异常", 16); 
        // cnt++;
        // if(cnt==20)
        //     {
        //     BEEP_ON;
        //     OLED_Show_Str(70, 6, "异常", 16);
        //     }
        // }
        // sprintf(str, "温度: %2d ℃", temperature / 10);
        // OLED_Show_Str(5, 4, str, 16);    // OLED显示界面
        // sprintf(str, "湿度: %2d %%RH", humidity / 10);
        // OLED_Show_Str(5, 6, str, 16);    // OLED显示界面
        delay_ms(200);
        DL_LN3X_Send(data, 5, ZIGBEE_RX_NODE);
    }

    return 0;
}







// /******************************************************
//  * 实验名称：温湿度监测实验
//  *
//  * 实验准备：龙芯1C102开发板，ESP8266模块，通讯底板，
//             温湿度传感器，4P小白线，3P小白线
//  *
//  * 实验接线：ESP8266模块接到龙芯1C102开发板的UART0接口，
//             使用通讯底板连接ESP8266模块的TXD和RXD接口
//             到开发板的GPIO_Pin_06和GPIO_Pin_07接口，
//  *
//  * 实验现象：通过ESP8266上传温湿度数据至云平台
// ******************************************************/
// #include "ls1x.h"
// #include "Config.h"
// #include "ls1x_gpio.h"
// #include "ls1x_latimer.h"
// #include "esp8266.h"
// #include "ls1c102_interrupt.h"
// #include "iic.h"
// #include "oled.h"
// #include "dht11.h"
// #include "BEEP.h"
// #include "key.h"
// #include "led.h"


// #define LED 20

// char str[50];
// static uint16_t temperature;
// static uint16_t humidity;
// // char shock[7];
// uint8_t data[8] = {0x55, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB}; //温湿度数据上云平台    数据包
// uint8_t cnt = 0;
// int main(int arg, char *args[])
// {
//     SystemClockInit(); // 时钟等系统配置
//     GPIOInit();        // io配置
//     OLED_Init();
//     EnableInt(); // 开总中断
//     Init_shock();
//     // Queue_Init(&Circular_queue);
//     BEEP_Init();
//     Uart0_init(9600); // 串口0初始化，io06 io07   串口初始化需要在开启EnableInt之后

//     // OLED_Show_Str(10, 0, "温湿度监测实验", 16); // OLED显示界面

//     while (DHT11_Init()) // 检测是否接入温湿度传感器
//     {
//         OLED_Show_Str(10, 4, "未检测到传感器", 16); // OLED显示界面
//     }
//     OLED_Show_Str(0, 0, "人员健康监测系统", 16); // OLED显示界面
//     OLED_Show_Str(20, 3, "体温:     ℃", 16);     // OLED显示界面
//     OLED_Show_Str(20 ,6, "行动: 正常", 16);
//     while (1)
//     {
//         DHT11_Read_Data(&temperature, &humidity); // 读取温湿度值state
//         // char shock[] = shock_get();
//         data[2] = temperature / 10;     //将温湿度数据存放至数据包中
//         data[3] = shock_get();
//         // data[3] = humidity / 10;

//         // sprintf(str, "%2d", temperature / 10);     
//         // OLED_Show_Str(70, 0, str, 16);           
//         sprintf(str, "%2d", temperature / 10);
//         OLED_Show_Str(70, 3, str, 16);           //显示WEN度
//         // sprintf(str, "%d", shock_get());
//         if(shock_get()==1)
//         {
//         OLED_Show_Str(70, 6, "正常", 16);           //显示state
//         BEEP_OFF;
//         // delay_ms(1500);
//         }
//         else{OLED_Show_Str(70, 6, "异常", 16); 
//         cnt++;

//         if(cnt==10)BEEP_ON;
        
//         }
//         if(wifi_connected == 0)
//         {
//             if(esp8266_check_cmd('T'))  //当收到'T'字符时，表示ESP8266连接成功
//             {
//                 OLED_Clear(); // OLED显示界面
//                 wifi_connected = 1;
//                 OLED_Show_Str(12, 3, "已连接到WIFI", 16);
//                 delay_ms(1000);
//                 OLED_Clear(); // OLED显示界面
//                 delay_ms(500);
//                 OLED_Show_Str(0, 0, "人员健康监测系统", 16); // OLED显示界面
//                 OLED_Show_Str(20, 3, "体温:    ℃", 16);     // OLED显示界面
//                 OLED_Show_Str(20, 6, "行动:   ", 16);
//                 gpio_write_pin(LED, 1);
//                 BEEP_ON;
//                 delay_ms(500);
//                 BEEP_OFF;
//             }
//         }
//         if(wifi_connected == 1)
//         {
//             delay_ms(20);
//             data[6] = (data[2] + data[3] + data[4] + data[5]) % 256;   //计算校验和
//             printf("%s",data);
//             UART_SendDataALL(UART0, data, 8);
            
//         }
//     }

//     return 0;
// }
