////////////////////////////////////////////////////////////////////
//Carrega bibliotecas
#include <OneWire.h>    //Biblioteca usada pelo termometro DS18B20
#include <DallasTemperature.h>  //Biblioteca usada pelo termometro DS18B20
#include <jled.h>       //Controle de LEDs
#include <Arduino.h>    //Display
#include <TM1637Display.h>  //Display
////////////////////////////////////////////////////////////////////
//Inicia constantes
#define tempo_fermentacao 300     //tempo limite de funcionamento do motor para fermentação
#define tempo_maturacao 3600      //tempo limite de funcionamento do motor para maturação
#define tempo_descanso 180        //tempo que o motor fica parado
#define temperatura_limite 10     //temperatura que define se é maturação ou fermentação
#define tempo_ciclo 120            //tempo entre cada ciclo de controle de temperatura
#define margem 0.1                //Margem aceitavel para variação de temperatura
#define tempo_delay 300           //Tempo entre loops em milisegundos
#define tempo_delay_mil 0.3       //Tempo entre loops em segundos
#define constante_de_calibragem 1.5 //Temperatura para calibrar LM-35
////////////////////////////////////////////////////////////////////
//Inicia variáveis
float temperatura_ideal = 3.5;
float temperatura_DS;      //Temperatura do DS18B20
int tempo_timer = 5;                 //Tempo que display fica ligado sem interação, em segundos
int tempo_motor = tempo_fermentacao;  //Variável que determina se motor funciona ou descansa
////////////////////////////////////////////////////////////////////
//Variáveis auxiliares
bool DS_LM = 0;
float tempo = tempo_ciclo + 1;
bool led_on = 1;              //Controla se LED começa funcionando
////////////////////////////////////////////////////////////////////
//Define portas
#define botao_inicio 7     // botao que liga display
#define botao_sobe 4     // botao que desce
#define botao_desce 3     // botao que sobe
#define rele 8  //Porta do rele
#define porta_LED 12  //Porta do LED
////////////////////////////////////////////////////////////////////
//Inicia Display TM1637
#define CLK 10   //Porta do clock
#define DIO 11  //Porta dos dados
TM1637Display display(CLK, DIO);
const uint8_t celsius[] = {
SEG_A | SEG_D | SEG_E | SEG_F // C
};
////////////////////////////////////////////////////////////////////
//Inicia driver do LED
JLed led = JLed(porta_LED).Off();
////////////////////////////////////////////////////////////////////
//Inicia DS18B20
#define chipSelect 4
#define ONE_WIRE_BUS 2      //Porta de dados
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
////////////////////////////////////////////////////////////////////
//Setup
void setup() {
  //Serial.begin(9600);   //Para debug
  pinMode(porta_LED, OUTPUT);
  pinMode(rele, OUTPUT);
  pinMode(botao_inicio, INPUT_PULLUP);
  pinMode(botao_sobe, INPUT_PULLUP);
  pinMode(botao_desce, INPUT_PULLUP);
  digitalWrite(rele, HIGH);   //Inicia rele de forma a desligar geladeira
  sensors.begin();        //Inicia sensor DS18B20
}
////////////////////////////////////////////////////////////////////
//Main
void loop() {
  if(digitalRead(botao_inicio)) {          //Testa se botão inicio foi precionado e entra no modo ativo
      configuracao();
  }
  if(tempo > tempo_ciclo) {                //A cada minuto testa temperatura, e faz as operações necessárias
    controle_temperatura();
    if(led_on) {
      controla_LED();
    }
    tempo = 0;                            //Reseta tempo
  }
  tempo = tempo + tempo_delay_mil;                    //Clock do programa
  led.Update();                           //Driver do LED atualiza estado
  delay(tempo_delay);
}
////////////////////////////////////////////////////////////////////
//Controle da temperatura
void controle_temperatura() {
  atualiza_termometros();
  if(temperatura_DS >= (temperatura_ideal + margem)) {      //Liga motor se necessário
    gela();
  }
  else {
    digitalWrite(rele, HIGH);     //Desliga motor se temperatura permitir
    reseta_tempo();               //Deixa motor preparado pra iniciar
  }
}
////////////////////////////////////////////////////////////////////
//Gela
void gela() {
  if(tempo_motor > 0) {         //Se o tempo do motor permitir, liga o motor para gelar
    digitalWrite(rele, LOW);
    tempo_motor = tempo_motor - tempo_ciclo;
  }
  else {
    if((tempo_motor + tempo_descanso) > 0) {      //Continua baixando até a soma dos 2 ser menor que 0
      digitalWrite(rele, HIGH);
      tempo_motor = tempo_motor - tempo_ciclo;
    }
    else {                    //Tempo de descanso acabou, pode ligar o motor
      reseta_tempo();
    }
  }
}
////////////////////////////////////////////////////////////////////
//Controle do LED
void controla_LED() {
   if((temperatura_DS <= (temperatura_ideal + 0.5)) && (temperatura_DS >= (temperatura_ideal - 0.5))) {
    led = JLed(porta_LED).Off();
   }
   else {
    if(temperatura_DS > (temperatura_ideal + 0.5)) {        //Se temperatura estiver muito alta, liga LED
      led = JLed(porta_LED).On();
    }
    else {
     if(temperatura_DS < temperatura_ideal - 0.5 && temperatura_DS > temperatura_ideal - 1)      //Se temperatura estiver muito baixa, pisca LED
     {
       led = JLed(porta_LED).Blink(1300, 1300).Forever();
     }
     else {                                            //Se temperatura estiver muito menor, pisca rápido
       led = JLed(porta_LED).Blink(300, 300).Forever();
     }
    }
   }
}
////////////////////////////////////////////////////////////////////
//Atualiza termômetros
void atualiza_termometros() {
  sensors.requestTemperatures();
  temperatura_DS = sensors.getTempCByIndex(0);
  //Serial.print(temperatura_DS);
}
////////////////////////////////////////////////////////////////////
//Função display, temperatura e modo
void configuracao() {
  atualiza_termometros();
  mostra(temperatura_DS);
  delay(300);
  DS_LM = 0;
  float timer = tempo_timer;
  while(timer > 0)
  {
    if(digitalRead(botao_inicio)) {          //Testa se botão inicio foi precionado. Se sim, reseta o timer e muda temperatura sendo mostrada
      timer = tempo_timer;
      if(!DS_LM) {
        delay(1500);
        if(digitalRead(botao_inicio)) {
          led_on = !led_on;         //Nesse modo, muda estado do LED
          led = JLed(porta_LED).Off();
          mostra(99.9);
          led.Update();
          delay(1500);
        }                   
        delay(300);
      }
      else {
       mostra(temperatura_DS);
       delay(300);
      }
      DS_LM = !DS_LM;
    }
    if(digitalRead(botao_sobe) || digitalRead(botao_desce)) {          //Testa se botão foi precionado
      set_temperatura();            //Se botão de sobe ou desce é precionado, começa a mudar a temperatura
      timer = 0;
    }
    led.Update();
    delay(10);
    timer = timer - 0.01;
  }
  tempo = tempo_ciclo + 1;
  desliga_display();
}
////////////////////////////////////////////////////////////////////
//Mostra temperatura
void mostra(float mostra_temperatura) {
  mostra_temperatura = mostra_temperatura * 10;
  display.setBrightness(0x0f);
  display.showNumberDecEx(mostra_temperatura, (0x80 >> 1), false, 3);
  display.setSegments(celsius, 1, 3);
}
////////////////////////////////////////////////////////////////////
//Desliga display
void desliga_display() {
  uint8_t data[] = {0x0, 0x0, 0x0, 0x0};
  display.setBrightness(false);
  display.setSegments(data);
}
////////////////////////////////////////////////////////////////////
//Muda temperatura
void set_temperatura() {
  float nova_temp = temperatura_ideal;
  mostra(temperatura_ideal);
  delay(300);
  while(!digitalRead(botao_inicio)){
    if(digitalRead(botao_sobe)) {
     nova_temp = nova_temp + 0.5;
      mostra(nova_temp);
     }
    if(digitalRead(botao_desce)) {
      nova_temp = nova_temp - 0.5;
      mostra(nova_temp);
     }
     led.Update();
     delay(150);
  }
  temperatura_ideal = nova_temp;
  reseta_tempo();
}
////////////////////////////////////////////////////////////////////
//Reseta tempo_motor
void reseta_tempo() {
  if(temperatura_ideal < temperatura_limite) {        //se temperatura for para maturação, pode cair rápido
    tempo_motor = tempo_maturacao;
  }
  else {                             //caso contrário, diminui mais lento para não cair demais
    tempo_motor = tempo_fermentacao;
  }
}
////////////////////////////////////////////////////////////////////
