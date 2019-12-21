#include <FastLED.h>
#include <OneButton.h>
#include <EEPROM.h>

FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few
// of the kinds of animation patterns you can quickly and easily
// compose using FastLED.
//
// This example also shows one easy way to define multiple
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    3
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    50
CRGB leds[NUM_LEDS];

#define FRAMES_PER_SECOND  480

#define MAX_BRIGHTNESS 255

#define BUTTON_PIN 7
#define EEPROM_ADDR 0


// STUFF FOR RETROWAVE


DEFINE_GRADIENT_PALETTE( retrowave_gp ) {
    0,   228,  0, 247,
   63,   253, 0, 255,
  127,   0,253,241,
  191,  0,253, 213,
  255, 228,  0, 247
};

DEFINE_GRADIENT_PALETTE(fire_gp){
  0, 255, 0, 0, // Red
  60, 255, 165, 0,  // Orange
  90, 255, 255, 0, // bright yellow
  127, 255, 255, 255, // White
  164, 255, 255, 0, // bright yellow
  194, 255, 165, 0, //Orange
  255, 255, 0, 0    // Red
};

// Gradient palette "Sunset_Real_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Sunset_Real.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( Sunset_Real_gp ) {
    0, 120,  0,  0,
   11, 179, 22,  0,
   25, 255,104,  0,
   42, 167, 22, 18,
   68, 100,  0,103,
   99,  16,  0,130,
   128,   0,  0,160,
   156,  16,  0,130,
   187, 100,  0,103,
   213, 167, 22, 18,
   230, 255,104,  0,
   244, 179, 22,  0,
   255, 120,  0,  0
};



// Shooter variables
bool shooter_bullets[NUM_LEDS];
int8_t bullet_pos = -1;

// rainbow_pulse values
uint8_t pulse_hue = 0;
bool hue_lock = false;


bool buttonFlag = false;
bool setBrightnessFlag = false;

uint8_t currentBrightnessInc = 0;

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

OneButton cycleButton(BUTTON_PIN, true, true);


void buttonPress(){
  buttonFlag = true;
}

void brightnessMode(){
  setBrightnessFlag = !setBrightnessFlag;
}

//Sets brightness based on the current increment value
uint8_t incBrightnessSet(uint8_t brightnessInc, bool setBrightness = true){
  if(brightnessInc > 7) brightnessInc = 7;
  uint8_t brightness = 32*(brightnessInc+1);
  if(brightness ==  0) brightness = 255;

  if(setBrightness){
    brightness = map(brightness, 0, 255, 0, MAX_BRIGHTNESS);
    FastLED.setBrightness(brightness);
  }

  return brightness;
}

void incrementBrightness(){
   currentBrightnessInc ++;
   if(currentBrightnessInc > 7) currentBrightnessInc = 0;
   EEPROM.write(EEPROM_ADDR+1, currentBrightnessInc);
   incBrightnessSet(currentBrightnessInc);
}


void setup() {

  //Serial.begin(9600);

  cycleButton.attachClick(buttonPress);
  cycleButton.attachLongPressStart(brightnessMode);

  gCurrentPatternNumber = EEPROM.read(EEPROM_ADDR);
  currentBrightnessInc = EEPROM.read(EEPROM_ADDR+1);
  // Serial.println(gCurrentPatternNumber);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  // set master brightness control
  incBrightnessSet(currentBrightnessInc);

  for( uint8_t i=0; i<NUM_LEDS; i++){
    shooter_bullets[i] = false;
  }

}

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void palette_show(CRGBPalette16 gPal, uint8_t increment){
  uint8_t local_hue = gHue;
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette(gPal, local_hue, 255, LINEARBLEND);
        local_hue += increment;
    }
}

void retrowave(){
    palette_show(retrowave_gp, 5);
}

void fire(){
    palette_show(fire_gp, 5);
}

void ocean(){
  palette_show(OceanColors_p, 5);
}

void forest(){
  palette_show(ForestColors_p, 5);
}

void sunset(){
  palette_show(Sunset_Real_gp, 3);
}

void shooter_increment(){
  for(uint8_t i = NUM_LEDS -1; i > NUM_LEDS/2; i--){
    leds[i] = leds[i -1];
  }
  fadeToBlackBy(leds+NUM_LEDS/2, 1, 20);

}

void shooter()
{
  const uint8_t chance = 1;
  const uint8_t pos_speed = 60; //Leds per second note that frame rate is 120 per second

  EVERY_N_MILLISECONDS(1000/pos_speed){shooter_increment();}

  if( random8() <= chance){ // Going to start a new streak
    leds[NUM_LEDS/2] = CRGB(CHSV(gHue, 255, 255));  //Add a new colour
  }

  for(uint8_t i=0; i< NUM_LEDS/2; i++){
    leds[i] = leds[NUM_LEDS - i -1 ];
  }

}

void rainbow_pulse(){
  const uint8_t min_bright = 20;

  uint8_t brightness = beatsin8(8, min_bright, 255);

  if(!hue_lock && brightness<=min_bright){
    pulse_hue += 32;
    hue_lock = true;
  }
  if(hue_lock && brightness > min_bright) hue_lock = false;

  fill_solid(leds, NUM_LEDS, CHSV(pulse_hue,255, brightness));
}

void full_white(){
  fill_solid(leds, NUM_LEDS, CRGB::White);
}



// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, retrowave, fire, ocean, forest, sunset, shooter, rainbow_pulse};

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
  EEPROM.write(EEPROM_ADDR, gCurrentPatternNumber);
}

void brightnessShow(){
  uint8_t ledsPerColour = (NUM_LEDS - 8)/4;
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  fill_solid(leds, ledsPerColour, CRGB::Red);
  fill_solid(leds+ledsPerColour, ledsPerColour, CRGB::Green);
  fill_solid(leds+2*ledsPerColour, ledsPerColour, CRGB::Blue);
  fill_solid(leds+3*ledsPerColour, ledsPerColour+(NUM_LEDS-8)%4, CRGB::White);  // Make up for the rounding

  for(uint8_t i = 0; i <= currentBrightnessInc; i++ ){
    uint8_t brightness = 32*(1+i);
    if(brightness == 0) brightness = 255;

    leds[4*ledsPerColour+i+(NUM_LEDS-8)%4] = CHSV(i*14, 255, 255);
  }


}


void loop()
{
    cycleButton.tick();

  if(gCurrentPatternNumber >= ARRAY_SIZE( gPatterns)) gCurrentPatternNumber = 1;

  if(!setBrightnessFlag){
    // Call the current pattern function once, updating the 'leds' array
    gPatterns[gCurrentPatternNumber]();
  }
  else{
    brightnessShow();
  }

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND);

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  if(buttonFlag){
    if(!setBrightnessFlag){
      nextPattern();
    }
    else{
      incrementBrightness();
    }
    buttonFlag = false;
  }

}
