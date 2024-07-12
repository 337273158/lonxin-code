/******************************************************
 * 实验名称：温湿度监测实验
 *
 * 实验准备：龙芯1C102开发板，ESP8266模块，通讯底板，
            温湿度传感器，4P小白线，3P小白线
 *
 * 实验接线：ESP8266模块接到龙芯1C102开发板的UART0接口，
            使用通讯底板连接ESP8266模块的TXD和RXD接口
            到开发板的GPIO_Pin_06和GPIO_Pin_07接口，
 *
 * 实验现象：通过ESP8266上传温湿度数据至云平台
******************************************************/
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
#include "esp8266.h"

#define LED 20

char str[50];
static uint16_t temp;
static uint16_t humi;
static uint16_t ptemp;
static uint8_t shock;
uint8_t data[8] = {0x55, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x01, 0xBB}; //温湿度数据上云平台    数据包

uint8_t received_data = 0;

uint8_t Read_Buffer[255]; // 设置接收缓冲数组
uint8_t Read_length;
uint8_t da = 0;
int main(int arg, char *args[])
{
    SystemClockInit(); // 时钟等系统配置
    GPIOInit();        // io配置
    OLED_Init();
    Uart1_init(9600);
    BEEP_Init();
    EnableInt(); // 开总中断
    DL_LN3X_Init(DL_LN3X_NODE, CHANNEL, Network1_Id);
    Queue_Init(&Circular_queue);
    // sprintf(str, "体度:    ℃", ptemp / 10);
    // OLED_Show_Str(0, 0, "智能冷库安全第一", 16);    // OLED显示界面
    // sprintf(str, "行动:   ", shock);
    OLED_Show_Str(5, 1, "冷库东空调温度：  ", 16);    // OLED显示界面
    // // sprintf(str, "温度:    ℃", temp / 10);
    // OLED_Show_Str(5, 4, "当前温度:   ℃", 16);    // OLED显示界面
    // // sprintf(str, "湿度:    %%RH", humi / 10);
    // OLED_Show_Str(5, 6, "设定温度:   ℃", 16);    // OLED显示界面
    // OLED_Show_Str(10, 0, "ZigBee通信实验", 16); // OLED显示界面
    while (1)
    {
    //     delay_ms(1000);
    //    BEEP_ON;
    //    delay_ms(1000);
        data[2] = ptemp ;
        data[3] = shock;
        data[4] = temp ;
        data[5] = humi ;
        // sprintf(str, "%2d", ptemp );
        // OLED_Show_Str(52, 0, str, 16);    // OLED显示界面
        // sprintf(str, "%d", shock);
        // OLED_Show_Str(52, 2, str, 16);    // OLED显示界面
        // sprintf(str, "%2d", temp );
        // OLED_Show_Str(80, 4, str, 16);    // OLED显示界面
        sprintf(str, "%2d", 50-temp );
        OLED_Clear;
        OLED_Show_Str(58, 3, str, 16);    // OLED显示界面


        if (Queue_isEmpty(&Circular_queue) == 0) // 判断队列是否为空，即判断是否收到数据
        {
            Read_length = Queue_HadUse(&Circular_queue);           // 返回队列中数据的长度
            Queue_Read(&Circular_queue, Read_Buffer, Read_length); // 读取队列缓冲区的值到接收缓冲区
            if(Read_Buffer[6]==0x02)           //
            {
                ptemp = Read_Buffer[7] ;
                shock = Read_Buffer[8] ;

                // BEEP_ON;
            }
            if(Read_Buffer[6]==0x03)
            {
                temp = Read_Buffer[7] ;
                humi = Read_Buffer[9] ;
                // temp = Read_Buffer[7] << 8 | Read_Buffer[8];
                // humi = Read_Buffer[9] << 8 | Read_Buffer[10];
                // BEEP_ON;
            }
        }
        else
        {
            memset(Read_Buffer, 0, 255); // 填充接收缓冲区为0p
        }
        // BEEP_OFF;

        
        if(wifi_connected == 0)
        {
            if(esp8266_check_cmd('T'))     //当收到'T'字符时，表示ESP8266连接成功
            {
                // OLED_Clear(); // OLED显示界面
                wifi_connected = 1;
                // OLED_Show_Str(12, 3, "已连接到WIFI", 16);
                // delay_ms(1000);
                // OLED_Clear(); // OLED显示界面
                gpio_write_pin(LED, 1);
                BEEP_ON;
                delay_ms(500);
                BEEP_OFF;
            }
        }
        if(wifi_connected == 1)
        {
            delay_ms(2000);
        // data[2] = ptemp / 10;
        // data[3] = shock;
        // data[4] = temp / 10;
        // data[5] = humi / 10;
        // sprintf(str, "%2d", ptemp / 10);
        // OLED_Show_Str(52, 0, str, 16);    // OLED显示界面
        // sprintf(str, "%d", shock);
        // OLED_Show_Str(52, 2, str, 16);    // OLED显示界面
        // sprintf(str, "%2d", temp / 10);
        // OLED_Show_Str(52, 4, str, 16);    // OLED显示界面
        // sprintf(str, "%2d", humi / 10);
        // OLED_Show_Str(52, 6, str, 16);    // OLED显示界面
            // data[6] = (data[2] + data[3] + data[4] + data[5]) % 256;   //计算校验和
            printf("%s",data);
            UART_SendDataALL(UART1, data, 8);
            
        }
    }

    return 0;
}
