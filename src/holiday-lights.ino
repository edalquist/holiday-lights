/*
 * Project holiday-lights
 * Description:
 * Author:
 * Date:
 */
#include "ArduinoJson.h"
#include <FastLED.h>
FASTLED_USING_NAMESPACE

// Roof Line & Ghosts
#define COLOR_ORDER  GRB
#define CHIPSET      WS2812B
#define NUM_LEDS     300

//
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


const CRGB ORANGE = CRGB(255, 90, 0);
const CRGB PURPLE = CRGB(102, 0, 102);
const CRGB GREEN = CRGB::ForestGreen;

int brightness = BRIGHTNESS;
int stripWidth = 4;

int setBrightness(String bStr) {
    const long b = bStr.toInt();
    if (b < 0) {
        brightness = 0;
    } else if (b > 255) {
        brightness = 255;
    } else {
        brightness = b;
    }
}

int setStripWidth(String wStr) {
    const long w = wStr.toInt();
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

    // Roof Line
    // FastLED.addLeds<CHIPSET, D0, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalPixelString );
    // FastLED.addLeds<CHIPSET, D1, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalPixelString );

    // Ghost Poles
    FastLED.addLeds<CHIPSET, D0, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalPixelString );
    FastLED.addLeds<CHIPSET, D1, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalPixelString );
    FastLED.addLeds<CHIPSET, D2, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalPixelString );

    // FastLED.addLeds<CHIPSET, D3, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalPixelString );
    // FastLED.addLeds<CHIPSET, D4, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalPixelString );
    FastLED.setBrightness(BRIGHTNESS);

    InitPixelStates();
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

const int CHASE_DELAY = 25;

// Base background color
#define BASE_COLOR       CRGB(32,32,32)

// Peak color to twinkle up to
#define PEAK_COLOR       CRGB(200,200,200)

// Currently set to brighten up a bit faster than it dims down,
// but this can be adjusted.

// Amount to increment the color by each loop as it gets brighter:
#define DELTA_COLOR_UP   CRGB(4,4,4)

// Amount to decrement the color by each loop as it gets dimmer:
#define DELTA_COLOR_DOWN CRGB(2,2,2)


// Chance of each pixel starting to brighten up.
// 1 or 2 = a few brightening pixels at a time.
// 10 = lots of pixels brightening at a time.
#define CHANCE_OF_TWINKLE 1


enum { SteadyDim, GettingBrighter, GettingDimmerAgain };
uint8_t PixelState[NUM_LEDS];

void InitPixelStates()
{
  memset( PixelState, sizeof(PixelState), SteadyDim); // initialize all the pixels to SteadyDim.
  fill_solid( leds, NUM_LEDS, BASE_COLOR);
}

void TwinkleMapPixels()
{
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    if( PixelState[i] == SteadyDim) {
      // this pixels is currently: SteadyDim
      // so we randomly consider making it start getting brighter
      if( random8() < CHANCE_OF_TWINKLE) {
        PixelState[i] = GettingBrighter;
      }

    } else if( PixelState[i] == GettingBrighter ) {
      // this pixels is currently: GettingBrighter
      // so if it's at peak color, switch it to getting dimmer again
      if( leds[i] >= PEAK_COLOR ) {
        PixelState[i] = GettingDimmerAgain;
      } else {
        // otherwise, just keep brightening it:
        leds[i] += DELTA_COLOR_UP;
      }

    } else { // getting dimmer again
      // this pixels is currently: GettingDimmerAgain
      // so if it's back to base color, switch it to steady dim
      if( leds[i] <= BASE_COLOR ) {
        leds[i] = BASE_COLOR; // reset to exact base color, in case we overshot
        PixelState[i] = SteadyDim;
      } else {
        // otherwise, just keep dimming it down:
        leds[i] -= DELTA_COLOR_DOWN;
      }
    }
  }
}

void drawChristmasPole() {
    // // This is the "clock" that determines how fast the animation runs
    // int startIdx = ((millis() / CHASE_DELAY) % (NUM_LEDS / LEDS_PER_ROW)) * LEDS_PER_ROW;
    //
    // for (int n = 0; n < NUM_LEDS; n++) {
    //     CRGB color = CRGB::Red;
    //     if ((n / LEDS_PER_ROW / stripWidth) % 2 == 1) {
    //         color = CRGB::Green;
    //     }
    //     leds[(startIdx + n) % NUM_LEDS] = color;
    // }


      int startIdx = (millis() / CHASE_DELAY) % NUM_LEDS;

      for (int n = 0; n < NUM_LEDS; n++) {
        uint8_t wavePosition = ((float)(startIdx + n) / NUM_LEDS) * 255;
        uint8_t waveValue = cubicwave8(wavePosition);
        leds[n] = CRGB::Green;
        leds[n].nscale8_video(waveValue);
      }
}

void drawGhostPole() {
    // This is the "clock" that determines how fast the animation runs
    int startIdx = ((millis() / CHASE_DELAY) % (NUM_LEDS / LEDS_PER_ROW)) * LEDS_PER_ROW;

    for (int n = 0; n < NUM_LEDS; n++) {
        CRGB color = ORANGE;
        if ((n / LEDS_PER_ROW / stripWidth) % 2 == 1) {
            color = PURPLE;
        }
        leds[(startIdx + n) % NUM_LEDS] = color;
    }
}


void drawWhiteRoofLine() {
  int startIdx = (millis() / CHASE_DELAY) % NUM_LEDS;

  for (int n = 0; n < NUM_LEDS; n++) {
    uint8_t wavePosition = ((float)(startIdx + n) / NUM_LEDS) * 255;
    uint8_t waveValue = cubicwave8(wavePosition);
    leds[n] = CRGB::White;
    leds[n].nscale8_video(waveValue);
  }
}

void drawRoofLine() {
  int startIdx = (millis() / CHASE_DELAY) % NUM_LEDS;

  for (int n = 0; n < NUM_LEDS; n++) {
    uint8_t wavePosition = ((float)(startIdx + n) / NUM_LEDS) * 255;
    uint8_t waveValue = cubicwave8(wavePosition);
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
void addGlitter(uint8_t chanceOfGlitter) {
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}
void addNegativeGlitter(uint8_t chanceOfGlitter) {
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ].nscale8(random8());
  }
}

bool first = true;
void drawSparkleLine() {
  // First time turn everything white
  if (first) {
    for (int n = 0; n < NUM_LEDS; n++) {
      leds[n] = CRGB::White;
    }
    first = false;
  }

  // Make every LED brighter, clamps at FF for each channel
  for (int n = 0; n < NUM_LEDS; n++) {
    leds[n].addToRGB(random8(128));
  }

  EVERY_N_MILLISECONDS(1000/30) {
    // Dim 50% of the LEDs 30 times per second
    for (int n = 0; n < NUM_LEDS; n++) {
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

void drawOff() {
  for (int n = 0; n < NUM_LEDS; n++) {
    leds[n] = CRGB::Black;
  }
}


void loop() {
    //drawOff();
    // drawWreath();
    // drawSparkleLine();
    // drawPumpkin();
    // drawGhostPole();
    // drawRoofLine();

    // drawChristmasPole();
    TwinkleMapPixels();
    // drawWhiteRoofLine();

    FastLED.setBrightness(brightness);
    FastLED.show();
    delay(1000 / FRAMES_PER_SECOND);
}
