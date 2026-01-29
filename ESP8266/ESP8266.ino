#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

/* ===== WIFI ===== */
const char* ssid = "iPhone de Pedro";
const char* password = "01235476Pp#";

/* ===== PINOS NODEMCU ===== */
#define TRIGPIN 14   // D5
#define ECHOPIN 12   // D6

/* ===== CALIBRAÇÃO NOVA ===== */
int distanciaCheio = 15;    // < 15cm = 100%
int distanciaVazio = 150;    // >150cm = 0%
int capacidadeLitros = 20;

/* ===== SERVIDOR ===== */
ESP8266WebServer server(80);

float duration;
float distance;

/* ===== FUNÇÃO MEDIÇÃO ===== */
int calcularNivelPercentual() {

  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);

  duration = pulseIn(ECHOPIN, HIGH, 30000);

  if (duration == 0) return -1;

  distance = (duration * 0.0343) / 2;

  distance = constrain(distance, distanciaCheio, distanciaVazio);

  int nivel = map(distance, distanciaVazio, distanciaCheio, 0, 100);
  nivel = constrain(nivel, 0, 100);

  return nivel;
}

/* ===== ENDPOINT ===== */
void handleDados() {

  int percentual = calcularNivelPercentual();

  if (percentual < 0) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(500, "application/json", "{\"erro\":true}");
    return;
  }

  int litros = map(percentual, 0, 100, 0, capacidadeLitros);

  String json = "{";
  json += "\"percentual\":" + String(percentual) + ",";
  json += "\"litros\":" + String(litros) + ",";
  json += "\"distancia\":" + String(distance,1);
  json += "}";

  server.sendHeader("Access-Control-Allow-Origin", "*");  // <<< ESSA LINHA
  server.send(200, "application/json", json);
}


/* ===== SETUP ===== */
void setup() {
  Serial.begin(115200);

  pinMode(TRIGPIN, OUTPUT);
  pinMode(ECHOPIN, INPUT);

  Serial.println("\nIniciando conexão WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado com sucesso!");
  Serial.print("IP do ESP8266: ");
  Serial.println(WiFi.localIP());

  server.on("/dados", handleDados);
  server.begin();

  Serial.println("Servidor ESP8266 ativo.");
}

/* ===== LOOP ===== */
void loop() {
  server.handleClient();
}