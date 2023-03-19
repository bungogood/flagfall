#include <FastLED.h>

#define LED_PIN 2
#define NUM_LEDS 64

CRGB leds[NUM_LEDS];
uint8_t brightness = 16;
int8_t display_direction = 0;

void setup() {
    set_LED_xy(3, 5, CRGB::BlueViolet);
    set_LED_ith(61, CRGB::Salmon);
    display_LED();
}


void loop() {

}

/*
* The setup() function in Led Control
* 
* @param brightness: Brightness of all LEDs, [0 - 255], recommended 16
*/
void setup_LED(int brightness) {
    // Clip brightness to avoid burning out LEDs
    if (brightness > 50) {
        brightness = 50;
    }
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(brightness);
}


/*
* Set the color of a LED at position (x, y)
*
* @param x: Row number [1 - 8]
* @param y: Column number [1 - 8]
*/ 
void set_LED_xy(int row, int col, CRGB color) {
    row--;
    col--;
    if (col % 2 == 0) {
        row = 7 - row;
    }
    leds[(col * 8) + row] = color;
}

/*
* Set the color of a LED at position i
*
* @param i: Integer between 0 (bottom left) and 63 (top right)
*/
void set_LED_ith(int i, CRGB color) {
    if (i / 8 % 2 == 0) {
        i = 7 - (i % 8);
    }
    leds[i] = color;
}

void display_LED() {
    FastLED.show();
}
