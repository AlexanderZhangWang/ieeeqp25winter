#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

#define SER_MON 1

// Define network credentials
const char* ssid = "A";
const char* password = "88888888";

// Define constants
const int TDS_PIN = 36;             // Analog input pin connected to the TDS sensor
const int TURB_PIN = 39;
const float VREF = 5.0;             // Reference voltage for Arduino (5V)
const float ADC_RESOLUTION = 4096;  // 12-bit ADC resolution
const float TEMP_COEFFICIENT = 0.02; // Compensation coefficient for water temperature (20°C as reference)

// Instantiate objects
TinyGPSPlus gps;
LiquidCrystal_PCF8574 lcd(0x27);
WebServer server(80);

// Use Hardware Serial 2 for GPS
HardwareSerial GPS_Serial(2);

void sendToDatabase(int analogValue, float voltage, float tdsValue, int turbidity, String gpsData) {
  HTTPClient http;
  // Update the URL below with your server’s endpoint that stores data
  String serverURL = "http://172.20.10.3:5000/store_data";  
  http.begin(serverURL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Build POST data – adjust parameter names as expected by your server script
  String postData = "analogValue=" + String(analogValue) +
                    "&voltage=" + String(voltage, 2) +
                    "&tdsValue=" + String(tdsValue, 2) +
                    "&turbidity=" + String(turbidity) +
                    "&gpsData=" + gpsData;
                    
  int httpResponseCode = http.POST(postData);
  if (httpResponseCode > 0) {
    Serial.print("Data sent successfully. Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Error sending data. HTTP response code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

//
// Handle the root page with a button to view & store data
//
void handleRoot() {
  String html = "<html><head><title>ESP32 Server</title></head><body>";
  html += "<h1>Hello from ESP32!</h1>";
  // When the button is clicked, the browser goes to the "/data" route.
  html += "<button onclick=\"window.location.href='/data'\">Show and Store Data</button>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

//
// Handle the "/data" route to display sensor data and store it in the database
//
void handleData() {
  // Read TDS sensor values
  int analogValue = analogRead(TDS_PIN);
  float voltage = (analogValue / ADC_RESOLUTION) * VREF;
  float tdsValue = voltage * 133.42;  // Adjust factor based on calibration
  
  // Read turbidity
  int turbidityValue = analogRead(TURB_PIN);
  int turbidity = map(turbidityValue, 0, ADC_RESOLUTION, 0, 100);
  
  // Collect GPS data (if available)
  String gpsData = "";
  if (gps.location.isValid()) {
    gpsData += "Lat:" + String(gps.location.lat(), 6) + ",Long:" + String(gps.location.lng(), 6);
  } else {
    gpsData += "No GPS";
  }
  if (gps.date.isValid()) {
    gpsData += ",Date:" + String(gps.date.day()) + "/" + String(gps.date.month()) + "/" + String(gps.date.year());
  }
  if (gps.time.isValid()) {
    gpsData += ",Time:" + String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second());
  }
  
  // Optionally send the data to a remote database
  sendToDatabase(analogValue, voltage, tdsValue, turbidity, gpsData);
  
  // Build the HTML content to display the sensor data
  String html = "<html><head><title>ESP32 Data</title></head><body>";
  html += "<h1>ESP32 Sensor Data</h1>";
  html += "<p><strong>Analog Value:</strong> " + String(analogValue) + "</p>";
  html += "<p><strong>Voltage:</strong> " + String(voltage, 2) + " V</p>";
  html += "<p><strong>TDS Value:</strong> " + String(tdsValue, 2) + " ppm</p>";
  html += "<p><strong>Turbidity:</strong> " + String(turbidity) + "</p>";
  html += "<h2>GPS Data:</h2>";
  html += "<p>" + gpsData + "</p>";
  html += "<br><button onclick=\"window.location.href='/data'\">Retore Data</button>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void setup() {
    Serial.begin(115200);     // Serial monitor
    GPS_Serial.begin(9600, SERIAL_8N1, 16, 17); // GPS Module (TX=16, RX=17)

    lcd.begin(16, 2);
    lcd.setBacklight(100);
    lcd.setCursor(0, 0);

    // Connect to network
    delay(100);
    WiFi.begin(ssid, password);
    int second = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      second++;  // Increment the second counter
      Serial.print("Connecting to WiFi... ");  
      Serial.print(second);  // Print the elapsed seconds
      Serial.print(" seconds elapsed.");
      Serial.println(WiFi.status());
    }
    Serial.println("Connected to WiFi!");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());

    // Set up the root route
    server.on("/", HTTP_GET, handleRoot);
    server.on("/data", HTTP_GET, handleData);

    server.begin();
}

void loop() {

    server.handleClient();

    if (SER_MON) Serial.print("\n\n\n\n\n\n\n");
    while (GPS_Serial.available()) {
        char c = GPS_Serial.read();
        gps.encode(c);
    }
    
    // Print GPS Data if available
    if (SER_MON && gps.location.isUpdated()) {
        Serial.print("Latitude: "); Serial.println(gps.location.lat(), 6);
        Serial.print("Longitude: "); Serial.println(gps.location.lng(), 6);
    }

    if (SER_MON && gps.date.isUpdated()) {
        Serial.print("Date: ");
        Serial.print(gps.date.day()); Serial.print("/");
        Serial.print(gps.date.month()); Serial.print("/");
        Serial.println(gps.date.year());
    }

    if (SER_MON && gps.time.isUpdated()) {
        Serial.print("Time (UTC): ");
        Serial.print(gps.time.hour()); Serial.print(":");
        Serial.print(gps.time.minute()); Serial.print(":");
        Serial.println(gps.time.second());
    }

    if (SER_MON && gps.speed.isUpdated()) {
        Serial.print("Speed (km/h): "); Serial.println(gps.speed.kmph());
    }

    if (SER_MON && gps.satellites.isUpdated()) {
        Serial.print("Satellites: "); Serial.println(gps.satellites.value());
    }
    
    // Read raw analog value
    int analogTDSValue = analogRead(TDS_PIN);
    int analogTurbidityValue = analogRead(TURB_PIN);
    
    // Convert analog value to voltage
    float voltage = (analogTDSValue / ADC_RESOLUTION) * VREF;

    // TDS calculation formula
    // TDS (ppm) = voltage * factor (based on calibration; assume a factor of 133.42)
    float tdsValue = (voltage * 133.42);  // Factor derived from the sensor datasheet

    // Turbidity calculation formula
    float turbidity = map(analogTurbidityValue, 0, ADC_RESOLUTION, 0, 100);

    if (SER_MON) {
      // Print results
      Serial.print("Analog TDS Value: ");
      Serial.println(analogTDSValue);

      Serial.print("Analog Turbidity Value: ");
      Serial.println(analogTurbidityValue);

      Serial.print("TDS Value: ");
      Serial.print(tdsValue, 2);  // Print TDS value with 2 decimal places
      Serial.println(" ppm");

      Serial.print("Turbidity Value: ");
      Serial.print(turbidity, 2);
      Serial.println(" ");
    }

    delay(100);

    lcd.setCursor(0, 0);
    lcd.print("Turbidity:");
    lcd.print("   ");
    lcd.setCursor(10, 0);
    lcd.print(turbidity); // replace with turbidity later

    lcd.setCursor(0, 1);
    lcd.print("TDS value:");
    lcd.print("   ");
    lcd.setCursor(10, 1);
    lcd.print(tdsValue); 

    delay(1000); // Update every second
}
