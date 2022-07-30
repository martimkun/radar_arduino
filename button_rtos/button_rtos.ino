#include "Arduino_FreeRTOS.h"
#include "task.h"
#include "semphr.h"
 
#include <math.h>

#include <Ultrasonic.h>
#include <Servo.h>
 
#define ULTR_TRIGGER 4
#define ULTR_ECHO 5
#define ENGINE_PWM 9
#define INIT_ANGLE 0
#define END_ANGLE 90
#define PIN_POTEN A0
#define BUTTON_PIN 7
#define LED_ON 12
#define LED_OFF 13

/* LED pin */
byte ledPin = 13;
/* pin that is attached to interrupt */
byte interruptPin = 7;
/* hold the state of LED when toggling */
volatile byte state = LOW;

void setup() {
  pinMode(ledPin, OUTPUT);
  /* set the interrupt pin as input pullup*/
  pinMode(interruptPin, INPUT_PULLUP);
  /* attach interrupt to the pin
  function blink will be invoked when interrupt occurs
  interrupt occurs whenever the pin change value */
  attachInterrupt(digitalPinToInterrupt(interruptPin), blink, CHANGE);
}

void loop() {
}

/* interrupt function toggle the LED */
void blink() {
  state = !state;
  digitalWrite(ledPin, state);
}


