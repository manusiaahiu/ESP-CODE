#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

#define DHTPIN 19 // Out dht pin 19 esp 
#define DHTTYPE DHT11 
DHT dht11(DHTPIN, DHTTYPE);

#define SOIL_SENSOR_PIN 34 // AO soil moisture pin 34 esp
#define RELAY_PIN 5        // IN relay pin 5 esp

const char* ssid = "titituit"; 
const char* password = "uwauuwau"; 

String dht11URL = "http://100.29.163.134/connect.php";
String soilURL = "http://100.29.163.134/koneksi.php";

int temperature = 0;
int humidity = 0;
int soilMoisture = 0;
String soilStatus = "";

void setup() {
  Serial.begin(115200);
  pinMode(SOIL_SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  
  dht11.begin(); 
  connectWiFi();`e2c
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  Load_DHT11_Data();
  Load_SoilMoisture_Data();

  // Mengirim data DHT1
  String dht11PostData = "temperature=" + String(temperature) + "&humidity=" + String(humidity);
  sendData(dht11URL, dht11PostData);

  // Mengirim data Soil Moisture 
  if (soilMoisture < 30) {
    soilStatus = "Tanah Kering";
  } else if (soilMoisture >= 30 && soilMoisture < 60) {
    soilStatus = "Tanah Lembab";
  } else {
    soilStatus = "Tanah Basah";
  }
  
  String soilPostData = "kelmbapan=" + String(soilMoisture) + "&status=" + soilStatus;
  sendData(soilURL, soilPostData);

  delay(3000); // delay perngiriman
}

void Load_DHT11_Data() {
  temperature = dht11.readTemperature(); // Celsius
  humidity = dht11.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    temperature = 0;
    humidity = 0;
  }

  Serial.printf("Temperature: %d °C\n", temperature);
  Serial.printf("Humidity: %d %%\n", humidity);
}

void Load_SoilMoisture_Data() {
  int raw_value = analogRead(SOIL_SENSOR_PIN);
  
  // Print Nilai mentah 
  Serial.print("Raw Soil Moisture Value: ");
  Serial.println(raw_value);
  
  // mengasumsikan 0 adalah basah dan 4095 adalah kering 
  soilMoisture = map(raw_value, 0, 4095, 100, 0); // Membalikkan pembacaan analog menjadi persentase
  soilMoisture = constrain(soilMoisture, 0, 100); // Memastikan nilainya adalah antara 0 - 100

  Serial.print("Moisture: ");
  Serial.print(soilMoisture);
  Serial.println("%");

  if (soilMoisture < 30 && temperature > 30) {  //PENYIRAMAN DILAKUKAN
    digitalWrite(RELAY_PIN, LOW); // mengaktifkan relay
    Serial.println("Penyiraman Aktif: Tanah Kering dan Suhu Tinggi");
  } else {
    digitalWrite(RELAY_PIN, HIGH); //mematikan relay
  }
}

void connectWiFi() {
  WiFi.mode(WIFI_OFF);
  delay(1000);
  WiFi.mode(WIFI_STA);
  
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
    
  Serial.print("Connected to: "); Serial.println(ssid);
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
}

void sendData(String url, String postData) {
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  int httpCode = http.POST(postData);
  String payload = "";

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    }
  } else {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();  // Menutup koneksi

  Serial.print("URL: "); Serial.println(url); 
  Serial.print("Data: "); Serial.println(postData);
  Serial.print("httpCode: "); Serial.println(httpCode);
  Serial.print("payload: "); Serial.println(payload);
  Serial.println("--------------------------------------------------");
}
