/**
 * @file radar_rtos_v2.ino
 * @author martim, michael (https://github.com/martimkun/radar_arduino)
 * @brief radar com sensor de distancia ultrassonico usando arduino
 * @version 0.1
 * @date 2022-08-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */

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
              STATE_ONE };      /// Estados possiveis, zero = nao operante ; um = operando
enum States state = STATE_ZERO; /// Inicia variavel de estado no estado zero
volatile bool running = false;  /// Variavel que indica se esta rodando a leitura das distancias
volatile int max_dist = 0;      /// Instancia variavel para distancia maxima com valor baixo
volatile int min_dist = 999;    /// INstancia variavel para distancia minima com valor alto

/// Tarefas
TaskHandle_t readSensorTaskH;   /// Cria handler para a tarefa de ler o sensor
TaskHandle_t servoTaskH;        /// Cria handler para a tarefa de controlar o motor
TaskHandle_t buttonTaskH;       /// Cria handler para a tarefa de ler o botao
TaskHandle_t logTaskH;          /// Cria handler para a tarefa de criar o log das distancias

/// Semaforos
SemaphoreHandle_t semServo;     /// Cria handler para o semaforo do motor
SemaphoreHandle_t semLog;       /// Cria handler para o semaforo do log

/// Declaração sensor Ultrasônico
Ultrasonic ultrasonic(ULTR_TRIGGER, ULTR_ECHO);   /// Instancia o sensor como um objeto linkando as portas designadas a ele

/**
 * @brief funcao de setup do sistema, define variaveis e tarefas
 * 
 */
void setup() {
  
  Serial.begin(9600); /// Inicializa Serial

  /// Setup inputs e outputs
  pinMode(BUTTON, INPUT);
  pinMode(LED_ON, OUTPUT);
  pinMode(LED_RUNNING, OUTPUT);

  /// Garante que os leds estarão desligados
  digitalWrite(LED_ON, LOW);
  digitalWrite(LED_RUNNING, LOW);

  while (!Serial) {
    ;  /// Espera até a porta serial conectar
  }

  semServo = xSemaphoreCreateBinary();    /// Cria o semaforo para o servo motor

  if (semServo == NULL) {
    Serial.println("Erro ao criar o semaforo do Servo motor"); /// Printa mensagem de erro ao criar semaforo do servo motor
  }

  semLog = xSemaphoreCreateBinary();      /// Cria o semaforo para o servo motor

  if (semLog == NULL) {
    Serial.println("Erro ao criar o semaforo de Log"); /// Printa mensagem de erro ao criar semaforo do log
  }

  ///Cria tarefa readSensorTask
  xTaskCreate(readSensorTask,    
              "readSensorTask",  
              128,               
              NULL,              
              1,                 
              &readSensorTaskH);

  ///Cria tarefa ledTask
  xTaskCreate(servoTask,    
              "servoTask",  
              128,          
              NULL,         
              1,            
              &servoTaskH);

  ///Cria tarefa buttonTask
  xTaskCreate(buttonTask,    
              "buttonTask",  
              128,           
              NULL,          
              2,             
              &buttonTaskH);

  ///Cria tarefa logTask
  xTaskCreate(logTask,   
              "logTask",  
              128,           
              NULL,          
              2,             
              &logTaskH);
}

/**
 * @brief funcao de loop do arduino
 * 
 */
void loop() {
  /// Nada é feito aqui, Todas as funções são feitas em Tasks e não no loop do arduino
}


/**
 * @brief Leitura sensor Ultrassonico
 * 
 * @param arg 
 */
void readSensorTask(void *arg) {

  static int distance;      /// Instancia variavel para medir a distancia

  /**
   * @brief testa se o estado = 1 e se for, faz a leitura do sensor e compara as distancias maximas e minimas, caso nao seja estado um, espera e verifica novamente
   * 
   */
  while (1) {
    if (state == STATE_ONE) {
      distance = ultrasonic.read();     /// Le o dado do sensor
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
  vTaskDelete(NULL);  /// Deleta a Task atual
}


/**
 * @brief Controle do motor servo motor
 * 
 * @param arg 
 */
void servoTask(void *arg) {

  Servo servo;                    /// instancia a variavel de objeto do servo motor
  servo.attach(ENGINE_PWM);       /// anexa o pino designado ao servo motor
  static int position;            /// define a variavel para a companhar a posicao do motor
  servo.write(0);                 /// redefine o valor de posicao para a inicial
  static int speed = 1;
  static int count = 1;

  /**
   * @brief verifica se semaforo esta disponivel, se sim começa a rotacao do motor ate 180 graus e volta novamente
   * 
   */
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
  }

  //O codigo nunca deve chegar aqui
  vTaskDelete(NULL);  //Deleta a Task atual
}

/* buttonTask
 *  
 */

/**
 * @brief Faz a leitura e controle do sistema atraves do botão
 * 
 * @param arg 
 */
void buttonTask(void *arg) {
  static bool button = digitalRead(BUTTON); /// Le estado atual do botao
  static bool lastButton;                   /// variavel que guarda o estado anterior do botao

  digitalWrite(LED_ON, HIGH);   /// Led indicando que o sistema esta pronto para iniciar

  /**
   * @brief guarda o estado anterior do botao, le o estado atual, se atinge os criterios para ligar o radar,
   * ele da o semaforo ao servo motor e inicia a leitura das distancias
   * 
   */
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


/**
 * @brief Faz o print dos dados de indice da captura e das distancias maximas e minimas para a porta serial
 * 
 * @param arg 
 */
void logTask(void *arg){

  /**
   * @brief verifica se semaforo esta disponivel para si e printa as informacoes na porta serial, apos,
   * reescreve os dados de distancia maxima e minima para a proxima execucao
   * 
   */
  while (1) {
      if(xSemaphoreTake(semLog, portMAX_DELAY)){       
          Serial.println("max"+String(max_dist)+";min"+String(min_dist));
          max_dist = 0;
          min_dist = 999;
      }
      vTaskDelay(300);
  }
    
  vTaskDelete(NULL);
}