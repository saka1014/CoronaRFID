/*
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read new NUID from a PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 * 
 * Example sketch/program showing how to the read data from a PICC (that is: a RFID Tag or Card) using a MFRC522 based RFID
 * Reader on the Arduino SPI interface.
 * 
 * When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Arduino IDE
 * then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
 * you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial output
 * will show the type, and the NUID if a new card has been detected. Note: you may see "Timeout in communication" messages
 * when removing the PICC from reading distance too early.
 * 
 * @license Released into the public domain.
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
 */

#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN 5
#define RST_PIN 22

//----------------------google_WiFi_includes-----------------------------------------------------//
#include "WiFiClientSecure.h"                 //Bilbiotheken für Internet verbindung               
#include "WiFi.h"                             // --""--
#include <Wire.h>
#include <HTTPClient.h>                       //für HTTP anfragen an google

//----------------------EIGENE_DATEN_EINTRAGEN->WLAN und Google Script----------------------------// 
const char* ssid     = "UPC59941AC";          //Netzwerk mit dem Verbunden werden soll  
const char* password = "yct2rJ7xxTev";      //Netzwerk Passwort
const char*  server = "script.google.com";    // Server URL
// google script key
const char* g_key = "AKfycbzLFQ8LO80gpmk_90JbwdvLogVJ8JZOWiDE7nXmBG3JEJqA_vQ"; //Identität des Google Scripts
//-------------------------------------BuzzerVariablen-------------------------------------------// 
int freq = 255;
int channel = 0;
int resolution = 8;
int dutyCycle = 0;
//-------------------------------------deepsleep-------------------------------------------// 
#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex
RTC_DATA_ATTR int bootCount = 0;


MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
RTC_DATA_ATTR byte nuidPICC[4];

RTC_DATA_ATTR byte nuidPICCReg[20][4];

void setup() { 
//-------------------------Setup_WiFi_verbindung----------------------------------------------------------//
 Serial.begin(115200);                                 //Baudrate von WLAN
  Serial.println("Warte auf Verbindung");
  WiFi.begin(ssid, password);                           //Verbindung beginnen

  int abbruch=0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    abbruch++;
    if (abbruch==20)ESP.restart();                      //nach belibigerzeit ohne Verbindung neustarten
  }
  Serial.println("");
  Serial.print("IP Addresse: ");
  Serial.println(WiFi.localIP());

//-------------------------------------SetupDSleep-------------------------------------------// 
  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  print_wakeup_reason();
  
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,1); //1 = High, 0 = Low
//-------------------------Setup_SPI----------------------------------------------------------// 
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  if(bootCount==1)
  {
    Serial.println("Overwrite Reg");
    for(byte i = 0; i < 20; i++)
    {
      for(byte j = 0; j < 4; j++)
      {
        nuidPICCReg[i][j] = 0xFF;
      }
    }
  }
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  //Serial.println(F("This code scan the MIFARE Classsic NUID."));
  //Serial.print(F("Using the following key:"));
  //printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
