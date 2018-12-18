#include <inttypes.h>


#define startcircle 23
#define addspeed 24
#define subspeed 25
bool starstop= LOW;
#define pwm A0 //定义电机pwm输出为IO16
int speed=127;
//PID配置
struct _pid{
    float SetSpeed;            //定义设定值
    float ActualSpeed;        //定义实际值
    float err;                //定义偏差值
    float err_next;            //定义上一个偏差值
    float err_last;            //定义最上前的偏差值
    float Kp,Ki,Kd;            //定义比例、积分、微分系数
}pid;

//LCD配置
int RS = 12;                             //数据/命令选择器引脚(RS)    
int RW = 11;                            //读/写选择器引脚(R/W)
int DB[] = {3, 4, 5, 6, 7, 8, 9, 10};    //使用数组来定义总线需要的管脚(D0-D7)
int Enable = 2;                            //使能(E)    

//DHT22配置 （DHT22采集温度和湿度数据）
unsigned int DHT_PIN = 15; //DHT22  Data管脚arduino IO15

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
//pid初始化函数
void PID_init(){
    pid.SetSpeed=0.0;
    pid.ActualSpeed=0.0;
    pid.err=0.0;
    pid.err_last=0.0;
    pid.err_next=0.0;
    pid.Kp=0.2;
    pid.Ki=0.015;
    pid.Kd=0.2;
}
//pid控制函数
float PID_realize(float speed){
    pid.SetSpeed=speed;
    pid.err=pid.SetSpeed-pid.ActualSpeed;
    float incrementSpeed=pid.Kp*(pid.err-pid.err_next)+pid.Ki*pid.err+pid.Kd*(pid.err-2*pid.err_next+pid.err_last);
    pid.ActualSpeed+=incrementSpeed;
    pid.err_last=pid.err_next;
    pid.err_next=pid.err;
    return pid.ActualSpeed;
}

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
    for (i=DB[0]; i <= RS; i++) //总线赋值
    {
        //这里的&是二进制的“与”运算。取指令码的最低位。
       digitalWrite(i,value & 0x01);
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
    for (i=DB[0]; i <= DB[7]; i++) {
       digitalWrite(i,value & 0x01);
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
   if ( digitalRead(starstop) == LOW )
     starstop=!starstop;
     analogWrite(pwm,int(starstop)*255);
}
void add_speed(){
   speed++;
   if(speed>255)
   speed=255;
   analogWrite(pwm,speed);
}
void sub_speed(){
     speed--;
     if(speed<0)
     speed=0;
    analogWrite(pwm,speed);
}

void setup (void) {
    int i = 0;
    pinMode(pwm,OUTPUT);//设置初始化IO15为输出管脚 与DHT22 DATA引脚相连
    analogWrite(pwm,127);
    pinMode(DHT_PIN,INPUT);//设置初始化IO15为输出管脚 与DHT22 DATA引脚相连
    digitalWrite(DHT_PIN, HIGH);//DHT22 DATA引脚初始化为高电平
    pinMode( startcircle, INPUT);//设置管脚为输入      //Enable中断管脚, 中断服务程序为onChange(), 监视引脚变化  
    attachInterrupt( digitalPinToInterrupt(startcircle), start_stop, CHANGE);
    pinMode( addspeed, INPUT);//设置管脚为输入      //Enable中断管脚, 中断服务程序为onChange(), 监视引脚变化  
    attachInterrupt( digitalPinToInterrupt(addspeed), add_speed, CHANGE);
    pinMode( subspeed, INPUT);//设置管脚为输入      //Enable中断管脚, 中断服务程序为onChange(), 监视引脚变化  
    attachInterrupt( digitalPinToInterrupt(subspeed), sub_speed, CHANGE);

    for (i=Enable; i <= RS; i++) {
       pinMode(i,OUTPUT);   
    }
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
//   analogWrite(pwm,value);
   
           
}
