/*Arduino在4位接法下不使用LiquidCrystal库
* 输出欢迎界面和Hello World！
* 作者：大大维
* 2016/10/23
*/

int RS = 12;                             //数据/命令选择器引脚(RS)    
int RW = 11;                            //读/写选择器引脚(R/W)
int DB[] = {6, 7, 8, 9};                //使用数组来定义总线需要的管脚(D0-D7)
int Enable = 10;                        //使能(E)    

void LcdCommandWrite(int value) {
    // 定义所有引脚
    int i = 0;
 //   digitalWrite(Enable,LOW);
    delayMicroseconds(1);        // 延时1us
    digitalWrite(RS, LOW);
    digitalWrite(RW, LOW);
    
    /* 6     7     8     9
    *  D7     D6      D5    D4 
    *  DB[0] DB[1] DB[2] DB[3]
    */
    int temp=value & 0xf0;//取指令高4位
    for (i=DB[0]; i <= 9; i++)
    {
        if((temp & 0x80)!=0)//取指令最高位
            digitalWrite(i,HIGH);
        else
            digitalWrite(i,LOW );    
        temp <<= 1;
    }
    delayMicroseconds(1);          // 延时1us
    digitalWrite(Enable,LOW);
    delayMicroseconds(1);        // 延时1us
    digitalWrite(Enable,HIGH);
    delayMicroseconds(1);          // 延时1us
    digitalWrite(Enable,LOW);
    
    temp=(value & 0x0f)<<4;
    for (i=DB[0]; i <= 9; i++)
    {
        if((temp & 0x80)!=0)//取指令最高位
            digitalWrite(i,HIGH);
        else
            digitalWrite(i,LOW );    
        temp <<= 1;
    }
    delayMicroseconds(1);          // 延时1us
    digitalWrite(Enable,LOW);    
    delayMicroseconds(1);          // 延时1us
    digitalWrite(Enable,HIGH);
    delayMicroseconds(1);          // 延时1us
    digitalWrite(Enable,LOW);
    delayMicroseconds(1);          // 延时1us
}

void LcdDataWrite(int value) {
    // 定义所有引脚
    int i = 0;
 //   digitalWrite(Enable,LOW);
    delayMicroseconds(1);        // 延时1us
    digitalWrite(RS, HIGH);
    digitalWrite(RW, LOW);
    int temp=value& 0xf0;
    for (i=DB[0]; i <= 9; i++)
    {
        if((temp & 0x80)!=0)//取指令最高位
            digitalWrite(i,HIGH);
        else
            digitalWrite(i,LOW );    
        temp <<= 1;
    }
    delayMicroseconds(1);          // 延时1us
    digitalWrite(Enable,LOW);    
    delayMicroseconds(1);          // 延时1us
    digitalWrite(Enable,HIGH);
    delayMicroseconds(1);          // 延时1us
    digitalWrite(Enable,LOW);
    delayMicroseconds(1);   

    temp=(value & 0x0f)<<4;
    for (i=DB[0]; i <= 9; i++)
    {
        if((temp & 0x80)!=0)//取指令最高位
            digitalWrite(i,HIGH);
        else
            digitalWrite(i,LOW );    
        temp <<= 1;
    }
    delayMicroseconds(1);          // 延时1us
    digitalWrite(Enable,LOW);    
    delayMicroseconds(1);          // 延时1us
    digitalWrite(Enable,HIGH);
    delayMicroseconds(1);          // 延时1us
    digitalWrite(Enable,LOW);
    delayMicroseconds(1);   
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
    LcdCommandWrite(0x06);      // 输入方式设定，自动增量，画面没有移位
    delay(20);                      
    LcdCommandWrite(0x0C);      // 显示设置，开启显示屏，光标关，无闪烁
    delay(20);                                  
}

void loop (void) {
 /* LcdCommandWrite(0x01);          // 屏幕清空，光标位置归零  
  delay(10); 
  LcdCommandWrite(0x80+5);         // 定义光标位置为第一行第五个位置
  delay(10);                     
  // 写入欢迎信息 
  LcdDataWrite('W');
  LcdDataWrite('e');
  LcdDataWrite('l');
  LcdDataWrite('c');
  LcdDataWrite('o');
  LcdDataWrite('m');
  LcdDataWrite('e');
  delay(10);
  LcdCommandWrite(0xc0+2);          // 定义光标位置为第二行第二个位置  
  delay(10); 
  LcdDataWrite('I');
  LcdDataWrite(' ');
  LcdDataWrite('a');
  LcdDataWrite('m');
  LcdDataWrite(' ');
  LcdDataWrite('D');
  LcdDataWrite('a');
  LcdDataWrite('D');
  LcdDataWrite('a');
  LcdDataWrite('W');
  LcdDataWrite('e');
  LcdDataWrite('i');
  delay(3000);
  LcdCommandWrite(0x01);         // 屏幕清空，光标位置归零
  delay(10);
  LcdCommandWrite(0x80+2);         //定义光标位置为第一行第二个位置
  delay(10);  
  LcdDataWrite('H');
  LcdDataWrite('e');
  LcdDataWrite('l');
  LcdDataWrite('l');
  LcdDataWrite('o');
  LcdDataWrite(' ');
  LcdDataWrite('W');
  LcdDataWrite('o');
  LcdDataWrite('r');
  LcdDataWrite('l');
  LcdDataWrite('d');
  LcdDataWrite('!');
  LcdCommandWrite(0xc0);          // 定义光标位置为第二行起始位置  
  delay(10);
  LcdDataWrite('4');
  LcdDataWrite('-');
  LcdDataWrite('B');
  LcdDataWrite('i');
  LcdDataWrite('t');
  LcdDataWrite('-');
  LcdDataWrite('I');
  LcdDataWrite('n');
  LcdDataWrite('t');
  LcdDataWrite('e');
  LcdDataWrite('r');
  LcdDataWrite('f');
  LcdDataWrite('a');
  LcdDataWrite('c');
  LcdDataWrite('e');
  delay(10); 
  delay(5000);
  */
}
