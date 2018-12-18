#include <inttypes.h>

#define speedin 18
#define StartButton 2
#define speedup 19
#define speeddown 20
#define Modeselct 21
unsigned long time = 0, old_time = 0; // 时间标记u
int Counter=0;      //脉冲信号数量
float SpeedMax=200;//设定最高转速
float SpeedMin=0;//设定最低转速
float TempMax=35; //电机转动温度上限
float TempMin=5; //电机转动温度下限
float LuxMax=1.5; //电机转动温度上限
float LuxMin=1; //电机转动温度上限
bool Automode=1; //自动/手动控制模式状态位，预设为自动控制模式
bool motorstate= 0; //电机状态标志
int pwmpin=10; //定义电机pwm输出为IO16
int Speed_default=127;  //转速默认值
float pwmvalue=0; //pwm输出变量，控制电机转速


//LCD配置
int RS = 12;                             //数据/命令选择器引脚(RS)    
int RW = 11;                            //读/写选择器引脚(R/W)
int DB[] = {3, 4, 5, 6, 7, 8, 9, 44};    //使用数组来定义总线需要的管脚(D0-D7)
int Enable = 45;                            //使能(E)    

//DHT22配置 （DHT22采集温度和湿度数据）
unsigned int DHT_PIN = 14; //DHT22  Data管脚arduino IO15

//DHT22状态标志
#define DHT_OK      1
#define DHT_ERR_CHECK 0
#define DHT_ERR_TIMEOUT -1

float humidity; //湿度变量
float temperature;//温度变量

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
  
  humidity = float((bits[0] << 8)+bits[1])/10;
  temperature = float((bits[2] << 8)+bits[3])/10;
  
  return DHT_OK;
}

