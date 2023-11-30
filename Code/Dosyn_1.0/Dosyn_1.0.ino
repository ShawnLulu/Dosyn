#include "Adafruit_FreeTouch.h"
#include "Adafruit_APDS9960.h"

//create the APDS9960 object
Adafruit_APDS9960 apds;
/////////////////////////////////////////////////////////////////////////////
//引脚分布↓
#define INT_PIN A3
#define LED_PWM_Pin A2
Adafruit_FreeTouch qt_1 = Adafruit_FreeTouch(A1, OVERSAMPLE_4, RESISTOR_50K, FREQ_MODE_NONE);//power 
Adafruit_FreeTouch qt_2 = Adafruit_FreeTouch(A7, OVERSAMPLE_4, RESISTOR_50K, FREQ_M ODE_NONE);//+
Adafruit_FreeTouch qt_3 = Adafruit_FreeTouch(A6, OVERSAMPLE_4, RESISTOR_50K, FREQ_MODE_NONE);//-

int qt_Threshold = 850;//QT灵敏度
int Proximity_Threshold = 100;//接近灵敏度
int counter = 0;
int counter1=0;
int Brightness = 0;//10%亮度为最低 brightness为目标亮度
int step = 10;
int Proximity_num = 0;
bool power = false;
bool fading = false;
bool closing = false;
bool Proximity_Short = false;
bool Proximity_Long  = false;
uint16_t r, g, b, c;


int cycle=0;
float distance,increment;
int GapTime,ADD,target;

void fadeBetween(void);////计算
/////////////////////////////////////////////////////////////////////////////
void fadeBetween(int start,int end,int direction){
  
  Serial.println("fading------------------");
  distance = abs(start - end);
  increment = distance/step;
  Serial.print("distance ");
  Serial.println(distance);
  Serial.print("increment ");
  Serial.println(increment);
  Serial.print("start ");
  Serial.println(start);
  Serial.print("end ");
  Serial.println(end);
  Serial.print("direction ");
  Serial.println(direction);

  cycle = start;
  GapTime  = 30/increment;             //increment越高 打入淡出越快 总时间不变
  Serial.print("cycle ");
  Serial.println(cycle);
  Serial.print("GapTime ");
  Serial.println(GapTime);
  
  ADD    = direction;
  target = end;
  fading = true;
}
/////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  //while (!Serial);
  //Serial.println("Ready");
  pinMode(INT_PIN, INPUT_PULLUP);
  pinMode(LED_PWM_Pin, OUTPUT);
  Brightness = 10;
/////////////////////////////////////////////////////////////////////////////
  if(!apds.begin()){
    Serial.println("failed to initialize device! Please check your wiring.");
  }
  else Serial.println("Device initialized!");
  //enable proximity mode
  apds.enableProximity(true);
  //set the interrupt threshold to fire when proximity reading goes above Proximity_Threshold
  apds.setProximityInterruptThreshold(0,Proximity_Threshold);
  //enable the proximity interrupt
  apds.enableProximityInterrupt();
  //enable color sensign mode
  apds.enableColor(true);

  apds.clearInterrupt();
/////////////////////////////////////////////////////////////////////////////
  if (! qt_1.begin())  
    Serial.println("Failed to begin qt on pin A0");
  if (! qt_2.begin())  
    Serial.println("Failed to begin qt on pin A7");
  if (! qt_3.begin())  
    Serial.println("Failed to begin qt on pin A2");
}


////////////////////////////////////////////////////////////////////////////
void loop() {
////接近感应
  //When the interrupt pin goes low
  if(!digitalRead(INT_PIN)){
    Serial.println(apds.readProximity());
    closing = true;
    Serial.println("closing");
    Serial.println(closing);
    Proximity_num ++;
    Serial.println("Proximity_num");
    Serial.println(Proximity_num);

    if(apds.readProximity()<Proximity_Threshold){
      if(30<Proximity_num&&Proximity_num<270){
        Serial.println("短按");
        Proximity_Short = true;
      }else if(280<Proximity_num&&Proximity_num<670){
        Serial.println("长按");
        Proximity_Long = true;                    
        apds.getColorData(&r, &g, &b, &c);
        Serial.print("red: ");
        Serial.print(r);
        Serial.print(" green: ");
        Serial.print(g);
        Serial.print(" blue: ");
        Serial.print(b);
        Serial.print(" clear: ");
        Serial.println(c);
        Serial.println();
        
        Serial.print(" Temperature: ");
        Serial.print(apds.calculateColorTemperature(r,g,b));
        Serial.print(" Lux: ");
        Serial.println(apds.calculateLux(r,g,b));
        Serial.println();        
      }
      closing = false;
      Serial.println("closing");
      Serial.println(closing);
      Proximity_num = 0;
      Serial.println("Proximity_num");
      Serial.println(Proximity_num);
      apds.clearInterrupt();
    }
  }
////触摸检测----------------------------------------------
  int qt1 = 0;
  int qt2 = 0;
  int qt3 = 0; 
  qt1 = qt_1.measure(); 
  qt2 = qt_2.measure();
  qt3 = qt_3.measure();
////保持cycle亮度
  if(!fading){analogWrite(A2,cycle/10*25);}
////淡入淡出
  if(fading){
    while(cycle != target){
    if((millis()-counter1)>GapTime){
      counter1 = millis();
      cycle = cycle + ADD;
      analogWrite(A2, int(cycle/10*25));
      Serial.print("CN ");
      Serial.println(cycle);
      }
    }
    if((cycle)==(target))
    {
    fading = false;
    Serial.println("faded------------------");
    }
  }
//////////////////////////////////////////////////////
///不在fading 且按键时 可以下一次操作 
  else if((millis()-counter)>300){           
    counter = millis();
////OFFline-------------------------------------------
////开灯 未经reset 仍保持熄灯前Brightness---------------
    if(!power){ 
      if(qt1>qt_Threshold||Proximity_Short){
        power = true;
        Proximity_Short = false;
        fadeBetween(0, Brightness, 1);
        Serial.print("QT 1: POWER "); 
        Serial.println(power);
        Serial.println(apds.readProximity());
        apds.clearInterrupt();
      }
    }else{
////Online--------------------------------------------
////关机键--------------------------------------------
      if(qt1>qt_Threshold||Proximity_Short){
        power = false;
        Proximity_Short = false;
        fadeBetween(Brightness, 0, -1); 
        Serial.println("QT 1: POWER "); 
        Serial.println(power);
        Serial.println(apds.readProximity());
        apds.clearInterrupt();
        delay(300);
      }
////加键----------------------------------------------
      else if((qt2>qt_Threshold)&&Brightness != 100){
        fadeBetween(Brightness, (Brightness + step), 1);
        Brightness = Brightness + step;
        Serial.print("QT 2: INCREASE   Brighiness "); 
        Serial.println(Brightness);
      }
////减键----------------------------------------------
      else if((qt3>qt_Threshold)&&Brightness != step){
        fadeBetween(Brightness, (Brightness - step), -1);
        Brightness = Brightness - step;
        Serial.print("QT 3: DECREASE    Brighiness "); 
        Serial.println(Brightness);
      }
    }
  }
}