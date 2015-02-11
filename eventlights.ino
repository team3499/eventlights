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
//    PULSE      TOP        LEFT        RIGHT
//    500        OFF        OFF         OFF
//    520        RED        RED         RED
//    541        RED        RED         GREEN
//    562        RED        GREEN       RED
//    583        RED        GREEN       GREEN
//    604        VIOLET     RED         RED
//    625        VIOLET     RED         GREEN
//    646        VIOLET     GREEN       RED
//    667        VIOLET     GREEN       GREEN
//    688        GREEN      RED         RED
//    709        GREEN      RED         GREEN
//    730        GREEN      GREEN       RED
//    751        GREEN      GREEN       GREEN
//    772        WHITE      WHITE       WHITE
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

#define FRONT_TOP_START_PIXEL   10
#define FRONT_LEFT_START_PIXEL   5
#define FRONT_RIGHT_START_PIXEL  0

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

  for (int i = startPixel ; i < count ; ++i) {
    if (i < numPixels) {
      ring->setPixelColor(i, color);
    } else {
      ring->setPixelColor(i - numPixels, color);
    }
  }
  ring->show();
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
}

void updateEventLights(uint32_t pwmPulseWidth) {
  if (pwmPulseWidth < 510) { updateAll(OFF, OFF, OFF); }
  else if (pwmPulseWidth < 531) { updateAll(RED, RED, RED); }
  else if (pwmPulseWidth < 552) { updateAll(RED, RED, GREEN); }
  else if (pwmPulseWidth < 573) { updateAll(RED, GREEN, RED); }
  else if (pwmPulseWidth < 594) { updateAll(RED, GREEN, GREEN); }
  else if (pwmPulseWidth < 615) { updateAll(VIOLET, RED, RED); }
  else if (pwmPulseWidth < 636) { updateAll(VIOLET, RED, GREEN); }
  else if (pwmPulseWidth < 657) { updateAll(VIOLET, GREEN, RED); }
  else if (pwmPulseWidth < 678) { updateAll(VIOLET, GREEN, GREEN); }
  else if (pwmPulseWidth < 699) { updateAll(GREEN, RED, RED); }
  else if (pwmPulseWidth < 720) { updateAll(GREEN, RED, GREEN); }
  else if (pwmPulseWidth < 741) { updateAll(GREEN, GREEN, RED); }
  else if (pwmPulseWidth < 762) { updateAll(GREEN, GREEN, GREEN); }
  else { updateAll(WHITE, WHITE, WHITE); }
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
  updateEventLights(samplePWM());
  delay(100);
}
