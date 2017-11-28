/*
 * Project holiday-lights
 * Description:
 * Author:
 * Date:
 */
#include "ArduinoJson.h"
#include <FastLED.h>
FASTLED_USING_NAMESPACE

#define LED_PIN      D0
#define COLOR_ORDER  GRB
#define CHIPSET      WS2812B
#define NUM_LEDS     300

// #define COLOR_ORDER  NSFastLED::GRB
// #define CHIPSET      WS2811
// #define NUM_LEDS     50

#define LEDS_PER_ROW 5
#define BRIGHTNESS  128
#define FRAMES_PER_SECOND 60

CRGB leds[NUM_LEDS];

// Gradient palette "GMT_hot_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/gmt/tn/GMT_hot.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

DEFINE_GRADIENT_PALETTE( GMT_hot_gp ) {
    0,   0,  0,  0,
   95, 255,  0,  0,
  191, 255,255,  0,
  255, 255,255,255};

// Gradient palette "bhw1_01_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_01.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 12 bytes of program space.

DEFINE_GRADIENT_PALETTE( bhw1_01_gp ) {
    0, 227,101,  3,
  117, 194, 18, 19,
  255,  92,  8,192};

// Gradient palette "bhw1_04_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_04.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( bhw1_04_gp ) {
    0, 229,227,  1,
   15, 227,101,  3,
  142,  40,  1, 80,
  198,  17,  1, 79,
  255,   0,  0, 45};

DEFINE_GRADIENT_PALETTE( halloween_p ) {
    0, 255, 90,  0,
  170, 102,  0,102,
  255, 102,  0,102};


const auto ORANGE = CRGB(255, 90, 0);
const auto PURPLE = CRGB(102, 0, 102);
const auto GREEN = CRGB::ForestGreen;

int brightness = BRIGHTNESS;
int stripWidth = 2;

int setBrightness(String bStr) {
    const auto b = bStr.toInt();
    if (b < 0) {
        brightness = 0;
    } else if (b > 255) {
        brightness = 255;
    } else {
        brightness = b;
    }
}

int setStripWidth(String wStr) {
    const auto w = wStr.toInt();
    if (w < 1) {
        stripWidth = 1;
    } else if (w > (NUM_LEDS / LEDS_PER_ROW)) {
        stripWidth = (NUM_LEDS / LEDS_PER_ROW) - 1;
    } else {
        stripWidth = w;
    }
}

void setup() {
    Particle.variable("brightness", brightness);
    Particle.variable("stripWidth", stripWidth);

    Particle.function("setBright", setBrightness);
    Particle.function("setWidth", setStripWidth);

    delay(5000); // setup guard
    FastLED.addLeds<CHIPSET, D0, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalPixelString );
    FastLED.addLeds<CHIPSET, D1, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalPixelString );
    // FastLED.addLeds<CHIPSET, D2, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalPixelString );
    // FastLED.addLeds<CHIPSET, D3, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalPixelString );
    // FastLED.addLeds<CHIPSET, D4, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalPixelString );
    FastLED.setBrightness(BRIGHTNESS);
}


void drawPumpkinPole() {
    const int STEM_START = NUM_LEDS - (6 * LEDS_PER_ROW);
    for (int n = 0; n < NUM_LEDS; n++) {
        if (n < STEM_START) {
            leds[n] = ORANGE;
        } else {
            leds[n] = GREEN;
        }
    }
}

const auto CHASE_DELAY = 25;

void drawGhostPole() {
    // This is the "clock" that determines how fast the animation runs
    auto startIdx = ((millis() / CHASE_DELAY) % (NUM_LEDS / LEDS_PER_ROW)) * LEDS_PER_ROW;

    for (auto n = 0; n < NUM_LEDS; n++) {
        auto color = ORANGE;
        if ((n / LEDS_PER_ROW / stripWidth) % 2 == 1) {
            color = PURPLE;
        }
        leds[(startIdx + n) % NUM_LEDS] = color;
    }
}


void drawRoofLine() {
  auto startIdx = (millis() / CHASE_DELAY) % NUM_LEDS;

  for (auto n = 0; n < NUM_LEDS; n++) {
    auto wavePosition = ((float)(startIdx + n) / NUM_LEDS) * 255;
    auto waveValue = cubicwave8(wavePosition);
    leds[n] = ORANGE;
    leds[n].nscale8_video(waveValue);
  }
}


