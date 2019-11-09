/* 
Погодная станция для снятия данных из аомосферы и передчи по протоколу mqtt брокеру.
Полное описание проекта тут: https://vc.ru/dev/83353
*/
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <ESP8266WebServerSecureAxTLS.h>
#include <ESP8266WebServerSecureBearSSL.h>
#include <ESP8266WebServerSecure.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Adafruit_CCS811.h> 
#include <PubSubClient.h> 
#include <DHT.h> 

//==================================================================
// ###           ПАРАМЕТРЫ НАСТРОЙКИ           ###
#define DHTPIN 0 // пин датчика температуры
#define DHTTYPE DHT11 // тип датчика
const int PIN_LED = 2; // пин светодиода для тестов (встроенный в esp12s светодиод)
const int TRIGGER_PIN = 0; // пин кнопки для перевода в режим прошивки
const char* hostaddr = "weather01"; //адрес платы в сети
const char* pwd = "99999"; // после смены пароля устройство нужно перезагрузить по питанию
const char* mqttname = "arduino";
const char* mqttpwd = "99999";
const long sensors_int = 10000; // интервал отправки данных с датчиков брокеру MQTT
const long blink_int = 1000; // интервал изменения статуса светодиода
const char* mqttServer = "192.168.0.3"; //адрес MQTT брокера
const int mqtt_port = 1883; // порт MQTT брокера
//==================================================================
bool ota_flag = true;
uint16_t time_elapsed = 0;
unsigned long startedAt = millis();
bool initialConfig = false; // Определяет - помнит ли плата SSID сети WiFi
String payload;
unsigned long prev_sens = 0; // переменная для хранения времени последней отправки данных от сенсоров
unsigned long prev_blink = 0; // переменная для хранения времени последнего изменения статуса тестового светодиода
//==================================================================
// Определение экземпляров
ESP8266WebServer server;
WiFiClient espClient; 
PubSubClient client(espClient); 
DHT dht(DHTPIN, DHTTYPE); 
Adafruit_CCS811 ccs;
//==================================================================
void setup() {
  Serial.begin(115200); // стартуем терминал
//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 
//Настройка WiFi через портал
  pinMode(PIN_LED, OUTPUT); // Использование светодиода для индикации статуса настройки WiFI
  Serial.println("\n Starting");
  WiFi.printDiag(Serial); // Если вы не хотите, чтобы WiFi пароль отправлялся в терминал - удалите эту строку
  delay(5000);
    if (WiFi.status()!=WL_CONNECTED){
    Serial.println("!=WL_CONNECTED");
  } else{
    Serial.print("local ip: ");
    Serial.println(WiFi.localIP());
  }
  
  if (WiFi.SSID()=="" || WiFi.status()!=WL_CONNECTED){
    Serial.println("We haven't got any access point credentials, so get them now");   
    initialConfig = true;
  }
  else{
    digitalWrite(PIN_LED, HIGH); // Гасим светодиод, так как конфигурирование не требуется
    WiFi.mode(WIFI_STA); // Переводим в режим станции, так как если этого не сделать, то после перезагрузки система снова поднимет точку доступа
    unsigned long startedAt = millis();
    Serial.print("After waiting ");
    int connRes = WiFi.waitForConnectResult();
    float waited = (millis()- startedAt);
    Serial.print(waited/1000);
    Serial.print(" secs in setup() connection result is ");
    Serial.println(connRes);
  }
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  if (WiFi.status()!=WL_CONNECTED){
    Serial.println("failed to connect, finishing setup anyway");
  } else{
    Serial.print("local ip: ");
    Serial.println(WiFi.localIP());
  }
//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 

  if (WiFi.status() == WL_CONNECTED) {
//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 
// Настройка OTA (Over the air) или обновление по воздуху
  ArduinoOTA.setHostname(hostaddr);
  ArduinoOTA.setPassword(pwd); 
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  pinMode(PIN_LED,OUTPUT);
//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 
// OTA web command control area
  server.on("/restart",[](){ //команда в браузере на перезагрузку
    server.send(200,"text/plain", "Restarting ...");
    delay(1000);
    ESP.restart();
    });

  server.on("/reburn",[](){ //команда в браузере на запуск возможности сменить прошивку
    server.send(200,"text/plain", "Reburning mode starting...");
    ota_flag = true;
    time_elapsed = 0;
  });

  server.on("/wifi",[](){
    server.send(200,"text/plain", "Starting WiFi manager portal...");
    delay(5000);
    WiFi.begin("0000", "0000");
    delay(3000);
    WiFi.printDiag(Serial);
    delay(3000);
    ESP.restart();
  });

  server.begin();
//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 
// запуск DHT и ccs811
  ccs.begin();  // 
  dht.begin(); // 
//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 
//  MQTT setup area
  client.setServer(mqttServer, mqtt_port); 
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
if (client.connect("esp2", mqttname, mqttpwd)) { 
      Serial.println("connected");
      client.publish("sensors/node2", "hello world");    //топик, фраза в топик
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
// Wait 5 seconds before retrying
delay(5000);
    }
  }
  }
}

