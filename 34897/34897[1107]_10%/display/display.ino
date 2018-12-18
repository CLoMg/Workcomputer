int RS = 12;                             //数据/命令选择器引脚(RS)    
int RW = 11;                            //读/写选择器引脚(R/W)
int DB[] = {3, 4, 5, 6, 7, 8, 9, 10};    //使用数组来定义总线需要的管脚(D0-D7)
int Enable = 2;                            //使能(E)    

void LcdCommandWrite(int value) {
    // 定义所有引脚
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

void LcdDataWrite(int value) {
    // 定义所有引脚
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
void Display(String value){
      for(int i=0;i<value.length();i++){
   LcdDataWrite(value[i]);
   
      }
   
}

void setup (void) {
    int i = 0;
    for (i=Enable; i <= RS; i++) {
       pinMode(i,OUTPUT);   
    }
    delay(100);
    // 短暂的停顿后初始化LCD
    // 用于LCD控制需要
    LcdCommandWrite(0x38);         // 设置为8-bit接口，2行显示，5x7文字大小                     
    delay(20); 
    LcdCommandWrite(0x06);     // 输入方式设定，自动增量，画面没有移位
    delay(20);  
     LcdCommandWrite(0x0C);    // 显示设置，开启显示屏，光标关，无闪烁
    delay(20); 
    LcdCommandWrite(0x01);    // 屏幕清空，光标位置归零  
    delay(10); 
    LcdCommandWrite(0x80);   // 定义光标位置为第一行第五个位置
    delay(10);                 
}

void loop (void) {

   Display("Temp:");
   LcdCommandWrite(0x88);     //定义光标位置为第一行第二个位置
   delay(10); 
   Display("Hum:");
   LcdCommandWrite(0xC0);     //定义光标位置为第一行第二个位置
   delay(10); 
   Display("Lux:");
   LcdCommandWrite(0xC8);     //定义光标位置为第一行第二个位置
   delay(10); 
   Display("Speed:");
   while(1);
   
   
           
}
