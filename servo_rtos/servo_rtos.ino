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
#define BUTTON_PIN 2
#define LED_ON 12
#define LED_OFF 13

enum States {STATE_ZERO = 0, STATE_ONE};
enum States state = STATE_ZERO;
volatile bool running = false;
 
//Tasks
TaskHandle_t readSensorTaskH;
TaskHandle_t servoTaskH;
TaskHandle_t buttonTaskH;

//Mutex
//SemaphoreHandle_t SerialMutex;



Ultrasonic ultrasonic(ULTR_TRIGGER, ULTR_ECHO);
 
void setup() {
    //Inicializa Serial
    Serial.begin(9600);
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_ON, OUTPUT);
    pinMode(LED_OFF, OUTPUT);
     
    //Cria tarefa readSensorTask
    xTaskCreate(readSensorTask,            //Funcao
                "readSensorTask",          //Nome
                128,                //Pilha
                NULL,               //Parametro
                1,                  //Prioridade
                &readSensorTaskH);

    //Cria tarefa ledTask
    xTaskCreate(servoTask,            //Funcao
                "servoTask",          //Nome
                128,                //Pilha
                NULL,               //Parametro
                1,                  //Prioridade
                &servoTaskH);

    //Cria tarefa buttonTask
    xTaskCreate(buttonTask,            //Funcao
                "buttonTask",          //Nome
                128,                //Pilha
                NULL,               //Parametro
                2,                  //Prioridade
                &buttonTaskH);
}
 
void loop() {
    // Nada é feito aqui, Todas as funções são feitas em Tasks
}
 
/* readSensorTask
 *  Leitura sensor Ultrassonico
 */
void readSensorTask(void *arg) {

    static int distance;

    while(1){
        if(state == STATE_ONE || running == true){
          distance = ultrasonic.read();
          Serial.println("Distance in CM: " + String(distance));
          vTaskDelay(10); // 1s

        } else {
          vTaskDelay(100);
        }
    }

    //O codigo nunca deve chegar aqui
    vTaskDelete(NULL);      //Deleta a Task atual
}

/* servoTask
 *  Controle motor servo
 */
void servoTask(void *arg) {

    Servo servo;
    servo.attach(ENGINE_PWM);
    static int position;
    servo.write(0);
    static int speed = 1;
    static int count = 2;

    while(1){

      if(state == STATE_ONE && count != 0) {
        running = true;
        for(position = INIT_ANGLE; position < END_ANGLE; position++)
        {
            servo.write(position);
            speed = analogRead(PIN_POTEN);
            speed = map(speed, 1, 1023, 1, 30);
            vTaskDelay(speed);
        }

        vTaskDelay(1);
        for(position = END_ANGLE; position >= INIT_ANGLE; position--)
        {
            servo.write(position);
            speed = analogRead(PIN_POTEN);
            speed = map(speed, 1, 1023, 1, 30);
            vTaskDelay(speed);
        }

        count--;
      }
      else {
        state = STATE_ZERO;
        running = false;
        count = 2;
        vTaskDelay(300);
      }

    }

    //O codigo nunca deve chegar aqui
    vTaskDelete(NULL);      //Deleta a Task atual
}

void buttonTask(void *arg){
  static bool button = digitalRead(BUTTON_PIN);
  static bool ledOn, ledOff, lastButton;

  while(1){
    lastButton = button;
    button = digitalRead(BUTTON_PIN);

    Serial.println("State: " + String(state));
    Serial.println("Button: " + String(button));

    switch(state) {
      case STATE_ZERO:
        if(button == 1){
          ledOn = HIGH;
          digitalWrite(LED_ON, ledOn);
          ledOff = LOW;
          digitalWrite(LED_ON, ledOff);
          state = STATE_ONE;      
        }
        button = digitalRead(BUTTON_PIN);

        break;

      case STATE_ONE:
        ledOn = LOW;
        digitalWrite(LED_ON, ledOn);
        ledOff = HIGH;
        digitalWrite(LED_ON, ledOff);
        button = digitalRead(BUTTON_PIN);
        break;
      default:
      break;
    }

    vTaskDelay(250);
  }

  //O codigo nunca deve chegar aqui
    vTaskDelete(NULL);      //Deleta a Task atual
}


