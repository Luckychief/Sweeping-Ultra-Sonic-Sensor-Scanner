#include<Servo.h>
//TFT SCREEN STUFF
#include <SPI.h>
#include <TFT_ST7735.h>
#include "fonts/fluide_caps.h"

#define __CS  10
#define __DC  8
#define __RST 9

TFT_ST7735 tft = TFT_ST7735(__CS, __DC, __RST);


//UltraSonic Stuff
#define trigPin 3
#define echoPin 5

#define MAX_DISTANCE 200
float distance = 0;
float timeOut = MAX_DISTANCE * 60;
int soundVelocity = 340;// m/s

//global Variable for Distance
float DISTANCE = 0;

//Servo Motor
Servo myservo;
unsigned int servoVal = 0;

unsigned int joystickX = A0; //note, becuase of the orientation of the joystick
unsigned int posX = 0; //we are measuring the Y-axis and all_left = 0 and all_right = 1023
int angleSum = 5;      //resting postion seems to be value 502
                            //<482 and >522 are going to be the trigger values
unsigned int i = 0;       
unsigned int j = 0;       //Gobal Counters
                          //j is being used for task number 1, which is moving the servo motor

typedef struct task{
  int state;
  unsigned long period;
  unsigned long elapsedTime;
  int(*TickFct)(int);
}task;

const unsigned short tasksNum = 3;
task tasks[tasksNum];

enum SM1_States{SM1_Start, SM1_S1, SM1_S2, SM1_S3} SM1_State;
int TickFct_SM1(int state) {
  switch(state) { //all transitions
    case SM1_Start:
    state = SM1_S1;
    break;

    case SM1_S1:
    if(analogRead(A0) > 600) {state = SM1_S2;}
    else if(analogRead(A0) < 400) {state = SM1_S2;}
    else {state = SM1_S1;}
    break;

    case SM1_S2:
    if(analogRead(A0) > 600) {state = SM1_S2;}
    else if(analogRead(A0) < 400) {state = SM1_S2;}
    else {state = SM1_S3;}
    break;

    case SM1_S3:
    if(j < 20){
      if(analogRead(A0) > 600) {state = SM1_S2, j = 0;}
      else if(analogRead(A0) < 400) {state = SM1_S2, j = 0;}
      else {state = SM1_S3;}
    }
    else{state = SM1_S1, j = 0;}
    break;

    default:
    break;
    
  }

  switch(state) {
    case SM1_S1:
    
    if (angleSum > 0) {
     if (posX >= 175) {
      posX = 180;
      angleSum = -5;
    }
     else { posX = posX + angleSum;}
    }

    else if (angleSum < 0){
      if (posX <= 5) {
      posX = 0;
      angleSum = 5;
    }
      else { posX = posX + angleSum;}
    }
     
    myservo.write(posX);
    break;

//    else{ posX = posX + angleSum;}
//
//    
//    if(posX == 0) {angleSum = 5, posX = posX + angleSum, myservo.write(posX);}
//    if(posX < 176){myservo.write(posX), posX = posX + angleSum;}
//    else {angleSum = -5, posX = posX + angleSum, myservo.write(posX);}
//    break;

    case SM1_S2:
//    moveServo();
    if(analogRead(A0) > 600) {
      if (posX >= 175) {posX = 180;}
      else {posX = posX + 5;}
    }
    else if(analogRead(A0) < 400) {
      if (posX <= 5) {posX = 0;}
      else {posX = posX - 5;}
    }

    myservo.write(posX);
    break;

    case SM1_S3:
    j = j + 1;
    break;
    
  }

  return state;
}


enum SM2_States{SM2_Start, SM2_S1} SM2_State; 

int TickFct_SM2(int state) {
  switch(state) { //transitions;
    case SM2_Start:
    state = SM2_S1;
    break;

    case SM2_S1:
    state = SM2_S1;
    break;

    default:
    break;
  }

  switch(state) { //state actions;
    case SM2_Start:
    DISTANCE = getSonar();
    break;

    case SM2_S1:
    DISTANCE = getSonar();
    break;

  }

  return state;
}



enum SM3_States{SM3_Start, SM3_S1} SM3_State; 

int TickFct_SM3(int state) {
  switch(state) { //transitions;
    case SM3_Start:
    state = SM3_S1;
    break;

    case SM3_S1:
    state = SM3_S1;
    break;

    default:
    break;
  }

  switch(state) { //state actions;
    case SM3_Start:
    break;

    case SM3_S1:
    tft.setFont(&internal);//this is the internal font, always loaded
    tft.clearScreen();
    
    tft.setCursor(1,40);
    tft.print("Distacne: ");
    tft.print(DISTANCE);
    tft.print("cm");
    
    tft.setCursor(1,60);
    tft.print("Angle: ");
    tft.print(posX);
    tft.print(" degrees");
    break;

  }

  return state;
}



void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(joystickX, INPUT);
  myservo.attach(6);
  Serial.begin(9600);
  tft.begin();

  i = 0;
  tasks[i].state = SM1_Start;
  tasks[i].period = 150; //Choose a more sensible time later
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &TickFct_SM1;
  i++;
  tasks[i].state = SM2_Start;
  tasks[i].period = 150; //Choose a more sensible time later
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &TickFct_SM2;
  i++;
  tasks[i].state = SM3_Start;
  tasks[i].period = 100; //Choose a more sensible time later
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &TickFct_SM3;
  
}

void message() {
//removed
}

//void moveServo(){
//  servoVal = analogRead(joystickX);
//  servoVal = map(servoVal, 0, 1023, 0, 180);
//  posX = servoVal; //updates heading to dislpay later.
//  myservo.write(servoVal);
//}


float getSonar() {
  unsigned long pingTime;

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  pingTime = pulseIn(echoPin, HIGH, timeOut);
  
  distance = (float)pingTime * soundVelocity / 2 / 10000;

  return distance;
}



void loop() {
  for(i = 0; i < tasksNum; i++) {
    if((millis() - tasks[i].elapsedTime) >= tasks[i].period) {
      tasks[i].state = tasks[i].TickFct(tasks[i].state);
      tasks[i].elapsedTime = millis();
    }
  }
  
  delay(50);
}