//光敏电阻读取很熟，标定不是很准，演示用
void Lux_read(){
   lux=float(analogRead(A0))/1023*5; //读取输入电压

   //电压转换为光照强度
   lux=lux/(5-lux)*res;
   lux=lux*a+b;
   
  // return lux;//返回光照强度
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
   for(unsigned int i=0;i<value.length();i++){
        LcdDataWrite(value[i]); //显示字符  
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

//定义pid结构体
struct {
    float set_speed;//设定速度 
    float actual_speed;//实际速度
    float error;//偏差  
    float error_next;//上一个偏差  
    float error_last;//上上一个偏差 
    float kp,ki,kd;//定义比例，积分，微分参数   
}pid;
//手动模式电机启停
void start_stop(){
   if (Automode) return;//如果是自动控制状态，不执行后面操作，直接返回
   if ( digitalRead(StartButton) == LOW )//检测按键按下
     motorstate=!motorstate;//切换开关状态
   if(motorstate==1)
     //pwmvalue = defaultvalue;
     pid.set_speed=Speed_default;//开启电机，速度为默认值
   else
     //pwmvalue = 0;
     pid.set_speed=0;//关闭电机
  // analogWrite(pwmpin,pwmvalue);//输出pwm
}

//电机转速增加
void add_speed(){
   if (Automode) return; //如果是自动控制状态，不执行后面操作，直接返回
   if (motorstate==0)return;//如果是关机状态，不执行后面操作，直接返回
   if ( digitalRead(speedup) == LOW )//检测按键按下
   {
      //pwmvalue+=10;//pwm占空比增加10/255
       pid.set_speed+=10;//速度增加10RPM
      //如果超过最大值，就等于最大值
      //if(pwmvalue>255)
       // pwmvalue=255;
      if(pid.set_speed>SpeedMax)//如果超过最大值，就等于最大值
     pid.set_speed=SpeedMax;
      
      //pwm输出
      //analogWrite(pwmpin,pwmvalue);
   }
}

//电机转速减小
void sub_speed(){
   if (Automode) return;//如果是自动控制状态，不执行后面操作，直接返回
   if (motorstate==0)return;//如果是关机状态，不执行后面操作，直接返回
   if ( digitalRead(speeddown) == LOW )//检测按键按下
   {
     //pwmvalue-=10;//pwm占空比减小0/255
     pid.set_speed-=10;//速度减小10RPM
   //如果超过最小值，就等于最小值
     //if(pwmvalue<0)
     //pwmvalue=0;
     if(pid.set_speed<SpeedMin)//如果超过最小值，就等于最小值
     pid.set_speed=SpeedMin;
     
     //pwm输出
    //analogWrite(pwmpin,pwmvalue); 
    }
}

//切换模式
void selectmode(){
   if ( digitalRead(Modeselct) == LOW )//检测按键按下
   {
     Automode=!Automode; //切换自动和手动模式
     if(Automode==0) motorstate =1; //切换到手动模式后，开关状态设置为开启状态
    }
}



//初始化pid参数
void pid_init()
{
    pid.set_speed = 0.0;//设定转速
    pid.actual_speed = 0.0;//实际转速
    pid.error = 0.0;       //误差
    pid.error_next = 0.0;  //上一次误差
    pid.error_last = 0.0;  //上上次误差
    //可调节PID 参数。使跟踪曲线慢慢接近阶跃函数200.0 // 
    pid.kp = 0.1;         
    pid.ki = 0.05;
    pid.kd = 0.0;
}

//pid计算
float pid_realise()//实现pid  
{
    //pid.actual_speed=speed_actual; //读取实际湿度
    //pid.set_speed = speed_set;
    pid.error = pid.set_speed - pid.actual_speed;//实际湿度和设定湿度差值
    float increment_speed;//增量
     //pid计算
    increment_speed = pid.kp*(pid.error)+pid.ki*pid.error_next+\
    pid.kd*pid.error_last;//增量计算公式  
    
    //设定pwm输出值
    pwmvalue+=increment_speed;
    if (pwmvalue>255)  pwmvalue=255;
    else if (pwmvalue<0)  pwmvalue=0;
    else ;
    
   //下一次误差值迭代 
    pid.error_last = pid.error_next; 
    pid.error_next = pid.error;
    
    return  pwmvalue; //返回pwm输出值   
}
float SpeedScalc(float humi)//根据湿度计算转速
{
   float SpeedS=SpeedMax-(humi-10)*(SpeedMax-SpeedMin)/40; //湿度为10%时为最高转速，湿度为50%时为最低转速
   if (SpeedS>SpeedMax)//大于最该转速则取最高转速
      return SpeedMax;
   else if (SpeedS<SpeedMin)//小于最低转速则取最低转速
      return SpeedMin;
   else
      return SpeedS; //返回设定转速
}
void Count_CallBack()//中断函数，测量脉冲数量
{
  Counter++;
}

int SpeedDetection()//速度测量和调节函数
{
  time = millis();//以毫秒为单位，计算当前时间 
  if(abs(time - old_time) >= 500) // 如果计时时间已达1秒
  {  
    detachInterrupt(speedin); // 关闭外部中断0
    pid.actual_speed=(float)Counter*120/20;//根据脉冲数计算转速
     pid_realise();//pid调节
     analogWrite(pwmpin,pwmvalue);//pwm输出
    Counter = 0;   //把脉冲计数值清零，以便计算下一秒的脉冲计数
    old_time=  millis();     // 记录每秒测速时的时间节点   
    attachInterrupt(speedin, Count_CallBack,FALLING); // 重新开放外部中断0
    return 1;
  }
  else
    return 0;
}

void setup (void) {
    //初始化LCB数据引脚
    for (int i=0; i <= 7; i++) {
       pinMode(DB[i],OUTPUT);   
    }
    pinMode(A0,INPUT);//初始化A0引脚，读取光敏电阻电压值
    pinMode(Enable,OUTPUT);//初始化IO2为输出管脚，与LCD Enable相连
    pinMode(RW,OUTPUT);//初始化IO11为输出管脚，与LCD RW相连
    pinMode(RS,OUTPUT);//初始化IO12为输出管脚，与LCD RS相连
     
    pinMode(pwmpin,OUTPUT);//设置初始化PWM为输出管脚,控制电机
    analogWrite(pwmpin,0); //将电机初速设为0
    pinMode(DHT_PIN,INPUT);//设置初始化IO15为输出管脚 与DHT22 DATA引脚相连
    digitalWrite(DHT_PIN, HIGH);//DHT22 DATA引脚初始化为高电平
    pinMode( StartButton, INPUT);//设置管脚为输入      //StartButton中断管脚, 中断服务程序为start_stop(), 监视引脚变化  
    attachInterrupt( digitalPinToInterrupt(StartButton), start_stop, CHANGE);
    pinMode(speedup, INPUT);//设置管脚为输入      //speedup中断管脚, 中断服务程序为add_speed(), 监视引脚变化  
    attachInterrupt( digitalPinToInterrupt(speedup), add_speed, CHANGE);
    pinMode(speeddown, INPUT);//设置管脚为输入      //speeddown中断管脚, 中断服务程序为sub_speed(), 监视引脚变化  
    attachInterrupt( digitalPinToInterrupt(speeddown), sub_speed, CHANGE);
    pinMode(speeddown, INPUT);//设置管脚为输入      //speeddown中断管脚, 中断服务程序为sub_speed(), 监视引脚变化  
    attachInterrupt( digitalPinToInterrupt(Modeselct), selectmode, CHANGE);
    pinMode(speedin, INPUT);//设置管脚为输入      //speedin, 中断服务程序为Count_CallBack, 监视引脚变化 ，下降沿促发 
    attachInterrupt( digitalPinToInterrupt(speedin), Count_CallBack, FALLING);
    delay(100);
    // 短暂的停顿后初始化LCD
    // 用于LCD控制需要
     LCDInit();//初始化LCD
     pid_init();//初始化PID
}

void loop (void) {

   DHT_read();                 //读取DTH22的温度和湿度数据
   LcdCommandWrite(0x82);     //定义光标位置为第一行第3个位置 
   Display(String(temperature,1));      //显示温度数据
   LcdCommandWrite(0x8A);     //定义光标位置为第一行第11个位置
   Display(String(humidity,1));         //显示湿度数据
   LcdCommandWrite(0xC2);     //定义光标位置为第二行第3个位置 
   Lux_read();//读取光照强度
   Display(String(lux,1));   //显示光照强度数据
   //Display(String(pid.set_speed,1));
   LcdCommandWrite(0xC6);     //定义光标位置为第一行第11个位置
   if(Automode)//自动模式，计算pid，并调节输出。手动模式则跳过
   { 
      if((LuxMin<lux)&&(lux<LuxMax)&&(TempMin<temperature)&&(temperature<TempMax))//判断温度和光照强度是否超限
      {
   pid.set_speed = SpeedScalc(humidity);//根据湿度计算转速
  
      }
      else
   pid.set_speed = 0;//超限，转速设置为0
       Display("A"); //表示自动控制
   }
   else Display("M");//表示手动模式
   LcdCommandWrite(0xCA);     //定义光标位置为第一行第11个位置
   SpeedDetection(); //速度测量和调节函数
   Display(String(pid.actual_speed,1));//显示转速

           
}


