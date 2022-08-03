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
#define END_ANGLE 180
#define PIN_POTEN A0
#define BUTTON 2
#define LED_ON 12
#define LED_RUNNING 11

enum States { STATE_ZERO = 0,
              STATE_ONE };
enum States state = STATE_ZERO;
volatile bool running = false;
volatile int max_dist = 0;
volatile int min_dist = 999;

//Tasks
TaskHandle_t readSensorTaskH;
TaskHandle_t servoTaskH;
TaskHandle_t buttonTaskH;
TaskHandle_t logTaskH;

// System apis
SemaphoreHandle_t semServo;
SemaphoreHandle_t semLog;

// Declaração sensor Ultrasônico
Ultrasonic ultrasonic(ULTR_TRIGGER, ULTR_ECHO);

void setup() {
  // Inicializa Serial
  Serial.begin(9600);

  // Setup inputs e outputs
  pinMode(BUTTON, INPUT);
  pinMode(LED_ON, OUTPUT);
  pinMode(LED_RUNNING, OUTPUT);

  // Garantindo que os leds estarão desligados
  digitalWrite(LED_ON, LOW);
  digitalWrite(LED_RUNNING, LOW);

  while (!Serial) {
    ;  // wait for serial port to connect.
  }

  semServo = xSemaphoreCreateBinary();

  if (semServo == NULL) {
    Serial.println("Erro ao criar o semaforo do Servomotor");
  }

  semLog = xSemaphoreCreateBinary();

  if (semLog == NULL) {
    Serial.println("Erro ao criar o semaforo de Log");
  }

  //Cria tarefa readSensorTask
  xTaskCreate(readSensorTask,    //Funcao
              "readSensorTask",  //Nome
              128,               //Pilha
              NULL,              //Parametro
              1,                 //Prioridade
              &readSensorTaskH);

  //Cria tarefa ledTask
  xTaskCreate(servoTask,    //Funcao
              "servoTask",  //Nome
              128,          //Pilha
              NULL,         //Parametro
              1,            //Prioridade
              &servoTaskH);

  //Cria tarefa buttonTask
  xTaskCreate(buttonTask,    //Funcao
              "buttonTask",  //Nome
              128,           //Pilha
              NULL,          //Parametro
              2,             //Prioridade
              &buttonTaskH);

  //Cria tarefa logTask
  xTaskCreate(logTask,    //Funcao
              "logTask",  //Nome
              128,           //Pilha
              NULL,          //Parametro
              2,             //Prioridade
              &logTaskH);
}

void loop() {
  // Nada é feito aqui, Todas as funções são feitas em Tasks
}

/* readSensorTask
 *  Leitura sensor Ultrassonico
 */
void readSensorTask(void *arg) {

  static int distance;

  while (1) {
    if (state == STATE_ONE) {
      distance = ultrasonic.read();
      if(distance > max_dist){
        max_dist = distance;
      }
      
      if(distance < min_dist){
        min_dist = distance;
      }
      //Serial.println("Distance in CM: " + String(distance));
      vTaskDelay(10);  // 1s

    } else {
      vTaskDelay(100);
    }
  }

  //O codigo nunca deve chegar aqui
  vTaskDelete(NULL);  //Deleta a Task atual
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

  while (1) {

    //if (state == STATE_ONE && count != 0) {
    if (xSemaphoreTake(semServo, portMAX_DELAY)) {
      running = true;

      while (count != 0) {
        for (position = INIT_ANGLE; position < END_ANGLE; position++) {
          servo.write(position);
          speed = analogRead(PIN_POTEN);
          speed = map(speed, 1, 1023, 1, 30);
          vTaskDelay(speed);
        }

        vTaskDelay(1);
        for (position = END_ANGLE; position >= INIT_ANGLE; position--) {
          servo.write(position);
          speed = analogRead(PIN_POTEN);
          speed = map(speed, 1, 1023, 1, 30);
          vTaskDelay(speed);
        }

        count--;
      }

      state = STATE_ZERO;
      digitalWrite(LED_RUNNING, LOW);
      running = false;
      count = 2;
      vTaskDelay(300);
      xSemaphoreGive(semLog);
    }
     /* else {
      state = STATE_ZERO;
      running = false;
      count = 2;
      vTaskDelay(300);
    } */
  }

  //O codigo nunca deve chegar aqui
  vTaskDelete(NULL);  //Deleta a Task atual
}

/* buttonTask
 *  Controle do sistema através do botão
 */
void buttonTask(void *arg) {
  static bool button = digitalRead(BUTTON);
  static bool lastButton;

  // Sistema pronto para iniciar
  digitalWrite(LED_ON, HIGH);

  while (1) {
    lastButton = button;
    button = digitalRead(BUTTON);

    //Serial.println("State: " + String(state));
    //Serial.println("Button: " + String(button));

    switch (state) {
      case STATE_ZERO:
        digitalWrite(LED_RUNNING, LOW);

        if (button == HIGH && button != lastButton) {
          state = STATE_ONE;
          digitalWrite(LED_RUNNING, HIGH);

          xSemaphoreGive(semServo);
          
          //vTaskResume(readSensorTaskH);
          //vTaskResume(servoTaskH);
        }

        break;

      case STATE_ONE:
        if (button == LOW && !running) {
          state = STATE_ZERO;
          digitalWrite(LED_RUNNING, LOW);
          //vTaskSuspend(readSensorTaskH);
          //vTaskSuspend(servoTaskH);
        }

        break;

      default:
        break;
    }

    vTaskDelay(100);
  }

  //O codigo nunca deve chegar aqui
  vTaskDelete(NULL);  //Deleta a Task atual
}

void logTask(void *arg){

    while (1) {
        if(xSemaphoreTake(semLog, portMAX_DELAY)){       // checa se semáforo está disponível
            Serial.println("max"+String(max_dist)+";min"+String(min_dist));
            max_dist = 0;
            min_dist = 999;
        }
        vTaskDelay(300);
    }
    vTaskDelete(NULL);
}