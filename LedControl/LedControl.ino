#include <FastLED.h>

#define LED_PIN 2
#define NUM_LEDS 64

CRGB leds[NUM_LEDS];

void setup() {

    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(16);

    // for (int i = 0; i < NUM_LEDS; i++) {
    //     leds[i] = CRGB(255, 0, 0);
    // }
    // leds[63] = CRGB(255, 0, 0);

    // FastLED.show();
    // delay(1000);
}


int hue = 0;

void loop() {
    fill_rainbow(leds, NUM_LEDS, hue, 255 / NUM_LEDS);

    FastLED.show();
    delay(10);
    hue++;
}