CRGBPalette16 currentPalette( CRGB::Blue );
CRGBPalette16 targetPalette( CRGB::Blue );

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
  uint8_t flicker = 30;
  uint8_t brightness = qadd8(sin8(colorIndex), 2 + flicker + random8(flicker)) - flicker;

  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette, colorIndex + sin8(i*16), brightness);
    colorIndex += 3;
  }
}

void ChangePalettePeriodically()
{
  uint8_t secondHand = (millis() / 1000) % 60;
  static uint8_t lastSecond = 99;
  static uint8_t changeAccumulator = 0;

  if( lastSecond != secondHand) {
    lastSecond = secondHand;
    changeAccumulator += random8(11);
    if (changeAccumulator > 75) {
      changeAccumulator = 0;
      switch (random8(6)) {
        case 0 : targetPalette = halloween_p; break;
        case 1 : targetPalette = CRGBPalette16(ORANGE); break;
        case 2 : targetPalette = CRGBPalette16(PURPLE); break;
        case 3 : targetPalette = CRGBPalette16(CRGB::Red); break;
        case 4 : targetPalette = CRGBPalette16(CRGB::Green); break;
        case 5 : targetPalette = CRGBPalette16(CRGB::Blue); break;
      }
    }
  }
}

void drawPumpkin() {
    ChangePalettePeriodically();

    // Crossfade current palette slowly toward the target palette
    //
    // Each time that nblendPaletteTowardPalette is called, small changes
    // are made to currentPalette to bring it closer to matching targetPalette.
    // You can control how many changes are made in each call:
    //   - the default of 24 is a good balance
    //   - meaningful values are 1-48.  1=veeeeeeeery slow, 48=quickest
    //   - "0" means do not change the currentPalette at all; freeze

    uint8_t maxChanges = 6;
    nblendPaletteTowardPalette( currentPalette, targetPalette, maxChanges);


    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */
    FillLEDsFromPaletteColors( startIndex);
}

//glitter effect
void addGlitter(auto chanceOfGlitter) {
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}
void addNegativeGlitter(auto chanceOfGlitter) {
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ].nscale8(random8());
  }
}

auto first = true;
void drawSparkleLine() {
  // First time turn everything white
  if (first) {
    for (auto n = 0; n < NUM_LEDS; n++) {
      leds[n] = CRGB::White;
    }
    first = false;
  }

  // Make every LED brighter, clamps at FF for each channel
  for (auto n = 0; n < NUM_LEDS; n++) {
    leds[n].addToRGB(random8(128));
  }

  EVERY_N_MILLISECONDS(1000/30) {
    // Dim 50% of the LEDs 30 times per second
    for (auto n = 0; n < NUM_LEDS; n++) {
      if (random8(100) < 50) {
        leds[random16(NUM_LEDS)].nscale8(random8(128) + 16);
      }
    }
  }
}

void drawWreath() {
  //
  // for (auto n = 0; n < NUM_LEDS; n++) {
  //   switch ((seconds16() / 2) % 4) {
  //     case 0: {
  //       leds[n] = CRGB(255,0,0);
  //       break;
  //     }
  //     case 1: {
  //       leds[n] = CRGB(0,255,0);
  //       break;
  //     }
  //     case 2: {
  //       leds[n] = CRGB(0,0,255);
  //       break;
  //     }
  //     case 3: {
  //       leds[n] = CRGB(255,255,255);
  //       break;
  //     }
  //   }
  //
  // }


  // for (auto n = 0; n < NUM_LEDS; n++) {
  //   switch ((n / 3) % 4) {
  //     case 0: {
  //       leds[n] = CRGB(255,0,0);
  //       break;
  //     }
  //     case 1: {
  //       leds[n] = CRGB(0,255,0);
  //       break;
  //     }
  //     case 2: {
  //       leds[n] = CRGB(0,0,255);
  //       break;
  //     }
  //     case 3: {
  //       leds[n] = CRGB(255,255,255);
  //       break;
  //     }
  //   }
  // }


   leds[0] = CRGB(255,0,0);
   leds[1] = CRGB(0,255,0);
   leds[2] = CRGB(0,255,0);
   leds[3] = CRGB(0,0,255);
   leds[4] = CRGB(0,0,255);
   leds[5] = CRGB(0,0,255);
}


void loop() {
    // drawWreath();
    drawSparkleLine();
    // drawPumpkin();
    // drawGhostPole();
    // drawRoofLine();

    FastLED.setBrightness(brightness);
    FastLED.show();
    delay(1000 / FRAMES_PER_SECOND);
}
