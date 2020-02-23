/*
 * This communicates with another Arduino device by serial connection
 * it must be connected on Serial1, which on the Mega is on pins
 * 19(RX) and 18(TX) and must operate at 9600 bps.
 * 
 * It will simply forward (or bridge) the characters
 * received on its main Serial port to the Serial1 port.
 * 
 * This is useful if another arduino is running on a USB powered battery
 * and we want to extract information from it without disconnecting it.
 */

#define computer Serial
#define logger Serial1

void setup() {
  computer.begin(9600);
  logger.begin(9600);
}

void loop() {
  int c;
  if (logger.available()) {
    while ( (c = logger.read()) != -1 ) {
       computer.write(c);
    } 
  } else if ( computer.available()) {  
    while ( (c = computer.read()) != -1 ) {
       logger.write(c);
    } 
  }
  delay(100);
}
