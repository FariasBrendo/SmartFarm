/*********
  Complete project details at https://randomnerdtutorials.com
*********/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>

/*#include <SPI.h>
  #define BME_SCK 14
  #define BME_MISO 12
  #define BME_MOSI 13
  #define BME_CS 15*/

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

unsigned long delayTime;

WiFiClient client;

//API KEY do seu ThingSpeak
String apiKey = "C7V222G351BAPP3U";

//wifi e senha
char ssid[] = "CDSA";
char pass[] = "ufcg-cdsa";

const char* server = "api.thingspeak.com"; //Não mudar

//Parte da função millis
long previousMillis = 0;
long notificacoes = 60000;
//Parte da função millis p notificações
//variaveis auxiliares para calcular media
int numeroDeMedicoes = 0;
double TempArAcum = 0;
double UmiArAcum = 0;

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, pass);
  Serial.println(F("BME280 test"));

  bool status;

  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  status = bme.begin(0x76);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  Serial.println("-- Default Test --");
  delayTime = 1000;
  Serial.println();

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado ");
  //salva hora atual
  previousMillis = millis();


}

void loop() {
  Serial.println("");

  Serial.print("Umidade : ");
  double h = bme.readHumidity();
  UmiArAcum += h;
  Serial.print(h);
  Serial.println(" %");

  Serial.print("Temperatura : ");
  double t = bme.readTemperature();
  TempArAcum += t;
  Serial.print(t);
  Serial.println(" *C");

  Serial.print("Pressao : ");
  double p = bme.readPressure() / 100.0F;
  Serial.print(p);
  Serial.println(" hPa");

  Serial.print("Altitude Aproximada : ");
  double a = bme.readAltitude(SEALEVELPRESSURE_HPA);
  Serial.print(a);
  Serial.println(" m");

  // float pluv();

  Serial.println();

  //incrementa n. de medicoes
  numeroDeMedicoes++;

  //Notificações e Envio
  unsigned long currentMillis = millis();    //Tempo atual em ms
  double deltaT = currentMillis - previousMillis;

  //Lógica de verificação do tempo
  if (deltaT > notificacoes) {
    previousMillis = currentMillis;    // Salva o tempo atual

    //Cálculo das médias
    double mediaTempAr = TempArAcum / numeroDeMedicoes;
    double mediaUmiAr = UmiArAcum / numeroDeMedicoes;

    Serial.print ("Média Temperatura do ar:");
    Serial.println(mediaTempAr);

    Serial.print ("Média Umidade do ar:");
    Serial.println (mediaUmiAr);


    //Cálculo das equações
    double hp = mediaUmiAr / 100; //percentual de umidade
    double TGN = (20.4156 + (0.544 * mediaTempAr) - (.0901 * hp));
    double TPO = (mediaTempAr - ((100 - hp) / 5));
    double ITU = (((mediaTempAr + 273) + 0.36 * (TPO + 273)) - 330.08);
    Serial.print("ITU: ");
    Serial.println(ITU);
    double ITGU = ((TGN + 273) + (0.36 * (TPO + 273)) - 330.08);
    Serial.print("ITGU: ");
    Serial.println(ITGU);


    if (client.connect(server, 80))  //   "184.106.153.149" or api.thingspeak.com
    {

      String postStr = apiKey;
      postStr += "&field1=";
      postStr += String(mediaTempAr);
      postStr += "&field2=";
      postStr += String(mediaUmiAr);
     postStr += "&field3=";
      postStr += String(ITU);
      postStr += "&field4=";
      postStr += String(ITGU);
      // postStr +="&field5=";
      // postStr += String(REEDCOUNT2);
      postStr += "\r\n\r\n";

      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(postStr.length());
      client.print("\n\n");
      client.print(postStr);

      Serial.println("Enviando para o THingspeak.");
    }
    UmiArAcum = 0;
    TempArAcum = 0;
    numeroDeMedicoes = 0;
  }
  delay(10000);

}
