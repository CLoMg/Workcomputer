#include <inttypes.h>

#define StartButton 18 //电机启停按键，接IO18
#define speedup 19     //加速按键，接IO19
#define speeddown 20   //减速按键，接IO20
bool motorstate= 0;    //初始化电机为关机状态
int pwmpin=10;        //定义电机pwm输出为IO10
int speed=127;        //初始化占空比为127/255

//LCD配置
int RS = 12;                             //数据/命令选择器引脚(RS)    
int RW = 11;                            //读/写选择器引脚(R/W)
int DB[] = {3, 4, 5, 6, 7, 8, 9, 44};    //使用数组来定义总线需要的管脚(D0-D7)
int Enable = 2;                            //使能(E)    

//DHT22配置 （DHT22采集温度和湿度数据）
unsigned int DHT_PIN = 22; //DHT22  Data管脚arduino IO15

#define DHT_OK      1
#define DHT_ERR_CHECK 0
#define DHT_ERR_TIMEOUT -1

String humidity;
String temperature;

//光照传感器配置（因为Proteus没有光照传感器，用光敏电阻代替）
float lux;
float a=-0.00001098;
float b=2.885;
int res=10000;

//DHT22 温度和湿度读取函数，输出为转换成字符串类型的温度和湿度数据
unsigned char DHT_read()
{
  // 接收数据Buffer
  unsigned char bits[5] = {0,0,0,0,0};
  unsigned char cnt = 7;
  unsigned char idx = 0;
  unsigned char sum;


  // 数据请求
  pinMode(DHT_PIN, OUTPUT);
  digitalWrite(DHT_PIN, LOW);
  delay(18);
  digitalWrite(DHT_PIN, HIGH);
  delayMicroseconds(40);
  pinMode(DHT_PIN, INPUT);

  // 数据应答或者超时判断
  unsigned int count = 10000;
  while(digitalRead(DHT_PIN) == LOW)
    if (count-- == 0) return DHT_ERR_TIMEOUT;

  count = 10000;
  while(digitalRead(DHT_PIN) == HIGH)
    if (count-- == 0) return DHT_ERR_TIMEOUT;

  // 读取输出 - 40 BITS => 5 BYTES or TIMEOUT
  for (int i=0; i<40; i++)
  {
    count = 10000;
    while(digitalRead(DHT_PIN) == LOW)
      if (count-- == 0) return DHT_ERR_TIMEOUT;

    unsigned long t = micros();

    count = 10000;
    while(digitalRead(DHT_PIN) == HIGH)
      if (count-- == 0) return DHT_ERR_TIMEOUT;

    if ((micros() - t) > 40) bits[idx] |= (1 << cnt);
    if (cnt == 0)   // 判断是否进入下一个字节？
    {
      cnt = 7;    // 从MSB开始写
      idx++;      // 下一位数据
    }
    else cnt--;
  }

  //计算数据校验和是否正确
  sum = bits[0]+bits[1]+bits[2]+bits[3];
  if(bits[4] != sum) return DHT_ERR_CHECK;
    
 //温度和湿度数据转换为字符串类型
  humidity = String(float((bits[0] << 8)+bits[1])/10,1);
  temperature = String(float((bits[2] << 8)+bits[3])/10,1);
  
  return DHT_OK;
}

//光敏电阻读取很熟，标定不是很准，演示用
String Lux_read(){
   lux=float(analogRead(14))/1023*5;
    lux=lux/(5-lux)*res;
    lux=lux*a+b;
   return String(lux,1);
}

//LCD写指令函数
void LcdCommandWrite(int value) {
    int i = 0;
    digitalWrite(RS, LOW);
    digitalWrite(RW, LOW);
    for (i=0; i <= 7; i++) //总线赋值
    {
        //这里的&是二进制的“与”运算。取指令码的最低位。
       digitalWrite(DB[i],value & 0x01);
       value >>= 1;
    }
    digitalWrite(Enable,LOW);
    delayMicroseconds(1);        // 延时1us
    digitalWrite(Enable,HIGH);
    delayMicroseconds(1);          // 延时1us
    digitalWrite(Enable,LOW);
    delayMicroseconds(1);          // 延时1us
}

//LCD写数据函数
void LcdDataWrite(int value) {
    int i = 0;
    digitalWrite(RS, HIGH);
    digitalWrite(RW, LOW);
    for (i=0; i <= 7; i++) {
       digitalWrite(DB[i],value & 0x01);
       value >>= 1;
    }
    digitalWrite(Enable,LOW);
    delayMicroseconds(1);        // 延时1us
    digitalWrite(Enable,HIGH);
    delayMicroseconds(1);        // 延时1us
    digitalWrite(Enable,LOW);
    delayMicroseconds(1);          // 延时1us
}

