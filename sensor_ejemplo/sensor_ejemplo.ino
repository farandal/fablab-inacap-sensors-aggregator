// ==============================================================================
// TÍTULO: Adquisición de Datos DHT11 y Envío por Bloques HTTP POST
// DESCRIPCIÓN: Acumula lecturas del DHT11 en un buffer y las envía en formato JSON
//              al backend (endpoint /ingest) mediante HTTP POST con Bearer Token.
// Compatible con ESP32 / ESP8266 (necesitan WiFi para el POST real).
// ==============================================================================

#include <DHT.h>
#include <ArduinoJson.h>   // ArduinoJson v6 o superior
#include <WiFi.h>          // ESP32. Para ESP8266 usar <ESP8266WiFi.h>
#include <HTTPClient.h>

// ---------- Configuracion de red y backend ----------
const char* WIFI_SSID     = "TU_WIFI";
const char* WIFI_PASSWORD = "TU_PASSWORD";
const char* SERVER_URL    = "https://tu-servicio.onrender.com/ingest";
const char* SENSOR_ID     = "arduino_r3_lab_1";
const char* BEARER_TOKEN  = "tu_token_secreto_123"; // debe coincidir con SENSOR_TOKENS del backend

#define DHTPIN 2
#define DHTTYPE DHT11
#define MAX_REGISTROS 5 // Cuantas lecturas se acumulan antes de enviar el bloque

DHT dht(DHTPIN, DHTTYPE);

// Estructura para almacenar una lectura individual en el buffer
struct Lectura {
  String atributo;
  float valor;
  unsigned long delta_ms;
};

Lectura buffer[MAX_REGISTROS];
int contadorRegistros = 0;
unsigned long tiempoPrimeraLectura = 0;

void conectarWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println(" OK");
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  conectarWiFi();
  Serial.println("Sistema iniciado. Acumulando lecturas para envio en bloques...");
}

void loop() {
  delay(2000); // Intervalo minimo recomendado para el DHT11

  float humedad = dht.readHumidity();
  float temperatura = dht.readTemperature();

  if (isnan(humedad) || isnan(temperatura)) {
    Serial.println("Error: No se pudo leer el sensor DHT11");
    return;
  }

  unsigned long tiempoActual = millis();

  // Si es la primera lectura del bloque, registramos el tiempo base
  if (contadorRegistros == 0) {
    tiempoPrimeraLectura = tiempoActual;
  }

  unsigned long delta_ms = tiempoActual - tiempoPrimeraLectura;

  // Almacenar temperatura en el buffer
  buffer[contadorRegistros].atributo = "temperatura";
  buffer[contadorRegistros].valor = temperatura;
  buffer[contadorRegistros].delta_ms = delta_ms;
  contadorRegistros++;

  // Verificar si el buffer tiene espacio para la humedad (cada ciclo genera ambas)
  if (contadorRegistros < MAX_REGISTROS) {
    buffer[contadorRegistros].atributo = "humedad";
    buffer[contadorRegistros].valor = humedad;
    buffer[contadorRegistros].delta_ms = delta_ms;
    contadorRegistros++;
  }

  // Cuando el buffer alcanza o supera MAX_REGISTROS, enviamos el POST
  if (contadorRegistros >= MAX_REGISTROS) {
    enviarBloqueHTTP();
    contadorRegistros = 0; // Limpiar buffer (delta se reinicia en la proxima lectura)
  }
}

void enviarBloqueHTTP() {
  StaticJsonDocument<1024> doc;
  doc["sensor_id"] = SENSOR_ID;

  JsonArray lecturas = doc.createNestedArray("lecturas");
  for (int i = 0; i < contadorRegistros; i++) {
    JsonObject obj = lecturas.createNestedObject();
    obj["atributo"] = buffer[i].atributo;
    obj["valor"] = buffer[i].valor;
    obj["delta_ms"] = buffer[i].delta_ms;
  }

  String jsonOutput;
  serializeJson(doc, jsonOutput);

  Serial.println("\n--- ENVIANDO BLOQUE HTTP POST ---");
  Serial.println(jsonOutput);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Sin WiFi, se descarta el bloque");
    return;
  }

  HTTPClient http;
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String("Bearer ") + BEARER_TOKEN);

  int codigo = http.POST(jsonOutput);
  Serial.printf("Respuesta del servidor: %d\n", codigo);
  if (codigo > 0) {
    Serial.println(http.getString());
  }
  http.end();
  Serial.println("----------------------------------\n");
}
