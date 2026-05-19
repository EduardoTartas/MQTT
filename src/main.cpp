/*
 * =============================================================================
 * TRABALHO IoT — Publicação de Dados Aleatórios via MQTT
 * Grupo 01 | Disciplina: Internet das Coisas
 * =============================================================================
 *
 * DESCRIÇÃO:
 *   Este firmware roda no ESP32 e implementa todos os requisitos do trabalho:
 *     1. Conecta-se à rede Wi-Fi
 *     2. Gera valores aleatórios entre 0 e 100
 *     3. Conecta-se a um broker MQTT (Mosquitto rodando em Docker)
 *     4. Publica os valores no tópico "iot/grupo01/sensor" a cada 5 segundos
 *     5. Exibe informações no monitor serial (115200 baud)
 *
 * REQUISITOS ADICIONAIS IMPLEMENTADOS (diferenciais):
 *     - Payload em formato JSON com campos: grupo, valor, envio
 *     - Reconexão automática ao Wi-Fi e ao broker MQTT
 *     - Contador incrementando a cada publicação
 *     - Temporização via millis() (sem travar o processador com delay)
 *
 * HARDWARE:
 *     Placa: ESP32 DOIT DevKit V1
 *
 * DEPENDÊNCIA:
 *     PubSubClient by Nick O'Leary (v2.8+)
 *     Instalar via PlatformIO: lib_deps = knolleary/PubSubClient @ ^2.8
 * =============================================================================
 */

#include <Arduino.h>    // Funções base do framework Arduino (Serial, millis, etc.)
#include <WiFi.h>       // Biblioteca Wi-Fi nativa do ESP32
#include <PubSubClient.h> // Biblioteca MQTT (publish/subscribe)

// =============================================================================
// CONFIGURAÇÕES — alterar conforme o ambiente de uso
// =============================================================================

// Nome e senha da rede Wi-Fi (deve ser 2.4 GHz, ESP32 não suporta 5 GHz)
const char* WIFI_SSID  = "VHA-LAB1L-007";
const char* WIFI_SENHA = "10203040";

// IP do computador que roda o broker Mosquitto via Docker
// IMPORTANTE: não usar "localhost" — o ESP32 acessa o broker pela rede Wi-Fi,
// então é necessário o IP real da máquina nessa rede.
// Como descobrir: no Linux/Mac execute: hostname -I | awk '{print $1}'
//                 no Windows execute: ipconfig → "Endereço IPv4"
const char* MQTT_BROKER = "10.42.0.164";

// Porta padrão do protocolo MQTT (sem criptografia)
const int MQTT_PORTA = 1883;

// Tópico MQTT único do grupo — estrutura hierárquica: área/grupo/tipo-de-dado
// O broker roteia mensagens por tópico; assinantes do mesmo tópico recebem tudo
const char* MQTT_TOPICO = "iot/grupo01/sensor";

// Identificador único deste cliente no broker (dois clientes com mesmo ID se desconectam)
const char* MQTT_CLIENT = "esp32-grupo01";

// Intervalo entre publicações em milissegundos (5000 ms = 5 segundos)
const unsigned long INTERVALO_MS = 5000;

// =============================================================================
// VARIÁVEIS GLOBAIS
// =============================================================================

// WiFiClient abre um socket TCP — o PubSubClient usa essa conexão para MQTT
WiFiClient   wifiClient;
PubSubClient mqttClient(wifiClient); // passa o socket TCP para o cliente MQTT

// Armazena o momento do último envio para calcular o intervalo sem usar delay()
unsigned long ultimoEnvio = 0;

// Contador de publicações — incrementa a cada envio, aparece no campo "envio" do JSON
unsigned int contador = 0;

// =============================================================================
// FUNÇÃO: conectarWiFi()
// Conecta o ESP32 à rede Wi-Fi e aguarda até ter IP atribuído pelo roteador.
// Fica em loop imprimindo "." até a conexão ser estabelecida.
// A verificação inicial (isConnected) evita reconectar desnecessariamente.
// =============================================================================
void conectarWiFi() {
    // Se já está conectado, não faz nada
    if (WiFi.isConnected()) return;

    Serial.printf("\nConectando ao Wi-Fi: %s", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_SENHA); // inicia o processo de associação

    // Aguarda até obter IP (DHCP do roteador atribui endereço)
    while (!WiFi.isConnected()) {
        delay(500);
        Serial.print(".");
    }

    // Exibe o IP atribuído — necessário para configurar no broker se for o contrário
    Serial.printf("\nWi-Fi conectado! IP: %s\n", WiFi.localIP().toString().c_str());
}

