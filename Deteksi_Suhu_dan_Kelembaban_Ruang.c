#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define DHTPIN 2  // Pin untuk sensor DHT22
#define DHTTYPE DHT22  // Jenis sensor DHT
DHT dht(DHTPIN, DHTTYPE);

// LCD I2C address 0x27, 16x2 LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);  

// Pin untuk LED
int ledPinGreen = 16;  // LED Hijau terhubung ke pin 13
int ledPinRed = 4;    // LED Merah terhubung ke pin 14
int ledPinBlue = 17;   // LED Biru terhubung ke pin 15

// Ambang batas suhu untuk menentukan status LED
int thresholdHot = 30;   // Suhu terlalu panas > 30°C
int thresholdCold = 15;  // Suhu terlalu dingin < 15°C

// Wi-Fi credentials
const char* ssid = "Wokwi-GUEST";  
const char* password = "";  

// Google Apps Script URL
String serverURL = "https://script.google.com/macros/s/AKfycbwLstw6TG_n7-6hHiFEWf8s4A2KjSv3xt6mERKGmLsvQ6g8syEC9NONDwpsgd8K_8L5RA/exec";  // Ganti dengan URL Web App Google Apps Script Anda

void setup() {
  lcd.init();  // Inisialisasi LCD
  lcd.clear();
  lcd.backlight();  // Mengaktifkan backlight

  // Inisialisasi pin LED
  pinMode(ledPinGreen, OUTPUT);
  pinMode(ledPinRed, OUTPUT);
  pinMode(ledPinBlue, OUTPUT);

  dht.begin();  // Memulai sensor DHT
  Serial.begin(9600);  // Memulai komunikasi serial

  // Menghubungkan ke Wi-Fi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi");
}

void loop() {
  // Membaca kelembapan dan suhu dari DHT22
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Memeriksa jika pembacaan gagal (NaN)
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error!");
    return;  // Keluar jika pembacaan gagal
  }

  // Menampilkan suhu dan kelembapan di LCD
  lcd.setCursor(0, 0);
  lcd.print("Suhu: ");
  lcd.print(temperature);
  lcd.print((char)223);  // Menampilkan simbol derajat Celsius
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Kelembapan: ");
  lcd.print(humidity);
  lcd.print(" %");

  // Mengontrol LED berdasarkan suhu
  if (temperature > thresholdHot) {  // Jika suhu > 30°C
    digitalWrite(ledPinGreen, LOW);  // Matikan LED Hijau
    digitalWrite(ledPinRed, HIGH);   // Nyalakan LED Merah (terlalu panas)
    digitalWrite(ledPinBlue, LOW);   // Matikan LED Biru
  } else if (temperature < thresholdCold) {  // Jika suhu < 15°C
    digitalWrite(ledPinGreen, LOW);  // Matikan LED Hijau
    digitalWrite(ledPinRed, LOW);   // Matikan LED Merah
    digitalWrite(ledPinBlue, HIGH); // Nyalakan LED Biru (terlalu dingin)
  } else {  // Suhu normal (antara 15°C - 30°C)
    digitalWrite(ledPinGreen, HIGH);  // Nyalakan LED Hijau
    digitalWrite(ledPinRed, LOW);    // Matikan LED Merah
    digitalWrite(ledPinBlue, LOW);   // Matikan LED Biru
  }

  // Mengirimkan data ke Google Sheets dengan HTTP POST
  HTTPClient http;
  String postData = "temperature=" + String(temperature) + "&humidity=" + String(humidity);  // Membuat data yang akan dikirim
  Serial.print("Sending data to: ");
  Serial.println(serverURL);  // Menampilkan URL untuk debugging

  // Menyiapkan HTTP POST request
  http.begin(serverURL);  // Mulai koneksi HTTP ke server
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");  // Set content type untuk POST data

  int httpResponseCode = http.POST(postData);  // Mengirim HTTP POST request dengan data suhu dan kelembapan

  // Debugging HTTP Response Code
  Serial.print("HTTP Response Code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode == 200) {
    Serial.println("Data sent successfully");
  } else {
    Serial.println("Error sending data");
    String payload = http.getString();
    Serial.println("Response Payload: ");
    Serial.println(payload);  // Tampilkan respons dari server
  }

  http.end();  // Menutup koneksi HTTP

  delay(20000);  // Kirim data setiap 20 detik
}
