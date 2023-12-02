#include <WiFi.h>
#include <HTTPClient.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <Adafruit_NeoPixel.h>
#include <ESPmDNS.h>

#define PIN 15
#define NUMPIXELS 7 // Adjust for the number of LEDs you are using
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

AsyncWebServer server(80);  // Create an instance of the web server on port 80
const char* ssid = "<WifiName>";
const char* password = "<WifiPassword>";
const char* serverIp = "10.0.0.141";
const char* serverPort = "8080";
const char* name = "Light one";

const char* BUTTON_ONE = "power";
const char* BUTTON_TWO = "not implemented";

const char* CHECKBOX_ONE = "color_sync_pattern";
const char* CHECKBOX_TWO = "chasing_pattern";
const char* CHECKBOX_THREE = "twinkling_white";
const char* CHECKBOX_FOUR = "warm_color_synth";
const char* CHECKBOX_FIVE = "cold_color_synth";

int looping = 1;

// Function declarations
void turnOffLEDs();
void chasingPattern();
void colorSyncPattern();
uint32_t Wheel(byte WheelPos);

void handleUpdate(AsyncWebServerRequest *request) {
  // Add CORS headers
  AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Update received");
  response->addHeader("Access-Control-Allow-Origin", "*");
  response->addHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
  response->addHeader("Access-Control-Allow-Headers", "Content-Type");

  handleButtonState(request, BUTTON_ONE, 2, 0);    // Example: Color Sync Pattern or Turn Off
  handleButtonState(request, BUTTON_TWO, 99, 0);    // Example: Chasing Pattern or Twinkling White

  handleButtonState(request, CHECKBOX_ONE, 2, 0);   // Twinkling White or Turn Off
  handleButtonState(request, CHECKBOX_TWO, 3, 0);  // Warm Color Synth or Turn Off
  handleButtonState(request, CHECKBOX_THREE, 4, 0);  // Cold Color Synth or Turn Off
  handleButtonState(request, CHECKBOX_FOUR, 5, 0);   // Custom Mode One or Turn Off
  handleButtonState(request, CHECKBOX_FIVE, 6, 0);  // Custom Mode Two or Turn Off

  // Send the response back to the client
  request->send(response);
}


void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.show();
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    flashRed(); // Flash all LEDs in red
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  server.on("/update", HTTP_POST, handleUpdate);  // Set up the /update endpoint
  server.begin();  // Start the server
}

void loop() {
  if (looping == 1) {
     
    String url = "http://" + String(serverIp)+ ":" + String(serverPort) + "/update";
    HTTPClient http;
    Serial.println("Sending Payload");
    http.begin(url);

    String payload = "{\"ip\":\"";
    payload += WiFi.localIP().toString();
    payload += "\",\"name\":\"";
    payload += String(name);
    payload += "\",\"buttons\":{";
    // payload += "\"" + String(BUTTON_ONE) + "\":false,"; // Assuming BUTTON_ONE is defined
    // payload += "\"" + String(BUTTON_TWO) + "\":false";  // Assuming BUTTON_TWO is defined
    payload += "},\"checkboxes\":{";
    payload += "\"" + String(CHECKBOX_ONE) + "\":false,";  // "twinkling_white":false
    payload += "\"" + String(CHECKBOX_TWO) + "\":false,";  // "warm_color_synth":false
    payload += "\"" + String(CHECKBOX_THREE) + "\":false,"; // "cold_color_synth":false
    payload += "\"" + String(CHECKBOX_FOUR) + "\":false,";  // "custom_mode_one":false
    payload += "\"" + String(CHECKBOX_FIVE) + "\":false";   // "custom_mode_two":false
    payload += "}}";

   int httpCode = http.POST(payload);
      if (httpCode > 0) {
        String response = http.getString();
        Serial.println(response);
      }
      http.end();
    
    looping = 99;

  } if (looping == 0) {
    turnOffLEDs();
  } else if (looping == 99) {
    colorSyncPattern();
    twinklingWhitePattern();
  } else if (looping == 2) {
    colorSyncPattern();
  } else if (looping == 3) {
    chasingPattern();
  } else if (looping == 4) {
    twinklingWhitePattern();
  } else if (looping == 5) {
    warmColorSynthPattern();
  } else if (looping == 6) {
    coldColorSynthPattern();
  } else if (looping == 7) {
    coldColorSynthPattern();
  } else if (looping == 8) {
    coldColorSynthPattern();
  }
}

