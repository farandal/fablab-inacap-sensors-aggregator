# Instructivo completo — Implementación del sensor (Arduino/ESP32)

Guía paso a paso para armar, configurar y probar su sensor conectado al
agregador del curso. Léanla completa antes de empezar a cablear o programar.

## 0. Resumen de lo que van a construir

```
[Sensor DHT11] --(cable)--> [ESP32/ESP8266] --WiFi--> [Backend en Render] --> [Supabase]
```

Su placa lee temperatura y humedad, junta varias lecturas en un "bloque", y
cada cierto tiempo envía ese bloque por HTTP al backend del curso, que lo
guarda y lo deja disponible para consultar.

## 1. Materiales necesarios

- 1x placa **ESP32** o **ESP8266** (necesitan WiFi integrado; un Arduino
  UNO/R3 normal no sirve solo, salvo que agreguen un módulo WiFi aparte).
- 1x sensor **DHT11** (o DHT22, con ajuste menor de código).
- Cables jumper (macho-macho o macho-hembra según su protoboard).
- 1x resistencia de 10kΩ (pull-up para el pin de datos del DHT11; muchos
  módulos DHT11 ya la traen integrada — revisen su módulo).
- Cable USB para programar la placa.
- Computador con Arduino IDE instalado.

## 2. Conexión física (wiring)

| DHT11 | ESP32 | ESP8266 |
|---|---|---|
| VCC | 3V3 | 3V3 |
| GND | GND | GND |
| DATA | GPIO 2 (ajustable en código) | D4 (ajustable en código) |

Si su módulo DHT11 no trae resistencia pull-up integrada, conecten una
resistencia de 10kΩ entre el pin **DATA** y **VCC**.

> Revisen el pinout específico de su placa (algunas etiquetan los pines GPIO
> distinto a la serigrafía). Si no están seguros, pregunten antes de alimentar
> el circuito.

## 3. Instalar Arduino IDE y soporte de placa

