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
 Esp32 -  RECIVER  |
 Esp32 -  RECEPTOR |
 ------------------
*/

//-----------------------------#Code ------------------------------------------------
#include <SPI.h>      //LoRa Serial
#include <LoRa.h>     //Lora
#include <Wire.h>     //i2c Communication
#include "SSD1306.h"  //Display communication
#include <WiFi.h>     //Wifi
#include <WiFiClient.h> // Wifi client
#include <WiFiAP.h>     // Wifi AP
// Libs / Bibliotecas

// LoRa Pin Set / Setando Pinos Do Lora
#define SCK 5    // GPIO5  -- SX127x's SCK
#define MISO 19  // GPIO19 -- SX127x's MISO
#define MOSI 27  // GPIO27 -- SX127x's MOSI
#define SS 18    // GPIO18 -- SX127x's CS
#define RST 14   // GPIO14 -- SX127x's RESET
#define DI00 26  // GPIO26 -- SX127x's IRQ(Interrupt Request)

//Define a Lora Frequency. 433E6 is 433000000MHz. others frequecies 868E6 and 915E6.
//Define uma frequência para o LoRa. 433E6 é 433000000MHz. Outras frequências 868E6 e 868E6.
#define BAND 433E6   


#define PABOOST true  //PADBOOST

// Networks Credentials
// Credenciais da rede a ser criada.
const char *ssid = "Projeto de pesquisa";
const char *password = "12345678";

//ip
String IP;

//Defines the i2c address of the Oled(0x3c) and the SDA(4) and SCL(15) pins of the ESP32
//Define o endereço do i2c do Oled(0x3c) e os pinos SDA(4) e SCL(15) do ESP32
SSD1306 display(0x3c, 4, 15);  

// Wifi Server Port
// Porta do servidor wifi
WiFiServer server(80);

// Sensors Data Struct
// Struct dos dados dos sensores
struct SensorsData {
  float ph;
  float turbidez;
  float temperature;
} sensorsData;


float ph_value; 
float turbidez_value;      
String temperature_value;

// Setup
void setup() {
  Serial.begin(115200);
  pinMode(16, OUTPUT);    //Define Reset Pin of Oled / Definie o Reset do Oled
  digitalWrite(16, LOW);
  delay(50);
  digitalWrite(16, HIGH);

  display.init();                     //Inicializa o Oled
  display.flipScreenVertically();     //Vira o Display para a vertical
  display.setFont(ArialMT_Plain_10);  //Define o tipo e tamanho da fonte
  delay(1500);
  display.clear();  //Limpa a tela

  display.drawString(0, 0, "Iniciando o sistema...");
  Serial.println("Iniciando o sistema....");
  display.display();
  delay(1500);

  // inicia o servidor
  if (!WiFi.softAP(ssid, password)) {
    display.clear();
    log_e("Soft AP creation failed.");
    while (1)
      ;
  }

  IPAddress myIP = WiFi.softAPIP();
  IP = myIP.toString();
  display.clear();
  display.drawString(0, 0, "IP Address:");
  Serial.println("AP IP Address:" + myIP.toString());
  delay(1500);
  display.drawString(0, 9, myIP.toString());
  delay(1500);
  display.display();
  server.begin();

  // inicia a comunicação loRa
  SPI.begin(SCK, MISO, MOSI, SS);  //Inicializa a comunicação Serial com o LoRa
  LoRa.setPins(SS, RST, DI00);     //Define os pinos que serão utilizados pelo LoRa

  if (!LoRa.begin(BAND, PABOOST)) {                        //Verifica se o LoRa foi iniciado com sussesso
    display.drawString(0, 8, "Falha ao iniciar o LoRa!");  //Seta o X e Y de onde irá imprimir o texto a seguir
    display.display();                                     //Imprime o texto
    while (1)
      ;  //Entra em um While e a execução do programa morre aqui
  }

  display.drawString(0, 20, "LoRA Iniciado com sucesso!");
  Serial.println("LoRa Iniciado com sucesso!");
  display.drawString(0, 30, "Wifi senha:");
  display.drawString(0, 40, password);
  display.display();
  delay(1000);
  display.drawString(0, 50, "Esperando por dados...");
  Serial.println("Esperando por dados...");
  delay(1000);
  display.display();
  LoRa.receive();  //Habilita o LoRa para receber dados
}

