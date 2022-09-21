// Pratik Mathur
// ENEE 699 Fall 2012 w/ Dr. G.L. Blankenship
#include <Servo.h>
 
Servo myservo;  // create servo object to control a servo
 
int potpin = 0;  // analog pin used to connect the potentiometer
int val;    // variable to read the value from the analog pin


void setup()
{
  Serial.begin(9600);
  Serial.println("Hello Pi");
  Serial.println("attaching to pin 9");
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
}
 
void loop()
{ 
  if (Serial.available())
  {
    char command = Serial.read();
    if (command=='L'){
     
      turn_left();
      delay(5000);
    }
    else if (command=='R'){
      turn_right();
      delay(5000);
    }
    
  }
}

void turn_left(){
  myservo.write(500);
}

void turn_right(){
  myservo.write(0);    
}

