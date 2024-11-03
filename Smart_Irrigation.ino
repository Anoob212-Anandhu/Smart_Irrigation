#include <ESP8266WiFi.h>
#include <DHT.h>

#define DHTPIN D3         // Humidity sensor 
#define DHTTYPE DHT22     // Sensor type: DHT22
#define MOISTURE_PIN A0   // Soil moisture 
#define RAIN_PIN D2       // Rain sensor 
#define RELAY_PIN D0      // Relay 

// WiFi credentials
const char* ssid = "SSID";       // Replace with your WiFi NAME
const char* password = "PASSWORD";    // Replace with your WiFi password

DHT dht(DHTPIN, DHTTYPE); 
WiFiServer server(80);    

// Function humidity humidity level
String getHumidityLevel(float humidity) {
    if (humidity < 30) return "Low";
    else if (humidity < 60) return "Medium";
    else return "High";
}

// Function for temperature level
String getTemperatureLevel(float temperature) {
    if (temperature < 15) return "Cold";
    else if (temperature <= 25) return "Cool";
    else if (temperature <= 35) return "Warm";
    else return "Hot";
}

// FUNCTION FOR MOISTURE LEVEL
String getMoistureLevel(int moisture) {
    if (moisture < 300) return "Dry";
    else if (moisture < 700) return "Moist";
    else return "Wet";
}

void setup() {
    Serial.begin(115200);                 
    WiFi.begin(ssid, password);           
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");                
    }
    Serial.println("\nConnected to WiFi"); 

    server.begin();                        
    dht.begin();                           
    pinMode(MOISTURE_PIN, INPUT);          
    pinMode(RAIN_PIN, INPUT);              
    pinMode(RELAY_PIN, OUTPUT);            
    digitalWrite(RELAY_PIN, LOW);          
    Serial.println("DHT sensor initialized");
}

// Loop function
void loop() {
    delay(2000);  // Add a delay for stable sensor readings

    WiFiClient client = server.available(); 
    if (!client) return;                    

    // Sensor readings
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    int moisture = analogRead(MOISTURE_PIN);
    int rainStatus = digitalRead(RAIN_PIN); 

    // Check if readings are valid
    if (isnan(humidity) || isnan(temperature)) {
        Serial.println("Failed to read from DHT sensor!");
    } else {
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.println("%");
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.println("°C");
    }

    // Map sensor values to percentage for display
    int temperatureLevel = map(temperature, 0, 50, 0, 100); 
    int moistureLevel = map(moisture, 0, 1023, 0, 100);     

    while (client.connected()) { 
        if (client.available()) { 
            String request = client.readStringUntil('\r'); 
            client.flush(); 

            
            if (request.indexOf("/relay/on") != -1) {
                digitalWrite(RELAY_PIN, HIGH); 
                Serial.println("Relay ON");
            } 
            else if (request.indexOf("/relay/off") != -1) {
                digitalWrite(RELAY_PIN, LOW); 
                Serial.println("Relay OFF");
            }

            
            String html = "<!DOCTYPE html><html><head><title>Smart Irrigation System</title>"
                          "<style>"
                          "body { font-family: Arial; text-align: center; }"
                          ".container { display: flex; justify-content: center; gap: 20px; margin-top: 20px; }"
                          ".thermometer { width: 30px; height: 150px; border: 2px solid black; border-radius: 15px; position: relative; overflow: hidden; }"
                          ".level { position: absolute; bottom: 0; width: 100%; border-radius: 15px; }"
                          ".humidity { background: blue; }"         // Blue color for Humidity
                          ".temperature { background: yellow; }"    // Yellow color for Temperature
                          ".moisture { background: rgb(184, 0, 31); }" // Red color for Moisture
                          "</style>"
                          "</head><body><h1>Smart Irrigation System</h1>";

            // Display sensor values
            html += "<p>Humidity: " + String(humidity) + "% (" + getHumidityLevel(humidity) + ")</p>";
            html += "<p>Temperature: " + String(temperature) + "°C (" + getTemperatureLevel(temperature) + ")</p>";
            html += "<p>Soil Moisture: " + String(moisture) + " (" + getMoistureLevel(moisture) + ")</p>";
            html += "<p>Rain Status: " + String(rainStatus == HIGH ? "No Rain" : "Rain Detected") + "</p>";

            // Thermometers container
            html += "<div class='container'>";

            // Humidity Thermometer
            html += "<div style='text-align: center;'><p>Humidity</p>";
            html += "<div class='thermometer'><div class='level humidity' style='height:" + String(humidity) + "%;'></div></div>";
            html += "<p>" + String(humidity) + "%</p></div>";

            // Temperature Thermometer
            html += "<div style='text-align: center;'><p>Temperature</p>";
            html += "<div class='thermometer'><div class='level temperature' style='height:" + String(temperatureLevel) + "%;'></div></div>";
            html += "<p>" + String(temperature) + "°C</p></div>";

            // Soil Moisture Thermometer
            html += "<div style='text-align: center;'><p>Moisture</p>";
            html += "<div class='thermometer'><div class='level moisture' style='height:" + String(moistureLevel) + "%;'></div></div>";
            html += "<p>" + String(moisture) + "</p></div>";

            html += "</div>"; 


            html += "<p><a href=\"/relay/on\"><button style=\"padding:10px 20px;\">Turn Motor ON</button></a>";
            html += "<a href=\"/relay/off\"><button style=\"padding:10px 20px;\">Turn Motor OFF</button></a></p>";
            html += "</body></html>";
client.println("HTTP/1.1 200 OK");
client.println("Content-type:text/html; charset=utf-8"); 
client.println("Connection: close");
client.println();
client.println(html);
client.stop(); 
        }
    }
}