//==================================================================
void loop() {
  unsigned long currentMillis = millis();
//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 
// WiFI setup waiting area
  if ((digitalRead(TRIGGER_PIN) == LOW) || (initialConfig)) {
     Serial.println("Configuration portal requested.");
     digitalWrite(PIN_LED, LOW); // turn the LED on by making the voltage LOW to tell us we are in configuration mode.
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

    //sets timeout in seconds until configuration portal gets turned off.
    //If not specified device will remain in configuration mode until
    //switched off via webserver or device is restarted.
    //wifiManager.setConfigPortalTimeout(600);

    //it starts an access point 
    //and goes into a blocking loop awaiting configuration
    if (!wifiManager.startConfigPortal()) {
      Serial.println("Not connected to WiFi but continuing anyway.");
    } else {
      //if you get here you have connected to the WiFi
      Serial.println("connected...yeey :)");
    }
    digitalWrite(PIN_LED, HIGH); // Turn led off as we are not in configuration mode.
    ESP.reset(); // This is a bit crude. For some unknown reason webserver can only be started once per boot up 
    // so resetting the device allows to go back into config mode again when it reboots.
    delay(5000);
  }
//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 
  if (WiFi.status() == WL_CONNECTED) {
//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
// mqtt area
if (!client.connected()){
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
if (client.connect("esp2", mqttname, mqttpwd)) {     // имя устройства, логин, пароль
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
// Wait 5 seconds before retrying
delay(5000);
     }
    }
  }
//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
// проверка времени для определения необходимости отправки данных с сенсоров
  if (currentMillis - prev_sens >= sensors_int) {
    prev_sens = currentMillis;  
//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =    
// dht reading and publishing
  int h = dht.readHumidity(); // считываем влажность  
  int t = dht.readTemperature(); // считываем температуру в градусах цельсия
    // проверяем полученные с датчика значения
  if (isnan(h) || isnan(t)) {
    Serial.println("DHT11 Error"); //сообщение об ошибке, если датчик работает неверно
    return;
  }
  if (h <= 100) {
  // отправка данных давления
  payload = "sensor humidity = ";
  payload += h;
  client.publish("sensors/node2/hum", (char*) payload.c_str());
  }
    if (t <= 100) {
  // отправка данных температуры
  payload = "sensor temperature = ";
  payload += t;  
  client.publish("sensors/node2/temp", (char*) payload.c_str());
  }  
    Serial.print("Влажность = ");
    Serial.println(h);
    Serial.print("Температура = ");
    Serial.println(t);
//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
//  чтение данных с ccs881 и их отправка
if(ccs.available()){
 ccs.readData();
      Serial.print("CO2: ");
      Serial.println(ccs.geteCO2());
      Serial.print("ppm, TVOC: ");
      Serial.println(ccs.getTVOC());
      int co2 = ccs.geteCO2();;
      payload = "CO2 = ";
      payload += co2;
      client.publish("sensors/node2/co2", (char*) payload.c_str());

      int tvoc = ccs.getTVOC();
      payload = "TVOC = ";
      payload += tvoc;
      client.publish("sensors/node2/tvoc", (char*) payload.c_str());
}
else
{
  Serial.println("CCS811 UnAvailable");
  }
//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =   
  }  
//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =  
// Запуск OTA
  if(ota_flag)
  {
    uint16_t time_start = millis();
    while(time_elapsed < 20000)
  {
   ArduinoOTA.handle();
   time_elapsed = millis() - time_start;
   delay(10);
  }
  ota_flag = false;  
}

  server.handleClient();
  }
//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =  
 // Мигание тестовым светодиодом
   if (currentMillis - prev_blink >= blink_int) {
    prev_blink = currentMillis;
  digitalWrite(PIN_LED, !digitalRead(PIN_LED));
  }
  
}
//==================================================================
