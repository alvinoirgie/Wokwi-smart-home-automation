#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <ESP32Servo.h>

// 1. Inisialisasi LCD 1602 I2C
// Alamat I2C umum Wokwi: 0x27, 16 kolom, 2 baris
LiquidCrystal_I2C lcd(0x27, 16, 2);

// 2. Inisialisasi Servo
Servo myServo;
const int SERVO_PIN = 18;

// 3. Pin PIR Sensor & LED
const int PIR_PIN = 13;
const int LED_PIN = 12;

// 4. Konfigurasi Keypad 4x4 (Harus Sesuai Pin diagram.json)
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Urutan pin Baris (R1, R2, R3, R4) dan Kolom (C1, C2, C3, C4)
byte rowPins[ROWS] = {19, 23, 5, 17}; 
byte colPins[COLS] = {16, 4, 2, 15};  

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Variable Sistem PIN
String secretPassword = "1234"; // PIN default untuk membuka pintu
String inputPassword = "";

void setup() {
  Serial.begin(115200);

  // --- Setup LCD ---
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("  SMART  HOME  ");
  lcd.setCursor(0, 1);
  lcd.print("  INITIALIZING ");
  delay(2000);
  lcd.clear();

  // --- Setup Servo ---
  ESP32PWM::allocateTimer(0);
  myServo.setPeriodHertz(50);    // Standard 50Hz servo
  myServo.attach(SERVO_PIN, 500, 2400);
  myServo.write(0);              // Posisi awal: Pintu Terkunci (0 Derajat)

  // --- Setup PIR & LED ---
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  updateDisplay();
}

void loop() {
  // --------------------------------------------------
  // 1. DOKUMENTASI OTOMATISASI LAMPU (PIR -> LED)
  // --------------------------------------------------
  int pirState = digitalRead(PIR_PIN);
  if (pirState == HIGH) {
    digitalWrite(LED_PIN, HIGH); // Menyala saat gerakan terdeteksi
  } else {
    digitalWrite(LED_PIN, LOW);  // Padam jika sepi
  }

  // --------------------------------------------------
  // 2. DOKUMENTASI KEAMANAN PINTU (KEYPAD -> SERVO)
  // --------------------------------------------------
  char key = keypad.getKey();
  if (key) {
    Serial.print("Tombol Ditekan: ");
    Serial.println(key);

    if (key == '#') {
      // Tombol '#' = Masukkan PIN / Submit
      checkPassword();
    } else if (key == '*') {
      // Tombol '*' = Reset / Hapus karakter
      inputPassword = "";
      updateDisplay();
    } else {
      // Tambahkan angka/karakter ke input
      if (inputPassword.length() < 4) {
        inputPassword += key;
        updateDisplay();
      }
    }
  }
}

// Fungsi Menampilkan Bintang PIN di LCD
void updateDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Masukkan PIN:");
  lcd.setCursor(0, 1);
  
  for (int i = 0; i < inputPassword.length(); i++) {
    lcd.print("*");
  }
}

// Fungsi Verifikasi PIN
void checkPassword() {
  lcd.clear();
  if (inputPassword == secretPassword) {
    // PIN BENAR
    lcd.setCursor(0, 0);
    lcd.print("Akses Diterima!");
    lcd.setCursor(0, 1);
    lcd.print("Pintu Terbuka");
    
    myServo.write(90); // Servo berputar 90 Derajat (Pintu Buka)
    delay(4000);       // Tahan terbuka selama 4 detik
    
    myServo.write(0);  // Kunci kembali pintu
    inputPassword = "";
    updateDisplay();
  } else {
    // PIN SALAH
    lcd.setCursor(0, 0);
    lcd.print("Akses Ditolak!");
    lcd.setCursor(0, 1);
    lcd.print("PIN Salah!");
    delay(2000);
    
    inputPassword = "";
    updateDisplay();
  }
}