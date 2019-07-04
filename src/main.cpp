/*
 * OTAWebUpdater.ino Example from ArduinoOTA Library
 * Rui Santos 
 * Complete Project Details http://randomnerdtutorials.com
 */

#include <WiFi.h>
#include <WebServer.h>
#include <AutoConnect.h>
#include <Update.h>

#include "pages.hpp"

//variabls for blinking an LED with Millis
const int led = LED_BUILTIN; // ESP32 Pin to which onboard LED is connected
unsigned long previousMillis = 0;  // will store last time LED was updated
const long interval = 1000;  // interval at which to blink (milliseconds)
int ledState = LOW;  // ledState used to set the LED

WebServer server(80);
AutoConnect portal(server);
AutoConnectConfig config;
AutoConnectAux auxPage("/", "mainPage");
AutoConnectAux auxPage2("/testPage", "testPage");
ACText(header, "Main page header");
ACText(caption, "Description");

void rootPage() {
  char content[] = "Ota updated<br><p style=\"padding-top:15px;text-align:center\">"
  "<a href=\"/_ac\"><img src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAC2klEQVRIS61VvWsUQRSfmU2pon9BUIkQUaKFaCBKgooSb2d3NSSFKbQR/KrEIiIKBiGF2CgRxEpjQNHs7mwOUcghwUQ7g58IsbGxEBWsb2f8zR177s3t3S2cA8ftzPu993vzvoaSnMu2vRKlaqgKp74Q/tE8qjQPyHGcrUrRjwlWShmDbFMURd/a6TcQwNiYUmpFCPElUebcuQ2vz6aNATMVReHEPwzfSSntDcNwNo2rI+DcvQzhpAbA40VKyV0p1Q9snzBG1qYVcYufXV1sREraDcxpyHdXgkfpRBj6Uwm2RsC5dxxmZ9pdOY9cKTISRcHTCmGiUCh4fYyplTwG2mAUbtMTBMHXOgK9QfyXEZr+TkgQ1oUwDA40hEgfIAfj+HuQRaBzAs9eKyUZ5Htx+T3ZODKG8DzOJMANhmGomJVMXPll+hx9UUAlzZrJJ4QNCDG3VEfguu7mcpmcB/gkBOtShhQhchAlu5jlLUgc9ENgyP5gf9+y6LTv+58p5zySkgwzLNOIGc8sEoT1Lc53NMlbCQQuvMxeCME1NNPVVkmH/i3IzzXDtCSA0qQQwZWOCJDY50jsQRjJmkslEOxvTcDRO6zPxOh5xZglKkYLhWM9jMVnkIsTyMT6NBj7IbOCEjm6HxNVVTo2WXqEWJZ1T8rytB6GxizyDkPhWVpBqfiXUtbo/HywYJSpA9kMamNNPZ71R9Hcm+TMHHZNGw3EuraXEUldbfvw25UdOjqOt+JhMwJd7+jSTpZaEiIcaCDwPK83jtWnTkwnunFMtxeL/ge9r4XItt1RNNaj/0GAcV2bR3U5sG3nEh6M61US+Qrfd9Bs31GGulI2GOS/8dgcQZV1w+ApjIxB7TDwF9GcNzJzoA+rD0/8HvPnXQJCt2qFCwbBTfRI7UyXumWVt+HJ9NO4XI++bdsb0YyrqXmlh+AWOLHaLqS5CLQR5EggR3YlcVS9gKeH2hnX8r8Kmi1CAsl36QAAAABJRU5ErkJggg==\" border=\"0\" title=\"AutoConnect menu\" alt=\"AutoConnect menu\"/></a>"
  "</p>";
  server.send(200, "text/html", content);
}

/*
 * setup function
 */
void setup(void) {
  delay(1000);
  
  pinMode(led,  OUTPUT);

  Serial.begin(115200);
  Serial.println();

  // server.on("/",  rootPage);
  server.on("/testPage",  rootPage);
  server.on("/login", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });

  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });

  //set autoconnect
  config.apid = "esp32ap";
  config.psk = "123456789";
  config.hidden = true;
  config.channel = 7;
  config.autoReconnect = true;
  config.autoSave = AC_SAVECREDENTIAL_AUTO;
  config.hostName = "esp32";
  config.title = "testOta";
  portal.config(config);

  auxPage.add({header, caption});
  portal.join({auxPage, auxPage2});
  
  if(portal.begin())
  {
    Serial.println("connected: "+ WiFi.SSID() );  
  }
  else
  {
    Serial.println("connection failed: " + WiFi.status());
  }


  Serial.println("Web server started:" + WiFi.localIP().toString());
}

void loop(void) {
  portal.handleClient();
  delay(1);

  //loop to blink without delay
 unsigned long currentMillis = millis();
 if (currentMillis - previousMillis >= interval) {
 // save the last time you blinked the LED
 previousMillis = currentMillis;
 // if the LED is off turn it on and vice-versa:
 ledState = not(ledState);
 // set the LED with the ledState of the variable:
 digitalWrite(led,  ledState);
 }

}
