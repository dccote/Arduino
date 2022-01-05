#include <FastLED.h>
#define NUM_LEDS 100
#define DATA_PIN 6

CRGB leds[NUM_LEDS];
uint8_t offset = 0;

void setup() {
  // put your setup code here, to run once:
  FastLED.addLeds<WS2812, DATA_PIN>(leds, NUM_LEDS);
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i=0; i < NUM_LEDS; i++) {
    CHSV color((((i-offset))*255)/NUM_LEDS,255,255);
    leds[(i)%NUM_LEDS] = color; 
  }
  FastLED.show();
  for (int i; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black; 
  }
  
  delay(40);
  offset += 1;
}
