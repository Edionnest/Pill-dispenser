// board  in use: Doit esp32 devkit 1
// Note: The reset button doesn't perform the intended funtion due. if the alarm is playing when the pill is not there, the sensitivity of the ir sensor can be tuned 
//          using a screw driver

#include <Arduino.h>
#include <Stepper.h>
#include <FastLED.h>

//==============================================================
// stepper motor setup
int stepsperRevolutions = 2048;
Stepper carousel1(stepsperRevolutions, 23, 21, 22, 19);
Stepper carousel2(stepsperRevolutions, 18, 4, 5, 2);
int motSpeed = 10;
int rotation_angle = 50;
//===============================================================

//===============================================================
// button setup
const int start_button = 25;
const int reset_button = 26;
int start_val;
int reset_val;
//===============================================================

//===============================================================
// infra-red sensor setup
const int ir_pin = 39;
int ir_val;
//===============================================================

//===============================================================
// ledstrip setup
const int led_strip_pin = 27;
const int num_leds = 7;
const int led_brightness = 225;
CRGB leds[num_leds];
//===============================================================

//===============================================================
// buzzer pin setup
const int buzzer_pin = 33;
//===============================================================

void setup() {
  Serial.begin(9600);

  pinMode(start_button, INPUT_PULLUP);
  pinMode(reset_button, INPUT_PULLUP);
  pinMode(ir_pin, INPUT);
  pinMode(buzzer_pin, OUTPUT);

  carousel1.setSpeed(motSpeed);
  carousel2.setSpeed(motSpeed);

  FastLED.addLeds<NEOPIXEL, led_strip_pin>(leds, num_leds);
  FastLED.setBrightness(led_brightness);
  clear_led_strip();
}

void loop() {
  start_val = digitalRead(start_button);
  ir_val = digitalRead(ir_pin);

  if (start_val == false) {
    rotateCarousel();
  }

  if (ir_val == false) {
    alarm_song();
  } else {
    stop_alarm();
  }

  noTone(buzzer_pin);
}

// convert the degrees needed to rotate the carouel to steps
int carousel_step(int angleDegrees) {
  return (stepsperRevolutions * angleDegrees) / 360;
}

// rotates the stepper motors
void rotateCarousel() {
  int steps = carousel_step(rotation_angle);

  carousel1.step(steps);
  carousel2.step(steps);
}

// turns off  the led strip
void clear_led_strip() {
  fill_solid(leds, num_leds, CRGB::Black);
  FastLED.show();
}

void stop_alarm() {
  noTone(buzzer_pin);S
  clear_led_strip();
}

bool pill_removed() {
  bool reset_button_pressed = digitalRead(reset_button) == false;

  bool ir_not_detect = digitalRead(ir_pin) == true;

  return reset_button_pressed || ir_not_detect;
}

void alarm_song() {

  static int alarm_stage = 0;

  // Set colour for the current full alarm loop
  if (alarm_stage == 0) {
    clear_led_strip() ;
    delay(1);
    set_led_strip(CRGB::Green);
  } 
  else if (alarm_stage == 1) {
    set_led_strip(CRGB::Yellow);

  } 
  else if (alarm_stage == 2) {
    set_led_strip(CRGB::Red);
  } 
  else {
    // After blue, green, and red stages,
    // blink red if the pill is still there
    while (!pill_removed()) {

      set_led_strip(CRGB::Red);
      tone(buzzer_pin, 480);
      if (wait_and_check(300)) {
        stop_alarm();
        alarm_stage = 0;
        return;
      }

      clear_led_strip();
      noTone(buzzer_pin);
      if (wait_and_check(300)) {
        stop_alarm();
        alarm_stage = 0;
        return;
      }

      set_led_strip(CRGB::Red);
      tone(buzzer_pin, 600);
      if (wait_and_check(300)) {
        stop_alarm();
        alarm_stage = 0;
        return;
      }

      clear_led_strip();
      noTone(buzzer_pin);
      if (wait_and_check(300)) {
        stop_alarm();
        alarm_stage = 0;
        return;
      }
    }

    stop_alarm();
    alarm_stage = 0;
    return;
  }

  // Full alarm loop for the current colour
  for (int note = 0; note <= 5; note++) {

    if (pill_removed()) {
      stop_alarm();
      alarm_stage = 0;
      return;
    }

    tone(buzzer_pin, 480);
    if (wait_and_check(1000)) {
      stop_alarm();
      alarm_stage = 0;
      return;
    }

    tone(buzzer_pin, 600);
    if (wait_and_check(1000)) {
      stop_alarm();
      alarm_stage = 0;
      return;
    }
  }

  // After the loop reaches the required number of loops, change colour for next full loop
  alarm_stage++;
}

bool wait_and_check(unsigned long duration) {
  unsigned long start_time = millis();

  while (millis() - start_time <= duration) {
    if (pill_removed()) {
      return true;
    }
  }

  return false;
}

// function to set the colour of the leds
void set_led_strip(CRGB colour) {
  fill_solid(leds, num_leds, colour);
  FastLED.show();
}