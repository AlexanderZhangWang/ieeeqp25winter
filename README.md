# Water Purity Testing System

This project implements a comprehensive water purity testing system using ESP32, various sensors, and a web interface for data visualization. The system measures Total Dissolved Solids (TDS), turbidity, and includes GPS tracking capabilities.

## System Components

### Hardware Requirements
- ESP32 Microcontroller
- TDS Sensor
- Turbidity Sensor
- GPS Module (connected via UART)
- LCD Display (I2C PCF8574)
- Power Supply (5V)

### Software Components
1. **ESP32 Firmware** (`waterpuritytestercode.ino`)
   - Reads sensor data (TDS and turbidity)
   - Collects GPS location data
   - Displays readings on LCD
   - Hosts a web server for data access
   - Sends data to Flask backend server

2. **Flask Backend Server** (`app.py`)
   - Receives and stores sensor data in SQLite database
   - Provides REST API endpoints for data access
   - Implements CORS for web interface compatibility

3. **Web Interface** (`index.html`)
   - Interactive map showing measurement locations
   - Real-time data visualization
   - Refresh and center map controls

4. **IP Address Utility** (`get_api.py`)
   - Helper script to get local IP address

## Setup Instructions

### 1. Hardware Setup
1. Connect the TDS sensor to GPIO pin 36
2. Connect the turbidity sensor to GPIO pin 39
3. Connect the GPS module to UART2 (TX: GPIO 16, RX: GPIO 17)
4. Connect the LCD display via I2C (default address: 0x27)

### 2. ESP32 Setup
1. Install the following Arduino libraries:
   - TinyGPS++
   - LiquidCrystal_PCF8574
   - WiFi
   - WebServer
   - HTTPClient

2. Update WiFi credentials in `waterpuritytestercode.ino`:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```

3. Update the Flask server URL in `sendToDatabase()` function to match your server's IP address.

### 3. Flask Server Setup
1. Install required Python packages:
   ```bash
   pip install -r requirements.txt
   ```

2. Run the Flask server:
   ```bash
   python app.py
   ```
   The server will run on port 5000 by default.

### 4. Web Interface Setup
1. Update the fetch URL in `index.html` to match your Flask server's IP address:
   ```javascript
   fetch('http://YOUR_IP:5000/get_data')
   ```

2. Open `index.html` in a web browser to view the data visualization.

## Usage

1. Power up the ESP32 system and ensure it's connected to WiFi
2. The LCD will display real-time TDS and turbidity readings
3. Access the ESP32's web interface through its IP address
4. View the collected data on the map interface by opening `index.html`
5. Use the "Refresh Data" button to update the map with new measurements
6. Use the "Center Map" button to focus on your current location

## Data Storage

The system stores the following data points in the SQLite database:
- Analog sensor values
- Voltage readings
- TDS values (ppm)
- Turbidity values
- GPS coordinates (latitude and longitude)
- Timestamp of measurements
- GPS date and time

## Troubleshooting

1. If the ESP32 fails to connect to WiFi:
   - Check WiFi credentials
   - Ensure the network is within range
   - Verify the network supports ESP32 connection

2. If data isn't appearing on the map:
   - Check the Flask server is running
   - Verify the IP addresses in the configuration
   - Check browser console for JavaScript errors
   - Ensure GPS module has a clear view of the sky

3. If sensor readings seem incorrect:
   - Verify sensor connections
   - Check power supply voltage
   - Calibrate sensors if necessary
