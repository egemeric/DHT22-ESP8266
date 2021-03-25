#include <DHT.h>
#include <DHT_U.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define DHTPIN D2
#define DHTTYPE    DHT22 


#ifndef STASSID
#define STASSID "your-wifi-name"
#define STAPSK  "wifi-passwd"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

ESP8266WebServer server(80);

uint32_t delayMS; //does not used for now

DHT_Unified dht(DHTPIN, DHTTYPE); //create dth sensor

float temperature,hum;// global variables for sensor reading

void handleRoot() {
  char temp[800];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  readDHT();
  snprintf(temp, 800,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\ 
    <meta charset='UTF-8'>\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\
    <title>DHT22</title>\
    <style>\
      html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\
      body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\
      p {font-size: 24px;color: #444444;margin-bottom: 10px;}\
    </style>\
  </head>\
  <body>\
    <h1>Egemeric's DHT22!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <p>Temp:%0.2f°C</p>\
    <p>Hum:%0.2f%%</p>\
  </body>\
</html>",

           hr, min % 60, sec % 60,temperature,hum
          );
  server.send(200, "text/html", temp);
  
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);

}

void setup(void) {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/temp", readDHT);
  server.on("/get_temp", []() {
    readDHT();
    char tmp[10];
    sprintf(tmp,"%f",temperature);
    server.send(200, "text/plain", tmp);
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
   dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}

void readDHT() {
  String out = "";
  char dum[50];
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading the sensor !"));
  }
  else {
    temperature = event.temperature;
    sprintf(dum,"Temp: %f \n", temperature);
    Serial.println(out);
    out+=dum;
    
  }dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    hum = event.relative_humidity;
    sprintf(dum,"hum: %f%%\n", hum);
    out += dum;
  }

}
