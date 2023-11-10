#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <LoRa.h>
//Libs---------------------------------

//variables and defines----------------------------
#define ONE_WIRE_BUS 6
#define colums 16
#define rows 2
#define address 0x27
#define BAND 433E6    //Define a frequência do LoRa. 433E6 é a mesma coisa de 433000000MHz. Você também pode usar 868E6 e 915E6.
#define PABOOST true  //Sem conhecimento dessa variavel mas ela deve aparecer para funcionar
#define turbidity_pin  A3

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

double calc_NTU(double volt);
double NTU = 0.0;

float calibration_value = 21.34 + 1;
unsigned long int avgValue;  //Store the average value of the sensor feedback
int buf[10], temp;
 

const byte phpin = A0; // ph pin
float voltage, temperature;
LiquidCrystal_I2C lcd(address, colums, rows); // lcd instância

///Code----------------------------------------------------------
void setup() {
  pinMode(turbidity_pin, INPUT);
  pinMode(phpin, INPUT); 

  Serial.begin(9600);
  lcd.init();
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setBacklight(HIGH);
  lcd.setCursor(0,0);
  lcd.print("Iniciando....");
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
    lcd.println("Lora Offline");
    while (1);
  }
  lcd.clear();
  lcd.println("loRa iniciado!");
  delay(1000);
}

void loop() {
  //turbidity
  int sensorValue = analogRead(turbidity_pin); 
  float voltage = sensorValue * (5.0 / 1024.0);  

  NTU = calc_NTU(voltage);
  
  //ph
  for(int i=0;i<10;i++){ //Get 10 sample value from the sensor for smooth the value
    buf[i]=analogRead(phpin);
    delay(10);
  }
  for(int i=0;i<9;i++){ //sort the analog from small to large
    for(int j=i+1;j<10;j++){
      if(buf[i]>buf[j]){
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;

  for(int i=2;i<8;i++)avgValue+=buf[i];  //take the average value of 6 center sample
     
  float phValue=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  phValue = -5.70 * phValue + calibration_value; //convert the millivolt into pH value
  // temperature
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
  delay(700);

  lcd.setCursor(0,0);
  lcd.print("TMP:");
  lcd.print(temp,2);
  lcd.println("       ");



  lcd.setCursor(0,1);
  lcd.print("PH:");
  lcd.print(phValue, 2);


  lcd.setCursor(9,1);
  lcd.print("TU:");
  lcd.print(NTU, 4);

  delay(700);

  loraSender(sensors.getTempCByIndex(0), phValue,NTU); // envia os dados pelo lora

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

double calc_NTU(double volt)
{
  double NTU_val;
  NTU_val = -(1120.4*volt*volt)+(5742.3*volt)-4352.9;
  return NTU_val;
} 