// =============================================================================
// FUNÇÃO: conectarMQTT()
// Conecta o ESP32 ao broker MQTT usando TCP via Wi-Fi.
// Em caso de falha, aguarda 3 segundos e tenta novamente.
// O código de retorno (rc) informa o motivo da falha (ex: rc=-2 = sem rota).
// =============================================================================
void conectarMQTT() {
    while (!mqttClient.connected()) {
        Serial.print("Conectando ao broker MQTT...");

        // connect() envia o pacote CONNECT do protocolo MQTT
        // Parâmetro: clientId — identificador único desta sessão no broker
        if (mqttClient.connect(MQTT_CLIENT)) {
            Serial.println(" Conectado!");
        } else {
            // state() retorna código de erro: -4=timeout, -3=servidor recusou,
            // -2=falha na rede, -1=cliente desconectado, 1-5=erros do protocolo
            Serial.printf(" Falhou (rc=%d). Tentando em 3s...\n", mqttClient.state());
            delay(3000);
        }
    }
}

// =============================================================================
// FUNÇÃO: publicarValor()
// Gera um número aleatório entre 0 e 100, monta o payload em JSON e publica
// no broker MQTT. Também imprime as informações no monitor serial.
//
// Estrutura do payload JSON:
//   {"grupo":"01","valor":73,"envio":1}
//    ^grupo  ^valor aleatorio  ^contador de envios
// =============================================================================
void publicarValor() {
    // Gera número aleatório entre 0 e 100 (inclusive)
    // random(min, max) retorna min <= x < max, então max=101 para incluir 100
    int valor = random(0, 101);

    // Incrementa o contador de envios (diferencial: rastreia quantas mensagens foram enviadas)
    contador++;

    // Monta o JSON na pilha — snprintf garante que não ultrapassa os 64 bytes
    // \"  é necessário para colocar aspas dentro de string C
    char payload[64];
    snprintf(payload, sizeof(payload),
             "{\"grupo\":\"01\",\"valor\":%d,\"envio\":%u}",
             valor, contador);

    // publish() empacota o payload e envia via protocolo MQTT (pacote PUBLISH)
    // O broker recebe e encaminha para todos os assinantes do tópico
    mqttClient.publish(MQTT_TOPICO, payload);

    // Feedback no monitor serial para acompanhar o envio em tempo real
    Serial.printf("[#%u] Valor enviado: %d | Topico: %s\n", contador, valor, MQTT_TOPICO);
    Serial.printf("Payload: %s\n\n", payload);
}

// =============================================================================
// FUNÇÃO: setup()
// Executada UMA VEZ ao ligar ou resetar o ESP32.
// Inicializa o hardware e as conexões antes do loop principal começar.
// =============================================================================
void setup() {
    // Inicializa a comunicação serial — 115200 baud é padrão para ESP32
    Serial.begin(115200);
    Serial.println("\n=== IoT MQTT — Grupo 01 ===");

    // randomSeed inicializa o gerador de números aleatórios com um valor imprevisível.
    // analogRead(0) lê ruído elétrico do pino A0 desconectado — sempre diferente.
    // Sem isso, random() geraria a mesma sequência toda vez que o ESP32 reinicia.
    randomSeed(analogRead(0));

    // Estabelece conexão Wi-Fi antes de qualquer operação de rede
    conectarWiFi();

    // Aponta o cliente MQTT para o endereço e porta do broker
    // Deve ser chamado antes de conectarMQTT()
    mqttClient.setServer(MQTT_BROKER, MQTT_PORTA);

    // Realiza o handshake MQTT com o broker
    conectarMQTT();
}

// =============================================================================
// FUNÇÃO: loop()
// Executada CONTINUAMENTE após o setup(). É o coração do programa.
// Mantém as conexões ativas, processa pacotes MQTT e dispara publicações
// no intervalo configurado sem bloquear o processador.
// =============================================================================
void loop() {
    // --- Manutenção da conexão Wi-Fi ---
    // Se a conexão Wi-Fi cair (ex: roteador reiniciou), reconecta automaticamente.
    // Requisito adicional: reconexão automática ao Wi-Fi.
    if (!WiFi.isConnected()) {
        Serial.println("Wi-Fi perdido. Reconectando...");
        conectarWiFi();
    }

    // --- Manutenção da conexão MQTT ---
    // Se o broker reiniciou ou houve timeout de keep-alive, reconecta.
    // Requisito adicional: reconexão automática ao broker MQTT.
    if (!mqttClient.connected()) {
        Serial.println("MQTT desconectado. Reconectando...");
        conectarMQTT();
    }

    // ESSENCIAL: mqttClient.loop() processa pacotes internos do protocolo MQTT:
    // keep-alive (PINGREQ/PINGRESP), mensagens recebidas, confirmações de QoS.
    // Sem essa chamada, a conexão cai em segundos por timeout de keep-alive.
    mqttClient.loop();

    // --- Temporização sem delay() ---
    // millis() retorna o tempo em ms desde que o ESP32 ligou (não bloqueia).
    // A diferença (agora - ultimoEnvio) mede o tempo decorrido.
    // Quando atinge INTERVALO_MS (5000ms), publica e atualiza ultimoEnvio.
    // Vantagem sobre delay(5000): o loop() continua rodando no intervalo,
    // mantendo o mqttClient.loop() ativo e as reconexões funcionando.
    unsigned long agora = millis();
    if (agora - ultimoEnvio >= INTERVALO_MS) {
        ultimoEnvio = agora; // registra o momento do envio para calcular o próximo
        publicarValor();
    }
}
