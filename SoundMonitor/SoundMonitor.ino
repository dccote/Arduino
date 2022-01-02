/*
  This example reads audio data from the on-board PDM microphones, and prints
  out the samples to the Serial console. The Serial Plotter built into the
  Arduino IDE can be used to plot the audio data (Tools -> Serial Plotter)

  Circuit:
  - Arduino Nano 33 BLE Sense board

  This example code is in the public domain.
*/


#include <PDM.h>
#include "arduinoFFT.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Digital IO pin connected to the button. This will be driven with a
// pull-up resistor so the switch pulls the pin to ground momentarily.
// On a high -> low transition the button press logic will execute.
#define BUTTON_PIN   2

#define PIXEL_PIN    12  // Digital IO pin connected to the NeoPixels.

#define PIXEL_COUNT 16  // Number of NeoPixels

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, NEO_RGBW + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

boolean oldState = HIGH;
int     mode     = 0;    // Currently-active animation mode, 0-9
int     alarm    = 1000;

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */
/*
These are the input and output vectors
Input vectors receive computed results from FFT
*/

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

#define N_SAMPLES 256

// buffer to read samples into, each sample is 16-bits
short sampleBuffer[N_SAMPLES];
// FFT buffer
double vReal[N_SAMPLES];
double vImag[N_SAMPLES];

// number of samples read
volatile int samplesRead;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  strip.begin(); // Initialize NeoPixel strip object (REQUIRED)
  strip.show();  // Initialize all pixels to 'off'

  // configure the data receive callback
  PDM.onReceive(onPDMdata);

  // optionally set the gain, defaults to 20
  PDM.setGain(10);

  // initialize PDM with:
  // - one channel (mono mode)
  // - a 16 kHz sample rate
  if (!PDM.begin(1, 16000)) {
    Serial.println("Failed to start PDM!");
    while (1);
  }

}

void loop() {
  // wait for samples to be read
  if (samplesRead ) {
    spectrum();
  }

//  soundAlarm();

}

void soundAlarm() {
  for (int freq = 1000; freq < 2000; freq += 100) {
    tone(5, freq);
    delay(50);
  }

}

void printSamplesRead() {
    // print samples to the serial monitor or plotter
    for (int i = 0; i < samplesRead; i++) {
      Serial.println(sampleBuffer[i]);
      // check if the sound value is higher than 500
      if (sampleBuffer[i]>=500){
        digitalWrite(LEDR,LOW);
        digitalWrite(LEDG,HIGH);
        digitalWrite(LEDB,HIGH);
      }
      // check if the sound value is higher than 250 and lower than 500
      if (sampleBuffer[i]>=250 && sampleBuffer[i] < 500){
        digitalWrite(LEDB,LOW);
        digitalWrite(LEDR,HIGH);
        digitalWrite(LEDG,HIGH);
      }
      //check if the sound value is higher than 0 and lower than 250
      if (sampleBuffer[i]>=0 && sampleBuffer[i] < 250){
        digitalWrite(LEDG,LOW);
        digitalWrite(LEDR,HIGH);
        digitalWrite(LEDB,HIGH);
      }
    }

    // clear the read count
    samplesRead = 0;
}

void onPDMdata() {
  // query the number of bytes available
  int bytesAvailable = PDM.available();

  // read into the sample buffer
  PDM.read(sampleBuffer, bytesAvailable);

  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;
}

//void loop()
//{
//  /* Build raw data */
//  double cycles = (((samples-1) * signalFrequency) / samplingFrequency); //Number of signal cycles that the sampling will read
//  for (uint16_t i = 0; i < samples; i++)
//  {
//    vReal[i] = int8_t((amplitude * (sin((i * (twoPi * cycles)) / samples))) / 2.0);/* Build data with positive and negative values*/
//    //vReal[i] = uint8_t((amplitude * (sin((i * (twoPi * cycles)) / samples) + 1.0)) / 2.0);/* Build data displaced on the Y axis to include only positive values*/
//    vImag[i] = 0.0; //Imaginary part must be zeroed in case of looping to avoid wrong calculations and overflows
//  }
//  /* Print the results of the simulated sampling according to time */
//  Serial.println("Data:");
//  PrintVector(vReal, samples, SCL_TIME);
//  FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);  /* Weigh data */
//  Serial.println("Weighed data:");
//  PrintVector(vReal, samples, SCL_TIME);
//  FFT.Compute(vReal, vImag, samples, FFT_FORWARD); /* Compute FFT */
//  Serial.println("Computed Real values:");
//  PrintVector(vReal, samples, SCL_INDEX);
//  Serial.println("Computed Imaginary values:");
//  PrintVector(vImag, samples, SCL_INDEX);
//  FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes */
//  Serial.println("Computed magnitudes:");
//  PrintVector(vReal, (samples >> 1), SCL_FREQUENCY);
//  double x = FFT.MajorPeak(vReal, samples, samplingFrequency);
//  Serial.println(x, 6);
////  while(1); /* Run Once */
//  delay(2000); /* Repeat after delay */
//}

