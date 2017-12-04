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

uint16_t thresholds[] = {1529, 1019, 764, 764, 764, 1274}; // biggest value the rainbow gradient takes before looping
uint16_t gradient=0; 

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

void setup() {
  Serial.begin(9600);
  // insert buttons here if they become relevant 
  strand.begin();
  strand.show();
  // sounds are input, lights are output
}

void loop() {
  // put your main code here, to run repeatedly:
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

void Visualize(){
  return Wave(); // starting with just one
  // add switch case for multiple, and assign them to global 
}

uint32_t ColorPalette(float num){
  switch (palette){
    case 0: return (num<0) ? Rainbow(gradient) : Rainbow(num);
  }
}

void Wave(){
  //if (bump) left = !left;
  fade(0.8); // fades on each loop pass
  if (bump) gradient += thresholds[palette] / 24;
  if (volume > avgVol) {
    //dotPos = random(strand.numPixels() - 1);
    int start = 1;
    //int finish = LED_HALF + (LED_HALF * (volume/maxVol)) + strand.numPixels() %2;
    int finish = 60;
    for (int i=start; i<finish; i++){
       float damp = sin((i-start) * PI / float(finish-start));
      damp = pow(damp, 2.0);
      int val = thresholds[palette] * (i-start) / (finish-start);
      val+=gradient;
      uint32_t col = ColorPalette(val); // note that changing this to "val" produces color variation
      uint32_t col2 = strand.getPixelColor(i);
      uint8_t colors[3];
      float avgCol = 0, avgCol2 = 0;
      for (int k=0; k<3; k++){
        colors[k] = split(col, k) * damp * knob * pow(volume/maxVol, 2);
        avgCol += colors[k];
        avgCol2 += split(col2, k);
      }
      avgCol /= 3.0, avgCol2 /= 3.0;
      if (avgCol > avgCol2) strand.setPixelColor(i, strand.Color(colors[0], colors[1], colors[2]));
    }
  }
  strand.show();
  //if (dotPos<0) dotPos = strand.numPixels()-1;
  //else if (dotPos >= strand.numPixels()) dotPos=0;
}


void fade(float damper){
  for (int i=0; i<strand.numPixels(); i++){
    uint32_t col = strand.getPixelColor(i);
    if (col==0) continue;
    float colors[3]; 
    for (int j=0; j<3; j++) colors[j] = split(col, j)*damper;
    strand.setPixelColor(i, strand.Color(colors[0], colors[1], colors [2]));
  }
}

int8_t split(uint32_t color, uint8_t i){
  if (i==0) return color >> 16;
  if (i==1) return color >> 8;
  if (i==2) return color >>0;
  return -1;
}


uint32_t Rainbow(unsigned int i){
  if (i > 1529) return Rainbow(i%1530); 
  if (i > 1274) return strand.Color(255, 0, 255- (i % 255)); // violet->red
  if (i > 1019) return strand.Color((i % 255), 0, 255); // blue->violet
  if (i > 764) return strand.Color(0, 255 - (i % 255), 255); // aqua->blue
  if (i > 509) return strand.Color(0, 255, (i % 255)); // green->aqua
  if (i > 255) return strand.Color(255 - (i%255), 255, 0); // yellow->green
  return strand.Color(255, i, 0); // red->yellow
}


