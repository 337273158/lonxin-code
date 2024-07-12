//连接wifi后登陆MQTT，然后每1s上报一次数据(数据每次加1)
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <String.h>
#include <Ticker.h>
#include <stdio.h>

#define led 2  //发光二极管连接在8266的GPIO2上

#define WIFI_SSID "test"    //wifi名称
#define WIFI_PASSWORD "00000000"  //wifi密码
//MQTT三元组
#define ClientId "ZBDX-TEST-01"
#define Username "admin"
#define Password "bkrc2023"

#define MQTT_Address "115.28.209.116"
#define MQTT_Port 1883
#define Iot_link_MQTT_Topic_Report "device/f307b2a7da27c622/up"
#define Iot_link_MQTT_Topic_Commands "device/f307b2a7da27c622/down"

WiFiClient myesp8266Client;
PubSubClient client(myesp8266Client);
StaticJsonBuffer<300> jsonBuffer;
//StaticJsonDocument<200> jsonBuffer2; //声明一个sonDocument对象，长度200
char printf_buf[16] = { 0 };
unsigned char buffer[32];
unsigned char network_buffer[5];
char total_data[50] = { 0 };

unsigned char rx_buffer[32];
uint16_t rx_length = 0;

int data_temp = 1;
int mqtt_state = 0;
int mqtt_sub_state = 1;

uint16_t Smoke_value = 0, bh1750_value = 0, balance = 0, door_flag = 0;
void setup() {
  // put your setup code here, to run once:
  pinMode(led, HIGH);
  Serial.begin(9600);
  WIFI_Init();
  MQTT_Init();
}
void loop() {
  client.loop();  //保持MQTT服务器连接
  if (Serial.available()) {
    //读取串口数据
    rx_buffer[rx_length] = Serial.available();
    if (rx_length == 0) {
      if (rx_buffer[0] == 0x55)  //包头数据判断
      {
        rx_length++;
      } else {
        rx_length = 0;
      }
    } else {
      // 读取串口数据
      rx_buffer[rx_length] = Serial.available();
      if (rx_buffer[rx_length] == 0xBB) {
        rx_length = 0;
      } else {
        // 指针自增
        rx_length++;
        // 如果储存超出长度
        if (rx_length == 27) {
          // 重新从数组0开始储存字节
          rx_length = 0;
        }
      }
    }
    //如果储存超出长度
    if (rx_length >= 27) {
      //重新从数组0开始储存字节
      rx_length = 0;
    }
    Serial.readBytes(rx_buffer, Serial.available());

    //将串口发来的数据进行转换JSON数据发布到云平台
    protocolJSON(rx_buffer);
    //清空串口数据
    while (Serial.read() >= 0) {};
  }
  // put your main code here, to run repeatedly:
  if (WiFi.status() == WL_CONNECTED) {
    //MQTT_POST();
  } else {
    WIFI_Init();
  }
  if (!client.connected() && mqtt_state == 1) {
    client.connect(ClientId, Username, Password);
    if (client.connect(ClientId)) {
      // Serial.println("connected");
      // 连接成功时订阅主题
      client.subscribe(Iot_link_MQTT_Topic_Commands);

    } else {

      //      Serial.print("failed, rc=");
      //      Serial.print(client.state());
      //      Serial.println(" try again in 5 seconds");
      network_buffer[0] = 0xAA;
      network_buffer[1] = 0x02;
      network_buffer[2] = 0x00;
      network_buffer[3] = 0x00;
      network_buffer[4] = 0xBB;
      Serial.write(network_buffer, 5);
      MQTT_Init();
      delay(500);
    }
  }
  delay(500);
  //data_temp++;
}
//连接wifi
void WIFI_Init() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(led, LOW);
    // Serial.println("WiFi Not Connect");
    network_buffer[0] = 0x55;
    network_buffer[1] = 0xAA;
    network_buffer[2] = 0x00;
    network_buffer[3] = 0x00;
    network_buffer[4] = 0x00;
    network_buffer[5] = 0xBB;
    Serial.write(network_buffer, 6);
  }
  digitalWrite(led, LOW);
  delay(500);
  digitalWrite(led, HIGH);
  // Serial.println("WiFi Connected OK!");
  network_buffer[0] = 0x55;
  network_buffer[1] = 0xAA;
  network_buffer[2] = 0x01;
  network_buffer[3] = 0x00;
  network_buffer[4] = 0x00;
  network_buffer[5] = 0xBB;
  Serial.write(network_buffer, 6);
  MQTT_Init();
}
//连接MQTT
void MQTT_Init() {
  client.setServer(MQTT_Address, MQTT_Port);
  client.setClient(myesp8266Client);
  client.subscribe(Iot_link_MQTT_Topic_Commands);
  client.setCallback(callback);

  while (!client.connected()) {
    client.connect(ClientId, Username, Password);
    if (client.connect(ClientId)) {
      // Serial.println("connected");
      // 连接成功时订阅主题
      client.subscribe(Iot_link_MQTT_Topic_Commands);

    } else {

      //      Serial.print("failed, rc=");
      //      Serial.print(client.state());
      //      Serial.println(" try again in 5 seconds");
      network_buffer[0] = 0x55;
      network_buffer[1] = 0xAA;
      network_buffer[2] = 0x02;
      network_buffer[3] = 0x00;
      network_buffer[4] = 0x00;
      network_buffer[5] = 0xBB;
      Serial.write(network_buffer, 6);
      MQTT_Init();
      delay(500);
    }
  }
  //先网关发送链接成功数据
  network_buffer[0] = 'T';
  Serial.write(network_buffer, 6);
  mqtt_state = 1;
  // Serial.println("MQTT Connected OK!");
}
//DynamicJsonDocument doc(1024);
/* 云端下发 */
void callback(char* topic, byte* payload, unsigned int length) {
  //  for (int i = 0; i < length; i++) {
  //    Serial.print((char)payload[i]);
  //  }
  mqtt_sub_state = 0;
  JsonObject& root = jsonBuffer.parseObject(payload);
  // Test if parsing succeeds.
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  String sign = root["message"];

  JsonObject& root1 = jsonBuffer.parseObject(sign);
  String sign1 = root1["sign"];

  //接收到，led灯闪烁一次
  digitalWrite(2, LOW);
  delay(100);
  digitalWrite(2, HIGH);

  jsonBuffer.clear();
}

