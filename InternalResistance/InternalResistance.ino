/* 
 * Pins à connecter 
 */

#define RELAY_PIN D2
#define VOLTMETER A0

#define ANALOG_RES 12

unsigned long nextTime = 0;

#define N 10000
float closedVoltages[N];
float openVoltages[N];
int i = 0;

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(VOLTMETER, INPUT);
  analogReadResolution(ANALOG_RES);
  Serial.begin(9600);
  nextTime = millis();
}

void loop() {
  unsigned long currentTime = millis();
  int INTERVAL = 20*1000;
  if (nextTime < currentTime) {
    nextTime = currentTime + INTERVAL;

    float openVoltage = readOpenCircuitVoltage();
    float closedVoltage = readClosedCircuitVoltage();

    if ( i < N) {
      closedVoltages[i] = closedVoltage;
      openVoltages[i] = openVoltage;
      printSingleEntry(i);
      i++;
    } else {
      Serial.println("Out of memory cannot save data");
    }
  }

  if (Serial.available()) {
      String c = Serial.readString();
      if ( c[0] == 'd') { // dump
        for (int j = 0; j < i; j++) { // i has been incremented already
          printSingleEntry(j);
        }
      } else if ( c[0] == 'c') { // clear
        i = 0;
        Serial.println("Clearing values");
      }
  }
}

void printSingleEntry(int j) {
    Serial.print("Closed: ");
    Serial.print(closedVoltages[j],7);
    Serial.print("\tOpen: ");
    Serial.print(openVoltages[j],7);
    Serial.print("\n");  
}

float readOpenCircuitVoltage() {
  digitalWrite(RELAY_PIN, LOW);
  delay(1000);
  float voltage = analogRead(VOLTMETER);
  return (float(voltage)*3.3)/float(1<<ANALOG_RES);
}

float readClosedCircuitVoltage() {
  digitalWrite(RELAY_PIN, HIGH);  
  delay(1000);
  float voltage = analogRead(VOLTMETER);
  return (float(voltage)*3.3)/float(1<<ANALOG_RES);
}