//LCD显示函数，输入为字符串类型
void Display(String value){
      for(int i=0;i<value.length();i++){
   LcdDataWrite(value[i]);   
      }
}
//LCD显示函数，输入为字符串类型
void LCDInit(){
      LcdCommandWrite(0x38);         // 设置为8-bit接口，2行显示，5x7文字大小                     
    delay(20); 
    LcdCommandWrite(0x06);     // 输入方式设定，自动增量，画面没有移位
    delay(20);  
     LcdCommandWrite(0x0C);    // 显示设置，开启显示屏，光标关，无闪烁
    delay(20); 
    LcdCommandWrite(0x01);    // 屏幕清空，光标位置归零  
    delay(10); 
    LcdCommandWrite(0x80);   // 定义光标位置为第一行第1个位置
    delay(10);  
    Display("T:");
   LcdCommandWrite(0x88);     //定义光标位置为第一行第9个位置
   delay(10); 
   Display("H:");
   LcdCommandWrite(0x8E);     //定义光标位置为第一行第15个位置 
   delay(10); 
   Display("%");
   LcdCommandWrite(0xC0);     //定义光标位置为第二行第1个位置
   delay(10); 
   Display("L:");
   LcdCommandWrite(0xC8);     //定义光标位置为第二行第9个位置
   delay(10); 
   Display("S:");    
}


void start_stop(){
   if ( digitalRead(StartButton) == LOW )
     motorstate=!motorstate;
     if(motorstate==1)
     analogWrite(pwmpin,speed);
     else
     analogWrite(pwmpin,0);
}
void add_speed(){
   if ( digitalRead(speedup) == LOW ){
   speed+=10;
   if(speed>255)
   speed=255;
   analogWrite(pwmpin,speed);
   }
}
void sub_speed(){
   if ( digitalRead(speeddown) == LOW ){
     speed-=10;
     if(speed<0)
     speed=0;
    analogWrite(pwmpin,speed); 
    }
}

void setup (void) {
    //初始化LCB数据引脚
    for (int i=0; i <= 7; i++) {
       pinMode(DB[i],OUTPUT);   
    }
    pinMode(Enable,OUTPUT);//初始化IO2为输出管脚，与LCD Enable相连
    pinMode(RW,OUTPUT);//初始化IO11为输出管脚，与LCD RW相连
    pinMode(RS,OUTPUT);//初始化IO12为输出管脚，与LCD RS相连
     
    pinMode(pwmpin,OUTPUT);//设置初始化PWM为输出管脚,控制电机
    analogWrite(pwmpin,0);
    pinMode(DHT_PIN,INPUT);//设置初始化IO15为输出管脚 与DHT22 DATA引脚相连
    digitalWrite(DHT_PIN, HIGH);//DHT22 DATA引脚初始化为高电平
    pinMode( StartButton, INPUT);//设置管脚为输入      //Enable中断管脚, 中断服务程序为onChange(), 监视引脚变化  
    attachInterrupt( digitalPinToInterrupt(StartButton), start_stop, CHANGE);
    pinMode(speedup, INPUT);//设置管脚为输入      //Enable中断管脚, 中断服务程序为onChange(), 监视引脚变化  
    attachInterrupt( digitalPinToInterrupt(speedup), add_speed, CHANGE);
    pinMode(speeddown, INPUT);//设置管脚为输入      //Enable中断管脚, 中断服务程序为onChange(), 监视引脚变化  
    attachInterrupt( digitalPinToInterrupt(speeddown), sub_speed, CHANGE);

    delay(100);
    // 短暂的停顿后初始化LCD
    // 用于LCD控制需要
     LCDInit();
}

void loop (void) {

   DHT_read();                 //读取DTH22的温度和湿度数据
   LcdCommandWrite(0x82);     //定义光标位置为第一行第3个位置
   delay(10); 
   Display(temperature);      //显示温度数据
   LcdCommandWrite(0x8A);     //定义光标位置为第一行第11个位置
   delay(10); 
   Display(humidity);         //显示湿度数据
   LcdCommandWrite(0xC2);     //定义光标位置为第二行第3个位置 
   delay(10); 
   Display(Lux_read());       //显示光照强度数据         
}
