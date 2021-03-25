#include <DHT.h>
#include <DHT_U.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define DHTPIN D2 //inside temp
#define DHTPIN2 D1 //outside temp
#define DHTTYPE    DHT22 


#ifndef STASSID
#define STASSID "your-wifi-ssid"
#define STAPSK  "your-wifi-passwd"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

ESP8266WebServer server(80);

uint32_t delayMS; //does not used for now

DHT_Unified dht(DHTPIN, DHTTYPE); //create dth sensor
DHT_Unified dht2(DHTPIN2, DHTTYPE);//create seconf dht sennsor for outsidetemp
 
float temperature_in, hum_in, temperature_out, hum_out;// global variables for sensor reading

void handleRoot() {
  char temp[2024];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  readDHT();
  readDHT2();
  snprintf(temp, 2024,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='60'/>\ 
    <meta charset='UTF-8'>\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">\
    <title>DHT22</title>\
    <style>\
      html {text-align: center; font-size: 20.5%; font-family: -apple-system, BlinkMacSystemFont, \"Segoe UI\", Roboto, \"Helvetica Neue\", Arial, \"Noto Sans\", sans-serif;}\
      body{font-size: 1.8rem; line-height: 1.618; max-width: 38em; margin: auto;color: #c9c9c9; background-color: #222222; padding: 13px;}\
      h1 {font-size: 2.35em; line-height: 1.1;  font-family: -apple-system, BlinkMacSystemFont, \"Segoe UI\", Roboto, \"Helvetica Neue\", Arial, \"Noto Sans\", sans-serif;  font-weight: 700;  margin-top: 3rem;  margin-bottom: 1.5rem; overflow-wrap: break-word; word-wrap: break-word; -ms-word-break: break-all; word-break: break-word;}\
      p {margin-top: 0px; margin-bottom: 2.5rem;}\
      hr{border-color: #ffffff;}\
    </style>\
  </head>\
  <body>\
    <h1>Egemeric's DHT22!</h1>\
    <p style='text-align: left;'>Uptime: %02d:%02d:%02d</p>\
    <p>Inside Temp:%0.2f°C</p>\
    <p>Inside Hum:%0.2f%%: <meter id=\"hum_in\" value=\"%0.1f\" min=\"0\" max=\"100\"></meter> </p>\
    <hr>\
    <p>Outside Temp:%0.2f°C</p>\
    <p>Outside Hum:%0.2f%%: <meter id=\"hum_out\" value=\"%0.1f\"min=\"0\" max=\"100\"></meter> </p>\
  </body>\
</html>",

           hr, min % 60, sec % 60,temperature_in,hum_in,hum_in,temperature_out,hum_out,hum_out
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
  server.on("/get_temp_in", []() {
    readDHT();
    char tmp[10];
    sprintf(tmp,"%f",temperature_in);
    server.send(200, "text/plain", tmp);
  });
  server.on("/get_hum_in", []() {
    readDHT();
    char tmp[10];
    sprintf(tmp,"%f",hum_in);
    server.send(200, "text/plain", tmp);
  });
  server.on("/get_temp_out", []() {
    readDHT2();
    char tmp[10];
    sprintf(tmp,"%f",temperature_out);
    server.send(200, "text/plain", tmp);
  });
  server.on("/get_hum_out", []() {
    readDHT2();
    char tmp[10];
    sprintf(tmp,"%f",hum_out);
    server.send(200, "text/plain", tmp);
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  dht.begin();
  dht2.begin();
  sensor_t sensor;
  sensor_t sensor2;
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
    temperature_in = event.temperature;
    sprintf(dum,"Temp: %f \n", temperature_in);
    Serial.println(out);
    out+=dum;
    
  }dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    hum_in = event.relative_humidity;
    sprintf(dum,"hum: %f%%\n", hum_in);
    out += dum;
  }

}
void readDHT2(){
  String out = "";
  char dum[50];
  sensors_event_t event2;
  dht2.temperature().getEvent(&event2);
  if (isnan(event2.temperature)) {
    Serial.println(F("Error reading the sensor !"));
  }
  else {
    temperature_out = event2.temperature;
    sprintf(dum,"Temp_out: %f \n", temperature_out);
    Serial.println(out);
    out+=dum;
    
  }dht2.humidity().getEvent(&event2);
  if (isnan(event2.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    hum_out = event2.relative_humidity;
    sprintf(dum,"hum_out: %f%%\n", hum_out);
    out += dum;
  }
  
}
