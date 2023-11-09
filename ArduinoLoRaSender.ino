/*
 -----------------------------------------------#Description-----------------------------------------------------------------------------------------------------------------------------------------------
 This is an example of data transmission through radio communication using LoRa (Long Range) and displaying it on both the OLED display and a web application developed based on a server created on Esp32
 Este é um exemplo de envio de dados em comunicação via rádio usando LoRa (Longa Distância) e mostrar tanto no display Oled quanto numa aplicação web desenvolvida com base num servidor criado no Esp32
 
 Developed by Arison Reis
 Desenvolvido por Arison Reis
 ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 
 ----------------------------#Sensors-----------------------------------
 Sensors used in the project:
 Sensores ultilizados no projeto:

 PH:
 en - PH Meter https://www.amazon.com.br/Module-PH0-14-Detect-Sensor-Electrode/dp/B095W2R3BS
 pt - Medidor de PH  https://produto.mercadolivre.com.br/MLB-4016196352-valor-de-ph-do-modulo-de-sensor-de-aquisico-de-sonda-de-ele-_JM#is_advertising=true&position=3&search_layout=grid&type=pad&tracking_id=473dda6a-7cf7-46cc-af97-5281125c70a4&is_advertising=true&ad_domain=VQCATCORE_LST&ad_position=3&ad_click_id=MGFhOGEzMTQtNDg3Yi00ODZmLTgyNTAtZDI3ZjMzN2JhYWI

 Temperature Meter: DS18B20 
 Medidor de temperatura: DS18B20
 ----------------------------------------------------------------------

 --------------------------#Repo-----------------------------------------
 My Repo: / Meu Repositório:
 $ https://github.com/arisonreis
 
 Project repo: / Repositório do projeto:
 $ https://github.com/arisonreis/MicroControllers-Radio-Communication
 ----------------------------------------------------------------------

 Controller:
 ------------------ 
 Arduino -  Sender |
 Arduino -  Sender |
 ------------------
*/

//-----------------------------#Code ------------------------------------------------
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <LoRa.h>
//Libs---------------------------------

#define ONE_WIRE_BUS 6
#define colums 16
#define rows 2
#define address 0x27
#define BAND 433E6    //Define a frequência do LoRa. 433E6 é a mesma coisa de 433000000MHz. Você também pode usar 868E6 e 915E6.
#define PABOOST true  //Sem conhecimento dessa variavel mas ela deve aparecer para funcionar

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int sensorPin = A3; // Atribui a porta A3 a variavel sensorPin -> turbidez
int leitura; //Variável responsável por guardar o valor da leitura analógica do sensor de turbidez

float Po; // ph output value
const byte phpin = A0; // ph pin
float voltage, temperature;
LiquidCrystal_I2C lcd(address, colums, rows); // lcd instância

///Code----------------------------------------------------------
void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setBacklight(HIGH);
  lcd.setCursor(0,0);
  lcd.print("Iniciando o Sistema....");
  delay(1000);
  lcd.clear();
  //loRa
  while (!Serial);
  lcd.clear();
  lcd.println("Iniciando LoRa");
  delay(1000);
  
  if (!LoRa.begin(BAND)) { // Frequencia de operação (ou 915E6)
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.println("Falha em iniciar"); // se a placa não for reconhecida
    lcd.setCursor(0, 1);
    lcd.println("Lora desconectado");
    while (1);
  }
  lcd.clear();
  lcd.println("loRa iniciado!");
  delay(1000);
}

void loop() {
  Po =  (1023 - analogRead(phpin)) / 73.07; //Realiza a leitura analógica do sensor ph
  sensors.requestTemperatures(); //Realiza a leitura analógica do sensor de temperatura
  leitura = analogRead(sensorPin); //Realiza a leitura analógica do sensor de turbidez
  loraSender(sensors.getTempCByIndex(0), Po,leitura); // envia os dados pelo lora
  delay(100);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("temperatura:");
  lcd.println(sensors.getTempCByIndex(0));
  lcd.setCursor(0,1);
  lcd.print("PH:");
  lcd.println(Po, 2);
  delay(1000);
  lcd.clear();

  // turbidez
  Serial.print("Valor lido: "); //Imprime no monitor serial
  Serial.println(leitura); // manda o valor de leitura para o monitor serial
  delay(1000); //Intervalo de 0,5 segundos entre as leituras
  if (leitura > 700) { //Se o valor de leitura analógica estiver acima de 700
    lcd.clear();
    lcd.println("AGUA: ");
    lcd.println("LIMPA"); //Imprime no monitor serial que a água está limpa
    delay(1000);
  }
  if ((leitura > 600) && (leitura <= 700)) { //Se o valor de leitura analógica estiver entre 600 e 700
    lcd.clear();
    lcd.print("AGUA:");
    lcd.println("SUJA"); //Imprime no monitor serial que a água está um pouco suja
    delay(1000);
  }
  if (leitura < 600) { //Se o valor de leitura analógica estiver abaixo de 600 
    lcd.clear();
    lcd.print("AGUA: ");
    lcd.println("MUITO SUJA"); //Imprime no monitor serial que a água está muito pouco suja
    delay(1000);
  }
}

void loraSender (float temp, float ph, float turbidez){
  struct SensorsData {
    float ph;
    float turbidez;
    float temperature;
  } sensorsData;

  sensorsData.temperature = temp;
  sensorsData.ph =  ph;
  sensorsData.turbidez = turbidez;
  LoRa.beginPacket();
  LoRa.write((uint8_t*)&sensorsData, sizeof(sensorsData));
  LoRa.endPacket();
  delay(100);
}
