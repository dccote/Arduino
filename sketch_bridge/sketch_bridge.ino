void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  while ( Serial1.available() ) {
       int c = Serial1.read();
       if (c != -1) {
         Serial.write(c);
       } else {
         Serial.print("No data read");
       }
  } 
  
  delay(1000);
  Serial.println("Requesting data");
  Serial1.write(1);    
}
