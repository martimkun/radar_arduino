#include "Arduino_FreeRTOS.h"
#include "task.h"
#include "semphr.h"
 
#include <math.h>

#include <Ultrasonic.h>
 
#define ULTR_01  12
#define ULTR_02  13
 
//Tasks
TaskHandle_t        readSensorTaskH;
 
//Mutex
SemaphoreHandle_t   SerialMutex;

Ultrasonic ultrasonic(ULTR_01, ULTR_02);
 
// void sendGantt(const char *name, unsigned int stime, unsigned int etime) {
//     if(xSemaphoreTake(SerialMutex, portMAX_DELAY) == pdTRUE) {  //Solicita Mutex
//         Serial.print("\t\t");
//         Serial.print(name);
//         Serial.print(": ");
//         Serial.print(stime);
//         Serial.print(", ");
//         Serial.println(etime);
//         xSemaphoreGive(SerialMutex);                            //Libera Mutex
//     }
// }
 
void setup() {
    //Inicializa Serial
    Serial.begin(9600);
    // Serial.print("1s is ");
    // Serial.print(configTICK_RATE_HZ);
    // Serial.print(" ticks at ");
    // Serial.print(F_CPU);
    // Serial.print(" Hz\n\n");
    // #if (defined(SEND_GANTT) && (SEND_GANTT==1))
    //     Serial.println("gantt\n\tdateFormat x\n\ttitle A gant diagram");
    // #endif
     
    // SerialMutex = xSemaphoreCreateMutex();
     
    //Cria tarefa ledTask
    xTaskCreate(readSensorTask,            //Funcao
                "readSensorTask",          //Nome
                128,                //Pilha
                NULL,               //Parametro
                1,                  //Prioridade
                &readSensorTaskH);  
}
 
void loop() {
    // Nada é feito aqui, Todas as funções são feitas em Tasks
}
 
/* readSensorTask
 *  Leitura sensor Ultrassonico
 */
void readSensorTask(void *arg) {

    while(1){
        distance = ultrasonic.read();
        Serial.println("Distance in CM: " + String(distance));
        vTaskDelay(1000); // 1s
    }

    //O codigo nunca deve chegar aqui
    vTaskDelete(NULL);      //Deleta a Task atual
}