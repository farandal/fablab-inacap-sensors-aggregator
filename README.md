# fablab-inacap-sensors-aggregator

Firmware de ejemplo (Arduino/ESP32/ESP8266) que acumula lecturas de sensores en
bloques y las envia por HTTP POST al backend agregador.

Backend/API que recibe estos datos: [fablab-inacap-sensors-backend](https://github.com/farandal/fablab-inacap-sensors-backend).

📘 **Estudiantes: ver [INSTRUCTIVO.md](INSTRUCTIVO.md)** para la guía completa
paso a paso (materiales, cableado, instalación de librerías, configuración y
solución de problemas).

## Contenido

- [`sensor_ejemplo/sensor_ejemplo.ino`](sensor_ejemplo/sensor_ejemplo.ino) —
  ejemplo con sensor DHT11 (temperatura/humedad) para ESP32/ESP8266.

## Como funciona

1. El sketch acumula hasta `MAX_REGISTROS` lecturas en un buffer en RAM, cada
   una con `atributo`, `valor` y `delta_ms` (milisegundos desde la primera
   lectura del bloque).
2. Al llenarse el buffer, arma un JSON `{ sensor_id, lecturas: [...] }` y hace
   `POST` al endpoint `/ingest` del backend con header
   `Authorization: Bearer <token>`.
3. El backend reconstruye el timestamp absoluto de cada lectura a partir de
   `delta_ms` y del momento de recepcion del bloque.

## Configuracion

Edita al inicio de [`sensor_ejemplo.ino`](sensor_ejemplo/sensor_ejemplo.ino):

```cpp
const char* WIFI_SSID     = "TU_WIFI";
const char* WIFI_PASSWORD = "TU_PASSWORD";
const char* SERVER_URL    = "https://tu-servicio.onrender.com/ingest";
const char* SENSOR_ID     = "arduino_r3_lab_1";
const char* BEARER_TOKEN  = "tu_token_secreto_123"; // debe coincidir con SENSOR_TOKENS del backend
```

## Requisitos

- Placa con WiFi: ESP32 o ESP8266 (un Arduino UNO/R3 sin WiFi necesita un
  modulo ESP8266 en modo AT o un shield Ethernet, adaptando el bloque de envio).
- Libreria **ArduinoJson** (v6+) — Sketch > Include Library > Manage Libraries.
- Libreria **DHT sensor library** (Adafruit) para el ejemplo DHT11.

## Payload enviado

```json
{
  "sensor_id": "arduino_r3_lab_1",
  "lecturas": [
    { "atributo": "temperatura", "valor": 24.5, "delta_ms": 0 },
    { "atributo": "humedad", "valor": 60.2, "delta_ms": 0 }
  ]
}
```
