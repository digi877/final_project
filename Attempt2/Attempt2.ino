#include <Adafruit_NeoPixel.h>  //Library for LED interactions
#ifdef __AVR__
#include <avr/power.h>   //Includes the library for power reduction registers if your chip supports them. 
#endif                  

// Defining pins and constants
#define LED_PIN   A5  //Pin for the pixel strand. Can be analog or digital.
#define LED_TOTAL 60  //Change this to the number of LEDs in your strand.
#define LED_HALF  LED_TOTAL/2
#define VISUALS   6   //Change this accordingly if you add/remove a visual in the switch-case in Visualize()

#define AUDIO_PIN A0  //Pin for the envelope of the sound detector
#define KNOB_PIN  A1  //Pin for the trimpot 10K
#define BUTTON_1  6   //Button 1 cycles color palettes
#define BUTTON_2  5   //Button 2 cycles visualization modes
#define BUTTON_3  4   //Button 3 toggles shuffle mode (automated changing of color and visual)

Adafruit_NeoPixel strand = Adafruit_NeoPixel(LED_TOTAL, LED_PIN, NEO_GRB + NEO_KHZ800); 
uint16_t gradient = 0; //Used to iterate and loop through each color palette gradually
uint16_t thresholds[] = {1529, 1019, 764, 764, 764, 1274};


uint8_t palette = 0; // color palette
uint8_t visual = 0; // current visual
uint8_t volume = 0; // volume level from detector
uint8_t last = 0; // volume from previous loop

float maxVol = 15; // largest volume so far
float knob = 1023.0; // % of trimpot twist
float avgBump = 0; // average volume change
float avgVol = 0; // average volume level 
float shuffleTime = 0; // how many seconds ago last shuffle was

bool bump = false; // "bump" in volume detected (increase)

int8_t dotPos = 0; // holds dot position
bool left = false; // holds direction
float timeBump = 0;
float avgTime = 0;

int8_t pos[LED_TOTAL] = { -2};    //Stores a population of color "dots" to iterate across the LED strand.
uint8_t rgb[LED_TOTAL][3] = {0};  //Stores each dot's specific RGB values.

void setup() {
 Serial.begin(9600);
  // insert buttons here if they become relevant 
  strand.begin();
  strand.show();
  // sounds are input, lights are output
}

void loop() {
  volume = analogRead(AUDIO_PIN); // volume level from sound detector
  knob = analogRead(KNOB_PIN) / 1023.0; // trimpot twist level
  if (volume < avgVol / 2.0 || volume < 15) volume = 0; // account for noise
  else avgVol = (avgVol + volume) / 2.0; // average volumes 
  if (volume > maxVol) maxVol = volume;
  // insert things about the palettes, buttons, and shuffle if desired
  if (gradient > thresholds[palette]){
    gradient %= thresholds[palette] + 1;
    maxVol = (maxVol + volume) / 2.0;
  }
  if (volume - last > 10) avgBump = (avgBump + (volume - last)) / 2.0;
  bump = (volume - last > avgBump * .9);
  if (bump) {
    avgTime = (((millis() / 1000.0) - timeBump) + avgTime) / 2.0;
    timeBump = millis() / 1000.0;
  }
  Visualize(); // calls visualization based on gradients
  gradient++; // increments the gradient
  last = volume; // current volume recorded for bump detection
  delay(30); // puts delay on visuals so you can actually see them
}

void Visualize() {
  return Wave();
}

void Wave(){
  fade(0.8);
  if (volume > 0){
    //int start = 0; 
    //int finish = LED_TOTAL-1;
    int initColor = -(255/volume)+100; // sets initial color
    if (dotPos==LED_TOTAL) dotPos=0;
    dotPos+=1;
    strand.setPixelColor(dotPos, strand.Color(255, 255, 0));
    //}
  }
  strand.show();
}

uint8_t split(uint32_t color, uint8_t i ) {

  //0 = Red, 1 = Green, 2 = Blue

  if (i == 0) return color >> 16;
  if (i == 1) return color >> 8;
  if (i == 2) return color >> 0;
  return -1;
}

void fade(float damper) {

  //"damper" must be between 0 and 1, or else you'll end up brightening the lights or doing nothing.

  for (int i = 0; i < strand.numPixels(); i++) {

    //Retrieve the color at the current position.
    uint32_t col = strand.getPixelColor(i);

    //If it's black, you can't fade that any further.
    if (col == 0) continue;

    float colors[3]; //Array of the three RGB values

    //Multiply each value by "damper"
    for (int j = 0; j < 3; j++) colors[j] = split(col, j) * damper;

    //Set the dampened colors back to their spot.
    strand.setPixelColor(i, strand.Color(colors[0] , colors[1], colors[2]));
  }
}

