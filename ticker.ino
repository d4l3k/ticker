#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h> 
#include <ESP8266mDNS.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include "GxEPD2_display_selection_new_style.h"
#include <ESP8266HTTPClient.h>


bool connectionWasAlive = false;
int lastLogged = 0;
int lastChecked = -10000000;
ESP8266WiFiMulti wifiMulti;

#include "./networks.h"

void monitorWiFi() {
  if (wifiMulti.run() != WL_CONNECTED){
    if (connectionWasAlive) {
      connectionWasAlive = false;
      Serial.print("Looking for WiFi ");
    }
    int now = millis();
    if ((now - lastLogged) > 1000) {
      Serial.print(".");
      lastLogged = now;
    }
  } else {
    if (!connectionWasAlive) {
      connectionWasAlive = true;
      Serial.printf(" connected to %s\n", WiFi.SSID().c_str());
      String msg = String("connected to ") + WiFi.SSID();
      displayText(msg.c_str());

  
      if (MDNS.begin("ticker")) {
        Serial.println("mDNS responder started");
      } else {
        Serial.println("Error setting up MDNS responder!");
        displayText("Error setting up MDNS responder!");
      }
    }
  }
}

void setup() {
  //pinMode(15, OUTPUT_PULLDOWN);

  Serial.begin(115200);
  setupNetworks();

  monitorWiFi();
  
  display.init();
  displayText("connecting to wifi...");
}

void displayText(const char* text) {
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center the bounding box by transposition of the origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(text);
  }
  while (display.nextPage());
}

void fetchData() {
  if (!connectionWasAlive) {
    return;
  }
  int now = millis();
  if ((now - lastChecked) <= 360000) {
    return;
  }
  lastChecked = now;

  // https://financialmodelingprep.com/api/v3/historical-price-full/AAPL?apikey=demo
  const char* host = "financialmodelingprep.com";
  String url = "/api/v3/quote-short/GME?apikey=fea446c0f65ec6992de5e44fe21ae490";
            
  WiFiClientSecure client;
  client.setInsecure(); //the magic line, use with caution
  if (!client.connect(host, 443)) {
    displayText("failed to connect");
    return;
  }
  HTTPClient http;
  http.begin(client, String(host), 443, url, true);
    
  String payload;
  if (http.GET() != HTTP_CODE_OK) {
    displayText("error!");
    return;
  }
  payload = http.getString();    

  displayText(payload.c_str());
}

void loop() {
  monitorWiFi();

  fetchData();
}
