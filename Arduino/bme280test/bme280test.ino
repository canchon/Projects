/*
Escribir datos:
https://la.mathworks.com/help/thingspeak/bulkwritejsondata.html

Leer datos:
https://la.mathworks.com/help/thingspeak/readdata.html

Channel:
https://thingspeak.mathworks.com/channels/2754061/private_show
*/


#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <time.h>

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;  // I2C

const char *ssid2 = "ELIZABETH";
const char *password2 = "sergio30900410";
// const char *ssid = "CANCHON";
// const char *password = "12345678";
const char *ssid = "USTA_Estudiantes";
const char *password = "#soytomasino";

WiFiClient client;
HTTPClient http;
HTTPClient httpData;
String dataToSend = "";

int TimeWithoutConnection = 50;
int TimeWithoutConnectionPermited = 180000;
bool printMessages = true;
String date = "";

// Configura la zona horaria para tu ubicación
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000;  // Hora estándar de Colombia (GMT-5)
const int daylightOffset_sec = 0;

unsigned long delayTime = 6000;

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;  // time to get serial running
  Serial.println(F("BME280 test"));

  unsigned status;

  // default settings
  status = bme.begin(0x76);
  // You can also pass in a Wire library object like &Wire2
  // status = bme.begin(0x77, &Wire2)

  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    Serial.print("SensorID was: 0x");
    Serial.println(bme.sensorID(), 16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }

  espInit();

  http.begin("https://api.thingspeak.com/channels/2754061/bulk_update.json");

  // Inicializa el servidor NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  Serial.println("-- Default Test --");
  Serial.println();
}

void loop() {
  printValues();
  fetch(String(bme.readTemperature()), String(bme.readPressure() / 100.0F));

  delay(delayTime);
}




void printValues() {
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" °C");

  Serial.print("Pressure = ");

  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.println();
}

//Iniciar la conexión WiFi
void espInit() {
  delay(TimeWithoutConnection);
  WiFi.begin(ssid2, password2);
  if (printMessages)
    Serial.print("Conectando a " + String(ssid2) + " ...");
  bool cambioRed = true;
  while (WiFi.status() != WL_CONNECTED)  // Check for the connection
  {
    TimeWithoutConnection += 500;
    delay(500);
    if (printMessages)
      Serial.print(".");
    if (TimeWithoutConnection > TimeWithoutConnectionPermited)  // Si tarda más de 15 segundos en conectarse a la primera red WiFi que se cambie a la segunda definida.
    {
      WiFi.disconnect(true);
      delay(50);
      if (cambioRed) {
        WiFi.begin(ssid, password);
        if (printMessages)
          Serial.println('Intentando conectar a ' + String(ssid));
        cambioRed = !cambioRed;
      } else {
        WiFi.begin(ssid2, password2);
        if (printMessages)
          Serial.println('Intentando conectar a ' + String(ssid2));
        cambioRed = !cambioRed;
      }
      TimeWithoutConnection = 50;
    }
  }
  if (printMessages)
    Serial.println("Conectado con éxito a una red wifi.");
}

//Escribir los datos en ThingSpeak
String fetch(String temperature, String presion) {
  if (WiFi.status() == WL_CONNECTED)  // Check WiFi connection status
  {
    printLocalTime();

    dataToSend = "{\"write_api_key\":\"JZ7UOPET4NEWE043\",\"updates\":[{\"created_at\":\"" + date + "\",\"field1\":\"" + temperature + "\",\"field2\":\"" + presion + "\"}]}";

    if (printMessages)
      Serial.println("\nDatos a enviar: " + String(dataToSend));

    http.addHeader("Content-Type", "application/json");
    int responseCode = http.POST(dataToSend);  // Enviamos el post pasándole los datos que queremos enviar. retorna un código http

    if (responseCode > 0)  // no hubo errores al hacer la petición
    {
      if (printMessages)
        Serial.println("Código HTTP ► " + String(responseCode));

      if (responseCode == 200)  // La API envió una respuesta
      {
        String response = http.getString();
        if (printMessages) {
          Serial.println("El servidor respondió ▼ ");
          Serial.println(response + "▼");
          Serial.println("------------------");
        }
        http.end();
        return response;
      }
    } else {
      String response = http.getString();
      http.end();

      if (printMessages) {
        Serial.print("Error enviando POST, código: ");
        Serial.println(String(responseCode));
        Serial.println(response);
      }
    }
  } else {
    if (printMessages)
      Serial.println("\nError en la conexión WIFI");
  }
  return "";
}

//Obtener la fecha actual
void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No se pudo obtener la hora.");
    return;
  }
  char buffer[80];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S %z", &timeinfo);
  date = String(buffer);
  Serial.println(buffer);
}