void spectrum() {

  for (uint16_t i = 0; i < N_SAMPLES; i++) {
    vReal[i] = double(sampleBuffer[i]);
    vImag[i] = 0.0; 
  }

  FFT.Compute(vReal, vImag, N_SAMPLES, FFT_FORWARD); /* Compute FFT */
  FFT.ComplexToMagnitude(vReal, vImag, N_SAMPLES); /* Compute magnitudes */
  vReal[0] = 0.;
  double maxValue = 0;
  int8_t binSize = (N_SAMPLES/2/2)/16;
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
//  Serial.println(maxValue);
  maxValue = 5000;
  uint8_t intensity[16];
  for (uint16_t i = 0; i < 16; i++) {
     intensity[i] = uint8_t(vReal[i]/maxValue*255.0);
//     Serial.print(i);
//     Serial.print("\t");
//     Serial.println(intensity[i]);
  }

  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
//    uint32_t color = strip.gamma32(strip.ColorHSV(65536/3, 255, intensity[i])); // hue -> RGB
    uint32_t color = strip.gamma32(strip.ColorHSV(65536/3 - (65536/3*intensity[i])/255, 255, intensity[i])); // hue -> RGB
//    uint32_t color = strip.Color(  0, intensity[i],   0);
//    uint32_t color = strip.Color(127, 127, 127);
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
//    delay(200);                           //  Pause for a moment
  }

//  
//
//  Serial.println(maxValue);
//  PrintVector(vReal, N_SAMPLES, SCL_INDEX);
//  delay(2000);
}


void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa = i;
//    /* Print abscissa value */
//    switch (scaleType)
//    {
//      case SCL_INDEX:
//        abscissa = (i * 1.0);
//  break;
//      case SCL_TIME:
//        abscissa = ((i * 1.0) / samplingFrequency);
//  break;
//      case SCL_FREQUENCY:
//        abscissa = ((i * 1.0 * samplingFrequency) / samples);
//  break;
//    }
    Serial.print(abscissa, 6);
    if(scaleType==SCL_FREQUENCY)
      Serial.print("Hz");
    Serial.print(" ");
    Serial.println(vData[i], 4);
  }
  Serial.println();
}


//void setup() {
//  pinMode(BUTTON_PIN, INPUT_PULLUP);
//  strip.begin(); // Initialize NeoPixel strip object (REQUIRED)
//  strip.show();  // Initialize all pixels to 'off'
//}

//void loop() {
//  // Get current button state.
//  boolean newState = digitalRead(BUTTON_PIN);
//
//  // Check if state changed from high to low (button press).
//  if((newState == LOW) && (oldState == HIGH)) {
//    // Short delay to debounce button.
//    delay(20);
//    // Check if button is still low after debounce.
//    newState = digitalRead(BUTTON_PIN);
//    if(newState == LOW) {      // Yes, still low
//      if(++mode > 8) mode = 0; // Advance to next mode, wrap around after #8
//      switch(mode) {           // Start the new animation...
//        case 0:
//          colorWipe(strip.Color(  0,   0,   0), 50);    // Black/off
//          break;
//        case 1:
//          colorWipe(strip.Color(255,   0,   0), 50);    // Red
//          break;
//        case 2:
//          colorWipe(strip.Color(  0, 255,   0), 50);    // Green
//          break;
//        case 3:
//          colorWipe(strip.Color(  0,   0, 255), 50);    // Blue
//          break;
//        case 4:
//          theaterChase(strip.Color(127, 127, 127), 50); // White
//          break;
//        case 5:
//          theaterChase(strip.Color(127,   0,   0), 50); // Red
//          break;
//        case 6:
//          theaterChase(strip.Color(  0,   0, 127), 50); // Blue
//          break;
//        case 7:
//          rainbow(10);
//          break;
//        case 8:
//          theaterChaseRainbow(50);
//          break;
//      }
//    }
//  }
//
//  // Set the last-read button state to the old state.
//  oldState = newState;
//}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 3 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 3*65536. Adding 256 to firstPixelHue each time
  // means we'll make 3*65536/256 = 768 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 3*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}
