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
