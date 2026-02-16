# Mega OPS (Система Охранно-Пожарной Сигнализации)

Прошивка для Arduino Mega, реализующая 8-зонную систему безопасности с управлением по RFID, звуковым оповещением и хранением данных в EEPROM.

## 📋 Основные характеристики (RU)
* **Контроль 8 зон:** Поддержка 6 пожарных зон и 2 охранных зон через аналоговые входы A0-A7.
* **RFID управление:** Постановка на охрану, снятие и сброс тревог с помощью карт MFRC522.
* **Голосовое оповещение:** Использование DFPlayer Mini для воспроизведения треков при пожаре или взломе.
* **Память:** Сохранение до 10 авторизованных RFID-карт в энергонезависимой памяти EEPROM.
* **Безопасность:** Встроенный Watchdog (сторожевой таймер) на 2 секунды для защиты от зависаний.

## 🔌 Подключение / Wiring
* **RFID (SPI):** RST -> 49, SS (SDA) -> 53, MOSI -> 51, MISO -> 50, SCK -> 52.
* **DFPlayer Mini:** Serial1 (TX/RX).
* **Buzzer / Зуммер:** Pin 6.
* **LED Status / Статус:** Pin 4.
* **Zone LEDs / Индикаторы зон:** Pins 22-29.
* **Exit Sign / Табло "ВЫХОД":** Pin 5.

---

# Mega FPS (Fire & Security Alarm System)

Arduino Mega-based firmware featuring an 8-zone security system with RFID control, voice notifications, and EEPROM data storage.

## 📋 Key Features (EN)
* **8-Zone Monitoring:** Supports 6 fire zones and 2 security zones via analog inputs A0-A7.
* **RFID Control:** Arm, disarm, and reset alarms using MFRC522 cards.
* **Voice Notification:** DFPlayer Mini integration for playing specific tracks during fire or intrusion events.
* **Non-volatile Memory:** Stores up to 10 authorized RFID cards in EEPROM.
* **Reliability:** Built-in 2-second Watchdog timer to prevent system freezes.

## ⚙️ Zone Logic
* **< 250 (SHORT_LIMIT):** Short Circuit / Alarm.
* **> 1000 (OPEN_LIMIT):** Line Break / Fault.
* **250 - 1000:** Normal state.

## 💻 Serial Commands (115200 baud)
* **RFID new** — Enter card programming mode.
* **RFID list** — Show all stored card IDs.
* **RFID N del** — Delete card at index N.

## 🎵 SD Card Structure
1. **001.mp3** — Fire Alarm track.
2. **002.mp3** — Security Alarm track.
3. 
