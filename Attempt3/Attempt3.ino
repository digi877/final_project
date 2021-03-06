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
  if (bump){
    int8_t slot = 0;
    for (slot; slot<sizeof(pos); slot++){
      if (pos[slot] < -1) break;
      else if (slot+1 >= sizeof(pos)) {
        slot=-3;
        break;
      }
    }
    if (slot != -3) {
      pos[slot] = strand.numPixels();
      uint32_t col = Rainbow(gradient);
      gradient += thresholds[palette] / 24; 
      for (int j=0; j<3; j++){
        rgb[slot][j] = split(col, j);
      }
    }
    if (volume>0){
      for (int i=0; i<sizeof(pos); i++){
        if (pos[i] < -1) continue;
        strand.setPixelColor(pos[i], strand.Color(float(rgb[i][0]) * pow(volume / maxVol, 2.0) * knob,
                              float(rgb[i][1]) * pow(volume / maxVol, 2.0) * knob,
                              float(rgb[i][2]) * pow(volume / maxVol, 2.0) * knob)
                          );
      }
      //dotPos+=1;
      //strand.setPixelColor(dotPos, strand.Color());
    }
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
  for (int i = 0; i < strand.numPixels(); i++) {
    uint32_t col = strand.getPixelColor(i);
    if (col == 0) continue;
    float colors[3]; //Array of the three RGB values
    for (int j = 0; j < 3; j++) colors[j] = split(col, j) * damper;
    strand.setPixelColor(i, strand.Color(colors[0] , colors[1], colors[2]));
  }
}

uint32_t Rainbow(int volume) {
  if (volume > 30) return Rainbow(volume%1530);
  if (volume > 25 && volume < 30) return strand.Color(255, 0, 255-(volume%255));
  if (volume > 20 && volume < 25) return strand.Color((volume%255), 0, 255);
  if (volume > 15 && volume < 20) return strand.Color(0, 255-(volume%255), 255);
  if (volume > 10 && volume < 15) return strand.Color(0, 255, (volume%255)); 
  if (volume > 5 && volume < 10) return strand.Color(255-(volume%255), 255, 0);
  return strand.Color(255, volume, 0); 
}