void loop() {
  WiFiClient client = server.available();  // listen for incoming clients
  int packetSize = LoRa.parsePacket();     //Declara uma variável que recebe o comando LoRa.parsePacket que verifica se recebeu algum dado
  if (packetSize) {                        //Verifica se a variável recebeu ou não dados, se recebeu faz o comando a seguir
    while (LoRa.available()) {
      // read packet
      LoRa.readBytes((uint8_t *)&sensorsData, sizeof(sensorsData));
      // Imprima os dados no Serial Monitor
      Serial.print("Temperatura: ");
      temperature_value = sensorsData.temperature;
      Serial.println(sensorsData.temperature);
      Serial.print("pH: ");
      ph_value = sensorsData.ph;
      Serial.println(sensorsData.ph);
      Serial.print("Turbidez: ");
      turbidez_value = sensorsData.turbidez;
      Serial.println(sensorsData.turbidez);
    }
    delay(10);
  }

  if (client) {
    display.clear();                                // if you get a client,
    display.drawString(0, 0, "Nova requisição. ");  // print a message out the serial port
    display.drawString(0, 20, "IP Address:");
    display.drawString(0, 30, IP);
    display.display();

    String currentLine = "";      // make a String to hold incoming data from the client
    while (client.connected()) {  // loop while the client's connected
      if (client.available()) {   // if there's bytes to read from the client,
        char c = client.read();   // read a byte, then
        display.drawString(0, 40, String(c));
        display.display();
        if (c == '\n') {  // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.println("<body style=\"width:100vw; height:100vh;\">");
            client.println("<h1 style=\"font-size:4rem;\">PH : ");
            client.println("<span style=\"color:#05ff2f;\">");
            client.println(ph_value);
            client.println("</span>");
            client.println("</h1>");
            client.println("<h1 style=\"font-size:4rem;\">Temperatura:");
            client.println("<span style=\"color:#05ff2f;\">");
            client.println(temperature_value);
            client.println("</span>");
            client.println("</h1>");
            client.println("<h1 style=\"font-size:4rem;\">TURBIDEZ : ");
            client.println("<span style=\"color:#05ff2f;\">");
            client.println(turbidez_value);
            client.println("</span>");
            client.println("</h1>");
            if (turbidez_value > 700) {  //Se o valor de leitura analógica estiver acima de 700
              client.println("<h1 style=\"font-size:4rem; color:#05ff2f;\">AGUA LIMPA!");
              client.println("</h1>");
            }
            if ((turbidez_value > 600) && (turbidez_value <= 700)) {  //Se o valor de leitura analógica estiver entre 600 e 700
              client.println("<h1 style=\"font-size:4rem;color: #dfa11b;\">AGUA SUJA!");
              client.println("</h1>");
            }
            if (turbidez_value < 600 && (turbidez_value >= 1)) {  //Se o valor de leitura analógica estiver abaixo de 600
              client.println("<h1 style=\"font-size:4rem; color:red;\">AGUA MUITO SUJA!");
              client.println("</h1>");
            }
            client.println("<div style=\"display:flex; justify-content:end; padding:0 9rem;\">");
            client.println("<button style=\"background:none;background-color: #00ffb3; font-size: 3rem; font-wight:bold; border: none; padding: 4rem 6rem; border-radius: 3rem;\">");
            client.println("<a href=\"/\"style=\"; text-decoration: none;\">ATUALIZAR</a>");
            client.println("</button>");
            client.println("</div>");
            client.println("</body>");
            client.println();
            // break out of the while loop:
            break;
          } else {  // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // close the connection:
    client.stop();
    display.clear();
    display.drawString(0, 0, "Requisição finalizada.");
    display.drawString(0, 20, "IP Address:");
    display.drawString(0, 30, IP);
    display.display();
  }
}