//-------------------------------------SetupBuzzer-------------------------------------------// 
  Serial.begin(115200);
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(12, channel);

  Serial.println("Scanning");
}

 
void loop() {

  //Serial.println("Alive");
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  
  if(searchTAG(rfid) == false) 
  {
    Serial.println(F("You are Logged In."));
    buzzerlogin();

    // Store NUID into nuidPICC array
    regTag(rfid); //reg Card
    for (byte i = 0; i < 4; i++) 
    {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
   
    Serial.println(F("The NUID tag is:"));
    //Serial.print(F("In hex: "));
    ///printHex(rfid.uid.uidByte, rfid.uid.size);
    //Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    //Go to sleep now
    Serial.println();
    delay(1000);
    Serial.println("Deep Sleep");
    esp_deep_sleep_start();
  }
  else
  {
    deleteTAG(rfid);
    Serial.println();
    Serial.println("Deep Sleep");
    delay(1000);
    esp_deep_sleep_start();
  }

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}


/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

bool searchTAG(MFRC522 rfid)
{
  bool search = false;
  for(byte i = 0; i < 20; i++)
  {
    if(nuidPICCReg[i][0] == rfid.uid.uidByte[0] && 
       nuidPICCReg[i][1] == rfid.uid.uidByte[1] && 
       nuidPICCReg[i][2] == rfid.uid.uidByte[2] && 
       nuidPICCReg[i][3] == rfid.uid.uidByte[3]   )
    {
      search = true;
      return true;
    }
  }
  return false; 
}

bool regTag(MFRC522 rfid)
{
  for(byte i = 0; i < 20; i++)
  {
    if(nuidPICCReg[i][0] == 0xFF && 
       nuidPICCReg[i][1] == 0xFF && 
       nuidPICCReg[i][2] == 0xFF && 
       nuidPICCReg[i][3] == 0xFF)
    {
      nuidPICCReg[i][0] = rfid.uid.uidByte[0]; 
      nuidPICCReg[i][1] = rfid.uid.uidByte[1];
      nuidPICCReg[i][2] = rfid.uid.uidByte[2];
      nuidPICCReg[i][3] = rfid.uid.uidByte[3];
      String ID = String(rfid.uid.uidByte[0]); 
             ID += String(rfid.uid.uidByte[1]); 
             ID += String(rfid.uid.uidByte[2]); 
             ID += String(rfid.uid.uidByte[3]);
      cloudfkt(ID);
      return true;
    }
  }
  return false;
}

bool deleteTAG(MFRC522 rfid)
{
  for(byte i = 0; i < 20; i++)
  {
    if(nuidPICCReg[i][0] == rfid.uid.uidByte[0] && 
       nuidPICCReg[i][1] == rfid.uid.uidByte[1] && 
       nuidPICCReg[i][2] == rfid.uid.uidByte[2] && 
       nuidPICCReg[i][3] == rfid.uid.uidByte[3]   )
    {
      nuidPICCReg[i][0] = 0xFF; 
      nuidPICCReg[i][1] = 0xFF;
      nuidPICCReg[i][2] = 0xFF;
      nuidPICCReg[i][3] = 0xFF;
      Serial.println(F("Card is Logged out."));
      buzzerlogoff();
      return true;
    }
  }
  return false; 
}

void cloudfkt(String ID)
{
  String URL="https://script.google.com/macros/s/";         //skript server
  URL += g_key;                                             //skript key(Identität)
  URL += "/exec?";
  URL += "1_Spalte=";                                       //1.Spalte
  URL += ID ;                                               //ID
 //Serial.println(URL);
 
 if ((WiFi.status() == WL_CONNECTED)) 
 { //Noch verbunden?
 
    HTTPClient http;
 
    http.begin(URL); //Specify the URL                                //verbindung zum Skript aufbauen
    int httpCode = http.GET();                                        //sende den Link an das Skript
 
    if (httpCode > 0) { //Check for the returning code                //Antwort von Website
 
        String payload = http.getString();
        //Serial.println(httpCode);
        //Serial.println(payload);
      }
 
    else {
      Serial.println("Error on HTTP request");
    }
 
    http.end(); //Free the resources
  Serial.println("Sended to Server"); 
  }
  else
  {
  Serial.println("No connection!!!");
  //ESP.restart(); 
  }  
}

void buzzerlogin()
{
   ledcWriteTone(channel, 150);

   dutyCycle = 300;
   ledcWrite(channel, dutyCycle);
   delay(50);
   ledcWrite(channel, 0);
}

void buzzerlogoff()
{
   ledcWriteTone(channel, 150);

   dutyCycle = 300;
   ledcWrite(channel, dutyCycle);
   delay(50);
   ledcWrite(channel, 0);
   delay(50);
   ledcWrite(channel, dutyCycle);
   delay(50);
   ledcWrite(channel, 0);
}

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}
