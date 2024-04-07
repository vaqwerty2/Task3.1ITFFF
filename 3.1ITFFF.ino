#include <WiFiNINA.h>
#include <Wire.h>
#include <BH1750.h>

// WiFi network details
const char* wifiSSID = "SHIVJI";
const char* wifiPassword = "Dreams11";

// IFTTT Webhooks details
const char* hostName = "maker.ifttt.com";
const char* eventPath = "/trigger/light_status/with/key/dgRt6ThDlm__7tFyyja7u7";

// Initialize the WiFi client library
WiFiClient client;

// Initialize BH1750 sensor
BH1750 lightMeter;

// State tracking variables
bool wasInSunlight = false; // Tracking the previous state of sunlight exposure.
unsigned long sunlightEntryTime = 0; // Tracks the time when the terrarium enters sunlight.
unsigned long totalSunlightDuration = 0; // Tracks the total duration the terrarium has been in sunlight.

void setup() {
  Serial.begin(9600);
  WiFi.begin(wifiSSID, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  
  // Initialize BH1750 sensor
  Wire.begin();
  lightMeter.begin();
}

void loop() {
  // Read light level from BH1750 sensor
  float lux = lightMeter.readLightLevel();
  
  if (lux >= 500 && !wasInSunlight) { // Entering sunlight
    wasInSunlight = true;
    sunlightEntryTime = millis(); // Record the time when the terrarium enters sunlight
    sendSunlightChangeToIFTTT("Entering_sunlight", 0, totalSunlightDuration); // Send notification for entering sunlight with default duration 0 and total duration
  } else if (lux < 500 && wasInSunlight) { // Exiting sunlight
    wasInSunlight = false;
    unsigned long sunlightExitTime = millis(); // Record the time when the terrarium exits sunlight
    unsigned long sunlightDuration = (sunlightExitTime - sunlightEntryTime) / 1000; // Calculate duration in seconds
    totalSunlightDuration += sunlightDuration; // Update total duration
    sendSunlightChangeToIFTTT("Exiting_sunlight", sunlightDuration, totalSunlightDuration); // Send notification for exiting sunlight along with durations
  }

  delay(10000); // Delay for a bit before taking the next reading
}

void sendSunlightChangeToIFTTT(const char* message, unsigned long duration, unsigned long totalDuration) {
  if (client.connect(hostName, 80)) {
    // Prepare the URL query string
    String queryString = String(eventPath) + "?value1=" + String(message) + "&value2=" + String(duration) + "&value3=" + String(totalDuration);

    client.println("GET " + queryString + " HTTP/1.1");
    client.println("Host: " + String(hostName));
    client.println("Connection: close");
    client.println(); // End of headers

    Serial.println("Sending data to IFTTT:");
    Serial.println("Message: " + String(message));
    Serial.println("Duration (Instance): " + String(duration) + " seconds");
    Serial.println("Total Duration: " + String(totalDuration) + " seconds");

    // Wait for response from the server
    while (!client.available()) {
      delay(100);
    }

    // Read and print response from server
    while (client.available()) {
      Serial.write(client.read());
    }
  } else {
    Serial.println("Connection to IFTTT failed");
  }
  
  client.stop();
}
