// Código exemplo para teste de Micro Servo Motor SG90
// Movimento do servo através de potenciômetro

#include <Servo.h> // Inclui a Biblioteca Servo.h

Servo meuservo; // Inicializa o servo no modo de teste
int angulo = 0; // Ajusta o ângulo inicial do Servo
int potencio = A0; // Inicializa o pino analógico para o potenciômetro

void setup() { 
meuservo.attach(9); // Define que o Servo está conectado a Porta 9
} 

void loop() {
angulo = analogRead(potencio); // Faz a leitura do valor do potenciômetro
angulo = map(angulo, 0, 1023, 0, 179); // Associa o valor do potenciômetro ao valor do ângulo
meuservo.write(angulo); // Comando para posicionar o servo no ângulo especificado
delay(5);
} 