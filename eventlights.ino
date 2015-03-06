#include <Adafruit_NeoPixel.h>

//
// Give a visual indication on front and rear facing 16 LED NeoPixel
// rings of the status of 4 robot sensors:
//
//   Left Ramp Reflectivity - Lower left third of ring
//   Right Ramp Reflectivity - Lower right third of ring
//     Red   = No ramp
//     Green = Ramp
//
//   Tote Presence - Top third of ring
//     Red    = No tote
//     Violet = Tote partially contained
//     BLue   = Tote completely contained
//
// PWM pulse duration used to select combination of colors:
//
//    RIO    PULSE      TOP        LEFT        RIGHT
//    000    000        OFF        OFF         OFF
//    001    250        RED        RED         RED
//    021    260        RED        RED         GREEN
//    041    270        RED        GREEN       RED
//    061    280        RED        GREEN       GREEN
//    081    290        VIOLET     RED         RED
//    101    300        VIOLET     RED         GREEN
//    121    310        VIOLET     GREEN       RED
//    141    320        VIOLET     GREEN       GREEN
//    161    330        GREEN      RED         RED
//    181    340        GREEN      RED         GREEN
//    201    350        GREEN      GREEN       RED
//    221    260        GREEN      GREEN       GREEN
//    241    370        WHITE      WHITE       WHITE
//

#define PWM_INPUT_PIN  2
#define FRONT_RING_PIN 5
#define BACK_RING_PIN  6

#define OFF    0x000000
#define RED    0xFF0000
#define GREEN  0x00FF00
#define BLUE   0x0000FF
#define VIOLET 0xFF00FF
#define WHITE  0xFFFFFF

#define TOP_PIXEL_COUNT   6
#define LEFT_PIXEL_COUNT  5
#define RIGHT_PIXEL_COUNT 5

#define FRONT_TOP_START_PIXEL   18
#define FRONT_LEFT_START_PIXEL  13
#define FRONT_RIGHT_START_PIXEL  8

#define BACK_TOP_START_PIXEL   10
#define BACK_LEFT_START_PIXEL   5
#define BACK_RIGHT_START_PIXEL  0

////////////////////////////////////////////////////////////////////////////

class RingSpan {

private:
  Adafruit_NeoPixel * ring;
  uint16_t startPixel;
  uint16_t count;

public:
  RingSpan(Adafruit_NeoPixel * ring, uint16_t startPixel, uint16_t count);

  void setColor(uint32_t color);
};

RingSpan::RingSpan(Adafruit_NeoPixel * ring, uint16_t startPixel, uint16_t count) {
  this->ring = ring;
  this->startPixel = startPixel;
  this->count = count;
}

void
RingSpan::setColor(uint32_t color) {
  uint16_t numPixels = ring->numPixels();

  for (int i = startPixel ; i < startPixel + count ; ++i) {
    if (i < numPixels) {
      ring->setPixelColor(i, color);
    } else {
      ring->setPixelColor(i - numPixels, color);
    }
  }
}

Adafruit_NeoPixel * frontRing = new Adafruit_NeoPixel(TOP_PIXEL_COUNT + LEFT_PIXEL_COUNT + RIGHT_PIXEL_COUNT, FRONT_RING_PIN, NEO_GRB | NEO_KHZ800);
Adafruit_NeoPixel * backRing = new Adafruit_NeoPixel(TOP_PIXEL_COUNT + LEFT_PIXEL_COUNT + RIGHT_PIXEL_COUNT, BACK_RING_PIN, NEO_GRB | NEO_KHZ800);

RingSpan * frontTop   = new RingSpan(frontRing, FRONT_TOP_START_PIXEL, TOP_PIXEL_COUNT);
RingSpan * frontLeft  = new RingSpan(frontRing, FRONT_LEFT_START_PIXEL, LEFT_PIXEL_COUNT);
RingSpan * frontRight = new RingSpan(frontRing, FRONT_RIGHT_START_PIXEL, RIGHT_PIXEL_COUNT);
RingSpan * backTop    = new RingSpan(backRing, BACK_TOP_START_PIXEL, TOP_PIXEL_COUNT);
RingSpan * backLeft   = new RingSpan(backRing, BACK_LEFT_START_PIXEL, LEFT_PIXEL_COUNT);
RingSpan * backRight  = new RingSpan(backRing, BACK_RIGHT_START_PIXEL, RIGHT_PIXEL_COUNT);

////////////////////////////////////////////////////////////////////////////

bool scanForPWMState(int state, uint32_t timeout = 10000) {
  timeout = micros() + timeout;

  while (timeout > micros()) {
    if (digitalRead(PWM_INPUT_PIN) == state) { return true; }
  }

  return false;
}

uint32_t samplePWM() {
  // scan for rising edge
  if (scanForPWMState(LOW)) {
    if (scanForPWMState(HIGH)) {
      uint32_t now = micros();
      if (scanForPWMState(LOW)) {
        return micros() - now;
      }
    }
  }

  return 0;
}

void updateAll(uint32_t top, uint32_t left, uint32_t right) {
  frontTop->setColor(top);
  frontLeft->setColor(left);
  frontRight->setColor(right);
  backTop->setColor(top);
  backLeft->setColor(left);
  backRight->setColor(right);
  frontRing->show();
  backRing->show();
}

void updateEventLights(uint32_t pwmPulseWidth) {
  if (pwmPulseWidth < 245) { updateAll(OFF, OFF, OFF); }
  else if (pwmPulseWidth < 255) { updateAll(RED, RED, RED); }
  else if (pwmPulseWidth < 265) { updateAll(RED, RED, GREEN); }
  else if (pwmPulseWidth < 275) { updateAll(RED, GREEN, RED); }
  else if (pwmPulseWidth < 285) { updateAll(RED, GREEN, GREEN); }
  else if (pwmPulseWidth < 295) { updateAll(VIOLET, RED, RED); }
  else if (pwmPulseWidth < 305) { updateAll(VIOLET, RED, GREEN); }
  else if (pwmPulseWidth < 315) { updateAll(VIOLET, GREEN, RED); }
  else if (pwmPulseWidth < 325) { updateAll(VIOLET, GREEN, GREEN); }
  else if (pwmPulseWidth < 335) { updateAll(GREEN, RED, RED); }
  else if (pwmPulseWidth < 345) { updateAll(GREEN, RED, GREEN); }
  else if (pwmPulseWidth < 355) { updateAll(GREEN, GREEN, RED); }
  else if (pwmPulseWidth < 365) { updateAll(GREEN, GREEN, GREEN); }
  else if (pwmPulseWidth < 375) { updateAll(WHITE, WHITE, WHITE); }
  else { updateAll(OFF, OFF, OFF); }
}

////////////////////////////////////////////////////////////////////////////

void setup() {
  pinMode(PWM_INPUT_PIN,  INPUT);
  pinMode(FRONT_RING_PIN, OUTPUT);
  pinMode(BACK_RING_PIN,  OUTPUT);

  updateAll(OFF, OFF, OFF);

  frontRing->setBrightness(0xFF);
  backRing->setBrightness(0xFF);
}

void loop() {
  // average 16 readings
  uint32_t pwmPulseWidth = 0;
  for (int i = 0; i < 16; ++i) { pwmPulseWidth += samplePWM(); }
  pwmPulseWidth /= 16;

  updateEventLights(pwmPulseWidth);
  delay(100);
}