1. Descarguen e instalen [Arduino IDE](https://www.arduino.cc/en/software)
   (versión 2.x recomendada).
2. Agreguen soporte para su placa:
   - **ESP32**: `Archivo > Preferencias > URLs Adicionales de Gestor de
     Placas` → agregar
     `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
     → luego `Herramientas > Placa > Gestor de Placas` → buscar `esp32` →
     instalar.
   - **ESP8266**: URL adicional
     `http://arduino.esp8266.com/stable/package_esp8266com_index.json` →
     mismo proceso, buscar `esp8266` en el Gestor de Placas.
3. Conecten la placa por USB y selecciónenla en `Herramientas > Placa` y el
   puerto correcto en `Herramientas > Puerto`.

## 4. Instalar librerías necesarias

`Programa/Sketch > Incluir Librería > Administrar Bibliotecas` (o
`Ctrl+Shift+I`) y busquen/instalen:

- **`DHT sensor library`** (de Adafruit)
- **`Adafruit Unified Sensor`** (dependencia de la anterior)
- **`ArduinoJson`** (versión 6.x, autor Benoit Blanchon)

## 5. Obtener el código base

Descarguen o clonen el repositorio con el ejemplo:

```
https://github.com/farandal/fablab-inacap-sensors-aggregator
```

Abran el archivo [`sensor_ejemplo/sensor_ejemplo.ino`](sensor_ejemplo/sensor_ejemplo.ino)
en Arduino IDE.

## 6. Datos que el profesor/ayudante les entrega

Antes de configurar el código necesitan que les asignen:

- **`SENSOR_ID`**: identificador único de su sensor/grupo, ej. `arduino_r3_lab_3`
- **`BEARER_TOKEN`**: token secreto asociado a ese `SENSOR_ID`

El endpoint del backend ya está fijo (no cambia entre grupos):

```
https://fablab-inacap-sensors-backend.onrender.com/ingest
```

## 7. Configurar el código

Al inicio de `sensor_ejemplo.ino`, editen estas líneas:

```cpp
const char* WIFI_SSID     = "SU_RED_WIFI";
const char* WIFI_PASSWORD = "SU_PASSWORD_WIFI";
const char* SERVER_URL    = "https://fablab-inacap-sensors-backend.onrender.com/ingest";
const char* SENSOR_ID     = "SU_SENSOR_ID";       // el que les asignaron
const char* BEARER_TOKEN  = "SU_TOKEN";           // el que les asignaron
```

Si usan **ESP8266** en vez de ESP32, cambien también:

```cpp
#include <WiFi.h>          // ESP32 -> reemplazar por:
#include <ESP8266WiFi.h>   // ESP8266
```

Y si conectaron el DHT11 a un pin distinto de GPIO 2, ajusten:

```cpp
#define DHTPIN 2   // cambiar por el pin que usaron
```

## 8. Compilar y cargar

1. Verifiquen que la placa y el puerto correctos estén seleccionados en
   `Herramientas`.
2. Click en **Verificar** (✓) para compilar sin errores.
3. Click en **Subir** (→) para cargar el programa a la placa.
4. Abran el **Monitor Serial** (`Herramientas > Monitor Serie`, velocidad
   `9600` baudios) para ver los logs.

## 9. Qué deberían ver en el Monitor Serial

```
Conectando a WiFi..... OK
Sistema iniciado. Acumulando lecturas para envio en bloques...

--- ENVIANDO BLOQUE HTTP POST ---
{"sensor_id":"arduino_r3_lab_3","lecturas":[{"atributo":"temperatura","valor":24.5,"delta_ms":0},{"atributo":"humedad","valor":60.2,"delta_ms":0},...]}
Respuesta del servidor: 201
{"ok":true,"insertados":5,"serverTimestamp":"..."}
----------------------------------
```

Si ven `Respuesta del servidor: 201`, su sensor está funcionando
correctamente.

## 10. Verificar sus datos en el backend

Desde un navegador o con `curl`, reemplazando `SU_SENSOR_ID`:

```
https://fablab-inacap-sensors-backend.onrender.com/lecturas?sensor_id=SU_SENSOR_ID
```

Deberían ver un arreglo JSON con sus lecturas y el `timestamp` calculado por
el servidor.

## 11. Solución de problemas

| Síntoma | Causa probable | Solución |
|---|---|---|
| `Conectando a WiFi.....` no termina nunca | SSID/password incorrectos, o red de 5GHz (ESP32/ESP8266 solo soportan 2.4GHz) | Revisar credenciales; usar red 2.4GHz |
| `Error: No se pudo leer el sensor DHT11` | Cableado incorrecto, sensor dañado, o pin mal configurado | Revisar conexión VCC/GND/DATA y el valor de `DHTPIN` |
| `Respuesta del servidor: 401` | Falta el header Authorization o está mal armado | Revisar que `BEARER_TOKEN` esté bien copiado (sin espacios extra) |
| `Respuesta del servidor: 403` | El token no corresponde a ese `SENSOR_ID` | Confirmar `SENSOR_ID` y `BEARER_TOKEN` con el profesor |
| `Respuesta del servidor: 413` | El bloque (`MAX_REGISTROS`) es más grande que el límite del servidor | Reducir `MAX_REGISTROS` en el código (probar con 5 o 10) |
| `Respuesta del servidor: -1` o negativa | Sin conexión a internet, o URL mal escrita | Revisar `SERVER_URL` y la conexión WiFi |
| Tarda varios segundos en responder la primera vez | El backend gratuito "duerme" tras inactividad | Es normal, esperar y reintentar |

## 12. Extensión opcional: agregar otro atributo/sensor

Si quieren reportar otra variable (ej. luz con un fotoresistor), agreguen una
línea similar a esta dentro de `loop()`, respetando el patrón
`atributo` / `valor` / `delta_ms`:

```cpp
if (contadorRegistros < MAX_REGISTROS) {
  buffer[contadorRegistros].atributo = "luz";
  buffer[contadorRegistros].valor = analogRead(34); // ejemplo con ESP32
  buffer[contadorRegistros].delta_ms = delta_ms;
  contadorRegistros++;
}
```

No hay que avisarle nada especial al backend: al ser un esquema EAV
(`atributo`/`valor`), cualquier nombre de atributo nuevo se acepta
automáticamente.

## 13. Entregables (checklist)

- [ ] Código `.ino` final subido a su repositorio.
- [ ] Foto o diagrama de su cableado.
- [ ] Captura del Monitor Serial mostrando al menos un `POST` exitoso (`201`).
- [ ] Captura de `GET /lecturas?sensor_id=SU_SENSOR_ID` mostrando sus datos.
- [ ] `SENSOR_ID` usado (para registrar cuál grupo es cuál).
