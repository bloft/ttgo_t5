// include library, include base class, make path known
#include <GxEPD.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#include <Fonts/FreeSans9pt7b.h>
#define DEFALUT_FONT FreeSans9pt7b

const GFXfont *fonts[] = {
    &FreeSans9pt7b,
};

#include <MD5Builder.h>
#include <WiFi.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <TJpg_Decoder.h>
#include "esp_wifi.h"
#include "Esp.h"
#include "board_def.h"
#include "ByteStream.h"
#include "config.h"

#define IP5306_ADDR 0X75
#define IP5306_REG_SYS_CTL0 0x00

GxIO_Class io(SPI, ELINK_SS, ELINK_DC, ELINK_RESET);
GxEPD_Class display(io, ELINK_RESET, ELINK_BUSY);

ByteStream imageData;
uint8_t imageMD5[16] = {0};

void connectToWiFi() {
  WiFi.mode(WIFI_MODE_STA);
  WiFi.enableSTA(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to "); Serial.println(WIFI_SSID);
 
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
 
    if ((++i % 16) == 0) {
      Serial.println(F(" still trying to connect"));
    }
  }
 
  Serial.print(F("Connected. My IP address is: "));
  Serial.println(WiFi.localIP());
}

bool drawImage() {
  HTTPClient http;
  long totalSize = 0;
  uint32_t t = millis();
  // configure server and url update based on your URL
  http.begin(IMAGE_URL);  //update based on your URL
  // start connection and send HTTP header

  long httpCode = http.GET();
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) {

      imageData.reset();
      http.writeToStream(&imageData);

      Serial.printf("[HTTP] size: %s\n", String(imageData.size));
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();

  uint8_t md5[16];

  t = millis() - t;
  Serial.printf("[HTTP] Download time: %d ms\n", t);
  Serial.printf("[HTTP] Total size: %s\n", String(totalSize));

  t = millis();
  MD5Builder md5Builder;
  md5Builder.begin();
  md5Builder.add(imageData.byteArray, imageData.size);
  md5Builder.calculate();
  md5Builder.getBytes(md5);
  Serial.printf("[MD5] md5: %s\n", md5Builder.toString().c_str());
  t = millis() - t;
  Serial.printf("[MD5] time: %d ms\n", t);

  if (memcmp(md5, imageMD5, sizeof(imageMD5)) != 0) {
    md5Builder.getBytes(imageMD5);
    Serial.println("[HTTP] Image update");
    
    t = millis();

    // Get the width and height in pixels of the jpeg if you wish
    uint16_t w = 0, h = 0;
    TJpgDec.getJpgSize(&w, &h, imageData.byteArray, imageData.size);
    Serial.printf("[JPG] Width: %d, Height: %d\n", w, h);

    if(w > 0 && h > 0) {
      display.fillScreen(GxEPD_WHITE);

      // Draw the image, top left at 0,0
      TJpgDec.drawJpg(0, 0, imageData.byteArray, imageData.size);

      // How much time did rendering take (ESP8266 80MHz 473ms, 160MHz 266ms, ESP32 SPI 116ms)
      t = millis() - t;
      Serial.printf("[JPG] Decode time: %d ms\n", t);

      return true;
    } else {
      Serial.println("[DISPLAY] Image size to low, ignoring");
    }
  } else {
    Serial.println("[HTTP] Image no change, no update needed");
  }
  return false;
}

// this function determines the minimum of two numbers
#define minimum(a,b)     (((a) < (b)) ? (a) : (b))

// This next function will be called during decoding of the jpeg file to
// render each block to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
   // Stop further decoding as image is running off bottom of screen
  if ( y >= display.height() ) return 0;

  uint16_t color = GxEPD_WHITE;
  display.startWrite();
  for (int16_t j = 0; j < h; j++, y++) {
    for (int16_t i = 0; i < w; i++) {
        if(bitmap[j * w + i] > 0x7BEF) { /* 128, 128, 128 */
            color = GxEPD_WHITE;
        } else {
            color = GxEPD_BLACK;
        }
        display.writePixel(x + i, y, color);
    }
  }

    display.endWrite();
    // Return 1 to decode next block
    return 1;
}

void displayInit(void) {
    static bool isInit = false;
    if (isInit) {
        return;
    }
    Serial.printf("[DISPLAY] init\n");
    isInit = true;
    display.init();
    display.setRotation(1);
    display.eraseDisplay();
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&DEFALUT_FONT);
    display.setTextSize(0);
}

bool setPowerBoostKeepOn(int en) {
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_REG_SYS_CTL0);
    if (en)
        Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
    else
        Wire.write(0x35); // 0x37 is default reg value
    return Wire.endTransmission() == 0;
}

void draw() {
    displayInit();
    if(drawImage()) {
      display.update();
      Serial.printf("[DISPLAY] Update\n");
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);

#ifdef ENABLE_IP5306
    Wire.begin(I2C_SDA, I2C_SCL);
    bool ret = setPowerBoostKeepOn(1);
    Serial.printf("Power KeepUp %s\n", ret ? "PASS" : "FAIL");
#endif

// It is only necessary to turn on the power amplifier power supply on the T5_V24 board.
#ifdef AMP_POWER_CTRL
    pinMode(AMP_POWER_CTRL, OUTPUT);
    digitalWrite(AMP_POWER_CTRL, HIGH);
#endif

    SPI.begin(SPI_CLK, SPI_MISO, SPI_MOSI, -1);

    connectToWiFi();

    // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
    TJpgDec.setJpgScale(1);

    // The decoder must be given the exact name of the rendering function above
    TJpgDec.setCallback(tft_output);
}

void loop() {
    draw();
    delay(15000);
}