/*
 * IoT MQTT — Grupo 01
 * ESP32 → Broker Mosquitto (Docker)
 *
 * Biblioteca: PubSubClient (Nick O'Leary)
 * Framework:  Arduino (PlatformIO)
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ─── Configurações ────────────────────────────────────────────────────────────

const char* WIFI_SSID     = "VHA-LAB1L-007";
const char* WIFI_SENHA    = "10203040";

// IP da máquina que roda o Docker (não usar localhost)
// Linux/Mac: ip route get 1 | awk '{print $7}' OU hostname -I
// Windows:   ipconfig → "Endereço IPv4"
const char* MQTT_BROKER   = "10.42.0.164";
const int   MQTT_PORTA    = 1883;
const char* MQTT_TOPICO   = "iot/grupo01/sensor";
const char* MQTT_CLIENT   = "esp32-grupo01";

const unsigned long INTERVALO_MS = 5000;  // publicar a cada 5 s

// ─── Variáveis globais ────────────────────────────────────────────────────────

WiFiClient   wifiClient;
PubSubClient mqttClient(wifiClient);

unsigned long ultimoEnvio = 0;
unsigned int  contador    = 0;

// ─── Funções ──────────────────────────────────────────────────────────────────

void conectarWiFi() {
    if (WiFi.isConnected()) return;

    Serial.printf("\nConectando ao Wi-Fi: %s", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_SENHA);

    while (!WiFi.isConnected()) {
        delay(500);
        Serial.print(".");
    }

    Serial.printf("\nWi-Fi conectado! IP: %s\n", WiFi.localIP().toString().c_str());
}

void conectarMQTT() {
    while (!mqttClient.connected()) {
        Serial.print("Conectando ao broker MQTT...");

        if (mqttClient.connect(MQTT_CLIENT)) {
            Serial.println(" Conectado!");
        } else {
            Serial.printf(" Falhou (rc=%d). Tentando em 3s...\n", mqttClient.state());
            delay(3000);
        }
    }
}

void publicarValor() {
    int valor = random(0, 101);
    contador++;

    // Monta payload JSON
    char payload[64];
    snprintf(payload, sizeof(payload),
             "{\"grupo\":\"01\",\"valor\":%d,\"envio\":%u}",
             valor, contador);

    mqttClient.publish(MQTT_TOPICO, payload);

    Serial.printf("[#%u] Valor enviado: %d | Topico: %s\n",
                  contador, valor, MQTT_TOPICO);
    Serial.printf("Payload: %s\n\n", payload);
}

// ─── Setup / Loop ─────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== IoT MQTT — Grupo 01 ===");

    randomSeed(analogRead(0));

    conectarWiFi();

    mqttClient.setServer(MQTT_BROKER, MQTT_PORTA);
    conectarMQTT();
}

void loop() {
    // Reconexão automática Wi-Fi
    if (!WiFi.isConnected()) {
        Serial.println("Wi-Fi perdido. Reconectando...");
        conectarWiFi();
    }

    // Reconexão automática MQTT
    if (!mqttClient.connected()) {
        Serial.println("MQTT desconectado. Reconectando...");
        conectarMQTT();
    }

    mqttClient.loop();

    // Publicar a cada INTERVALO_MS sem usar delay()
    unsigned long agora = millis();
    if (agora - ultimoEnvio >= INTERVALO_MS) {
        ultimoEnvio = agora;
        publicarValor();
    }
}
