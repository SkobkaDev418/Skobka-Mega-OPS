# Mega OPS (Система Охранно-Пожарной Сигнализации)

[span_0](start_span)[span_1](start_span)[span_2](start_span)Прошивка для Arduino Mega, реализующая 8-зонную систему безопасности с управлением по RFID, звуковым оповещением и хранением данных в EEPROM[span_0](end_span)[span_1](end_span)[span_2](end_span).

## 📋 Основные характеристики (RU)
* **[span_3](start_span)[span_4](start_span)Контроль 8 зон:** Поддержка 6 пожарных зон и 2 охранных зон через аналоговые входы A0-A7[span_3](end_span)[span_4](end_span).
* **[span_5](start_span)[span_6](start_span)[span_7](start_span)[span_8](start_span)RFID управление:** Постановка на охрану, снятие и сброс тревог с помощью карт MFRC522[span_5](end_span)[span_6](end_span)[span_7](end_span)[span_8](end_span).
* **[span_9](start_span)[span_10](start_span)Голосовое оповещение:** Использование DFPlayer Mini для воспроизведения треков при пожаре или взломе[span_9](end_span)[span_10](end_span).
* **[span_11](start_span)[span_12](start_span)Память:** Сохранение до 10 авторизованных RFID-карт в энергонезависимой памяти EEPROM[span_11](end_span)[span_12](end_span).
* **[span_13](start_span)[span_14](start_span)Безопасность:** Встроенный Watchdog (сторожевой таймер) на 2 секунды для защиты от зависаний[span_13](end_span)[span_14](end_span).

## 🔌 Подключение / Wiring
* **[span_15](start_span)RFID (SPI):** RST -> 49, SS (SDA) -> 53, MOSI -> 51, MISO -> 50, SCK -> 52[span_15](end_span).
* **[span_16](start_span)[span_17](start_span)DFPlayer Mini:** Serial1 (TX/RX)[span_16](end_span)[span_17](end_span).
* **[span_18](start_span)Buzzer / Зуммер:** Pin 6[span_18](end_span).
* **[span_19](start_span)LED Status / Статус:** Pin 4[span_19](end_span).
* **[span_20](start_span)Zone LEDs / Индикаторы зон:** Pins 22-29[span_20](end_span).
* **[span_21](start_span)Exit Sign / Табло "ВЫХОД":** Pin 5[span_21](end_span).

---

# Mega FPS (Fire & Security Alarm System)

[span_22](start_span)[span_23](start_span)[span_24](start_span)Arduino Mega-based firmware featuring an 8-zone security system with RFID control, voice notifications, and EEPROM data storage[span_22](end_span)[span_23](end_span)[span_24](end_span).

## 📋 Key Features (EN)
* **[span_25](start_span)[span_26](start_span)8-Zone Monitoring:** Supports 6 fire zones and 2 security zones via analog inputs A0-A7[span_25](end_span)[span_26](end_span).
* **[span_27](start_span)[span_28](start_span)[span_29](start_span)[span_30](start_span)RFID Control:** Arm, disarm, and reset alarms using MFRC522 cards[span_27](end_span)[span_28](end_span)[span_29](end_span)[span_30](end_span).
* **[span_31](start_span)[span_32](start_span)Voice Notification:** DFPlayer Mini integration for playing specific tracks during fire or intrusion events[span_31](end_span)[span_32](end_span).
* **[span_33](start_span)[span_34](start_span)Non-volatile Memory:** Stores up to 10 authorized RFID cards in EEPROM[span_33](end_span)[span_34](end_span).
* **[span_35](start_span)[span_36](start_span)Reliability:** Built-in 2-second Watchdog timer to prevent system freezes[span_35](end_span)[span_36](end_span).

## ⚙️ Zone Logic
* **[span_37](start_span)[span_38](start_span)< 250 (SHORT_LIMIT):** Short Circuit / Alarm[span_37](end_span)[span_38](end_span).
* **[span_39](start_span)[span_40](start_span)> 1000 (OPEN_LIMIT):** Line Break / Fault[span_39](end_span)[span_40](end_span).
* **[span_41](start_span)250 - 1000:** Normal state[span_41](end_span).

## 💻 Serial Commands (115200 baud)
* [span_42](start_span)`RFID new` — Enter card programming mode[span_42](end_span).
* [span_43](start_span)`RFID list` — Show all stored card IDs[span_43](end_span).
* [span_44](start_span)[span_45](start_span)`RFID N del` — Delete card at index N[span_44](end_span)[span_45](end_span).

## 🎵 SD Card Structure
1. [span_46](start_span)[span_47](start_span)`001.mp3` — Fire Alarm track[span_46](end_span)[span_47](end_span).
2. [span_48](start_span)[span_49](start_span)`002.mp3` — Security Alarm track[span_48](end_span)[span_49](end_span).
3. 
