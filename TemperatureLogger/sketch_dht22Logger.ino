#include "DHT.h"

#define DHTPIN 7 // define pin where DHT22 is connected
#define DHTTYPE DHT22 // define DHT22 sensor (it could be DHT11)

DHT dht(DHTPIN, DHTTYPE);

#define N 600
float humidity[N];
float temperature[N];
int current = 0;

void acquireDHT();

void setup()
{
  // Start the Arduino hardware serial port at 9600 baud
  Serial1.begin(9600);
  // Start DHT22 communication
  
  dht.begin();
}

void loop() {
    // This sketch displays information every time a new sentence is correctly encoded.
    acquireDHT();

    if (Serial1.available() > 0) {
        while (Serial1.read() != -1) {
          ;
        }  
        sendAll();    
    }

    if (current == N-1) {
        current = 0;
    } else {
    
    }

    delay(6000);
}

void acquireDHT()
{
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (isnan(h) || isnan(t)) { 
        return;
    } else {
        humidity[current] = h;
        temperature[current] = t;
        current++;
    }    
}

void sendAll() {
  int j = 0;
  for (j = 0 ; j < current; j++) {
    Serial1.print(F("Humidity: ")); Serial1.print(humidity[j]); Serial1.print(F("% "));
    Serial1.print(F("Temperature: ")); Serial1.print(temperature[j]); Serial1.println(F(" °C ")); 
  }
}
