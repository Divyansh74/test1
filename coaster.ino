#include <Adafruit_NeoPixel.h>
#include "hsv.h"
 // https://github.com/monkbroc/particle-hsv
// data pin
#define PIN 6
// led count
#define CNT 24
// max Hue
#define MAXHUE 256*6

int position = 0;
int hue = 0;
char output[128];

Adafruit_NeoPixel strip = Adafruit_NeoPixel(CNT, PIN, NEO_GRB + NEO_KHZ800);

uint32_t getPixelColorHsv(
  uint16_t n, uint16_t h, uint8_t s, uint8_t v) {

  uint8_t r, g, b;

  if (!s) {
    // Monochromatic, all components are V
    r = g = b = v;
  } else {
    uint8_t sextant = h >> 8;
    if (sextant > 5)
      sextant = 5;  // Limit hue sextants to defined space

    g = v;    // Top level

    // Perform actual calculations

    /*
       Bottom level:
       --> (v * (255 - s) + error_corr + 1) / 256
    */
    uint16_t ww;        // Intermediate result
    ww = v * (uint8_t)(~s);
    ww += 1;            // Error correction
    ww += ww >> 8;      // Error correction
    b = ww >> 8;

    uint8_t h_fraction = h & 0xff;  // Position within sextant
    uint32_t d;      // Intermediate result

    if (!(sextant & 1)) {
      // r = ...slope_up...
      // --> r = (v * ((255 << 8) - s * (256 - h)) + error_corr1 + error_corr2) / 65536
      d = v * (uint32_t)(0xff00 - (uint16_t)(s * (256 - h_fraction)));
      d += d >> 8;  // Error correction
      d += v;       // Error correction
      r = d >> 16;
    } else {
      // r = ...slope_down...
      // --> r = (v * ((255 << 8) - s * h) + error_corr1 + error_corr2) / 65536
      d = v * (uint32_t)(0xff00 - (uint16_t)(s * h_fraction));
      d += d >> 8;  // Error correction
      d += v;       // Error correction
      r = d >> 16;
    }

    // Swap RGB values according to sextant. This is done in reverse order with
    // respect to the original because the swaps are done after the
    // assignments.
    if (!(sextant & 6)) {
      if (!(sextant & 1)) {
        uint8_t tmp = r;
        r = g;
        g = tmp;
      }
    } else {
      if (sextant & 1) {
        uint8_t tmp = r;
        r = g;
        g = tmp;
      }
    }
    if (sextant & 4) {
      uint8_t tmp = g;
      g = b;
      b = tmp;
    }
    if (sextant & 2) {
      uint8_t tmp = r;
      r = b;
      b = tmp;
    }
  }
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

void setup() {
  Serial.begin(115200);
  strip.begin();
}

void loop() {
  // hue - red
  // saturation - max
  // value - 0-255
  for (int i = 0; i < CNT; i++) {
    strip.setPixelColor((i + position) % CNT, getPixelColorHsv(i, hue, 255, strip.gamma8(i * (255 / CNT))));
  }
  strip.show();
  sprintf(output, "hue: %d", hue);
  Serial.println(output);
  position++;
  position %= CNT;
  hue += 2;
  hue %= MAXHUE;
  delay(25);
}
