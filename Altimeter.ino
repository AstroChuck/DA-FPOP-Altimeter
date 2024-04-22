#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <Servo.h>


/********************
Arduino Altimeter Project
Discover Aerospace FPOP 2016
Charlie Garcia MIT AeroAstro

This altimeter uses I2C to talk to a BMP180 sensor. It records the highest altitude. 
The sensor detects launch and landing. A piezo buzzer announces altitude via beep 
sequences. Optionally a servo can be driven by this device. The servo will travel
from 0 to 180, delay for 1s, and then return to 0.

Pinout:
Trinket
SDA: 0
SCL: 2
Piezo: 3
Piezo: 5
Servo: 9
BAT+ : Bat+
GND: Gnd, SGnd, PGnd
3v: Vcc, SVcc

*********************/



Adafruit_BMP085 bmp;
Servo servo;

int timeTone = 400; //duration of a tone in milliseconds
int waitTime = 400; //duration between tones in milliseconds
int freq = 1012;    //frequency of tone in Hz, a "B"
int alarmCount = 100; //duration of apogee or launch alarm

float altitude = 0; //Where is the vehicle now?
float maxAltitude = 0; //How high did it go.
float groundLevel = 0; //Default ground level

int launchAlarm = 0;
int apogeeAlarm = 0;
int landedAlarm = 0;

bool servoOpen = false;

String state = "prelaunch";

void beep (unsigned char speakerPin)
{   // http://web.media.mit.edu/~leah/LilyPad/07_sound_code.html
          int x;   
          long delayAmount = (long)(1000000/1200);
          long loopTime = (long)((400*1000)/(delayAmount*2));
          for (x=0;x<loopTime;x++)   
          {  
              digitalWrite(speakerPin,HIGH);
              delayMicroseconds(delayAmount);
              digitalWrite(speakerPin,LOW);
              delayMicroseconds(delayAmount);
          }  
}

void beepNum(int num) {
  if(num == 0){
    num = 10;
  }
  for(int i = 0; i < num; i++){
     beep(5);
     delay(waitTime);
  }
 
}

void splitNum(float number){
  int ones,tens,hundreds,thousands;
  number = (int)round(number);
  if(number > 999){
    thousands = (int)number/1000;
    number = number-(thousands*1000);
    beepNum(thousands);
    delay(1000);
  }
  if(number > 99){
    hundreds = (int)number/100;
    number = (int)number-(hundreds*100);
    beepNum(hundreds);
    delay(1000);
  }
    
  tens = (int)number/10;
  ones = (int)number-(tens*10);
  
  beepNum(tens);
  delay(1000);
  beepNum(ones);
  delay(1000);
}

bool heightCheck(){
  altitude = bmp.readAltitude();  //check altitude
  if (state == "prelaunch" && (altitude - groundLevel) > 8){
      launchAlarm ++;
      if (launchAlarm > alarmCount){
          state = "launch";
      }
  }
  if (state == "launch" && altitude > maxAltitude){
      maxAltitude = altitude;
      apogeeAlarm = 0;
  }
  if (state == "launch"){
      apogeeAlarm ++;
      if (apogeeAlarm > alarmCount){
          state = "falling";
          apogee();
      }
  }
  if (state == "falling" && (altitude - groundLevel) < 4){
      landedAlarm ++;
      if (landedAlarm > alarmCount){
          state = "landed";
          landed();
      }
  }
  if (state == "landed"){
    landed();
  }
  if (digitalRead(12)==LOW){
    servoOpen = !servoOpen;
    if (servoOpen){
      servo.write(80);
    }
    else{
      servo.write(255);
    }
  }
}

void apogee() {
  delay(1000);
  servo.write(0);
  digitalWrite(13, LOW);
}

void landed(){
  splitNum(maxAltitude);
  delay(3000);
}

void setup(){
  pinMode(13, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(12, INPUT_PULLUP);
  digitalWrite(3, LOW);                 //Pin the piezo is on must be low
  
  if(!bmp.begin()){    //Initalize pressure sensor, check functionality.
    beepNum(20);          //Tell someone if it isn't working
    while(1);
  }
  groundLevel = bmp.readAltitude();

  servo.attach(9);                      //Which pin drives the servo
  servo.write(255);                       //Make sure the servo is zero'd

  delay(50);

  digitalWrite(13, HIGH);               //Ready for flight
}

void loop() {
  heightCheck();

}
