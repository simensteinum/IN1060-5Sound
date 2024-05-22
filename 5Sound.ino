#include <FastLED.h>

//Pins
#define LED_PIN     3             // LEDs connected digital pin 3

//Leds configurations
#define NUM_LEDS    7
#define BRIGHTNESS  10

#define LED_TYPE    WS2813
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

void setup() {
  // put your setup code here, to run once:
  
  //FastLED.addLeds<WS2813, DATA_PIN>(leds, NUM_LEDS);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
    leds[0] = CRGB(255, 0, 0);
  FastLED.show();
  delay(500);  
  leds[1] = CRGB(0, 255, 0);
  FastLED.show();
  delay(500);
  leds[2] = CRGB(0, 0, 255);
  FastLED.show();
  delay(500);
  leds[3] = CRGB(150, 0, 255);
  FastLED.show();
}

void leds_color(int r, int g, int b) {
  for (int i = 0; i < NUM_LEDS ; ++i) {
    leds[i] = CRGB(r, g, b);
  }
  FastLED.show();
}