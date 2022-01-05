/*
  This is a simple sound monitor I made by reusing a lot of code from examples.

  With a Nano BLE Sense 33, we use the microphone, compute the spectrum and
  show it on an NeoPixel 16-LED circular element.
  
*/


#include <PDM.h>
#include "arduinoFFT.h"
#include <Arduino_APDS9960.h>
  
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define PIXEL_PIN   12  // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 100  // Number of NeoPixels

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, NEO_BRG + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

/* Microphone variables */
#define N_SAMPLES 256
short sampleBuffer[N_SAMPLES]; // buffer to read samples into, each sample is 16-bits
volatile int samplesRead;      // number of samples read
int gain = 20;

/* Audio spectrum variables */
arduinoFFT FFT = arduinoFFT(); /* Create FFT object */
/*
These are the input and output vectors, and also used for FFT
Input vectors receive computed results from FFT
*/
double vReal[N_SAMPLES];
double vImag[N_SAMPLES];


uint16_t gOffset = 0;

#define SPECTRUM_METER 0
#define NOISE_LEVEL 1
int mode = SPECTRUM_METER;

void binSoundHistogram(int8_t binSize = (N_SAMPLES/2/2));
void showSoundLevels(int mode = SPECTRUM_METER, int offset=0, int scale=65536/3, double maxValue = 5000);
void blink(int repetition = 1);


void setup() {
//  delay(1000);
  
  /* Feedback LED to avoid using serial port */
  pinMode(LED_BUILTIN, OUTPUT);
  /* Setup serial port */
  Serial.begin(9600);

  /* Setup neopixel strip */
  strip.begin(); // Initialize NeoPixel strip object (REQUIRED)
  strip.show();  // Initialize all pixels to 'off'

  /* Setup microphone */
  PDM.onReceive(onPDMdata); // callback
  PDM.setGain(gain);          
  PDM.setBufferSize(N_SAMPLES*sizeof(short));
  
  if (!PDM.begin(1, 16000)) { // mono at 16 kHz
    blink(2);      
    while (1);
  }

  /* Setup gesture detection */
  if (!APDS.begin()) {
    Serial.println("Error initializing APDS-9960 sensor!");
  }

  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, 0);         //  Set pixel's color (in RAM)    
  }
  strip.show();                          //  Update strip to match

}

void blink(int repetition) {
  while (repetition) {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(150);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(150); 
    repetition--;
  }
}

void loop() {
  // When we have samples, we manipulate the spectrum
  if (samplesRead) {
    computeSoundSpectrum();
//    binSoundHistogram(N_SAMPLES/8);
    showSoundLevels(mode, 0, 65536/PIXEL_COUNT, 10000);
  }

//  soundAlarm();

  if (APDS.gestureAvailable()) {
    // a gesture was detected, read and print to Serial Monitor
    Serial.println("Gesture");
    int gesture = APDS.readGesture();

    switch (gesture) {
      case GESTURE_UP:
        increaseGain();
        break;

      case GESTURE_DOWN:
        decreaseGain();
        break;

      case GESTURE_LEFT:
        mode++;
        mode %= 2;
        modeChange();
        break;
      case GESTURE_RIGHT:
        mode--;
        mode %= 2;
        modeChange();
        break;
      default:
        // ignore
        break;
    }
  }

}

void increaseGain() {
  if (gain >= 50) {
    gain = 50;
    soundUpperLimit();
  } else {
    gain += 5;          
    soundUp();
  }
  PDM.setGain(gain);          
}

void decreaseGain() {
  if (gain <= 5) {
    gain = 1;
    soundLowerLimit();
  } else {
    gain -= 5;
    soundDown();
  }
  PDM.setGain(gain);          
}

void soundUp() {
  tone(2, 500, 60);
  delay(30);
  tone(2, 1000, 60);
}

void soundDown() {
  tone(2, 1000, 60);
  delay(60);
  tone(2, 500, 60);
}

void modeChange() {
  tone(2, 2000, 60);
  delay(60);
  tone(2, 1000, 60);
}

void soundLowerLimit() {
  tone(2, 500, 60);
  delay(120);
  tone(2, 500, 60);
  delay(120);
  tone(2, 500, 60);  
}

void soundUpperLimit() {
  tone(2, 1000, 60);
  delay(120);
  tone(2, 1000, 60);
  delay(120);
  tone(2, 1000, 60);    
}

void soundAlarm() {
  for (int freq = 1000; freq < 2000; freq += 100) {
    tone(5, freq);
    delay(50);
  }

}

void onPDMdata() {
  // query the number of bytes available
  int bytesAvailable = PDM.available();
  
  PDM.read(sampleBuffer, bytesAvailable);

  samplesRead = bytesAvailable / sizeof(short);
}

void computeSoundSpectrum() {
  
  for (uint16_t i = 0; i < N_SAMPLES; i++) {
    vReal[i] = double(sampleBuffer[i]);
    vImag[i] = 0.0; 
  }

  FFT.Compute(vReal, vImag, N_SAMPLES, FFT_FORWARD); /* Compute FFT */
  FFT.ComplexToMagnitude(vReal, vImag, N_SAMPLES); /* Compute magnitudes */
  vReal[0] = 0.;  // Remove DC component, always very large

}

void showSoundLevels(int mode, int offset, int scale, double maxValue) {
  uint8_t intensity[256];
  for (uint16_t i = 0; i < 256; i++) {
     double value = vReal[i]/maxValue*255.0;
     if (value > 255) {
        intensity[i] = 255;
     } else {
        intensity[i] = uint8_t(value);
     }
  }
  
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
//    uint32_t color = strip.gamma32(strip.ColorHSV(65536/3, 255, intensity[i])); // hue -> RGB
//    uint32_t color = strip.Color(  0, intensity[i],   0);
//    uint32_t color = strip.Color(127, 127, 127);
//    int offset = 0*65536*2/3;
//    int scale = -65536/16;
//    uint32_t color = strip.gamma32(strip.ColorHSV((scale*intensity[i])/255+offset, 255, intensity[i])); // hue -> RGB
    uint32_t color;
    if (mode == SPECTRUM_METER) {
      color = strip.gamma32(strip.ColorHSV(offset + scale*i, 255, intensity[i])); // hue -> RGB
    } else if (mode == NOISE_LEVEL) {
      color = strip.gamma32(strip.ColorHSV(offset + scale*intensity[i], 255, intensity[i])); // hue -> RGB      
    }
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)    
  }
  strip.show();                          //  Update strip to match

}

void binSoundHistogram(int8_t binSize) {
  double maxValue = 0;

  for (uint16_t i = 0; i < 16; i++) {
    double sum = 0;
    for (uint16_t j = 0; j < binSize; j++) {
      sum += double(vReal[binSize*i + j]);
    }
    vReal[i] = sum;
    if (sum > maxValue) {
      maxValue = sum;
    }
  }
}
