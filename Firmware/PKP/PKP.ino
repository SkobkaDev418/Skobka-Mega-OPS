#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#include <SPI.h>
#include <MFRC522.h>
#include <avr/wdt.h>
#include <EEPROM.h>

// === RFID (MFRC522) ===
#define RST_PIN 49
#define SS_PIN  53
MFRC522 mfrc522(SS_PIN, RST_PIN);

// === DFPlayer ===
DFRobotDFPlayerMini myDFPlayer;

// === Конфигурация ===
const int FIRE_TRACK      = 1;
const int SEC_TRACK       = 2;
const int buzzerPin       = 6;
const int ledPin          = 4;      
const int exitSignPin     = 5;      
const int zoneLed7        = 7;      
const int zoneLed8        = 8;      
const int onboardLed      = 13;     
const uint8_t zonePins[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
const uint8_t zoneLedPins[8] = {22, 23, 24, 25, 26, 27, 28, 29};

const int SHORT_LIMIT     = 250;
const int OPEN_LIMIT      = 1000;

// === EEPROM настройки ===
#define EEPROM_MAGIC 0x5A
#define MAX_CARDS 10
#define CARD_SIZE 16

// === Состояния ===
bool isFireActive = false, isSecActive = false, isArmed = false, armingProcess = false;
bool isFaultActive = false;
byte activeZone = 255;
unsigned long armingStartTime = 0, serialTimer = 0;
unsigned long beepTimer = 0;
int currentPlayingTrack = 0;
bool dfTrackStarted = false;
bool alarmMemory[8] = {false};

// === RFID ===
String authorizedCards[MAX_CARDS];
int cardCount = 0;
bool rfidAddMode = false;
unsigned long rfidBeepTimer = 0;

// === EEPROM функции ===
void saveCardsToEEPROM() {
  EEPROM.write(0, EEPROM_MAGIC);
  EEPROM.write(1, cardCount);

  int addr = 2;
  for (int i = 0; i < cardCount; i++) {
    String card = authorizedCards[i];
    int len = min(card.length(), CARD_SIZE - 1);
    EEPROM.write(addr++, len);
    for (int j = 0; j < len; j++) {
      EEPROM.write(addr++, card[j]);
    }
  }
}

void loadCardsFromEEPROM() {
  if (EEPROM.read(0) != EEPROM_MAGIC) {
    Serial.println(F("EEPROM: нет сохранённых данных"));
    return;
  }

  cardCount = EEPROM.read(1);
  if (cardCount > MAX_CARDS || cardCount < 0) {
    cardCount = 0;
    return;
  }

  int addr = 2;
  for (int i = 0; i < cardCount; i++) {
    int len = EEPROM.read(addr++);
    if (len <= 0 || len >= CARD_SIZE) {
      cardCount = 0;
      break;
    }
    char buffer[CARD_SIZE];
    for (int j = 0; j < len; j++) {
      buffer[j] = EEPROM.read(addr++);
    }
    buffer[len] = '\0';
    authorizedCards[i] = String(buffer);
  }

  if (cardCount > 0) {
    Serial.print(F("EEPROM: загружено карт: "));
    Serial.println(cardCount);
  }
}

void saveCard(String uid) {
  if (cardCount < MAX_CARDS) {
    authorizedCards[cardCount++] = uid;
    saveCardsToEEPROM();
    Serial.print(F("Карта добавлена и сохранена: "));
    Serial.println(uid);
  }
}

bool isCardAuthorized(String uid) {
  for (int i = 0; i < cardCount; i++) {
    if (authorizedCards[i] == uid) return true;
  }
  return false;
}

void deleteCard(int index) {
  if (index >= 1 && index <= cardCount) {
    for (int i = index - 1; i < cardCount - 1; i++) {
      authorizedCards[i] = authorizedCards[i + 1];
    }
    cardCount--;
    saveCardsToEEPROM();
    Serial.println(F("Карта удалена"));
  }
}

String readCommand() {
  String cmd = "";
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') break;
    cmd += c;
  }
  return cmd;
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);

  // Watchdog
  wdt_disable();
  delay(100);
  wdt_enable(WDTO_2S);

  // Загрузка карт из EEPROM
  loadCardsFromEEPROM();

  SPI.begin();
  mfrc522.PCD_Init();
  delay(4);

  pinMode(onboardLed, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(exitSignPin, OUTPUT);
  pinMode(zoneLed7, OUTPUT);
  pinMode(zoneLed8, OUTPUT);

  for (int i = 0; i < 8; i++) {
    pinMode(zoneLedPins[i], OUTPUT);
    digitalWrite(zoneLedPins[i], LOW);
  }

  if (!myDFPlayer.begin(Serial1, false, true)) {
    Serial.println(F("DFPlayer: FAIL"));
  } else {
    myDFPlayer.volume(30);
    Serial.println(F("DFPlayer: OK"));
  }

  Serial.println(F("=== Mega OPS (ВСЁ ПО КАРТЕ) ==="));
  Serial.println(F("Команды: RFID new, RFID list, RFID N del"));
}