void protocolJSON(unsigned char JsonData[]) {
  // 创建缓冲区用于存储JSON消息
  char JSONmessageBuffer[1024];
  char JSONmessageBuffer1[1024];

  // 定义静态的JSON缓冲区用于存储JSON消息
  StaticJsonBuffer<1024> json_buffer;
  JsonObject& root = json_buffer.createObject();
  JsonObject& data = root.createNestedObject("data");
  root["sign"] = "f307b2a7da27c622";
  root["type"] = 1;

  // 初始化temp和Light字段
  // JsonObject& temp = data.createNestedObject("temp");
  // JsonObject& shock = data.createNestedObject("shock");
  // JsonObject data = root.createNestedObject("data");
  JsonObject& staff1 = data.createNestedObject("staff1");
  JsonObject& store = data.createNestedObject("store");
  char printf_buf[10];  // 用于存储格式化后的字符串
  char total_data[10];  // 用于存储格式化后的数据STOR

  sprintf(printf_buf, "%d", 0);
  staff1["temp"] = printf_buf;
  staff1["shock"] = printf_buf;
  store["temp"] = printf_buf;
  store["hum"] = printf_buf;

  // 检查JsonData的前两个字节是否为0x55和0xAA
  if (JsonData[0] == 0x55 && JsonData[1] == 0xAA) {
    // 处理温度数据
    sprintf(total_data, "%d", JsonData[2]);
    staff1["temp"] = total_data;
    memset(total_data, 0, sizeof(total_data));
    sprintf(total_data, "%d", JsonData[3]);
    staff1["shock"] = total_data;
    memset(total_data, 0, sizeof(total_data));
    sprintf(total_data, "%d", JsonData[4]);
    store["temp"] = total_data;
    memset(total_data, 0, sizeof(total_data));
    sprintf(total_data, "%d", JsonData[5]);
    store["hum"] = total_data;
    memset(total_data, 0, sizeof(total_data));
    // 将第一个JSON对象转换为字符串
    root.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    // 将JSON消息发布到MQTT主题
    client.publish(Iot_link_MQTT_Topic_Report, JSONmessageBuffer);
  }

  // 准备网络缓冲区并通过串口发送确认信息
  unsigned char network_buffer[6] = {0x55, 0xAA, 0x03, 0x00, 0x00, 0xBB};
  Serial.write(network_buffer, sizeof(network_buffer));

  // LED指示
  digitalWrite(2, LOW);  // 关闭LED
  delay(100);  // 延时100毫秒
  digitalWrite(2, HIGH);  // 打开LED
}

