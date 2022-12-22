#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define LED_RED   32
#define LED_GREEN 33
#define LED_BLUE  25

#define PIEZO_PIN 26

#define BUTTON    34

#define RST_PIN   15
#define SS_PIN    5

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
LiquidCrystal_I2C lcd(0x27,16,2);  // Устанавливаем дисплей
WiFiClient wifi;

const char* ssid = "qwerty";
const char* pass = "1298347612";

int balance = 0;

void blink_led_red(){
  for (int i = 0; i < 3; i++){
    digitalWrite(LED_RED, HIGH);
    tone(PIEZO_PIN, 493, 100);
    delay(100);
    digitalWrite(LED_RED, LOW);
    tone(PIEZO_PIN, 493, 100);
    delay(100);
  }
}

void blink_led_green(){
  for (int i = 0; i < 3; i++){
    digitalWrite(LED_GREEN, HIGH);
    tone(PIEZO_PIN, 493, 100);
    delay(100);
    digitalWrite(LED_GREEN, LOW);
    tone(PIEZO_PIN, 493, 100);
    delay(100);
  }
}

void blink_led_blue(){
  for (int i = 0; i < 3; i++){
    digitalWrite(LED_BLUE, HIGH);
    tone(PIEZO_PIN, 493, 100);
    delay(100);
    digitalWrite(LED_BLUE, LOW);
    tone(PIEZO_PIN, 493, 100);
    delay(100);
  }
}

void off_led(){
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, LOW);
}

// void blink(){
//     digitalWrite(LED_RED, HIGH);
//     delay(200);
//     digitalWrite(LED_GREEN, HIGH);
//     delay(200);
//     digitalWrite(LED_BLUE, HIGH);
//     delay(200);
//     digitalWrite(LED_BLUE, LOW);
//     delay(200);
//     digitalWrite(LED_GREEN, LOW);
//     delay(200);
//     digitalWrite(LED_RED, LOW);
//     delay(200);
// }

void update_balance(int *balance){
  int buf = (*balance);
  int len_buffer = 16;
  byte buffer[len_buffer];
  for (int i = 0; i < len_buffer; i++){
    if (buf >= 0) buffer[i] = buf+'0';
    else buffer[i] = ' ';
  }
  MFRC522::MIFARE_Key key;
  MFRC522::StatusCode status;
  byte block = 1;
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("PCD_Authenticate() success: "));

  // Write block
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));

}

void buy(int *balance){

  blink_led_green();

  int price = 123;
  lcd.setCursor(0, 0);
 
  
  lcd.print("Welcome");
  delay(100);
  lcd.setCursor(2, 1);
  lcd.print("Esp32");

  lcd.setCursor(8, 1);
  lcd.print(price);
  lcd.print("$");

  if ((*balance) >= price){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("thanks for");
    lcd.setCursor(1, 0);
    lcd.print("the purchase :)");
    (*balance) -= price;
    update_balance(balance);
    blink_led_green();
    delay(5000);
  }
  else if ((*balance) < price){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Insufficient");
    lcd.setCursor(1, 0);
    lcd.print("funds :(");
    blink_led_red();
    delay(5000);
  }

  lcd.clear();

  blink_led_green();

}

// void dump_card(){
//   // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
// 	if ( ! mfrc522.PICC_IsNewCardPresent()) {
// 		return;
// 	}

// 	// Select one of the cardsq
// 	if ( ! mfrc522.PICC_ReadCardSerial()) {
// 		return;
// 	}

//   off_led();
//   blink_led_green();
  
//   mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  
//   blink_led_green();
//   delay(500);
// }

void read_data(int *balance){
  
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  blink_led_blue();
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

  byte buffer1[18];

  block = 4;
  len = 18;

  //------------------------------------------- GET FIRST NAME
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  byte buffer2[18];
  block = 1;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // //PRINT LAST NAME
  // for (uint8_t i = 0; i < 16; i++) {
  //   Serial.write(buffer2[i] );
  // }

  //----------------------------------------

  int buff = 0;
  for (int i = 0; i < 18; i++){
    if (buffer2[i] != '$') {
      buff += buffer2[i]-'0';
      buff *= 10; 
    }
    else if (buffer2[i] == '$'){
      buff /= 10; 
      break;
    }
  }

  if ((*balance) != buff){
    (*balance) = buff;
  }

  delay(100); //change value if you want to read cards faster

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  blink_led_blue();

}

void setup() {
  Serial.begin(230400);
  
  WiFi.begin(ssid, pass);
  Serial.println("Connecting");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay (1000);
    Serial.print(".");
  }

  Serial.println("Connected");
  
  
  //LED
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED,   OUTPUT);
  pinMode(LED_BLUE,  OUTPUT);
  //BUTTONS
  pinMode(BUTTON, INPUT);

  while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin();			// Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522
	delay(4);				// Optional delay. Some board do need more time after init to be ready, see Readme
	mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

  lcd.init();                     
  lcd.backlight();
}

void loop() {

  if (digitalRead(BUTTON) == LOW) {
    delay(100);
    if (digitalRead(BUTTON) == LOW){
      read_data(&balance);
      Serial.println(balance);
    }
    else buy(&balance);
  }
  else buy(&balance);
  int price = 123;
  lcd.setCursor(0, 0);
 
  
  lcd.print("Welcome");
  delay(100);
  lcd.setCursor(2, 1);
  lcd.print("Esp32");


  lcd.setCursor(8, 1);
  lcd.print(price);
  lcd.print("$");
  String payload="";
  
  if (WiFi.status() == WL_CONNECTED) 
  {
    HTTPClient http;
    String server_name = "https://iot-php-tsaruk.azurewebsites.net/WebAzure/esp32_test/getParameters.php?"; // nazov vasho webu a web stranky, ktoru chcete nacitat
    server_name += "balance="; // nazov premennej na webe
    server_name += balance; // hodnota premmenej
    http.begin(server_name.c_str());
    int httpCode = http.GET(); // http code
    if (httpCode>0) 
    {
      payload= http.getString();
      delay(100); // 0.1 sekunda 
    }
    http.end();
  }

} 