void loop() {
  wdt_reset();

  unsigned long currentMillis = millis();
  digitalWrite(onboardLed, (currentMillis / 500) % 2);

  // === Команды от ПК ===
  if (Serial.available()) {
    String cmd = readCommand();
    cmd.trim();
    
    if (cmd == "RFID new") {
      rfidAddMode = true;
      Serial.println(F("Режим добавления карты. Приложите карту..."));
    }
    else if (cmd == "RFID list") {
      Serial.println(F("Список карт:"));
      for (int i = 0; i < cardCount; i++) {
        Serial.print(i + 1);
        Serial.print(F(" - "));
        Serial.println(authorizedCards[i]);
      }
      if (cardCount == 0) Serial.println(F("Нет сохранённых карт"));
    }
    else if (cmd.endsWith(" del")) {
      int spaceIndex = cmd.indexOf(' ');
      if (spaceIndex != -1) {
        String numStr = cmd.substring(spaceIndex + 1, cmd.length() - 4);
        int num = numStr.toInt();
        deleteCard(num);
      }
    }
  }

  // === Опрос RFID ===
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
      uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();

    if (rfidAddMode) {
      saveCard(uid);
      tone(buzzerPin, 2000, 300);
      rfidAddMode = false;
    }
    else if (isCardAuthorized(uid)) {
      // СБРОС ВСЕХ ТРЕВОГ ПО КАРТЕ
      if (isFireActive || isSecActive || isFaultActive) {
        isFireActive = isSecActive = isFaultActive = false;
        myDFPlayer.stop();
        noTone(buzzerPin);
        currentPlayingTrack = 0;
        dfTrackStarted = false;
        for (int i = 0; i < 8; i++) alarmMemory[i] = false;
        activeZone = 255;
        tone(buzzerPin, 1000, 300);
        Serial.println(F(">>> ТРЕВОГА СБРОШЕНА (RFID) <<<"));
      }
      // УПРАВЛЕНИЕ ОХРАНОЙ
      else if (isArmed) {
        isArmed = false;
        armingProcess = false;
        tone(buzzerPin, 1000, 200);
        Serial.println(F(">>> СНЯТО С ОХРАНЫ (RFID) <<<"));
      }
      else {
        armingProcess = true;
        armingStartTime = currentMillis;
        tone(buzzerPin, 1500, 200);
        Serial.println(F(">>> ПОСТАНОВКА НА ОХРАНУ (RFID) <<<"));
      }
    }
    else {
      tone(buzzerPin, 500, 200);
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }

  // === Звук в режиме добавления ===
  if (rfidAddMode && (currentMillis - rfidBeepTimer >= 500)) {
    rfidBeepTimer = currentMillis;
    tone(buzzerPin, 1000, 100);
  }

  // === ЛОГИКА ЗОН И ТРЕВОГ ===
  bool rawFire = false, rawSec = false, isAnyFault = false;
  byte currentFaultZone = 255;
  byte currentAlarmZone = 255;

  bool printStatus = (currentMillis - serialTimer >= 1000);
  if (printStatus) serialTimer = currentMillis;

  for (int i = 0; i < 8; i++) {
    int val = analogRead(zonePins[i]);
    if (val >= OPEN_LIMIT) isAnyFault = true;
    if (val < SHORT_LIMIT) {
      if (i < 6) { 
        rawFire = true; 
        currentAlarmZone = i; 
        if (!alarmMemory[i]) alarmMemory[i] = true; 
      } else if (isArmed && !armingProcess) { 
        rawSec = true; 
        currentAlarmZone = i; 
        if (!alarmMemory[i]) alarmMemory[i] = true; 
      }
    }
  }

  // Вывод статуса
  if (printStatus) {
    for (int i = 0; i < 8; i++) {
      int val = analogRead(zonePins[i]);
      Serial.print(F("Z")); Serial.print(i+1); Serial.print(F(":"));
      if (val < SHORT_LIMIT) Serial.print(F("КЗ"));
      else if (val >= OPEN_LIMIT) Serial.print(F("ОБРЫВ"));
      else Serial.print(val);
      Serial.print(F(" "));
    }
    Serial.print(F(" | СТАТУС: "));
    if (isFireActive) Serial.print(F("ПОЖАР!"));
    else if (isSecActive) Serial.print(F("ВЗЛОМ!"));
    else if (isFaultActive) Serial.print(F("НЕИСПРАВНОСТЬ"));
    else if (armingProcess) Serial.print(F("ВЗЯТИЕ..."));
    else if (isArmed) Serial.print(F("ОХРАНА"));
    else Serial.print(F("СНЯТО"));
    Serial.println();
  }

  // Активация тревог
  if ((rawFire || rawSec) && !isFireActive && !isSecActive && !isFaultActive) {
    if (rawFire) { isFireActive = true; activeZone = currentAlarmZone; }
    else { isSecActive = true; activeZone = currentAlarmZone; }
    dfTrackStarted = false;
  }
  if (isAnyFault && !isFireActive && !isSecActive && !isFaultActive) {
    isFaultActive = true;
    activeZone = currentFaultZone;
    myDFPlayer.stop();
    dfTrackStarted = false;
  }

  // Таймер постановки
  if (armingProcess) {
    if (currentMillis - armingStartTime >= 15000) {
      armingProcess = false; isArmed = true;
      tone(buzzerPin, 1500, 500);
    } else {
      digitalWrite(ledPin, (currentMillis / 500) % 2);
      if (currentMillis - beepTimer >= 1000) {
        beepTimer = currentMillis;
        tone(buzzerPin, 1000, 150);
      }
    }
  } else {
    digitalWrite(ledPin, isArmed ? HIGH : LOW);
  }

  // Индикация зон 1-8
  for (int i = 0; i < 8; i++) {
    if (isFaultActive && i == activeZone) 
      digitalWrite(zoneLedPins[i], (currentMillis / 500) % 2);
    else if (isFaultActive) 
      digitalWrite(zoneLedPins[i], LOW);
    else if ((isFireActive || isSecActive) && alarmMemory[i]) 
      digitalWrite(zoneLedPins[i], (currentMillis / 500) % 2);
    else 
      digitalWrite(zoneLedPins[i], (i < 6) ? HIGH : (isArmed ? HIGH : LOW));
  }

  // Дублирующие индикаторы
  digitalWrite(zoneLed7, 
    isFaultActive && activeZone == 6 ? (currentMillis / 500) % 2 :
    (isFireActive || isSecActive) && alarmMemory[6] ? (currentMillis / 500) % 2 :
    isArmed ? HIGH : LOW
  );
  digitalWrite(zoneLed8, 
    isFaultActive && activeZone == 7 ? (currentMillis / 500) % 2 :
    (isFireActive || isSecActive) && alarmMemory[7] ? (currentMillis / 500) % 2 :
    isArmed ? HIGH : LOW
  );

  // Табло "ВЫХОД"
  digitalWrite(exitSignPin, 
    (isFireActive || isSecActive || isFaultActive || armingProcess) ? 
    (currentMillis / 500) % 2 : HIGH
  );

  // DFPlayer
  if ((isFireActive || isSecActive) && !dfTrackStarted) {
    if (isFireActive) { 
      myDFPlayer.loop(FIRE_TRACK); 
      currentPlayingTrack = FIRE_TRACK; 
    } else { 
      myDFPlayer.loop(SEC_TRACK); 
      currentPlayingTrack = SEC_TRACK; 
    }
    dfTrackStarted = true;
  }

  // Зуммер
  if (isFireActive || isSecActive) {
    if ((currentMillis / 300) % 2) tone(buzzerPin, 2500); else noTone(buzzerPin);
  } 
  else if (isFaultActive) {
    unsigned long cycle = currentMillis % 2500;
    if (cycle < 150) tone(buzzerPin, 1600);
    else if (cycle < 300) noTone(buzzerPin);
    else if (cycle < 450) tone(buzzerPin, 1200);
    else if (cycle < 600) noTone(buzzerPin);
    else if (cycle < 750) tone(buzzerPin, 800);
    else noTone(buzzerPin);
  }
}
