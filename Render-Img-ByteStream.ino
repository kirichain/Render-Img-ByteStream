#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <TFT_eSPI.h>
//#include <WiFiClientSecure.h>
//#include "certs.h"
//#include "img.h"

TFT_eSPI tft = TFT_eSPI();

#define MAX_IMAGE_WDITH 100 // Adjust for your images

//const char *ssid = "ERPLTD";
//const char *password = "erp@@2020";

const char *ssid = "Guest";
const char *password = "24091995";

//const char *ssid = "SFS OFFICE";
//const char *password = "sfs#office!@";

//const char *imageUrl = "http://203.113.151.196:8080/img/avatars/imgpsh.png";
//const char *imageUrl = "http://10.102.40.102:890/ProcessImage/GetBMP";
//const char *imageUrl = "http://10.102.40.102:890/ProcessImage/GetBMP?path=apple100.png&depth=32";
const char *imageUrl = "http://10.102.40.102:890/ProcessImage/GetBMP?path=apple100.png&depth=16";
//const char *imageUrl = "http://64a77ed6096b3f0fcc815dc3.mockapi.io/api/8bit/arr";
//const char *imageUrl = "http://64a77ed6096b3f0fcc815dc3.mockapi.io/api/8bit/apple";

// Declare the array to store the image, image size is 100x100
uint16_t imageArray[20000] = {0};

void setup() {
    Serial.begin(115200);

    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
}

void loop() {
    if (downloadAndDisplayImage(imageUrl)) {
        Serial.println("Image downloaded and displayed successfully");
        //WiFi.disconnect(true);
    } else {
        Serial.println("Failed to download or display the image");
    }
    // Delay for 5 mins
    delay(300000);
}

bool downloadAndDisplayImage(const char *url) {
    WiFiClient client;
    HTTPClient http;

    // Clear image array
    memset(imageArray, 0, sizeof(imageArray));
    if (http.begin(client, url)) {
        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            //Print http code on TFT
            tft.drawString("HTTP code: " + String(httpCode), 0, 0);
            // Waiting for 5 seconds, then make loading animation, then display the image
            delay(2000);
            tft.fillScreen(TFT_BLACK);
            // file found at server
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                // get length of document (is -1 when Server sends no Content-Length header)
                int len = http.getSize();
                // Print size to Serial
                Serial.printf("Image Size: %d\n", len);
                // Variable to count the number of bytes read
                int count = 0;
                WiFiClient *stream = http.getStreamPtr();
                // Buffer to read
                uint8_t buff[128] = {0};
                // Array to store continuous group of 2 bytes, then we combine them to get the original byte
                uint8_t buff2[2] = {0};
                // uint16_t variable to store the original byte
                uint16_t buff16 = 0;
                // Index for imageArray to determine where to store the next byte
                int index = 0;
                // Read all data from server
                while (http.connected() && (len > 0 || len == -1)) {
                    // get available data size
                    size_t size = stream->available();
                    // Count the number of bytes read
                    count += size;
                    // Print size to Serial with line break
                    Serial.printf("Size available: %d\n", size);
                    Serial.println();
                    if (size) {
                        // read up to 128 byte
                        int c = stream->readBytes(buff, std::min((size_t) len, sizeof(buff)));
                        int i = 0;
                        while (i < c) {
                            // Print buff[i] to Serial as HEX
                            Serial.printf("%02X ", buff[i]);
                            Serial.printf("%02X ", buff[i + 1]);
                            // Print buff[i] to Serial as DEC
                            //Serial.printf("%d ", buff[i]);
                            // Add buff[i] and buff[i+1] to buff2
                            buff2[0] = buff[i];
                            buff2[1] = buff[i + 1];
                            // Concatenate buff2[0] and buff2[1] to 16-bit variable buff16
                            buff16 = (buff2[0] << 8) | buff2[1];
                            // Print buff16 to Serial as HEX
                            Serial.printf("%04X ", buff16);
                            // Print buff16 to Serial as DEC
                            //Serial.printf("%d ", buff16);
                            // Reset buff2
                            memset(buff2, 0, sizeof(buff2));
                            // Write buff16 to imageArray
                            //imageArray[index] = buff[i];
                            imageArray[index] = buff16;
                            // Increase index
                            index++;
                            // Increase i by 2
                            i += 2;
                        }
                        if (len > 0) {
                            len -= c;
                        }
                    }
                    Serial.println();
                    delay(1);
                }
                Serial.println();
                // Show count value
                Serial.printf("Bytes count: %d\n", count);
                // Show index value
                Serial.printf("Index: %d\n", index);
                Serial.println();
                Serial.println("End of stream");
                // Print imageArray to Serial, the array is in uint16_t format
//                for (int i = 0; i < sizeof(imageArray); i++) {
//                    Serial.printf("%02X ", imageArray[i]);
//                }
                Serial.println();
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                tft.drawString("Loading image...", 0, 0);
                // Loading animation
                for (int i = 0; i < 100; i++) {
                    tft.drawPixel(i, 10, TFT_WHITE);
                    delay(10);
                }
                // Clearing the screen
                tft.fillScreen(TFT_BLACK);
                // Swap the colour byte order when rendering
                tft.setSwapBytes(true);
                // Display the image
                //tft.pushImage(0, 0, 100, 100, img);
                // Display the image
                tft.pushImage(0, 0, 100, 100, imageArray);
            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        // Done
        Serial.println("Done");
        http.end();
        return true;
    } else {
        Serial.println("Failed to connect to the server");
        return false;
    }
}