void handleButtonState(AsyncWebServerRequest *request, const char* buttonName, int trueMode, int falseMode) {
  if (request->hasParam(buttonName, true)) {
    String buttonStatus = request->getParam(buttonName, true)->value();
    looping = (buttonStatus == "true") ? trueMode : falseMode;
  }
}

void colorSyncPattern() {
  int startingColor = random(0, 255 * 5); // Random starting color index
  int endColor = startingColor + (255 * 5); // Full cycle

  for (int color = startingColor; color < endColor; color++) {
    for (int i = 0; i < NUMPIXELS; i++) {
      reconnectWiFi();
      if (looping != 2) {
        return;
      }
      strip.setPixelColor(i, Wheel(color % (255 * 5)));
    }
    strip.show();
    delay(500);  
  }
}

void chasingPattern() {
   const uint32_t red = strip.Color(255, 0, 0);
  const uint32_t blue = strip.Color(0, 0, 255);

  while (looping == 3) {
    reconnectWiFi();

    for (int i = 0; i < NUMPIXELS; i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
    
    int twinklingLEDs = random(3, 8);


    for (int i = 0; i < twinklingLEDs; i++) {
      int ledIndex = random(NUMPIXELS);
      uint32_t color = random(2) % 2 == 0 ? red : blue;
      strip.setPixelColor(ledIndex, color);
    }

    strip.show();
    delay(200);

    if (looping != 3) {
      return;
    }
  }
}



void twinklingWhitePattern() {
  const uint32_t colors[] = {strip.Color(255, 255, 255),   // White
                             strip.Color(255, 240, 220),   // Yellow-White
                             strip.Color(255, 105, 180)};  // Pink

  while (looping == 4) {
    reconnectWiFi();
    int randomLED = random(NUMPIXELS);
    int randomColor = random(3);

    strip.setPixelColor(randomLED, colors[randomColor]);
    strip.show();
    delay(100);

    strip.setPixelColor(randomLED, strip.Color(0, 0, 0));
    strip.show();
    delay(50);
  }
}


void warmColorSynthPattern() {
  for (int i = 0; i < NUMPIXELS; i++) {
    reconnectWiFi();
    if (looping != 5) {
      return;
    }

    strip.setPixelColor(i, strip.Color(255, 140, 0));
    strip.show();
    delay(100);
  }
}

void coldColorSynthPattern() {
  const uint32_t colors[] = {strip.Color(0, 0, 255),   // Blue
                             strip.Color(128, 0, 128), // Purple
                             strip.Color(255, 105, 180)}; // Pink
  int colorIndex = 0;

  while (looping == 6) {
    reconnectWiFi();  // Check WiFi connection

    for (int brightness = 0; brightness <= 255; brightness++) { // Fade in
      for (int i = 0; i < NUMPIXELS; i++) {
        strip.setPixelColor(i, dimColor(colors[colorIndex], brightness));
      }
      strip.show();
      delay(10);
    }

    for (int brightness = 255; brightness >= 0; brightness--) { // Fade out
      for (int i = 0; i < NUMPIXELS; i++) {
        strip.setPixelColor(i, dimColor(colors[colorIndex], brightness));
      }
      strip.show();
      delay(10);
    }

    colorIndex = (colorIndex + 1) % 3; // Move to the next color
  }
}

uint32_t dimColor(uint32_t color, int brightness) {
  uint8_t r = (uint8_t)(brightness * ((color >> 16) & 0xFF) / 255);
  uint8_t g = (uint8_t)(brightness * ((color >> 8) & 0xFF) / 255);
  uint8_t b = (uint8_t)(brightness * (color & 0xFF) / 255);
  return strip.Color(r, g, b);
}

void flashRed() {
  for(int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0));
  }
  strip.show();
  delay(100);
  turnOffLEDs();
  delay(100);
}

void Success() {
  for(int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(26, 255, 0));
  }
  strip.show();
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

void turnOffLEDs() {
  for(int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();
}

void reconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Disconnected from WiFi. Attempting reconnection...");
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    if(WiFi.status() == WL_CONNECTED) {
      Serial.println("\nReconnected to WiFi.");
    } else {
      Serial.println("\nFailed to reconnect.");
    }
  }
  }
