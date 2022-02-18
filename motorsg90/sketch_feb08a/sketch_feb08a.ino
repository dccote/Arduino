#include <Servo.h>

Servo servo;
int angle = 10;

void setup() {
  servo.attach(2);
  servo.write(angle);
}


void loop() 
{ 
  int duration = 2;
 // scan from 0 to 180 degrees
  for(angle = 10; angle < 180; angle++)  
  {                                  
    servo.write(angle);               
    delay(duration);                   
  } 
  // now scan back from 180 to 0 degrees
  for(angle = 180; angle > 10; angle--)    
  {                                
    servo.write(angle);           
    delay(duration);       
  } 
}
