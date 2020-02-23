/*
 * A temperature and humidity logger that can store values every 6 seconds
 * and send them to a secondary serial port to be retrieved.
 * 
 * It makes use of the DHT22 sensor, a digital sensor.
 * Whenever any character is sensed on Serial1 (connected on the Mega 
 * on 19(RX) and 18(TX), and operates at 9600 bps), it will simply send 
 * all of the current data to that port.
 * 
 */

#include "DHT.h"

#define computer Serial
#define bridge Serial1

#define DHTPIN 7 // define pin where DHT22 is connected
#define DHTTYPE DHT22 // define DHT22 sensor (it could be DHT11)

#define N 600
float humidity[N];
float temperature[N];
int current = 0;

DHT dht(DHTPIN, DHTTYPE); // initialize the sensor, global variable

void setup()
{
  Serial1.begin(9600);
  dht.begin();
}

void loop() {
  acquireDHT();

  if (bridge.available()) {
    int c;
    while ( (c = bridge.read()) != -1 ) {
       ;
    } 

    sendAll();    
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
    if (current == N) {
      current = 0;
    }
}

void sendAll() {
  int j = 0;
  for (j = 0 ; j < current; j++) {
    bridge.print(F("Humidity: ")); bridge.print(humidity[j]); bridge.print(F("% "));
    bridge.print(F("Temperature: ")); bridge.print(temperature[j]); bridge.println(F(" °C ")); 
  }
}
