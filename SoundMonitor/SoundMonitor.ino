/*
  This is a simple sound monitor I made by reusing a lot of code from examples.

  With a Nano BLE Sense 33, we use the microphone, compute the spectrum and
  show it on an NeoPixel 16-LED circular element.
  
*/


#include <PDM.h>
#include "arduinoFFT.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define PIXEL_PIN   12  // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 16  // Number of NeoPixels

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, NEO_GRBW + NEO_KHZ800);
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

/* Audio spectrum variables */
arduinoFFT FFT = arduinoFFT(); /* Create FFT object */
/*
These are the input and output vectors, and also used for FFT
Input vectors receive computed results from FFT
*/
double vReal[N_SAMPLES];
double vImag[N_SAMPLES];

void binSoundHistogram(int8_t binSize = (N_SAMPLES/2/2)/PIXEL_COUNT);
void showSoundLevels(double maxValue = 5000);
void blink(int repetition = 1);

void setup() {
  /* Feedback LED to avoid using serial port */
  pinMode(LED_BUILTIN, OUTPUT);
  /* Setup serial port */
  Serial.begin(9600);

  /* Setup neopixel strip */
  strip.begin(); // Initialize NeoPixel strip object (REQUIRED)
  strip.show();  // Initialize all pixels to 'off'

  /* Setup microphone */
  PDM.onReceive(onPDMdata); // callback
  PDM.setGain(20);          

  if (!PDM.begin(1, 16000)) { // mono at 16 kHz
    blink(2);      
    while (1);
  }

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
    binSoundHistogram();
    showSoundLevels();
  }

//  soundAlarm();

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

  // read into the sample buffer
  if (bytesAvailable/2 > N_SAMPLES) {
    Serial.println("PDM returning more bytes than expected. Limiting to buffer size.");
    bytesAvailable = N_SAMPLES;
  }
  
  PDM.read(sampleBuffer, bytesAvailable);

  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;
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

void showSoundLevels(double maxValue) {
  uint8_t intensity[16];
  for (uint16_t i = 0; i < 16; i++) {
     intensity[i] = uint8_t(vReal[i]/maxValue*255.0);
  }
  
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
//    uint32_t color = strip.gamma32(strip.ColorHSV(65536/3, 255, intensity[i])); // hue -> RGB
//    uint32_t color = strip.Color(  0, intensity[i],   0);
//    uint32_t color = strip.Color(127, 127, 127);
    uint32_t color = strip.gamma32(strip.ColorHSV((65536/6*intensity[i])/255, 255, intensity[i])); // hue -> RGB
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
  }

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
